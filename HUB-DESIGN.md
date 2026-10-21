# Lanternhold as a hub

Chris, after the first real end-to-end run:

> the lanternhold is still pretty dark on its own, and the stairs going up are
> all weird, and the whole place is strange, going to a raid isn't really
> explanatory, a proper hub, roblox style is needed.

He is right, and the reason is worth naming precisely: **Lanternhold was built
as a faithful model of the Johnstown Masonic Temple, not as a hub.** Those are
different jobs. The building is good. Its plan is working against the one thing
a hub has to do.

## What a hub is for

A hub has exactly one required job: get the player into a run, the first time,
without being told. Everything else in it is optional content sitting beside
that path.

Right now the required job is buried. You spawn in a basement, the departure
board is somewhere else in a four-storey building, and nothing points at it.
A player who does not already know the building cannot find the game.

## What the research says

Roblox's own onboarding docs treat the first minutes as the thing to optimise,
and recommend testing guidance styles — a guided arrow versus a dialogue
sequence — against Day 1 retention rather than guessing.[^onboard] A Roblox
staff post on funnel events describes a case where players stalled on a step
and the fix was simply **a small icon over the object they were supposed to
use**.[^funnel] That is the scale of fix this needs: not a tutorial, a marker.

Community practice is consistent on the shape:

- The route into the match should be obvious and appealing — most commonly **a
  pad the player steps onto**, which is the standard for round-based
  games.[^lobby]
- Keep the layout simple so players do not get lost, guide the eye with clear
  paths and open space rather than a maze, and **put shops and extras to the
  sides, not blocking the main route**.[^lobby]
- Teach by doing and put hints in the world — signs, trails, highlighted paths
  — instead of walls of text.[^notext] Directional arrows, beams and world
  targets are the usual tools.[^onboardfw]
- An empty-feeling lobby loses people; give them something to do while they
  wait.[^lobby]

Dead Rails is the closest live comparison: you spawn in a Lobby that exists to
prepare in, and you start a run by **walking into a clearly marked area near
the train tracks**.[^deadrails] The departure is a place you walk to, not a
menu you find.

## The change I would make

Keep the building. Change the plan around the player.

**1. The ground floor is the hub. The other three are content.**
Everything required to play lives on one floor, reachable without stairs: the
departure, the Lamp Wall, the hand-in, the relic plinths. The ballroom, the
Egyptian room, the Gothic chapel and the museum stay exactly as they are and
become things you *choose* to go and look at. This also removes the stairs from
the critical path, which sidesteps "the stairs going up are all weird" for the
player who just wants to play.

**2. Spawn facing the departure.**
The first frame should contain the way out. Not a lobby you orient yourself in
— a lobby where the answer is already on screen.

**3. One departure pad, lit, with a beam.**
A marked pad you stand on, with a `Beam` from the spawn to it for a player's
first session only. This is the arrow the Roblox docs describe testing, and the
icon-over-the-object fix from the funnel post.

**4. The side rooms stay side rooms.**
Lamp Wall, hand-in, plinths and workshop arranged along the spine so they are
visible from the walk to the departure and never block it.

**5. Light it like a refuge.**
Lanternhold being dark is a bug, not atmosphere. The hub should be warm and
legible precisely because the mall is not — the contrast is what makes the
lantern mean something when you leave. Dark hub, dark raid, no contrast.

## Also from this run

Separate from the hub, recorded so they are not lost:

- **Extraction is unsignposted.** "i had no clue where to extract until i got
  to the store." `LootService.beginRun` already picks one active exit; the HUD
  needs to point at it. Same fix class as the departure beam.
- **Loot distance rule.** Chris: higher-tier loot further from the extraction,
  easier loot closer. Worth noting this cannot be baked into static markers,
  because the active exit is chosen per run — tiers have to be assigned at
  `beginRun` relative to the exit that was picked, or relics placed far from
  *both* exits.
- **Escalator clipping on the upper levels** — cannot get through. Map bug,
  mine.
- **No loot on extraction** despite a successful run. Unverified; may be that
  nothing was picked up.
- **Sweepers spawned inside visible boxes** — fixed, the spawn markers are now
  invisible.

[^onboard]: [Onboarding — Roblox Creator Docs](https://create.roblox.com/docs/production/game-design/onboarding)
[^funnel]: [Improving Onboarding through Funnel Events — Roblox DevForum](https://devforum.roblox.com/t/improving-onboarding-through-funnel-events/3064458)
[^lobby]: [Roblox Lobby System — spawn area, queue, teleport to match](https://picoo.io/how-to-make/roblox-lobby-system)
[^notext]: [Onboarding without a wall of text](https://www.kidsgames.top/en/blog/onboarding-without-a-wall-of-text)
[^onboardfw]: [OnBoard — onboarding & tutorial framework](https://devforum.roblox.com/t/release-onboard-%E2%80%94-modern-lightweight-onboarding-tutorial-framework/4751478)
[^deadrails]: [Roblox Dead Rails guide — Sportskeeda](https://www.sportskeeda.com/roblox-news/roblox-the-takeover-dead-rails-guide)
