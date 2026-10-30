// Named lantern levels, and the three ways they could quietly not work.
//
// Chris's call, 2026-10-30: *"Build it after the exposure pass as three named
// states: Hooded, Working, Open... The important implementation rule: each
// level needs a separate authored 'detectability' value, not just
// intensity/radius. Otherwise a dim lantern still marks its owner as fully
// lit."*
//
// He is describing a failure this project had already walked into twice on
// paper, and the whole of this file is the three ways it could happen again:
//
//   1. **A dial over intensity does nothing.** Every light threshold in the
//      game is `I * 0.5f`, so "lit" reduces to `d < r` and the intensity
//      cancels out of its own comparison. A level that only changes brightness
//      changes no rule at all - and because the room visibly dims, it looks
//      like it worked. Asserted here by holding detectability still and moving
//      intensity, and requiring every answer to stay put.
//   2. **A dial over radius does nothing either**, which is less obvious.
//      `ALLKSweeper` reads light at the keeper's OWN position, where their own
//      lantern sits at distance zero: brightness there is `I` against a bar of
//      `I/2`, cleared by a factor of two at any radius. So detectability is
//      authored instead, and asserted here to be what moves the Sweeper's
//      reach.
//   3. **A visual crossfade must not move gameplay.** Chris: *"transitions
//      smoothly lerp visually, while gameplay resolves to the named state."*
//      `GetLitLightSources` used to read `AttenuationRadius`, which is exactly
//      the value a fade is entitled to move. The seam is asserted here by
//      moving the component's attenuation radius behind the game's back and
//      requiring the gameplay reach not to follow.
//
// The fourth assertion is the one that makes this safe to land before any
// level is authored: **with no levels in the data, every answer is the one the
// game gave before this file existed.**

#include "Data/LLKLanternData.h"
#include "Enemies/LLKPerception.h"
#include "Data/LLKEnemyDef.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "LLKUnits.h"
#include "Misc/AutomationTest.h"
#include "Storm/LLKLightField.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKLanternLevelTest,
	"LLK.Lantern.Levels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace
{
	/** The authored Sweeper: 12 studs dark, 38 lit. The two ends of the range. */
	FLLKEnemyDef Sweeper()
	{
		FLLKEnemyDef Def;
		Def.Family = ELLKEnemyFamily::Sweepers;
		Def.AggroRadiusDark = 12.f;
		Def.AggroRadiusLit = 38.f;
		return Def;
	}

	FLLKLanternLightLevel Level(const FName Name, float RadiusStuds, float Cone, float Detectability)
	{
		FLLKLanternLightLevel Out;
		Out.Name = Name;
		Out.RadiusStuds = RadiusStuds;
		Out.ConeAngleDegrees = Cone;
		Out.Detectability = Detectability;
		return Out;
	}

	/** One source at a position, with an authored detectability. */
	FLLKLightSource Lantern(const FVector& Where, float Detectability, float Intensity = 250.f)
	{
		FLLKLightSource Source;
		Source.Position = Where;
		Source.Intensity = Intensity;
		Source.RadiusCm = LLKUnits::DistanceFromStuds(30.f);
		Source.Detectability = Detectability;
		return Source;
	}
}

bool FLLKLanternLevelTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK lantern level test reached RunTest."));

	// =====================================================================
	// 1. NO AUTHORED LEVELS IS THE STATE THE GAME IS IN TODAY
	// =====================================================================
	//
	// `data/lantern.json` has no `light.levels` key and `Tools/import_data.py`
	// would reject it if it did (its `exact()` check refuses unknown keys), so
	// every asset in existence resolves with an empty list. That has to be a
	// declared behaviour rather than a hole: the synthesised level carries the
	// block's own radius and cone at detectability 1, which is precisely the
	// single setting the game shipped with.
	{
		FLLKLanternLightRules Rules;
		Rules.RadiusStuds = 30.f;
		Rules.ConeAngleDegrees = 72.f;
		Rules.Intensity = 250.f;

		TestEqual(TEXT("an unlevelled lantern still has exactly one level"),
			Rules.LevelCount(), 1);

		const FLLKLanternLightLevel Synthesised = Rules.ResolveLevel(0);
		TestEqual(TEXT("and it carries the authored reach"), Synthesised.RadiusStuds, 30.f);
		TestEqual(TEXT("and the authored cone"), Synthesised.ConeAngleDegrees, 72.f);
		TestEqual(TEXT("and marks its holder exactly as much as light always did"),
			Synthesised.Detectability, 1.f);
		TestEqual(TEXT("and is the one Chris named as the practical setting"),
			Synthesised.Name, FName(TEXT("Working")));

		// An index nobody should have passed. Clamped rather than refused,
		// because the safe degradation for a light is to shine at an authored
		// setting - a light that goes dark on a caller bug is a black screen.
		TestEqual(TEXT("an out-of-range level clamps rather than going dark"),
			Rules.ResolveLevel(97).RadiusStuds, 30.f);
		TestEqual(TEXT("and a negative one does too"),
			Rules.ResolveLevel(-5).RadiusStuds, 30.f);
	}

	// =====================================================================
	// 2. THE DEFAULT LEVEL IS FOUND BY NAME, NOT BY POSITION
	// =====================================================================
	//
	// Resolved by name so that reordering `levels` in the JSON cannot silently
	// change what a lantern switches on to - a reorder is the kind of edit
	// nobody reviews.
	{
		FLLKLanternLightRules Rules;
		Rules.Levels = {
			Level(TEXT("Hooded"), 10.f, 44.f, 0.2f),
			Level(TEXT("Working"), 30.f, 72.f, 0.75f),
			Level(TEXT("Open"), 52.f, 100.f, 1.f),
		};

		TestEqual(TEXT("three authored levels are three levels"), Rules.LevelCount(), 3);
		TestEqual(TEXT("and Working is where a lantern starts"), Rules.DefaultLevelIndex(), 1);

		// Reordered, same answer. This is the assertion the name-lookup exists
		// for and the only one that would fail if somebody "simplified" it to
		// an index.
		Rules.Levels.Swap(0, 1);
		TestEqual(TEXT("reordering the list does not change which level is default"),
			Rules.Levels[Rules.DefaultLevelIndex()].Name, FName(TEXT("Working")));

		// A name nobody authored resolves to the first level, never to
		// INDEX_NONE: ResolveLevel would clamp -1 to 0 anyway, and returning
		// the dimmest setting is the quiet failure rather than the dangerous
		// one.
		Rules.DefaultLevelName = TEXT("Floodlight");
		TestEqual(TEXT("an unauthored default name falls back to the first level"),
			Rules.DefaultLevelIndex(), 0);
	}

	// =====================================================================
	// 3. DETECTABILITY IS WHAT MOVES AN ENEMY. INTENSITY IS NOT.
	// =====================================================================
	//
	// **The assertion this whole file is for.** Both halves matter: that
	// detectability moves the reach, and that intensity does not move it at
	// all. The second is what keeps `light.intensity` a value Chris can tune
	// by eye during a lighting pass (`docs/LIGHTING-IN-UNREAL.md` rule 2)
	// instead of a balance change needing a test run.
	{
		const FLLKEnemyDef Def = Sweeper();
		const FVector Origin = FVector::ZeroVector;

		// Detectability moves it, monotonically, across the authored span.
		const float Hooded = LLKPerception::NoticeRangeStuds(
			Def, LLKLightField::DetectabilityAt({ Lantern(Origin, 0.2f) }, Origin));
		const float Working = LLKPerception::NoticeRangeStuds(
			Def, LLKLightField::DetectabilityAt({ Lantern(Origin, 0.75f) }, Origin));
		const float Open = LLKPerception::NoticeRangeStuds(
			Def, LLKLightField::DetectabilityAt({ Lantern(Origin, 1.f) }, Origin));

		TestTrue(TEXT("a hooded lantern is noticed from nearer than a working one"),
			Hooded < Working);
		TestTrue(TEXT("and a working one from nearer than an open one"), Working < Open);
		TestTrue(TEXT("hooded still gives away more than being dark - Chris's slightest light"),
			Hooded > LLKPerception::NoticeRangeStuds(Def, 0.f));
		TestEqual(TEXT("and an open lantern lands exactly on the authored lit radius"),
			Open, 38.f);

		// **Now hold detectability still and move intensity by a factor of
		// forty.** Every answer must be identical. If this ever fails,
		// somebody has derived a gameplay threshold from intensity again and
		// the lighting pass has stopped being free.
		for (const float Candelas : { 10.f, 250.f, 400.f })
		{
			const float Reach = LLKPerception::NoticeRangeStuds(
				Def, LLKLightField::DetectabilityAt({ Lantern(Origin, 0.75f, Candelas) }, Origin));
			TestEqual(
				*FString::Printf(TEXT("at %.0f candelas the reach is unchanged"), Candelas),
				Reach, Working);
		}
	}

	// =====================================================================
	// 4. THE FIELD SUMS, CLAMPS, AND HAS NO CLIFF
	// =====================================================================
	{
		const FVector Origin = FVector::ZeroVector;
		const FVector Away = FVector(LLKUnits::DistanceFromStuds(30.f), 0.f, 0.f);

		TestEqual(TEXT("a dark crew is not detectable at all"),
			LLKLightField::DetectabilityAt({}, Origin), 0.f);

		// Summed, for the same physics reason brightness is: three keepers
		// huddled together give away more than one. `LLKLightField`'s header
		// makes this argument at length and it is the reason a frightened
		// crew's instinct to bunch up has a mechanical consequence.
		const float Alone = LLKLightField::DetectabilityAt({ Lantern(Origin, 0.3f) }, Origin);
		const float Huddled = LLKLightField::DetectabilityAt(
			{ Lantern(Origin, 0.3f), Lantern(Origin, 0.3f) }, Origin);
		TestTrue(TEXT("two hooded keepers standing together give away more than one"),
			Huddled > Alone);

		// But clamped at the top, which is the one place this differs from
		// brightness. The result indexes between two authored radii, and a
		// crew of four must not be noticed from four times the lit radius -
		// row 5 prices light, and a superlinear price on standing together
		// would delete the huddling the field is built to reward.
		const float Crowd = LLKLightField::DetectabilityAt(
			{ Lantern(Origin, 1.f), Lantern(Origin, 1.f), Lantern(Origin, 1.f) }, Origin);
		TestEqual(TEXT("but a whole crew cannot exceed fully marked"), Crowd, 1.f);
		TestEqual(TEXT("so four open lanterns still cap at the authored lit radius"),
			LLKPerception::NoticeRangeStuds(Sweeper(), Crowd), 38.f);

		// No cliff at the radius, deliberately - see LLKPerceptionFailureTest
		// for the design argument. At exactly the radius a teammate is half as
		// marked as the holder, and it keeps decaying from there.
		TestTrue(TEXT("at its own radius a lantern marks a teammate half as much"),
			FMath::IsNearlyEqual(
				LLKLightField::DetectabilityAt({ Lantern(Origin, 1.f) }, Away), 0.5f, 0.01f));

		// A source with no reach is not a light. Checked on radius only:
		// intensity is deliberately NOT a guard here, because a level's
		// brightness is free to be retuned to anything including very dim.
		FLLKLightSource Dimmest = Lantern(Origin, 0.5f, 0.0001f);
		TestTrue(TEXT("an almost invisible lantern still marks its holder"),
			LLKLightField::DetectabilityAt({ Dimmest }, Origin) > 0.f);

		FLLKLightSource NoReach = Lantern(Origin, 1.f);
		NoReach.RadiusCm = 0.f;
		TestEqual(TEXT("but a lantern with no reach marks nobody"),
			LLKLightField::DetectabilityAt({ NoReach }, Origin), 0.f);
	}

	// =====================================================================
	// 5. A VISUAL FADE CANNOT MOVE THE GAMEPLAY REACH
	// =====================================================================
	//
	// Chris's rule, and the seam it needs. `GetLitLightSources` read
	// `AttenuationRadius` until row 80, which was correct while a lantern had
	// one setting: nothing ever moved that value except a resolve. A visual
	// crossfade between two named levels moves it every frame of the
	// transition, so relic legibility and every Sweeper's reach would have
	// wobbled for the length of a fade.
	//
	// Simulated the only honest way - by moving the attenuation radius behind
	// the game's back, exactly as a fade would - and requiring the gameplay
	// answer not to follow.
	{
		ULLKLanternLightComponent* Light =
			NewObject<ULLKLanternLightComponent>(GetTransientPackage());

		// Cleared explicitly: the default points at the importer's output, so
		// in an editor where that asset exists this would otherwise assert
		// against whatever Chris last imported. LLKLanternLightTest found this
		// the hard way.
		Light->SetLanternData(TSoftObjectPtr<ULLKLanternData>());
		Light->ResolveAndApplyLight();

		const float ResolvedStuds = Light->GetRadiusStuds();
		const float GameplayCm = Light->GetGameplayRadiusCm();

		TestTrue(TEXT("the fallback light has a real reach to begin with"), ResolvedStuds > 0.f);
		TestEqual(TEXT("and the gameplay reach is the authored studs through LLKUnits"),
			GameplayCm, LLKUnits::DistanceFromStuds(ResolvedStuds));
		TestEqual(TEXT("which is what the component applied to the light as well"),
			Light->AttenuationRadius, GameplayCm);

		// The fade, in one line.
		Light->SetAttenuationRadius(GameplayCm * 0.25f);

		TestEqual(TEXT("a cosmetic fade moves the component's attenuation radius"),
			Light->AttenuationRadius, GameplayCm * 0.25f);
		TestEqual(TEXT("and the gameplay reach does not move with it"),
			Light->GetGameplayRadiusCm(), GameplayCm);
		TestEqual(TEXT("nor does the reach in studs that enemy logic agrees with"),
			Light->GetRadiusStuds(), ResolvedStuds);

		// And switching level is a no-op when there is only one, which is what
		// keeps `ELLKRunEvent::LightLevelChanged` a readable signal rather
		// than a stream of repeats.
		TestFalse(TEXT("switching to the level already in use reports no change"),
			Light->SetActiveLevelIndex(0));
		TestFalse(TEXT("and an out-of-range switch on a one-level lantern is also no change"),
			Light->SetActiveLevelIndex(2));
		TestEqual(TEXT("so the active level stays where it was"),
			Light->GetActiveLevelIndex(), 0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
