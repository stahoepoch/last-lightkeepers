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
  check("WardenConverges is the last threshold", thresholds.at(-1).id === "WardenConverges");

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
  check(
    "dark crew meets the Warden inside a 10-12 minute run",
    thresholds.at(-1).value / base >= 600 && thresholds.at(-1).value / base <= 720,
    `got ${(thresholds.at(-1).value / base).toFixed(0)}s`,
  );
  // The other half of the curve: lantern lit about half the time is 1.75x.
  check(
    "regular lantern use pulls first pressure into minute 2-3",
    thresholds[0].value / (base * 1.75) >= 120 && thresholds[0].value / (base * 1.75) <= 180,
    `got ${(thresholds[0].value / (base * 1.75)).toFixed(0)}s`,
  );
  check(
    "regular lantern use pulls serious pursuit into minute 6-7",
    thresholds.at(-1).value / (base * 1.75) >= 360 && thresholds.at(-1).value / (base * 1.75) <= 420,
    `got ${(thresholds.at(-1).value / (base * 1.75)).toFixed(0)}s`,
  );

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
  const runMax = 10 * 60;
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

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  process.exit(1);
}
console.log("");
