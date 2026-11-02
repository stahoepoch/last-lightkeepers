# How space and time make knowledge — research, and what it buys us

Chris asked what different elements of time and space do to knowledge. There
is a deep literature on exactly this, and the useful finding is that it splits
cleanly into three questions this game already answers with three separate
systems.

| Question | Research area | The system that answers it |
|---|---|---|
| What kind of knowledge does a *place* give you? | Spatial knowledge acquisition | **The store signature** |
| When does knowledge stop being true? | Event segmentation | **Storm stage** |
| How does knowledge move between people? | Transactive memory / distributed cognition | **The Fieldboard** |

Nobody appears to have built a horror game where these are three deliberately
separate, independently tuned systems. That is the thesis of this page.

---

## 1. Places give different *kinds* of knowledge

Siegel and White's 1975 framework describes spatial knowledge arriving in
three forms:

- **Landmark knowledge** — recognising salient features.
- **Route knowledge** — an ordered sequence of actions linking landmarks.
- **Survey knowledge** — a map-like representation of global layout. This is
  the level that supports shortcuts, detours, and pointing at something you
  cannot see.

Thorndyke and Hayes-Roth (1982) found that **the way you learn determines
which you get**: navigating a place preferentially builds route knowledge,
while studying a map builds survey knowledge. They converge eventually, but
not quickly.

### What this buys us

Chris's correction — *mirrored ceilings are not everywhere; each store has a
unique element* — becomes a taxonomy instead of a vibe. **The store signature
decides which kind of spatial knowledge the building hands you for free**, and
that is a far deeper difference between stores than different furniture.

- **Mirrored ceiling islands → survey knowledge.** A plan view from directly
  below. The store feels controllable, and shortcuts become possible.
- **A working PA or pneumatic tube run → route knowledge.** A sequence of
  audio cues along a path. You learn the way without learning the shape.
- **An open escalator well or atrium → survey knowledge, vertically.** Three
  floors legible at once; the building's section rather than its plan.
- **Skylights → landmark knowledge.** Bright places you can name and steer
  by, and which expose you, inverting everything.

A crew that has learned Boswell's walks into Sayers and finds not just new
furniture but **a different way of knowing where they are.** Their habit is
gone. That is a real second-store experience rather than a reskin.

It also reframes the lantern projection. Charging light to consult the map
(see `UI-NOVELTY.md` §0) is not only a balance decision — the map is the
survey-knowledge instrument, so its price decides **which kind of knowledge
the player buys.** A crew that pays for the map thinks in shapes; a crew that
refuses to pay thinks in routes. Both are viable, and they will play
noticeably differently.

**The caveat, and it is live.** Montello (1998) argued survey knowledge can
develop in parallel rather than last, and at least one review states the
staged model has not received convincing empirical support as a description of
how spatial knowledge actually develops. So: use the three *kinds* as a design
vocabulary, which they clearly are. Do not build a progression system that
assumes players must pass through them in order.

---

## 2. Boundaries in space make knowledge expire

The **doorway effect** (also called the location updating effect) is a memory
deficit for things encountered in another room. Walking through a doorway
causes more forgetting than walking the same distance within one room.

The explanation is event-model theory: a doorway acts as an **event
boundary**. The brain closes one event model and opens a new one, and
information from the previous model becomes harder to reach because it is no
longer the relevant one.

### What this buys us

**Third floor already does this, and nobody designed it in.** `THE-STORE.md`
describes it as *"showroom walls cut the floor into rooms, so sightlines are
short and you are never sure what the next doorway holds,"* and assigns it the
lesson *the building takes away what you learned to rely on — your
sightlines.*

The research says it takes something else too. A floor made of doorways will
produce real, measurable forgetting in real players. The lesson is stronger
than its author knew, and the floor is the correct place for the Fieldboard to
matter most: a board consulted right after crossing a boundary is compensating
for an actual cognitive deficit, not a fake one.

**And it validates storm stage as the staleness clock.** Event segmentation
is about boundaries causing memory updating — a stage change is an event
boundary in time exactly as a doorway is one in space. So marking a row *"before
the surge"* is not a cosmetic phrasing choice. It is naming a boundary the
player's own memory has already used to file that information as belonging to
before. The UI and the player's cognition agree, which is why it will feel
right rather than clever.

**The honest caution.** The mechanism is contested. Pettijohn and Radvansky
(2016) themselves ask whether it is event structure or a temporary processing
disruption, and a Columbia study found event-boundary effects in memory only
when the working-memory task was event-related. The effect is well replicated;
the explanation is not settled. So design to *benefit* from it, never to
*depend* on it. A mechanic that only works if players forget is a mechanic
built on a contested finding.

---

## 3. Knowledge lives between people, not in them

Hutchins's *Cognition in the Wild* studied a navy navigation team and argued
that cognition is distributed across tools, across people, and across time —
and that a group can have cognitive properties none of its members have.

A crew navigating a large vessel by distributing knowledge across people,
instruments and time is, with no translation required, four keepers
navigating a dead department store.

**Transactive memory** (Wegner, and later Hollingshead) sharpens it: a group
remembers by combining what individuals know **with a collective awareness of
who knows what.** Members do not need to know what their teammates know. They
need to know *where the knowledge lives.*

Team situation awareness work adds the mechanism: awareness is distributed
through **transactions of information**, and more transactions predict better
performance.

### What this buys us

**The attribution field on a Fieldboard row is the whole feature, not
flavour.**

> Sweeper — toy department — *Mara, 4m ago, before the surge*

Strip "Mara" out and it is a bulletin board. Leave it in and it is a
transactive memory index: it tells you **who to ask on the radio.** That is
the single most load-bearing word on the row, and it is the reason the radio
and the board are one system rather than two.

It also suggests a feature I have not seen anywhere, and it falls straight out
of the theory:

> **A "who knows what" column.** Per crew member, what they have attested —
> not where they are, not their health, not a live anything. Their public
> record. *"Mara: 4 relics read, floors 1 and 3."*

That is a transactive memory index made literal. It is not a radar, it leaks
no private or positional information, it cannot narrate, and it answers the
question a real crew actually asks under pressure: *who has been down there?*
Every co-op game shows you teammate status. None of them shows you teammate
**expertise**, which is the thing the research says groups actually run on.

The transaction framing also gives the radio a measurable job: a crew that
talks has more transactions, more shared awareness, and should perform better.
That is a reason to make the radio good beyond atmosphere — and a reason the
no-punishment rule on proximity voice matters, since a mechanic that
discourages transacting is a mechanic that degrades crew cognition on purpose.

---

## 4. The synthesis

Three systems, three questions, one sentence each:

- **The store signature decides what kind of knowing the building gives you.**
  Survey, route or landmark — and therefore how a crew thinks in that store.
- **The storm stage decides when knowing expires.** A boundary in time, which
  players' own event segmentation already treats as one.
- **The Fieldboard decides how knowing moves.** Attributed, timestamped,
  contradictions preserved — a transactive index rather than a state readout.

The lantern sits under all three without changing: it makes existing things
legible and never narrates. Every system above moves knowledge that a person
acquired. None of them manufactures knowledge the world did not contain.

That is the version of "the light is an interpreter" that survives contact
with the cognitive science, and I think it is the strongest frame this design
has.

---

## Sources

- [Acquisition of landmark, route, and survey knowledge in a wayfinding task: in stages or in parallel?](https://www.researchgate.net/publication/342933676_Acquisition_of_landmark_route_and_survey_knowledge_in_a_wayfinding_task_in_stages_or_in_parallel)
- [Landmark-Route-Survey knowledge theory, Siegel and White](https://www.researchgate.net/figure/Landmark-Route-Survey-Knowledge-theory-according-to-Siegel-and-White-23_fig4_270215846)
- [Different "routes" to a cognitive map: dissociable forms of spatial knowledge from route and cartographic map learning — PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC4169750/)
- [Where are we going and where have we been? Effects of maps on spatial learning in indoor guided navigation — Cognitive Research](https://cognitiveresearchjournal.springeropen.com/articles/10.1186/s41235-020-00213-w)
- [Spatial knowledge acquisition from direct experience — ScienceDirect](https://www.sciencedirect.com/science/article/abs/pii/S0010028505000733)
- [Walking through doorways causes forgetting: event structure or updating disruption? — Pettijohn & Radvansky (PDF)](https://memorylab.nd.edu/assets/259392/pettijohn_radvansky_2016_quarterly_journal_of_experimental_psychology_.pdf)
- [Walking through doorways causes forgetting — Notre Dame News](https://news.nd.edu/news/walking-through-doorways-causes-forgetting-new-research-shows/)
- [The effect of spatial boundaries on memory in a virtual environment — PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC13461692/)
- [Predictable boundaries — Davachi Lab (PDF)](https://davachilab.psychology.columbia.edu/sites/davachilab.psychology.columbia.edu/files/content/Predictable%20Boundaries.pdf)
- [Distributed Cognition (PDF overview)](http://faculty.lawrence.edu/wp-content/uploads/sites/18/2015/11/DCog.pdf)
- [Transactive memory systems: a mechanistic analysis of emergent group memory](https://www.researchgate.net/publication/257797625_Transactive_Memory_Systems_A_Mechanistic_Analysis_of_Emergent_Group_Memory)
- [Situation awareness as distributed cognition](https://www.researchgate.net/publication/228358661_Situation_awareness_as_distributed_cognition)
- [Keeping it together: transactional situation awareness in team performance — ScienceDirect](https://www.sciencedirect.com/science/article/abs/pii/S0169814116300117)
- [Team cognition — ScienceDirect topic overview](https://www.sciencedirect.com/topics/psychology/team-cognition)
