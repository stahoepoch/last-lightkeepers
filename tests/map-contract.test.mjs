// Map contracts that are easy to regress during a procedural rebuild.
//
// This does not own gameplay banking logic. It makes the physical map keep
// its side of the deal: a relay is an upstairs detour, is clearly signed, and
// declares the lantern/group contract that the gameplay service enforces.

import { existsSync, readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const path = join(root, "src", "map", "GalleriaBuilder.luau");

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

check("Galleria builder exists", existsSync(path));
const src = existsSync(path) ? readFileSync(path, "utf8") : "";

check(
  "relay is not placed in the center court",
  !src.includes('"RelayBank_Court"'),
  "a center-court relay is on the efficient route and makes banking free insurance",
);
check(
  "relay is on the west balcony",
  src.includes('"RelayBank_WestBalcony"') && src.includes("F2F + 2"),
  "the relay should require an upstairs detour",
);
check(
  "relay declares the lit-lantern requirement",
  src.includes('SetAttribute("RequiresLanternLit", true)'),
  "gameplay must require light for the whole transfer",
);
check(
  "relay declares the group banking requirement",
  src.includes('SetAttribute("BanksCrewInRange", true)'),
  "a relay must bank the crew in range together, not reward a solo runner",
);
check(
  "relay remains tagged for the interaction system",
  src.includes('AddTag(relay, "RelayBank")'),
);
check(
  "relay announces its light cost in-world",
  src.includes("RELAY ANNEX  •  LANTERNS REQUIRED"),
  "the player should see the commitment before using the relay",
);

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) process.exitCode = 1;
