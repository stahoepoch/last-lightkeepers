// What the Sweeper's motor claims, asserted rather than intended.
//
// `Enemies/LLKSweeperPresence.h` exists because a Sweeper walked 71.2 s of
// path in silence. The balance in it is a design argument, and the point of
// this file is that the argument is pinned: if somebody retunes a number in a
// way that takes back the thing the system was added for, a named test fails
// instead of a playtest going quiet.
//
// Four claims, and what each one is protecting:
//
//   1. **Every state is audible except the telegraph.** The whole system is
//      one defect's answer - a machine you cannot hear coming - so a state
//      that silently returns zero is that defect back, in one branch.
//   2. **The telegraph is exactly silent, and immediate.** Not quiet: zero,
//      reached in one tick. A glide into it plays the announcement over a
//      decaying motor, which is the one thing the header forbids.
//   3. **Pursue is louder and faster than patrol, and Stock is tellable from
//      patrol by ear.** Row 47 makes filing the first encounter; if a player
//      cannot hear the difference they cannot know they have a free approach.
//   4. **The glide is frame-rate independent and cannot overshoot.** A motor
//      whose ramp depends on frame time sounds different on Chris's machine
//      than on a tester's, and a volume that overshoots past the target
//      clips the loop's headroom.

#include "Enemies/LLKSweeperPresence.h"

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKSweeperPresenceTest,
	"LLK.Enemies.SweeperPresence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKSweeperPresenceTest::RunTest(const FString&)
{
	using namespace LLKSweeperPresence;

	const ELLKSweeperState AllStates[] = {
		ELLKSweeperState::Patrol,
		ELLKSweeperState::Telegraph,
		ELLKSweeperState::Pursue,
		ELLKSweeperState::Search,
		ELLKSweeperState::Stock,
	};

	// 1. Every state is audible except the telegraph.
	for (const ELLKSweeperState State : AllStates)
	{
		const FLLKVoice Heard = VoiceFor(State);
		const bool bShouldBeSilent = State == ELLKSweeperState::Telegraph;
		if (bShouldBeSilent)
		{
			TestEqual(
				TEXT("the telegraph is exactly silent, because the cue has to land in a hole"),
				Heard.Volume, 0.f);
		}
		else
		{
			TestTrue(
				FString::Printf(TEXT("state %d is audible while it moves"), static_cast<int32>(State)),
				Heard.Volume > 0.f);
		}

		// The loop is rendered with headroom; a multiplier above one spends it.
		TestTrue(
			FString::Printf(TEXT("state %d stays inside the loop's headroom"), static_cast<int32>(State)),
			Heard.Volume <= 1.f);
		// Playback rate moves the motor fundamental and the castor together.
		// Far outside this and it stops reading as the same machine.
		TestTrue(
			FString::Printf(TEXT("state %d keeps a plausible motor rate"), static_cast<int32>(State)),
			Heard.Pitch > 0.5f && Heard.Pitch < 2.f);
	}

	// 2. The telegraph's silence is immediate, not glided into.
	{
		const FLLKVoice Running = VoiceFor(ELLKSweeperState::Pursue);
		const FLLKVoice Silent = VoiceFor(ELLKSweeperState::Telegraph);
		const FLLKVoice Cut = GlideToward(Running, Silent, 1.f / 60.f, /*bImmediate=*/true);
		TestEqual(
			TEXT("entering the telegraph cuts the motor in one tick"), Cut.Volume, 0.f);

		const FLLKVoice Glided = GlideToward(Running, Silent, 1.f / 60.f, /*bImmediate=*/false);
		TestTrue(
			TEXT("and without the immediate flag it would NOT have, which is why the flag exists"),
			Glided.Volume > 0.f);
	}

	// 3. The states a player has to tell apart are actually apart.
	{
		const FLLKVoice Patrol = VoiceFor(ELLKSweeperState::Patrol);
		const FLLKVoice Pursue = VoiceFor(ELLKSweeperState::Pursue);
		const FLLKVoice Stock = VoiceFor(ELLKSweeperState::Stock);

		TestTrue(TEXT("pursuing is louder than patrolling"), Pursue.Volume > Patrol.Volume);
		TestTrue(TEXT("and faster"), Pursue.Pitch > Patrol.Pitch);

		// A just-noticeable difference in loudness is around 1 dB, which is
		// about a 12% amplitude change. Anything under that is a number that
		// looks like a cue in the source and is not one in the room.
		TestTrue(
			TEXT("filing stock is audibly busier than patrolling, not just numerically"),
			Stock.Volume >= Patrol.Volume * 1.12f);
		TestTrue(
			TEXT("and pursuing is audibly louder than filing, so the first encounter escalates"),
			Pursue.Volume >= Stock.Volume * 1.12f);
	}

	// 4. The glide is frame-rate independent and cannot overshoot.
	{
		const FLLKVoice From = VoiceFor(ELLKSweeperState::Patrol);
		const FLLKVoice To = VoiceFor(ELLKSweeperState::Pursue);

		// Same half second, reached in one step and in thirty.
		const FLLKVoice OneStep = GlideToward(From, To, 0.5f, false);
		FLLKVoice Many = From;
		for (int32 i = 0; i < 30; ++i)
		{
			Many = GlideToward(Many, To, 0.5f / 30.f, false);
		}
		TestEqual(
			TEXT("the ramp is the same at 2 fps and at 60 fps"),
			Many.Volume, OneStep.Volume, 0.001f);

		// A long frame must not sail past the target.
		const FLLKVoice Hitch = GlideToward(From, To, 10.f, false);
		TestEqual(
			TEXT("a hitch lands on the target rather than overshooting it"),
			Hitch.Volume, To.Volume, KINDA_SMALL_NUMBER);
		TestEqual(
			TEXT("and the same for the rate"), Hitch.Pitch, To.Pitch, KINDA_SMALL_NUMBER);

		// Patrol to pursue in about a fifth of a second, as the header claims.
		const float Climb = (To.Volume - From.Volume) / VolumeGlidePerSecond;
		TestTrue(
			FString::Printf(TEXT("the reaction is fast enough to read as one: %.3f s"), Climb),
			Climb > 0.f && Climb < 0.4f);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
