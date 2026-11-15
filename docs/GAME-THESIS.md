# Last Lightkeepers — current game thesis

Status: **working product direction, 2026-11-14.** Chris's latest direction is a
solo **horror RPG with stealth and environmental puzzles**. This page describes
the game to build. The October co-op extraction thesis is preserved at
`docs/archive/GAME-THESIS-RAID-2026-10.md`; it is history, not an instruction.
Unresolved story and mechanic details remain unresolved here.

## The experience

You are a young Lightkeeper separated from your people after a mission goes
wrong. You move through the Galleria and the Valley to find your family, learn
what the machines are doing, and find a way home. The journey moves forward;
the story, the people you find, and the places you change give the player a
reason to keep going. Lanternhold matters because people depend on it, not
because every chapter ends with a loot payout there.

The fantasy is physical: walk through a believable department store, read its
departments and routes, duck behind a counter, hear a machine before you see
it, use the lantern to discover or operate something important, and escape
through a route you understood. The store must feel inhabited, full-scale and
specific even when it is abandoned. Good assets, materials, animation, sound
and lighting are part of this promise, not a reward deferred until after a
greybox-only game has been proven.

The game is frightening, tense and sometimes tragic, with room for amazement
and hope. Fear comes from intelligible danger and the cost of acting under it.
The player keeps control while moving, hiding, solving, and fleeing. A machine
must not turn a corner or a narrow aisle into an unavoidable pin.

## What the genres mean here

- **Horror:** sound, visibility, uncertain routes, and machines doing legible
  but inhuman work. Darkness must preserve navigation; it must not be a black
  screen. Threat cues give the player something to understand and answer.
- **Stealth:** cover, crouching, light, movement and noise change what machines
  can notice. Hiding has a purpose, but a discovered player can still flee.
- **Puzzle:** learn how a real place works, find a clue or tool, restore or
  redirect part of its infrastructure, and open a route. The lantern may read
  detail and present light to authored readers; it is not a gun against robots.
- **RPG:** people, discoveries, choices and lasting changes in the world carry
  progression. The exact character systems are still open. Calling this an RPG
  does not authorize combat stats, loot tiers or a quest-marker checklist.

## The lantern's job

The lantern is one issued instrument with multiple physical ways to use its
light. It must make a meaningful difference to reading, interacting, route
finding or risk. Basic movement through the store cannot require leaving it
on constantly; a player must be able to see the floor, exits and cover under
credible emergency light. The lantern reveals information or enables an
action that ambient light does not. Using it also makes the keeper more
noticeable, and the machine's response must show that cost without a debug HUD.
Visual light and gameplay detection remain separate so art can be tuned
without silently changing enemy balance.

## Story and structure

The opening playable crossing begins in Boswell's and carries the player across
the entire overgrown, scavenged, dangerous Galleria Mall to Sayers. Boswell's
should be a real two-floor department store with departments, service routes
and safe ways around a machine; the concourse and intervening stores must be
real traversal territory, not an empty connector. The first chapter needs a
beginning, discovery, decision and destination, not just collectibles in a
room. Storms change conditions and the place; they are not a raid timer that
ejects the player.

The family mission, the fate of the crew, the role of Lanternhold and the final
action of the game need an authored story outline and play proof. Do not turn
the proposed answers in `DECISIONS.md` rows 84–91 into implementation facts by
repeating them here. The player's age, camp-failure cost, chapter return rules
and ending mechanism remain open at their current decision status.

## The first proof

A fresh player can enter Boswell's, understand where they are, cross the whole
mall through overgrowth, scavenged routes, cover and danger, use the lantern,
survive a machine without being pinned, discover why they are going to Sayers,
and reach the chapter exit. They should want to continue. This must be observed
in a playable Unreal build, with art and sound good enough to test the intended
experience. A green data test, a render of an empty aisle, or a Playground game
cannot substitute for it.

The old extraction, co-op and Roblox implementations are available as
reference. They do not define the current product's acceptance test.
