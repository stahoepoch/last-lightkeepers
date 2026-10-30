# Handoff to Codex — rename the clock, because it is no longer a storm meter

**RELEASED BY CHRIS, 2026-10-30.** *"In parallel, hand Codex the dead JSON-key /
importer cleanup from the zone-clock handoff."* Both halves below are yours to
take now - the rename and the two dead keys - and nothing is waiting on Claude.

**One thing changed under this handoff since it was written, and it constrains
the rename.** M3's extraction state machine now reads
`ULLKStormChargeData::ExtractionCall` for two of its three fields -
`SirenSeconds` and `bConvergeAllPatrols` - from
`Source/LLK/Private/Run/LLKExtractionSubsystem.cpp`. So:

- Those two fields must survive the rename. Rename the type around them freely;
  the extraction subsystem is a read-only consumer and will follow.
- `StormChargeSpike` is the one to delete, and deleting it is now guarded on
  Claude's side too: `tests/extraction.test.mjs` fails if anything in the
  extraction code ever reads it, and `FLLKExtractionAnnounce` has no field it
  could be copied into. Row 75.
- `data/loot.json`'s `extraction.*` block is read by the same subsystem for
  `arrivalSeconds`, `payoutModifier` and `requires`. It is not part of this
  rename; it is named here so that nothing in `loot.json` is treated as dead
  while the keys beside the clock's are being removed.

Written 2026-10-29 by Claude. One mechanical rename that crosses a lane, which
is the only reason it is a handoff rather than a commit.

Read `DECISIONS.md` rows 70 and 72 first. Row 72 landed in `a3e5442`; this is
the vocabulary half that did not.

---

## Why it cannot be a drive-by

Row 70: *"the player-facing meter cannot universally be 'Storm Charge'. It
should be a zone pressure clock - Mall: storm arrival. Solar farm: solar
output."* So the class, the asset and the data keys are all misnamed: a meter
called Storm Charge is wrong in half the game that is planned.

The blocker is one dependency. `ULLKStormChargeData` is a `UDataAsset` type
read by **`Source/LLKEditor/Private/LLKImportDataCommandlet.cpp`**, which is
yours (`docs/LANES.md`). Renaming a reflected type without the importer moving
in the same commit breaks `Tools/import_data.py`'s whole output, and the
generated `Content/Data/DA_StormCharge.uasset` is a redirect problem on top.
One writer, one commit.

## What to rename

**Chris chose the names, 2026-10-29**, taking it further than the `Pressure`
I proposed - *"take the rename further than just Pressure where type names
matter"* - because `Zone` is what makes the mechanic honest across zones:

| Now | Chris's choice |
| --- | --- |
| `ULLKStormChargeSubsystem` | `ULLKZonePressureSubsystem` |
| `ULLKStormChargeData` | `ULLKZonePressureData` |
| `FLLKStormSnapshot` | `FLLKZonePressureSnapshot` |
| `/Game/Data/DA_StormCharge` | `/Game/Data/DA_ZonePressure` |
| `data/storm-charge.json` | `data/zone-pressure.json` |

His reasoning, and it is the part to keep: *"'Storm' remains the mall's
authored pressure source and stage fiction; it stops pretending to be the
universal mechanic. Solar Output can later use the same clock without a
semantic lie."* So the word Storm does not disappear - it moves from the type
system into the mall's data, where it is true.

`ELLKStormSpike` is **not** on the list because it no longer exists: row 73
deleted the spike machinery entirely.

**`Charge` as a field name should go too.** Chris: *"We should avoid the word
'charge' for both systems... The lantern has a cell, if it ever needs one."*
`GetCharge()` becomes `GetPressure()`, `Charge` becomes `Pressure`.

**What the player sees is not any of these.** The zone authors its own label -
*Storm Arrival* in the mall, *Solar Output* at a solar farm - so the display
name belongs in data beside the stage names, not in a C++ identifier. That
part is mine and is not in this handoff; the rename just has to stop making it
impossible.

## What to leave alone

- **`data/storm-charge.json`'s values.** Row 55: the schema is Claude's, the
  values are Chris's. Rename keys if you rename the file, change no numbers.
  In particular `warden.arrivalBandSeconds` and `warden.chargeValue` are
  his. He retuned `warden.arrivalBandSeconds` to 660-720 on 2026-10-29 (row
  73) so that the 700-charge arrival at 11:40 sits inside it; do not move
  either number while renaming keys.
- **The retired lantern block.** `lantern.perLanternBonus`,
  `diminishingDecay`, `maxMultiplier` and `_dutyProfiles` are dead as of row
  72 and deliberately still in the file for Chris to delete. The code no
  longer reads them and `tests/balance.test.mjs` has a source guard that fails
  if it ever does again. Do not re-wire them while renaming keys.

- **The behaviour.** Nothing in this rename changes a rule. If a test needs
  its expected values edited, something has moved that should not have.


### Two keys to DELETE, which is the one data change in this handoff

Row 73: *"Remove extractionCall.stormChargeSpike too. Calling extraction
already has the right price: siren, visible activity, and patrol
convergence. A radio call cannot accelerate the weather."*

- `extractionCall.stormChargeSpike` (180) and its `_spikeNote`
- `anchorRevive.stormChargeSpike` (120), dead since row 72

**Both have to go out through you, not me**, and that is the only reason they
are still there. `Tools/import_data.py` validates with an `exact()` key-set
match per block (lines 130-131 and 137-138), so deleting a key from the JSON
without the importer's spec moving in the same commit fails the data contract
immediately - and `Tools/**` is yours. The C++ no longer reads either value,
and `ULLKStormChargeData`'s `StormChargeSpike` fields can come out with them.

`tests/escalation.test.mjs` already guards the end state: it counts the
expressions that add to the meter and fails if there is ever more than one.
## Done when

1. `./Tools/build_editor.ps1` succeeds and `./Tools/run_automation.ps1`
   passes 25 tests, with no test's *expectations* edited - only its
   identifiers.
2. `npm test` fails on exactly the one line it fails on now, the Warden's
   arrival band, and on nothing else.
3. `Tools/import_data.py` regenerates `Content/Data/` with no hand editing and
   is still idempotent (M0.5's bar).
4. `grep -ri "stormcharge" Source/ data/ tests/ docs/` returns only historical
   references in `docs/` - decision rows and commit archaeology, which should
   keep the old name because they are records of what was true then.

## One judgement call that is yours

Whether `Pressure` is the right word. I am proposing it because it is what row
70 called the concept, it is zone-neutral, and it does not collide with
`Charge`. If you prefer something else, say so before the rename rather than
after - this is the kind of change nobody wants to do twice, and the name is
cheaper to argue about than to redo.
