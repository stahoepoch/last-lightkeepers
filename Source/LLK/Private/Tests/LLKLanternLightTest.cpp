// The lantern light's resolution, asserted. M1.3's placeholder.
//
// `tests/balance.test.mjs` asserts the authored numbers against each other -
// that the radius sits inside `aggroRadiusLit`, that the colour is warm, that
// the cone is a lantern's and not a torch's. This asserts something the Node
// suite structurally cannot: that the ENGINE applies those numbers, converted
// once through `LLKUnits`, to the actual light.
//
// The gap between those two is where this class could go wrong without any
// existing check noticing:
//
//   1. **The conversion.** `radiusStuds: 30` read straight into
//      `AttenuationRadius` is 30cm - a lantern that lights the keeper's own
//      boots. Row 61 makes `LLKUnits` the single authority, and this asserts
//      the call actually happens rather than trusting that it was written.
//   2. **The fallback agreeing with the data.** On the fallback path the
//      default IS the value, so a fallback that disagrees with
//      `data/lantern.json` is a silent disagreement - the exact bug already
//      found once in this project with `bAutoOffAtEmpty`.
//   3. **The cone's two halves.** The authored angle is the OUTER cone. If it
//      were applied to the inner one the lit area would be wrong while every
//      number in the asset stayed right.

#include "Data/LLKLanternData.h"
#include "Engine/Scene.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKLanternLightTest,
	"LLK.Lantern.LightRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKLanternLightTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK lantern light test reached RunTest."));

	ULLKLanternLightComponent* Light = NewObject<ULLKLanternLightComponent>(GetTransientPackage());

	// 1. It starts dark and unresolved. Row 5 makes being seen a decision,
	//    and a light that is on before anything resolved it would make the
	//    first second of a run a decision nobody got to make.
	TestFalse(TEXT("a new light is not visible"), Light->IsVisible());
	TestEqual(TEXT("and has resolved nothing yet"),
		Light->GetLightSource(), ELLKDataSource::Unresolved);

	// 2. With no asset it falls back, loudly and declaredly - the state a
	//    clean clone is in before `Tools/import_data.py` has run.
	//
	//    Cleared EXPLICITLY rather than relying on the default being unset.
	//    Since 2026-10-29 the default points at the importer's output path,
	//    so in an editor where that asset exists this test would otherwise
	//    resolve it and assert the fallback values against the asset's - and
	//    pass or fail depending on whether somebody had run the importer,
	//    which is not a test.
	//
	//    The fallback announces itself with a Warning rather than an Error by
	//    design: a clean clone is an expected state, not a broken one.
	Light->SetLanternData(TSoftObjectPtr<ULLKLanternData>());
	Light->ResolveAndApplyLight();
	TestEqual(TEXT("with no asset it reports the fallback"),
		Light->GetLightSource(), ELLKDataSource::Fallback);

	const FLLKLanternLightRules& Fallback = Light->GetResolvedRules();

	// 3. THE FALLBACK MUST AGREE WITH THE DATA. On this path the default IS
	//    the value, so these are the numbers the game runs on - and the one
	//    already-found bug of this exact shape in this project was a default
	//    that quietly disagreed with `lantern.json`.
	TestEqual(TEXT("the declared fallback radius matches lantern.json"), Fallback.RadiusStuds, 30.f);
	TestEqual(TEXT("the declared fallback cone matches lantern.json"), Fallback.ConeAngleDegrees, 72.f);
	TestEqual(TEXT("the declared fallback intensity matches lantern.json"), Fallback.Intensity, 250.f);
	TestTrue(TEXT("the declared fallback casts shadows, as lantern.json does"), Fallback.bCastsShadows);
	TestEqual(TEXT("the declared fallback colour is warm, as lantern.json is"), Fallback.WarmColor.R, 1.f);
	TestTrue(TEXT("and warm means r > g > b"),
		Fallback.WarmColor.R > Fallback.WarmColor.G && Fallback.WarmColor.G > Fallback.WarmColor.B);

	// 4. ROW 61, THE ONE THAT WOULD NOT LOOK LIKE A BUG. Read straight into
	//    AttenuationRadius, 30 studs is 30cm - a lantern lighting the keeper's
	//    own boots, which reads as a broken light rather than as wrong units.
	TestEqual(TEXT("the radius went through LLKUnits"),
		Light->AttenuationRadius, LLKUnits::DistanceFromStuds(30.f));
	TestEqual(TEXT("which is 840cm, not 30"), Light->AttenuationRadius, 840.f);

	// 5. The authored angle is the OUTER cone. Applied to the inner one the
	//    lit area would be wrong while every number in the asset was right.
	TestEqual(TEXT("the authored angle is the outer cone"), Light->OuterConeAngle, 72.f);
	TestTrue(TEXT("and the inner cone is strictly inside it"),
		Light->InnerConeAngle < Light->OuterConeAngle && Light->InnerConeAngle > 0.f);

	TestEqual(TEXT("intensity reached the light"), Light->Intensity, 250.f);

	// THE BUG OF 2026-10-29, now asserted. The unit is what makes the number
	// mean anything: ULocalLightComponent defaults to Unitless with Intensity
	// 5000, so an authored 6 was not a dim lantern but 6 on a scale whose
	// normal value is five thousand - invisible, and indistinguishable from a
	// toggle that never fired. Pinning the unit is the fix; asserting it is
	// what stops a project setting from quietly undoing the fix.
	TestEqual(TEXT("the intensity unit is pinned to candelas"),
		Light->IntensityUnits, ELightUnits::Candelas);
	TestTrue(TEXT("and the lantern is bright enough to see at all"), Light->Intensity > 50.f);
	TestTrue(TEXT("shadows reached the light"), Light->CastShadows);
	TestEqual(TEXT("the colour reached the light"), Light->LightColor.R, uint8(255));

	// 6. An asset wins over the fallback, and its numbers go through the same
	//    conversion. Values deliberately unlike the fallbacks so that a
	//    resolution which silently kept the fallback cannot pass.
	ULLKLanternData* Data = NewObject<ULLKLanternData>(GetTransientPackage());
	Data->Light.RadiusStuds = 11.f;
	Data->Light.ConeAngleDegrees = 50.f;
	Data->Light.Intensity = 2.f;
	Data->Light.bCastsShadows = false;
	Data->Light.WarmColor.R = 0.5f;
	Data->Light.WarmColor.G = 0.25f;
	Data->Light.WarmColor.B = 0.125f;

	Light->SetLanternData(Data);
	TestEqual(TEXT("an asset wins over the fallback"),
		Light->GetLightSource(), ELLKDataSource::Asset);
	TestEqual(TEXT("the asset's radius is the one reported"), Light->GetRadiusStuds(), 11.f);
	TestEqual(TEXT("and it went through the same conversion"),
		Light->AttenuationRadius, LLKUnits::DistanceFromStuds(11.f));
	TestEqual(TEXT("the asset's cone is the outer one"), Light->OuterConeAngle, 50.f);
	TestEqual(TEXT("the asset's intensity reached the light"), Light->Intensity, 2.f);
	TestFalse(TEXT("the asset can turn shadows off"), Light->CastShadows);

	// 7. Resolution is idempotent. It is public so a live retune can be
	//    pushed without a restart, which only holds if calling it twice is
	//    the same as calling it once.
	const float RadiusAfterFirst = Light->AttenuationRadius;
	Light->ResolveAndApplyLight();
	TestEqual(TEXT("resolving twice changes nothing"), Light->AttenuationRadius, RadiusAfterFirst);
	TestEqual(TEXT("and still reports the asset"),
		Light->GetLightSource(), ELLKDataSource::Asset);

	// 8. Resolution never turns the light on. The lantern's state decides
	//    that and nothing else does - the arrow points one way, and a retune
	//    that lit every dark keeper would be a rule change disguised as a
	//    tuning change.
	TestFalse(TEXT("resolving the numbers does not light the lantern"), Light->IsVisible());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
