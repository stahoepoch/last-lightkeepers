// Who is standing in the bay. M3, row 75.
//
// The zone actor owns a trigger volume and a list, and Chris scoped it to
// exactly that: *"no dock mesh, no authored map placement, no recovery-rig
// visuals yet."* So the thing worth testing is the list, and the list is a
// plain struct with no engine in it - `FLLKZoneOccupancy` - which is why these
// rules can be checked with no level, no capsule and no overlap.
//
// **What is NOT tested here, said plainly.** The two overlap handlers. They
// need a world, a moving pawn and a built collision scene, which is PIE work
// and belongs with the greybox. What they do once an overlap has happened is
// one call each into the struct below, and everything that call could get
// wrong afterwards is asserted.
//
// **The four ways a hold could advance with an empty bay**, which is the
// failure this file exists to make impossible:
//
//   1. A double begin-overlap. Unreal reports one per overlapping body, so a
//      capsule entering a box can report twice; a keeper counted twice is
//      still in the zone after walking out of it once.
//   2. A destroyed keeper. The run can destroy an actor standing in the bay,
//      and a strong pointer would hold a hold open with a corpse.
//   3. A downed keeper. Row 75's rule, which is row 74's fiction: no driver
//      comes into the building to collect anybody.
//   4. Confusing the two counts. `CountInside` is geometry and `CountStanding`
//      is the rule, and only one of them may advance a hold.

#include "Characters/LLKVitalsComponent.h"
#include "Components/BoxComponent.h"
#include "Misc/AutomationTest.h"
#include "Run/LLKExtractionZone.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKExtractionZoneTestFixture
{
	/**
	 * A keeper is a vitals component, which is all the zone ever asks about.
	 *
	 * No actor and no pawn: the zone finds vitals on whatever walks in, and the
	 * only question it asks afterwards is whether they are down.
	 */
	ULLKVitalsComponent* Keeper()
	{
		return NewObject<ULLKVitalsComponent>(GetTransientPackage());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKExtractionZoneTest,
	"LLK.Run.ExtractionZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKExtractionZoneTest::RunTest(const FString& Parameters)
{
	using namespace LLKExtractionZoneTestFixture;

	// ---- 1. an empty zone, which is most of a run -----------------------
	{
		FLLKZoneOccupancy Occupancy;
		TestEqual(TEXT("an empty zone has nobody inside"), Occupancy.CountInside(), 0);
		TestEqual(TEXT("and nobody standing"), Occupancy.CountStanding(), 0);
		TestEqual(TEXT("and nobody down in it"), Occupancy.CountDowned(), 0);

		// Null is not a keeper, and leaving when you were never there is not
		// an error - both are reachable from an overlap with a non-keeper.
		Occupancy.Enter(nullptr);
		Occupancy.Leave(nullptr);
		Occupancy.Leave(Keeper());
		TestEqual(TEXT("and none of that puts anybody in it"), Occupancy.CountInside(), 0);
	}

	// ---- 2. entering twice is entering once -----------------------------
	//
	// Failure mode one. The most likely real bug in this file, because it
	// depends on how many bodies a capsule has rather than on anything the
	// game does.
	{
		FLLKZoneOccupancy Occupancy;
		ULLKVitalsComponent* One = Keeper();

		Occupancy.Enter(One);
		Occupancy.Enter(One);
		Occupancy.Enter(One);
		TestEqual(TEXT("three begin-overlaps for one keeper is one keeper"),
			Occupancy.CountInside(), 1);

		// And one leave empties the bay. If the duplicates had been kept, this
		// is where a hold would keep advancing with nobody in the zone.
		Occupancy.Leave(One);
		TestEqual(TEXT("and one end-overlap empties the zone"), Occupancy.CountInside(), 0);
	}

	// ---- 3. a whole crew, and they leave independently ------------------
	{
		FLLKZoneOccupancy Occupancy;
		ULLKVitalsComponent* A = Keeper();
		ULLKVitalsComponent* B = Keeper();
		ULLKVitalsComponent* C = Keeper();
		ULLKVitalsComponent* D = Keeper();

		for (ULLKVitalsComponent* Each : {A, B, C, D})
		{
			Occupancy.Enter(Each);
		}
		TestEqual(TEXT("four keepers in the bay are four"), Occupancy.CountInside(), 4);
		TestEqual(TEXT("and all four are standing"), Occupancy.CountStanding(), 4);

		Occupancy.Leave(B);
		Occupancy.Leave(D);
		TestEqual(TEXT("two walk out and two remain"), Occupancy.CountInside(), 2);

		// The hold still advances on one keeper. Row 75: at least one, and no
		// faster for more - the hold is the rig loading, not work the crew does.
		Occupancy.Leave(A);
		TestEqual(TEXT("and one keeper is still somebody"), Occupancy.CountInside(), 1);
		Occupancy.Leave(C);
		TestEqual(TEXT("until the last one leaves"), Occupancy.CountInside(), 0);
	}

	// ---- 4. ROW 75: a downed keeper does not hold the route -------------
	//
	// Failure mode three, and the one that is a design rule rather than a
	// bookkeeping slip. Row 74 is explicit that nobody comes inside to collect
	// anybody, so a crew that drags itself to the dock and goes down there has
	// not extracted.
	{
		FLLKZoneOccupancy Occupancy;
		ULLKVitalsComponent* Standing = Keeper();
		ULLKVitalsComponent* Down = Keeper();
		Down->SetDownedForTest(true, ELLKDownedReason::Contained);

		Occupancy.Enter(Standing);
		Occupancy.Enter(Down);

		TestEqual(TEXT("both keepers are physically in the zone"), Occupancy.CountInside(), 2);
		TestEqual(TEXT("ROW 75: but only one of them holds the route"),
			Occupancy.CountStanding(), 1);
		TestEqual(TEXT("and the readout can say one is down in there"),
			Occupancy.CountDowned(), 1);

		// The whole crew down in the bay is a stalled hold, not a secured one.
		Standing->SetDownedForTest(true, ELLKDownedReason::Contained);
		TestEqual(TEXT("a crew down in the bay still occupies it"), Occupancy.CountInside(), 2);
		TestEqual(TEXT("ROW 75: and holds nothing"), Occupancy.CountStanding(), 0);
	}

	// ---- 5. a destroyed keeper is not in the zone -----------------------
	//
	// Failure mode two. Weak pointers, dropped when counted rather than
	// tracked - the same no-ghost rule the lit-lantern count and the run's
	// roll both follow.
	{
		FLLKZoneOccupancy Occupancy;
		ULLKVitalsComponent* Doomed = Keeper();
		Occupancy.Enter(Doomed);
		TestEqual(TEXT("in the zone to begin with"), Occupancy.CountInside(), 1);

		Doomed->MarkAsGarbage();
		TestEqual(TEXT("a destroyed keeper is nobody, inside"), Occupancy.CountInside(), 0);
		TestEqual(TEXT("and nobody, standing"), Occupancy.CountStanding(), 0);
		TestEqual(TEXT("and does not read as somebody lying in there"),
			Occupancy.CountDowned(), 0);
	}

	// ---- 6. the zone actor is the thin half it was scoped to be ---------
	{
		ALLKExtractionZone* Zone = NewObject<ALLKExtractionZone>(GetTransientPackage());

		TestNotNull(TEXT("the zone has a trigger volume"), Zone->GetTrigger());
		const bool bHasExtent = Zone->GetTrigger() != nullptr
			&& Zone->GetTrigger()->GetUnscaledBoxExtent().X > 0.0;
		TestTrue(TEXT("with some extent to it"), bHasExtent);
		TestTrue(TEXT("and no exit until somebody types one in"),
			Zone->ExtractionPointId.IsNone());

		// The counts the subsystem reads, through the actor rather than the
		// struct, because that is the call the rule actually makes.
		ULLKVitalsComponent* One = Keeper();
		Zone->EnterForTest(One);
		TestEqual(TEXT("the actor reports its occupancy"), Zone->CountKeepersInside(), 1);
		TestEqual(TEXT("and who in it is standing"), Zone->CountKeepersStanding(), 1);

		One->SetDownedForTest(true, ELLKDownedReason::Contained);
		TestEqual(TEXT("a keeper going down in the bay empties the route"),
			Zone->CountKeepersStanding(), 0);
		TestEqual(TEXT("without emptying the bay"), Zone->CountKeepersInside(), 1);

		Zone->LeaveForTest(One);
		TestEqual(TEXT("and leaving empties both"), Zone->CountKeepersInside(), 0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
