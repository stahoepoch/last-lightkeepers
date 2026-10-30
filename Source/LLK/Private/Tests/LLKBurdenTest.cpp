// What a haul costs to carry. M3.
//
// **The authored numbers, which are Chris's (row 55):** 0.04 per relic to a
// cap of 0.12. So one relic is 4% slower, two 8%, three 12%, and the fourth is
// free - and that last part is the design rather than an oversight. His note
// in `loot.json`: *"ordinary loot must not make a raid feel sluggish. Only
// relics impose a small movement penalty; supplies, components, and
// restoration crates do not."*
//
// **The properties are asserted, not the four numbers.** A property survives a
// retune and a number does not, which this project learned the expensive way
// when `tests/balance.test.mjs` passed 160 checks against a model the code no
// longer implemented. So: carrying more is never faster, the cap is honoured,
// and nothing can make a keeper unable to move. The specific percentages are
// checked once, against the data, so that a retune moves the test with it.
//
// **Two failure modes that are about bad data rather than about design**, and
// both would read as a broken character rather than as a wrong number:
//
//   1. A `maxSpeedPenalty` of 1.0 typed in by accident - a keeper frozen in
//      place while holding a brooch. The hard rail at 0.9 is why that cannot
//      happen, and it is deliberately far from the authored 0.12 so that
//      hitting it means the data is wrong.
//   2. A cap of zero read as "the harshest possible setting" rather than as
//      "no cap". An unfilled field must never be the cruellest one.

#include "Loot/LLKBurden.h"
#include "Misc/AutomationTest.h"

#include <limits>

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKBurdenTestFixture
{
	/** `loot.json`'s burden block, so a retune moves the test. */
	FLLKBurdenRules Authored()
	{
		FLLKBurdenRules Rules;
		Rules.SpeedPenaltyPerPoint = 0.04f;
		Rules.MaxSpeedPenalty = 0.12f;
		return Rules;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKBurdenTest,
	"LLK.Loot.Burden",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKBurdenTest::RunTest(const FString& Parameters)
{
	using namespace LLKBurdenTestFixture;

	const FLLKBurdenRules Rules = Authored();

	// ---- 1. empty-handed is exactly the authored speed ------------------
	//
	// A rule rather than arithmetic: the numbers in `lantern.json` have to
	// mean what they say for a keeper on the way in.
	{
		TestEqual(TEXT("carrying nothing costs nothing"),
			LLKBurden::PenaltyFraction(0, Rules), 0.f);
		TestEqual(TEXT("and the multiplier is exactly one"),
			LLKBurden::SpeedMultiplier(0, Rules), 1.f);

		// A negative count is not a speed bonus. Reachable only from a bug,
		// and a keeper who got faster from one would be very hard to explain.
		TestEqual(TEXT("a negative count is not a bonus"),
			LLKBurden::PenaltyFraction(-3, Rules), 0.f);
	}

	// ---- 2. the authored curve, checked against the data ----------------
	{
		TestNearlyEqual(TEXT("one relic is four per cent"),
			LLKBurden::PenaltyFraction(1, Rules), 0.04f, 0.0001f);
		TestNearlyEqual(TEXT("two is eight"),
			LLKBurden::PenaltyFraction(2, Rules), 0.08f, 0.0001f);
		TestNearlyEqual(TEXT("three is twelve, which is the cap"),
			LLKBurden::PenaltyFraction(3, Rules), 0.12f, 0.0001f);
		TestNearlyEqual(TEXT("and a keeper at the cap moves at 88% speed"),
			LLKBurden::SpeedMultiplier(3, Rules), 0.88f, 0.0001f);
	}

	// ---- 3. the cap holds, and the fourth relic is free -----------------
	//
	// The part of the design worth protecting: a crew can always choose to be
	// greedy without becoming unable to run, which is what makes burden a
	// nudge rather than a leash.
	{
		TestNearlyEqual(TEXT("four relics cost no more than three"),
			LLKBurden::PenaltyFraction(4, Rules), 0.12f, 0.0001f);
		TestNearlyEqual(TEXT("and neither does forty"),
			LLKBurden::PenaltyFraction(40, Rules), 0.12f, 0.0001f);

		TestEqual(TEXT("the readout can say where the cap is"),
			LLKBurden::PointsUntilCapped(Rules), 3);
	}

	// ---- 4. carrying more is never faster -------------------------------
	//
	// The property, asserted across the whole range rather than at four
	// points, because this is the one that must survive any retune.
	{
		float Previous = LLKBurden::SpeedMultiplier(0, Rules);
		for (int32 Points = 1; Points <= 50; ++Points)
		{
			const float Now = LLKBurden::SpeedMultiplier(Points, Rules);
			TestTrue(TEXT("a heavier haul is never faster"), Now <= Previous);
			TestTrue(TEXT("and never stops a keeper moving"), Now > 0.f);
			Previous = Now;
		}
	}

	// ---- 5. burden authored off, which is every run before M3 -----------
	{
		FLLKBurdenRules Off;
		TestEqual(TEXT("an unauthored penalty is no penalty"),
			LLKBurden::PenaltyFraction(4, Off), 0.f);
		TestEqual(TEXT("and the readout says there is no cap to reach"),
			LLKBurden::PointsUntilCapped(Off), 0);
	}

	// ---- 6. bad data cannot freeze a keeper -----------------------------
	{
		// A cap of zero means NO CAP, not total immobility. An unfilled field
		// must never be the harshest setting.
		FLLKBurdenRules NoCap;
		NoCap.SpeedPenaltyPerPoint = 0.04f;
		NoCap.MaxSpeedPenalty = 0.f;
		TestNearlyEqual(TEXT("a cap of zero means no cap rather than no movement"),
			NoCap.MaxSpeedPenalty <= 0.f ? LLKBurden::PenaltyFraction(2, NoCap) : -1.f,
			0.08f, 0.0001f);

		// The hard rail. 1.0 per point would otherwise be a keeper who cannot
		// move while holding one brooch.
		FLLKBurdenRules Absurd;
		Absurd.SpeedPenaltyPerPoint = 1.f;
		Absurd.MaxSpeedPenalty = 1.f;
		TestTrue(TEXT("the hard rail keeps a keeper moving through absurd data"),
			LLKBurden::SpeedMultiplier(1, Absurd) > 0.f);
		TestTrue(TEXT("and it is far from anything authored, so hitting it means bad data"),
			LLKBurden::PenaltyFraction(1, Absurd) >= 0.9f - KINDA_SMALL_NUMBER);

		// Negative data is not a bonus either.
		FLLKBurdenRules Negative;
		Negative.SpeedPenaltyPerPoint = -0.5f;
		Negative.MaxSpeedPenalty = -1.f;
		TestEqual(TEXT("negative tuning does not make a haul a speed boost"),
			LLKBurden::SpeedMultiplier(3, Negative), 1.f);

		// And garbage.
		FLLKBurdenRules NotNumbers;
		NotNumbers.SpeedPenaltyPerPoint = std::numeric_limits<float>::quiet_NaN();
		NotNumbers.MaxSpeedPenalty = std::numeric_limits<float>::infinity();
		TestEqual(TEXT("a penalty that is not a number is no penalty"),
			LLKBurden::SpeedMultiplier(3, NotNumbers), 1.f);
		TestEqual(TEXT("and the cap readout refuses to guess"),
			LLKBurden::PointsUntilCapped(NotNumbers), 0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
