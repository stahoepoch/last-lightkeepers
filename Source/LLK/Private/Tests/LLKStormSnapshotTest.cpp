// The replication seam. M1.7's second half.
//
// The storm accrues on the server alone, so a client's own subsystem holds an
// honest zero and `ALLKGameState` carries what clients may know. Codex's
// review asked for exactly this, and his reason is the one that decides what
// is worth asserting here: M1.9 is a two-machine test, and a two-machine test
// where the client's screen shows nothing cannot verify the lantern/storm
// relationship from the side that matters.
//
// So the property under test is **agreement**. A client is not shown a
// countdown the server computed; it computes its own from the replicated
// meters and its own copy of `DA_StormCharge`. That is cheaper on the wire and
// smoother on screen, but it only works if both sides reach the same number -
// and nothing about the two code paths being "the same function" guarantees
// that, because the inputs differ. The server projects from `WardenCharge`;
// the client projects from `Snapshot.WardenCharge`. This file is where those
// are shown to be the same thing.
//
// What would go wrong without it, in order of how badly:
//
//   1. **The snapshot measuring the wrong meter.** When the Warden is next,
//      `NextFromCharge` must be the Warden's charge, not the thresholds'.
//      They differ by the whole of row 54's dampening, and the error points
//      the reassuring way - a lit crew would read as safer than it is.
//   2. **A default snapshot rendering as a real meter.** Field for field, a
//      default snapshot is identical to a genuine start of night, and they
//      mean opposite things. `bPublished` is the only thing separating them.
//   3. **A client being handed a source it did not read.** The snapshot
//      carries the SERVER's `RulesSource`, because which data the night runs
//      on is not the same question as which data this machine loaded.

#include "Data/LLKStormChargeData.h"
#include "Lantern/LLKLanternComponent.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKStormChargeSubsystem.h"
#include "Storm/LLKStormSnapshot.h"
#include "UI/LLKDebugHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKStormSnapshotTest,
	"LLK.Storm.Snapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKStormSnapshotTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK storm snapshot test reached RunTest."));

	// ---- 1. a default snapshot is not a meter --------------------------
	//
	// Failure mode 2, and the cheapest one to get wrong. Every field below
	// is also what a real night starts at.
	const FLLKStormSnapshot Fresh;
	TestFalse(TEXT("a default snapshot is not published"), Fresh.bPublished);
	TestFalse(TEXT("and claims nothing is coming"), Fresh.bHasNextEscalation);
	TestEqual(TEXT("and its charge is zero, like a real start of night"), Fresh.Charge, 0.f);
	TestEqual(TEXT("so bPublished is the ONLY thing separating the two"),
		Fresh.RulesSource, ELLKDataSource::Unresolved);

	// ---- the server ----------------------------------------------------
	ULLKStormChargeSubsystem* Server = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	ULLKStormChargeData* Rules = ULLKStormChargeSubsystem::MakeFallbackRules(Server);
	Server->SetRulesForTest(Rules);

	// A client: a second subsystem with the SAME rules asset and no meters of
	// its own, which is exactly a client's situation - it loads
	// DA_StormCharge in Initialize like the server does, and accrues nothing.
	ULLKStormChargeSubsystem* Client = NewObject<ULLKStormChargeSubsystem>(GetTransientPackage());
	Client->SetRulesForTest(Rules);

	// ---- 2. the snapshot agrees with the meter it was built from --------
	Server->AdvanceForTest(120.f, 1);
	FLLKStormSnapshot Snapshot = Server->BuildSnapshot();

	TestTrue(TEXT("a built snapshot is published"), Snapshot.bPublished);
	TestEqual(TEXT("charge matches the server"), Snapshot.Charge, Server->GetCharge());
	TestEqual(TEXT("the Warden's charge matches the server"),
		Snapshot.WardenCharge, Server->GetWardenCharge());
	TestEqual(TEXT("elapsed matches the server"), Snapshot.ElapsedSeconds, Server->GetElapsedSeconds());
	TestEqual(TEXT("the multiplier matches the server"),
		Snapshot.Multiplier, Server->GetCurrentMultiplier());
	TestEqual(TEXT("the Warden's multiplier matches the server"),
		Snapshot.WardenMultiplier, Server->GetCurrentWardenMultiplier());
	TestEqual(TEXT("the source is the server's, not a guess"),
		Snapshot.RulesSource, Server->GetRulesSource());

	// The two meters must have genuinely diverged by now, or the assertions
	// below could pass by both being the same number.
	TestTrue(TEXT("by now the two meters disagree, so the test can tell them apart"),
		Snapshot.Charge > Snapshot.WardenCharge + 1.f);

	// ---- 3. THE AGREEMENT. A client reaches the server's answer ---------
	//
	// This is the assertion M1.9 rests on. The client projects from the
	// snapshot; the server projects from its own live meters; both must land
	// on the same second.
	for (int32 Lit = 0; Lit <= 4; ++Lit)
	{
		const float ServerSeconds = Server->SecondsUntilSnapshotEscalation(Server->BuildSnapshot(), Lit);
		const float ClientSeconds = Client->SecondsUntilSnapshotEscalation(Snapshot, Lit);
		TestNearlyEqual(
			*FString::Printf(TEXT("client and server agree on the countdown at %d lantern(s)"), Lit),
			ClientSeconds, ServerSeconds, 0.01f);

		// And the two render identically, which is what the player compares.
		TestEqual(
			*FString::Printf(TEXT("and render the same text at %d lantern(s)"), Lit),
			ALLKDebugHUD::FormatSeconds(ClientSeconds),
			ALLKDebugHUD::FormatSeconds(ServerSeconds));
	}

	// The counterfactual too, since it is the line that teaches. A client
	// whose "if you light one lantern" disagreed with the server's would be
	// teaching a lesson the server does not enforce.
	TestNearlyEqual(TEXT("client and server agree on the dark counterfactual"),
		Client->SecondsUntilSnapshotEscalation(Snapshot, 0),
		Server->SecondsUntilSnapshotEscalation(Server->BuildSnapshot(), 0), 0.01f);

	// ---- 4. FAILURE MODE 1: which meter the next escalation is measured on
	//
	// While a threshold is next, it is measured against the thresholds'
	// charge.
	TestEqual(TEXT("with a threshold next, NextFromCharge is the thresholds' meter"),
		Snapshot.NextFromCharge, Snapshot.Charge);

	// Cross both thresholds so the Warden becomes next, and the measured
	// meter must switch with it. Getting this wrong is wrong by the whole of
	// the dampening, and wrong in the reassuring direction.
	Server->AdvanceForTest(500.f, 0);
	Snapshot = Server->BuildSnapshot();
	TestEqual(TEXT("the Warden is now next"),
		Snapshot.NextEscalationId, FName(TEXT("WardenConverges")));
	TestEqual(TEXT("and NextFromCharge switched to the Warden's meter"),
		Snapshot.NextFromCharge, Snapshot.WardenCharge);
	TestNotEqual(TEXT("which is NOT the thresholds' charge"),
		Snapshot.NextFromCharge, Snapshot.Charge);

	// A client projecting the Warden from that snapshot must still match.
	TestNearlyEqual(TEXT("client and server agree on the Warden's arrival"),
		Client->SecondsUntilSnapshotEscalation(Snapshot, 0),
		Server->SecondsUntilWarden(0), 0.01f);

	// ---- 5. nothing left to send, over the wire ------------------------
	Server->AdvanceForTest(1200.f, 0);
	Snapshot = Server->BuildSnapshot();
	TestTrue(TEXT("the Warden has converged"), Snapshot.bWardenConverged);
	TestFalse(TEXT("and the snapshot reports nothing still coming"), Snapshot.bHasNextEscalation);
	TestEqual(TEXT("so a client projects never, and renders a dash"),
		ALLKDebugHUD::FormatSeconds(Client->SecondsUntilSnapshotEscalation(Snapshot, 0)),
		FString(TEXT("--:--")));

	// ---- 6. the lit count, and the ghost light it must not have ---------
	//
	// Real registered components rather than AdvanceForTest's parameter,
	// because the count on the snapshot comes from the live recount - which
	// is the thing worth testing here. The subsystem recounts every tick
	// instead of tracking a counter precisely so that a keeper downed while
	// lit cannot leave light behind, and that property deserves an assertion
	// rather than a comment.
	ULLKLanternComponent* First = NewObject<ULLKLanternComponent>(GetTransientPackage());
	ULLKLanternComponent* Second = NewObject<ULLKLanternComponent>(GetTransientPackage());
	ULLKLanternComponent* Third = NewObject<ULLKLanternComponent>(GetTransientPackage());
	for (ULLKLanternComponent* Lantern : {First, Second, Third})
	{
		Server->RegisterLantern(Lantern);
		Lantern->ApplyStateForTest(true, ELLKLanternChangeReason::PlayerToggled);
	}

	TestEqual(TEXT("three lit lanterns replicate as three"),
		int32(Server->BuildSnapshot().LitLanternCount), 3);

	// Registering the same lantern twice must not count it twice. Under the
	// diminishing curve that is not merely double - it is a different point
	// on the curve entirely.
	Server->RegisterLantern(First);
	TestEqual(TEXT("registering one twice still counts it once"),
		int32(Server->BuildSnapshot().LitLanternCount), 3);

	// One keeper puts their lantern out.
	Second->ApplyStateForTest(false, ELLKLanternChangeReason::PlayerToggled);
	TestEqual(TEXT("putting one out drops the count"),
		int32(Server->BuildSnapshot().LitLanternCount), 2);

	// NO GHOST LIGHT. A keeper destroyed while still lit - downed, torn down,
	// disconnected - must stop accelerating the storm. The subsystem holds
	// weak pointers and recounts, so a stale entry resolves to null rather
	// than to a lantern that is lit forever.
	TestTrue(TEXT("the third lantern is still lit before it is destroyed"), Third->IsLanternOn());
	Third->MarkAsGarbage();
	TestEqual(TEXT("a lantern destroyed while lit leaves no light behind"),
		int32(Server->BuildSnapshot().LitLanternCount), 1);

	// And the surviving one still counts, so the recount is not simply
	// returning zero once anything goes stale.
	TestTrue(TEXT("the surviving lit lantern still counts"), First->IsLanternOn());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
