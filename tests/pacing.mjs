// Run-pacing simulation. A report, not a test.
//
// It existed to interrogate one number: maxMultiplier in storm-charge.json.
// Chris retired that number on 2026-10-27 by switching the curve to
// diminishing, which self-limits and needs no clamp. The harness still runs
// both shapes side by side at the bottom, which is the evidence he decided on.
// I picked 5.0 without evidence and flagged it as the most consequential
// untested value in the project. This turns that guess into something Chris can
// read and decide on before a playtest, rather than after.
//
// It simulates Storm Charge against real lantern-charge drain, which matters:
// a crew cannot actually hold a 100% duty cycle, because a full charge only
// buys 150 seconds of light. Lanterns die and the multiplier falls back on its
// own. That feedback loop is invisible if you only read the multiplier.
//
//   node tests/pacing.mjs

import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const load = (n) => JSON.parse(readFileSync(join(root, "data", n), "utf8"));

const storm = load("storm-charge.json");
const lanternCfg = load("lantern.json");

const DT = 0.1;
const CHARGE_SECONDS = lanternCfg.charge.secondsOfLightPerFullCharge;
// Mirrors Config.lanternDrainPerSecond: 0 seconds means the lantern never runs
// out, not that it empties instantly. Getting this wrong made every crew and
// every duty cycle produce an identical row, because each lantern died on the
// first tick and nobody ever accelerated the storm.
const DRAIN = CHARGE_SECONDS > 0 ? 1 / CHARGE_SECONDS : 0;
// Read the real run shape out of data/storm-charge.json. A number that lives in
// two places is a number that lies in one of them - this report's headline
// conclusion once inverted silently because the value was hardcoded at 10
// minutes while Chris had moved the ceiling to 20.
//
// MOVED 2026-10-28: this used to regex targetMaxSeconds out of
// src/shared/Config.luau, which is frozen Luau (DECISIONS row 33) and does not
// port to Unreal. It also carried a 10-minute fallback, so once src/ goes the
// simulation would have quietly resumed reporting against a stale number
// instead of failing. Run length now lives in the portable layer under names
// that distinguish a ceiling from a target, and there is deliberately no
// fallback: if the key is missing this should stop rather than guess.
if (!storm.run?.hardCeilingSeconds || !storm.run?.targetSeconds) {
  throw new Error("data/storm-charge.json run block is missing hardCeilingSeconds or targetSeconds");
}
const RUN_MAX = storm.run.hardCeilingSeconds;
const RUN_TARGET = storm.run.targetSeconds;
// RUN_MIN is the BOTTOM OF THE TARGET BAND, not the early exit. The old code
// read a "targetMinSeconds" with a 6-minute fallback, which was right while the
// target was 6-10 but is wrong now: Chris made six minutes the earliest smart
// extraction call (run.earlyExitSeconds) and the target band 600-720. Using 360
// here would make "the Warden arrived before a short run ended" fire against a
// voluntary early exit rather than against a real run.
const [RUN_MIN] = storm.run.targetBandSeconds;
const [WARDEN_LOW, WARDEN_HIGH] = storm.warden.arrivalBandSeconds;

// Mirrors Config.lanternMultiplier. Kept in sync by a balance test.
function multiplier(n, curve = storm.lantern.curve) {
  if (n <= 0) return 1;
  const { perLanternBonus: bonus, maxMultiplier: cap, diminishingDecay: decay } = storm.lantern;
  if (curve === "diminishing") {
    const series = decay === 1 ? n : (1 - decay ** n) / (1 - decay);
    return 1 + bonus * series;
  }
  return Math.min(1 + bonus * n, cap);
}

function rate(activeLanterns, curve) {
  return storm.baselineRatePerSecond * multiplier(activeLanterns, curve);
}

// Each crew member keeps their lantern lit for `duty` of the time, in 20s
// blocks, and only while they have charge left.
function simulate(crew, duty) {
  const charges = Array(crew).fill(1);
  let t = 0;
  let value = 0;
  let wardenValue = 0; // the Warden's separate, dampened meter - see the loop below
  const hits = {};
  let lanternSecondsBurned = 0;

  while (t < 30 * 60) { // long horizon: report the true time, not "never"
    const block = Math.floor(t / 20);
    const wantLit = duty > 0 && (block % Math.round(1 / duty) === 0);

    let active = 0;
    for (let i = 0; i < crew; i += 1) {
      if (wantLit && charges[i] > 0) {
        active += 1;
        charges[i] = Math.max(charges[i] - DRAIN * DT, 0);
        lanternSecondsBurned += DT;
      }
    }

    value += rate(active) * DT;
    for (const th of storm.thresholds) {
      if (hits[th.id] === undefined && value >= th.value) hits[th.id] = t;
    }

    // The Warden accrues on its OWN dampened meter since 2026-10-28, when Chris
    // split it out of the generic thresholds: "The Warden is an unkillable,
    // late-night escalation - not the normal consequence of using a lantern."
    // wardenMultiplier = 1 + lightSensitivity * (lanternMultiplier - 1), so a
    // lit crew still summons it earlier (light always costs) without a lantern
    // becoming a button that starts a six-minute chase.
    const fullMult = multiplier(active);
    const wardenMult = 1 + storm.warden.lightSensitivity * (fullMult - 1);
    wardenValue += storm.baselineRatePerSecond * wardenMult * DT;
    if (hits.WardenConverges === undefined && wardenValue >= storm.warden.chargeValue) {
      hits.WardenConverges = t;
    }

    if (hits.WardenConverges !== undefined) break;
    t += DT;
  }
  return { hits, lanternSecondsBurned };
}

const mmss = (s) =>
  s === undefined ? "  never" : `${Math.floor(s / 60)}:${String(Math.floor(s % 60)).padStart(2, "0")}`.padStart(7);

console.log(`\nStorm Charge pacing`);
// Under diminishing the clamp never applies, so printing it as "cap" would
// misreport what is holding the multiplier down.
const DIMINISHING = storm.lantern.curve === "diminishing";
const ceilingLabel = DIMINISHING
  ? `self-limits at ${(1 + storm.lantern.perLanternBonus / (1 - storm.lantern.diminishingDecay)).toFixed(2)}x`
  : `cap ${storm.lantern.maxMultiplier}x`;
console.log(`baseline ${storm.baselineRatePerSecond}/s · per-lantern +${storm.lantern.perLanternBonus} · ${storm.lantern.curve}, ${ceilingLabel} · charge ${CHARGE_SECONDS > 0 ? CHARGE_SECONDS + "s" : "unlimited"}\n`);
// Rows are labelled with the NAMED duty profiles from data/storm-charge.json
// _dutyProfiles. Chris, 2026-10-28: without the names "someone later will read
// normally lit crew as 50%, see a seven-minute Warden, and correctly think the
// system drifted again." Only `normal` is held to the Warden band.
const PROFILES = storm._dutyProfiles;
const DUTY_ROWS = [
  { duty: 0, name: "dark" },
  { duty: PROFILES.normal.duty, name: "normal" },
  { duty: PROFILES.bright.duty, name: "bright" },
  { duty: PROFILES.panic.duty, name: "panic" },
];
console.log(`crew  duty  profile   alert    hunt  WARDEN   lantern-sec   verdict`);
console.log(`${"-".repeat(72)}`);

const rows = [];
for (const crew of [1, 2, 3, 4]) {
  for (const { duty, name: profile } of DUTY_ROWS) {
    const { hits, lanternSecondsBurned } = simulate(crew, duty);
    const warden = hits.WardenConverges;
    let verdict;
    if (warden === undefined) verdict = "no Warden";
    // Labels are against the TARGET BAND (600-720s), not against a 6-minute
    // floor - that floor was the old 6-10 target and is now the voluntary early
    // exit instead. "Before the band" means a crew meets the Warden before the
    // intended raid length, which is interesting rather than automatically bad:
    // the Warden is unkillable and converges on the brightest position, so
    // meeting it early means being hunted for the rest of the night.
    // Judged against the WARDEN's own band (storm.warden.arrivalBandSeconds),
    // not the run band. Chris set them separately on purpose: the Warden should
    // land at 9-11 minutes for a normally lit crew, just before the 10-12 minute
    // raid band opens, and LATER than that for a dark crew. Measuring it against
    // the run band made every lit row read as a failure when it was the design.
    else if (profile === "normal")
      verdict =
        warden < WARDEN_LOW
          ? "OFF TARGET - earlier than the band"
          : warden <= WARDEN_HIGH
            ? "on target"
            : "OFF TARGET - later than the band";
    else if (profile === "dark") verdict = "darkness bought the whole night";
    else if (profile === "bright")
      verdict =
        warden > storm.run.earlyExitSeconds
          ? "early by choice, still survivable"
          : "TOO EARLY - invalidates the run";
    else
      verdict = warden > 300 ? "panic state, above the floor" : "BELOW THE MINUTE-FOUR FLOOR";

    rows.push({ crew, duty, warden, verdict });
    console.log(
      `  ${crew}   ${String(Math.round(duty * 100)).padStart(3)}%  ${profile.padEnd(7)} ${mmss(hits.PatrolsAlert)} ${mmss(hits.PatrolsHunt)} ${mmss(warden)}   ${String(Math.round(lanternSecondsBurned)).padStart(5)}s        ${verdict}`,
    );
  }
}

console.log(`\nWhat this says`);

const alwaysLit = rows.filter((r) => r.duty === 1);
const tooFast = alwaysLit.filter((r) => r.warden !== undefined && r.warden < WARDEN_LOW);
if (tooFast.length > 0) {
  console.log(`  - A fully-lit crew meets the Warden before its ${WARDEN_LOW / 60}-minute band opens, at`);
  console.log(
    `    crew size${tooFast.length > 1 ? "s" : ""} ${tooFast.map((r) => r.crew).join(", ")}. ` +
      (DIMINISHING
        ? `Holding every lantern lit for a whole run is`
        : `That is the cap biting.`),
  );
  if (DIMINISHING) {
    console.log(`    meant to be unaffordable; the 25% rows are the ones to read.`);
  }
} else {
  console.log(`  - No fully-lit crew meets the Warden before a 6-minute run ends. The`);
  console.log(`    ${ceilingLabel} ceiling is less punishing than it looks, because lantern charge`);
  console.log(`    runs out first and the multiplier falls back on its own.`);
}

const dark = rows.filter((r) => r.duty === 0);
console.log(`  - Playing fully dark, the Warden arrives at ${mmss(dark[0].warden).trim()} regardless of crew size,`);
console.log(`    which is past the end of even a long run. A dark crew faces no escalation.`);

console.log(
  CHARGE_SECONDS > 0
    ? `  - A full charge buys ${CHARGE_SECONDS}s of light against a ${RUN_MIN / 60}-${RUN_MAX / 60} minute run, so no crew can`
    : `  - Light is unlimited, so nothing but the meter stops a crew staying lit:`
);
console.log(`    stay lit the whole way. Light is a budget, which is what the design wants.\n`);

// ---- curve comparison -----------------------------------------------------
//
// The clamp flattens crew size. This is the evidence for whether that matters.

console.log(`Lantern multiplier by curve`);
console.log(`lanterns   linearCapped   diminishing   marginal cost of the Nth lantern`);
console.log(`${"-".repeat(72)}`);
let prevA = 1;
let prevB = 1;
for (const n of [1, 2, 3, 4]) {
  const a = multiplier(n, "linearCapped");
  const b = multiplier(n, "diminishing");
  const dA = a - prevA;
  const dB = b - prevB;
  const note = dA === 0 ? "  <- free under the clamp" : "";
  console.log(
    `    ${n}          ${a.toFixed(2)}x          ${b.toFixed(2)}x        capped +${dA.toFixed(2)} · diminishing +${dB.toFixed(2)}${note}`,
  );
  prevA = a;
  prevB = b;
}
const asymptote = 1 + storm.lantern.perLanternBonus / (1 - storm.lantern.diminishingDecay);
console.log(`\n  diminishing self-limits at ${asymptote.toFixed(2)}x, so it needs no clamp.`);
console.log(`  Both give exactly ${multiplier(1, "diminishing").toFixed(2)}x for one lantern, so the locked anchor holds either way.`);
console.log(`  Active curve: ${storm.lantern.curve}\n`);
