// Lighting guard.
//
// The lighting in this game has been broken and re-broken several times, each
// time by someone changing one number in good faith, and each time it cost an
// evening to find. DONTFUCKUPTHELIGHTS.md explains why these values are what
// they are and how they were proven. This file is that document with teeth:
// if you change a value it names, this fails and points you at it.
//
// It is deliberately blunt. If a change here is genuinely right, update
// DONTFUCKUPTHELIGHTS.md and this file together, having looked at BOTH maps.
//
//   node tests/lighting.test.mjs

import { readFileSync, readdirSync, existsSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");

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

const project = JSON.parse(readFileSync(join(root, "default.project.json"), "utf8"));
const L = project.tree?.Lighting?.$properties ?? {};

// ---- 1. the two that wash the picture out ---------------------------------
//
// Ambient is a flat fill added to every surface, lit or not. It ignores
// Brightness, exposure and whether any lamp is on, so when it is up nothing
// else you change appears to do anything. EnvironmentDiffuseScale is sky
// bounce, which fills interiors from above regardless of the roof.

const ambient = L.Ambient ?? [0, 0, 0];
check(
  "Lighting.Ambient is black",
  Array.isArray(ambient) && ambient.every((c) => c === 0),
  `got ${JSON.stringify(ambient)} — a flat fill on every surface; this is the one that makes the world milky and makes every other lighting change look like it does nothing`,
);

check(
  "Lighting.EnvironmentDiffuseScale is at or near zero",
  typeof L.EnvironmentDiffuseScale === "number" && L.EnvironmentDiffuseScale <= 0.2,
  `got ${L.EnvironmentDiffuseScale} — sky bounce fills every interior from above as if it had no roof`,
);

// ---- 1b. the runtime owner --------------------------------------------------
//
// LightingService applies a profile to Lighting on every playtest, which makes
// it the real authority - it overwrites whatever the project file and the
// place say, a moment after load. That is a good mechanism (one owner beats
// a tug-of-war over editor state) but it means guarding default.project.json
// alone is useless: the file can be correct while the game is white.

const lightingServicePath = join(root, "src", "server", "services", "LightingService.luau");
if (existsSync(lightingServicePath)) {
  const svc = readFileSync(lightingServicePath, "utf8");
  const amb = svc.match(/Ambient\s*=\s*Color3\.fromRGB\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/);
  check(
    "LightingService profile Ambient is black",
    amb !== null && Number(amb[1]) === 0 && Number(amb[2]) === 0 && Number(amb[3]) === 0,
    `got ${amb ? amb.slice(1, 4).join(",") : "unreadable"} — this runs every playtest and overrides the project file`,
  );
  const env = svc.match(/EnvironmentDiffuseScale\s*=\s*([\d.]+)/);
  check(
    "LightingService profile EnvironmentDiffuseScale is at or near zero",
    env !== null && Number(env[1]) <= 0.2,
    `got ${env ? env[1] : "unreadable"} — unconditional sky fill, applied on every playtest`,
  );
}

// ---- 2. shadows ------------------------------------------------------------

check(
  "Lighting.GlobalShadows is on",
  L.GlobalShadows === true,
  "with shadows off, interiors take the full sky and the hub renders as a white box",
);

// ---- 3. it is still night --------------------------------------------------

check(
  "Lighting.ClockTime is still night",
  L.ClockTime === 2,
  `got ${L.ClockTime} — the game is set in the dark; this is not a tuning knob`,
);

check(
  "Lighting.Brightness stays modest",
  typeof L.Brightness === "number" && L.Brightness <= 0.6,
  `got ${L.Brightness}`,
);

// ---- 4. things that ignore the lighting entirely ---------------------------

function luauFiles(dir) {
  const out = [];
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const p = join(dir, entry.name);
    if (entry.isDirectory()) out.push(...luauFiles(p));
    else if (entry.name.endsWith(".luau")) out.push({ rel: p.slice(root.length + 1), src: readFileSync(p, "utf8") });
  }
  return out;
}
const files = luauFiles(join(root, "src"));

// A Beam at LightInfluence 0 is drawn at full brightness no matter what the
// Lighting block says. The hub's floor guide was one, and no lighting change
// could touch it.
for (const f of files) {
  const bad = /LightInfluence\s*(?:,[^=]*)?=\s*0\b/.test(f.src) || /\.LightInfluence\s*=\s*0\b/.test(f.src);
  check(
    `${f.rel} has no Beam at LightInfluence 0`,
    !bad,
    "a beam at 0 ignores every light setting; set LightInfluence = 1",
  );
}

// Neon is fully emissive and blooms. Small glowing eyes are the point; panels
// are not. Fourteen Neon ceiling lenses once washed the whole Galleria white.
const NEON_BUDGET = 4;
for (const f of files) {
  const n = (f.src.match(/Material\.Neon/g) || []).length;
  check(
    `${f.rel} keeps Neon surfaces to a few small ones (${n})`,
    n <= NEON_BUDGET,
    `${n} uses of Material.Neon — Neon ignores the lighting and blooms; it is for a Sweeper's eye, never a panel`,
  );
}

// ---- 5. lamp brightness ----------------------------------------------------
//
// The house lamp is 1.5. Anything much brighter does not read as "important",
// it reads as a blown highlight: the departure markers were 2.2 and 2.6 and
// just whited out the end of the hall.

const LAMP_CEILING = 2;
for (const f of files.filter((x) => x.rel.includes("map"))) {
  for (const m of f.src.matchAll(/Brightness(?:,[^=]*)?\s*=\s*([\d.]+)/g)) {
    const v = Number(m[1]);
    check(
      `${f.rel} lamp brightness ${v} is within the house range`,
      v <= LAMP_CEILING,
      `the house lamp is 1.5; ${v} blows out rather than drawing the eye`,
    );
  }
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read DONTFUCKUPTHELIGHTS.md before changing any of these.\n");
  process.exit(1);
}
console.log("");
