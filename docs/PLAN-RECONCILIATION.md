# Reconciling the production plan with the decision log

Status: **review**, 2026-10-27. Written by Claude against Codex's
`docs/STEAM-PRODUCTION-PLAN.md` (uncommitted, "proposed for approval").

Three documents were written on the same day by three parties who could not
see each other's work: `docs/LANES.md` and `docs/ROADMAP.md` (the playtest
session), the row 43 tutorial curriculum in `docs/THE-STORE.md` (Claude), and
the production plan (Codex). They agree on far more than they disagree on —
PC-first, UE5, listen server, server authority from day one, data-driven
tuning, no expansion before the loop proves fun, one store rather than the
mall. **Nothing below is a disagreement about the game.** They are collisions
of record: places where two documents now state different things, and a
reader cannot tell which is canon.

Twelve are listed. Six need Chris and nobody else.

---

## Needs Chris

### 1. First person or third person — never decided, and now assumed both ways

`ROADMAP.md` M1.1 specifies a **first-person** keeper. The plan's week 1 says
**third-person** movement and camera. Neither document knows it is in
disagreement, and **no row in `DECISIONS.md` has ever settled this.** That is
the real finding: the most consequential unrecorded decision in the project.

It is not a preference. The whole game is *light is how you see*:

- **First person** makes the lantern the player's own eye. The darkness is
  genuinely blinding, the cone of light is the entire interface, and a thing
  standing outside it is unseen. It is also the only perspective where the
  player cannot see around a corner their light does not reach — which is
  exactly the information the design is trying to withhold.
- **Third person** shows the player their own keeper, which sells the carried
  lantern, the burden animation and the downed crawl, and is far kinder to
  co-op legibility — you can see your crew. But it hands the player sight the
  lantern did not buy, and that is the mechanic's one currency.

The genre is split and gives no answer: Lethal Company and Content Warning are
first person, R.E.P.O. is third. Both sold.

**My recommendation: first person**, because every rule in this project is
about what you can and cannot see, and third person quietly refunds some of it.
But this is a game-feel decision, it is Chris's, and whichever way it goes it
needs a row, because two lanes are currently building toward opposite answers.

### 2. Git or Perforce

The plan says **Perforce for UE assets, Git optionally for code and docs.**
`ROADMAP.md` M0.3 says **Git with LFS for `Content/**`**, and the
`.gitattributes` and `.gitignore` for it were committed this morning.

Both are defensible. Perforce is what the industry uses for binary assets, it
locks files properly, and Helix Core is free for up to five users. Git LFS is
already here, costs nothing to continue, and is where every document and every
agent contribution in this project lives.

The hidden cost of the split is the thing to weigh: **two version-control
systems means no single commit ever captures a whole change.** A lighting fix
that touches a `.umap` and a C++ file would land in two places with no shared
history, and the decision log's whole value is that every change has one
reasoned record.

**My recommendation: stay on Git + LFS until `Content/` actually hurts** — and
it cannot hurt yet, because `Content/` does not exist. Revisit when the first
real `.umap` is being edited weekly. Row 41 (Chris owns `Content/` alone)
already removes the merge problem that Perforce's locking is for.

### 3. Run length: the plan locks 10–12 minutes, O1 does not

The plan states a **10–12 minute** expedition as a lock. **O1** in
`DECISIONS.md` has the target at **6–10 minutes** with a hard ceiling of 20,
and notes those are different kinds of number. An agent cannot lock a row;
only Chris can.

New evidence that bears on it, from today's research: the median demo playtime
across roughly 130 games is **14 minutes** (Zukowski's data). If a raid is
10–12 minutes, a median demo player sees **one run** and never learns the
loop is a loop. That argues for the shorter end of O1, or for the first run
being deliberately shorter than the steady-state run.

### 4. Blueprint policy

Row 40 (mine, **PROPOSED**): gameplay logic in C++, never Blueprint. The plan:
C++ for replicated core systems, **Blueprints for authored encounters and
content.**

The plan's version is more practical and I would mostly concede it — but
"authored encounters" is precisely where the fear lives, and an encounter
authored in a Blueprint graph is an encounter no agent can read, review or
fix. The workable line is narrower than either document: **C++ owns every
rule; Blueprint may place, sequence and dress authored content, and may not
contain a number that changes how the game plays.** The test is whether a
reviewer who can only read text can tell what the thing does.

### 5. The engine version

The plan names **UE 5.8**. `ROADMAP.md` M0.1 is Chris's task and says install
and **pin** the version, deliberately leaving it unnamed.

Worth knowing before pinning, from today's research: Lumen's software
detail-trace path was **deprecated in 5.6**, and **MegaLights** — the feature
whose shape actually fits one moving light in a black interior — is Beta in
5.7 and requires **hardware ray tracing**. So the version choice carries a
minimum-spec consequence and should not be made casually. It should be made
once, written into the roadmap, and not moved mid-milestone.

### 6. Which roadmap is canon

`ROADMAP.md` is **M0–M7, gate-based**, with owners and a "done when" per task.
The plan is a **16-week calendar**. They do not contradict each other in
spirit; both refuse to expand before the loop proves fun, and both put the
playtest before the art pass.

But two roadmaps means neither is canon.

**My recommendation: `ROADMAP.md`'s milestones stay the structure, and the
plan's calendar becomes the schedule inside M0–M4.** Milestones gate on
evidence; calendars gate on dates, and a date-based plan written while the
engine is not yet installed will be wrong in week one. M0.1 — install Unreal —
is still the only true blocker in the project, and it is Chris's.

---

## Reconciled without needing a decision

### 7. The slice's spaces and the tutorial curriculum are the same building

The plan lists five spaces: entrance/cosmetics, home goods, fitting rooms and
staff hall, stockroom and loading bay, one upper overlook. `THE-STORE.md` has
a four-floor-plus-basement ruin gradient, which as of row 43 is also the
teaching curriculum. The plan was written before row 43 existed, and on its
face it drops two lessons — the basement's *no ambient light at all* and the
fourth floor's *storm-lit, roof gone*.

**It mostly does not, and the fix is free.** The plan's "stockroom and loading
bay" **is** `THE-STORE.md`'s basement — receiving and the loading dock are
already down there. Name it as the basement and curriculum lessons five and
seven (the choice is made for you; leaving is the loudest thing you do) come
back at no cost, in the space the plan already budgeted.

What genuinely does not fit in the slice is the roofless fourth floor. The
plan's "upper-floor overlook" can carry a weaker version of it — storm light
from outside, your lantern worth less — and the full lesson waits. Recommend
the overlook be the top of the stairwell with weather coming in.

### 8. The plan's own teaching gate needs the brightest-position rule

The plan's Weeks 6–9 gate is *a new tester reaches the objective and
understands extraction without a tutorial wall.* That is row 43 arrived at
independently, and it is the right gate.

It will fail for a reason the plan cannot see. Today's research found that in
four-player co-op the keeper who lights the lantern and the keeper who meets
the enemy are usually different people, so neither learns the rule. The fix is
the already-locked rule that **enemies converge on the brightest position, not
the nearest player** — see `THE-STORE.md`. The archived prototype breaks it.
The UE5 build must not inherit that bug, and the gate above should be tested
**solo first**, because solo is the only configuration where the lesson cannot
miss.

### 9. GAS, World Partition and OFPA are a real cost for one person

The plan specifies the **Gameplay Ability System**, **World Partition** and
**One File Per Actor**. All three are correct at mall scale and two of them
are expensive now.

- **OFPA: yes, immediately.** It is the cheapest merge-conflict reduction
  available and it costs nothing to turn on first.
- **World Partition: defer.** It is streaming infrastructure for a world that
  does not fit in memory. One department store does. It is the right call for
  the Galleria and the wrong one for M1.
- **GAS: defer until a second ability needs it.** Lantern, sprint, crouch,
  carry, knockdown and revive are six functions on a component. GAS is a
  framework with a long learning curve and real networking subtlety, adopted
  for a solo developer's convenience it does not yet need. Row 42's
  replication requirement is satisfied without it.

None of this is a disagreement about the destination. It is build order.

### 10. The defense tool matches COMBAT.md, which is still proposed

The plan's "one active defense tool and one diversion — do not turn the game
into a shooter" lands almost exactly where `docs/COMBAT.md` landed on
2026-10-26. That is agreement, not collision, and worth noting because it was
reached independently.

The flag: `COMBAT.md` is **proposed**, not locked. The plan depends on it.

### 11. Dedicated-server simulation is listed as required, and dedicated servers are cut

Minor and probably intentional, but it reads as a contradiction. Testing in
the editor's dedicated-server mode is a **bug-finding tool** — it is the
fastest way to catch client-authority mistakes — while shipping dedicated
servers is a product decision the plan correctly cuts. Say which it is, so
nobody later reads it as a commitment to host servers.

### 12. Nine lanes for a team of one

The plan's division-of-labour table has nine roles, then says one person may
hold several, then reduces to three. The three are right and match
`LANES.md`'s shape. The nine-role table is useful as a checklist of jobs that
exist whether or not anyone is doing them — which is what `LANES.md`'s
responsibility table is for too. Recommend the nine-role table be kept
explicitly as *jobs nobody has volunteered for*, not as a staffing plan, so no
reader mistakes it for a team.

---

## What the plan adds that nothing else in the repo had

Said plainly, because the list above is all friction and the plan deserves
better than that.

- **An asset pipeline.** The modular retail kit, the single master damage
  material, the dressing recipes, and the density rule — *realism comes from
  deliberate repetition* — are the first credible answer to "massively stocked
  without eight thousand unique models," and they are more concrete than what
  `THE-STORE.md` had.
- **Audio as a first-playable system rather than a polish pass.** This project
  has an accessibility rule (row 16: every enemy telegraphs with sound before
  sight) that nothing was scheduled to satisfy. Now something is.
- **"Every light must have a physical reason," and "darkness is navigable,
  never illegible black."** Two rules the lighting lane needed and did not
  have.
- **Per-feature test modes.** Solo, listen host plus remote client,
  dedicated-server simulation, packaged build. The prototype's entire history
  is bugs that only appeared in one of those.
- **Metrics that decide expansion**, which is M4's gate given actual
  instrumentation — the one thing the research said everything else is
  hypothesis without.

The plan should be committed and kept. These are edits, not objections.
