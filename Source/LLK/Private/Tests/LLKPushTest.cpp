// Pushing a fixture, and which sound a machine goes to look at.
//
// The first verb in this game that changes the building, and the first one
// whose cost is measured in being heard rather than in being seen. Both halves
// are arithmetic and both are asserted here with no world, no pawn and no
// level - the same split `LLKLanternFeel` and `LLKSweeperPresence` use.
//
// The rules worth breaking the build over:
//
//   1. The rail is finite at both ends. A fixture that could be pushed past
//      its authored limit is a fixture a player can wedge in a doorway, and
//      a chapter soft-locked by its own furniture is worse than an empty one.
//   2. Facing is a gate, not a multiplier. Scaling speed by how squarely you
//      face a thing reads as the push being broken rather than as you being
//      off-axis.
//   3. Pushing is louder than sprinting and quieter than being seen in the
//      light. That ordering is the whole economy of the verb.
//   4. A deliberate loud thing beats your own footsteps. Without this a
//      distraction can never work, because the person making the distraction
//      is standing next to it.

#include "Enemies/LLKHearing.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"
#include "World/LLKPushFeel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKPushTest, "LLK.World.Push",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKPushTest::RunTest(const FString& Parameters)
{
	constexpr float Travel = 8.f;

	// ---- 1. The rail is finite at both ends -----------------------------
	{
		// Shove it forward for far longer than the rail is long.
		float At = 0.f;
		for (int32 Frame = 0; Frame < 2400; ++Frame)
		{
			At = LLKPushFeel::AdvancePush(At, 1.f, 1.f / 60.f, Travel);
		}
		TestEqual(TEXT("a fixture stops at the end of its authored rail"),
			At, Travel, 0.001f);

		// And back the other way, for as long again.
		for (int32 Frame = 0; Frame < 2400; ++Frame)
		{
			At = LLKPushFeel::AdvancePush(At, -1.f, 1.f / 60.f, Travel);
		}
		TestEqual(TEXT("and at the start when pushed back"), At, 0.f, 0.001f);

		// A zero-length rail is an authoring mistake, not a crash.
		TestEqual(TEXT("a zero-length rail simply never moves"),
			LLKPushFeel::AdvancePush(0.f, 1.f, 1.f, 0.f), 0.f);
	}

	// ---- 2. Facing is a gate, not a multiplier --------------------------
	{
		const float Square = LLKPushFeel::AdvancePush(0.f, 1.f, 1.f, Travel);
		const float Angled = LLKPushFeel::AdvancePush(0.f, 0.6f, 1.f, Travel);
		const float Sideways = LLKPushFeel::AdvancePush(0.f, 0.2f, 1.f, Travel);

		TestEqual(TEXT("a square push moves at the authored speed"),
			Square, LLKPushFeel::PushSpeedStuds, 0.001f);
		TestEqual(TEXT("and an off-axis push past the gate moves just as fast"),
			Angled, Square, 0.001f);
		TestEqual(TEXT("while one short of the gate does not move at all"),
			Sideways, 0.f);
	}

	// ---- 3. It is frame-rate independent --------------------------------
	//
	// A fixture that travels further on a fast machine is a puzzle with a
	// different answer per computer.
	{
		float Slow = 0.f;
		for (int32 Frame = 0; Frame < 4; ++Frame)
		{
			Slow = LLKPushFeel::AdvancePush(Slow, 1.f, 0.5f, Travel);
		}

		float Fast = 0.f;
		for (int32 Frame = 0; Frame < 120; ++Frame)
		{
			Fast = LLKPushFeel::AdvancePush(Fast, 1.f, 1.f / 60.f, Travel);
		}

		TestEqual(TEXT("two seconds of pushing is two seconds at any frame rate"),
			Slow, Fast, 0.001f);
		TestEqual(TEXT("and two seconds is two seconds of authored speed"),
			Slow, LLKPushFeel::PushSpeedStuds * 2.f, 0.001f);
	}

	// ---- 4. It is slow enough to be a decision --------------------------
	{
		// Against the keeper's own crouch, which is the slowest they move,
		// and against the Sweeper's 6 studs/s from enemies.json.
		TestTrue(TEXT("pushing is slower than crouching"),
			LLKPushFeel::PushSpeedStuds < 3.f);
		TestTrue(TEXT("and a Sweeper covers at least four studs for every one it moves"),
			6.f / LLKPushFeel::PushSpeedStuds >= 4.f);
	}

	// ---- 5. The noise economy -------------------------------------------
	{
		// A sprint is 12.5 studs by LLKHearing's one-second-of-travel rule;
		// the Sweeper's lit sight is 38 in enemies.json.
		TestTrue(TEXT("pushing is louder than sprinting"),
			LLKPushFeel::PushNoiseStuds > 12.5f);
		TestTrue(TEXT("and still quieter than being seen in the light"),
			LLKPushFeel::PushNoiseStuds < 38.f);
	}

	// ---- 6. What salience actually ranks, measured rather than assumed --
	//
	// **This section is written the way it is because the first version of it
	// failed, and the claim was wrong rather than the rule.** It asserted
	// that a fixture beats footsteps outright, with the keeper sprinting two
	// studs from the machine and the fixture nine studs away. Excess reach
	// makes that 12.5 - 2 = 10.5 for the sprint against 16 - 9 = 7 for the
	// fixture, so the sprint wins - and it SHOULD. A keeper running past a
	// machine's shoulder is the more urgent thing in the room, and a rule
	// that sent it to look at furniture instead would be a worse rule.
	//
	// So the true claim is narrower and is the one that actually matters:
	// **a fixture outranks the noise of the person pushing it.** That is what
	// a distraction needs, because the pusher is standing next to the thing.
	{
		const FVector Ear = FVector::ZeroVector;
		const FVector AtFixture(LLKUnits::DistanceFromStuds(9.f), 0.f, 0.f);

		// The pusher is at the fixture - that is what pushing means - and is
		// moving, because they are shoving it.
		const LLKHearing::FLLKNoise PusherSteps =
			LLKHearing::NoiseFromMovement(AtFixture, LLKUnits::SpeedFromStuds(5.f));
		const LLKHearing::FLLKNoise Fixture{ AtFixture, LLKPushFeel::PushNoiseStuds };

		TestTrue(TEXT("the fixture outranks the steps of whoever is pushing it"),
			LLKPushFeel::Salience(Fixture, Ear) > LLKPushFeel::Salience(PusherSteps, Ear));

		// And the distraction case proper: the keeper is hiding somewhere
		// else, barely moving, while the fixture grinds.
		const LLKHearing::FLLKNoise Creeping =
			LLKHearing::NoiseFromMovement(
				FVector(LLKUnits::DistanceFromStuds(2.f), 0.f, 0.f),
				LLKUnits::SpeedFromStuds(3.f));

		TestTrue(TEXT("and outranks a crouching keeper much closer to the machine"),
			LLKPushFeel::Salience(Fixture, Ear) > LLKPushFeel::Salience(Creeping, Ear));

		// The counter-case, asserted so nobody later "fixes" it: a sprint at
		// the machine's shoulder is meant to win.
		const LLKHearing::FLLKNoise Bolting =
			LLKHearing::NoiseFromMovement(
				FVector(LLKUnits::DistanceFromStuds(2.f), 0.f, 0.f),
				LLKUnits::SpeedFromStuds(12.5f));

		TestTrue(TEXT("but a sprint past the machine still beats distant furniture"),
			LLKPushFeel::Salience(Bolting, Ear) > LLKPushFeel::Salience(Fixture, Ear));
	}

	// ---- 7. ...but not from anywhere ------------------------------------
	//
	// The counterweight. If a fixture outranked everything at any range the
	// machine would be permanently fixated on furniture, so the reach limit
	// has to still be the thing that decides.
	{
		const FVector Ear = FVector::ZeroVector;
		const LLKHearing::FLLKNoise TooFar{
			FVector(LLKUnits::DistanceFromStuds(LLKPushFeel::PushNoiseStuds + 2.f), 0.f, 0.f),
			LLKPushFeel::PushNoiseStuds };

		TestFalse(TEXT("a fixture past its own carry is not heard at all"),
			LLKHearing::NoiseReaching(TooFar, Ear));
		TestTrue(TEXT("and its salience is negative, so it loses to anything audible"),
			LLKPushFeel::Salience(TooFar, Ear) < 0.f);
	}

	// ---- 8. Moving is moving --------------------------------------------
	//
	// `IsMoving` decides whether the fixture makes a sound at all, so a
	// fixture pinned at the end of its rail must report silence rather than
	// grinding forever against the stop.
	{
		TestFalse(TEXT("a fixture at its limit reports that it is not moving"),
			LLKPushFeel::IsMoving(
				Travel, LLKPushFeel::AdvancePush(Travel, 1.f, 1.f / 60.f, Travel)));
		TestTrue(TEXT("and one mid-rail reports that it is"),
			LLKPushFeel::IsMoving(
				1.f, LLKPushFeel::AdvancePush(1.f, 1.f, 1.f / 60.f, Travel)));
		TestFalse(TEXT("and one nobody is pushing straight reports silence"),
			LLKPushFeel::IsMoving(
				1.f, LLKPushFeel::AdvancePush(1.f, 0.1f, 1.f / 60.f, Travel)));
	}

	return true;
}

#endif
