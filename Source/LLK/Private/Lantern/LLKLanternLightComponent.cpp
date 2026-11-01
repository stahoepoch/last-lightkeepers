#include "Lantern/LLKLanternLightComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/Scene.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LLK.h"
#include "LLKUnits.h"

ULLKLanternLightComponent::ULLKLanternLightComponent()
{
	// Ticks for the cosmetic transform only. **Nothing gameplay reads is
	// computed here**, which is what makes a tick acceptable at all in a class
	// whose header calls itself a listener: a tick that could change what the
	// storm sees would be a second writer of a replicated fact.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

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

FVector ULLKLanternLightComponent::GetGameplayLightLocation() const
{
	if (const USceneComponent* Anchor = GetAttachParent())
	{
		// The parent is the keeper's stable hand-space anchor. Feel writes this
		// component's relative offset only, so it cannot leak into gameplay.
		return Anchor->GetComponentLocation();
	}

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		// A component with no owner is not in a world, so there is no honest
		// anchor to derive. Returning the component's own location is the only
		// answer available and it is noted rather than hidden: if this is ever
		// reached in a running game, the cosmetic position has become the
		// gameplay position and that is the bug this method exists to prevent.
		return GetComponentLocation();
	}

	return LLKLanternFeel::GameplayLightPosition(
		Owner->GetActorLocation(), LLKLanternFeel::KeeperEyeHeightCm);
}

void ULLKLanternLightComponent::TickFeel(float DeltaSeconds)
{
	FLLKLanternFeelInput Input;
	Input.DeltaSeconds = DeltaSeconds;

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	// The look, read and never written. Guarantee 1: the cone trails the
	// camera and the camera trails nothing, so this is an input only.
	Input.Look = GetComponentRotation();

	const FVector Velocity = Owner->GetVelocity();
	Input.SpeedCm = FVector(Velocity.X, Velocity.Y, 0.f).Size();

	if (const ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Input.bSprinting = Input.SpeedCm > LLKUnits::SpeedFromStuds(18.f);

			// A landing is an edge, not a state: airborne last frame and not
			// this one. Read here rather than from ACharacter::Landed so this
			// component needs no hook in a class it does not own.
			const bool bFalling = Movement->IsFalling();
			if (bWasFalling && !bFalling)
			{
				Input.LandingImpactSpeed = FMath::Abs(LastFallSpeed);
			}
			if (bFalling)
			{
				LastFallSpeed = Velocity.Z;
			}
			bWasFalling = bFalling;
		}
	}

	// Guarantee 2, measured here and decided in LLKLanternFeel. A sphere
	// trace from the honest anchor toward where the cone wants to be, so the
	// clamp knows how much room there actually is. Negative means unmeasured,
	// which collapses the offset to zero rather than guessing.
	Input.FreeDistanceCm = -1.f;
	if (const UWorld* World = GetWorld())
	{
		const FVector Anchor = GetGameplayLightLocation();
		const FVector Desired = Anchor + GetComponentTransform().TransformVector(FeelState.OffsetCm);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(LLKLanternFeel), /*bTraceComplex=*/false);
		Params.AddIgnoredActor(Owner);

		FHitResult Hit;
		const bool bHit = World->SweepSingleByChannel(
			Hit, Anchor, Desired, FQuat::Identity, ECC_Visibility,
			FCollisionShape::MakeSphere(4.f), Params);

		Input.FreeDistanceCm = bHit
			? Hit.Distance
			: FMath::Max(0.f, FeelRules.MaxOffsetCm);
	}

	FeelState = LLKLanternFeel::AdvanceFeel(FeelState, Input, FeelRules);

	// Applied to the component's own relative transform, which is cosmetic by
	// construction: GetGameplayLightLocation does not read it.
	SetRelativeLocation(BaseRelativeLocation + FeelState.OffsetCm);
}

void ULLKLanternLightComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickFeel(DeltaTime);
}

void ULLKLanternLightComponent::BeginPlay()
{
	Super::BeginPlay();

	// Captured once, because the sway writes the relative location every frame
	// and an offset added to its own result is a drift.
	BaseRelativeLocation = GetRelativeLocation();
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
		// **Validated, not trusted.** An asset generated before the `light`
		// block existed resolves fine and hands back zeros, which is how this
		// shone nothing while logging "light from ULLKLanternData". Resolving
		// is not the same as being current.
		if (Data->Light.DescribesARealLight())
		{
			ResolvedRules = Data->Light;
			LightSource = ELLKDataSource::Asset;
		}
		else
		{
			UE_LOG(LogLLK, Error,
				TEXT("ULLKLanternLightComponent: %s resolved but its `light` block is empty ")
				TEXT("(radius %.0f, cone %.0f, intensity %.0f). The asset predates the light ")
				TEXT("block in data/lantern.json and needs regenerating - run the import ")
				TEXT("commandlet. Using declared fallbacks so the lantern still works."),
				*Data->GetName(), Data->Light.RadiusStuds, Data->Light.ConeAngleDegrees,
				Data->Light.Intensity);
		}
	}

	// The level comes from the data by NAME, so reordering `levels` in the
	// JSON cannot change what a lantern switches on to. Clamped by
	// DefaultLevelIndex, which returns 0 rather than INDEX_NONE for a name
	// nobody authored - a light with no level would be a light with no reach.
	ActiveLevelIndex = ResolvedRules.DefaultLevelIndex();
	ApplyActiveLevel();

	SetIntensity(ResolvedRules.Intensity);
	SetLightColor(ResolvedRules.WarmColor.ToLinearColor());
	SetCastShadows(ResolvedRules.bCastsShadows);

	if (LightSource == ELLKDataSource::Fallback)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKLanternLightComponent: no ULLKLanternData resolved - running on DECLARED FALLBACK light ")
			TEXT("(radius %.0f studs / %.0f cm, cone %.0f deg, intensity %.1f, shadows %s). ")
			TEXT("Run Tools/import_data.py and set LanternData."),
			GetRadiusStuds(), GetGameplayRadiusCm(),
			GetActiveLevel().ConeAngleDegrees, ResolvedRules.Intensity,
			ResolvedRules.bCastsShadows ? TEXT("on") : TEXT("off"));
	}
	else
	{
		// The level count is in the log on purpose. `light.levels` does not
		// exist in `data/lantern.json` yet, so a current asset reports ONE
		// level - which is the line that tells whoever reads it that the
		// importer has not been taught the key rather than that the dial is
		// broken. See docs/HANDOFF-CODEX-LANTERN-LEVELS.md.
		const FLLKLanternLightLevel Level = GetActiveLevel();
		UE_LOG(LogLLK, Log,
			TEXT("ULLKLanternLightComponent: light from ULLKLanternData - level %s (%d of %d), ")
			TEXT("radius %.0f studs / %.0f cm, cone %.0f deg, intensity %.1f, detectability %.2f."),
			*Level.Name.ToString(), ActiveLevelIndex + 1, GetLevelCount(),
			Level.RadiusStuds, GetGameplayRadiusCm(),
			Level.ConeAngleDegrees, ResolvedRules.Intensity, Level.Detectability);
	}
}

void ULLKLanternLightComponent::ApplyActiveLevel()
{
	const FLLKLanternLightLevel Level = GetActiveLevel();

	// The one conversion, through the one authority (row 61). A raw radius in
	// centimetres here would make this the second place spatial values are
	// authored, which is precisely what LLKUnits.h exists to prevent.
	//
	// Applied instantly. Chris's rule is that gameplay resolves to the named
	// state while the LOOK lerps, so a future crossfade belongs in TickFeel
	// alongside the sway - never here, and never where
	// GetGameplayRadiusCm could read it.
	SetAttenuationRadius(LLKUnits::DistanceFromStuds(Level.RadiusStuds));

	// Outer is the authored angle - the edge of what is lit, and the half
	// other systems read. Inner is a fraction of it and stays in code,
	// because it softens the falloff and changes nothing about reach.
	SetOuterConeAngle(Level.ConeAngleDegrees);
	SetInnerConeAngle(Level.ConeAngleDegrees * InnerConeFraction);
}

bool ULLKLanternLightComponent::SetActiveLevelIndex(int32 Index)
{
	const int32 Clamped = FMath::Clamp(Index, 0, GetLevelCount() - 1);
	if (Clamped == ActiveLevelIndex)
	{
		return false;
	}

	ActiveLevelIndex = Clamped;
	ApplyActiveLevel();

	const FLLKLanternLightLevel Level = GetActiveLevel();
	UE_LOG(LogLLK, Log,
		TEXT("ULLKLanternLightComponent: level %s - reach %.0f studs, cone %.0f deg, ")
		TEXT("detectability %.2f."),
		*Level.Name.ToString(), Level.RadiusStuds, Level.ConeAngleDegrees, Level.Detectability);
	return true;
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
