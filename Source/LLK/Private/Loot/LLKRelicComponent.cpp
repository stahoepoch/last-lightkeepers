#include "Loot/LLKRelicComponent.h"

#include "Engine/World.h"
#include "LLK.h"
#include "Net/UnrealNetwork.h"
#include "Storm/LLKStormChargeSubsystem.h"

#define LOCTEXT_NAMESPACE "LLKRelic"

ULLKRelicComponent::ULLKRelicComponent()
{
	// It ticks, unlike the lantern, because legibility depends on light that
	// moves - a keeper walking past is the event, and nothing broadcasts
	// "a lantern went near a relic". Throttled rather than per-frame.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void ULLKRelicComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Everyone sees what the crew has read. A relic one keeper identified is
	// identified for the crew - they are standing in the same room arguing
	// about whether it is worth the burden.
	DOREPLIFETIME(ULLKRelicComponent, bLegible);
}

void ULLKRelicComponent::BeginPlay()
{
	Super::BeginPlay();
	ResolveRules();
}

void ULLKRelicComponent::ResolveRules()
{
	bRelicsRequireLight = true;
	float LanternIntensity = FallbackLanternIntensity;
	RulesSource = ELLKDataSource::Fallback;

	if (const ULLKLootRulesData* Loot = LootRules.LoadSynchronous())
	{
		bRelicsRequireLight = Loot->Rules.bRelicsRequireLanternLight;
		RulesSource = ELLKDataSource::Asset;
	}

	// **The silent half of the same stale-asset bug.** A zero intensity here
	// makes the threshold zero, which makes every relic readable in pitch
	// darkness - row 53 deleted with no symptom whatsoever, because a relic
	// that is always readable looks exactly like a relic that works. Validated
	// for that reason and not for tidiness.
	const ULLKLanternData* Lantern = LanternData.LoadSynchronous();
	if (Lantern != nullptr && Lantern->Light.DescribesARealLight())
	{
		LanternIntensity = Lantern->Light.Intensity;
	}
	else
	{
		RulesSource = ELLKDataSource::Fallback;
		if (Lantern != nullptr)
		{
			UE_LOG(LogLLK, Error,
				TEXT("ULLKRelicComponent: %s resolved but its `light` block is empty, so the ")
				TEXT("identify threshold would have been zero and EVERY RELIC would read in ")
				TEXT("the dark. Regenerate the asset. Using the declared fallback intensity."),
				*Lantern->GetName());
		}
	}

	// **Derived, not chosen.** "Enough light to read by" is one lantern's worth
	// at the edge of its own reach: BrightnessAt for a single source at
	// distance == radius is Intensity / (1 + r^2/r^2) = Intensity / 2. So a
	// relic is readable exactly when a lantern's own attenuation radius
	// reaches it, which needs no new data key and retunes itself if Chris
	// changes the lantern.
	IdentifyThreshold = LanternIntensity * 0.5f;

	// Belt and braces, because the cost of being wrong here is row 53 with no
	// symptom. If anything ever reaches this line with a non-positive
	// threshold, a relic in a pitch-dark room satisfies `Brightness >=
	// Threshold` and reads itself.
	if (IdentifyThreshold <= 0.f)
	{
		IdentifyThreshold = FallbackLanternIntensity * 0.5f;
		UE_LOG(LogLLK, Error,
			TEXT("ULLKRelicComponent: identify threshold resolved to zero or less, which would ")
			TEXT("make every relic readable in darkness. Forced to %.1f."), IdentifyThreshold);
	}

	// `relicsRequireLanternLight: false` would mean relics read like baseline
	// loot. Honoured rather than ignored, because a flag the code quietly
	// overrides is worse than no flag.
	if (!bRelicsRequireLight && Visibility == ELLKLootVisibility::LanternOnly)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKRelicComponent: loot.json says relicsRequireLanternLight is false, so this ")
			TEXT("relic is legible without light despite being tiered LanternOnly."));
	}

	if (RulesSource == ELLKDataSource::Fallback)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKRelicComponent: rules not fully resolved - running on DECLARED FALLBACKS ")
			TEXT("(threshold %.1f from lantern intensity %.1f). Run Tools/import_data.py."),
			IdentifyThreshold, LanternIntensity);
	}

	// Baseline loot and a relic in a world where light is not required are
	// both readable from the start. Evaluated once here so a HUD drawn on the
	// first frame is already right.
	if (Visibility == ELLKLootVisibility::Always || !bRelicsRequireLight)
	{
		const AActor* Owner = GetOwner();
		if (Owner != nullptr && Owner->HasAuthority())
		{
			bCurrentlyLit = true;
			EvaluateLegibility(TNumericLimits<float>::Max());
		}
	}
}

void ULLKRelicComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SecondsSinceCheck += DeltaTime;
	if (SecondsSinceCheck < CheckIntervalSeconds)
	{
		return;
	}
	SecondsSinceCheck = 0.f;

	const UWorld* World = GetWorld();
	const AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr)
	{
		return;
	}

	const ULLKStormChargeSubsystem* Storm = World->GetSubsystem<ULLKStormChargeSubsystem>();
	if (Storm == nullptr)
	{
		return;
	}

	// The same summed field the brightest-position rule uses, evaluated at the
	// relic instead of at a keeper. One definition of "how lit is this spot",
	// so what the enemies converge on and what the player can read cannot
	// drift apart.
	const float Brightness = LLKLightField::BrightnessAt(Storm->GetLitLightSources(), Owner->GetActorLocation());

	// Clients evaluate too, but only to keep `bCurrentlyLit` honest for
	// `CanBeTaken` - which has to answer on the frame the player presses the
	// key, with no round trip. The legibility decision below is the server's.
	bCurrentlyLit = Brightness >= IdentifyThreshold;

	if (Owner->HasAuthority())
	{
		EvaluateLegibility(Brightness);
	}

	// Every machine, authority or not. This is the affordance channel, and it
	// belongs where the prompt is.
	RefreshInteractable();
}

void ULLKRelicComponent::RefreshInteractable()
{
	const bool bNow = CanBeTaken();
	if (bNow == bInteractableNow)
	{
		// Changed nothing, so said nothing. The throttled tick samples ten
		// times a second and a prompt wired to an unconditional broadcast
		// would rebuild itself ten times a second with it.
		return;
	}

	bInteractableNow = bNow;
	OnInteractableChanged.Broadcast(bNow, GetInteractBlock());
}

ELLKRelicInteractBlock ULLKRelicComponent::GetInteractBlock() const
{
	if (!bLegible)
	{
		return ELLKRelicInteractBlock::Unidentified;
	}

	if (!bRequiresLightToTake || Visibility == ELLKLootVisibility::Always)
	{
		return ELLKRelicInteractBlock::None;
	}

	// Read, but dark. The one a prompt should say out loud: a player reaching
	// for a relic they already identified and being told "needs light" is row
	// 5 explaining itself in the exact moment the rule applies to them.
	return bCurrentlyLit ? ELLKRelicInteractBlock::None : ELLKRelicInteractBlock::NeedsLight;
}

void ULLKRelicComponent::EvaluateLegibility(float BrightnessAtRelic)
{
	if (bLegible)
	{
		// Sticky. Identity is knowledge the player already paid for; charging
		// them again for the same relic would make the lesson a tax.
		return;
	}

	if (BrightnessAtRelic < IdentifyThreshold)
	{
		return;
	}

	bLegible = true;
	OnLegibilityChanged.Broadcast(true, ELLKRelicChangeReason::LitAndRead);
}

bool ULLKRelicComponent::CanBeTaken() const
{
	if (!bLegible)
	{
		// You cannot pocket what you cannot identify. This is the half of row
		// 53 that makes the relic a decision rather than a pickup.
		return false;
	}

	if (!bRequiresLightToTake || Visibility == ELLKLootVisibility::Always)
	{
		return true;
	}

	return bCurrentlyLit;
}

FText ULLKRelicComponent::GetDisplayName() const
{
	if (bLegible && !RelicName.IsEmpty())
	{
		return RelicName;
	}

	if (bLegible)
	{
		return LOCTEXT("UnnamedRelic", "Relic");
	}

	// **Not empty, and not hidden.** The player can see the thing; what they
	// cannot do is read it. A HUD given nothing would draw nothing, which is
	// the invisible-relic failure row 53 forbids, arriving through the UI
	// instead of through the spawner.
	return LOCTEXT("UnreadableRelic", "Something on the shelf");
}

void ULLKRelicComponent::SetRelicIdentity(FName InId, const FText& InName, int32 InValue)
{
	RelicId = InId;
	RelicName = InName;
	Value = InValue;
}

void ULLKRelicComponent::OnRep_Legible()
{
	// The client did not decide this and does not claim it did.
	OnLegibilityChanged.Broadcast(bLegible, ELLKRelicChangeReason::Replicated);

	// And the affordance may have changed with it, without waiting for the
	// next sample. A relic that became legible on the server while this client
	// was standing in its light is takeable the instant the state arrives.
	RefreshInteractable();
}

#undef LOCTEXT_NAMESPACE
