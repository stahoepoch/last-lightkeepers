// Map contracts that are easy to regress during a procedural rebuild.
//
// This does not own gameplay routing logic. It makes the physical map keep
// its side of the deal: the Relay Annex is an upstairs detour with a readable
// active-exit board, rather than a fake banking interaction.

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
  "relay declares an active-route board",
  src.includes('SetAttribute("RouteBoard", true)'),
  "the relay needs a concrete player-facing job",
);
check(
  "relay remains tagged for the interaction system",
  src.includes('AddTag(relay, "RelayBank")'),
);
check(
  "relay announces its route-board role in-world",
  src.includes("RELAY ANNEX  •  ACTIVE ROUTE"),
  "the player should learn what the fixture is before approaching it",
);

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) process.exitCode = 1;
