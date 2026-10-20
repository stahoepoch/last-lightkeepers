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

  // The handoff's anchor: 8 minutes to the first threshold in permanent darkness.
  check(
    "first threshold is 8 min in darkness",
    near(thresholds[0].value / base, 480, 2),
    `got ${(thresholds[0].value / base).toFixed(0)}s`,
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

  // The trust promise. Never sell a way around this.
  check("banked loot fully survives a wipe", loot.banking.bankedKeptOnWipe === 1);
  check("unbanked loot is partially lost", loot.banking.unbankedKeptOnWipe > 0 && loot.banking.unbankedKeptOnWipe < 1);
  check("disconnect uses the wipe rule", loot.banking.disconnectUsesSameRule === true);

  const dock = loot.extraction.LoadingDock;
  const lift = loot.extraction.FreightLift;
  check("the deeper exit pays more", lift.payoutModifier > dock.payoutModifier);
  check("the early exit is instant", dock.arrivalSeconds === 0);
  check("the freight lift takes 20s", lift.arrivalSeconds === 20);
  check("rooftop is not an exit", !("Rooftop" in loot.extraction));
}

console.log("\nEnemies");
{
  check("the Warden is unkillable", enemies.Warden.killable === false);
  check("the Warden drops nothing", enemies.Warden.dropsLoot === false);

  // Locked 2026-10-19: 2s scan, 20s tag, no meter tax, no permanent waypoint.
  check("Glimmer scan is 2s", enemies.Glimmers.scanSeconds === 2);
  check("Glimmer tag is 20s", enemies.Glimmers.tagSeconds === 20);
  check("Glimmer tag adds no Storm Charge", enemies.Glimmers.tagAddsStormCharge === 0);
  check("Glimmer tag gives no Warden waypoint", enemies.Glimmers.tagGivesWardenPermanentWaypoint === false);
  check("Glimmer tag reveals to all robots", enemies.Glimmers.tagRevealsPositionToAllRobots === true);

  // Light attracts danger. No small-enemy repulsion exception in v1.
  check("light makes Sweepers more aware", enemies.Sweepers.aggroRadiusLit > enemies.Sweepers.aggroRadiusDark);

  for (const [name, def] of Object.entries(enemies)) {
    if (name.startsWith("_")) continue;
    check(`${name} telegraphs with sound`, def.audioTelegraphSeconds > 0);
  }
}

console.log("\nLantern, knockdown, revive");
{
  const runMax = 10 * 60;
  check("a full charge cannot cover a whole run", lantern.charge.secondsOfLightPerFullCharge < runMax);
  check("charge is long enough to matter", lantern.charge.secondsOfLightPerFullCharge >= 60);
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

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  process.exit(1);
}
console.log("");
