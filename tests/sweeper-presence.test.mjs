// A motor that compiles, loads nothing, and plays silently.
//
// `LLKSweeperPresenceTest` proves the balance: every state audible but the
// telegraph, the glide frame-rate independent, pursue louder than patrol. All
// of it is true of a Sweeper that makes no sound at all, because a C++ test
// cannot see whether the asset the component asks for is on disk, whether
// anything ever calls `Play()`, or whether `UpdateVoice` is reached from a
// tick. Those are exactly the three ways this project's recurring defect
// arrives: a correct system over an absent thing, with a clean log.
//
// It has arrived four times already — a crate in a recess, fixture collision
// never imported, a lantern 189 cm above a 150 cm reach, an opening with no
// navmesh — and the pattern each time was that nothing in the repo could have
// reported it. This file is what reports this one.
//
// Six checks, each named for the way it fails in play:
//
//   1. The master exists.          No script, no re-render in another engine.
//   2. The render exists, and is a real loop rather than a stub.
//   3. The path the C++ asks for is the file that is actually on disk.
//      This is the check. A `FObjectFinder` on a missing asset does not
//      error; `Succeeded()` is false, `PresenceSound` stays null, and the
//      Sweeper is as silent as it was before any of this was written.
//   4. Something calls Play(). A component with bAutoActivate false and no
//      Play() is a configured silence.
//   5. UpdateVoice is reached from Tick, not merely defined — the
//      broadcast-with-no-subscriber shape `cues.test.mjs` was built for.
//   6. The telegraph's volume is still exactly zero in the header, because
//      that single constant is the design claim the whole system rests on
//      and a retune is where it would quietly go.
//
//   node tests/sweeper-presence.test.mjs

import { existsSync, readFileSync, statSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");

let checks = 0;
let failures = 0;

function check(name, ok, why) {
  checks += 1;
  if (ok) {
    console.log(`  ok   ${name}`);
  } else {
    failures += 1;
    console.log(`  FAIL ${name}`);
    if (why) console.log(`       ${why}`);
  }
}

const read = (p) => (existsSync(p) ? readFileSync(p, "utf8") : "");

const SYNTH = join(root, "assets", "audio", "make_sweeper_presence.py");
const WAV = join(root, "assets", "audio", "sweeper-presence-v1.wav");
const CPP = join(root, "Source", "LLK", "Private", "Enemies", "LLKSweeper.cpp");
const HDR = join(root, "Source", "LLK", "Public", "Enemies", "LLKSweeperPresence.h");

console.log("\nThe Sweeper's presence: the asset, the wiring, and the one constant\n");

// 1. The master.
check(
  "the synthesis script is in the repo",
  existsSync(SYNTH),
  "assets/audio/make_sweeper_presence.py is the master; the .wav is one " +
    "render of it. docs/ASSETS.md requires we own what we ship, and a wav " +
    "with no script behind it cannot be re-rendered or defended",
);

// 2. The render, and that it is a loop rather than a placeholder.
const wavOk = existsSync(WAV);
check(
  "and it has been rendered",
  wavOk,
  "run `python assets/audio/make_sweeper_presence.py`",
);
if (wavOk) {
  // 44.1 kHz, 16-bit mono: 88200 bytes a second. A real loop is seconds long.
  const seconds = (statSync(WAV).size - 44) / (44100 * 2);
  check(
    `the render is a usable loop length (${seconds.toFixed(2)}s)`,
    seconds > 1.5 && seconds < 30,
    "under about a second and a half the loop period itself becomes the " +
      "tell rather than the machine; over thirty it is not a loop, it is a " +
      "track, and the file has probably been replaced by something else",
  );
}

// 3. The one that catches the project's recurring defect: does the path the
//    runtime asks for name a file that exists?
const cpp = read(CPP);
const asked = cpp.match(/FObjectFinder<USoundBase>\s+PresenceLoop\(\s*TEXT\("([^"]+)"\)/);
if (!asked) {
  check(
    "the runtime asks for a presence loop by path",
    false,
    "no FObjectFinder<USoundBase> named PresenceLoop in LLKSweeper.cpp, so " +
      "either this system was removed or it was renamed and this guard is " +
      "now passing on nothing",
  );
} else {
  // "/Game/Audio/Sweeper/name.name" -> Content/Audio/Sweeper/name.uasset
  const pkg = asked[1].split(".")[0];
  const rel = pkg.replace(/^\/Game\//, "");
  const onDisk = join(root, "Content", `${rel}.uasset`);
  check(
    `the asset the runtime asks for is on disk (${pkg})`,
    existsSync(onDisk),
    `${pkg} does not exist as Content/${rel}.uasset. FObjectFinder does not ` +
      "error on a missing asset: Succeeded() is false, PresenceSound stays " +
      "null, and every Sweeper is silent while it moves - which is the exact " +
      "defect this component was added to remove, looking identical in logs " +
      "to a working one. Import the wav to that path",
  );
}

// 4. A component that is never played.
check(
  "something actually starts the loop",
  /Presence->Play\(\)/.test(cpp),
  "bAutoActivate is false on purpose, so without a Play() call the " +
    "component is a configured silence - it will pass every balance test " +
    "and make no sound in the game",
);
check(
  "and the sound is handed to it before it plays",
  cpp.indexOf("Presence->SetSound(") >= 0 &&
    cpp.indexOf("Presence->SetSound(") < cpp.indexOf("Presence->Play()"),
  "Play() on a component with no sound set does nothing, quietly",
);

// 5. Defined is not called.
const tickStart = cpp.indexOf("void ALLKSweeper::Tick(");
const tickBody = tickStart >= 0 ? cpp.slice(tickStart, tickStart + 3000) : "";
check(
  "UpdateVoice is reached from Tick, not merely defined",
  /UpdateVoice\s*\(/.test(tickBody),
  "the state mapping only reaches the audio component if something calls " +
    "it every frame; a defined-and-uncalled UpdateVoice is the same shape " +
    "as the delegate nobody bound that cues.test.mjs exists for",
);

// 6. The constant the design rests on.
const hdr = read(HDR);
const telegraph = hdr.match(/TelegraphVolume\s*=\s*([0-9.]+)f/);
check(
  "the telegraph is still exactly silent",
  telegraph !== null && Number(telegraph[1]) === 0,
  telegraph
    ? `TelegraphVolume is ${telegraph[1]}, not 0. The header's argument is ` +
      "that the announcement has to land in a hole in the motor; any " +
      "non-zero value plays the cue over a running machine at the same " +
      "location and the player hears one sound. If this is a deliberate " +
      "retune, change the claim in the header and in this guard together"
    : "could not find TelegraphVolume in LLKSweeperPresence.h, which means " +
      "this guard is passing on nothing",
);

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("A Sweeper can be inaudible in play while every C++ test about its");
  console.log("volume balance passes. That is what these checks are for.\n");
  process.exit(1);
}
console.log("");
