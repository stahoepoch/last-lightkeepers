// Documentation drift auditor. NOT part of `npm test` — it costs money.
//
// `docs/HANDOFFS.md` exists because three closed findings were still on disk
// reading as live problems. Then the commit that created HANDOFFS.md shipped
// with a claim that was already false when it was written. Both failures are
// the same shape: a true sentence about code that has since changed. No grep
// catches that, because nothing about the sentence is malformed — it is just
// no longer true. That is the one job here a model can do and a regex cannot,
// and it is the only reason this file is allowed to exist.
//
// What it does NOT do:
//
//   - It does not edit a single document. It prints suspicions. A model
//     wrote the stale claim in the first place; letting one rewrite the
//     record would be the same mistake with more confidence.
//   - It does not run in `npm test`. The suite is offline, free and
//     deterministic, and it stays that way.
//   - It does not spend money unless you ask it to. Default is a dry run.
//
// Every finding it prints must carry a verbatim quote and the evidence that
// contradicts it, so you can check it by hand in about ten seconds. A finding
// you cannot check in ten seconds is a finding this tool got wrong.
//
//   node tests/drift-audit.mjs              # dry run: what it would send, and the cost
//   node tests/drift-audit.mjs --run        # actually calls the API
//   node tests/drift-audit.mjs --run --doc docs/FINDINGS-FIXTURE-LIGHT-BAND.md
//
// Needs ANTHROPIC_API_KEY in the environment for --run. Nothing in the game
// or the shipped build ever calls this; it is a desk tool.

import { readFileSync, existsSync, statSync } from "node:fs";
import { execFileSync } from "node:child_process";
import { fileURLToPath } from "node:url";
import { dirname, join, relative } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");

const MODEL = "claude-opus-5-5";
const INPUT_USD_PER_MTOK = 4.0; // claude-opus-5-5, 2026-11
const OUTPUT_USD_PER_MTOK = 20.0;
const MAX_EVIDENCE_CHARS = 60_000; // per evidence file
const MAX_OUTPUT_TOKENS = 16_000;

// The documents whose claims go stale. Findings are written in the present
// tense about a live problem, which is exactly the shape that rots; the
// ledger is the index that is supposed to notice, and didn't.
const DOC_GLOBS = ["docs/FINDINGS-*.md", "docs/HANDOFFS.md"];

const args = process.argv.slice(2);
const live = args.includes("--run");
const onlyDoc = (() => {
  const i = args.indexOf("--doc");
  return i >= 0 ? args[i + 1] : null;
})();

// ---- 1. which documents ----------------------------------------------------

const git = (...a) => execFileSync("git", a, { cwd: root, encoding: "utf8" }).trim();

const docs = onlyDoc
  ? [onlyDoc]
  : DOC_GLOBS.flatMap((g) =>
      git("ls-files", g)
        .split("\n")
        .filter(Boolean),
    );

if (docs.length === 0) {
  console.log("No documents matched. Nothing to audit.");
  process.exit(0);
}

// ---- 2. what each document points at --------------------------------------
//
// A claim can only be checked against something. So for each document we
// collect the things it cites by name — repo paths and commit shas inside
// backticks — and hand the model their CURRENT contents. The model is then
// asked only about claims this evidence actually bears on, which is what
// keeps it from inventing drift out of prose it cannot verify.

// Text extensions only. A `.umap` or `.uasset` is a binary Unreal asset: 60kB
// of its bytes is not evidence about anything, it is noise with a filename.
const PATH_RE = /`([A-Za-z0-9_./\-*]+\.(?:cpp|h|mjs|js|py|json|ini|md|ps1))`/g;
const SHA_RE = /`([0-9a-f]{7,40})`/g;

// These documents cite most files by bare name — `LANES.md`, `LLKCharacter.cpp`
// — because that is how people talk. Resolving them against the git index is
// not a nicety: without it the tool tells the model that nineteen live files
// have been deleted, which is a fabricated contradiction and exactly the kind
// of false positive this whole design is trying to avoid. The first dry run
// did precisely that.
const indexByBasename = (() => {
  const map = new Map();
  for (const p of git("ls-files").split("\n").filter(Boolean)) {
    const base = p.slice(p.lastIndexOf("/") + 1);
    if (!map.has(base)) map.set(base, []);
    map.get(base).push(p);
  }
  return map;
})();

// Returns the repo path, or null when it genuinely is not here. An ambiguous
// bare name resolves to nothing: a guess would be worse than a gap.
const resolveCitation = (cited) => {
  if (existsSync(join(root, cited))) return cited;
  if (cited.includes("/")) return null;
  const hits = indexByBasename.get(cited);
  return hits && hits.length === 1 ? hits[0] : null;
};

const readCapped = (abs) => {
  const text = readFileSync(abs, "utf8");
  if (text.length <= MAX_EVIDENCE_CHARS) return { text, truncated: false };
  return { text: text.slice(0, MAX_EVIDENCE_CHARS), truncated: true };
};

const truncatedFiles = [];

const gatherEvidence = (docText, docRel) => {
  const paths = new Set();
  for (const m of docText.matchAll(PATH_RE)) paths.add(m[1]);

  const files = [];
  const seen = new Set();
  for (const cited of [...paths].sort()) {
    if (cited.includes("*")) continue; // a glob in prose, not a citation
    const p = resolveCitation(cited);
    if (p === null) {
      files.push({ path: cited, missing: true });
      continue;
    }
    if (p === docRel) continue; // the document is not evidence about itself
    if (seen.has(p)) continue; // cited twice, by path and by bare name
    seen.add(p);
    const abs = join(root, p);
    if (!statSync(abs).isFile()) continue;
    // An LFS pointer is not the asset; saying so beats pasting the pointer.
    const { text, truncated } = readCapped(abs);
    if (text.startsWith("version https://git-lfs")) {
      files.push({ path: p, lfs: true });
      continue;
    }
    if (truncated) truncatedFiles.push(`${docRel} -> ${p}`);
    files.push({ path: p, text, truncated });
  }

  const shas = [];
  for (const m of docText.matchAll(SHA_RE)) {
    const sha = m[1];
    if (/^[0-9]+$/.test(sha)) continue; // a number in backticks, not a sha
    try {
      shas.push(git("log", "-1", "--format=%h %ad %s", "--date=short", sha));
    } catch {
      shas.push(`${sha} — NOT IN THIS REPOSITORY`);
    }
  }

  return { files, shas: [...new Set(shas)] };
};

// ---- 3. the question ------------------------------------------------------
//
// Worded to make a false positive expensive and a quiet pass acceptable. The
// failure mode to design against is not a missed finding — it is a confident
// list of ten things that are all fine, which costs an evening to disprove
// and teaches everyone to ignore the tool.

const SYSTEM = `You are auditing one engineering document in a game project against the current state of the code it cites, looking for claims that have gone stale.

A stale claim is a statement the document makes in the present tense about the project that the evidence below contradicts or can no longer support. The archetype: a document says a defect is live, and the code that defect was about has since been fixed.

Rules:

1. Report a claim ONLY when the supplied evidence contradicts it. If checking a claim would need something not in the evidence — a measurement, a reading taken in the editor, a judgment about how the game feels, a number someone has to choose — it is not in scope. Say nothing about it.
2. A document that is honest about its own uncertainty is NOT stale. Many of these documents carry an explicit status line saying what is computed rather than measured, or what is open. Respect it. A caveat the author already wrote is the opposite of drift.
3. Quote verbatim. Every finding carries the document's own words, short, so a human can grep for it.
4. Prefer silence to volume. An empty findings list is the expected and correct result for a document that is up to date. Do not pad. Do not report tidiness, tone, formatting, or things you would have written differently. Do not propose rewrites.
5. You cannot see the running game, the editor, or any rendered image. Never report a claim about visual appearance, feel, or a measured value as contradicted.

Reply with JSON only, no prose around it, in exactly this shape:

{"findings":[{"quote":"the document's own words, verbatim","why_stale":"what in the evidence contradicts it, citing the file or commit by name","confidence":"high|medium|low"}]}

An empty list is {"findings":[]}.`;

const buildUserMessage = (docRel, docText, evidence) => {
  const parts = [`# The document under audit: ${docRel}\n\n${docText}`];

  parts.push(`\n\n# Evidence: current contents of the files this document cites\n`);
  if (evidence.files.length === 0) {
    parts.push("(This document cites no files that exist in the repository.)\n");
  }
  for (const f of evidence.files) {
    if (f.missing) {
      parts.push(
        `\n## ${f.path}\n\nNot found in this repository. It may have been deleted, renamed, or it may live outside the repository (some notes are kept in a shared folder that is not checked in), or the name may be ambiguous. Treat this as unknown, NOT as evidence that the document is wrong.\n`,
      );
    } else if (f.lfs) {
      parts.push(`\n## ${f.path}\n\nA binary asset stored in Git LFS. Contents not available; do not reason about what is inside it.\n`);
    } else {
      parts.push(
        `\n## ${f.path}${f.truncated ? " (TRUNCATED — first " + MAX_EVIDENCE_CHARS + " characters only; do not conclude anything from its absence past that point)" : ""}\n\n\`\`\`\n${f.text}\n\`\`\`\n`,
      );
    }
  }

  if (evidence.shas.length) {
    parts.push(`\n# Evidence: the commits this document names\n\n${evidence.shas.join("\n")}\n`);
  }

  parts.push(
    `\n# Also for reference: the twenty most recent commits\n\n${git("log", "-20", "--format=%h %ad %s", "--date=short")}\n`,
  );

  return parts.join("");
};

// ---- 4. dry run, or spend -------------------------------------------------

const estTokens = (s) => Math.ceil(s.length / 4); // rough; real counts run a little lower

const jobs = docs.map((docRel) => {
  const abs = join(root, docRel);
  if (!existsSync(abs)) {
    console.log(`FAIL  ${docRel} does not exist`);
    process.exit(1);
  }
  const docText = readFileSync(abs, "utf8");
  const evidence = gatherEvidence(docText, docRel);
  const user = buildUserMessage(docRel, docText, evidence);
  return { docRel, user, evidence };
});

const totalIn = jobs.reduce((n, j) => n + estTokens(SYSTEM) + estTokens(j.user), 0);
const estCost = (totalIn / 1e6) * INPUT_USD_PER_MTOK;

console.log(`\nDocumentation drift audit — ${jobs.length} document(s), model ${MODEL}\n`);
for (const j of jobs) {
  const cited = j.evidence.files.filter((f) => !f.missing && !f.lfs).length;
  const missing = j.evidence.files.filter((f) => f.missing).length;
  console.log(
    `  ${j.docRel}\n` +
      `      ${cited} cited file(s) loaded as evidence` +
      (missing ? `, ${missing} citation(s) unresolved` : "") +
      `, ${j.evidence.shas.length} commit(s), ~${estTokens(j.user).toLocaleString()} input tokens`,
  );
}

if (truncatedFiles.length) {
  console.log(
    `\n  NOTE: evidence truncated at ${MAX_EVIDENCE_CHARS.toLocaleString()} chars for:\n` +
      truncatedFiles.map((t) => `      ${t}`).join("\n") +
      `\n  Findings about the unread remainder cannot be trusted.`,
  );
}

console.log(
  `\n  ~${totalIn.toLocaleString()} input tokens total, about $${estCost.toFixed(2)} in` +
    ` (plus output, up to $${((MAX_OUTPUT_TOKENS * jobs.length) / 1e6 * OUTPUT_USD_PER_MTOK).toFixed(2)} at the ceiling).` +
    `\n  Token counts are a 4-chars-per-token estimate, not a measurement.\n`,
);

if (!live) {
  console.log("  Dry run. Nothing was sent and nothing was spent.");
  console.log("  Add --run to call the API.\n");
  process.exit(0);
}

const key = process.env.ANTHROPIC_API_KEY;
if (!key) {
  console.log("  FAIL  --run needs ANTHROPIC_API_KEY in the environment.\n");
  process.exit(1);
}

// Overridable so the parse-and-report half can be exercised against a local
// fake without spending anything. Nothing in normal use sets it.
const ENDPOINT = process.env.LLK_DRIFT_ENDPOINT ?? "https://api.anthropic.com/v1/messages";

const ask = async (job) => {
  const res = await fetch(ENDPOINT, {
    method: "POST",
    headers: {
      "content-type": "application/json",
      "x-api-key": key,
      "anthropic-version": "2023-06-01",
    },
    body: JSON.stringify({
      model: MODEL,
      max_tokens: MAX_OUTPUT_TOKENS,
      system: SYSTEM,
      output_config: { effort: "high" },
      messages: [{ role: "user", content: job.user }],
    }),
  });

  if (!res.ok) {
    throw new Error(`HTTP ${res.status} on ${job.docRel}: ${await res.text()}`);
  }
  const body = await res.json();

  if (body.stop_reason === "refusal") {
    throw new Error(`${job.docRel}: the model declined (${body.stop_details?.category ?? "no category"})`);
  }
  if (body.stop_reason === "max_tokens") {
    throw new Error(`${job.docRel}: hit max_tokens — the reply is truncated and must not be parsed`);
  }

  const text = body.content
    .filter((b) => b.type === "text")
    .map((b) => b.text)
    .join("")
    .trim();

  // Tolerate a fenced block, refuse anything else. A reply we cannot parse is
  // reported as such, never guessed at.
  const json = text.replace(/^```(?:json)?\s*/, "").replace(/\s*```$/, "");
  let parsed;
  try {
    parsed = JSON.parse(json);
  } catch {
    throw new Error(`${job.docRel}: reply was not JSON:\n${text.slice(0, 500)}`);
  }
  if (!Array.isArray(parsed.findings)) {
    throw new Error(`${job.docRel}: reply had no findings array`);
  }
  return { findings: parsed.findings, usage: body.usage };
};

let spentIn = 0;
let spentOut = 0;
let total = 0;
let failed = 0;

for (const job of jobs) {
  process.stdout.write(`  reading ${job.docRel} ... `);
  try {
    const { findings, usage } = await ask(job);
    spentIn += usage.input_tokens ?? 0;
    spentOut += usage.output_tokens ?? 0;
    total += findings.length;
    console.log(findings.length === 0 ? "up to date" : `${findings.length} suspected`);
    for (const f of findings) {
      console.log(`\n      [${(f.confidence ?? "?").toUpperCase()}] ${job.docRel}`);
      console.log(`      says:  "${f.quote}"`);
      console.log(`      but:   ${f.why_stale}`);
    }
    if (findings.length) console.log("");
  } catch (e) {
    failed += 1;
    console.log(`ERROR\n      ${e.message}\n`);
  }
}

const realCost =
  (spentIn / 1e6) * INPUT_USD_PER_MTOK + (spentOut / 1e6) * OUTPUT_USD_PER_MTOK;

console.log(
  `\n${total} suspected stale claim(s) across ${jobs.length} document(s)` +
    (failed ? `, ${failed} document(s) errored` : "") +
    `.\nBilled ${spentIn.toLocaleString()} in / ${spentOut.toLocaleString()} out — $${realCost.toFixed(4)}.\n`,
);

if (total > 0) {
  console.log("These are suspicions, not findings. Check each quote against the");
  console.log("named evidence before you change a word of any document, and close");
  console.log("what is real where the finding lives, per HANDOFFS.md.\n");
}

// A suspicion is not a build failure. This exits 0 unless the tool itself broke.
process.exit(failed > 0 ? 1 : 0);
