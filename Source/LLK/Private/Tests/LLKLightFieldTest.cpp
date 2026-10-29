// The brightest position, asserted before any enemy can read it wrong.
//
// Enemies converge on the brightest position, never the nearest player. That
// is locked, it is written into `data/enemies.json` as the Warden's behaviour,
// and `docs/THE-STORE.md` explains why it carries the whole tutorial: it makes
// the keeper who lit the lantern the keeper who gets found, so cause and
// consequence land on the same pair of hands. In four-player co-op the default
// outcome is that those are two different people and nobody learns anything.
//
// The archived Roblox `EnemyService` retargeted to the nearest player. That
// looked like a fidelity bug for months and it is not - it is the bug that
// would stop the building teaching. This file exists before the first enemy
// so that there is nothing for UE5 to inherit it from.
//
// Four properties, and the first two are the rule itself:
//
//   1. **A dark crew has no brightest position**, and the answer is "no" rather
//      than the world origin. An enemy handed (0,0,0) walks to the middle of
//      the map all night.
//   2. **A lone lit keeper is the brightest position**, whoever is nearer to
//      anything. This is the nearest-player bug stated as an assertion.
//   3. **A cluster outranks a lone keeper**, because brightness sums. Nobody
//      designed that; it falls out of two lanterns being brighter than one,
//      which is why the rule needs no tie-break policy invented for it.
//   4. **Identical input gives an identical answer.** A server and a client
//      must not disagree about who is being hunted, and neither must two
//      consecutive frames.
//
// **One note on how this file was verified, because it failed the first
// time.** The nearest-player bug was planted deliberately to check the test
// catches it, and the test PASSED - because the cluster had been placed at the
// world origin, where a nearest-to-origin rule picks the same answer. The
// assertion was passing by coincidence. The positions below are now arranged
// so that nearest and brightest disagree, and there is an invariant check that
// does not depend on the coordinates at all: the returned position must be the
// brightest candidate, which catches a wrong selection rule of any kind rather
// than only the one somebody thought to plant.

#include "Misc/AutomationTest.h"
#include "Storm/LLKLightField.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKLightFieldTest,
	"LLK.Light.BrightestPosition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace
{
	/** A keeper's lantern on `data/lantern.json`'s own numbers: 250cd, 840cm. */
	FLLKLightSource Keeper(const FVector& Where)
	{
		FLLKLightSource Source;
		Source.Position = Where;
		Source.Intensity = 250.f;
		Source.RadiusCm = 840.f;
		return Source;
	}
}

bool FLLKLightFieldTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK light field test reached RunTest."));

	FVector Position = FVector(-12345.f, -12345.f, -12345.f);
	const FVector Untouched = Position;
	float Brightness = -1.f;

	// ---- 1. a dark crew has no brightest position ----------------------
	TestFalse(TEXT("nothing lit means no brightest position"),
		LLKLightField::FindBrightestPosition({}, Position, Brightness));
	TestEqual(TEXT("and the out parameter is left alone, not zeroed"), Position, Untouched);

	// A registered-but-unlit crew is the same case. This is the normal state
	// of this game, not an edge case: row 5 makes being lit a decision.
	TArray<FLLKLightSource> Dark;
	FLLKLightSource Unlit = Keeper(FVector(500.f, 0.f, 0.f));
	Unlit.Intensity = 0.f;
	Dark.Add(Unlit);
	TestFalse(TEXT("a keeper with no intensity is not a light"),
		LLKLightField::FindBrightestPosition(Dark, Position, Brightness));

	// A lantern with reach but no brightness, and one with brightness but no
	// reach, are both not lights - and neither may divide by zero.
	FLLKLightSource NoReach = Keeper(FVector(700.f, 0.f, 0.f));
	NoReach.RadiusCm = 0.f;
	TestFalse(TEXT("a lantern with no reach is not a light"),
		LLKLightField::FindBrightestPosition({NoReach}, Position, Brightness));
	TestEqual(TEXT("and contributes nothing rather than infinity"),
		LLKLightField::BrightnessAt({NoReach}, FVector::ZeroVector), 0.f);

	// ---- 2. THE RULE. One lit keeper is the answer ---------------------
	const FVector LitSpot(1000.f, 2000.f, 300.f);
	TestTrue(TEXT("one lit keeper gives a brightest position"),
		LLKLightField::FindBrightestPosition({Keeper(LitSpot)}, Position, Brightness));
	TestEqual(TEXT("and it is that keeper"), Position, LitSpot);
	TestTrue(TEXT("with a brightness above zero"), Brightness > 0.f);

	// The nearest-player bug, stated as an assertion. A lit keeper far away
	// must outrank the origin, which is where a nearest-to-spawn
	// implementation would send everything.
	TArray<FLLKLightSource> OneFarLight{Keeper(FVector(50000.f, 0.f, 0.f))};
	TestTrue(TEXT("a distant lit keeper is still the brightest position"),
		LLKLightField::FindBrightestPosition(OneFarLight, Position, Brightness));
	TestEqual(TEXT("so distance from anything else is irrelevant"),
		Position, FVector(50000.f, 0.f, 0.f));

	// ---- 3. A CLUSTER OUTRANKS A LONE KEEPER ---------------------------
	//
	// Two keepers together, one keeper far off. The pair must win, and
	// nobody had to decide that - it falls out of summation. This is also
	// the mechanical consequence of a crew's instinct to bunch up when
	// frightened, which is worth knowing is real.
	// **The coordinates matter and the first version of this test got them
	// wrong.** The pair sat at the world origin and the loner far from it,
	// which meant a nearest-to-origin implementation picked the pair too - so
	// the test passed against a deliberately planted nearest-player bug. It
	// was passing by coincidence.
	//
	// So they are now arranged to DIVERGE: the loner is nearer to the origin
	// and nearer to the spawn than either of the pair. Nearest-anything picks
	// the loner; only brightness picks the pair.
	const FVector Loner(1000.f, 0.f, 0.f);
	const FVector PairA(20000.f, 0.f, 0.f);
	const FVector PairB(20200.f, 0.f, 0.f);

	TArray<FLLKLightSource> Crew{Keeper(Loner), Keeper(PairA), Keeper(PairB)};
	TestTrue(TEXT("a mixed crew has a brightest position"),
		LLKLightField::FindBrightestPosition(Crew, Position, Brightness));
	TestTrue(TEXT("and it is one of the pair, not the nearer loner"),
		Position == PairA || Position == PairB);
	TestNotEqual(TEXT("the lone keeper is specifically NOT the brightest"), Position, Loner);

	// And the invariant, which does not depend on my choice of coordinates at
	// all: whatever position comes back must be the one with the highest
	// summed brightness of any candidate. This catches a wrong SELECTION rule
	// of any kind - nearest player, first in array, last in array - rather
	// than only the one I happened to think to plant.
	{
		float BestSeen = -1.f;
		for (const FLLKLightSource& Source : Crew)
		{
			BestSeen = FMath::Max(BestSeen, LLKLightField::BrightnessAt(Crew, Source.Position));
		}
		TestNearlyEqual(TEXT("the answer is the brightest candidate, by construction"),
			LLKLightField::BrightnessAt(Crew, Position), BestSeen, 0.001f);
		TestNearlyEqual(TEXT("and the reported brightness is that value"),
			Brightness, BestSeen, 0.001f);
	}

	// Stated the other way too, since the ordering above could hide it: the
	// loner is listed FIRST, is nearest the origin, and still loses.
	TestTrue(TEXT("two together are brighter than one alone"),
		LLKLightField::BrightnessAt(Crew, PairA) > LLKLightField::BrightnessAt(Crew, Loner));
	TestTrue(TEXT("even though the loner is nearer to the origin"),
		Loner.Size() < PairA.Size());

	// But a pair far enough apart stops being a cluster, which is what makes
	// this a field rather than a head count.
	TArray<FLLKLightSource> Spread{Keeper(FVector(0.f, 0.f, 0.f)), Keeper(FVector(40000.f, 0.f, 0.f))};
	TestTrue(TEXT("two keepers a long way apart are each about as bright as the other"),
		FMath::IsNearlyEqual(
			LLKLightField::BrightnessAt(Spread, FVector::ZeroVector),
			LLKLightField::BrightnessAt(Spread, FVector(40000.f, 0.f, 0.f)), 0.1f));

	// ---- 4. determinism ------------------------------------------------
	//
	// Two keepers in exactly the same place. Rare, but a server and a client
	// must name the same one, and so must consecutive frames.
	TArray<FLLKLightSource> Stacked{Keeper(FVector(10.f, 20.f, 30.f)), Keeper(FVector(10.f, 20.f, 30.f))};
	FVector FirstAnswer;
	FVector SecondAnswer;
	float Ignored = 0.f;
	TestTrue(TEXT("a stacked pair resolves"),
		LLKLightField::FindBrightestPosition(Stacked, FirstAnswer, Ignored));
	TestTrue(TEXT("and resolves again"),
		LLKLightField::FindBrightestPosition(Stacked, SecondAnswer, Ignored));
	TestEqual(TEXT("to the same keeper both times"), FirstAnswer, SecondAnswer);

	// Brightness is monotone in distance, which is what stops an enemy
	// changing its mind as a keeper takes one step. Sampled outward from a
	// single light; no hard cutoff anywhere, including past its own radius.
	const TArray<FLLKLightSource> Single{Keeper(FVector::ZeroVector)};
	float Previous = LLKLightField::BrightnessAt(Single, FVector::ZeroVector);
	for (float Distance = 100.f; Distance <= 5000.f; Distance += 100.f)
	{
		const float Current = LLKLightField::BrightnessAt(Single, FVector(Distance, 0.f, 0.f));
		TestTrue(*FString::Printf(TEXT("brightness still falling at %.0fcm"), Distance),
			Current < Previous && Current > 0.f);
		Previous = Current;
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
