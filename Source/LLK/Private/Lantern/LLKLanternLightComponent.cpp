#include "Lantern/LLKLanternLightComponent.h"

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

	// A held lantern casts shadows or it is a flashlight-shaped fog. Row 5's
	// trade is about what you reveal, and revealed geometry with no shadow
	// reads as flat. Placeholder quality, not a final look.
	SetCastShadows(true);

	// Wide enough to light a room you are standing in rather than a dot on a
	// wall. The keeper holds a lantern, not a torch.
	SetInnerConeAngle(34.f);
	SetOuterConeAngle(72.f);

	// Warm, because every other light in this game is the storm. Placeholder.
	SetLightColor(FLinearColor(1.f, 0.82f, 0.56f));
	SetIntensity(6.f);
}

void ULLKLanternLightComponent::BeginPlay()
{
	Super::BeginPlay();

	// The one conversion, through the one authority (row 61). A raw radius in
	// centimetres here would be the second place spatial values are authored.
	SetAttenuationRadius(LLKUnits::DistanceFromStuds(PlaceholderRadiusStuds));
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
