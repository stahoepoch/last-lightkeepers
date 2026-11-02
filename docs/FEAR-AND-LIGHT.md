# Fear and light — what the research actually supports

Researched 2026-11-02, on Chris's question: *"isn't there anything on the
internet to research or any academics that tell about lighting... horror...
psychology?"*

**Evidence labelling.** `[HARD]` = peer-reviewed experimental work. `[REVIEW]`
= published review. `[WEAK]` = design blog, uncited or citing secondhand.

**The honest headline first:** searching for "horror game lighting psychology"
returns almost entirely content-farm material. The useful literature is not
about horror games at all — it is about **threat predictability** and **how the
eye works in the dark**, and both transfer directly.

---

## 1. Fear and anxiety are different responses, and the difference is cueing

`[HARD]` The **NPU-threat test** (Schmitz & Grillon, *Nature Protocols* 2012)
separates them with three conditions: **N**o threat, **P**redictable threat
(signalled by a cue), **U**npredictable threat (no cue).

- Predictable threat drives **fear**-potentiated startle: it spikes when the
  cue appears, and it resolves.
- Unpredictable threat drives **anxiety**-potentiated startle: sustained,
  present without any cue, and it does not resolve.

`[HARD]` Chronically elevated startle to *unpredictable* but not predictable
threat is a documented marker of panic disorder and PTSD. It is the signature
of the conditions people seek treatment for.

### What this means for the Sweeper

**A roving enemy with no learnable pattern is the U condition, continuously.**
That produces sustained anxiety with no release — which is exhausting rather
than thrilling, and it is exactly Chris's report that the Sweeper *"patrols too
hard"*.

The pleasure in horror lives in the **alternation**. P gives you spikes you can
see coming and survive. N gives you the baseline that makes the spikes mean
anything. A game that is all U has deleted both.

**The concrete defect:** `ALLKSweeper::NextPatrolPoint` advances `PatrolIndex`
the instant it arrives within tolerance. There is no dwell. A floor cleaner
that never once pauses is not reading as a machine doing a job — it reads as
something hunting, permanently.

Adding a dwell at waypoints converts the patrol from U to P: the loop becomes
learnable, the gap becomes timeable, and crossing it becomes a decision the
player can feel clever about. That is a small change with a large effect on
what the encounter *is*.

### And this is the WildStar connection, which is not a coincidence

A telegraph is **a device for converting unpredictable threat into predictable
threat.** It is the P condition, authored deliberately, at high density. That
is why skilled players loved it: it made lethal danger legible and then
demanded you act on the information in time.

Row 16 already requires every enemy to announce itself with sound before
sight, and calls it an accessibility requirement rather than polish. The
research says it is also the thing that makes the encounter *enjoyable* rather
than merely stressful. Those are the same rule.

## 2. Safety signals are a distinct mechanism, and we have none

`[HARD]` A **conditioned inhibitor** — a cue that actively predicts the
*absence* of threat — inhibits fear, and it is neurally distinct from a cue
that merely was never paired with threat. A 7T fMRI study found the inhibitor
engaged dorsal striatum, anterior insula and dlPFC, while a passive safety cue
drove ventromedial PFC instead. `[REVIEW]` Laing & Harrison (2021) note safety
learning is under-researched next to extinction, so treat the mechanism as
established and the details as provisional.

### What this means for light

**Light in this game is currently a pure cost.** It reveals value and attracts
danger (row 5), and nothing anywhere is coded as safe. There is no N condition
in the whole design.

That is worth changing, carefully: if some light is *learnedly* safe — a place
where nothing has ever come — then darkness regains its meaning by contrast,
and the player acquires something to lose.

It also makes the false all-clear in `WORLD-SYSTEMS-BOARD.md` into a mechanism
rather than a metaphor. **A safety signal that stops being safe is worth more
than any jump scare**, because the player built the expectation themselves over
hours, and the game only has to withdraw it once.

## 3. The dark eye is a different instrument

`[HARD]` Scotopic (rod-mediated) vision, under genuine low light:

- **No colour.** Rods do not signal it.
- **Poor acuity.** Fine detail is unavailable.
- **Concentrated outside the fovea**, so motion is caught better at the edge of
  vision than straight ahead.
- `[HARD]` An *iScience* study (Feb 2024) tested behaviour under true scotopic
  conditions — participants still read accurately and still showed the face
  inversion effect, so the dark eye is not simply a degraded one.

### Three consequences for the lighting that is currently bad

1. **Threat should be detectable peripherally.** A beacon that only reads when
   looked at directly fights the eye. Low-frequency luminance change and motion
   at the edge of vision is how darkness actually alarms people.
2. **Colour is the wrong channel in the dark.** The green emergency fixture is
   doing work the eye cannot receive at that light level — a luminance
   difference would do it better. This is a physiological reason, not a taste
   argument, for why the cream/green placeholder reads as off.
3. **Detail is unavailable in the dark, physiologically.** Row 5's rule that a
   relic is visible but unreadable without lantern light is not an arbitrary
   game conceit. It is what human vision does.

## 4. The one design claim the weak sources agree on

`[WEAK]` Across the design blogs, the recurring and plausible claim is that
**uncertainty rather than darkness is the frightening part, and total darkness
is worse than contrast** — players still need to navigate, and a uniformly
black room gives the eye nothing to work with.

That agrees with sections 1 and 3 from two independent directions, which is the
only reason it is here.

**Applied to Boswell's as it stands:** a uniformly black room with two pools
landing on the *ceiling* is maximum uncertainty and minimum legibility at once.
The fix is not more light. It is **structure** — pools with dark between them,
so the eye has edges and motion to read the space by, and so that moving
between them is a decision rather than a fumble.

---

## Sources

- Schmitz & Grillon, *Nature Protocols* 7:527-532 (2012), the NPU-threat test —
  via [Semantic Scholar](https://www.semanticscholar.org/paper/Assessing-fear-and-anxiety-in-humans-using-the-of-Schmitz-Grillon/09633a42f66194c7ba5fccce6b6d4d0f5eda10ac)
- [Impact of trauma type on startle reactivity to predictable and unpredictable threats](https://pmc.ncbi.nlm.nih.gov/articles/PMC8633038/)
- [Startle potentiation to uncertain threat across internalizing disorders](https://pmc.ncbi.nlm.nih.gov/articles/PMC5215951/)
- [Transdiagnostic fear and anxiety: the NPU task](https://www.sciencedirect.com/science/article/pii/S2667174322001367)
- [Cortico-striatal activity characterizes human safety learning via Pavlovian conditioned inhibition](https://pmc.ncbi.nlm.nih.gov/articles/PMC9233447)
- [Safety learning and the Pavlovian conditioned inhibition of fear in humans (review)](https://pmc.ncbi.nlm.nih.gov/articles/PMC6936350)
- [iScience (Feb 2024), behaviour under scotopic conditions](https://doaj.org/article/cb824bcedec74d70b9627227b2bbabcc)
- [Peripheral vision and what we can see in the dark](https://www.snexplores.org/article/peripheral-vision-and-what-we-can-see-in-the-dark)
- `[WEAK]` [Horror game lighting techniques](https://descent-freespace.com/horror-game-lighting-techniques/), [The art of fear: horror level design](https://www.algoryte.com/blogs/the-art-of-fear-secrets-of-horror-game-level-design/), [How colour elevates fear in horror video games](https://drwedge.uk/colour-horror-videogames/)

## What already existed, and nearly got researched twice

`research_notes/` holds roughly 1,240 lines from an earlier deep-research pass:
fear habituation, the indifferent antagonist, non-lethal escalation, retention
drivers, premium endgame. **None of it covers lighting**, which is why this
file exists — but it was checked first, and `indifferent_antagonist.md` in
particular is directly load-bearing for row 47.

That folder is **gitignored** as "regenerable, not source of truth". It is on
one disk, in one place, and nothing is holding onto it.
