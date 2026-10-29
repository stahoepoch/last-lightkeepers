// Which sense reached the keeper, and whether light decided it. M2.4.
//
// The defs below are built by hand rather than loaded, and they carry the
// authored numbers only where a relationship is being asserted. That is
// deliberate: this file tests the RULE, and `tests/enemies.test.mjs` tests the
// DATA. A test that hardcoded all six families' stats would fail every time
// Chris retuned one, which trains people to edit the test instead of reading
// it.
//
// The failures worth catching, in order of how badly each would go wrong:
//
//   1. **Studs compared against centimetres.** Every radius in
//      `enemies.json` is in studs and every `FVector` is in cm, so a missing
//      conversion makes every enemy notice from a twenty-eighth of its range.
//      That reads as enemies being broken, not as a unit bug, so the
//      assertions below are placed in studs either side of a real boundary.
//   2. **Light read from the keeper's own switch.** The gate is brightness at
//      the keeper's position. If it were "my lantern is on", a dark keeper
//      standing in a teammate's cone would be invisible, and the crew's light
//      would stop being a shared cost - which is the one lesson
//      `docs/THE-STORE.md` is built to teach.
//   3. **A cone-less sight range treated as a sight sense.** WindowWalkers
//      and Ficus carry `sightRange` with no `sightConeDegrees`. Treating that
//      as vision makes every mannequin an omnidirectional watchman and
//      deletes "remain still until a nearby active lantern catches them".
//   4. **An absent sense defaulted into existence.** A Stocker with an
//      invented aggro radius notices everybody, which looks like cheating.
//      `TOptional` makes absence expressible and this file checks nothing
//      undoes it.
//   5. **The Warden given a radius.** Row 7 and row 54: it converges on the
//      brightest position when its own meter fills. A radius here is the
//      first step to making it a boss.
//   6. **A half-angle used as a full angle.** Doubles or halves every
//      Stocker's vision and would never show up in a log.

#include "Data/LLKEnemyDef.h"
#include "Enemies/LLKPerception.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	/** A keeper at a stud distance straight ahead of an enemy at the origin. */
	FLLKPerceivedKeeper KeeperAt(float Studs, float Brightness)
	{
		FLLKPerceivedKeeper Keeper;
		Keeper.Position = FVector(LLKUnits::DistanceFromStuds(Studs), 0.f, 0.f);
		Keeper.Brightness = Brightness;
		return Keeper;
	}

	/** An enemy at the origin facing +X, which is where KeeperAt puts people. */
	FLLKPerceiver FacingForward()
	{
		FLLKPerceiver Perceiver;
		Perceiver.Position = FVector::ZeroVector;
		Perceiver.Facing = FVector::ForwardVector;
		return Perceiver;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKPerceptionTest,
	"LLK.Enemies.Perception",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKPerceptionTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK perception test reached RunTest."));

	// The lantern authors 250 intensity in `data/lantern.json`, so the lit
	// threshold is 125 - half of it, one lantern at the edge of its reach.
	const float LanternIntensity = 250.f;
	const float Threshold = LLKPerception::LitThresholdFrom(LanternIntensity);
	TestEqual(TEXT("the lit threshold is half the lantern's intensity"), Threshold, 125.f);
	TestTrue(TEXT("and a negative intensity cannot produce a negative threshold"),
		LLKPerception::LitThresholdFrom(-50.f) >= 0.f);

	const float Lit = Threshold + 1.f;
	const float Dark = Threshold - 1.f;

	// ---- 1 and 2. Sweepers: proximity, with light deciding how far ------
	FLLKEnemyDef Sweeper;
	Sweeper.Family = ELLKEnemyFamily::Sweepers;
	Sweeper.AggroRadiusDark = 12.f;
	Sweeper.AggroRadiusLit = 38.f;

	const FLLKPerceiver At = FacingForward();

	// The relationship the whole lantern decision rests on: the same keeper,
	// the same spot, noticed or not depending only on light.
	TestEqual(TEXT("a lit keeper at 20 studs is noticed by proximity"),
		LLKPerception::SenseReaching(Sweeper, At, KeeperAt(20.f, Lit), Threshold),
		ELLKNoticeSense::LitProximity);
	TestEqual(TEXT("and the same keeper gone dark at the same spot is not noticed at all"),
		LLKPerception::SenseReaching(Sweeper, At, KeeperAt(20.f, Dark), Threshold),
		ELLKNoticeSense::None);

	// FAILURE MODE 1: either side of the authored boundary, in studs. With the
	// conversion missing, 37 studs is 1036cm against a 38cm radius and this
	// first assertion fails loudly.
	TestEqual(TEXT("a lit keeper just inside 38 studs is noticed"),
		LLKPerception::SenseReaching(Sweeper, At, KeeperAt(37.f, Lit), Threshold),
		ELLKNoticeSense::LitProximity);
	TestEqual(TEXT("and just outside it is not"),
		LLKPerception::SenseReaching(Sweeper, At, KeeperAt(39.f, Lit), Threshold),
		ELLKNoticeSense::None);

	// The dark radius bounds too, so the smaller number is real rather than
	// unused.
	TestEqual(TEXT("a dark keeper inside 12 studs is still noticed"),
		LLKPerception::SenseReaching(Sweeper, At, KeeperAt(11.f, Dark), Threshold),
		ELLKNoticeSense::LitProximity);
	TestEqual(TEXT("but not at 13"),
		LLKPerception::SenseReaching(Sweeper, At, KeeperAt(13.f, Dark), Threshold),
		ELLKNoticeSense::None);

	// FAILURE MODE 2, stated as the property it protects. Brightness is the
	// input, so a keeper whose OWN lantern is off but who stands in a
	// teammate's light is lit here - there is no other switch to read.
	TestEqual(TEXT("light is read from the field, so standing in a teammate's light is being lit"),
		LLKPerception::SenseReaching(Sweeper, At, KeeperAt(30.f, Lit), Threshold),
		ELLKNoticeSense::LitProximity);

	// And the range accessor agrees with the rule, since the readout and the
	// floor plan will read it rather than re-deriving.
	TestEqual(TEXT("the notice range is the lit radius when lit"),
		LLKPerception::NoticeRangeStuds(Sweeper, Lit, Threshold), 38.f);
	TestEqual(TEXT("and the dark radius when dark"),
		LLKPerception::NoticeRangeStuds(Sweeper, Dark, Threshold), 12.f);

	// A Sweeper must never be noticing by sight - under a bool return this
	// would have been an invisible pass.
	FLLKEnemyDef HalfSweeper = Sweeper;
	HalfSweeper.AggroRadiusLit.Reset();
	TestEqual(TEXT("FAILURE MODE 4: a Sweeper missing one radius has no proximity sense"),
		LLKPerception::SenseReaching(HalfSweeper, At, KeeperAt(5.f, Lit), Threshold),
		ELLKNoticeSense::StormAuthority);

	// ---- 6. Stockers: looking, and only where they face -----------------
	FLLKEnemyDef Stocker;
	Stocker.Family = ELLKEnemyFamily::Stockers;
	Stocker.SightRange = 70.f;
	Stocker.SightConeDegrees = 70.f;

	TestEqual(TEXT("a keeper straight ahead inside sight range is seen"),
		LLKPerception::SenseReaching(Stocker, At, KeeperAt(50.f, Dark), Threshold),
		ELLKNoticeSense::SightCone);

	// Darkness does not solve Stockers, which is what makes them the family
	// the lantern rule does not answer. Asserted both ways round.
	TestEqual(TEXT("and being dark does not help - sight ignores light"),
		LLKPerception::SenseReaching(Stocker, At, KeeperAt(50.f, Dark), Threshold),
		LLKPerception::SenseReaching(Stocker, At, KeeperAt(50.f, Lit), Threshold));

	TestEqual(TEXT("beyond sight range nothing is seen"),
		LLKPerception::SenseReaching(Stocker, At, KeeperAt(71.f, Lit), Threshold),
		ELLKNoticeSense::None);

	// FAILURE MODE 6. The authored 70 degrees is the cone's full width, so
	// 34 degrees off-axis is inside and 36 is outside. A half-angle mistake
	// puts the boundary at 70 and both of these flip.
	{
		const float Distance = LLKUnits::DistanceFromStuds(50.f);
		auto KeeperAtAngle = [Distance](float Degrees)
		{
			FLLKPerceivedKeeper Keeper;
			const float Radians = FMath::DegreesToRadians(Degrees);
			Keeper.Position = FVector(
				Distance * FMath::Cos(Radians), Distance * FMath::Sin(Radians), 0.f);
			Keeper.Brightness = 0.f;
			return Keeper;
		};

		TestEqual(TEXT("34 degrees off a 70-degree cone's axis is inside it"),
			LLKPerception::SenseReaching(Stocker, At, KeeperAtAngle(34.f), Threshold),
			ELLKNoticeSense::SightCone);
		TestEqual(TEXT("36 degrees is outside it, so 70 is the FULL width"),
			LLKPerception::SenseReaching(Stocker, At, KeeperAtAngle(36.f), Threshold),
			ELLKNoticeSense::None);
		TestEqual(TEXT("and directly behind is certainly outside it"),
			LLKPerception::SenseReaching(Stocker, At, KeeperAtAngle(180.f), Threshold),
			ELLKNoticeSense::None);
	}

	// A keeper standing on top of the enemy has no direction to be in, and is
	// noticed rather than missed through a division nobody meant.
	{
		FLLKPerceivedKeeper Overlapping;
		Overlapping.Position = FVector::ZeroVector;
		Overlapping.Brightness = 0.f;
		TestEqual(TEXT("a keeper standing on the enemy is seen, not lost to a zero vector"),
			LLKPerception::SenseReaching(Stocker, At, Overlapping, Threshold),
			ELLKNoticeSense::SightCone);
	}

	// ---- Glimmers: scanning, and only what is lit -----------------------
	FLLKEnemyDef Glimmer;
	Glimmer.Family = ELLKEnemyFamily::Glimmers;
	Glimmer.ScanRange = 30.f;
	Glimmer.bScanRequiresActiveLantern = true;

	TestEqual(TEXT("a lit keeper inside scan range is scanned"),
		LLKPerception::SenseReaching(Glimmer, At, KeeperAt(20.f, Lit), Threshold),
		ELLKNoticeSense::Scan);
	TestEqual(TEXT("row 5: a dark crew is not scanned, at any distance"),
		LLKPerception::SenseReaching(Glimmer, At, KeeperAt(1.f, Dark), Threshold),
		ELLKNoticeSense::None);
	TestEqual(TEXT("and the scan range is zero for a dark keeper, not 30"),
		LLKPerception::NoticeRangeStuds(Glimmer, Dark, Threshold), 0.f);

	// The flag's absence must fail safe TOWARD the player: a Glimmer that
	// scans less, never one that scans a crew who did everything right.
	FLLKEnemyDef GlimmerNoFlag = Glimmer;
	GlimmerNoFlag.bScanRequiresActiveLantern.Reset();
	TestEqual(TEXT("with the lantern flag missing, a dark crew is STILL not scanned"),
		LLKPerception::SenseReaching(GlimmerNoFlag, At, KeeperAt(10.f, Dark), Threshold),
		ELLKNoticeSense::None);

	// ---- 3. WindowWalkers: furniture until the light arrives ------------
	//
	// The authored shape that makes failure mode 3 reachable: wakeRange 32
	// and a sightRange of 36 with NO cone.
	FLLKEnemyDef Walker;
	Walker.Family = ELLKEnemyFamily::WindowWalkers;
	Walker.WakeRange = 32.f;
	Walker.SightRange = 36.f;
	Walker.bWakeRequiresActiveLantern = true;

	TestEqual(TEXT("a lit keeper inside wake range wakes it"),
		LLKPerception::SenseReaching(Walker, At, KeeperAt(30.f, Lit), Threshold),
		ELLKNoticeSense::Wake);
	TestEqual(TEXT("a dark keeper walking past does not"),
		LLKPerception::SenseReaching(Walker, At, KeeperAt(2.f, Dark), Threshold),
		ELLKNoticeSense::None);

	// FAILURE MODE 3. At 34 studs the keeper is outside wakeRange and inside
	// sightRange. If a cone-less sight range counted as vision, this would
	// return SightCone and every mannequin in the building would be a
	// watchman.
	TestEqual(TEXT("lit at 34 studs - inside sightRange, outside wakeRange - notices NOTHING"),
		LLKPerception::SenseReaching(Walker, At, KeeperAt(34.f, Lit), Threshold),
		ELLKNoticeSense::None);
	TestEqual(TEXT("so a cone-less sight range is for following, not for spotting"),
		LLKPerception::NoticeRangeStuds(Walker, Lit, Threshold), 32.f);

	// ---- 5. the Warden: not perception ---------------------------------
	FLLKEnemyDef Warden;
	Warden.Family = ELLKEnemyFamily::Warden;
	Warden.bKillable = false;
	Warden.RetargetIntervalSeconds = 5.f;

	TestEqual(TEXT("the Warden has no sense, and says so as StormAuthority"),
		LLKPerception::SenseReaching(Warden, At, KeeperAt(1.f, Lit), Threshold),
		ELLKNoticeSense::StormAuthority);
	TestEqual(TEXT("and it is StormAuthority even for a distant dark keeper - it is still coming"),
		LLKPerception::SenseReaching(Warden, At, KeeperAt(500.f, Dark), Threshold),
		ELLKNoticeSense::StormAuthority);
	TestEqual(TEXT("row 7: the Warden has no notice range to size anything against"),
		LLKPerception::NoticeRangeStuds(Warden, Lit, Threshold), 0.f);
	TestFalse(TEXT("and it still cannot be damaged"), Warden.CanBeDamaged());

	// StormAuthority must be distinguishable from None, or a caller reads
	// "does not see you" as "is not coming".
	TestTrue(TEXT("StormAuthority is not None, which is the whole reason it exists"),
		ELLKNoticeSense::StormAuthority != ELLKNoticeSense::None);

	// ---- the shape of the floor plan's own rule --------------------------
	//
	// `LLKStoreFloorPlan` is tuned so no sightline exceeds the range a lit
	// keeper is noticed from. It restates 38 in a comment today; this is the
	// accessor that lets it ask instead.
	TestTrue(TEXT("a lit keeper is noticed from further than a dark one, which is the floor's rule"),
		LLKPerception::NoticeRangeStuds(Sweeper, Lit, Threshold)
			> LLKPerception::NoticeRangeStuds(Sweeper, Dark, Threshold));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
