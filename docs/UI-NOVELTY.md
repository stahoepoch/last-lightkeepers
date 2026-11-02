# Looking up instead of down — novel directions for the Fieldboard, map and projection

Supplement to `docs/HUD-STATUS-SCREEN.md` (`e0a91fb`). The four-job split in
that document — world / Tab / M / radio — is correct and I am not proposing to
change it. What follows is about what goes *inside* those jobs, and one rule
the current plan quietly breaks.

Two of the ideas here appear to be unoccupied ground. I searched for prior art
and did not find any; sources at the end.

---

## 0. The rule the map currently breaks

Everything in this game obeys one law: **information costs light, and light
costs safety.** The relic is unreadable until you light it. `aggroRadiusLit`
is 38 against `aggroRadiusDark` 12. The ground floor's whole lesson is that
the player who lit the lantern is the player who gets found.

Then the map is free.

A lantern route projection that is "cosmetic only, not a second gameplay
light" is a safe menu wearing a costume. It looks diegetic and behaves like a
pause screen. Every other instrument in the game charges you; this one does
not, so players will consult it constantly and it will feel like UI no matter
how it is drawn.

**The projection should put the lantern into the gameplay light field for as
long as it is up.** Not a second light source — the same one, doing the thing
it already does. Checking where you are is exactly as dangerous as reading a
relic, because it is the same act: using the instrument to make something
legible.

That single change turns the map from a menu into a decision, and it needs no
new system at all.

---

## 1. The ceiling is the map

Chris asked for an upside-down point of view. The store already has one, and
it is sitting unused in the art direction.

`THE-STORE.md` locks **mirrored ceiling islands** as a non-negotiable visual
signature: a player should periodically look up and see the store doubled
above them.

A mirrored ceiling over an aisle, seen from below, **is a top-down view of
the floor plan.** That is what a map is. The player looks up and sees the
layout of the department they are standing in — aisle runs, gondola
positions, which way the concourse goes, where the gaps are.

No UI. No overlay. No key. The map is a building material.

Why this is worth more than it sounds:

- **It is free.** Reflection captures are already budgeted for the ceiling
  pass. Nothing new renders.
- **It is geographically self-balancing.** Mirrors go over main aisles, never
  over department interiors or the dark pockets. So wayfinding is easy
  exactly where the store is navigable and absent exactly where it is not —
  restrooms, fitting rooms, service corridors, basement. The difficulty curve
  falls out of the art direction without anyone tuning it.
- **It makes M meaningful instead of redundant.** The projection stops being
  a general-purpose minimap and becomes the thing you need *only* where the
  ceiling gives you nothing. Which is the dark, which is where it costs the
  most to use. The two systems divide the building between them.
- **It inverts the genre reflex.** Every other game puts the map at the
  bottom of the screen and you glance down. Here you tilt your head back and
  take your eyes off the floor to find out where you are. That is a horror
  beat built into an act of navigation.
- **The segmentation we already require is a feature here.** Faceted panels
  fragment the reflection, so the ceiling gives you shape and direction but
  never precision. It orients; it does not solve.

### It is a store signature, not a system

**Chris, 2026-10-31: mirrored ceilings are not everywhere — only some
department stores had them, and each store will have a unique element of its
own.**

That is the right constraint and it makes the idea stronger rather than
weaker. The mirrored ceiling is **Boswell's signature affordance**, not the
game's navigation system. `DECISIONS.md` 19 names four anchors — Boswell's,
Sayers, Penhurst, Bellevue — and `THE-STORE.md` already asks each department
for *one landmark object that exists nowhere else in the building.* This
extends that from a landmark to a **way the building helps you**, one per
store:

- **Boswell's** — mirrored ceiling islands. You orient by looking up.
- **A store with a working PA or pneumatic tube run** — you orient by sound,
  and the building tells you where you are without light.
- **A store built around an open escalator well or atrium** — you orient
  vertically, seeing three floors at once.
- **A store with skylights** — you orient by storm light, which inverts
  everything: the readable places are the exposed ones.

Each store then plays differently at the level of *how you know where you
are*, which is a far deeper difference than different furniture. A crew that
has learned Boswell's walks into Sayers and finds their navigation habit
gone.

**The hard requirement this creates:** the universal tools — the lantern
projection, the directory boards, the Fieldboard — must be sufficient on
their own in a store with no signature at all. The signature is always a
bonus layer, never load-bearing. Otherwise the second store is unplayable
with the first store's muscle memory, which is the opposite of the intent.

**The honest counterargument**, and it is real: mirrors invert left and
right, and at least one discussion of mirrored geometry in games names
disorientation as the reason reflections are usually avoided for wayfinding.

I do not think that disqualifies it, for two reasons. The reflection is seen
from directly below rather than facing a wall mirror, so it reads as a plan
view rather than a flipped room. And disorientation is not a bug in this
genre — a navigation aid that is slightly wrong-handed under pressure is a
better horror instrument than a correct one. But it needs a playtest before
it is locked, and it should never be the *only* way to orient.

---

## 2. The lantern is a projector, not a hologram

A hologram is science fiction. This lantern is an issued service instrument
with a lens, and the honest period version of "project the route" is a
**slide projector**: it throws an image onto a surface.

So M does not float a hologram in front of your face. It casts the route onto
whatever you point it at.

- Point it at a clean wall and you get a readable image.
- Point it at a gondola run and the image breaks across the shelves.
- Point it at the floor and you get it underfoot, foreshortened.

This does several things at once. It makes consulting the map a small ritual
with a position and a posture — find a surface, stand still, look. It gives
the player a reason to care about the room's geometry. It is unmistakably a
physical object doing a physical thing, which is the entire fiction. And it
pairs naturally with the light cost in section 0, because a lantern pointed at
a wall is a lantern that is on.

It also keeps the interpreter rule exactly: the lantern is making something
legible that already exists. It is not narrating.

---

## 3. The Fieldboard should model knowledge, not state

This is the one that matters most for co-op, and I think it is the most
genuinely novel thing available here.

Every co-op HUD shows **state**: here is what is true right now. The honest
version for this game shows **knowledge**: here is what the crew found out,
and when.

So the Fieldboard carries timestamps rather than truth.

> East stairwell — clear — *reported by Mara, 6 minutes ago*

That is not a lie and it is not a radar. It is a record of an observation a
real player made at a real moment. It can be stale. It can be wrong now. The
board never claims otherwise, because it always says when.

What this unlocks:

- **Enemy sightings become shareable without becoming a radar.** You can
  absolutely show "Sweeper seen here, 40 seconds ago" — that is a teammate's
  observation, not a live position. The staleness is the gameplay. A
  40-second-old sighting is useful; a six-minute-old one is a ghost.
- **It resolves the thing `HUD-STATUS-SCREEN.md` deferred.** The named relic
  ledger is waiting on server-owned relic records, and `bLegible` already
  latches — once identified, always identified. A latched flag is a record of
  something that happened, which is precisely what this board is for. The
  ledger is not a different feature; it is the first row of this one.
- **It gives the radio a job no other channel can do.** Information gets onto
  the board because somebody put it there deliberately. That is the lantern's
  rule inverted: the machine never narrates, but a person may.
- **Four players produce a document together.** Nobody has built a co-op
  horror UI that is a shared, decaying, player-authored record. The genre's
  standard is either full information or none.

The discipline that makes it work: **nothing appears on the board that no
player observed.** The moment the game writes a row by itself, it has become
a narrator and the interpreter rule is broken by the back door.

---

## 4. Suspense without hindrance: the board occludes

Chris's constraint was a HUD that does not hinder the player but can add
suspense meaningfully. Those pull in opposite directions in a fullscreen
overlay, and they stop fighting the moment the board is a physical object.

Hold it at chest height. It blocks the lower-centre of your view. You can see
over it, around it, past it — the horizon, the aisle ahead, movement at the
edges. What you cannot see is **the floor directly in front of your feet**,
which is where something low and quiet would come from.

Nothing is taken away that you need in order to run. Something is taken away
that you would quite like to have. That is suspense nobody had to script, and
it costs one layout decision.

This is the co-op half of a known pattern. Non-paused inventory as a tension
device is established — Dead Space's holographic RIG projection is the
reference, and the forum argument is right that the world keeps running
because the character stays visible. What is undocumented is what happens in
co-op: **your menu time exposes other people.** I could find no sources on
that at all. Four people reading boards at once is a crew with nobody watching
the room, and that is a real decision the game can ask without a line of UI
copy.

---

## 5. Where the map comes from

One more piece, because "the crew has a map" is an assumption worth paying
for.

`THE-STORE.md` already lists **a directory board by the escalator** in the
signage kit. Make that the source. You do not start with a map. You read the
store's own directory — and reading it is the same act as reading a relic,
which means it needs the lantern, which means it costs light, in a fixed
location near the escalator well where the sightlines are long.

One player lights the directory; the whole crew gets that floor. It is a real
objective, it teaches the game's central trade in a place where the answer is
affordable, and it means the map arrives as something the building gave you
rather than something the UI granted you.

---

## 6. On Tab: toggle is right, with one addition

Agreed with the conclusion in `e0a91fb` — toggle, not hold. Hold-to-read is a
sustained input for a reading task, which is what the accessibility guidance
warns against, and reading time varies by person.

The addition: **make closing it faster than opening it.** Not audio — motion.
Tab down is a considered action; Tab up should be immediate and physical, so
that the instant you decide you need to look at the room, you are already
looking at it. The cost of having opened it must never include a delay in
closing it. That is the difference between a UI that creates tension and one
that creates frustration, and it lives entirely in the animation curve.

---

## What appears to be unoccupied

I looked for prior art on both load-bearing ideas and did not find it.

- **Reflections as navigation.** Diegetic maps are well established — BD-1's
  projected hologram in *Jedi: Fallen Order*, the PDA in *STALKER 2*, *Dead
  Space*. Mirrors appear as a combat cue (*Hellblade*'s focus mirror), as
  portals, and as puzzle surfaces. Nothing found uses a reflection to tell you
  where you are.
- **UI consultation costing visibility.** *Thief*'s light gem is the closest
  relative — it turns light into exposure — but it measures your state rather
  than charging you for reading something. Nothing found charges the player
  for consulting their own interface.

Both are small ideas mechanically. Neither needs a new system. They are
unoccupied because they only make sense in a game where light is already the
central currency, and that is the game this is.

---

## Sources

- [Diegetic Interface — TV Tropes](https://tvtropes.org/pmwiki/pmwiki.php/Main/DiegeticInterface)
- [Diegesis and designing for immersion — Game Developer](https://www.gamedeveloper.com/design/diegesis-and-designing-for-immersion)
- [Diegetic interfaces in game design — Wayline](https://www.wayline.io/blog/diegetic-interfaces-game-design)
- [Menus Don't Pause Gameplay — Giant Bomb](https://giantbomb.com/wiki/Concepts/Menus_Dont_Pause_Gameplay)
- [Dead Space style survival horror template — Summer Engine](https://www.summerengine.com/templates/horror/survival-horror/dead-space-style.md)
- [Partial pausing the game — GameDev.net](https://gamedev.net/forums/topic/698180-menu-ui-partial-pausing-the-game/)
- [Alone in the Dark makes terrifying use of its inventory — DreadXP](https://www.dreadxp.com/?p=25903)
- [Why so many games avoid working mirrors — Quora](https://www.quora.com/Why-do-so-many-video-games-have-an-aversion-to-using-working-mirrors-in-their-environments)
- [Why reflections are one of the best tricks in video games — How-To Geek](https://www.howtogeek.com/why-reflections-are-one-of-the-best-tricks-in-video-games-and-how-thats-changing/)
- [Thief series — Wikipedia](https://en.wikipedia.org/wiki/Thief_(series))
- [Why stealth games turned light into the enemy — Aside House](https://asidehouse.com/respawn/why-stealth-games-turned-light-into-the-enemy/)
