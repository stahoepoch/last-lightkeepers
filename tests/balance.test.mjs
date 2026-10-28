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

  // Chris's Warden arrival rule, 2026-10-28. He set the targets; lightSensitivity
  // was derived to hit them. These lock the shape so a future tuning pass cannot
  // quietly turn the lantern back into a button that starts a six-minute chase.
  {
    const w = storm.warden;
    const decay = lamp.diminishingDecay;
    const mult = (n) =>
      storm.lantern.curve === "diminishing"
        ? 1 + lamp.perLanternBonus * (1 - decay ** n) / (1 - decay)
        : Math.min(1 + lamp.perLanternBonus * n, lamp.maxMultiplier);
    // arrival for `crew` lanterns lit a `duty` fraction of the night
    const wardenAt = (crew, duty) => {
      const full = 1 + (mult(crew) - 1) * duty;
      return w.chargeValue / (base * (1 + w.lightSensitivity * (full - 1)));
    };

    check("the Warden still costs light something", w.lightSensitivity > 0);
    check(
      "the Warden is NOT on the full lantern curve - that is the whole point of the split",
      w.lightSensitivity < 1,
    );
    check(
      "a fully dark crew meets the Warden near the end of the night, not before it",
      wardenAt(4, 0) >= targetLow,
      `got ${wardenAt(4, 0).toFixed(0)}s`,
    );
    // "Warden: the closing act, around minutes 9-11 for a normally lit crew."
    const [wardenLow, wardenHigh] = w.arrivalBandSeconds;

    // PROFILE "normal" (25%) - the baseline. Must land in the band, every crew size.
    for (const crew of [1, 2, 3, 4]) {
      check(
        `[${PROFILES.normal.label}] crew of ${crew} meets the Warden in ${wardenLow}-${wardenHigh}s`,
        wardenAt(crew, PROFILES.normal.duty) >= wardenLow && wardenAt(crew, PROFILES.normal.duty) <= wardenHigh,
        `got ${wardenAt(crew, PROFILES.normal.duty).toFixed(0)}s`,
      );
    }

    // PROFILE "bright" (50%) - MAY be early, must stay survivable. Chris,
    // 2026-10-28: "deliberately bright/risky play; the Warden can arrive
    // earlier, but should remain survivable and not invalidate the run." The
    // testable form of "does not invalidate the run" is that a bright crew
    // still gets at least a complete early-exit raid before the closing act.
    for (const crew of [1, 2, 3, 4]) {
      check(
        `[${PROFILES.bright.label}] crew of ${crew} still gets a full early-exit raid first`,
        wardenAt(crew, PROFILES.bright.duty) > storm.run.earlyExitSeconds,
        `got ${wardenAt(crew, PROFILES.bright.duty).toFixed(0)}s vs early exit ${storm.run.earlyExitSeconds}s`,
      );
    }
    check(
      `[${PROFILES.bright.label}] is allowed to be earlier than the band - light always costs`,
      wardenAt(4, PROFILES.bright.duty) < wardenAt(4, PROFILES.normal.duty),
    );

    // PROFILE "panic" (100%) - not regular play. One rule only: his floor.
    for (const crew of [1, 2, 3, 4]) {
      check(
        `[${PROFILES.panic.label}] crew of ${crew} never meets the Warden at minute four`,
        wardenAt(crew, PROFILES.panic.duty) > 300,
        `got ${wardenAt(crew, PROFILES.panic.duty).toFixed(0)}s`,
      );
    }
    check(
      "a dark crew gets the Warden LATER than a lit one - darkness buys the whole raid",
      wardenAt(4, 0) > wardenHigh,
      `dark ${wardenAt(4, 0).toFixed(0)}s vs band top ${wardenHigh}s`,
    );
    // "Four players using light should reach the Warden somewhat earlier than
    // solo, but never at minute four."
    check(
      "four lit players reach the Warden earlier than a lit solo keeper",
      wardenAt(4, NORMALLY_LIT_DUTY) < wardenAt(1, NORMALLY_LIT_DUTY),
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

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  process.exit(1);
}
console.log("");
