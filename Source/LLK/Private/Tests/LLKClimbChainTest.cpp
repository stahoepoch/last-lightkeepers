// Getting a child onto a counter, in a world, with something listening.
//
// `LLKClimbTest` proves the rules: a counter sorts climbable and a shelf does
// not, the ceiling is the eye, the mantle rises before it advances. All of it
// is true of a game where the trace never finds a surface - and the trace is
// the whole risky part, because it is the piece that touches geometry.
//
// **What only a world can answer:** whether a downward probe in front of the
// keeper finds a counter's top at all, whether the room check above it
// behaves, whether the mantle actually moves the capsule when the component
// ticks, and whether a machine across the aisle hears the scramble. That last
// one matters most: a mantle moves the capsule with `SetActorLocation`, so the
// keeper's velocity is zero throughout, and without
// `ULLKClimbComponent::GetCurrentNoise` the most helpless second in the
// chapter would be the quietest thing in it.

#include "Characters/LLKCharacter.h"
#include "Characters/LLKClimbComponent.h"
#include "Characters/LLKClimbFeel.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/LLKEnemyDef.h"
#include "Enemies/LLKNoiseSource.h"
#include "Enemies/LLKSweeper.h"
#include "Harness/LLKStoreFloorPlan.h"
#include "Lantern/LLKLanternFeel.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"
#include "Tests/LLKWorldFixture.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKClimbChainFixture
{
	/**
	 * A solid block with its top face at `TopCm`, centred `ForwardCm` ahead.
	 *
	 * Built from a `UBoxComponent` at runtime rather than a mesh actor,
	 * because a mesh needs an asset and this test must not depend on one
	 * being imported - the import boundary is where this project's assets go
	 * missing, and a test that fails when an asset moves is a test that
	 * reports the wrong thing.
	 */
	AActor* SpawnBlock(
		const LLKWorldFixture::FScopedWorld& World, const float ForwardCm, const float TopCm,
		const float HalfDepthCm = 50.f)
	{
		AActor* Block = World.Spawn<AActor>(FVector(ForwardCm, 0.f, TopCm * 0.5f));
		if (Block == nullptr)
		{
			return nullptr;
		}

		UBoxComponent* Box = NewObject<UBoxComponent>(Block, TEXT("Solid"));
		Block->SetRootComponent(Box);
		Box->RegisterComponent();
		Box->SetBoxExtent(FVector(HalfDepthCm, 50.f, TopCm * 0.5f));
		Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Box->SetCollisionObjectType(ECC_WorldStatic);
		Box->SetCollisionResponseToAllChannels(ECR_Block);
		Block->SetActorLocation(FVector(ForwardCm, 0.f, TopCm * 0.5f));
		return Block;
	}

	FLLKEnemyDef Sweeper()
	{
		FLLKEnemyDef Def;
		Def.Family = ELLKEnemyFamily::Sweepers;
		Def.MoveSpeed = 6.f;
		Def.AudioTelegraphSeconds = 1.f;
		Def.AggroRadiusDark = 12.f;
		Def.AggroRadiusLit = 38.f;
		return Def;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKClimbChainTest, "LLK.World.ClimbChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKClimbChainTest::RunTest(const FString& Parameters)
{
	const FLLKStoreFloorPlan Plan;
	const float CounterCm = LLKUnits::DistanceFromStuds(Plan.CounterHeightStuds);
	const float ShelfCm = LLKUnits::DistanceFromStuds(Plan.ShelfHeightStuds);
	const float HalfHeight = LLKLanternFeel::KeeperCapsuleHalfHeightCm;

	// ---- 1. A counter, and a child who gets onto it ---------------------
	{
		LLKWorldFixture::FScopedWorld World;
		if (!TestTrue(TEXT("a game world could be created"), World.IsValid()))
		{
			return false;
		}

		AActor* Counter = LLKClimbChainFixture::SpawnBlock(World, 80.f, CounterCm);
		ALLKCharacter* Keeper = World.Spawn<ALLKCharacter>(FVector(0.f, 0.f, HalfHeight));

		if (!TestNotNull(TEXT("the counter spawned"), Counter)
			|| !TestNotNull(TEXT("the keeper spawned"), Keeper))
		{
			return false;
		}

		Keeper->SetActorRotation(FRotator::ZeroRotator);

		ULLKClimbComponent* Climb = Keeper->FindComponentByClass<ULLKClimbComponent>();
		if (!TestNotNull(TEXT("the keeper has a climb component at all"), Climb))
		{
			return false;
		}

		// The trace, which is the part no pure test can reach.
		const FLLKLedge Ledge = Climb->FindLedge();
		TestTrue(TEXT("the probe finds the counter in front of the keeper"), Ledge.bFound);
		TestEqual(TEXT("and measures its height as the authored counter height"),
			Ledge.HeightCm, CounterCm, 2.f);

		TestTrue(TEXT("and the climb starts"), Climb->TryClimb());
		TestTrue(TEXT("and reports that it is climbing"), Climb->IsClimbing());

		// **Audible while it happens.** The capsule is being moved directly,
		// so velocity is zero and this is the only thing making a sound.
		LLKHearing::FLLKNoise Noise;
		TestTrue(TEXT("and is making a noise while it does"),
			Climb->GetCurrentNoise(Noise));
		TestEqual(TEXT("at the authored climb carry"),
			Noise.RadiusStuds, LLKClimbFeel::ClimbNoiseStuds, 0.001f);

		// Long enough for any ledge under the ceiling.
		World.StepFor(2.f);

		TestFalse(TEXT("the climb finishes rather than running forever"),
			Climb->IsClimbing());
		TestTrue(TEXT("and the keeper is standing on top of the counter"),
			FMath::Abs(Keeper->GetActorLocation().Z - (CounterCm + HalfHeight)) < 5.f);
		TestTrue(TEXT("and has moved forward onto it, not just upward"),
			Keeper->GetActorLocation().X > 20.f);

		TestFalse(TEXT("and goes quiet once it is over"),
			Climb->GetCurrentNoise(Noise));
	}

	// ---- 2. A shelf, which is the whole reason push exists --------------
	//
	// Asserted in a world and not just in the arithmetic, because the trace
	// is what would quietly disagree: a probe that found the shelf's top and
	// a height gate that refused it are two different behaviours, and only
	// one of them is correct.
	{
		LLKWorldFixture::FScopedWorld World;
		if (!TestTrue(TEXT("a second world could be created"), World.IsValid()))
		{
			return false;
		}

		LLKClimbChainFixture::SpawnBlock(World, 80.f, ShelfCm);
		ALLKCharacter* Keeper = World.Spawn<ALLKCharacter>(FVector(0.f, 0.f, HalfHeight));
		if (!TestNotNull(TEXT("the keeper spawned"), Keeper))
		{
			return false;
		}
		Keeper->SetActorRotation(FRotator::ZeroRotator);

		ULLKClimbComponent* Climb = Keeper->FindComponentByClass<ULLKClimbComponent>();
		if (!TestNotNull(TEXT("the keeper has a climb component"), Climb))
		{
			return false;
		}

		TestFalse(TEXT("a shelf at 168 cm offers no ledge to a 145 cm child"),
			Climb->FindLedge().bFound);
		TestFalse(TEXT("so the climb refuses, and jump happens instead"),
			Climb->TryClimb());

		const float Before = Keeper->GetActorLocation().Z;
		World.StepFor(1.f);
		TestEqual(TEXT("and the keeper has not been lifted anywhere"),
			static_cast<float>(Keeper->GetActorLocation().Z), Before, 5.f);
	}

	// ---- 3. Open floor is not a ledge -----------------------------------
	{
		LLKWorldFixture::FScopedWorld World;
		ALLKCharacter* Keeper = World.Spawn<ALLKCharacter>(FVector(0.f, 0.f, HalfHeight));
		if (!TestNotNull(TEXT("the keeper spawned"), Keeper))
		{
			return false;
		}

		ULLKClimbComponent* Climb = Keeper->FindComponentByClass<ULLKClimbComponent>();
		if (!TestNotNull(TEXT("the keeper has a climb component"), Climb))
		{
			return false;
		}

		TestFalse(TEXT("with nothing in front, there is no ledge"),
			Climb->FindLedge().bFound);
		TestFalse(TEXT("and jumping in open floor costs no RPC"), Climb->TryClimb());
	}

	// ---- 4. A machine that CANNOT see it still hears it ------------------
	//
	// **This section was wrong twice, and the second version is a design
	// finding rather than a test fix.** It first put the machine six studs
	// away with clear line of sight, and the machine telegraphed instead of
	// searching - because it had SEEN the keeper, which is correct and is
	// what `LLKHearingTest` asserts. Sight outranks sound.
	//
	// Trying to separate the two exposed the real constraint:
	//
	//     climb noise  8 studs  <  Sweeper dark sight  12 studs
	//
	// So a climb can never be heard-but-not-seen in the open - anywhere close
	// enough to hear the scramble is somewhere the machine could already see
	// you. **The climb's sound only matters when sight is blocked**, which is
	// also the only time anybody climbs: behind a counter, out of view.
	// Pushing is the opposite (16 studs against sight's 12) and so is a
	// sprint (12.5), which is why those two are audible commitments in the
	// open and this one is not.
	//
	// That is the right arrangement and it is not an accident worth
	// "fixing" by raising the climb's volume: climbing louder than sprinting
	// would break the vocabulary walk < climb < sprint < push, and a player
	// who hauls themselves onto a counter in full view of a machine has
	// already been seen, so the sound adds nothing.
	//
	// So the test builds the real case: a counter to climb, a wall between,
	// and a machine seven studs off on the far side of it.
	{
		LLKWorldFixture::FScopedWorld World;
		if (!TestTrue(TEXT("a fourth world could be created"), World.IsValid()))
		{
			return false;
		}

		LLKClimbChainFixture::SpawnBlock(World, 80.f, CounterCm);
		// Tall and thin, past the counter: it breaks the line of sight and is
		// never what the climb probe finds, which reaches only 42 cm ahead.
		LLKClimbChainFixture::SpawnBlock(World, 140.f, 250.f, 10.f);

		ALLKCharacter* Keeper = World.Spawn<ALLKCharacter>(FVector(0.f, 0.f, HalfHeight));
		ALLKSweeper* Machine = World.Spawn<ALLKSweeper>(FVector(215.f, 0.f, HalfHeight));

		if (!TestNotNull(TEXT("the keeper spawned"), Keeper)
			|| !TestNotNull(TEXT("the Sweeper spawned"), Machine))
		{
			return false;
		}

		Keeper->SetActorRotation(FRotator::ZeroRotator);
		Machine->SetDefForTest(LLKClimbChainFixture::Sweeper());

		FLLKSweeperMind Patrolling;
		Patrolling.State = ELLKSweeperState::Patrol;
		Machine->SetMindForTest(Patrolling);

		ULLKClimbComponent* Climb = Keeper->FindComponentByClass<ULLKClimbComponent>();
		if (!TestNotNull(TEXT("the keeper has a climb component"), Climb))
		{
			return false;
		}

		// 215 cm is 7.68 studs: inside the climb's 8-stud carry, and inside
		// the 12-stud dark sight it would be seen by if the wall were not
		// there. Both halves asserted, so neither can drift unnoticed.
		const float SeparationStuds = 215.f / LLKUnits::StudsToCm;
		TestTrue(TEXT("the machine is close enough to hear a scramble"),
			SeparationStuds <= LLKClimbFeel::ClimbNoiseStuds);
		TestTrue(TEXT("and close enough that only the wall keeps it from seeing"),
			SeparationStuds < 12.f);

		TestTrue(TEXT("the climb starts"), Climb->TryClimb());
		World.StepFor(0.25f);

		TestEqual(TEXT("the Sweeper heard the scramble it could not see, and went to look"),
			Machine->GetMind().State, ELLKSweeperState::Search);
		TestTrue(TEXT("and has a position to look at"), Machine->GetMind().bHasLastKnown);
		TestFalse(TEXT("and did not telegraph, because it has seen nobody"),
			Machine->GetMind().State == ELLKSweeperState::Telegraph);
	}

	return true;
}

#endif
