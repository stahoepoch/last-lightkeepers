// Balance invariant tests.
//
// These run on plain Node against the JSON in /data — no Roblox, no Rojo, no
// Luau toolchain. They do not test gameplay code; they test that the numbers
// still say what the handoff says, and that nobody has tuned a value into
// nonsense. Cheap to run, and the only automated check this project has today.
//
//   node tests/balance.test.mjs

import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const load = (name) => JSON.parse(readFileSync(join(root, "data", name), "utf8"));

const storm = load("storm-charge.json");
const loot = load("loot.json");
const enemies = load("enemies.json");
const lantern = load("lantern.json");

let failures = 0;
let checks = 0;

function check(name, condition, detail = "") {
  checks += 1;
  if (condition) {
    console.log(`  ok   ${name}`);
  } else {
    failures += 1;
    console.log(`  FAIL ${name}${detail ? ` — ${detail}` : ""}`);
  }
}

function near(a, b, tolerance = 1) {
  return Math.abs(a - b) <= tolerance;
}

console.log("\nStorm Charge");
{
  const { baselineRatePerSecond: base, thresholds, lantern: lamp } = storm;
  const oneLanternRate = base * (1 + lamp.perLanternBonus);

  check("baseline rate is positive", base > 0);
  check("thresholds ascend", thresholds.every((t, i) => i === 0 || t.value > thresholds[i - 1].value));
  // The Warden left this array on 2026-10-28. It is the closing act, not the
  // normal consequence of using a lantern, so it accrues on its own dampened
  // meter - see storm.warden and the block of checks further down.
  check("the Warden is no longer a generic threshold", !thresholds.some((t) => t.id === "WardenConverges"));
  check("the generic thresholds are the two that keep the building active", thresholds.length === 2);

  // Chris's curve, 2026-10-26, replacing the handoff's 8-minute anchor: for a
  // normal 10-12 minute run, a fully dark crew meets its first real pressure
  // around minute 4-5 and severe pressure near the final minutes. The old
  // anchor put the Warden at 15:59, past the end of any run anyone would play,
  // so darkness carried no consequence at all.
  check(
    "dark crew meets first pressure in minute 4-5",
    thresholds[0].value / base >= 240 && thresholds[0].value / base <= 300,
    `got ${(thresholds[0].value / base).toFixed(0)}s`,
  );
  // The 10-12 minute band is Chris's (2026-10-28, settling O1) and now lives in
  // data/storm-charge.json run.targetBandSeconds. Read it rather than repeating
  // it: a duplicated number is how O1 got four sources carrying three figures.
  const [targetLow, targetHigh] = storm.run.targetBandSeconds;
  check(
    `dark crew meets the Warden inside the target band (${targetLow}-${targetHigh}s)`,
    storm.warden.chargeValue / base >= targetLow && storm.warden.chargeValue / base <= targetHigh,
    `got ${(storm.warden.chargeValue / base).toFixed(0)}s`,
  );
  check(
    "the early exit is earlier than the target run",
    storm.run.earlyExitSeconds < targetLow,
    `early ${storm.run.earlyExitSeconds}s vs target low ${targetLow}s`,
  );
  check(
    "the hard ceiling is above the target band, and is not a second looting window",
    storm.run.hardCeilingSeconds > targetHigh &&
      storm.run.hardCeilingSeconds <= targetHigh + 120,
    `ceiling ${storm.run.hardCeilingSeconds}s vs band top ${targetHigh}s`,
  );
  // The three duty profiles are NAMED in data/storm-charge.json _dutyProfiles,
  // each with its own contract, because Chris warned that "normally lit crew"
  // is a phrase rather than a number: "someone later will read normally lit
  // crew as 50%, see a seven-minute Warden, and correctly think the system
  // drifted again." Assert against the names, never against bare decimals.
  const PROFILES = storm._dutyProfiles;
  const NORMALLY_LIT_DUTY = PROFILES.normal.duty;
  check("the duty profiles are named in data, not implied here", 
    PROFILES.normal.duty < PROFILES.bright.duty && PROFILES.bright.duty < PROFILES.panic.duty);

  // ---- ROW 72: the night has one length, and this suite used to deny it --
  //
  // **Everything between here and the stage targets was a simulation of a
  // model the code no longer implements, and it passed 160/160 while being
  // wrong.** That is the worst state a suite can be in, because a green run
  // reads as evidence. What was here: `wardenAt(crew, duty)`, which projected
  // the Warden's arrival from a lantern multiplier on a diminishing curve and
  // a separate dampened Warden meter, asserted across three named duty
  // profiles and four crew sizes.
  //
  // Row 72 retired all of it. Light does not change the clock's rate, there
  // is one clock rather than two, and so the night is the same length for
  // every crew whatever they do with their lanterns. The simulation had
  // nothing left to vary.
  //
  // What replaces it is smaller and it is the honest shape: one arrival time,
  // derived from the baseline, checked against the bands Chris authored.
  {
    const w = storm.warden;
    const arrival = w.chargeValue / base;
    const [wardenLow, wardenHigh] = w.arrivalBandSeconds;

    // **This is the one number row 72 knocked out of tune, and it is Chris's
    // to set.** The band 540-660s was authored for a NORMALLY LIT crew under
    // the old multiplier: at duty 0.25 the dampened Warden rate was about
    // 1.11x, so 700 charge landed around 630s, inside it. At the flat rate
    // 700 charge is 700 seconds, which is past the band - and there is no
    // longer any such thing as a normally lit crew for the band to describe.
    //
    // Two one-number fixes, both his: set `warden.chargeValue` to about 600
    // (dead centre of the band at 1/s), or widen `warden.arrivalBandSeconds`
    // to include 700. Deliberately left failing rather than papered over: the
    // data and the rule genuinely disagree, and a guard that hid that would
    // be the thing this suite exists to prevent.
    check(
      `the Warden arrives at ${arrival.toFixed(0)}s, inside the authored band ${wardenLow}-${wardenHigh}s`,
      arrival >= wardenLow && arrival <= wardenHigh,
      `got ${arrival.toFixed(0)}s — ROW 72 RETUNE, CHRIS'S CALL: at the flat rate, warden.chargeValue ${storm.warden.chargeValue} is ${arrival.toFixed(0)}s. Set chargeValue to ~${Math.round((wardenLow + wardenHigh) / 2)} or widen arrivalBandSeconds`,
    );
    check(
      "and after the early exit, so an early-leaving crew never meets it",
      arrival > storm.run.earlyExitSeconds,
      `arrival ${arrival.toFixed(0)}s vs early exit ${storm.run.earlyExitSeconds}s`,
    );
    check(
      "and before the hard ceiling, so the ceiling is not a second night",
      arrival <= storm.run.hardCeilingSeconds,
    );
    check(
      "the thresholds are crossed before the Warden arrives",
      thresholds.every((t) => t.value < w.chargeValue),
    );
  }

  // ---- and the guard that the retired data is actually retired ----------
  //
  // The lantern curve, the dampening and the duty profiles are still in
  // data/storm-charge.json: deleting values there is Chris's call (row 55),
  // not this file's. So the guard is not "the data is gone" - it is "the code
  // does not read it." A re-coupling would be one line in the subsystem, and
  // this is the line that would catch it.
  {
    const subsystem = readFileSync(
      join(root, "Source", "LLK", "Private", "Storm", "LLKStormChargeSubsystem.cpp"),
      "utf8",
    ).replace(/\/\*[\s\S]*?\*\//g, "").replace(/\/\/.*/g, "");

    check(
      "ROW 72: the subsystem does not read the lantern curve",
      !subsystem.includes("Rules->Lantern") && !subsystem.includes("LanternMultiplier("),
      "light must not be an input to the clock's rate",
    );
    check(
      "ROW 72: and does not read the Warden's light sensitivity",
      !subsystem.includes("LightSensitivity"),
      "that was the second meter's dampening; there is one meter now",
    );
    check(
      "ROW 72: the rate is the baseline and nothing else",
      /GetCurrentRate\(\) const\s*\{\s*return Rules != nullptr \? Rules->BaselineRatePerSecond : 0\.f;/.test(subsystem),
      "a rate expression with a second term in it is the regression",
    );
    check(
      "the retired lantern block is still in the data, for Chris to delete or keep",
      typeof lamp.perLanternBonus === "number",
      "this guard is about the code, not about tidying his file",
    );
  }

  // Chris's stage targets, 2026-10-28: "First pressure: early, around minutes
  // 3-4. Serious pursuit / normal enemy pressure: around minutes 5-7."
  //
  // REWRITTEN the same day, and the reason matters. The old pair measured
  // "serious pursuit" with `thresholds.at(-1)` - which WAS the Warden. The two
  // ideas were conflated in the test exactly as they were conflated in the
  // data, so splitting the Warden out made this assertion fail and exposed it.
  // "Serious pursuit" is PatrolsHunt and nothing else.
  //
  // Both are read at the same "normally lit" duty used for the Warden above, so
  // the file has one definition of normal play rather than two.
  {
    const normallyLit = base * (1 + (1 + lamp.perLanternBonus - 1) * NORMALLY_LIT_DUTY);
    const alertAt = thresholds[0].value / normallyLit;
    const huntAt = thresholds[1].value / normallyLit;
    check(
      "a normally lit crew meets first pressure in minutes 3-4",
      alertAt >= 180 && alertAt <= 240,
      `got ${alertAt.toFixed(0)}s`,
    );
    check(
      "a normally lit crew meets serious pursuit in minutes 5-7",
      huntAt >= 300 && huntAt <= 420,
      `got ${huntAt.toFixed(0)}s`,
    );
    // Heavier light use pulls both earlier, and that is the point - light always
    // costs. What the Warden split fixed is that it no longer pulls the CLOSING
    // ACT earlier by the same amount.
    const heavilyLit = base * 1.75;
    check(
      "heavier light use pulls first pressure earlier still",
      thresholds[0].value / heavilyLit < alertAt,
    );
  }

  // The handoff's other anchor: one active lantern ≈ 2.5× baseline.
  check(
    "one lantern is ~2.5x baseline",
    near(1 + lamp.perLanternBonus, 2.5, 0.05),
    `got ${(1 + lamp.perLanternBonus).toFixed(2)}x`,
  );

  // Documented per-threshold seconds must match the rates, or the doc lies.
  for (const t of thresholds) {
    check(`${t.id} darkSeconds matches rate`, near(t.value / base, t.darkSeconds, 2));
    check(`${t.id} oneLanternSeconds matches rate`, near(t.value / oneLanternRate, t.oneLanternSeconds, 2));
  }

  check("cap is not below one lantern", lamp.maxMultiplier >= 1 + lamp.perLanternBonus);
  check("solo gets exactly one Anchor Revive", storm.anchorRevive.chargesPerRun === 1);
  check("Anchor Revive costs Storm Charge", storm.anchorRevive.stormChargeSpike > 0);
}

console.log("\nLoot and banking");
{
  const tiers = loot.tiers;
  const lanternOnly = Object.entries(tiers).filter(([, t]) => t.visibility === "lanternOnly");

  // The load-bearing rule: if light revealed everything, darkness would stop
  // being a viable decision.
  check("baseline loot is always visible", loot.rules.baselineLootAlwaysVisible === true);
  check("exactly one tier is lantern-only", lanternOnly.length === 1, `got ${lanternOnly.length}`);
  check("that tier is Relics", lanternOnly[0]?.[0] === "Relics");
  check("Supplies are always visible", tiers.Supplies.visibility === "always");
  check("Components are always visible", tiers.Components.visibility === "always");
  check("crates are not pocketable", loot.rules.restorationCratesArePocketable === false);
  check("crates block combat", tiers.RestorationCrates.blocksCombatWhileCarried === true);

  // The extraction promise. Never sell a way around this.
  check("banked loot is lost without extraction", loot.banking.bankedKeptOnWipe === 0);
  check("carried loot is lost without extraction", loot.banking.unbankedKeptOnWipe === 0);
  check("disconnect uses the wipe rule", loot.banking.disconnectUsesSameRule === true);

  const dock = loot.extraction.LoadingDock;
  const lift = loot.extraction.FreightLift;
  check("the deeper exit pays more", lift.payoutModifier > dock.payoutModifier);
  check("the dock requires a 20-second hold", dock.arrivalSeconds === 20);
  check("the freight lift takes 30 seconds", lift.arrivalSeconds === 30);
  check("rooftop is not an exit", !("Rooftop" in loot.extraction));
}

console.log("\nEnemies");
{
  check("the Warden is unkillable", enemies.Warden.killable === false);
  check("the Warden drops nothing", enemies.Warden.dropsLoot === false);

  // Locked 2026-10-19: 2s scan, 20s tag, no meter tax, no permanent waypoint.
  check("Glimmer scan is 2s", enemies.Glimmers.scanSeconds === 2);
  check("Glimmer tag is 20s", enemies.Glimmers.tagSeconds === 20);
  check(
    "Glimmer scan stays local",
    enemies.Glimmers.scanRange > 0 && enemies.Glimmers.scanRange <= 35,
    `got ${enemies.Glimmers.scanRange} studs; the scan should pressure one court, not the whole mall`,
  );
  check("Glimmer tag adds no Storm Charge", enemies.Glimmers.tagAddsStormCharge === 0);
  check("Glimmer tag gives no Warden waypoint", enemies.Glimmers.tagGivesWardenPermanentWaypoint === false);
  check("Glimmer tag reveals to all robots", enemies.Glimmers.tagRevealsPositionToAllRobots === true);

  check("Window Walker wakes at a local range", enemies.WindowWalkers.wakeRange > 0 && enemies.WindowWalkers.wakeRange <= 40);
  check("Window Walker walks slower than a Sweeper", enemies.WindowWalkers.moveSpeed < enemies.Sweepers.moveSpeed);
  check("Window Walker needs lantern light to wake", enemies.WindowWalkers.wakeRequiresActiveLantern === true);

  // Light attracts danger. No small-enemy repulsion exception in v1.
  check("light makes Sweepers more aware", enemies.Sweepers.aggroRadiusLit > enemies.Sweepers.aggroRadiusDark);
  check(
    "the Alert sweep radius is larger than dark detection",
    enemies.Sweepers.swarmCallRadius > enemies.Sweepers.aggroRadiusDark,
    "PatrolsAlert needs to change dark-run pressure rather than be a no-op",
  );

  for (const [name, def] of Object.entries(enemies)) {
    if (name.startsWith("_")) continue;
    check(`${name} telegraphs with sound`, def.audioTelegraphSeconds > 0);
  }
}

console.log("\nThe data contract (DATA-PIPELINE.md)");
{
  // Added 2026-10-28 after hand-auditing every top-level key against the
  // contract. The audit found three keys with no home in the spec, which rule 2
  // turns into a hard build error rather than a warning - so a stale contract
  // breaks the build, and that is worth a test rather than a periodic reread.
  const workshop = load("workshop.json");
  const tierNames = Object.keys(loot.tiers);

  // workshop.currency parses as an enum of loot tiers, not free text.
  check(
    "every workshop currency is a real loot tier",
    workshop.currency.every((c) => tierNames.includes(c)),
    `${workshop.currency.filter((c) => !tierNames.includes(c))} not in ${tierNames}`,
  );

  // FLLKEnemyDef is heterogeneous. The contract's decision: five universal
  // fields required, family fields TOptional, and the importer validates by
  // row name. These assert the shape that decision assumes, so the data cannot
  // drift away from the spec without someone noticing.
  const UNIVERSAL = ["formerJob", "behaviour", "counterplay", "moveSpeed", "audioTelegraphSeconds"];
  const FAMILY = {
    Sweepers: ["aggroRadiusDark", "aggroRadiusLit", "swarmCallRadius", "attractedToDroppedLoot"],
    Stockers: ["sightConeDegrees", "losesTargetAfterSeconds"],
    Glimmers: [
      "scanRange", "scanSeconds", "scanRequiresActiveLantern", "tagSeconds",
      "tagRevealsPositionToAllRobots", "tagAddsStormCharge", "tagGivesWardenPermanentWaypoint",
    ],
    Warden: ["killable", "dropsLoot", "spawnStage", "retargetIntervalSeconds"],
  };
  const roster = Object.keys(enemies).filter((k) => !k.startsWith("_"));

  check("the roster is the six of DECISIONS row 13", roster.length === 6, roster.join(", "));
  for (const name of roster) {
    check(
      `${name} carries all five universal fields`,
      UNIVERSAL.every((f) => enemies[name][f] !== undefined),
      UNIVERSAL.filter((f) => enemies[name][f] === undefined).join(", "),
    );
  }
  // A family field must appear ONLY on its family, or the "validate by row
  // name" guarantee that replaces rule 3 stops holding.
  for (const [owner, fields] of Object.entries(FAMILY)) {
    for (const f of fields) {
      const holders = roster.filter((n) => enemies[n][f] !== undefined);
      check(
        `${f} belongs to ${owner} alone`,
        holders.length === 1 && holders[0] === owner,
        `found on ${holders.join(", ") || "nobody"}`,
      );
    }
  }
  // The Warden's missing health is a rule, not an omission (row 7). If this
  // ever fails, either the Warden became killable or someone let the importer
  // default it - and defaulting it to 0 inverts row 7 rather than breaking it.
  check("the Warden has no health value at all", enemies.Warden.health === undefined);
  check("the Warden is explicitly unkillable", enemies.Warden.killable === false);
  check(
    "every other enemy does have health",
    roster.filter((n) => n !== "Warden").every((n) => typeof enemies[n].health === "number"),
  );
}

console.log("\nThe underscore contract (DATA-PIPELINE.md rule 1)");
{
  // Chris, 2026-10-28: "the Unreal importer should explicitly strip or reject
  // underscore-prefixed analysis fields, not merely happen to ignore them...
  // _dutyProfiles belongs to tests and tuning reports; runtime should only
  // receive actual gameplay values."
  //
  // The engine side is the importer's job and cannot be tested here. What CAN
  // be tested here is the thing that would silently defeat it: an analysis
  // field losing its underscore and becoming indistinguishable from a gameplay
  // value. These guard the boundary from the data side.
  const files = [
    "storm-charge.json",
    "loot.json",
    "enemies.json",
    "lantern.json",
    "ledger.json",
    "relics.json",
    "town.json",
    "workshop.json",
  ];

  // Analysis-only structures that must NEVER appear without their underscore.
  // Add to this list whenever a new tuning-report structure is introduced.
  const analysisOnly = ["dutyProfiles"];

  let underscoreTotal = 0;
  const shadowed = [];
  const promoted = [];

  const walk = (node, file) => {
    if (Array.isArray(node)) return node.forEach((n) => walk(n, file));
    if (node === null || typeof node !== "object") return;
    const keys = Object.keys(node);
    for (const k of keys) {
      if (k.startsWith("_")) {
        underscoreTotal += 1;
        // `x` and `_x` in one object is how a typo turns documentation into
        // configuration, or the reverse.
        if (keys.includes(k.slice(1))) shadowed.push(`${file}: ${k} shadows ${k.slice(1)}`);
      } else if (analysisOnly.includes(k)) {
        promoted.push(`${file}: ${k} lost its underscore`);
      }
      walk(node[k], file);
    }
  };
  for (const f of files) walk(load(f), f);

  check(
    "no analysis-only structure has been promoted to a runtime key",
    promoted.length === 0,
    promoted.join("; "),
  );
  check("no object carries both `x` and `_x`", shadowed.length === 0, shadowed.join("; "));
  check("_dutyProfiles is underscore-prefixed, so the importer strips it", storm._dutyProfiles !== undefined);
  check("the duty profiles carry no runtime-looking twin", storm.dutyProfiles === undefined);
  // Rule 1b asks the importer to REPORT what it stripped. Print ours for the
  // same reason: a silent strip cannot be audited, and if this count ever drops
  // sharply, documentation was deleted rather than ignored.
  console.log(`  ..   ${underscoreTotal} underscore keys across ${files.length} files (importer must strip, and say so)`);
}

console.log("\nLantern, knockdown, revive");
{
  // Chris's call: the lantern never runs out, so the cost of light is the Storm
  // Charge it pours into the meter and nothing else. A fuel gauge made running
  // dry a second failure state on top of the storm. These lock the new rule -
  // either light is unlimited, or it lasts long enough to be worth carrying.
  const runMax = storm.run.hardCeilingSeconds; // was hardcoded 10 * 60 - one of O1's four disagreeing sources
  const chargeSeconds = lantern.charge.secondsOfLightPerFullCharge;
  check("light is either unlimited or meaningfully long", chargeSeconds === 0 || chargeSeconds >= 60);
  if (chargeSeconds > 0) {
    check("a finite charge cannot cover a whole run", chargeSeconds < runMax);
  }
  // Whatever the fuel rule, light must still cost something, or darkness stops
  // being a decision. That cost is the meter.
  check("light still costs Storm Charge", storm.lantern.perLanternBonus > 0);
  check("a downed player cannot walk", lantern.knockdown.downedWalkSpeed === 0);
  check("reviving takes real time", lantern.revive.teammateHoldSeconds > 0);
  check("reviving needs proximity", lantern.revive.maxReviveDistance > 0);

  // One Sweeper survivable, a swarm not.
  check("one Sweeper hit is not a knockdown", lantern.sweeperContact.hitsToKnockDown >= 2);
  check("contact decays", lantern.sweeperContact.hitDecaySeconds > 0);
  check(
    "decay is slower than the hit cooldown",
    lantern.sweeperContact.hitDecaySeconds > lantern.sweeperContact.contactCooldownSeconds,
    "otherwise a lone Sweeper can never stack a knockdown",
  );
  check("sprint is faster than walking", lantern.movement.sprintSpeed > lantern.movement.walkSpeed);
  check("crouch is slower than walking", lantern.movement.crouchSpeed < lantern.movement.walkSpeed);
  check("valuable loot takes longer to gather", loot.tiers.Relics.pickupSeconds > loot.tiers.Supplies.pickupSeconds);
  check("restoration crates take longest to gather", loot.tiers.RestorationCrates.pickupSeconds > loot.tiers.Relics.pickupSeconds);
  check("relics are heavier than supplies", loot.tiers.Relics.burden > loot.tiers.Supplies.burden);
}

console.log("\nLantern multiplier curve");
{
  const lamp = storm.lantern;
  const mult = (n, curve) => {
    if (n <= 0) return 1;
    if (curve === "diminishing") {
      const d = lamp.diminishingDecay;
      const series = d === 1 ? n : (1 - d ** n) / (1 - d);
      return 1 + lamp.perLanternBonus * series;
    }
    return Math.min(1 + lamp.perLanternBonus * n, lamp.maxMultiplier);
  };

  check("curve is a documented option", lamp.curve in lamp._curveOptions, `got "${lamp.curve}"`);
  check("decay is a proper fraction", lamp.diminishingDecay > 0 && lamp.diminishingDecay < 1);

  // The 2.5x one-lantern anchor is locked. Neither curve may move it.
  for (const curve of Object.keys(lamp._curveOptions)) {
    check(`${curve} keeps one lantern at 2.5x`, near(mult(1, curve), 2.5, 0.05), `got ${mult(1, curve).toFixed(2)}x`);
    check(`${curve} is monotonic in lantern count`, [1, 2, 3, 4].every((n) => mult(n, curve) >= mult(n - 1, curve)));
  }

  // The whole reason the diminishing curve exists.
  const marginal = [1, 2, 3, 4].map((n) => mult(n, "diminishing") - mult(n - 1, "diminishing"));
  check("no lantern is free under diminishing", marginal.every((x) => x > 0));
  check(
    "diminishing self-limits below the clamp",
    1 + lamp.perLanternBonus / (1 - lamp.diminishingDecay) <= lamp.maxMultiplier,
  );

  // Chris's call, 2026-10-27: "Switch storm-charge.json to the diminishing
  // curve so no keeper's light is free." The assertions above prove the
  // diminishing SHAPE has that property; this one proves the shape actually in
  // use does. Without it, flipping `curve` back to linearCapped would make a
  // fourth keeper's lantern cost the crew nothing and no test would notice.
  const liveMarginal = [1, 2, 3, 4].map((n) => mult(n, lamp.curve) - mult(n - 1, lamp.curve));
  check(
    "no keeper's light is free under the active curve",
    liveMarginal.every((x) => x > 0),
    `${lamp.curve} marginals ${liveMarginal.map((x) => x.toFixed(2)).join(" / ")}`,
  );
  check("the active curve is recorded, not inferred", typeof lamp._curveChoice === "string");
}

console.log("\nSolo rule");
{
  const anchor = storm.anchorRevive;
  // "Solo players receive one Anchor Revive per run" is a resolved decision.
  // Granting it to a crew silently breaks the whole-crew-down wipe.
  check("Anchor Revive is solo-only", anchor.soloOnly === true);
  check("solo gets exactly one", anchor.chargesPerRun === 1);
  check("it costs a full charge", anchor.consumesFullLanternCharge === true);
}

console.log("\nThe ledger and the rooms");
{
  const ledger = load("ledger.json");
  const rungIds = new Set(ledger.rungs.map((r) => r.id));

  check(
    "every rung states its trade",
    ledger.rungs.every((r) => r.title && r.job && r.opens && r.wakes),
    "docs/QUESTS.md rule 2: the trade is stated before the player accepts it",
  );
  check("rung ids are unique", rungIds.size === ledger.rungs.length);

  const roomIds = new Set(ledger.rooms.map((r) => r.id));
  check("room ids are unique", roomIds.size === ledger.rooms.length);
  check(
    "every room names its sealed and restored state",
    ledger.rooms.every((r) => r.room && r.sealed && r.restored),
  );
  check(
    "every room costs at least one crate",
    ledger.rooms.every((r) => Number.isInteger(r.crates) && r.crates > 0),
    "a free room is not a decision, and the hand-in that pays for it has nothing to feel",
  );
  check(
    "every room points at a real rung",
    ledger.rooms.every((r) => rungIds.has(r.rung)),
    "a room with no job behind it is a door that opens for no reason",
  );

  // The first room has to be reachable on a first run, or the first session
  // ends without the one beat docs/FIRST-SIXTY-SECONDS.md is built around.
  const cheapest = Math.min(...ledger.rooms.map((r) => r.crates));
  check("one room opens on a single crate", cheapest === 1, `cheapest is ${cheapest}`);

  // Rooms are paid for out of credited crates and never subtract from them, so
  // the landmark plates keep reading the gross figure. If the rooms cost more in
  // total than the last landmark, a keeper finishes the whole town with rooms
  // still sealed, which reads as a bug rather than as a choice.
  const roomTotal = ledger.rooms.reduce((n, r) => n + r.crates, 0);
  const lastLandmark = Math.max(...load("town.json").landmarks.map((l) => l.crates));
  check(
    "the rooms cost less than the whole town",
    roomTotal <= lastLandmark,
    `rooms ${roomTotal}, last landmark ${lastLandmark}`,
  );
}

console.log("\nRung one, the departure generator");
{
  const ledger = load("ledger.json");
  const loot = load("loot.json");
  const built = ledger.rungs.filter((r) => r.restoration);

  check(
    "exactly one rung is built",
    built.length === 1 && built[0].id === "DepartureGenerator",
    `built: ${built.map((r) => r.id).join(", ") || "none"}`,
  );

  const r = built[0].restoration;

  // The room is the home half of the same job: crates open it, salvage makes
  // the thing inside it run. A rung pointing at a room that does not exist
  // would be unreachable rather than hard.
  check(
    "the rung's room is a real room",
    ledger.rooms.some((room) => room.id === r.requiresRoom),
    `wants ${r.requiresRoom}`,
  );
  check(
    "the room points back at the rung",
    ledger.rooms.some((room) => room.id === r.requiresRoom && room.rung === built[0].id),
    "a rung and its room must agree, or one of them is for something else",
  );

  // Salvage is spent out of `materials`, so every tier named has to be a tier
  // the game actually pays out.
  const tiers = Object.keys(loot.tiers);
  const asked = Object.keys(r.salvage);
  check("the salvage costs something", asked.length > 0);
  check(
    "every salvage tier is a real loot tier",
    asked.every((tier) => tiers.includes(tier)),
    `asked ${asked.join(", ")}; tiers ${tiers.join(", ")}`,
  );
  check(
    "salvage amounts are whole and positive",
    asked.every((tier) => Number.isInteger(r.salvage[tier]) && r.salvage[tier] > 0),
  );
  // Chris's job line is fuses and wiring, which loot.json files under
  // Components. A rung asking for relics would break REWARDS.md, which keeps
  // relics for cosmetics and landmarks.
  check(
    "rung one does not spend relics",
    !asked.includes("Relics"),
    "docs/REWARDS.md keeps relics out of progression costs",
  );

  // What it opens has to be worth the trip, and what it wakes has to be real.
  const d = r.departure;
  check(
    "restoring the board makes departures faster",
    d.poweredSeconds < d.unpoweredSeconds,
    `${d.unpoweredSeconds}s unpowered, ${d.poweredSeconds}s powered`,
  );
  check(
    "an unpowered departure can actually fail",
    d.unpoweredFalseStartChance > 0 && d.unpoweredFalseStartChance < 1,
    `got ${d.unpoweredFalseStartChance} - at 0 "reliable departures" is not a reward, at 1 the game cannot start`,
  );
  check(
    "a powered departure is still a wait",
    d.poweredSeconds > 0,
    "the countdown is what lets a crew see the road open",
  );

  // The complication. Every enemy it names has to exist and has to be one that
  // waking to light means anything for.
  const enemies = load("enemies.json");
  const woken = r.wakesWithoutLantern;
  check("the rung wakes something", Array.isArray(woken) && woken.length > 0);
  check(
    "every woken enemy is in the roster",
    woken.every((name) => enemies[name]),
    `names ${woken.join(", ")}`,
  );
  check(
    "every woken enemy woke to lantern light before",
    woken.every((name) => enemies[name].wakeRequiresActiveLantern === true),
    "powered light only changes an enemy that was waiting for light; naming any other is a no-op",
  );
  check(
    "the rung's own prose says it wakes them",
    /window walker/i.test(built[0].wakes),
    `wakes: "${built[0].wakes}"`,
  );
}
// ---- the lantern's reach, against the range it is noticed from -----------
//
// Added 2026-10-29, when Chris moved the light's settings out of C++ and into
// data/lantern.json: "the actual reach must be portable, tunable, and shared
// by light visibility/enemy logic later."
//
// This is the check that moving them was FOR. While the radius was a constant
// in ULLKLanternLightComponent and the aggro ranges were in enemies.json,
// nothing could compare them - and they are the two halves of one decision.
// Row 5 makes light reveal value and attract danger, and whether that trade is
// real depends entirely on which of these two numbers is larger.
{
  const light = lantern.light ?? {};
  const sweeper = enemies.Sweepers ?? {};

  check(
    "the lantern has a light block at all",
    typeof light.radiusStuds === "number",
    "data/lantern.json.light is where the reach lives since 2026-10-29",
  );

  check(
    "a lit keeper is noticed from beyond the edge of their own light",
    light.radiusStuds < sweeper.aggroRadiusLit,
    `radius ${light.radiusStuds} studs vs aggroRadiusLit ${sweeper.aggroRadiusLit}: at or ` +
      `above it, a keeper sees patrols before the patrols react and light stops costing ` +
      `anything. Row 5 needs the danger to arrive out of the dark you lit yourself into.`,
  );

  check(
    "but turning the lantern on still widens the ring around you",
    sweeper.aggroRadiusLit > sweeper.aggroRadiusDark,
    `lit ${sweeper.aggroRadiusLit} must exceed dark ${sweeper.aggroRadiusDark}`,
  );

  check(
    "and light still reaches further than a dark keeper is noticed from",
    light.radiusStuds > sweeper.aggroRadiusDark,
    `radius ${light.radiusStuds} vs aggroRadiusDark ${sweeper.aggroRadiusDark}: below it, ` +
      `lighting the lantern would never show you anything you had not already been detected ` +
      `by, so it would be pure cost and no player would use it`,
  );

  check(
    "the cone is wide enough to be a lantern rather than a torch",
    light.coneAngleDegrees >= 45 && light.coneAngleDegrees <= 150,
    `${light.coneAngleDegrees} degrees: narrow enough to aim makes this a flashlight game, ` +
      `and row 43 needs the store legible enough to teach`,
  );

  check(
    "the colour is linear 0-1 on three channels with no alpha",
    ["r", "g", "b"].every(
      (c) =>
        typeof light.warmColor?.[c] === "number" &&
        light.warmColor[c] >= 0 &&
        light.warmColor[c] <= 1,
    ) && Object.keys(light.warmColor ?? {}).length === 3,
    "a light has no opacity, so an authored alpha would be a field that does nothing",
  );

  check(
    "the lantern reads warm against the storm",
    light.warmColor.r > light.warmColor.g && light.warmColor.g > light.warmColor.b,
    `r ${light.warmColor.r} > g ${light.warmColor.g} > b ${light.warmColor.b}: the keeper's ` +
      `lantern is the one human light in the building`,
  );

  check(
    "the lantern is bright enough to actually see",
    light.intensity >= 50,
    `${light.intensity} candelas. This check exists because 6 shipped once: the component ` +
      `pins the unit to Candelas, but ULocalLightComponent's own default is Unitless with ` +
      `Intensity 5000, so a small number here is indistinguishable from a lantern that never ` +
      `turned on. Chris found it by eye, which is the expensive way.`,
  );

  check(
    "the lantern casts shadows",
    light.castsShadows === true,
    "revealed geometry with no shadow reads as flat, and looting is a judgement about shape",
  );
}


console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  process.exit(1);
}
console.log("");
