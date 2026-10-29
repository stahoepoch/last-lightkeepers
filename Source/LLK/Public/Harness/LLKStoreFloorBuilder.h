// Builds the ground floor from engine primitives, at runtime.
//
// Nothing here writes to `Content/`. Every piece is `/Engine/BasicShapes/Cube`
// scaled and placed, spawned into the running world, gone when the level
// ends - the same discipline the twelve-pillar arena followed, at the scale of
// a department store floor.
//
// **Read `LLKStoreFloorPlan.h` first.** It holds every dimension, in studs,
// with the reasoning attached. This file is only the assembly.
//
// The layout, from the street inwards:
//
//   1. **Sealed street front.** You spawn just inside it, facing in. The
//      doors do not open - `THE-STORE.md` has the street entrances sealed
//      with the water still against them - so the entrance reads as where you
//      came from rather than as an exit, which is what makes extraction a
//      decision later.
//   2. **Cosmetics.** A grid of glass counters. Short sightlines, baseline
//      loot on top, and the first thing a keeper sees is that there are
//      things here and they can be carried. Lesson one of the ground floor.
//   3. **The mirrored columns.** Mid-concourse, and they are not decoration:
//      they break the one long sightline into two legal ones, and they are
//      how a lit keeper sees their own light arrive somewhere they are not.
//      That is lesson two's second channel.
//   4. **Jewellery.** Locked glass cases, deeper in, relic-dense. Visible and
//      unreadable until lit (row 53).
//   5. **The escalator well.** The landmark, and the inclined transition. A
//      dead escalator climbing to a mezzanine stub you can be seen from.
//   6. **The service route.** Behind an internal wall, two doors, jogging
//      halfway so no straight exceeds the sightline rule. The escape.

#pragma once

#include "CoreMinimal.h"
#include "Harness/LLKStoreFloorPlan.h"

class UStaticMesh;
class UWorld;

namespace LLKStoreFloor
{
	/** What the builder produced, so the GameMode and the tests can check it. */
	struct FBuildResult
	{
		/** Just inside the sealed street doors, facing into the shop. */
		FVector SpawnPoint = FVector::ZeroVector;

		/** Where extraction will be. The way you came in is the way out. */
		FVector ExtractionPoint = FVector::ZeroVector;

		/**
		 * Where M2.6's first enemy can start.
		 *
		 * Authored positions rather than generated ones, per row 63's
		 * reasoning about climb points: a machine that appears anywhere is a
		 * machine the player cannot learn. These are the service route, the
		 * back of jewellery, and the mezzanine - all places a keeper arrives
		 * at rather than starts in.
		 */
		TArray<FVector> EnemySpawnPoints;

		/** Somewhere a downed keeper can plausibly fall and be reached. */
		TArray<FVector> RescueTestPoints;

		int32 Blocks = 0;
		int32 Relics = 0;
		int32 BaselineLoot = 0;
	};

	/**
	 * Spawns the whole floor. Server-side; call once, before login.
	 *
	 * `Cube` may be null - every use is null-checked and the floor degrades to
	 * invisible-but-solid, which is the rule the first harness set and the
	 * reason a missing engine asset never stops Play working.
	 */
	LLK_API FBuildResult BuildGroundFloor(
		UWorld* World, UStaticMesh* Cube, const FVector& Origin, const FLLKStoreFloorPlan& Plan);
}
