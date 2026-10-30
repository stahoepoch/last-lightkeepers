// One position decides what the game does about light. This guards which one.
//
// Chris, 2026-10-29, finding the constraint before it shipped: the lantern
// light component is both the visible cone and the position
// `ULLKStormChargeSubsystem::GetLitLightSources` reports into
// `LLKLightField`. That position decides storm accrual, relic legibility, and
// what the Warden converges on - so once the cone sways for feel, reading the
// component's transform means the player sees one light position while the
// server judges another.
//
// `LLK.Lantern.Feel` asserts that `GameplayLightPosition` is pure: no feel
// state can reach it. **That is not enough, and this file exists because of
// the gap.** A pure anchor nobody calls is worth nothing: changing one line in
// the storm subsystem back to `GetComponentLocation()` would pass all 25
// in-engine tests, because no test asserts which position the subsystem
// actually asks for. This does.
//
// Same technique as the threshold-effect guard in `escalation.test.mjs`, and
// for the same reason: the regression is one word in one line, and the thing
// that would catch it is a grep rather than a behaviour.
//
//   node tests/light-authority.test.mjs

import { readFileSync, readdirSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const read = (...p) => readFileSync(join(root, ...p), "utf8");

let failures = 0;
let checks = 0;
const check = (name, condition, detail) => {
  checks += 1;
  if (condition) console.log(`  ok   ${name}`);
  else {
    failures += 1;
    console.log(`  FAIL ${name}${detail ? ` — ${detail}` : ""}`);
  }
};

const storm = read("Source", "LLK", "Private", "Storm", "LLKStormChargeSubsystem.cpp");
const feelHeader = read("Source", "LLK", "Public", "Lantern", "LLKLanternFeel.h");
const feelImpl = read("Source", "LLK", "Private", "Lantern", "LLKLanternFeel.cpp");
const lightImpl = read("Source", "LLK", "Private", "Lantern", "LLKLanternLightComponent.cpp");

// ---- 1. the storm asks for the honest anchor ---------------------------
check(
  "GetLitLightSources reads the gameplay anchor",
  storm.includes("Light->GetGameplayLightLocation()"),
  "this is the position the light field, the Warden and relic legibility all judge",
);
check(
  "and does NOT read the component's own transform",
  !storm.includes("Light->GetComponentLocation()"),
  "the component's transform is the cosmetic one; reading it here is the bug this file exists for",
);

// ---- 2. nobody else reads the cone's transform as a position ----------
//
// The one legitimate use is the owner-null fallback inside the light
// component itself, which is documented there as the case that should never
// be reached in a running game.
const sourceFiles = [];
const walk = (dir) => {
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const p = join(dir, entry.name);
    if (entry.isDirectory()) walk(p);
    else if (/\.(cpp|h)$/.test(entry.name)) {
      sourceFiles.push({ rel: p.slice(root.length + 1).replace(/\\/g, "/"), src: readFileSync(p, "utf8") });
    }
  }
};
walk(join(root, "Source", "LLK"));

const allowed = new Set([
  "Source/LLK/Private/Lantern/LLKLanternLightComponent.cpp",
]);

// Comments stripped first. The headers in this project explain the bug they
// are avoiding by quoting the bad line, and a guard that cannot tell prose
// from code reports the explanation as the offence.
const stripComments = (src) =>
  src.replace(/\/\*[\s\S]*?\*\//g, "").replace(/\/\/.*/g, "");

for (const { rel, src } of sourceFiles) {
  if (allowed.has(rel) || rel.includes("/Tests/")) continue;
  check(
    `${rel} does not treat a lantern light's transform as a position`,
    !/Light(ern)?[A-Za-z]*->GetComponentLocation\(\)/.test(stripComments(src)),
    "only LLKLanternLightComponent may read its own transform, and only for the owner-null case",
  );
}

// ---- 3. the anchor's signature is the guarantee ----------------------
const anchorDecl = feelHeader.match(/GameplayLightPosition\(([^)]*)\)/)?.[1] ?? "";
check(
  "GameplayLightPosition is declared at all",
  anchorDecl.length > 0,
);
check(
  "and takes no feel, sway, offset or state argument",
  !/feel|sway|offset|state/i.test(anchorDecl),
  `signature is (${anchorDecl}) — guarantee 3 is that no cosmetic value can be passed in`,
);
check(
  "and its body reads nothing but its two arguments",
  /return CapsuleLocation \+ FVector\(0\.f, 0\.f, Finite\(EyeHeightCm\)\);/.test(feelImpl),
  "a derived anchor that starts consulting state is an anchor that can disagree with itself",
);

// ---- 4. every cosmetic offset goes through the clamp -----------------
check(
  "AdvanceFeel returns its offset through ClampedOffset",
  (feelImpl.match(/ClampedOffset\(/g) ?? []).length >= 2,
  "an unclamped path is a cone that can be thrown through a wall",
);
check(
  "the clamp collapses to zero when nothing was measured",
  /FreeDistanceCm < 0\.f/.test(feelImpl) && /return FVector::ZeroVector;/.test(feelImpl),
  "an unmeasured frame must show the truth, not a guess",
);
check(
  "and the hard rail is applied before anything else",
  feelImpl.indexOf("Rules.MaxOffsetCm") < feelImpl.indexOf("FreeDistanceCm < 0.f"),
  "clamping to MaxOffsetCm after the wall test would let an absurd offset skip the rail",
);

// ---- 5. the cosmetic state stays out of gameplay files --------------
for (const file of ["Storm/LLKStormChargeSubsystem.cpp", "Loot/LLKRelicComponent.cpp"]) {
  check(
    `${file} never reads the cosmetic feel state`,
    !read("Source", "LLK", "Private", ...file.split("/")).includes("FeelState"),
    "guarantee 3 is that sway cannot change storm, relic or enemy logic",
  );
}

// ---- 6. the component applies the sway to itself, not to the camera --
check(
  "the sway is written to the light's own relative location",
  lightImpl.includes("SetRelativeLocation(BaseRelativeLocation + FeelState.OffsetCm)"),
);
check(
  "and the authored base is captured once rather than accumulated",
  lightImpl.includes("BaseRelativeLocation = GetRelativeLocation();"),
  "an offset added to its own result is a drift",
);
check(
  "nothing in the light component writes to a camera",
  !/Camera[A-Za-z]*->Set/.test(lightImpl),
  "guarantee 1: the cone trails the look and the look trails nothing",
);

console.log(`\n${checks} checks, ${failures} failed`);
process.exit(failures === 0 ? 0 : 1);
