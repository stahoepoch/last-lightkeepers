// A Sweeper may threaten a keeper, but it may never become a movable wall.
// The first Boswell's pass demonstrated why: a cart that blocks the Pawn
// channel can seal an aisle corner and turn a chase into forced containment.
// Contact is therefore an overlap, resolved by LLKSweeperBrain's authored
// range and cooldown. World collision remains blocking so the cart still
// respects the store's gondolas and walls.

#include "Components/CapsuleComponent.h"
#include "Characters/LLKStumbleCameraShake.h"
#include "Data/LLKLanternData.h"
#include "Enemies/LLKSweeper.h"
#include "Misc/AutomationTest.h"
#include "Sound/SoundAttenuation.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKSweeperCollisionTest,
	"LLK.Enemies.SweeperCollision",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKSweeperCollisionTest::RunTest(const FString& Parameters)
{
	const ALLKSweeper* Sweeper = GetDefault<ALLKSweeper>();
	if (!TestNotNull(TEXT("the Sweeper class has a default object"), Sweeper))
	{
		return false;
	}

	const UCapsuleComponent* Capsule = Sweeper->GetCapsuleComponent();
	if (!TestNotNull(TEXT("the Sweeper has the ACharacter capsule"), Capsule))
	{
		return false;
	}

	TestEqual(TEXT("a keeper overlaps the Sweeper instead of being body-blocked"),
		static_cast<int32>(Capsule->GetCollisionResponseToChannel(ECC_Pawn)),
		static_cast<int32>(ECR_Overlap));
	TestEqual(TEXT("the Sweeper still blocks store geometry"),
		static_cast<int32>(Capsule->GetCollisionResponseToChannel(ECC_WorldStatic)),
		static_cast<int32>(ECR_Block));

	const USoundAttenuation* CueAttenuation = FindObject<USoundAttenuation>(
		const_cast<ALLKSweeper*>(Sweeper), TEXT("SweeperCueAttenuation"));
	if (TestNotNull(TEXT("the Sweeper has a spatial cue attenuation asset"), CueAttenuation))
	{
		TestTrue(TEXT("the temporary telegraph cue is spatialized"),
			CueAttenuation->Attenuation.bSpatialize);
		TestTrue(TEXT("the temporary telegraph cue falls off with distance"),
			CueAttenuation->Attenuation.bAttenuate);
	}

	const ULLKStumbleCameraShake* StumbleShake = GetDefault<ULLKStumbleCameraShake>();
	if (TestNotNull(TEXT("a stumble has an input-safe camera acknowledgement"), StumbleShake))
	{
		TestTrue(TEXT("the stumble shake is brief rather than a second containment timer"),
			StumbleShake->OscillationDuration > 0.f && StumbleShake->OscillationDuration < 1.f);
		TestTrue(TEXT("the stumble shake cannot stack into visual noise"), StumbleShake->bSingleInstance);
	}

	// Read the generated asset, not merely data/lantern.json. The first run
	// had the authored false value while an older DA_Lantern still made the
	// live Sweeper contain the keeper. A source-data test cannot catch that.
	const ULLKLanternData* LanternData = LoadObject<ULLKLanternData>(
		nullptr, TEXT("/Game/Data/DA_Lantern.DA_Lantern"));
	if (!TestNotNull(TEXT("the generated Lantern data asset loads"), LanternData))
	{
		return false;
	}

	TestFalse(TEXT("the imported Sweeper contact rule never forces a revive"),
		LanternData->SweeperContact.bKnockdownOnContact);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
