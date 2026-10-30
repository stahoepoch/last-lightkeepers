// What a Sweeper decides. M3's first encounter.
//
// **The question this encounter exists to answer is Chris's:** whether the
// dock hold is *"a tense decision or twenty seconds of waiting."* Twenty
// seconds alone on a floor is waiting; twenty seconds with something walking a
// route nearby is a decision. So the rules below are the ones that make a
// Sweeper pressure rather than a hazard, and each is asserted with no world -
// there is no pawn yet and no navmesh to put one on.
//
// **Row 16 is the one that is a build requirement rather than a preference.**
// Every enemy announces itself with sound before sight, distinguishably.
// `enemies.json` gives Sweepers `audioTelegraphSeconds: 1.0`, and the
// assertion is not that a sound plays - this layer has no audio - but that a
// Sweeper which has noticed you cannot be moving toward you until it has
// finished saying so. A telegraph that can be skipped is not a telegraph.
//
// **The three rules that make light a gamble instead of a switch:**
//
//   1. Losing the notice sends it to where you WERE. Not to where you are,
//      which would be omniscience, and not straight back to patrol, which
//      would make a lantern an off switch.
//   2. Re-acquiring does not re-telegraph. A free second every time a keeper
//      rounds a corner would make the warning worthless by inflation.
//   3. Giving up clears the last known position. A Sweeper that kept it would
//      walk to a two-minute-old ghost the next time anything noticed it.
//
// **And the contact numbers, which are what "weak alone" means.** Three hits
// at a 1.5-second cooldown is four and a half seconds of being touched, and
// `hitDecaySeconds: 6` drains the whole stack - so breaking away is always
// worth doing, and a swarm is the thing that actually knocks you down.

#include "Data/LLKEnemyDef.h"
#include "Data/LLKLanternData.h"
#include "Enemies/LLKSweeperBrain.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKSweeperBrainTestFixture
{
	/** `enemies.json`'s Sweeper, so a retune moves the test. */
	FLLKEnemyDef Sweeper()
	{
		FLLKEnemyDef Def;
		Def.Family = ELLKEnemyFamily::Sweepers;
		Def.MoveSpeed = 14.f;
		Def.AudioTelegraphSeconds = 1.f;
		Def.AggroRadiusDark = 12.f;
		Def.AggroRadiusLit = 38.f;
		return Def;
	}

	/** `lantern.json`'s `sweeperContact`. Authored since M0.5 and read today. */
	FLLKSweeperContactRules Contact()
	{
		FLLKSweeperContactRules Rules;
		Rules.bKnockdownOnContact = true;
		Rules.ContactRange = 5.f;
		Rules.ContactCooldownSeconds = 1.5f;
		Rules.HitsToKnockDown = 3;
		Rules.HitDecaySeconds = 6.f;
		return Rules;
	}

	/** Noticed, at a distance, with a patrol route under its feet. */
	FLLKSweeperSenses Seen(const float DeltaSeconds, const float DistanceStuds)
	{
		FLLKSweeperSenses Senses;
		Senses.DeltaSeconds = DeltaSeconds;
		Senses.Notice.Sense = ELLKNoticeSense::LitProximity;
		Senses.Notice.Visibility = ELLKVisibility::Clear;
		Senses.Notice.bNoticed = true;
		// Row 61: the conversion lives in one place, including in a test. A raw
		// 28 here is the same drift the guard exists to catch, and it caught
		// this one.
		Senses.KeeperPosition = FVector(LLKUnits::DistanceFromStuds(DistanceStuds), 0.f, 0.f);
		Senses.DistanceToKeeperStuds = DistanceStuds;
		Senses.bHasPatrolPoint = true;
		return Senses;
	}

	/** Not noticed - the lantern went out, or a shelf got in the way. */
	FLLKSweeperSenses Unseen(const float DeltaSeconds)
	{
		FLLKSweeperSenses Senses;
		Senses.DeltaSeconds = DeltaSeconds;
		Senses.Notice.Sense = ELLKNoticeSense::LitProximity;
		Senses.Notice.Visibility = ELLKVisibility::Blocked;
		Senses.Notice.bNoticed = false;
		Senses.bHasPatrolPoint = true;
		return Senses;
	}

	/** A Sweeper mid-chase, telegraph spent. */
	FLLKSweeperMind Pursuing()
	{
		FLLKSweeperMind Mind;
		Mind.State = ELLKSweeperState::Pursue;
		Mind.bHasLastKnown = true;
		Mind.LastKnownKeeperPosition = FVector(280.f, 0.f, 0.f);
		return Mind;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKSweeperBrainTest,
	"LLK.Enemies.SweeperBrain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKSweeperBrainTest::RunTest(const FString& Parameters)
{
	using namespace LLKSweeperBrainTestFixture;

	const FLLKEnemyDef Def = Sweeper();
	const FLLKSweeperContactRules Rules = Contact();

	// ---- 1. patrolling, and a map with no route --------------------------
	{
		FLLKSweeperMind Mind;
		const FLLKSweeperStep Walking =
			LLKSweeperBrain::Advance(Mind, Unseen(0.1f), Def, Rules);
		TestEqual(TEXT("an unbothered Sweeper walks its route"),
			FString(LLKSweeperBrain::NameOf(Walking.Move)), FString(TEXT("PatrolPoint")));
		TestEqual(TEXT("and stays in Patrol"),
			FString(LLKSweeperBrain::NameOf(Walking.Mind.State)), FString(TEXT("Patrol")));

		// Nobody measured a route: today's normal case, since no map has
		// patrol points. It holds rather than inventing somewhere to be.
		FLLKSweeperSenses NoRoute = Unseen(0.1f);
		NoRoute.bHasPatrolPoint.Reset();
		TestEqual(TEXT("with no patrol points measured it holds rather than guessing"),
			FString(LLKSweeperBrain::NameOf(
				LLKSweeperBrain::Advance(Mind, NoRoute, Def, Rules).Move)),
			FString(TEXT("Hold")));
	}

	// ---- 2. ROW 16: sound before sight, and it cannot be skipped --------
	{
		FLLKSweeperMind Mind;

		const FLLKSweeperStep Noticing =
			LLKSweeperBrain::Advance(Mind, Seen(0.016f, 10.f), Def, Rules);
		TestTrue(TEXT("ROW 16: noticing a keeper starts the telegraph"),
			Noticing.bTelegraphStarted);
		TestEqual(TEXT("and the state says so"),
			FString(LLKSweeperBrain::NameOf(Noticing.Mind.State)), FString(TEXT("Telegraph")));
		TestEqual(TEXT("ROW 16: and it is NOT moving toward the keeper yet"),
			FString(LLKSweeperBrain::NameOf(Noticing.Move)), FString(TEXT("Hold")));
		TestEqual(TEXT("for the authored second"),
			Noticing.Mind.TelegraphSecondsRemaining, 1.f);

		// Nine tenths of a second later it is still announcing. This is the
		// head start - the whole value of the telegraph is that a player can
		// use it.
		FLLKSweeperMind Mind2 = Noticing.Mind;
		const FLLKSweeperStep Mid = LLKSweeperBrain::Advance(Mind2, Seen(0.9f, 10.f), Def, Rules);
		TestEqual(TEXT("ROW 16: nine tenths of a second in, it still has not moved"),
			FString(LLKSweeperBrain::NameOf(Mid.Move)), FString(TEXT("Hold")));
		TestFalse(TEXT("and the telegraph does not restart"), Mid.bTelegraphStarted);

		// And it never hits during the telegraph, even stood on top of you.
		const FLLKSweeperStep Adjacent =
			LLKSweeperBrain::Advance(Mind2, Seen(0.5f, 1.f), Def, Rules);
		TestFalse(TEXT("ROW 16: and cannot touch a keeper it is still announcing to"),
			Adjacent.bContactLanded);

		// The second is up: now it comes.
		const FLLKSweeperStep Committed =
			LLKSweeperBrain::Advance(Mid.Mind, Seen(0.2f, 10.f), Def, Rules);
		TestEqual(TEXT("once it has announced, it pursues"),
			FString(LLKSweeperBrain::NameOf(Committed.Mind.State)), FString(TEXT("Pursue")));
		TestEqual(TEXT("and moves at the keeper"),
			FString(LLKSweeperBrain::NameOf(Committed.Move)), FString(TEXT("Keeper")));
	}

	// ---- 3. a keeper who used the telegraph well ------------------------
	//
	// Out of sight before the second was up: it goes to where you were, and
	// has never been told where you are.
	{
		FLLKSweeperMind Mind;
		const FLLKSweeperStep Noticed =
			LLKSweeperBrain::Advance(Mind, Seen(0.016f, 10.f), Def, Rules);

		const FLLKSweeperStep Gone =
			LLKSweeperBrain::Advance(Noticed.Mind, Unseen(1.2f), Def, Rules);
		TestEqual(TEXT("a keeper who breaks away during the telegraph is searched for"),
			FString(LLKSweeperBrain::NameOf(Gone.Mind.State)), FString(TEXT("Search")));
		TestEqual(TEXT("at the place it last had them"),
			FString(LLKSweeperBrain::NameOf(Gone.Move)), FString(TEXT("LastKnown")));
		TestTrue(TEXT("which it remembers"), Gone.Mind.bHasLastKnown);
	}

	// ---- 4. losing a chase: where you were, not where you are ----------
	{
		const FLLKSweeperStep Lost =
			LLKSweeperBrain::Advance(Pursuing(), Unseen(0.1f), Def, Rules);
		TestEqual(TEXT("a lantern going out ends the sighting, not the hunt"),
			FString(LLKSweeperBrain::NameOf(Lost.Mind.State)), FString(TEXT("Search")));
		TestEqual(TEXT("and it heads for the last known position"),
			FString(LLKSweeperBrain::NameOf(Lost.Move)), FString(TEXT("LastKnown")));
		TestEqual(TEXT("which is where the keeper was, not where they are"),
			Lost.Mind.LastKnownKeeperPosition, FVector(280.f, 0.f, 0.f));

		// It gives up after the declared search time, and clears the memory
		// with the state - otherwise it would walk to a stale ghost the next
		// time anything noticed it.
		FLLKSweeperMind Mind = Lost.Mind;
		FLLKSweeperStep Step;
		for (int32 Tick = 0; Tick < 100 && Mind.State == ELLKSweeperState::Search; ++Tick)
		{
			Step = LLKSweeperBrain::Advance(Mind, Unseen(0.5f), Def, Rules);
			Mind = Step.Mind;
		}
		TestEqual(TEXT("it eventually gives up and goes back to its route"),
			FString(LLKSweeperBrain::NameOf(Mind.State)), FString(TEXT("Patrol")));
		TestFalse(TEXT("and forgets where it last saw anybody"), Mind.bHasLastKnown);
		TestTrue(TEXT("saying so once, for the log"), Step.bLostTheKeeper);
	}

	// ---- 5. re-acquiring does not buy the keeper a second telegraph -----
	{
		FLLKSweeperMind Searching;
		Searching.State = ELLKSweeperState::Search;
		Searching.SearchSecondsRemaining = 2.f;
		Searching.bHasLastKnown = true;

		const FLLKSweeperStep Found =
			LLKSweeperBrain::Advance(Searching, Seen(0.1f, 8.f), Def, Rules);
		TestEqual(TEXT("a Sweeper that re-finds you resumes the chase"),
			FString(LLKSweeperBrain::NameOf(Found.Mind.State)), FString(TEXT("Pursue")));
		TestFalse(TEXT("without a second telegraph, which would be a free second per corner"),
			Found.bTelegraphStarted);
	}

	// ---- 6. contact: weak alone -----------------------------------------
	{
		FLLKSweeperMind Mind = Pursuing();

		// In range and off cooldown: one hit.
		FLLKSweeperStep Step = LLKSweeperBrain::Advance(Mind, Seen(0.1f, 4.f), Def, Rules);
		TestTrue(TEXT("a Sweeper in contact range lands a hit"), Step.bContactLanded);
		TestFalse(TEXT("one hit is not a knockdown"), Step.bKnockedDown);
		TestNearlyEqual(TEXT("the cooldown starts"),
			Step.Mind.ContactCooldownRemaining, 1.5f, 0.001f);

		// Still touching, but on cooldown: no second hit this frame.
		const FLLKSweeperStep Again =
			LLKSweeperBrain::Advance(Step.Mind, Seen(0.1f, 4.f), Def, Rules);
		TestFalse(TEXT("and it cannot hit again until the cooldown is spent"),
			Again.bContactLanded);

		// Out of range: no hit however long it stands there.
		const FLLKSweeperStep Far =
			LLKSweeperBrain::Advance(Pursuing(), Seen(2.f, 9.f), Def, Rules);
		TestFalse(TEXT("and nothing lands from outside contact range"), Far.bContactLanded);

		// Three hits, with the cooldown respected between them, is the floor.
		FLLKSweeperMind Chase = Pursuing();
		int32 Hits = 0;
		bool bDown = false;
		for (int32 Tick = 0; Tick < 200 && !bDown; ++Tick)
		{
			const FLLKSweeperStep Touch =
				LLKSweeperBrain::Advance(Chase, Seen(0.1f, 3.f), Def, Rules);
			Chase = Touch.Mind;
			Hits += Touch.bContactLanded ? 1 : 0;
			bDown = Touch.bKnockedDown;
		}
		TestTrue(TEXT("three hits knock a keeper down"), bDown);
		TestEqual(TEXT("and it takes exactly the authored three"), Hits, 3);
		TestEqual(TEXT("the stack is spent rather than carried into the next one"),
			Chase.HitsLanded(), 0);
	}

	// ---- 7. breaking away drains the stack, which is the counterplay ----
	{
		FLLKSweeperMind Mind = Pursuing();
		Mind.HitAgesSeconds.Add(0.f);
		Mind.HitAgesSeconds.Add(0.f);

		// Six seconds of not being touched - `hitDecaySeconds` - and the two
		// hits are gone. This is what makes "kill lights, move" a real answer
		// rather than a delay.
		for (int32 Second = 0; Second < 6; ++Second)
		{
			Mind = LLKSweeperBrain::Advance(Mind, Unseen(1.f), Def, Rules).Mind;
		}
		TestEqual(TEXT("six seconds clear of a Sweeper drains the hits"), Mind.HitsLanded(), 0);
	}

	// ---- 8. an unmeasurable frame is not a sighting ---------------------
	//
	// `LLKPerception` already refuses to notice on `Unknown`, and this refuses
	// to act on it a second time: an enemy that guesses on a failed trace
	// hunts you through geometry nobody can see.
	{
		FLLKSweeperSenses Unmeasured;
		Unmeasured.DeltaSeconds = 0.1f;
		Unmeasured.Notice.Sense = ELLKNoticeSense::LitProximity;
		Unmeasured.Notice.Visibility = ELLKVisibility::Unknown;
		Unmeasured.Notice.bNoticed = false;
		Unmeasured.DistanceToKeeperStuds = 1.f;
		Unmeasured.bHasPatrolPoint = true;

		FLLKSweeperMind Mind;
		const FLLKSweeperStep Step = LLKSweeperBrain::Advance(Mind, Unmeasured, Def, Rules);
		TestFalse(TEXT("an unknown visibility never starts a telegraph"), Step.bTelegraphStarted);
		TestEqual(TEXT("and never becomes a chase"),
			FString(LLKSweeperBrain::NameOf(Step.Mind.State)), FString(TEXT("Patrol")));

		// Nor a hit, from a Sweeper already chasing.
		const FLLKSweeperStep Touch = LLKSweeperBrain::Advance(Pursuing(), Unmeasured, Def, Rules);
		TestFalse(TEXT("and nothing lands on a keeper nobody could see"), Touch.bContactLanded);
	}

	// ---- 9. zero and garbage deltas -------------------------------------
	{
		FLLKSweeperMind Mind;
		Mind.State = ELLKSweeperState::Telegraph;
		Mind.TelegraphSecondsRemaining = 1.f;

		FLLKSweeperSenses Hitch = Seen(0.f, 10.f);
		TestEqual(TEXT("a zero-length frame does not spend the telegraph"),
			LLKSweeperBrain::Advance(Mind, Hitch, Def, Rules).Mind.TelegraphSecondsRemaining, 1.f);

		Hitch.DeltaSeconds = -4.f;
		TestEqual(TEXT("and neither does a negative one"),
			LLKSweeperBrain::Advance(Mind, Hitch, Def, Rules).Mind.TelegraphSecondsRemaining, 1.f);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
