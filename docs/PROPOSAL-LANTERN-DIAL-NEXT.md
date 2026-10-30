# Proposal — mounting, and what brightness should do to a Sweeper

Written 2026-10-30 by Claude, from Chris's two follow-ons to the named lantern
levels. **Status: PROPOSED, nothing built.** Both are explicitly sequenced
behind playing the levels, and this file exists so the reasoning survives the
wait rather than being re-derived.

The rule layer is in (`d065469`), the seeds are checked (`fee91de`), and the
data key is Codex's (`docs/HANDOFF-CODEX-LANTERN-LEVELS.md`).

---

## 1. Brightness changes acquisition, not speed

Chris's instruction, 2026-10-30:

> For Sweepers: don't make brighter light make them run faster. Make it
> increase how readily they acquire, converge, and maintain pursuit. Their
> movement stays understandable; your light changes how much of the room you
> have volunteered to them.

**All three knobs already exist as authored numbers**, which is why this is a
coupling rather than a feature:

| What he named | The authored number | Today |
| --- | --- | --- |
| **acquire** | `Sweepers.audioTelegraphSeconds` | 1.0s |
| **converge** | `Sweepers.swarmCallRadius` | 45 studs |
| **maintain pursuit** | `Sweepers.losesTargetAfterSeconds` | 4s |
| *untouched* | `Sweepers.moveSpeed` | 14 studs/s |

`moveSpeed` staying put is the point of the instruction. A Sweeper that moves at
one speed is a Sweeper a player can learn to outrun and then *choose* to risk;
one whose speed tracks your lantern is a Sweeper nobody can form a model of, and
row 16's telegraph rule exists for the same reason - the threat must be
legible before it is dangerous.

### The structural gap, which is one field

`FLLKSweeperSenses` carries `FLLKNotice` and not detectability. Perception
resolves detectability into a radius and then throws the number away, so the
brain currently has no way to ask how lit the keeper it is chasing is. Adding
`float Detectability` to that struct is the whole of the plumbing;
`ALLKSweeper::SenseKeeper` already computes it.

### The consequence worth building it for

**`losesTargetAfterSeconds` should be read continuously, not latched at
acquisition.** That makes hooding down mid-chase a real escape move: a keeper
who kills their light shortens the pursuit that is already happening rather
than only avoiding the next one.

That is counterplay `data/enemies.json` has claimed since the Roblox project -
the Sweeper's own `counterplay` field reads *"Kill lights, move, or drop loot as
a decoy"* - and which the binary system could only express as an on/off. It also
turns the dial from a looting tool into a tool you reach for under pressure,
which is the difference between a setting players configure once and a verb
they use.

### What stays Chris's

The scaling curve. Every number above is authored, so how a detectability of
0.2 maps onto a 4-second pursuit is a value and not a schema - and three
couplings on an input nobody has played yet is exactly the shape of change that
should be seeded rather than derived. The honest default is linear in
detectability between an authored dim and bright pair per knob, with the bright
end being today's number so that Open plays as the game plays now.

---

## 2. Mounting, as its own physical action

Chris, same message:

> For mounting, I'd make it a separate physical action after these levels feel
> right: 0.8–1.2 seconds to mount or unmount. Mounted lantern holds its
> direction/level; no instant hood adjustment. Handheld lets you aim and adjust
> quickly, but occupies a hand and sways. No speed penalty from merely having
> light on.

Recorded rather than designed, because the sequencing is the instruction. Three
notes for whoever picks it up:

**"No speed penalty from merely having light on" is already locked**, as row 77:
*"light still never costs speed"*. Burden costs speed and light does not, and
the mounted/handheld distinction must not become a back door into that - a
handheld lantern that occupies a hand is a cost in what you can carry and do,
never in how fast you walk.

**"Mounted holds its direction and level" is a gameplay fact, so it needs the
gameplay seam, not the cosmetic one.** A mounted lantern's direction is
authoritative - it decides what the field illuminates - while a handheld one
sways, and the sway is already firewalled out of gameplay by
`GetGameplayLightLocation()`. A mount that pins direction is pinning something
`LLKLanternFeel` currently treats as free, so the two have to agree about which
is which before either moves.

**The sway is built and the hand is not.** `FLLKLanternFeelRules` already does
trail, sway and landing kick; there is no carry-slot concept for a hand at all.
So "occupies a hand" is the new mechanic in that paragraph, and it reaches into
`loot.json`'s `carrySlots` rather than into the lantern.

---

## The order, as it stands

1. **Exposure clamp** in the map (Codex). Still first - every brightness value
   is temporary until it lands.
2. **`light.levels`** in the JSON and the importer (Codex), on Chris's seeds.
3. **Replicated level state and a key to cycle it** (Claude). Without this the
   levels are inert data and nobody can feel anything.
4. **Play it.** Three questions, row 79's instrument: does anyone change level
   unprompted, does hooding get used while looting or while escaping, and does
   Open ever get chosen on purpose. `ELLKRunEvent::LightLevelChanged` is in
   place to answer the first two.
5. Then **the Sweeper coupling**, then **mounting**.

Steps 5 and 6 are both this file. Neither should be built before step 4,
because both are tuning on an input whose feel is unmeasured - and the whole
reason row 79 exists is that this project decided to stop answering that kind
of question with opinions.
