# The store is still running, and you are inventory

Chris, 2026-10-31: *"the world around the lightkeepers is still moving, in some
way shipments still arrive, cars still get worked on, but it's all robotic,
humans are gone really. The consumption is in a cycle, things just keep
moving... it's programmed but somehow strangely permissive. Unknowing, but
angry."*

This is the largest fiction idea on the table and it unifies things that were
already in the data without changing any of them.

---

## 1. The roster was always job-shaped

Look at what is already authored in `data/enemies.json`:

| Enemy | `formerJob` |
|---|---|
| Sweepers | Floor cleaners |
| Stockers | Shelf stackers |
| Window Walkers | display mannequins |
| Ficus | mall landscaping |

Nothing in that list is a monster. Every one of them is **a role in a
retail operation.** The roster was built around jobs before anyone said the
store was still running, which means the fiction is not being bolted on — it
is being *noticed*.

So they are not hunting you. **They are working the shift.** You are not prey.
You are an irregularity on the floor.

"Unknowing, but angry" is exactly right, and it is the best description of the
tone available: there is no malice, because there is nobody home to be
malicious. The anger is **the friction of a process meeting an obstruction.**
A machine that cannot complete its cycle, escalating.

---

## 2. The lantern files a work order

This is the part that makes the whole thing click, because it gives the game's
central mechanic a reason it did not have before.

**Light reveals value and attracts danger.** `aggroRadiusLit` 38 against
`aggroRadiusDark` 12. Why does light attract?

Not because they see better. Because **a lit item is an item being handled.**
Turning on your lantern does not make you visible — it files a work order. The
store dispatches staff to anything that is being worked on, because that is
what a store does.

That reframes the locked interpreter rule without touching a word of it. *The
lantern makes existing things legible and causes visible machine responses.*
The visible machine response is **the store noticing a task.** The lantern was
always an instrument of the operation; the crew just uses it for salvage.

It also explains the thing that was previously only a number:
`attractedToDroppedLoot: true` on Sweepers. Dropped loot is **spill**, and a
floor cleaner goes to spill. That line of data stops being a balance decision
and becomes the most in-character behaviour in the game.

And at the climax, `storm-charge.json`'s `convergeAllPatrols`: a dock event is
a **receiving event**, and the whole store services receiving. The siren is not
an alarm about you. It is the shift being called to the dock.

---

## 3. Extraction channels: the store permits what fits its logic

Chris: *"Maybe other extraction points, like a side door but u can't carry as
much out, quicker pickups, cost u. Big loads at other slots, everything unique
and played on the environment and routing."*

The fiction makes this a system rather than a list of doors. **Each exit is a
logistics channel, and the store's permissiveness depends on whether what you
are doing matches what the channel is for.**

| Channel | What the store thinks is happening | Capacity | Speed | Heat |
|---|---|---|---|---|
| **Freight dock** | A shipment | Highest | Slowest | Highest — scheduled, loud, converges patrols |
| **Service / fire door** | An alarm | What a person carries | Fast | Different, not less — it is not a channel, so nothing services it, but something answers an alarm |
| **Customer entrance** | A purchase | Medium | Medium | Lowest, and conditional — the store is strangely permissive about goods leaving the front |
| **Parcel pickup / tube** | A small-goods transfer | One or two relics | Instant | Minimal |

That last column is the design: **capacity is paid for in heat.** Taking the
big load means telling the store you are a shipment, and the store responds to
shipments with its whole attention.

The customer entrance is the strangest and the most worth building, because it
is where "strangely permissive" becomes a mechanic. The store expects people
to leave through the front carrying things. It has no concept of theft — only
of process. Whether that stays unconditional or acquires a condition is a real
design question with a lot of room in it.

**Routing is the choice.** You do not pick a door at the end. You pick it when
you decide what to pick up, four rooms earlier, because the haul and the
channel have to match. That is the decision the middle game has been missing.

---

## 4. Penhurst's two entrances — corrected

Chris: *"Two entrances in Penhurst meant two from the mall to the store, only
one extraction unless we play on the side doors."*

Correcting `FOUR-ANCHORS.md`, which took them as two extractions. They are
two **mall-to-store** thresholds, not two ways out of the building.

What they actually do, which is still a lot:

- **Patrol loops.** Two connections to the concourse means routes that circle
  rather than dead-end. No other anchor allows that, and a looping patrol is a
  fundamentally different thing to evade than a back-and-forth one.
- **Crew splitting with a reason.** Two ways in means a crew can enter in two
  places deliberately, which is the first time the game has offered that.
- **Two ways in for things that are not the crew.**

Extraction variety comes from the channels in §3, not from store entrances.
Those are separate systems and conflating them was my error.

---

## 5. The mall: lit and exposed, stores dark and rich

Chris wants the real Galleria in Johnstown — full layout, fountain, carousel,
a little park, elevator clocktower, **high skylight ceilings.**

The skylights decide the structure of the whole game, and they invert it:

> **The concourse is lit. The stores are dark.**

Storm light pours through the skylights onto the mall floor. So the open
public space — fountain, carousel, park — is where you can see without paying,
and therefore where you are seen from a long way off. The anchors are dark,
rich, and the only places worth going.

That gives a clean loop with no tutorialising: **you cross exposed ground to
reach dark rooms.** The lantern is useless in the concourse and essential in
the stores, which means players learn to switch it off in the open without
anyone telling them. It is the fourth-floor lesson from `THE-STORE.md` — *roof
gone, lit by the storm, your lantern adds little and you are visible anyway* —
promoted from one floor to the connective tissue of the entire mall.

**The fixtures all earn their keep under the still-running fiction:**

- **The carousel.** If one thing in the mall should still be moving, it is the
  machine whose entire purpose is to go round forever. It is the cycle, made
  literal, in the middle of the building. It should run, and it should be
  audible from a distance, and it should be a navigation landmark precisely
  because it never stops.
- **The clocktower elevator.** A clock that still keeps time in a building
  with no one to be on time. It is also the natural diegetic home for Storm
  Charge — a rising value that only ever rises, displayed as a time.
- **The fountain.** Water, in a game about a flood. Running or stopped is a
  single prop decision that says which way the building is going.
- **The little park.** Where the Ficus came from. `docs/ZONES.md` already makes
  mall landscaping its origin; the park is where it worked.

---

## 6. The anchor that was never built

Chris wants the store slot that was *meant* to be occupied and never was.

Under the still-running fiction this is not set dressing, it is a structural
exception:

> **A store that never opened has no former staff.**

Every enemy in the game is a former job. The phantom anchor had no jobs. So it
is the one space in the mall with nothing of its own in it — raw concrete,
fixtures still crated, no stock, no signage, no history, nothing that ever
consumed anything.

Which makes it:

- **The only room with no native threat.** Anything you meet there walked in
  from somewhere else, and that is a different kind of fear — it has followed
  you rather than belonged there.
- **The natural forward camp.** A place inside the mall to regroup, drop a
  haul, breathe. The game currently has no such place between the hold and the
  hostile.
- **Thematically the sharpest room in the building.** Everything else is a
  machine still performing a function nobody needs. This is the one that never
  got to start, and it is therefore the only quiet place. That is a very
  pointed thing to sit in the middle of a game about consumption cycling on
  after the consumers are gone.

**It is not Lanternhold.** I checked `docs/LANTERNHOLD.md` before proposing
that and it would have broken locked geography: the hall is *out past the
farms*, rural, deliberately distant — *"You are safe because you are far."*
Taken from a real building whose cornerstone was laid in 1931 and whose main
room was not finished until 1948. The phantom anchor is a room in the mall; the
hall is a day's travel away. Two different unfinished buildings, and the
contrast between them is worth keeping rather than collapsing.

---

## 7. What cutting floors costs, and it is payable

Chris: *"idk if we need three floors, we can donate basement, idk if they have
one, idc. I know Boswell's has a third floor, but its anchor."*

Fewer floors is fine, but one thing has to move with them. `THE-STORE.md`'s
curriculum assigns **one lesson per floor**, in order, and two of the seven
live in the basement:

- **Lesson 5 — "sometimes the choice is made for you."** The floor with no
  ambient light at all, where running dark is not an option.
- **Lesson 7 — "leaving is the loudest thing you do."** Extraction at the
  loading dock.

Both have obvious new homes that do not need a basement:

- Lesson 5 belongs in **any sealed interior space** — a stockroom, a service
  corridor, the back-of-house behind the parts counter. The lesson needs
  *no ambient light*, not *underground*.
- Lesson 7 belongs wherever the freight channel is, which under §3 is a
  property of the channel rather than of a floor. In Sayers it is the bay
  doors.

So the floor count is negotiable. **The lesson count is not**, and the rule
that one floor teaches one thing has to survive the cut. If Boswell's goes to
three floors, three lessons live there and the rest redistribute — they do not
get dropped, because the building is the tutorial and a dropped lesson is a
thing nobody is ever taught.
