// Enemy data invariants. M2.4's data half.
//
// `LLK.Enemies.Perception` tests the RULE; this file tests the DATA the rule
// reads. The split matters because the rule's correctness is about senses and
// light, while the data's correctness is about six families staying internally
// consistent while Chris retunes them - and a C++ test that hardcoded all six
// stat lines would fail on every retune, which trains people to edit the test
// instead of reading it.
//
// What this guards, and why each one is here rather than imagined:
//
//   1. **"Absent means absent" has to stay unambiguous.** `FLLKEnemyDef` uses
//      `TOptional` so a family without `aggroRadiusLit` is distinguishable
//      from one with a zero. That only works while no family AUTHORS a zero
//      for a sense it has - the moment one does, `LLKPerception`'s
//      `WithinStuds` guard treats it as unreachable and the two cases become
//      the same thing again.
//   2. **One sense per family.** `SenseReaching` resolves ties by enum order
//      and its comment promises that no family in the data has two senses
//      that can both fire. That promise is checkable, so it is checked: if a
//      family ever gets two, the precedence becomes a design decision
//      somebody has to make rather than one the function already made.
//   3. **Row 64 line three, reaching into the roster.** Escalation may never
//      be caused by failure. `Glimmers.tagAddsPressure` is 0, and being
//      tagged is something that HAPPENS to a crew rather than something they
//      chose - so a non-zero value here would be failure feeding the meter.
//      It would also need an `ELLKStormSpike` entry, which
//      `escalation.test.mjs` guards, so this is the same rule caught one step
//      earlier and in the place somebody would actually edit.
//   4. **Row 16: every enemy telegraphs.** `audioTelegraphSeconds` is
//      universal and a zero there is a design bug, not a quiet enemy.
//   5. **Row 7: the Warden stays unkillable and senseless.** The one family
//      whose absences are load-bearing.
//
//   node tests/enemies.test.mjs

import { readFileSync, existsSync } from "node:fs";
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

const data = JSON.parse(readFileSync(join(root, "data/enemies.json"), "utf8"));
const families = Object.entries(data).filter(
  ([key, value]) => !key.startsWith("_") && value && typeof value === "object",
);

// The roster's size is a guardrail with a documented history - `_scopeNote`
// records that it moved from four to five to six, each time by Chris's
// approval. So this asserts the count is deliberate, not that it is six
// forever: a seventh is fine, it just has to arrive with the note updated.
check(
  "the roster has the six families the data claims",
  families.length === 6,
  `found ${families.length}: ${families.map(([k]) => k).join(", ")}. ` +
    `_scopeNote says the number moves only by Chris's approval`,
);

// ---- the senses, and which fields constitute one -------------------------
//
// Mirrors LLKPerception::SenseReaching exactly. A sense is the SET of fields
// the rule requires, not any one of them - which is why Sweepers need both
// radii and Stockers need both the range and the cone.
const SENSES = {
  LitProximity: ["aggroRadiusDark", "aggroRadiusLit"],
  SightCone: ["sightRange", "sightConeDegrees"],
  Scan: ["scanRange"],
  Wake: ["wakeRange"],
};

for (const [name, family] of families) {
  // ---- 4. row 16 ---------------------------------------------------------
  check(
    `${name} telegraphs before sight`,
    typeof family.audioTelegraphSeconds === "number" && family.audioTelegraphSeconds > 0,
    "row 16 is an accessibility requirement; a zero here is a design bug, " +
      "not a quiet enemy",
  );

  // ---- 1. no zero stands in for an absence ------------------------------
  const senseFields = new Set(Object.values(SENSES).flat());
  for (const field of senseFields) {
    if (!(field in family)) continue;
    check(
      `${name}.${field} is a real value, not a zero standing in for absence`,
      typeof family[field] === "number" && family[field] > 0,
      "FLLKEnemyDef uses TOptional so absence is expressible; a zero makes " +
        "'this family has no such sense' and 'this sense reaches nothing' the " +
        "same state again",
    );
  }

  // ---- 2. one sense per family ------------------------------------------
  const present = Object.entries(SENSES)
    .filter(([, fields]) => fields.every((f) => typeof family[f] === "number"))
    .map(([sense]) => sense);

  if (name === "Warden") {
    // ---- 5. row 7 --------------------------------------------------------
    check(
      "the Warden has no sense at all - it converges, it does not notice",
      present.length === 0,
      `found ${present.join(", ")}. Row 7 and row 54: it arrives on its own ` +
        `meter and converges on the brightest position. A radius here is the ` +
        `first step to making it a boss`,
    );
    check(
      "and the Warden is still unkillable",
      family.killable === false,
      "row 7. If it ever becomes killable the meter stops meaning anything",
    );
    continue;
  }

  check(
    `${name} has exactly one sense that can fire`,
    present.length === 1,
    `found ${present.length}: ${present.join(", ") || "none"}. ` +
      `LLKPerception::SenseReaching resolves ties by enum order and documents ` +
      `that no family needs it to; two senses makes that precedence a design ` +
      `decision somebody has to make deliberately`,
  );
}

// ---- 3. row 64 line three, in the roster ---------------------------------
{
  const glimmers = data.Glimmers ?? {};
  check(
    "being tagged does not feed zone pressure",
    glimmers.tagAddsPressure === 0,
    "row 64 line three: escalation comes from choices the crew can identify, " +
      "never from failing. A tag happens TO a crew. A non-zero value here " +
      "would also need an ELLKStormSpike entry, which escalation.test.mjs " +
      "guards - this is the same rule caught where somebody would edit it",
  );
  check(
    "and a tag does not hand the Warden a permanent waypoint",
    glimmers.tagGivesWardenPermanentWaypoint === false,
    "row 7 keeps the Warden an escalation rather than a punishment; a " +
      "permanent waypoint earned by being scanned is failure compounding " +
      "itself with no decision left to make",
  );
}

// ---- the relationship the floor plan is built on -------------------------
//
// `LLKStoreFloorPlan`'s one hard rule is that no sightline exceeds the range a
// LIT keeper is noticed from, and `balance.test.mjs` pins the lantern's reach
// against the same number. This is the third leg: the asymmetry itself.
{
  const sweepers = data.Sweepers ?? {};
  check(
    "a lit keeper is noticed from further than a dark one",
    sweepers.aggroRadiusLit > sweepers.aggroRadiusDark,
    "this asymmetry IS the lantern decision. Inverted or equal, light costs " +
      "nothing and the whole game stops teaching",
  );
  check(
    "and by enough to be felt rather than measured",
    sweepers.aggroRadiusLit >= sweepers.aggroRadiusDark * 2,
    "a lit radius a few studs larger is a difference a player cannot notice, " +
      "which is a trade nobody learns to make",
  );
}

// ---- ROW 16: the brain cannot skip the telegraph ------------------------
//
// Chris, 2026-10-30, on why the Sweeper comes before the siren: it "answers
// the real question: whether extraction is a tense decision or twenty seconds
// of waiting." It only answers that fairly if the announcement cannot be
// skipped, which is row 16 - sound before sight, as a build requirement.
//
// `LLK.Enemies.SweeperBrain` asserts the behaviour. This asserts what a
// behavioural test cannot: that the brain's own code has no path from noticing
// to moving which does not pass through the announcement, and that it stays
// world-free so the encounter can be judged before a map exists.
{
  const stripBrainComments = (src) =>
    src.replace(/\/\*[\s\S]*?\*\//g, "").replace(/\/\/[^\n]*/g, "");

  const brainPath = "Source/LLK/Private/Enemies/LLKSweeperBrain.cpp";
  const brainFull = join(root, brainPath);
  const brain = existsSync(brainFull) ? stripBrainComments(readFileSync(brainFull, "utf8")) : "";

  check("the Sweeper brain exists", brain.length > 0, brainPath);

  check(
    "ROW 16: noticing a keeper enters Telegraph",
    /State = ELLKSweeperState::Telegraph;/.test(brain),
    "a Sweeper that went straight to Pursue would be a fair fight with an " +
      "unfair presentation",
  );
  check(
    "ROW 16: and Pursue is only ever entered from Telegraph or Search",
    (brain.match(/State = ELLKSweeperState::Pursue;/g) ?? []).length === 2,
    "two: the telegraph finishing, and a search re-acquiring. A third would " +
      "be a path from noticing to chasing with no announcement in between",
  );
  check(
    "ROW 16: the telegraph length is authored rather than a constant",
    /Def\.AudioTelegraphSeconds/.test(brain),
    "audioTelegraphSeconds is Chris's number, per family (row 55)",
  );

  for (const forbidden of ["UWorld", "GetWorld", "AActor", "MoveTo", "NavigationSystem"]) {
    check(
      `the brain never names ${forbidden}`,
      !brain.includes(forbidden),
      "the pawn measures and moves; the brain decides. The same split " +
        "LLKMapReadiness and LLKLanternFeel use",
    );
  }
}

// ---- the pawn authors nothing the map should ---------------------------
//
// Chris, 2026-10-30: *"Do not invent patrol points, route shapes, collision
// tuning, or spawn spacing. The dock map should author those."* Each of those
// is a thing somebody could add to the pawn in one line and nobody would
// notice until a real floor disagreed with it.
{
  const pawnPath = "Source/LLK/Private/Enemies/LLKSweeper.cpp";
  const pawnFull = join(root, pawnPath);
  const pawn = existsSync(pawnFull) ? readFileSync(pawnFull, "utf8") : "";
  const code = pawn.replace(/\/\*[\s\S]*?\*\//g, "").replace(new RegExp("//[^" + String.fromCharCode(92) + "n]*", "g"), "");

  check("the Sweeper pawn exists", code.length > 0, pawnPath);

  // Collision tuning: the capsule, the gravity, the channels. The defaults
  // stand until a map asks for something else.
  for (const forbidden of [
    "SetCapsuleSize",
    "SetCollisionResponseToChannel",
    "GravityScale",
  ]) {
    check(
      `the pawn does not tune collision (${forbidden})`,
      !code.includes(forbidden),
      "a Sweeper's real dimensions are a question about doorways and shelf " +
        "gaps, which is map work",
    );
  }

  // `SetCollisionEnabled` was a fourth entry in that list until 2026-11-02,
  // and banning the symbol was the wrong operand for the rule. Chris's rule
  // forbids the pawn CHOOSING ITS DIMENSIONS. Switching a decorative mesh off
  // is the opposite of that - it is what keeps the footprint the capsule's,
  // exactly as the constructor promises - and `8196893` needed it, because a
  // cosmetic chassis with collision was carving a navmesh hole that travelled
  // with the pawn. The ban made that fix report as a violation.
  //
  // So the shape matters, not the symbol. Two things are forbidden: touching
  // the pawn's own capsule, and setting any mode other than off.
  const collisionCalls = [
    ...code.matchAll(/([A-Za-z_][A-Za-z0-9_()]*)\s*->\s*SetCollisionEnabled\(\s*([^)]*?)\s*\)/g),
  ].map(([, receiver, argument]) => ({ receiver, argument }));

  const onTheCapsule = collisionCalls.filter(({ receiver }) =>
    /^(GetCapsuleComponent\(\)|RootComponent|GetMesh\(\))$/.test(receiver),
  );
  check(
    "the pawn never switches its own capsule's collision",
    onTheCapsule.length === 0,
    "collision remains ACharacter's capsule, which is the constructor's own " +
      `promise; found ${onTheCapsule.map((c) => c.receiver).join(", ")}`,
  );

  const retuned = collisionCalls.filter(
    ({ argument }) => argument !== "ECollisionEnabled::NoCollision",
  );
  check(
    "decorative parts are only ever switched off, never given a collision mode",
    retuned.length === 0,
    "choosing a mode is tuning; switching off is declining to tune. Found " +
      retuned.map((c) => `${c.receiver} -> ${c.argument}`).join(", "),
  );

  // Route shape and spacing: no generated points, no spawn offsets.
  check(
    "the route is whatever the map tagged",
    /ActorHasTag\(PatrolPointTag\)/.test(code),
    "three empty actors in a line is a valid route; the pawn has no opinion " +
      "about the shape",
  );
  check(
    "and the pawn never spawns its own patrol points",
    !/SpawnActor/.test(code),
    "a generated route would look like a working patrol and teach a floor " +
      "plan that does not exist",
  );

  // The authored speed, converted at the boundary and nowhere invented.
  check(
    "the speed is the authored one",
    /SpeedFromStuds\(Def\.MoveSpeed\)/.test(code),
    "moveSpeed 14 is Chris's number (row 55), and row 61 keeps the " +
      "conversion in LLKUnits",
  );

  // And the brain stays the only thing that decides: the pawn may not
  // shortcut a state, which is how row 16 would quietly die.
  check(
    "ROW 16: the pawn never sets a state itself",
    !/Mind\.State\s*=/.test(code),
    "the brain decides and this acts. A pawn that could set Pursue directly " +
      "could skip the telegraph without touching the tested rule",
  );
}
// ---- what the generated asset actually received --------------------------
//
// THE GAP THIS FILE HAD. Everything above reads `data/enemies.json`, and the
// automation tests assign `AggroRadiusDark = 12.f` by hand. Both sides stayed
// green while the number authored here never reached the runtime at all: six
// PIE runs on 2026-10-31 logged `notices a dark keeper at 0 studs and a lit
// one at 0`, and nothing anywhere compared the authored value against the
// generated one. Row 5's whole trade - a lit keeper noticed from 8 studs
// beyond the edge of their own light - was absent from the build for a day.
//
// Why, so this is not left looking like a mystery.
// `UDataTable::CreateTableFromJSONString` sends property types it does not
// recognise through `DataTableUtils::AssignStringToProperty`, which opens with
// `if (InProp && IsSupportedTableProperty(InProp))` - and that predicate
// (`DataTableUtils.cpp:407`, 5.8.4) does not list `FOptionalProperty`. So the
// field is skipped, NO import problem is recorded, and the commandlet's
// error check - which is correct, and does fail on problems - is told nothing.
// Reflection and serialisation are not the issue: `FJsonObjectConverter`
// handles optionals in both directions. It is this one import path, and the
// struct's `TOptional` design is sound.
//
// The method here is a byte scan for the IEEE-754 float32 of each authored
// number. Crude on purpose: it needs no editor, no engine and no commandlet
// run, which is the only reason it can run from this side at all. Two things
// keep it honest.
//
//   - **A control group.** The universal fields are plain `float` and DO
//     import. If one of THOSE is missing, the scan is wrong - the asset format
//     changed under it - and that is reported as a different failure from a
//     field the importer dropped. A check that cannot fail for its own reasons
//     is a check nobody can trust.
//   - **Presence only, one direction.** A hit does not prove the value sits on
//     the right row, and a small integer can appear by coincidence - 0.0
//     occurs thousands of times in the file, and 32.0 and 2.0 both appear
//     while being dropped. So this can catch a value that is ABSENT and can
//     never confirm one is correctly placed. With all seventeen optionals
//     dropped, absence is exactly the signal, and a value restored by a fix
//     cannot hide in the noise because the check fails if ANY is missing.
//
// `Content/Data/` is gitignored, so a fresh checkout has no asset to read and
// this section says it compared nothing rather than quietly passing. That skip
// is the honest shape of the remaining hole: what would close it in CI is
// value verification at import time, beside the existing `VerifyTable`
// row-name check. That is the import commandlet's job and not this lane's.
const UNIVERSAL_FIELDS = new Set(["moveSpeed", "audioTelegraphSeconds"]);

const float32 = (value) => {
  const bytes = Buffer.alloc(4);
  bytes.writeFloatLE(value);
  return bytes;
};

const assetPath = join(root, "Content/Data/DT_Enemies.uasset");
if (!existsSync(assetPath)) {
  console.log(
    "  --   DT_Enemies.uasset is not in this checkout (Content/Data is " +
      "gitignored), so no authored number was compared against a generated one",
  );
} else {
  const asset = readFileSync(assetPath);
  const droppedUniversal = [];
  const droppedOptional = [];

  for (const [family, row] of families) {
    for (const [field, value] of Object.entries(row)) {
      if (typeof value !== "number") continue;
      if (asset.includes(float32(value))) continue;
      const where = `${family}.${field} = ${value}`;
      if (UNIVERSAL_FIELDS.has(field)) droppedUniversal.push(where);
      else droppedOptional.push(where);
    }
  }

  // The control group first, because it decides how to read the next one.
  check(
    "the scan can still see the fields that are known to import",
    droppedUniversal.length === 0,
    `${droppedUniversal.join(", ")} should be in the asset as plain floats. ` +
      "If these are missing the scan is broken, not the data - the asset " +
      "format or the import changed and this check needs rewriting before " +
      "its other result means anything",
  );

  check(
    "every authored enemy number reached the generated DataTable",
    droppedOptional.length === 0,
    `absent from DT_Enemies.uasset: ${droppedOptional.join(", ")}. ` +
      "These are the TOptional fields, which DataTableUtils.cpp:407 does not " +
      "list as a supported table property, so CreateTableFromJSONString " +
      "skips them without reporting a problem. Re-importing will not help " +
      "until the import path stops using that call for this struct",
  );
}

// Decorative components on a moving pawn must not carve the navmesh.
//
// **Nothing in this project set `bCanEverAffectNavigation` anywhere, so the
// default — true — applied to every component.** On 2026-11-02 that stopped
// the Sweeper dead in Boswell's: its temporary chassis punched a nav hole
// centred on itself, `RuntimeGeneration` is `Dynamic` so the hole followed it,
// and it could not path out of a hole it was carrying.
//
// The trap worth guarding is that **`SetCollisionEnabled(NoCollision)` does
// not cover this.** Navigation relevance is a separate property, so the
// obvious correct line gives no protection — which is exactly why a human
// reading the constructor saw "presentation only: collision remains
// ACharacter's capsule" and was right about collision and wrong about nav.
//
// And the symptom hid: perception never consults the navmesh, so the Sweeper
// kept noticing the crate and starting the job. Not a frozen pawn, which
// anyone would have spotted — a pawn behaving correctly in every respect
// except moving.
{
  const pawnPath = "Source/LLK/Private/Enemies/LLKSweeper.cpp";
  const pawnFile = join(root, pawnPath);
  const src = existsSync(pawnFile) ? readFileSync(pawnFile, "utf8") : "";

  // Every mesh the constructor creates, by its subobject name.
  const created = [...src.matchAll(/CreateDefaultSubobject<U(?:Static|Skeletal)MeshComponent>\(TEXT\("([^"]+)"\)\)/g)]
    .map((m) => m[1]);

  check(
    `${pawnPath} still builds its temporary bodies in C++`,
    created.length > 0,
    "no mesh subobjects found. If the temporary rig moved to Content or a " +
      "Blueprint, this check is blind and the nav-relevance rule needs a new " +
      "home — a guard that cannot fail is worse than no guard",
  );

  for (const name of created) {
    // The component's local variable, so the assertion follows a rename.
    const assign = new RegExp(`(\\w+)\\s*=\\s*CreateDefaultSubobject<U(?:Static|Skeletal)MeshComponent>\\(TEXT\\("${name}"\\)\\)`);
    const varName = (src.match(assign) || [, null])[1];
    const disabled =
      varName !== null &&
      new RegExp(`${varName}->SetCanEverAffectNavigation\\(\\s*false\\s*\\)`).test(src);

    check(
      `${name} does not carve the navmesh`,
      disabled,
      `${name} is a decorative mesh on a pawn that walks, and nothing calls ` +
        `SetCanEverAffectNavigation(false) on it. The default is true, so it ` +
        `will punch a dynamic nav hole around the Sweeper and the Sweeper ` +
        `will stop moving while still noticing the player. NoCollision does ` +
        `NOT prevent this`,
    );
  }
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read data/enemies.json and Source/LLK/Public/Enemies/LLKPerception.h.\n");
  process.exit(1);
}
console.log("");
