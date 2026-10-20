// Run-pacing simulation. A report, not a test.
//
// It exists to interrogate one number: maxMultiplier in storm-charge.json.
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
const RUN_MIN = 6 * 60;
const RUN_MAX = 10 * 60;

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
    if (hits.WardenConverges !== undefined) break;
    t += DT;
  }
  return { hits, lanternSecondsBurned };
}

const mmss = (s) =>
  s === undefined ? "  never" : `${Math.floor(s / 60)}:${String(Math.floor(s % 60)).padStart(2, "0")}`.padStart(7);

console.log(`\nStorm Charge pacing`);
console.log(`baseline ${storm.baselineRatePerSecond}/s · per-lantern +${storm.lantern.perLanternBonus} · cap ${storm.lantern.maxMultiplier}x · charge ${CHARGE_SECONDS > 0 ? CHARGE_SECONDS + "s" : "unlimited"}\n`);
console.log(`crew  duty   alert    hunt  WARDEN   lantern-sec   verdict`);
console.log(`${"-".repeat(72)}`);

const rows = [];
for (const crew of [1, 2, 3, 4]) {
  for (const duty of [0, 0.25, 0.5, 1]) {
    const { hits, lanternSecondsBurned } = simulate(crew, duty);
    const warden = hits.WardenConverges;
    let verdict;
    if (warden === undefined) verdict = "no Warden";
    else if (warden < RUN_MIN) verdict = "WARDEN BEFORE A SHORT RUN ENDS";
    else if (warden < RUN_MAX) verdict = "Warden mid-run";
    else verdict = "Warden after a long run";

    rows.push({ crew, duty, warden, verdict });
    console.log(
      `  ${crew}   ${String(Math.round(duty * 100)).padStart(3)}% ${mmss(hits.PatrolsAlert)} ${mmss(hits.PatrolsHunt)} ${mmss(warden)}   ${String(Math.round(lanternSecondsBurned)).padStart(5)}s        ${verdict}`,
    );
  }
}

console.log(`\nWhat this says`);

const alwaysLit = rows.filter((r) => r.duty === 1);
const tooFast = alwaysLit.filter((r) => r.warden !== undefined && r.warden < RUN_MIN);
if (tooFast.length > 0) {
  console.log(`  - A fully-lit crew meets the Warden before a 6-minute run can finish at`);
  console.log(`    crew size${tooFast.length > 1 ? "s" : ""} ${tooFast.map((r) => r.crew).join(", ")}. That is the cap biting.`);
} else {
  console.log(`  - No fully-lit crew meets the Warden before a 6-minute run ends. The`);
  console.log(`    ${storm.lantern.maxMultiplier}x cap is less punishing than it looks, because lantern charge`);
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
