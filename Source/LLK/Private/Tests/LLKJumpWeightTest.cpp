// How heavy the keeper feels, asserted. And why row 63 needs it.
//
// **Chris, playing the harness: "the jump feels like gravity is a little
// light, i float a little much."** A feel complaint, which is normally the
// kind of thing no test should touch - you cannot assert "weighty". But two
// things about this one are assertable, and the second matters more than the
// first.
//
//   1. **The arithmetic.** Apex and airtime follow from the launch speed and
//      the gravity, so the numbers Chris is reacting to can be stated rather
//      than felt for. Unreal's defaults are a 90cm apex over 0.86s; this
//      class is tuned to roughly half of each.
//
//   2. **ROW 63 DEPENDS ON THE JUMP BEING SHORT.** His traversal brief wants
//      vaulting a counter to feel "desperate and physical", and a 90cm apex
//      clears a 90cm counter - so the stock jump already did the vault's job,
//      and a vault you do not need is not desperate, it is a slower way to do
//      something you could already do. That was true before anybody noticed
//      the floating, and it is the part a test can protect: if the jump ever
//      grows back to counter height, the traversal design quietly dies and
//      nothing else would complain.
//
// So the assertions are a band rather than exact values. Feel is Chris's and
// he should be able to retune inside the band without a test arguing with
// him; what the test defends is the design consequence at the edges.

#include "Characters/LLKCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKJumpWeightTest,
	"LLK.Movement.JumpWeight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKJumpWeightTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK jump weight test reached RunTest."));

	// The CDO, so this needs no world: the properties and the arithmetic over
	// them are what is being checked, not a spawned pawn's physics.
	const ALLKCharacter* Keeper = GetDefault<ALLKCharacter>();
	if (!TestNotNull(TEXT("the keeper class has a default object"), Keeper))
	{
		return false;
	}

	// ---- 1. the conversion ---------------------------------------------
	//
	// A jump speed is a speed, so it goes through LLKUnits like the walk and
	// the sprint. Read raw, 14 would be 14cm/s and the keeper would not leave
	// the floor - the same class of bug as a 30cm lantern.
	TestEqual(TEXT("the jump speed went through LLKUnits"),
		Keeper->GetJumpVelocity(), LLKUnits::SpeedFromStuds(13.2f));
	TestNearlyEqual(TEXT("which is Codex's 370 cm/s, not 13.2"),
		Keeper->GetJumpVelocity(), 370.f, 0.5f);

	// ---- 2. the float, measured ----------------------------------------
	const float Apex = Keeper->GetJumpApexCm();
	const float Airtime = Keeper->GetJumpAirtimeSeconds();

	TestTrue(TEXT("the apex is a real height"), Apex > 1.f);
	TestTrue(TEXT("and the airtime is a real duration"), Airtime > 0.05f);

	// Unreal's defaults, for comparison, and the thing being moved away from:
	// 420 cm/s at gravity 1.0 is a 90cm apex over 0.857s.
	const float StockApex = (420.f * 420.f) / (2.f * 980.f);
	const float StockAirtime = (2.f * 420.f) / 980.f;
	TestTrue(TEXT("the keeper hangs for less time than a stock Unreal character"),
		Airtime < StockAirtime * 0.75f);
	TestTrue(TEXT("and does not jump as high"), Apex < StockApex * 0.75f);

	// ---- 3. ROW 63. A COUNTER MUST NOT BE JUMPABLE ----------------------
	//
	// The harness counter is 3.2 studs tall, which is about 90cm and is the
	// honest height for a shop counter. If the jump clears it, the vault and
	// mantle in row 63 have nothing to do, and "desperate and physical"
	// becomes a slower alternative to a hop.
	const float CounterHeightCm = LLKUnits::DistanceFromStuds(3.2f);
	TestTrue(TEXT("a shop counter cannot be cleared by jumping - row 63 needs the vault"),
		Apex < CounterHeightCm);

	// With a margin, so the rule does not depend on a centimetre. A jump that
	// *nearly* clears a counter is worse than one that clearly does not:
	// players would try it, fail inconsistently, and blame the game.
	TestTrue(TEXT("and not even close, so nobody tries it and blames the game"),
		Apex < CounterHeightCm * 0.75f);

	// ---- 4. the band Chris can retune inside ---------------------------
	//
	// Deliberately loose. Feel is his and a test should not argue about 5cm;
	// what it defends is that the jump stays a jump. Below 25cm it stops
	// clearing anything worth clearing and reads as a stumble; above 70cm it
	// is heading back toward counter height and row 63's problem.
	TestTrue(TEXT("the jump still clears something worth clearing"), Apex > 25.f);
	TestTrue(TEXT("and has not crept back toward counter height"), Apex < 70.f);

	// Airtime is the float itself. Past about two thirds of a second it reads
	// as floaty again whatever the height is.
	TestTrue(TEXT("the airtime does not read as floaty"), Airtime < 0.67f);
	TestTrue(TEXT("but is long enough to be a jump rather than a twitch"), Airtime > 0.3f);

	// ---- 5. gravity is heavier than stock, which is the actual fix ------
	const UCharacterMovementComponent* Movement = Keeper->GetCharacterMovement();
	if (TestNotNull(TEXT("the CDO has a movement component"), Movement))
	{
		TestTrue(TEXT("gravity is heavier than Unreal's default"), Movement->GravityScale > 1.f);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
