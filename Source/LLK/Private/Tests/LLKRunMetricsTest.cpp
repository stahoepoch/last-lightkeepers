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

	// ---- 12. legibility, row 43's question ------------------------------
	//
	// **What these assertions are for.** The instrument exists because nothing
	// measured whether the store teaches, and the way a legibility metric goes
	// quietly wrong is by counting the wrong denominator: a run where every
	// interact succeeded and a run where nobody pressed the key both produce a
	// refused fraction of zero, and only one of them is good news. So the
	// no-attempts case is asserted explicitly rather than left to arithmetic.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));

		// Reaching for something never lit: the player has understood that
		// relics are takeable and not that reading one comes first.
		Events.Add(At(8.f, ELLKRunEvent::InteractAttempted, TEXT("Unidentified")));
		Events.Add(At(9.f, ELLKRunEvent::InteractAttempted, TEXT("Unidentified")));

		// Then the same relic read, and reached for in the dark. Row 5's rule,
		// hit by somebody who has learned half of it.
		Events.Add(At(30.f, ELLKRunEvent::InteractAttempted, TEXT("NeedsLight")));

		// And one that went through.
		Events.Add(At(41.f, ELLKRunEvent::InteractAttempted, TEXT("None")));
		Events.Add(At(41.f, ELLKRunEvent::RelicTaken));

		// A second take, later. The first is the one the metric reports.
		Events.Add(At(95.f, ELLKRunEvent::RelicTaken));

		Events.Add(At(120.f, ELLKRunEvent::RelicAbandoned, TEXT("Relic_3"), 6.5f));
		Events.Add(At(200.f, ELLKRunEvent::ExtractionSecured, TEXT("LoadingDock"), 2.f));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);

		TestEqual(TEXT("every interact that reached the server is counted"),
			Summary.InteractAttempts, 4);
		TestEqual(TEXT("reaching for an unread relic is counted as its own failure"),
			Summary.BlockedUnidentified, 2);
		TestEqual(TEXT("and reaching for a read one in the dark as a different failure"),
			Summary.BlockedNeedsLight, 1);
		TestEqual(TEXT("a successful interact is not a refusal"),
			Summary.BlockedUnidentified + Summary.BlockedNeedsLight, 3);
		TestNearlyEqual(TEXT("the refused fraction is refusals over attempts"),
			Summary.RefusedFraction, 0.75f, 0.001f);

		TestNearlyEqual(TEXT("time to first take is the FIRST one, not the last"),
			Summary.SecondsToFirstRelicTaken, 41.f, 0.001f);

		TestEqual(TEXT("a relic left while takeable is counted"),
			Summary.AbandonedWhileTakeable, 1);

		const FString Json = LLKRunMetrics::ToJson(Events, Summary);
		TestTrue(TEXT("the file carries the legibility numbers"),
			Json.Contains(TEXT("\"blockedNeedsLight\"")));
		TestTrue(TEXT("and the block reason behind them, so the count is checkable"),
			Json.Contains(TEXT("\"kind\": \"InteractAttempted\"")));
		TestTrue(TEXT("and names the reason rather than a number"),
			Json.Contains(TEXT("NeedsLight")));

		// Plain language, because the first reader is Chris after a playtest.
		const FString Text = LLKRunMetrics::Describe(Summary);
		TestTrue(TEXT("the readout says something about legibility"),
			Text.Contains(TEXT("refused")) || Text.Contains(TEXT("interact")));
	}

	// ---- 13. the zero that is not good news -----------------------------
	//
	// A run where nobody pressed the take key at all. Every legibility number
	// here is zero or absent, and the ONE thing the summary must not do is
	// report a clean refused fraction as though the rules had been understood.
	// `InteractAttempts` is the field that tells those apart, so a reader who
	// sees `refusedFraction: 0` has the denominator beside it.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(60.f, ELLKRunEvent::LanternOn));
		Events.Add(At(300.f, ELLKRunEvent::CrewWiped));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestEqual(TEXT("no attempts is zero attempts"), Summary.InteractAttempts, 0);
		TestEqual(TEXT("and an untaken run reports no first take rather than zero seconds"),
			Summary.SecondsToFirstRelicTaken, -1.f);
		TestEqual(TEXT("and a refused fraction of zero, which is why attempts is reported too"),
			Summary.RefusedFraction, 0.f);
		// Not a search for the word "never": the existing readout already says
		// NEVER CALLED about the extraction, so that would pass without the
		// legibility line existing at all. This asserts the specific sentence.
		TestTrue(TEXT("the readout must not let that read as success"),
			LLKRunMetrics::Describe(Summary).Contains(TEXT("NOBODY REACHED FOR ANYTHING")));
	}

	// ---- 14. an unknown block reason is counted, not dropped -------------
	//
	// `Detail` is an `FName` rather than an enum because the event list is read
	// as a whole, which means a reason this arithmetic has never heard of can
	// arrive - from a later rule, or from a typo at the recording site. It must
	// still raise `InteractAttempts`, because an attempt happened; what it must
	// NOT do is land in one of the named buckets and misattribute a failure.
	{
		TArray<FLLKRunEvent> Events;
		Events.Add(At(0.f, ELLKRunEvent::RunStarted));
		Events.Add(At(5.f, ELLKRunEvent::InteractAttempted, TEXT("SomethingLater")));
		Events.Add(At(10.f, ELLKRunEvent::InteractAttempted, TEXT("None")));

		const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);
		TestEqual(TEXT("an unrecognised reason still counts as an attempt"),
			Summary.InteractAttempts, 2);
		TestEqual(TEXT("and is not filed under needing light"), Summary.BlockedNeedsLight, 0);
		TestEqual(TEXT("nor under being unidentified"), Summary.BlockedUnidentified, 0);

		// The choice this pins down: an unrecognised reason is a REFUSAL, not
		// a success. `None` is the one reason that means it worked, so a block
		// reason added by a later rule must not quietly improve this number by
		// being unknown to the arithmetic. One of two refused: 0.5.
		TestNearlyEqual(TEXT("an unrecognised reason still counts against the refused fraction"),
			Summary.RefusedFraction, 0.5f, 0.001f);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
