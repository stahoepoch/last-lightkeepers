# Finding — the gameplay light anchor moves 88 cm as the keeper looks around

> **Status: OPEN, corrected 2026-11-04. Handed to Codex in
> `HANDOFF-CODEX-LANTERN-ANCHOR.md`. Nothing was changed here** — this is
> gameplay code and Chris's standing instruction fences this writer out of it.
>
> **This file said something different this morning and that version was
> wrong.** It claimed the gameplay light sits at 240 cm, one capsule
> half-height above the keeper's eyes. It does not. The arithmetic was right
> about the function it read and wrong about which function runs: the path it
> analysed is **unreachable** from `ALLKCharacter`. The live anchor is **133 cm
> standing still**, and it travels **88 cm vertically with look pitch alone**.
> Section "How this was got wrong" has the cause, because the shape of the
> mistake is reusable.

Found while speccing the retail-floor swap in
`FINDINGS-BOSWELLS-READS-AS-STOCKROOM.md`, and corrected while writing the
handoff that would have asked somebody to fix the wrong line.

## Where the gameplay light actually is

`GetGameplayLightLocation()` is the one function four systems judge from —
`LLKZonePressureSubsystem.cpp:345` feeds `Source.Position` from it, so the
storm, the light field, the Warden's convergence and relic legibility all read
it. It opens with an early return:

```cpp
// LLKLanternLightComponent.cpp:57-62
if (const USceneComponent* Anchor = GetAttachParent())
{
    return Anchor->GetComponentLocation();
}
```

`ALLKCharacter` builds the attachment chain in its constructor, so
`GetAttachParent()` is **never null in a running game**:

| Component | Attached to | Relative location | World Z, standing, pitch 0 |
| --- | --- | --- | --- |
| `CapsuleComponent` | the actor | — | **88** (half-height, UE default) |
| `FirstPersonCamera` | the capsule | `(0, 0, 64)` | **152** — the keeper's eye |
| `LanternAnchor` | the camera | `(42, 25, -19)` | **133** |
| `LanternLight` | `LanternAnchor` | feel offset, cosmetic | reports its parent's transform |

`LLKCharacter.cpp:67-71` is explicit that 133 is on purpose: *"A hand lantern
belongs low and to the side of the view. The old pose was almost eye-height and
centred, so the temporary model read as a helmet lamp."* **19 cm below the eye
is a deliberate, defensible number** and nothing below argues with it.

## The part that is not deliberate: a 42 cm lever arm on a rotating parent

`LanternAnchor` sits 42 cm **forward** of a camera with
`bUsePawnControlRotation = true`. Forward rotates with pitch. So the anchor's
world Z is

```
88 + 64 + 42·sin(pitch) − 19·cos(pitch)
```

which is not a constant:

| look pitch | gameplay light, above floor |
| --- | --- |
| +89.9° (straight up, the clamp) | **194.0** |
| +60° | 178.9 |
| +30° | 156.5 |
| +24.3° (crosses eye height) | 152.0 |
| **0° (level)** | **133.0** |
| −30° | 114.5 |
| −65.7° (the minimum) | **105.9** |
| −89.9° (straight down) | 110.0 |

**Travel: 88.1 cm, from 105.9 to 194.0, with the keeper standing perfectly
still.** It is 88 by coincidence and not by the capsule arithmetic this file
originally blamed.

The two comments that govern this both hold, and neither covers it:

- `LanternAnchor`'s own comment — *"Cosmetic sway is applied below this
  component, so a hand-feel pass cannot quietly move a relic-read or enemy-
  perception source."* True. Feel writes `LanternLight`'s relative offset, and
  gameplay reads the parent. The sway is genuinely sealed out.
- Guarantee 1 in the feel header — *"the cone trails the camera and the camera
  trails nothing, so this is an input only."* Also true, about the **cone**.

What neither says is that the **position** gameplay reads is a child of the
camera with a 42 cm moment arm. The sway was sealed out of the gameplay anchor;
the look direction never was, because it arrives through the parent transform
rather than through the feel struct, which is the one input anybody thought to
fence.

**Ruled on, 2026-11-04: it is not a defect and it stays.** Chris, now
`DECISIONS.md` row 80 — *"if a lantern moves and it's supposed to move then it
moves… Everything moves you kind of shift around. That's the name of the game."*
The arguments either way are kept below because they are what the ruling decided
between, and because the third one is now an open question rather than a
complaint — see `FINDINGS-LANTERN-PHYSICS.md` §6.

- It is arguably *right*. A keeper raising a lantern to look at a high shelf
  really does lift the light, and a held lamp really does swing with the arm.
- It is arguably wrong, because the magnitude is a side effect of a pose chosen
  to stop a placeholder mesh reading as a helmet lamp, not of anything about
  how much a lantern should move. Nobody picked 88 cm of travel.
- It makes enemy perception and Warden convergence depend on **where the player
  is looking**, which is an input the player does not know is load-bearing. The
  project's own rule is that light makes the keeper legible; it does not say
  that glancing at the floor makes them 28 cm less so.

What the ruling leaves open is the *other* half: the sway, which is the real
pendulum and the larger real motion, is still fenced out of gameplay while this
rigid lever is inside it. That is the reverse of what physics would choose, and
it is now a question rather than a defect. `FINDINGS-LANTERN-PHYSICS.md` has the
numbers and the three ways to answer it.

## The second thing, which is latent rather than live

The branch this file originally analysed is real and still wrong; it is just
not reached. `LLKLanternLightComponent.cpp:75` falls through to:

```cpp
return LLKLanternFeel::GameplayLightPosition(
    Owner->GetActorLocation(), LLKLanternFeel::KeeperEyeHeightCm);
```

`KeeperEyeHeightCm` is 152 (`LLKLanternFeel.cpp:7`), and its header says the
number is *"derived from two engine defaults"* — which it is: the capsule's 88
plus the camera's 64, exactly the chain in the table above. **152 is therefore
eye height above the floor, and `GetActorLocation()` on a `ACharacter` is the
capsule centre, 88 above the floor.** The 88 is counted twice and the fallback
lands at 240.

Unreachable today, because only `ALLKCharacter` owns one of these components
and it always attaches it. It becomes reachable the moment anything else does
not: a lantern placed in the world, a dropped or docked lantern, a second body
rig, a test fixture that spawns the component bare. The guarded path in between
— `Owner == nullptr` — carries a comment saying that reaching it means *"the
cosmetic position has become the gameplay position and that is the bug this
method exists to prevent."* The 240 branch deserves the same treatment and has
none.

`LLKLanternFeelTest.cpp:92` pins the double-count rather than catching it:

```cpp
const FVector Capsule(1200.f, -400.f, 90.f);
TestEqual(TEXT("the gameplay anchor is the capsule plus the authored eye height"),
    Anchor, FVector(1200.f, -400.f, 90.f + 152.f));
```

The fixture's `Capsule.Z` of 90 is itself a capsule-centre height, so the test
asserts the sum is 242 and would keep passing after any fix that changed only
the call site. **Guarantee 3 is still sound** — it proves no cosmetic value
reaches the anchor, which is a claim about *purity*, not *position*, and holds
either way.

The other caller is **correct**, and that is worth recording because it is the
reference. `LLKMapReadiness.cpp:299` builds its sightline eye as
`KeeperSpawn.Z + EyeHeightCm` from a `PlayerStart` location, and this project
treats a `PlayerStart` Z as the floor surface — `LLKTestGameMode.cpp:195-196`
says so while building the harness floor: *"The engine cube is 100cm… **Its top
surface sits at the harness origin** and the generated PlayerStart goes above
it."* The cube is spawned at `HarnessOrigin - (0,0,25)` with thickness 50, so
its top face lands exactly on the `PlayerStart` Z. So floor + 152 = an eye.
`LLKMapReadiness` measures sightlines from the right place.

## What this does to the retail floor, which is why it was chased

`PROPOSAL-BOSWELLS-RETAIL-FLOOR.md` said this question had to be settled before
the dressing pass, on the grounds that a light at 240 would clear every gondola
while the keeper saw over almost none. **That specific claim is void.** The
corrected numbers are more interesting than the alarming ones:

| Shelf | eye, 152 | light level, 133 | light looking up | light looking down |
| --- | --- | --- | --- | --- |
| 48" / 122 cm | sees over | clears | clears | **blocked** |
| 54" / 137 cm | sees over, just | **blocked** | clears | blocked |
| 60" / 152 cm | eye level | blocked | clears | blocked |
| 72" / 183 cm | blocked | blocked | clears above 72° | blocked |
| 78" / 198 cm | blocked | blocked | **never clears** | blocked |
| racking, 250 cm | blocked | blocked | blocked | blocked |

So on a 137 cm sales floor the keeper can see across the room while their light
cannot — they read the shape of the floor without lighting it, and raising the
lantern to look at the far wall spills light over every shelf between. That is a
legible, teachable trade and it is the kind of thing the dressing pass was for.
**It is an argument for the retail floor rather than a blocker on it**, and the
proposal's prerequisite paragraph should be read as withdrawn.

## Consequence for crouch, which survives the correction

`data/lantern.json` calls crouch *"a deliberate slow mode for tight shops and
careful approach"* and `movement.test.mjs` asserts *"crouch does NOT outrun a
Sweeper, so crouch is concealment and not escape."* Crouch goes through UE's own
`Crouch()`/`UnCrouch()` (`LLKCharacter.cpp:712-729`) with no second source of
truth, so the capsule shrinks, the camera drops with it and the anchor drops
too — correctly, and for free.

Against 250 cm racking a standing keeper is already fully hidden, so **crouch
currently buys no concealment it did not already have.** It costs 43.75% of walk
speed and returns a smaller silhouette nothing is testing against. Low shelving
is what gives crouch its authored job for the first time, which is the second
argument for the retail floor.

## How this was got wrong, because the shape repeats

The morning version of this file read `GetGameplayLightLocation()` from the
bottom, found `GameplayLightPosition(GetActorLocation(), 152)`, did correct
arithmetic on it, and then spent real effort **proving the premise of the wrong
branch** — confirming by elimination that no Blueprint subclass and no
`SetCapsuleSize` call could move the capsule off 88. All of that was true. None
of it mattered, because six lines above there is an `if (GetAttachParent())`
that always takes.

The elimination proof is what made it convincing. It is easy to mistake *"I have
ruled out every way this number could be different"* for *"this number is the
one in play," and the first says nothing about reachability.* `ADVERSARY.md`
already carries "the wrong vintage" for a document that describes a world that
has stopped existing; this is its sibling — **a correct analysis of code that
does not run.** The check that would have caught it costs one grep: before
reasoning about a function's body, find its callers, and read the early returns
above the line you care about.

It also explains the thing that should have been suspicious at the time and was
not: the file argued the bug was invisible because 250 cm racking blocks both
240 and 152. A defect in a live path that four systems read, in a project with
an automation suite and a lighting audit, surviving on a coincidence of shelf
height, is a weaker story than *the path isn't reached*. **"Nobody noticed
because of luck" should be the last explanation reached for, not the first.**
