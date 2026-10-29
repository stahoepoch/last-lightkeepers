#include "Lantern/LLKLanternLightComponent.h"

#include "Engine/Scene.h"
#include "LLK.h"
#include "LLKUnits.h"

ULLKLanternLightComponent::ULLKLanternLightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Starts dark, matching ULLKLanternComponent's own default. Row 5 makes
	// being seen a decision, and a light that is on until told otherwise would
	// make the first second of every run a decision nobody got to make.
	SetVisibility(false);

	// Movable: the keeper carries it. Anything else bakes a light into a
	// lighting build that the player then walks away from.
	SetMobility(EComponentMobility::Movable);

	// **Pinned, because the unit is what makes the number mean anything.**
	// ULocalLightComponent defaults to Unitless with Intensity 5000, so an
	// authored 6 was not a dim lantern - it was 6 on a scale whose normal
	// value is five thousand, which is invisible rather than subtle. Assigned
	// directly rather than through SetIntensityUnits because that setter
	// refuses changes outside dynamic-data-allowed contexts, and a constructor
	// is one of them.
	//
	// It is pinned in code rather than authored in `data/` on purpose: the
	// unit is Unreal's concept, not a portable one, and leaving it to the
	// project default would mean `intensity` silently changing meaning if
	// somebody flipped a project setting.
	IntensityUnits = ELightUnits::Candelas;

	// The declared fallbacks, mirroring `data/lantern.json`'s `light` block.
	// Set here rather than as member initialisers so there is exactly one
	// place to compare against the JSON when either moves.
	FallbackRules.RadiusStuds = 30.f;
	FallbackRules.ConeAngleDegrees = 72.f;
	FallbackRules.Intensity = 250.f;
	FallbackRules.WarmColor.R = 1.f;
	FallbackRules.WarmColor.G = 0.82f;
	FallbackRules.WarmColor.B = 0.56f;
	FallbackRules.bCastsShadows = true;
}

void ULLKLanternLightComponent::BeginPlay()
{
	Super::BeginPlay();
	ResolveAndApplyLight();
}

void ULLKLanternLightComponent::SetLanternData(const TSoftObjectPtr<ULLKLanternData>& InData)
{
	// Unchanged pointer, nothing to do. Without this the owner's push right
	// after BeginPlay re-resolved the same asset and logged the same warning
	// twice, which is how a duplicate line in Chris's log pointed at a
	// redundant load nobody had noticed.
	if (LanternData == InData && LightSource != ELLKDataSource::Unresolved)
	{
		return;
	}

	LanternData = InData;
	ResolveAndApplyLight();
}

void ULLKLanternLightComponent::ResolveAndApplyLight()
{
	ResolvedRules = FallbackRules;
	LightSource = ELLKDataSource::Fallback;

	// Synchronous, and for the same reason the character's speeds are: these
	// are the numbers the light cannot exist without, and a frame rendered at
	// the wrong reach is a frame of gameplay built on a default.
	if (const ULLKLanternData* Data = LanternData.LoadSynchronous())
	{
		ResolvedRules = Data->Light;
		LightSource = ELLKDataSource::Asset;
	}

	// The one conversion, through the one authority (row 61). A raw radius in
	// centimetres here would make this the second place spatial values are
	// authored, which is precisely what LLKUnits.h exists to prevent.
	SetAttenuationRadius(LLKUnits::DistanceFromStuds(ResolvedRules.RadiusStuds));

	// Outer is the authored angle - the edge of what is lit, and the half
	// other systems read. Inner is a fraction of it and stays in code,
	// because it softens the falloff and changes nothing about reach.
	SetOuterConeAngle(ResolvedRules.ConeAngleDegrees);
	SetInnerConeAngle(ResolvedRules.ConeAngleDegrees * InnerConeFraction);

	SetIntensity(ResolvedRules.Intensity);
	SetLightColor(ResolvedRules.WarmColor.ToLinearColor());
	SetCastShadows(ResolvedRules.bCastsShadows);

	if (LightSource == ELLKDataSource::Fallback)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKLanternLightComponent: no ULLKLanternData resolved - running on DECLARED FALLBACK light ")
			TEXT("(radius %.0f studs / %.0f cm, cone %.0f deg, intensity %.1f, shadows %s). ")
			TEXT("Run Tools/import_data.py and set LanternData."),
			ResolvedRules.RadiusStuds, LLKUnits::DistanceFromStuds(ResolvedRules.RadiusStuds),
			ResolvedRules.ConeAngleDegrees, ResolvedRules.Intensity,
			ResolvedRules.bCastsShadows ? TEXT("on") : TEXT("off"));
	}
	else
	{
		UE_LOG(LogLLK, Log,
			TEXT("ULLKLanternLightComponent: light from ULLKLanternData - radius %.0f studs / %.0f cm, ")
			TEXT("cone %.0f deg, intensity %.1f."),
			ResolvedRules.RadiusStuds, LLKUnits::DistanceFromStuds(ResolvedRules.RadiusStuds),
			ResolvedRules.ConeAngleDegrees, ResolvedRules.Intensity);
	}
}

void ULLKLanternLightComponent::WatchLantern(ULLKLanternComponent* InLantern)
{
	if (InLantern == nullptr)
	{
		return;
	}

	if (ULLKLanternComponent* Previous = WatchedLantern.Get())
	{
		Previous->OnLanternStateChanged.RemoveDynamic(this, &ULLKLanternLightComponent::OnLanternStateChanged);
	}

	WatchedLantern = InLantern;
	InLantern->OnLanternStateChanged.AddDynamic(this, &ULLKLanternLightComponent::OnLanternStateChanged);

	// Match the current state rather than waiting for the next change. A
	// lantern that replicated in as already-lit before this component existed
	// would otherwise stay dark until the keeper toggled it twice.
	ApplyVisibleState(InLantern->IsLanternOn());
}

void ULLKLanternLightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ULLKLanternComponent* Watched = WatchedLantern.Get())
	{
		Watched->OnLanternStateChanged.RemoveDynamic(this, &ULLKLanternLightComponent::OnLanternStateChanged);
	}
	WatchedLantern.Reset();

	Super::EndPlay(EndPlayReason);
}

void ULLKLanternLightComponent::OnLanternStateChanged(bool bIsOn, ELLKLanternChangeReason Reason)
{
	// The reason is ignored here on purpose: a lantern forced off by a
	// knockdown and one the keeper switched off look identical. If they should
	// ever differ - a dying flicker, say - that is M1.3's real art, not this.
	ApplyVisibleState(bIsOn);
}

void ULLKLanternLightComponent::ApplyVisibleState(bool bIsOn)
{
	SetVisibility(bIsOn);
}
