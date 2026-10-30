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

## 3. Three constraints Chris set, each overriding something proposed above

Added 2026-10-30, relayed through the Unreal MCP session. Each of these narrows
or retracts something this thread had already written down, which is why they
are recorded here rather than folded in silently.

### Visual smoothing stays uncoupled from gameplay until it has been felt

> *"A radius boundary visibly travelling inward is compelling, but it can also
> make perception feel arbitrary during the animation. We can test that
> deliberately instead of silently baking it in."*

**This retracts the continuously-animating gameplay radius as a default.** It is
now a hypothesis to test at step 5, not a design decision. Visual falloff may
smooth; the gameplay radius may not, until proven.

**Already true in code, not merely agreed.** `d065469` made
`GetLitLightSources` read `ULLKLanternLightComponent::GetGameplayRadiusCm()`
rather than the component's `AttenuationRadius`, and `SetActiveLevelIndex`
applies a level instantly. So the conservative option is the one that ships by
default, and `LLK.Lantern.Levels` section 5 asserts it by moving the attenuation
radius behind the game's back and requiring the gameplay answer not to follow.
Testing the other hypothesis later means changing one call; nothing has to be
undone first.

### `CarriedCount` must not be the only contact signal

> *"An empty-handed keeper still needs to understand they are in danger."*

Chris accepts that contacts should dismantle a run rather than deplete a hidden
body meter - aim disruption, then a dropped haul, then restraint or downing, so
every hit has a visible consequence. The objection is that the escalation
collapses for exactly the keeper who just extracted or has already lost
everything, so a second signal has to live on the keeper's **own body or
lantern** rather than in inventory.

Two notes for whoever builds it, neither of which is an argument against him:

- **The hidden meter it replaces is deliberate and already justified**, so the
  replacement has to beat its argument rather than just differ from it.
  `data/lantern.json` authors `hitsToKnockDown: 3` with `hitDecaySeconds: 6`,
  and its own note says why: *"hitsToKnockDown with decay is what makes one
  Sweeper survivable and four lethal, without giving players a health bar to
  optimise."* A visible consequence per hit is a better answer to that than
  three invisible ticks - but "no health bar to optimise" is the property to
  preserve.
- **The lantern is the obvious second channel**, and it is already a replicated,
  player-legible, shared-cost object. A contact that disturbs the light itself
  is visible to the keeper, to their crew, and to every other Sweeper through
  the field - which is more information than a body effect would carry, for the
  same hit.

Nothing to build now: the Sweeper coupling is sequenced behind playing the
levels, and so is this.

### Mounted is an information tax, not a stat tax

> *"Avoid 'mounted = slower stats.' The real cost is committing your beam and
> attention to one direction while the dark exists everywhere else."*

Consistent with row 77 and with section 2 above, and it closes off the lazy
implementation. **It does not resolve the collision this file already flagged:**
"mounted holds its direction" makes direction authoritative, while
`LLKLanternFeel` treats direction as cosmetic and free, and those two have to
agree before either moves. That stays open - and it may not be resolved by
giving mounted a movement penalty.

---

## The order, as Chris set it

Relayed 2026-10-30. **Step 1 is not negotiable, and the reason is this thread's
whole subject:**

> *"Clamp first. Absolutely. Do not author or tune three gameplay radii through
> a blown-out white beam; we need a trustworthy visual baseline before the level
> system becomes playable."*

1. **Map exposure clamp, and a real PIE lantern pass** (Codex).
2. **`light.levels` through the importer** (Codex), on Chris's seeds.
3. **Replicated Hooded / Working / Open state and a cycle key** (Claude).
   Explicitly Claude's, explicitly not yet - it waits on 1 and 2.
4. **Play it before building mounting.** Row 79's instrument: does anyone change
   level unprompted, does hooding get used while looting or while escaping, and
   does Open ever get chosen on purpose.
   `ELLKRunEvent::LightLevelChanged` is in place to answer the first two.
5. **Only then** decide whether mount transitions animate the gameplay radius
   continuously or resolve at a clear completion point - and the Sweeper
   coupling, and mounting.

Sections 1 and 2 of this file are both step 5. Neither should be built before
step 4, because both are tuning on an input whose feel is unmeasured - and the
whole reason row 79 exists is that this project decided to stop answering that
kind of question with opinions.

**Nothing here moves `data/lantern.json` or its `_radiusNote`.** Those are
Chris's and he has not moved them.
