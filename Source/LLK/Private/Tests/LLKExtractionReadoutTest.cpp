// The five things extraction says, and the four it must not. M3.
//
// Chris specified the wording and the placement together, which is why they
// are tested together: *"Put the actionable prompt in-world at the zone;
// reserve centre-screen messaging for the active climax."* Placement is a rule
// about which of two slots is filled, so it is as checkable as the text.
//
// **What is worth asserting here**, beyond the obvious that each state says
// its line:
//
//   1. **They never both speak.** The prompt is an offer and the centre is an
//      event. Once the climax is running the offer is gone, because calling
//      again is refused as `AlreadyUnderway` - a prompt still saying "call
//      Lanternhold" would be advertising a key that does nothing.
//   2. **An unpublished snapshot says nothing.** A default snapshot is
//      field-by-field identical to a genuine idle run, so before the server
//      speaks the prompt must not appear. The storm readout learned this the
//      same way.
//   3. **The stalled hold has no timer**, and that is deliberate rather than
//      forgotten: a paused countdown is a lie and a frozen one reads as a
//      broken HUD.
//   4. **The countdown rounds up.** An eight-second siren reading 0:07 the
//      instant it starts is the kind of thing nobody notices in a screenshot
//      and everybody notices in play.

#include "Misc/AutomationTest.h"
#include "UI/LLKExtractionReadout.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKExtractionReadoutTestFixture
{
	FLLKExtractionSnapshot Published(const ELLKExtractionPhase Phase)
	{
		FLLKExtractionSnapshot Snapshot;
		Snapshot.bPublished = true;
		Snapshot.Phase = Phase;
		Snapshot.PointId = TEXT("LoadingDock");
		return Snapshot;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKExtractionReadoutTest,
	"LLK.UI.ExtractionReadout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKExtractionReadoutTest::RunTest(const FString& Parameters)
{
	using namespace LLKExtractionReadoutTestFixture;

	// ---- 1. nothing to say, which is most of a run ----------------------
	{
		// Not at a zone, nothing called: the screen is clear. This is the
		// state the readout is in for nine minutes of a ten-minute run, and
		// the whole of Chris's "not another top-left debug paragraph".
		const FLLKExtractionReadout Quiet =
			LLKExtractionReadout::Describe(Published(ELLKExtractionPhase::Idle), false);
		TestFalse(TEXT("an idle run away from the dock says nothing at all"),
			Quiet.HasAnything());

		// Before the server has published, an idle snapshot and a
		// default-constructed one are identical. The prompt must not appear on
		// the strength of a struct nobody filled in.
		FLLKExtractionSnapshot Unpublished;
		Unpublished.Phase = ELLKExtractionPhase::Idle;
		const FLLKExtractionReadout Unknown = LLKExtractionReadout::Describe(Unpublished, true);
		TestFalse(TEXT("and an unpublished snapshot says nothing even at the dock"),
			Unknown.HasAnything());
	}

	// ---- 2. the offer, in the world, at the zone ------------------------
	{
		const FLLKExtractionReadout AtZone =
			LLKExtractionReadout::Describe(Published(ELLKExtractionPhase::Idle), true);

		TestTrue(TEXT("standing at a zone offers the call"),
			AtZone.ZonePrompt.Contains(TEXT("Hold [E]")));
		TestTrue(TEXT("and names who is being called"),
			AtZone.ZonePrompt.Contains(TEXT("call Lanternhold")));
		TestTrue(TEXT("and says nothing centre screen, because there is no climax yet"),
			AtZone.Centre.IsEmpty());
		TestFalse(TEXT("an offer is not urgent"), AtZone.bUrgent);
	}

	// ---- 3. the siren: an announcement with a countdown -----------------
	{
		FLLKExtractionSnapshot Snapshot = Published(ELLKExtractionPhase::Announcing);
		Snapshot.SirenSecondsRemaining = 8.f;

		const FLLKExtractionReadout Incoming = LLKExtractionReadout::Describe(Snapshot, false);
		TestEqual(TEXT("the siren announces the rig with its countdown"),
			Incoming.Centre, FString::Printf(TEXT("RECOVERY INCOMING \x2014 0:08")));
		TestFalse(TEXT("and it is an announcement rather than a demand"), Incoming.bUrgent);

		// **Standing at the zone during the siren offers nothing**, because
		// there is nothing to offer: a second call is refused as
		// AlreadyUnderway. The two slots never both speak.
		const FLLKExtractionReadout AtZone = LLKExtractionReadout::Describe(Snapshot, true);
		TestTrue(TEXT("and the offer is withdrawn once the rig is coming"),
			AtZone.ZonePrompt.IsEmpty());
		TestEqual(TEXT("while the centre line is unchanged by where the keeper stands"),
			AtZone.Centre, Incoming.Centre);
	}

	// ---- 4. the hold: a demand, with the time left ----------------------
	{
		FLLKExtractionSnapshot Snapshot = Published(ELLKExtractionPhase::Holding);
		Snapshot.HoldSecondsRequired = 20.f;
		Snapshot.HoldSecondsRemaining = 20.f;

		const FLLKExtractionReadout Holding = LLKExtractionReadout::Describe(Snapshot, true);
		TestEqual(TEXT("the hold demands the dock, with the time left"),
			Holding.Centre, FString::Printf(TEXT("HOLD THE DOCK \x2014 0:20")));
		TestTrue(TEXT("and it is urgent"), Holding.bUrgent);
		TestTrue(TEXT("and still offers nothing to press"), Holding.ZonePrompt.IsEmpty());

		// Four seconds in.
		Snapshot.HoldSecondsRemaining = 16.f;
		TestEqual(TEXT("the countdown follows the hold"),
			LLKExtractionReadout::Describe(Snapshot, true).Centre,
			FString::Printf(TEXT("HOLD THE DOCK \x2014 0:16")));
	}

	// ---- 5. ROW 75: the empty dock, and why it has no timer -------------
	{
		FLLKExtractionSnapshot Snapshot = Published(ELLKExtractionPhase::Holding);
		Snapshot.HoldSecondsRequired = 20.f;
		Snapshot.HoldSecondsRemaining = 5.f;
		Snapshot.bHoldStalled = true;

		const FLLKExtractionReadout Stalled = LLKExtractionReadout::Describe(Snapshot, false);
		TestTrue(TEXT("ROW 75: an empty dock says the rig is holding, not failing"),
			Stalled.Centre.Contains(TEXT("RECOVERY HOLDING")));
		TestTrue(TEXT("and tells the crew what to do about it"),
			Stalled.Centre.Contains(TEXT("return to the dock")));
		TestTrue(TEXT("and it is urgent"), Stalled.bUrgent);

		// The assertion that is really about honesty: the hold is paused, so
		// there is no number on this line. A countdown would be false and a
		// frozen one would read as a broken HUD.
		TestFalse(TEXT("ROW 75: and carries no countdown, because a paused one would be a lie"),
			Stalled.Centre.Contains(TEXT("0:05")) || Stalled.Centre.Contains(TEXT(":")));
	}

	// ---- 6. secured, and failed saying nothing --------------------------
	{
		const FLLKExtractionReadout Out =
			LLKExtractionReadout::Describe(Published(ELLKExtractionPhase::Secured), false);
		TestEqual(TEXT("one word for the only way to keep a haul"),
			Out.Centre, FString(TEXT("SECURED")));

		// Even standing in the bay afterwards: there is nothing left to call.
		const FLLKExtractionReadout StillThere =
			LLKExtractionReadout::Describe(Published(ELLKExtractionPhase::Secured), true);
		TestTrue(TEXT("and no offer to call again"), StillThere.ZonePrompt.IsEmpty());

		// A wipe has its own centre-screen message. Two of them at once is the
		// paragraph Chris asked this not to become.
		const FLLKExtractionReadout Lost =
			LLKExtractionReadout::Describe(Published(ELLKExtractionPhase::Failed), true);
		TestFalse(TEXT("a failed extraction says nothing, because CREW CONTAINED already does"),
			Lost.HasAnything());
	}

	// ---- 7. the countdown's arithmetic ---------------------------------
	{
		TestEqual(TEXT("eight seconds reads 0:08"),
			LLKExtractionReadout::FormatSeconds(8.f), FString(TEXT("0:08")));

		// Rounds UP. The instant an eight-second siren starts it has 7.99
		// seconds left, and it must not read 0:07.
		TestEqual(TEXT("and 7.99 still reads 0:08 rather than 0:07"),
			LLKExtractionReadout::FormatSeconds(7.99f), FString(TEXT("0:08")));
		TestEqual(TEXT("a tenth of a second left is still a second on screen"),
			LLKExtractionReadout::FormatSeconds(0.1f), FString(TEXT("0:01")));
		TestEqual(TEXT("and nothing left is 0:00"),
			LLKExtractionReadout::FormatSeconds(0.f), FString(TEXT("0:00")));
		TestEqual(TEXT("a negative or garbage duration is 0:00, not a minus sign"),
			LLKExtractionReadout::FormatSeconds(-4.f), FString(TEXT("0:00")));

		// Over a minute, for the freight lift's thirty and for anything a
		// later zone authors.
		TestEqual(TEXT("thirty seconds reads 0:30"),
			LLKExtractionReadout::FormatSeconds(30.f), FString(TEXT("0:30")));
		TestEqual(TEXT("and ninety reads 1:30"),
			LLKExtractionReadout::FormatSeconds(90.f), FString(TEXT("1:30")));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
