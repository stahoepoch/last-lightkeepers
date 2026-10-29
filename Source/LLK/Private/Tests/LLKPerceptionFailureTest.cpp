// The ways perception must fail, and the one way light travels between keepers. M2.9.
//
// `LLK.Enemies.Perception` walks the happy path of each sense. This file is
// the other half, and Chris named its shape on 2026-10-29: *"Focus the tests
// on failure-shaped behavior, not more happy paths."* His six, in his order,
// are the six sections below.
//
// **Two of them are not restatements of the rule, they are the rule meeting
// another system.** Section 1 runs light through the real
// `LLKLightField::BrightnessAt` rather than handing perception a number, so
// "a lit teammate exposes a dark one" is asserted through the code that
// actually computes it - which is the only version of that claim worth
// having, because the bug it guards against is perception reading a bool
// somewhere instead. Section 6 does the same for the Warden: its convergence
// must come from the storm and the brightest position, and asserting that
// beside its perception returning `StormAuthority` is what shows the two
// paths are genuinely separate rather than one feeding the other.
//
// **Writing section 5 found a real flaw in the rule as shipped.** A malformed
// family - a Sweeper that lost one of its two radii - has no reaching sense,
// and `SenseReaching` read "no sense" as "must be the Warden", returning
// `StormAuthority`. That is the most dangerous possible answer: broken data
// would have behaved like the one enemy that cannot be escaped. The Warden is
// now named rather than inferred, and `LLK.Enemies.Perception`'s assertion
// that encoded the old behaviour was flipped. Chris's own list is what
// surfaced it, which is the argument for writing failure tests from somebody
// else's list rather than from your own.

#include "Data/LLKEnemyDef.h"
#include "Enemies/LLKPerception.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKLightField.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	/** `data/lantern.json` `light`: 250 candelas over 30 studs. */
	constexpr float LanternIntensity = 250.f;
	constexpr float LanternRadiusStuds = 30.f;

	FLLKLightSource LanternAt(const FVector& Position)
	{
		FLLKLightSource Source;
		Source.Position = Position;
		Source.Intensity = LanternIntensity;
		Source.RadiusCm = LLKUnits::DistanceFromStuds(LanternRadiusStuds);
		return Source;
	}

	FVector StudsAlongX(float Studs) { return FVector(LLKUnits::DistanceFromStuds(Studs), 0.f, 0.f); }

	FLLKEnemyDef MakeSweeper()
	{
		FLLKEnemyDef Def;
		Def.Family = ELLKEnemyFamily::Sweepers;
		Def.AggroRadiusDark = 12.f;
		Def.AggroRadiusLit = 38.f;
		return Def;
	}

	FLLKEnemyDef MakeStocker()
	{
		FLLKEnemyDef Def;
		Def.Family = ELLKEnemyFamily::Stockers;
		Def.SightRange = 70.f;
		Def.SightConeDegrees = 70.f;
		return Def;
	}

	FLLKEnemyDef MakeWalker()
	{
		FLLKEnemyDef Def;
		Def.Family = ELLKEnemyFamily::WindowWalkers;
		Def.WakeRange = 32.f;
		Def.SightRange = 36.f;
		Def.bWakeRequiresActiveLantern = true;
		return Def;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKPerceptionFailureTest,
	"LLK.Enemies.PerceptionFailures",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKPerceptionFailureTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK perception failure test reached RunTest."));

	const float Threshold = LLKPerception::LitThresholdFrom(LanternIntensity);

	// =====================================================================
	// 1. A LIT TEAMMATE CAN EXPOSE A DARK TEAMMATE
	// =====================================================================
	//
	// The whole of co-op light in one assertion pair, run through the real
	// field. Keeper B never lights anything; whether B is noticed depends
	// entirely on what A decided.
	{
		const FVector KeeperA = FVector::ZeroVector;
		const FVector KeeperB = StudsAlongX(10.f);

		// The Sweeper sits 20 studs from B: outside its 12-stud dark radius,
		// inside its 38-stud lit one. So B's exposure is decided by light and
		// by nothing else.
		FLLKPerceiver Sweeper;
		Sweeper.Position = KeeperB + FVector(0.f, LLKUnits::DistanceFromStuds(20.f), 0.f);
		Sweeper.Facing = FVector::ForwardVector;

		const FLLKEnemyDef SweeperDef = MakeSweeper();

		// --- A's lantern on -------------------------------------------
		const TArray<FLLKLightSource> ALit = { LanternAt(KeeperA) };

		FLLKPerceivedKeeper B;
		B.Position = KeeperB;
		B.Brightness = LLKLightField::BrightnessAt(ALit, KeeperB);

		TestTrue(TEXT("a dark keeper 10 studs from a lit teammate is measurably lit"),
			B.Brightness > Threshold);
		TestEqual(TEXT("so a Sweeper 20 studs away notices them, on a lantern they never lit"),
			LLKPerception::SenseReaching(SweeperDef, Sweeper, B, Threshold),
			ELLKNoticeSense::LitProximity);

		// --- A's lantern off, nothing else changed --------------------
		const TArray<FLLKLightSource> AllDark;
		FLLKPerceivedKeeper BDark = B;
		BDark.Brightness = LLKLightField::BrightnessAt(AllDark, KeeperB);

		TestEqual(TEXT("with the crew dark that same keeper has no light on them at all"),
			BDark.Brightness, 0.f);
		TestEqual(TEXT("and the same Sweeper in the same spot notices nothing"),
			LLKPerception::SenseReaching(SweeperDef, Sweeper, BDark, Threshold),
			ELLKNoticeSense::None);

		// --- how close you have to stand, which is a derived answer ---
		//
		// **This relationship is not a coincidence and is worth pinning.**
		// The lit threshold is half the lantern's intensity, and
		// `BrightnessAt` is `I / (1 + d^2/r^2)`, so the crossover sits
		// exactly at `d == r`: a teammate exposes you out to the lantern's
		// own authored attenuation radius and not one stud further. That
		// makes "stay outside their light" a rule a player can see rather
		// than a number they have to be told.
		const float JustInside =
			LLKLightField::BrightnessAt(ALit, StudsAlongX(LanternRadiusStuds - 1.f));
		const float JustOutside =
			LLKLightField::BrightnessAt(ALit, StudsAlongX(LanternRadiusStuds + 1.f));

		TestTrue(TEXT("one stud inside the lantern's radius a teammate is still lit"),
			JustInside > Threshold);
		TestTrue(TEXT("one stud outside it they are not"), JustOutside < Threshold);
		TestTrue(TEXT("so exposure reaches exactly as far as the lantern says it does"),
			JustInside > Threshold && JustOutside < Threshold);

		// And at 40 studs the teammate is genuinely clear, not marginally so.
		FLLKPerceivedKeeper Far;
		Far.Position = StudsAlongX(40.f);
		Far.Brightness = LLKLightField::BrightnessAt(ALit, Far.Position);
		TestTrue(TEXT("a keeper 40 studs from the lit one is not exposed by them"),
			Far.Brightness < Threshold);
	}

	// =====================================================================
	// 2. A SWEEPER NEVER "SEES" THROUGH ITS PROXIMITY RULE
	// =====================================================================
	//
	// Proximity is omnidirectional, and the failure to guard against is
	// somebody later adding a facing check to it "for consistency" - which
	// would make a Sweeper safe to stand behind and delete the swarm.
	{
		const FLLKEnemyDef SweeperDef = MakeSweeper();

		FLLKPerceiver Sweeper;
		Sweeper.Position = FVector::ZeroVector;
		Sweeper.Facing = FVector::ForwardVector;

		FLLKPerceivedKeeper Behind;
		Behind.Position = StudsAlongX(-10.f);
		Behind.Brightness = Threshold + 1.f;

		TestEqual(TEXT("a lit keeper directly BEHIND a Sweeper is still noticed"),
			LLKPerception::SenseReaching(SweeperDef, Sweeper, Behind, Threshold),
			ELLKNoticeSense::LitProximity);

		// Every direction, not just the convenient two.
		for (int32 Degrees = 0; Degrees < 360; Degrees += 45)
		{
			const float Radians = FMath::DegreesToRadians(static_cast<float>(Degrees));
			const float Distance = LLKUnits::DistanceFromStuds(20.f);

			FLLKPerceivedKeeper Around;
			Around.Position = FVector(
				Distance * FMath::Cos(Radians), Distance * FMath::Sin(Radians), 0.f);
			Around.Brightness = Threshold + 1.f;

			TestEqual(
				*FString::Printf(TEXT("and noticed from %d degrees, by proximity not sight"), Degrees),
				LLKPerception::SenseReaching(SweeperDef, Sweeper, Around, Threshold),
				ELLKNoticeSense::LitProximity);
		}

		// Facing must not change the answer at all, which is the property
		// rather than the symptom.
		FLLKPerceiver Reversed = Sweeper;
		Reversed.Facing = -FVector::ForwardVector;
		TestEqual(TEXT("turning a Sweeper around changes nothing it notices"),
			LLKPerception::SenseReaching(SweeperDef, Reversed, Behind, Threshold),
			LLKPerception::SenseReaching(SweeperDef, Sweeper, Behind, Threshold));
	}

	// =====================================================================
	// 3. A STOCKER NEEDS BOTH RANGE AND CONE
	// =====================================================================
	//
	// Either field alone must produce no sight sense. Half a sense is the
	// shape a bad import takes, and the two halves fail in opposite
	// directions: a range without a cone is an omnidirectional watchman, a
	// cone without a range is unbounded vision.
	{
		FLLKPerceiver Stocker;
		Stocker.Position = FVector::ZeroVector;
		Stocker.Facing = FVector::ForwardVector;

		FLLKPerceivedKeeper Ahead;
		Ahead.Position = StudsAlongX(50.f);
		Ahead.Brightness = Threshold + 1.f;

		TestEqual(TEXT("a complete Stocker sees a keeper ahead of it"),
			LLKPerception::SenseReaching(MakeStocker(), Stocker, Ahead, Threshold),
			ELLKNoticeSense::SightCone);

		FLLKEnemyDef NoCone = MakeStocker();
		NoCone.SightConeDegrees.Reset();
		TestEqual(TEXT("a Stocker with a range but no cone sees nothing"),
			LLKPerception::SenseReaching(NoCone, Stocker, Ahead, Threshold),
			ELLKNoticeSense::None);

		FLLKEnemyDef NoRange = MakeStocker();
		NoRange.SightRange.Reset();
		TestEqual(TEXT("and a Stocker with a cone but no range sees nothing either"),
			LLKPerception::SenseReaching(NoRange, Stocker, Ahead, Threshold),
			ELLKNoticeSense::None);

		// Not even at zero distance, where an unbounded-vision bug would
		// most plausibly still return a hit.
		FLLKPerceivedKeeper Touching;
		Touching.Position = FVector::ZeroVector;
		Touching.Brightness = Threshold + 1.f;
		TestEqual(TEXT("a half-imported Stocker does not notice somebody standing on it"),
			LLKPerception::SenseReaching(NoRange, Stocker, Touching, Threshold),
			ELLKNoticeSense::None);
	}

	// =====================================================================
	// 4. A WALKER OR FICUS CANNOT WAKE FROM BARE RANGE ALONE
	// =====================================================================
	{
		FLLKPerceiver Walker;
		Walker.Position = FVector::ZeroVector;
		Walker.Facing = FVector::ForwardVector;

		const FLLKEnemyDef WalkerDef = MakeWalker();

		// Inside wakeRange but dark: the counterplay the behaviour line
		// promises. "Go dark and break sight."
		FLLKPerceivedKeeper DarkClose;
		DarkClose.Position = StudsAlongX(5.f);
		DarkClose.Brightness = Threshold - 1.f;
		TestEqual(TEXT("a dark keeper can walk right past a Window Walker"),
			LLKPerception::SenseReaching(WalkerDef, Walker, DarkClose, Threshold),
			ELLKNoticeSense::None);

		// Lit, but outside wakeRange and inside the bare sightRange. If that
		// range counted as vision, every mannequin would be a watchman.
		FLLKPerceivedKeeper LitBetween;
		LitBetween.Position = StudsAlongX(34.f);
		LitBetween.Brightness = Threshold + 1.f;
		TestEqual(TEXT("a lit keeper inside the bare sightRange but outside wakeRange is unnoticed"),
			LLKPerception::SenseReaching(WalkerDef, Walker, LitBetween, Threshold),
			ELLKNoticeSense::None);

		// And a Walker with ONLY the bare range has no sense whatsoever.
		FLLKEnemyDef RangeOnly = MakeWalker();
		RangeOnly.WakeRange.Reset();
		FLLKPerceivedKeeper LitClose;
		LitClose.Position = StudsAlongX(5.f);
		LitClose.Brightness = Threshold + 1.f;
		TestEqual(TEXT("a Walker with a sight range and no wake range notices nothing at all"),
			LLKPerception::SenseReaching(RangeOnly, Walker, LitClose, Threshold),
			ELLKNoticeSense::None);
		TestEqual(TEXT("and offers no notice range, so nothing can be sized against it"),
			LLKPerception::NoticeRangeStuds(RangeOnly, Threshold + 1.f, Threshold), 0.f);

		// The positive control, so the four negatives above are not all
		// passing for the same uninteresting reason.
		TestEqual(TEXT("but a lit keeper inside wakeRange does wake it"),
			LLKPerception::SenseReaching(WalkerDef, Walker, LitClose, Threshold),
			ELLKNoticeSense::Wake);
	}

	// =====================================================================
	// 5. MISSING OR MALFORMED DATA MEANS NO PERCEPTION, NEVER A FALLBACK
	// =====================================================================
	//
	// **The section that found the flaw.** Every malformed shape must come
	// back `None` - and specifically NOT `StormAuthority`, which would make
	// broken data behave like the one enemy that cannot be escaped.
	{
		FLLKPerceiver At;
		At.Position = FVector::ZeroVector;
		At.Facing = FVector::ForwardVector;

		// Point blank and brightly lit: the input most likely to coax a
		// false positive out of a defaulted radius.
		FLLKPerceivedKeeper PointBlank;
		PointBlank.Position = FVector::ZeroVector;
		PointBlank.Brightness = LanternIntensity * 4.f;

		struct FMalformed
		{
			const TCHAR* What;
			FLLKEnemyDef Def;
		};

		TArray<FMalformed> Broken;

		// Nothing imported at all, for each non-Warden family.
		for (const ELLKEnemyFamily Family : {
				 ELLKEnemyFamily::Sweepers, ELLKEnemyFamily::Stockers,
				 ELLKEnemyFamily::Glimmers, ELLKEnemyFamily::WindowWalkers,
				 ELLKEnemyFamily::Ficus })
		{
			FLLKEnemyDef Empty;
			Empty.Family = Family;
			Broken.Add({ TEXT("a row with no senses imported"), Empty });
		}

		FLLKEnemyDef LitOnly = MakeSweeper();
		LitOnly.AggroRadiusDark.Reset();
		Broken.Add({ TEXT("a Sweeper with only its lit radius"), LitOnly });

		FLLKEnemyDef DarkOnly = MakeSweeper();
		DarkOnly.AggroRadiusLit.Reset();
		Broken.Add({ TEXT("a Sweeper with only its dark radius"), DarkOnly });

		FLLKEnemyDef ConeOnly = MakeStocker();
		ConeOnly.SightRange.Reset();
		Broken.Add({ TEXT("a Stocker with only its cone"), ConeOnly });

		// Zeroes, which are the other way data arrives malformed - and the
		// reason `tests/enemies.test.mjs` forbids them in the authored file.
		FLLKEnemyDef ZeroRadii = MakeSweeper();
		ZeroRadii.AggroRadiusDark = 0.f;
		ZeroRadii.AggroRadiusLit = 0.f;
		Broken.Add({ TEXT("a Sweeper whose radii imported as zero"), ZeroRadii });

		FLLKEnemyDef NegativeRadii = MakeSweeper();
		NegativeRadii.AggroRadiusLit = -38.f;
		NegativeRadii.AggroRadiusDark = -12.f;
		Broken.Add({ TEXT("a Sweeper whose radii imported negative"), NegativeRadii });

		FLLKEnemyDef ZeroScan;
		ZeroScan.Family = ELLKEnemyFamily::Glimmers;
		ZeroScan.ScanRange = 0.f;
		ZeroScan.bScanRequiresActiveLantern = false;
		Broken.Add({ TEXT("a Glimmer with a zero scan range and no light gate"), ZeroScan });

		for (const FMalformed& Case : Broken)
		{
			const ELLKNoticeSense Sense =
				LLKPerception::SenseReaching(Case.Def, At, PointBlank, Threshold);

			TestEqual(
				*FString::Printf(TEXT("%s notices nothing, even point blank and lit"), Case.What),
				Sense, ELLKNoticeSense::None);

			// Said separately, because this is the specific wrong answer the
			// rule used to give and the one that matters most.
			TestTrue(
				*FString::Printf(TEXT("and %s is NOT mistaken for the Warden"), Case.What),
				Sense != ELLKNoticeSense::StormAuthority);

			TestEqual(
				*FString::Printf(TEXT("and %s offers no range to invent from"), Case.What),
				LLKPerception::NoticeRangeStuds(Case.Def, PointBlank.Brightness, Threshold), 0.f);
		}
	}

	// =====================================================================
	// 6. THE WARDEN STAYS OUTSIDE PERCEPTION, AND CONVERGES ON THE STORM
	// =====================================================================
	{
		FLLKEnemyDef Warden;
		Warden.Family = ELLKEnemyFamily::Warden;
		Warden.bKillable = false;
		Warden.RetargetIntervalSeconds = 5.f;

		FLLKPerceiver At;
		At.Position = FVector::ZeroVector;
		At.Facing = FVector::ForwardVector;

		// Its answer does not depend on where anybody is or how lit they are,
		// which is what "outside perception" means operationally.
		for (const float Studs : { 1.f, 20.f, 100.f, 500.f })
		{
			for (const float Brightness : { 0.f, LanternIntensity })
			{
				FLLKPerceivedKeeper Keeper;
				Keeper.Position = StudsAlongX(Studs);
				Keeper.Brightness = Brightness;

				TestEqual(
					*FString::Printf(
						TEXT("the Warden's answer at %.0f studs and %.0f brightness is StormAuthority"),
						Studs, Brightness),
					LLKPerception::SenseReaching(Warden, At, Keeper, Threshold),
					ELLKNoticeSense::StormAuthority);
			}
		}

		TestEqual(TEXT("row 7: and it has no notice range at any brightness"),
			LLKPerception::NoticeRangeStuds(Warden, LanternIntensity, Threshold), 0.f);
		TestFalse(TEXT("and it still cannot be damaged"), Warden.CanBeDamaged());

		// --- the other half: convergence is the light field's answer ---
		//
		// Brightest, never nearest. A dim keeper close by and a bright one
		// far away: the Warden's target is the bright one, which is what puts
		// cause and consequence in the same pair of hands.
		const FVector Near = StudsAlongX(5.f);
		const FVector Far = StudsAlongX(60.f);

		FLLKLightSource Dim = LanternAt(Near);
		Dim.Intensity = LanternIntensity * 0.25f;
		const FLLKLightSource Bright = LanternAt(Far);

		FVector Target = FVector::ZeroVector;
		float TargetBrightness = 0.f;
		const bool bFound = LLKLightField::FindBrightestPosition(
			{ Dim, Bright }, Target, TargetBrightness);

		TestTrue(TEXT("with anybody lit the Warden has somewhere to converge on"), bFound);
		TestEqual(TEXT("and it is the BRIGHTEST keeper, not the nearest"), Target, Far);

		// And with the crew dark it has no target rather than the origin,
		// which is a real place somebody will stand.
		FVector NoTarget = StudsAlongX(999.f);
		float NoBrightness = -1.f;
		TestFalse(TEXT("with the crew dark there is no position to converge on"),
			LLKLightField::FindBrightestPosition({}, NoTarget, NoBrightness));

		// The separation itself: perception said StormAuthority regardless of
		// those lights, and the convergence target came from the field. One
		// does not feed the other.
		FLLKPerceivedKeeper BrightKeeper;
		BrightKeeper.Position = Far;
		BrightKeeper.Brightness = TargetBrightness;
		TestEqual(TEXT("the Warden does not perceive its own convergence target either"),
			LLKPerception::SenseReaching(Warden, At, BrightKeeper, Threshold),
			ELLKNoticeSense::StormAuthority);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
