// The seam: a fixture moves, and a machine across the aisle turns around.
//
// **This is the test the last two commits could not write.** `LLKPushTest`
// proves the push arithmetic, `LLKHearingTest` proves the brain's rules, and
// both build their inputs by hand. Neither can see whether a real
// `ALLKPushableFixture` spawned into a real world is ever *found* by
// `ALLKSweeper::Observe` - whether the actor iterator matches the interface,
// whether `GetCurrentNoise` is reached, whether the salience comparison picks
// it, and whether the brain acts on the result.
//
// Each of those is one line of plumbing, each is correct in isolation, and
// every one of them fails silently. `docs/FINDINGS-THE-STORE-IS-ALREADY-A-
// PUZZLE.md` lists this exact chain as the thing nothing had run end to end.
//
// **No keeper, deliberately.** `Observe` scans noise sources *before* its
// keeper early-returns, and this test is what proves that ordering is real
// rather than intended: a fixture grinding in an empty building is a sound,
// and row 47's machine does not care whether anybody is there to have made it.
// It also keeps the test free of a pawn, a controller and a navmesh, none of
// which this claim is about.

#include "Data/LLKEnemyDef.h"
#include "GameFramework/DefaultPawn.h"
#include "Enemies/LLKHearing.h"
#include "Enemies/LLKNoiseSource.h"
#include "Enemies/LLKSweeper.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"
#include "Tests/LLKWorldFixture.h"
#include "World/LLKPushFeel.h"
#include "World/LLKPushableFixture.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKPushChainFixture
{
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
	FLLKPushChainTest, "LLK.World.PushChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKPushChainTest::RunTest(const FString& Parameters)
{
	LLKWorldFixture::FScopedWorld World;
	if (!TestTrue(TEXT("a game world could be created"), World.IsValid()))
	{
		return false;
	}

	// Eight studs apart: inside the fixture's 16-stud carry, outside the
	// Sweeper's 12-stud dark notice radius. So the machine can hear it and
	// could not have seen anything even if a keeper were standing there.
	constexpr float SeparationStuds = 8.f;

	ALLKPushableFixture* Fixture = World.Spawn<ALLKPushableFixture>(FVector::ZeroVector);
	ALLKSweeper* Machine = World.Spawn<ALLKSweeper>(
		FVector(LLKUnits::DistanceFromStuds(SeparationStuds), 0.f, 0.f));

	if (!TestNotNull(TEXT("the fixture spawned"), Fixture)
		|| !TestNotNull(TEXT("the Sweeper spawned"), Machine))
	{
		return false;
	}

	Machine->SetDefForTest(LLKPushChainFixture::Sweeper());

	FLLKSweeperMind Patrolling;
	Patrolling.State = ELLKSweeperState::Patrol;
	Machine->SetMindForTest(Patrolling);

	// ---- 1. A fixture nobody is pushing is silent -----------------------
	//
	// Asserted first and asserted at all, because every claim below would
	// also pass if the fixture simply screamed continuously.
	{
		LLKHearing::FLLKNoise Noise;
		TestFalse(TEXT("an untouched fixture reports no noise"),
			Fixture->GetCurrentNoise(Noise));

		World.StepFor(0.5f);
		TestEqual(TEXT("and a patrolling Sweeper beside it stays on patrol"),
			Machine->GetMind().State, ELLKSweeperState::Patrol);
		TestEqual(TEXT("and the fixture has not moved itself"),
			Fixture->GetTravelStuds(), 0.f, 0.001f);
	}

	// ---- 2. Somebody pushes it ------------------------------------------
	//
	// Through the interface, as the interactor does, rather than by setting
	// travel directly - the point is the real path.
	{
		// **`ADefaultPawn` and not a bare `APawn`.** A plain APawn has no
		// root component, so `SetActorLocation` on one silently does nothing
		// and it stays at the origin - which cost a run of this test, and is
		// a good illustration of why the world layer is worth having: that
		// failure is invisible to every pure test ever written.
		ADefaultPawn* Pusher = World.Spawn<ADefaultPawn>(FVector(-80.f, 0.f, 0.f));
		if (!TestNotNull(TEXT("a pusher spawned"), Pusher))
		{
			return false;
		}

		// **Standing where a child would stand**, 80 cm back along the rail
		// and facing down it. Not on top of the fixture: the first version of
		// this test put the pawn at the fixture's own location, which made
		// the push point straight down from the eye and failed the aim gate.
		// That failure was the useful one - it was the fixture being focusable
		// only from directly overhead, and it is fixed in the actor rather
		// than worked around here.
		//
		// At 80 cm back, eye 125 cm up and hands on the fixture at 70 cm, the
		// keeper is looking 34 degrees down, inside the 41 the gate allows.
		Pusher->SetActorRotation(FRotator::ZeroRotator);
		TestTrue(TEXT("the pusher is actually standing where it was put"),
			FVector::Dist(Pusher->GetActorLocation(), FVector(-80.f, 0.f, 0.f)) < 1.f);

		TestTrue(TEXT("the fixture will accept this keeper's focus"),
			Fixture->CanFocus(Pusher));

		Fixture->BeginInteract(Pusher);

		World.StepFor(1.f);

		TestTrue(TEXT("one second of pushing moves it about one second's worth"),
			FMath::IsNearlyEqual(
				Fixture->GetTravelStuds(), LLKPushFeel::PushSpeedStuds, 0.1f));
		TestTrue(TEXT("and while it moves it is making a noise"),
			Fixture->IsMoving());

		LLKHearing::FLLKNoise Noise;
		TestTrue(TEXT("and reports that noise through ILLKNoiseSource"),
			Fixture->GetCurrentNoise(Noise));
		TestEqual(TEXT("at the authored carry"),
			Noise.RadiusStuds, LLKPushFeel::PushNoiseStuds, 0.001f);
	}

	// ---- 3. The machine across the aisle heard it -----------------------
	//
	// THE claim. Everything above is true of a world in which `Observe` never
	// looks at a noise source.
	{
		TestEqual(TEXT("the Sweeper left patrol to investigate the sound"),
			Machine->GetMind().State, ELLKSweeperState::Search);
		TestTrue(TEXT("and now has somewhere to look"),
			Machine->GetMind().bHasLastKnown);

		// **It goes to where the sound WAS, not where the fixture got to.**
		// This assertion was wrong first and the code was right: it checked
		// against the fixture's live position and failed by about a stud.
		// That is `LLKHearing`'s rule working - a sound that arrives
		// mid-search does not move the target - so the machine is walking to
		// the spot where the grinding started while the fixture has gone on
		// travelling. A machine that tracked the live position would be
		// using hearing as a tracking sense, which is the thing the design
		// refuses outright.
		const FVector LookingAt = Machine->GetMind().LastKnownKeeperPosition;

		TestTrue(TEXT("it is looking at where the sound started, not where the fixture is now"),
			FVector::Dist(LookingAt, FVector::ZeroVector) < LLKUnits::DistanceFromStuds(0.5f));
		TestTrue(TEXT("and the fixture has meanwhile moved away from that spot"),
			FVector::Dist(Fixture->GetActorLocation(), LookingAt)
				> LLKUnits::DistanceFromStuds(0.5f));
	}

	// ---- 4. Letting go stops it -----------------------------------------
	//
	// The runaway-object case, and the one with the worst failure mode: a
	// fixture that kept moving would keep broadcasting, and the machine would
	// never return to its route.
	{
		Fixture->EndInteract(nullptr);
		const float Where = Fixture->GetTravelStuds();

		World.StepFor(1.f);

		TestEqual(TEXT("a released fixture stops where it was let go"),
			Fixture->GetTravelStuds(), Where, 0.001f);
		TestFalse(TEXT("and goes quiet"), Fixture->IsMoving());

		LLKHearing::FLLKNoise Noise;
		TestFalse(TEXT("and reports no noise to anything listening"),
			Fixture->GetCurrentNoise(Noise));
	}

	// ---- 5. And the machine gives up ------------------------------------
	//
	// Bounded, which is what slice 3 demands. A sound that could hold a
	// machine's attention forever is harassment with extra steps.
	{
		// Comfortably past the fallback search interval of 3 s.
		World.StepFor(6.f);

		TestEqual(TEXT("with nothing more to hear, the Sweeper returns to its route"),
			Machine->GetMind().State, ELLKSweeperState::Patrol);
		TestFalse(TEXT("and forgets where it was looking"),
			Machine->GetMind().bHasLastKnown);
	}

	return true;
}

#endif
