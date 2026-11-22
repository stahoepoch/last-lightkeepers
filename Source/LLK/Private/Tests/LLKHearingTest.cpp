// Being heard. The half of stealth that did not exist.
//
// `LLKPerception`, `LLKVisibility`, the light field and five authored aggro
// radii all answer one question: can this machine SEE the keeper. Nothing
// answered whether it could hear them, so sprinting cost nothing in the dark
// and crouch was a visibility tool with no acoustic meaning. `LLKHearing.h`
// states the model and the reasoning; this file pins the parts that are
// arguable, and it needs no world, no pawn and no navmesh to do it.
//
// **The split with `tests/hearing.test.mjs` is deliberate and is the same one
// `LLKSweeperPresenceTest` uses.** Everything here is arithmetic and brain
// rules, which C++ can assert exactly. Whether the radii still line up with
// the gaits in `data/lantern.json` and the radii in `data/enemies.json` is a
// question about two JSON files, and a C++ constant copied out of them is a
// copy that drifts. That claim lives in the .mjs, which reads the files.
//
// The four rules worth breaking the build over:
//
//   1. A keeper who has stopped moving is silent. Not quiet - silent, at any
//      distance, including zero. Stopping is the verb this gives the player.
//   2. Sound can START a search and never a pursuit. Contact is reachable
//      only through sight, by the rules that already existed.
//   3. Sight outranks sound. Heard and seen in the same frame is a telegraph,
//      because row 16 is owed to a keeper who has actually been found.
//   4. Sound cannot EXTEND a search. A noise arriving mid-search does not
//      refresh the clock or move the target, which is what keeps a sprinting
//      keeper from being followed across the store indefinitely.

#include "Data/LLKEnemyDef.h"
#include "Enemies/LLKHearing.h"
#include "Enemies/LLKSweeperBrain.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKHearingTestFixture
{
	/** `enemies.json`'s Sweeper. A retune moves the test, not the other way. */
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

	FLLKSweeperContactRules Contact()
	{
		FLLKSweeperContactRules Rules;
		Rules.bKnockdownOnContact = false;
		Rules.ContactRange = 5.f;
		Rules.ContactCooldownSeconds = 1.5f;
		Rules.HitsToKnockDown = 3;
		Rules.HitDecaySeconds = 6.f;
		return Rules;
	}

	/** `data/lantern.json`'s authored gaits, in Unreal cm/s. */
	constexpr float CrouchCms = LLKUnits::SpeedFromStuds(3.f);
	constexpr float WalkCms = LLKUnits::SpeedFromStuds(5.f);
	constexpr float SprintCms = LLKUnits::SpeedFromStuds(12.5f);

	/** Unseen, on a route, with a noise at `NoiseStuds` from the machine. */
	FLLKSweeperSenses Heard(const float DeltaSeconds, const float NoiseStuds)
	{
		FLLKSweeperSenses Senses;
		Senses.DeltaSeconds = DeltaSeconds;
		Senses.Notice.bNoticed = false;
		Senses.bHasPatrolPoint = true;
		Senses.HeardNoisePosition =
			FVector(LLKUnits::DistanceFromStuds(NoiseStuds), 0.f, 0.f);
		Senses.DistanceToHeardStuds = NoiseStuds;
		return Senses;
	}

	/** Unseen, on a route, and nothing audible. */
	FLLKSweeperSenses Silent(const float DeltaSeconds)
	{
		FLLKSweeperSenses Senses;
		Senses.DeltaSeconds = DeltaSeconds;
		Senses.Notice.bNoticed = false;
		Senses.bHasPatrolPoint = true;
		return Senses;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKHearingTest, "LLK.Enemies.Hearing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKHearingTest::RunTest(const FString& Parameters)
{
	using namespace LLKHearingTestFixture;

	// ---- 1. The radius is one second of travel ---------------------------
	//
	// Asserted against the gaits rather than against literals, so the claim
	// under test is the RELATION and not three numbers somebody can satisfy
	// by changing the test.
	{
		const FVector Origin = FVector::ZeroVector;

		const LLKHearing::FLLKNoise Crouch =
			LLKHearing::NoiseFromMovement(Origin, CrouchCms);
		const LLKHearing::FLLKNoise Walk =
			LLKHearing::NoiseFromMovement(Origin, WalkCms);
		const LLKHearing::FLLKNoise Sprint =
			LLKHearing::NoiseFromMovement(Origin, SprintCms);

		TestEqual(TEXT("a crouch carries its own speed in studs"),
			Crouch.RadiusStuds, 3.f * LLKHearing::SecondsOfTravel, 0.001f);
		TestEqual(TEXT("a walk carries its own speed in studs"),
			Walk.RadiusStuds, 5.f * LLKHearing::SecondsOfTravel, 0.001f);
		TestEqual(TEXT("a sprint carries its own speed in studs"),
			Sprint.RadiusStuds, 12.5f * LLKHearing::SecondsOfTravel, 0.001f);

		// The gear change has to be audible as a gear change, or the player
		// has three speeds and one acoustic signature.
		TestTrue(TEXT("each gait is meaningfully louder than the one below it"),
			Walk.RadiusStuds > Crouch.RadiusStuds * 1.5f
				&& Sprint.RadiusStuds > Walk.RadiusStuds * 1.5f);
	}

	// ---- 2. Standing still is silent, at zero distance -------------------
	//
	// The case a `<=` comparison inside `NoiseReaching` would get wrong, and
	// the one that matters most: a keeper frozen behind a gondola with a
	// Sweeper beside them must be inaudible. Being found there is sight's job.
	{
		const LLKHearing::FLLKNoise Still =
			LLKHearing::NoiseFromMovement(FVector::ZeroVector, 0.f);

		TestEqual(TEXT("a keeper who has stopped makes a noise of radius zero"),
			Still.RadiusStuds, 0.f);
		TestFalse(TEXT("and it reaches a listener standing on the exact spot"),
			LLKHearing::NoiseReaching(Still, FVector::ZeroVector));

		// Negative speed cannot arise from `Size2D`, but a future caller
		// handing this a signed component must not produce a negative radius
		// that compares as inaudible by accident rather than by rule.
		const LLKHearing::FLLKNoise Nonsense =
			LLKHearing::NoiseFromMovement(FVector::ZeroVector, -500.f);
		TestEqual(TEXT("and a negative speed clamps to silence rather than wrapping"),
			Nonsense.RadiusStuds, 0.f);
	}

	// ---- 3. The boundary is where the arithmetic says it is --------------
	{
		const LLKHearing::FLLKNoise Sprint =
			LLKHearing::NoiseFromMovement(FVector::ZeroVector, SprintCms);
		const float ReachCm = LLKUnits::DistanceFromStuds(Sprint.RadiusStuds);

		TestTrue(TEXT("a listener just inside the radius hears it"),
			LLKHearing::NoiseReaching(Sprint, FVector(ReachCm - 1.f, 0.f, 0.f)));
		TestFalse(TEXT("and one just outside it does not"),
			LLKHearing::NoiseReaching(Sprint, FVector(ReachCm + 1.f, 0.f, 0.f)));

		// Distance is reported in studs because the senses are, row 61.
		TestEqual(TEXT("the reported distance is in studs"),
			LLKHearing::DistanceStuds(Sprint, FVector(ReachCm, 0.f, 0.f)),
			Sprint.RadiusStuds, 0.01f);
	}

	// ---- 4. Hearing starts a search, from patrol -------------------------
	{
		FLLKSweeperMind Mind;
		Mind.State = ELLKSweeperState::Patrol;

		const FLLKSweeperStep Step = LLKSweeperBrain::Advance(
			Mind, Heard(0.1f, 8.f), Sweeper(), Contact());

		TestEqual(TEXT("a patrolling Sweeper that heard something searches"),
			Step.Mind.State, ELLKSweeperState::Search);
		TestEqual(TEXT("and walks to the last known position"),
			Step.Move, ELLKSweeperMove::LastKnown);
		TestTrue(TEXT("and says so once, so a playtest can see why it moved"),
			Step.bNoiseInvestigated);

		// The sound's position, not the keeper's. Hearing is not tracking.
		TestEqual(TEXT("and it goes to where the SOUND was"),
			static_cast<float>(Step.Mind.LastKnownKeeperPosition.X),
			LLKUnits::DistanceFromStuds(8.f), 0.01f);
		TestTrue(TEXT("and it now has a last known position at all"),
			Step.Mind.bHasLastKnown);

		// Row 16 is owed to a keeper who has been SEEN. Spending it on a
		// false alarm teaches a player to ignore the cue that matters.
		TestFalse(TEXT("and it does not telegraph, because it has seen nobody"),
			Step.bTelegraphStarted);
		TestNotEqual(TEXT("and it never goes straight to pursuit on a sound"),
			Step.Mind.State, ELLKSweeperState::Pursue);
	}

	// ---- 5. Sight outranks sound ----------------------------------------
	{
		FLLKSweeperMind Mind;
		Mind.State = ELLKSweeperState::Patrol;

		FLLKSweeperSenses Senses = Heard(0.1f, 8.f);
		Senses.Notice.Sense = ELLKNoticeSense::LitProximity;
		Senses.Notice.Visibility = ELLKVisibility::Clear;
		Senses.Notice.bNoticed = true;
		Senses.KeeperPosition = FVector(LLKUnits::DistanceFromStuds(6.f), 0.f, 0.f);
		Senses.DistanceToKeeperStuds = 6.f;

		const FLLKSweeperStep Step =
			LLKSweeperBrain::Advance(Mind, Senses, Sweeper(), Contact());

		TestEqual(TEXT("heard AND seen is a telegraph, not a search"),
			Step.Mind.State, ELLKSweeperState::Telegraph);
		TestTrue(TEXT("and the announcement is owed and made"),
			Step.bTelegraphStarted);
		TestFalse(TEXT("and the sound is not also reported as the reason"),
			Step.bNoiseInvestigated);
	}

	// ---- 6. Sound cannot extend a search --------------------------------
	//
	// The anti-harassment rule, and the one most likely to be undone by a
	// later convenience. A noise arriving mid-search must not refresh the
	// clock, or a sprinting keeper is followed until they stop.
	{
		FLLKSweeperMind Mind;
		Mind.State = ELLKSweeperState::Search;
		Mind.SearchSecondsRemaining = 2.f;
		Mind.bHasLastKnown = true;
		Mind.LastKnownKeeperPosition = FVector(100.f, 0.f, 0.f);

		const FLLKSweeperStep Step = LLKSweeperBrain::Advance(
			Mind, Heard(0.5f, 4.f), Sweeper(), Contact());

		TestTrue(TEXT("a noise mid-search does not refresh the search clock"),
			Step.Mind.SearchSecondsRemaining < 2.f);
		TestFalse(TEXT("and does not re-report as a fresh investigation"),
			Step.bNoiseInvestigated);
		TestEqual(TEXT("and does not move the target to the new sound"),
			static_cast<float>(Step.Mind.LastKnownKeeperPosition.X), 100.f, 0.01f);
	}

	// ---- 7. Sound never becomes contact ---------------------------------
	//
	// Run a machine that can hear the keeper every single frame for twenty
	// seconds - far past the authored search interval, so it expires and
	// re-triggers repeatedly - and assert it never once reaches Pursue. That
	// transition is the only route to contact, and sound must not open it.
	{
		FLLKSweeperMind Mind;
		Mind.State = ELLKSweeperState::Patrol;

		bool bEverPursued = false;
		bool bEverTelegraphed = false;
		int32 Investigations = 0;

		for (int32 Frame = 0; Frame < 1200; ++Frame)
		{
			const FLLKSweeperStep Step = LLKSweeperBrain::Advance(
				Mind, Heard(1.f / 60.f, 6.f), Sweeper(), Contact());
			Mind = Step.Mind;

			bEverPursued = bEverPursued || Mind.State == ELLKSweeperState::Pursue;
			bEverTelegraphed = bEverTelegraphed || Step.bTelegraphStarted;
			Investigations += Step.bNoiseInvestigated ? 1 : 0;
		}

		TestFalse(TEXT("twenty seconds of constant noise never causes a pursuit"),
			bEverPursued);
		TestFalse(TEXT("and never spends row 16's announcement"),
			bEverTelegraphed);

		// It re-investigates at most once per search interval - the clock is
		// 3 s by fallback, so twenty seconds allows a handful and nothing
		// like the 1200 a per-frame trigger would produce.
		TestTrue(TEXT("and re-investigates on the search interval, not per frame"),
			Investigations > 1 && Investigations <= 10);
	}

	// ---- 8. Silence still behaves exactly as it did ----------------------
	//
	// The regression guard on everything above: a Sweeper in a world where
	// nothing makes a sound must patrol as it always has. If this fails, the
	// new sense is not an addition, it is a change.
	{
		FLLKSweeperMind Mind;
		Mind.State = ELLKSweeperState::Patrol;

		const FLLKSweeperStep Step = LLKSweeperBrain::Advance(
			Mind, Silent(0.1f), Sweeper(), Contact());

		TestEqual(TEXT("a silent floor leaves a patrolling Sweeper patrolling"),
			Step.Mind.State, ELLKSweeperState::Patrol);
		TestEqual(TEXT("and walking its route"),
			Step.Move, ELLKSweeperMove::PatrolPoint);
		TestFalse(TEXT("and it reports no investigation"),
			Step.bNoiseInvestigated);
		TestFalse(TEXT("and it has invented no last known position"),
			Step.Mind.bHasLastKnown);
	}

	return true;
}

#endif
