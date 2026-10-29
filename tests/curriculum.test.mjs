// Curriculum guard — row 16 over row 43.
//
// `DECISIONS.md` row 16 is LOCKED: every enemy telegraphs before sight through
// at least two distinguishable channels, one of which may be sound. It is an
// accessibility requirement, not polish. Chris's line is the test: "Voice chat
// must never be the reason someone misses a threat."
//
// `DECISIONS.md` row 43 is LOCKED too: the department store IS the tutorial.
// So the store's curriculum is where those two rules meet, and for a day they
// met badly. The curriculum in docs/THE-STORE.md taught lesson four by taking
// the audio channel AWAY — "sound stops arriving from a direction you can
// use" — which for a deaf or hard-of-hearing player is not a hard floor, it is
// a floor with no lesson on it. Row 16's own note said the rewrite "repairs
// the curriculum", and the repair was an amendment paragraph naming the
// requirement without supplying it.
//
// Prose cannot be unit tested and this file does not pretend otherwise. What
// it can do is make the SHAPE of the fix durable: every lesson in the table
// has a second-channel cell, and none of them is empty. That is the thing
// that silently regressed once and would regress again the next time somebody
// adds a floor in a hurry.
//
// What this CANNOT check, stated plainly so nobody trusts it too far: whether
// a second channel is any good. "Its second channel: a sound" would pass. A
// human reads the column; this stops the column from vanishing.
//
//   node tests/curriculum.test.mjs

import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const store = readFileSync(join(root, "docs", "THE-STORE.md"), "utf8");

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

// ---- find the curriculum table -------------------------------------------
//
// Located by its header row rather than by line number, so inserting prose
// above it does not break the guard.
const lines = store.split("\n");
const headerIndex = lines.findIndex(
  (line) => line.startsWith("| Order |") && line.includes("The one new idea"),
);

check(
  "the curriculum table is still in docs/THE-STORE.md",
  headerIndex !== -1,
  "looked for a row starting `| Order |` containing `The one new idea`",
);

if (headerIndex === -1) {
  console.log(`\n${checks - failures}/${checks} passed`);
  console.log("Cannot continue without the table.\n");
  process.exit(1);
}

const cells = (line) =>
  line
    .split("|")
    .slice(1, -1)
    .map((cell) => cell.trim());

const header = cells(lines[headerIndex]);

// ---- 1. the second-channel column exists ---------------------------------
const channelColumn = header.findIndex((h) => /second channel/i.test(h));
check(
  "the table has a second-channel column",
  channelColumn !== -1,
  `columns are: ${header.join(" / ")}. Row 16 needs every lesson to name a ` +
    `channel that is not the one voice chat occupies`,
);

// ---- 2. every lesson row fills it ----------------------------------------
const rows = [];
for (let i = headerIndex + 2; i < lines.length; i += 1) {
  const line = lines[i];
  if (!line.startsWith("|")) break;
  rows.push(cells(line));
}

check("the curriculum still has lessons in it", rows.length >= 7, `found ${rows.length}`);

if (channelColumn !== -1) {
  for (const row of rows) {
    const order = row[0] ?? "?";
    const floor = row[1] ?? "?";
    const channel = row[channelColumn] ?? "";
    check(
      `lesson ${order} (${floor.replace(/\*/g, "")}) names a second channel`,
      channel.length > 20,
      `row 16 is an accessibility requirement and it is LOCKED; a lesson with ` +
        `no second channel is a lesson a deaf player never receives`,
    );
  }
}

// ---- 3. the specific regression that happened ----------------------------
//
// Named rather than generalised. This exact sentence was the violation, and a
// guard that only checks shape would let it come back inside a filled cell.
// Scoped to the TABLE, not the page. The prose above the table quotes the old
// sentence on purpose - the history of a rule being broken is worth keeping -
// and a guard that scanned the whole file would fail on its own explanation.
// That happened on the first run of this file.
const tableText = rows.map((row) => row.join(" | ")).join(" ;; ");
check(
  "no lesson is taught by taking hearing away",
  !/sound stops arriving from a direction/i.test(tableText),
  `lesson four used to read "sound stops arriving from a direction you can ` +
    `use", which teaches by removing the audio channel - for a deaf player ` +
    `that is a floor with no lesson on it. The Third floor takes SIGHTLINES, ` +
    `which its own art direction already said`,
);

// ---- 4. the rule is still written down, not just obeyed ------------------
//
// A table that happens to comply is one edit from not complying. The reason
// has to survive beside it, or the next person fills the column with anything.
check(
  "the page still states why the column is there",
  /row 16/i.test(store) && /two distinguishable channels|one channel/i.test(store),
  "the curriculum must carry row 16's reasoning, not merely satisfy it",
);

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read DECISIONS.md row 16 and docs/SOUND.md before changing these.\n");
  process.exit(1);
}
console.log("");
