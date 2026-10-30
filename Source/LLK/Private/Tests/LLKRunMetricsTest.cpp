// What a run was, as arithmetic. M4's instrument.
//
// **Why the metrics are tested rather than trusted.** These numbers decide
// whether M4 passes, and the roadmap is explicit that M4 is the gate: *"do
// testers kill their lanterns, bank, and argue for one more room? Four real
// testers, instrumented."* A metric that is subtly wrong is worse than no
// metric, because it is an argument that sounds like evidence - and the
// specific way these would be wrong is quiet. A lit clock that double-counts
// overlapping lanterns reports 140% of a run spent lit and nobody notices
// until somebody reads the file.
//
// So the four that could be quietly wrong, each with a case:
//
//   1. **Overlapping lanterns.** Four keepers, four lights, in and out of each
//      other's intervals. Lit seconds are seconds with at least one burning,
//      which is depth-counting rather than a bool.
//   2. **Unclosed intervals.** A lantern still on when the run ends, and a
//      hold still stalled when the crew went down on the dock.
//   3. **Row 8.** A wipe keeps nothing, whatever the crew was carrying -
//      asserted here as arithmetic rather than trusted to the caller.
//   4. **One ending.** A run cannot be both secured and wiped, and whichever
//      came first is the one that counts.

#include "Misc/AutomationTest.h"
#include "Run/LLKRunMetrics.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKRunMetricsTestFixture
{
	FLLKRunEvent At(const float Seconds, const ELLKRunEvent Kind,
		const FName Detail = NAME_None, const float Value = 0.f)
	{
		FLLKRunEvent Event;
		Event.AtSeconds = Seconds;
		Event.Kind = Kind;
		Event.Detail = Detail;
		Event.Value = Value;
		return Event;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKRunMetricsTest,
	"LLK.Run.Metrics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKRunMetricsTest::RunTest(const FString& Parameters)
{
	using namespace LLKRunMetricsTestFixture;

	// ---- 1. nothing happened -------------------------------------------
	{
		const FLLKRunSummary Empty = LLKRunMetrics::Summarise({});
		TestEqual(TEXT("an empty run is unresolved rather than failed"),
			FString(LLKRunMetrics::NameOf(Empty.Outcome)), FString(TEXT("Unresolved")));
		TestEqual(TEXT("and has no length"), Empty.RunSeconds, 0.f);
		TestEqual(TEXT("and no call"), Empty.ExtractionCalledSeconds, -1.f);

		// The description must still read as something rather than crashing or
		// printing a blank: a run nobody finished is a real playtest result.
		TestTrue(TEXT("and still describes itself"),
			LLKRunMetrics::Describe(Empty).Contains(TEXT("NEVER CALLED")));
	}

	// ---- 2. ROW 3's first question: do they kill their lanterns? --------
	//
	// A ten-minute run with one lantern on for two minutes of it.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(60.f, ELLKRunEvent::LanternOn));
		Events.Add(At(180.f, ELLKRunEvent::LanternOff));
		Events.Add(At(600.f, ELLKRunEvent::CrewWiped));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestNearlyEqual(TEXT("two minutes of light"), Summary.LitSeconds, 120.f, 0.01f);
		TestNearlyEqual(TEXT("which is a fifth of the run"), Summary.LitFraction, 0.2f, 0.001f);
		TestNearlyEqual(TEXT("and the run is as long as its last event"),
			Summary.RunSeconds, 600.f, 0.01f);
	}

	// ---- 3. overlapping lanterns are not double-counted -----------------
	//
	// The quiet failure: a crew of four whose lights overlap would otherwise
	// report more lit seconds than the run has, and a `litFraction` above 1
	// in a file nobody reads twice.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(10.f, ELLKRunEvent::LanternOn));   // one light
		Events.Add(At(20.f, ELLKRunEvent::LanternOn));   // two
		Events.Add(At(30.f, ELLKRunEvent::LanternOff));  // one again
		Events.Add(At(40.f, ELLKRunEvent::LanternOff));  // dark
		Events.Add(At(100.f, ELLKRunEvent::ExtractionSecured, TEXT("LoadingDock"), 2.f));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestNearlyEqual(TEXT("overlapping lanterns are thirty lit seconds, not forty"),
			Summary.LitSeconds, 30.f, 0.01f);
		TestTrue(TEXT("and a lit fraction can never exceed one"), Summary.LitFraction <= 1.f);
	}

	// ---- 4. a lantern still burning when the run ends -------------------
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(30.f, ELLKRunEvent::LanternOn));
		Events.Add(At(90.f, ELLKRunEvent::CrewWiped));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestNearlyEqual(TEXT("an unclosed lantern interval runs to the end of the run"),
			Summary.LitSeconds, 60.f, 0.01f);
	}

	// ---- 5. ROW 8: a wipe keeps nothing ---------------------------------
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(100.f, ELLKRunEvent::RelicTaken));
		Events.Add(At(140.f, ELLKRunEvent::RelicTaken));
		Events.Add(At(200.f, ELLKRunEvent::RelicTaken));
		Events.Add(At(300.f, ELLKRunEvent::CrewWiped));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestEqual(TEXT("three relics taken"), Summary.RelicsTaken, 3);
		TestEqual(TEXT("ROW 8: and none of them out"), Summary.RelicsSecured, 0);
		TestEqual(TEXT("the outcome is a wipe"),
			FString(LLKRunMetrics::NameOf(Summary.Outcome)), FString(TEXT("Wiped")));
	}

	// ---- 6. the climax, which is Chris's actual question about the dock --
	//
	// *"whether extraction is a tense decision or twenty seconds of waiting."*
	// A run that is 95% looting has a hold nobody thought about.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(480.f, ELLKRunEvent::ExtractionCalled, TEXT("LoadingDock"), 8.f));
		Events.Add(At(488.f, ELLKRunEvent::RigArrived, TEXT("LoadingDock")));
		Events.Add(At(508.f, ELLKRunEvent::ExtractionSecured, TEXT("LoadingDock"), 4.f));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestNearlyEqual(TEXT("the call is on the record to the second"),
			Summary.ExtractionCalledSeconds, 480.f, 0.01f);
		TestNearlyEqual(TEXT("and the climax is the last twenty-eight seconds of it"),
			Summary.ClimaxFraction, 28.f / 508.f, 0.001f);
		TestEqual(TEXT("four relics out"), Summary.RelicsSecured, 4);
		TestEqual(TEXT("and the run was secured"),
			FString(LLKRunMetrics::NameOf(Summary.Outcome)), FString(TEXT("Secured")));
	}

	// ---- 7. ROW 75's pause, as two numbers ------------------------------
	//
	// How often the dock emptied and for how long. The thing no self-report
	// would ever produce accurately.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(100.f, ELLKRunEvent::ExtractionCalled, TEXT("LoadingDock"), 8.f));
		Events.Add(At(108.f, ELLKRunEvent::RigArrived, TEXT("LoadingDock")));
		Events.Add(At(115.f, ELLKRunEvent::HoldStalled));
		Events.Add(At(120.f, ELLKRunEvent::HoldResumed));
		Events.Add(At(125.f, ELLKRunEvent::HoldStalled));
		Events.Add(At(135.f, ELLKRunEvent::HoldResumed));
		Events.Add(At(150.f, ELLKRunEvent::ExtractionSecured, TEXT("LoadingDock"), 1.f));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestEqual(TEXT("the dock emptied twice"), Summary.HoldStallCount, 2);
		TestNearlyEqual(TEXT("for fifteen seconds in total"),
			Summary.HoldStallSeconds, 15.f, 0.01f);
	}

	// ---- 8. a crew that went down holding the dock ----------------------
	//
	// The stall never resumed, which is the most informative run there is: it
	// must count to the end rather than be dropped for having no closing event.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(60.f, ELLKRunEvent::ExtractionCalled, TEXT("LoadingDock"), 8.f));
		Events.Add(At(68.f, ELLKRunEvent::RigArrived, TEXT("LoadingDock")));
		Events.Add(At(75.f, ELLKRunEvent::KeeperDowned));
		Events.Add(At(75.f, ELLKRunEvent::HoldStalled));
		Events.Add(At(95.f, ELLKRunEvent::CrewWiped));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestEqual(TEXT("one knockdown"), Summary.KnockdownCount, 1);
		TestEqual(TEXT("one stall"), Summary.HoldStallCount, 1);
		TestNearlyEqual(TEXT("and it lasted until the run ended"),
			Summary.HoldStallSeconds, 20.f, 0.01f);
	}

	// ---- 9. the argument: called, cancelled, called again ---------------
	//
	// The closest honest proxy for *"argue for one more room"*, which is the
	// one question in M4's gate that cannot be measured directly.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(240.f, ELLKRunEvent::ExtractionCalled, TEXT("LoadingDock"), 8.f));
		Events.Add(At(250.f, ELLKRunEvent::HoldStalled));
		Events.Add(At(255.f, ELLKRunEvent::ExtractionCancelled, TEXT("LoadingDock")));
		Events.Add(At(570.f, ELLKRunEvent::ExtractionCalled, TEXT("LoadingDock"), 8.f));
		Events.Add(At(600.f, ELLKRunEvent::ExtractionSecured, TEXT("LoadingDock"), 6.f));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestEqual(TEXT("one cancel is on the record"), Summary.CancelCount, 1);
		TestNearlyEqual(TEXT("and the FIRST call is the one kept"),
			Summary.ExtractionCalledSeconds, 240.f, 0.01f);

		// A cancel closes an open stall, so the five seconds between them
		// count and nothing runs to the end of the run.
		TestNearlyEqual(TEXT("a cancel closes the stall it was in"),
			Summary.HoldStallSeconds, 5.f, 0.01f);
	}

	// ---- 10. one ending, whichever came first ---------------------------
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(100.f, ELLKRunEvent::RelicTaken));
		Events.Add(At(200.f, ELLKRunEvent::ExtractionSecured, TEXT("LoadingDock"), 1.f));

		// The subsystem refuses to record this, and the arithmetic refuses to
		// be fooled by it if something ever does: a wipe after a secure must
		// not erase a haul that already left the building.
		Events.Add(At(220.f, ELLKRunEvent::CrewWiped));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestEqual(TEXT("a late wipe does not un-secure a haul"), Summary.RelicsSecured, 0);
		TestEqual(TEXT("though it does change the outcome, which is why the subsystem refuses it"),
			FString(LLKRunMetrics::NameOf(Summary.Outcome)), FString(TEXT("Wiped")));
	}

	// ---- 11. the file is readable, and says what it measured ------------
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(12.f, ELLKRunEvent::LanternOn));
		Events.Add(At(40.f, ELLKRunEvent::ExtractionSecured, TEXT("LoadingDock"), 2.f));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		const FString Json = LLKRunMetrics::ToJson(Events, Summary);

		TestTrue(TEXT("the file carries the summary"), Json.Contains(TEXT("\"litFraction\"")));
		TestTrue(TEXT("and the events behind it"), Json.Contains(TEXT("\"kind\": \"LanternOn\"")));
		TestTrue(TEXT("and names the exit"), Json.Contains(TEXT("LoadingDock")));

		// No names, no ids, no machine. A run file describes a run.
		TestFalse(TEXT("and nothing about who was playing"),
			Json.Contains(TEXT("Player")) || Json.Contains(TEXT("user")));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
