// Light the crew paid for rather than chose. M3, row 75.
//
// **This code had no test at all until now, which is worth saying plainly.**
// The distress pulse arrived with row 72 as the anchor revive's new price and
// was exercised only through the revive path, which needs a world. Row 75 made
// it load-bearing twice over: the extraction rig's work lights are a pulse,
// and they are a pulse that gets re-armed every tick for as long as a recovery
// operation runs.
//
// **The rule the refresh exists for is Chris's, and it is the whole reason a
// second entry point was worth adding.** His lock on the paused hold: *"the
// siren, rig lights, and converged patrols persist, so stepping away buys
// survival time, not a free reset or a safer retry."* A pulse lit once for
// siren-plus-hold seconds cannot do that, because a paused hold has no
// knowable length - the lights would go out underneath a long pause and make
// hiding the safe play. So the rig re-arms a short pulse while it is working.
//
// Which creates exactly one new way to be wrong, and it is the one this file
// mostly guards: re-arming must REPLACE, never stack. A pulse added per frame
// would be sixty lights a second, each one a `FLLKLightSource` the Warden's
// brightest-position search walks.
//
// It needs no world. `AddDistressPulse` and `Advance` both work on a
// `NewObject` subsystem, which is what made the storm rules testable in the
// first place.

#include "Data/LLKZonePressureData.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKLightField.h"
#include "Storm/LLKZonePressureSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace LLKDistressPulseTestFixture
{
	ULLKZonePressureSubsystem* MakeClock()
	{
		ULLKZonePressureSubsystem* Clock = NewObject<ULLKZonePressureSubsystem>(GetTransientPackage());
		ULLKZonePressureData* Rules = ULLKZonePressureSubsystem::MakeFallbackRules(Clock);
		Rules->BaselineRatePerSecond = 1.f;
		Clock->SetRulesForTest(Rules);
		return Clock;
	}

	const FName Rig(TEXT("ExtractionRig"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKDistressPulseTest,
	"LLK.Storm.DistressPulse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKDistressPulseTest::RunTest(const FString& Parameters)
{
	using namespace LLKDistressPulseTestFixture;

	// ---- 1. a pulse is a light in the field, and then it is not ----------
	{
		ULLKZonePressureSubsystem* Clock = MakeClock();
		TestEqual(TEXT("no pulses to begin with"), Clock->GetDistressPulseCount(), 0);

		Clock->AddDistressPulse(FVector(100.f, 0.f, 0.f), 1200.f, 900.f, 8.f);
		TestEqual(TEXT("the revive's flare is lit"), Clock->GetDistressPulseCount(), 1);

		// It is a source exactly like a lantern is, which is what makes "wakes
		// up the room" work with no enemy code: the Warden's search and
		// perception's lit test both read this list.
		const TArray<FLLKLightSource> Sources = Clock->GetLitLightSources();
		TestEqual(TEXT("and it is in the light field"), Sources.Num(), 1);
		TestEqual(TEXT("with the brightness it was given"), Sources[0].Intensity, 1200.f);
		TestEqual(TEXT("and the reach it was given"), Sources[0].RadiusCm, 900.f);

		// Row 72's price is eight seconds, not forever.
		Clock->AdvanceForTest(7.9f);
		TestEqual(TEXT("it is still lit a tenth of a second before it ends"),
			Clock->GetDistressPulseCount(), 1);
		Clock->AdvanceForTest(0.2f);
		TestEqual(TEXT("and gone after its duration"), Clock->GetDistressPulseCount(), 0);
		TestEqual(TEXT("leaving nothing in the light field"),
			Clock->GetLitLightSources().Num(), 0);
	}

	// ---- 2. a pulse with nothing to it is refused rather than added ------
	{
		ULLKZonePressureSubsystem* Clock = MakeClock();

		AddExpectedError(TEXT("Distress pulse refused"), EAutomationExpectedErrorFlags::Contains, 0);

		Clock->AddDistressPulse(FVector::ZeroVector, 0.f, 900.f, 8.f);
		Clock->AddDistressPulse(FVector::ZeroVector, 1200.f, 0.f, 8.f);
		Clock->AddDistressPulse(FVector::ZeroVector, 1200.f, 900.f, 0.f);
		TestEqual(TEXT("a pulse with no brightness, reach or duration is not a pulse"),
			Clock->GetDistressPulseCount(), 0);
	}

	// ---- 3. ROW 75: a named pulse refreshes, and never stacks ------------
	//
	// The one new way to be wrong, and the reason this file exists.
	{
		ULLKZonePressureSubsystem* Clock = MakeClock();

		for (int32 Frame = 0; Frame < 600; ++Frame)
		{
			Clock->AddOrRefreshDistressPulse(Rig, FVector(Frame * 10.f, 0.f, 0.f), 1200.f, 900.f, 1.f);
			Clock->AdvanceForTest(1.f / 60.f);
		}

		TestEqual(TEXT("ROW 75: ten seconds of re-arming is one light, not six hundred"),
			Clock->GetDistressPulseCount(), 1);
		TestEqual(TEXT("and one source in the field"), Clock->GetLitLightSources().Num(), 1);

		// It follows the operation. The dock is where the light is, and if the
		// route anchor ever moves the pulse goes with it.
		const TArray<FLLKLightSource> Sources = Clock->GetLitLightSources();
		TestEqual(TEXT("at the position of the most recent refresh"),
			static_cast<float>(Sources[0].Position.X), 5990.f);

		// **The persistence Chris locked.** Stop refreshing - the crew ducked
		// behind a shelf, or the extraction ended - and the light runs out
		// one refresh later rather than blinking off at that instant.
		Clock->AdvanceForTest(0.5f);
		TestEqual(TEXT("ROW 75: the lights do not go out the moment the rig stops being told to shine"),
			Clock->GetDistressPulseCount(), 1);
		Clock->AdvanceForTest(0.6f);
		TestEqual(TEXT("and they are out a refresh later"), Clock->GetDistressPulseCount(), 0);
	}

	// ---- 4. an unnamed refreshable pulse is the stacking bug -------------
	{
		ULLKZonePressureSubsystem* Clock = MakeClock();

		AddExpectedError(TEXT("A refreshable pulse needs a source name"), EAutomationExpectedErrorFlags::Contains, 0);

		Clock->AddOrRefreshDistressPulse(NAME_None, FVector::ZeroVector, 1200.f, 900.f, 1.f);
		TestEqual(TEXT("ROW 75: a refreshable pulse with no owner is refused"),
			Clock->GetDistressPulseCount(), 0);
	}

	// ---- 5. two owners are two lights, and the anchor revive is its own --
	//
	// A keeper reviving at the dock during an extraction is the case: both
	// lights are real and the building should see both.
	{
		ULLKZonePressureSubsystem* Clock = MakeClock();

		Clock->AddOrRefreshDistressPulse(Rig, FVector(0.f, 0.f, 0.f), 1200.f, 900.f, 1.f);
		Clock->AddDistressPulse(FVector(500.f, 0.f, 0.f), 1200.f, 900.f, 8.f);
		TestEqual(TEXT("the rig's light and a revive's flare are two lights"),
			Clock->GetDistressPulseCount(), 2);

		// Refreshing the rig re-arms the rig and leaves the revive alone -
		// which is the part a single shared array could get wrong.
		Clock->AddOrRefreshDistressPulse(Rig, FVector(0.f, 0.f, 0.f), 1200.f, 900.f, 1.f);
		TestEqual(TEXT("and refreshing one does not touch the other"),
			Clock->GetDistressPulseCount(), 2);

		// Two seconds on: the rig's one-second pulse is gone and the revive's
		// eight-second flare is not.
		Clock->AdvanceForTest(2.f);
		TestEqual(TEXT("they expire on their own schedules"), Clock->GetDistressPulseCount(), 1);
		TestEqual(TEXT("and the one left is the revive's, at its own position"),
			static_cast<float>(Clock->GetLitLightSources()[0].Position.X), 500.f);
	}

	// ---- 6. ROW 73: none of this moves the clock -------------------------
	//
	// The whole point of the pulse as a price: it is light, which is local.
	// Row 72 left light allowed to do exactly this and nothing else.
	{
		ULLKZonePressureSubsystem* Clock = MakeClock();
		Clock->AdvanceForTest(10.f);
		const float Before = Clock->GetPressure();

		for (int32 Frame = 0; Frame < 120; ++Frame)
		{
			Clock->AddOrRefreshDistressPulse(Rig, FVector::ZeroVector, 5000.f, 5000.f, 1.f);
		}
		Clock->AddDistressPulse(FVector::ZeroVector, 5000.f, 5000.f, 8.f);

		TestEqual(TEXT("ROW 73: no amount of light moves the clock"), Clock->GetPressure(), Before);
		TestEqual(TEXT("and the clock is still just elapsed seconds"),
			Clock->GetPressure(), Clock->GetElapsedSeconds());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
