// A real `UWorld` for a test, created and torn down around one claim.
//
// **Every automation test in this project until now was pure or a CDO read.**
// `LLKSweeperBrainTest` asserts decisions with no world, `LLKPushTest` asserts
// arithmetic, `LLKSweeperCollisionTest` reads the class default object. That
// split has been deliberate and it has been right: a rule you can assert
// without a level is a rule you can assert in two milliseconds, forever.
//
// It also leaves one whole category unprovable, and that category is exactly
// where this project's defects live. A pawn that never finds the actor beside
// it, an iterator that skips an interface it should match, a component that is
// never ticked, an actor spawned into a world where nothing looks for it -
// none of these are visible to a pure test, and all of them produce a clean
// log. **The recurring failure shape is a correct system over an absent thing,
// and "absent" is a property of a world.**
//
// So: a minimal game world, spawned actors, and a real tick. Not a level -
// there is no geometry, no navmesh and no player controller, and anything
// needing those belongs in the `LLK.CheckMap` gate instead. This is for the
// seam between two objects that both already have their own pure tests.
//
// Kept to one small header because the first one of these in a codebase
// decides how the next ten look.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKWorldFixture
{
	/**
	 * A game world that cleans itself up.
	 *
	 * Destruction matters more than creation here: a leaked world keeps every
	 * actor in it alive, and the next test in the run would then iterate
	 * actors it never spawned. That failure presents as one test breaking
	 * because an unrelated one ran first, which is the worst kind to chase.
	 */
	class FScopedWorld
	{
	public:
		FScopedWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World == nullptr)
			{
				return;
			}

			Context = &GEngine->CreateNewWorldContext(EWorldType::Game);
			Context->SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}

		~FScopedWorld()
		{
			if (World == nullptr)
			{
				return;
			}

			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		FScopedWorld(const FScopedWorld&) = delete;
		FScopedWorld& operator=(const FScopedWorld&) = delete;

		UWorld* Get() const { return World; }
		bool IsValid() const { return World != nullptr; }

		/**
		 * One step, by calling the actors' own `Tick` directly.
		 *
		 * **Not `UWorld::Tick`, and the first version of this used it.** A
		 * world built this way never begins play - `HasBegunPlay()` is false
		 * and every spawned actor comes back with ticking disabled - so the
		 * engine's tick scheduler runs nothing and a test asserting that an
		 * actor moved fails for a reason that has nothing to do with the
		 * actor. Standing that machinery up needs a game instance, world
		 * settings and a match start, which is a large amount of engine
		 * ceremony to prove something about two of our own classes.
		 *
		 * So this drives the real `Tick` bodies on the real actors in a real
		 * world, in spawn order. What it proves: our Tick code, our actor
		 * iteration, our interface casts, our state machines. What it does
		 * NOT prove: that UE will schedule those ticks, their relative order
		 * under tick groups, or anything about dependencies - all engine
		 * behaviour, none of it ours, and the `LLK.CheckMap` boot gate
		 * exercises it on a real level anyway.
		 */
		void Step(const float DeltaSeconds = 1.f / 60.f) const
		{
			if (World == nullptr)
			{
				return;
			}

			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (AActor* Actor = *It)
				{
					if (Actor->PrimaryActorTick.bCanEverTick)
					{
						Actor->Tick(DeltaSeconds);
					}
				}
			}
		}

		/** Advance for a duration, in fixed steps. Returns steps taken. */
		int32 StepFor(const float Seconds, const float DeltaSeconds = 1.f / 60.f) const
		{
			const int32 Steps = FMath::Max(0, FMath::RoundToInt(Seconds / DeltaSeconds));
			for (int32 Taken = 0; Taken < Steps; ++Taken)
			{
				Step(DeltaSeconds);
			}
			return Steps;
		}

		/**
		 * Spawn an actor and begin its play.
		 *
		 * `DispatchBeginPlay` is explicit here for the same reason `Step` is:
		 * the world never begins play on its own, so a spawned actor would
		 * otherwise never run `BeginPlay` - and for a pushable that is where
		 * the rail origin and axis are captured, so it would slide from
		 * wherever the last caller left it.
		 */
		template <typename T>
		T* Spawn(const FVector& Where, const FRotator& Facing = FRotator::ZeroRotator) const
		{
			if (World == nullptr)
			{
				return nullptr;
			}

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			T* Actor = World->SpawnActor<T>(T::StaticClass(), Where, Facing, Params);
			if (Actor != nullptr)
			{
				Actor->DispatchBeginPlay();
			}
			return Actor;
		}

	private:
		UWorld* World = nullptr;
		FWorldContext* Context = nullptr;
	};
}

#endif
