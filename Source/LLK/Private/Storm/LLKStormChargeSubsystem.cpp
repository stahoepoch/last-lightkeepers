#include "Storm/LLKStormChargeSubsystem.h"

#include "Engine/World.h"
#include "LLK.h"
#include "Lantern/LLKLanternComponent.h"

namespace
{
	/**
	 * The declared fallbacks, mirroring `data/storm-charge.json`.
	 *
	 * This is a bigger copy than the character's three speeds and the cost is
	 * real, so it is in one place, loudly announced by `GetRulesSource()` and
	 * by a warning at startup. The alternative - refusing to run without
	 * `Content/Data/` - would make a clean clone un-runnable, and the
	 * alternative to THAT is a quiet zero that stops the night from ever
	 * escalating, which is the one failure mode nobody would notice.
	 */
	constexpr float FallbackBaselineRate = 1.f;
	constexpr float FallbackPerLanternBonus = 1.5f;
	constexpr float FallbackMaxMultiplier = 5.f;
	constexpr float FallbackDiminishingDecay = 0.6f;
	constexpr float FallbackWardenChargeValue = 700.f;
	constexpr float FallbackWardenLightSensitivity = 0.3f;
	constexpr float FallbackPatrolsAlertValue = 270.f;
	constexpr float FallbackPatrolsHuntValue = 480.f;
	constexpr float FallbackExtractionSpike = 180.f;
	constexpr float FallbackSirenSeconds = 8.f;
	constexpr float FallbackAnchorReviveSpike = 120.f;
}

ULLKStormChargeData* ULLKStormChargeSubsystem::MakeFallbackRules(UObject* Outer)
{
	ULLKStormChargeData* Fallback = NewObject<ULLKStormChargeData>(
		Outer != nullptr ? Outer : GetTransientPackage());

	Fallback->BaselineRatePerSecond = FallbackBaselineRate;

	Fallback->Lantern.PerLanternBonus = FallbackPerLanternBonus;
	Fallback->Lantern.MaxMultiplier = FallbackMaxMultiplier;
	// Row 12: diminishing is the active curve. The clamp above is inert under
	// it and kept only so switching back needs no code change.
	Fallback->Lantern.Curve = ELLKLanternCurve::Diminishing;
	Fallback->Lantern.DiminishingDecay = FallbackDiminishingDecay;

	// Row 54: the Warden is NOT one of the thresholds. Two entries, as in the data.
	FLLKStormThreshold Alert;
	Alert.Id = TEXT("PatrolsAlert");
	Alert.Value = FallbackPatrolsAlertValue;
	Fallback->Thresholds.Add(Alert);

	FLLKStormThreshold Hunt;
	Hunt.Id = TEXT("PatrolsHunt");
	Hunt.Value = FallbackPatrolsHuntValue;
	Fallback->Thresholds.Add(Hunt);

	Fallback->Warden.ChargeValue = FallbackWardenChargeValue;
	Fallback->Warden.LightSensitivity = FallbackWardenLightSensitivity;

	// The two spikes, because they are meter values and this class applies
	// them. Everything else on the asset - loot payouts, the run's band,
	// the revive timings - is somebody else's row and is left at zero rather
	// than half-copied here, where a stale duplicate would be worse than an
	// obvious hole.
	Fallback->ExtractionCall.StormChargeSpike = FallbackExtractionSpike;
	Fallback->ExtractionCall.SirenSeconds = FallbackSirenSeconds;
	Fallback->ExtractionCall.bConvergeAllPatrols = true;
	Fallback->AnchorRevive.StormChargeSpike = FallbackAnchorReviveSpike;

	return Fallback;
}

bool ULLKStormChargeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// Game and PIE worlds only. An editor preview world ticking a storm meter
	// would accrue charge while nobody is playing, and an asset thumbnail
	// world would do it dozens of times over.
	const UWorld* World = Cast<UWorld>(Outer);
	return World != nullptr && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void ULLKStormChargeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResolveRules();
}

void ULLKStormChargeSubsystem::ResolveRules()
{
	// The importer's output path. Soft-loaded by name rather than configured,
	// because a subsystem has no designer-editable default to point at one.
	Rules = ::LoadObject<ULLKStormChargeData>(nullptr, TEXT("/Game/Data/DA_StormCharge.DA_StormCharge"));
	RulesSource = Rules != nullptr ? ELLKDataSource::Asset : ELLKDataSource::Fallback;

	if (RulesSource == ELLKDataSource::Fallback)
	{
		Rules = MakeFallbackRules(this);
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKStormChargeSubsystem: no /Game/Data/DA_StormCharge - running on DECLARED FALLBACK tuning ")
			TEXT("(baseline %.2f/s, per-lantern +%.2f, decay %.2f, Warden %.0f at sensitivity %.2f). ")
			TEXT("Run Tools/import_data.py."),
			Rules->BaselineRatePerSecond, Rules->Lantern.PerLanternBonus,
			Rules->Lantern.DiminishingDecay, Rules->Warden.ChargeValue,
			Rules->Warden.LightSensitivity);
	}
	else
	{
		UE_LOG(LogLLK, Log,
			TEXT("ULLKStormChargeSubsystem: tuning from ULLKStormChargeData - baseline %.2f/s, ")
			TEXT("%d thresholds, Warden %.0f."),
			Rules->BaselineRatePerSecond, Rules->Thresholds.Num(), Rules->Warden.ChargeValue);
	}
}

TStatId ULLKStormChargeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULLKStormChargeSubsystem, STATGROUP_Tickables);
}

bool ULLKStormChargeSubsystem::HasStormAuthority() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->GetNetMode() != NM_Client;
}

void ULLKStormChargeSubsystem::RegisterLantern(ULLKLanternComponent* Lantern)
{
	if (Lantern == nullptr)
	{
		return;
	}

	// AddUnique: registering twice would make one keeper's light count twice,
	// which under the diminishing curve is not merely double but a different
	// point on the curve entirely.
	Lanterns.AddUnique(Lantern);
}

void ULLKStormChargeSubsystem::UnregisterLantern(ULLKLanternComponent* Lantern)
{
	Lanterns.Remove(Lantern);
}

int32 ULLKStormChargeSubsystem::GetLitLanternCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ULLKLanternComponent>& Weak : Lanterns)
	{
		const ULLKLanternComponent* Lantern = Weak.Get();
		if (Lantern != nullptr && Lantern->IsLanternOn())
		{
			++Count;
		}
	}
	return Count;
}

float ULLKStormChargeSubsystem::LanternMultiplier(const FLLKLanternChargeRules& InRules, int32 LitCount)
{
	// A dark crew pays the baseline and nothing more. Not a special case so
	// much as the curve's own value at zero, stated rather than computed:
	// `decay^0` is 1, so the series is 0 and the multiplier is 1.
	if (LitCount <= 0)
	{
		return 1.f;
	}

	if (InRules.Curve == ELLKLanternCurve::Diminishing)
	{
		// `(1 - decay^n) / (1 - decay)`, with the limit taken at decay == 1
		// where that expression divides by zero and the series is just n.
		const float Decay = InRules.DiminishingDecay;
		const float Series = FMath::IsNearlyEqual(Decay, 1.f)
			? static_cast<float>(LitCount)
			: (1.f - FMath::Pow(Decay, static_cast<float>(LitCount))) / (1.f - Decay);
		return 1.f + InRules.PerLanternBonus * Series;
	}

	// Retired on 2026-10-27 and deliberately still here, row 12: every lantern
	// past the clamp is free under this curve, which is why it was retired.
	return FMath::Min(1.f + InRules.PerLanternBonus * static_cast<float>(LitCount), InRules.MaxMultiplier);
}

float ULLKStormChargeSubsystem::WardenMultiplierFrom(float LanternMult, float LightSensitivity)
{
	return 1.f + LightSensitivity * (LanternMult - 1.f);
}

float ULLKStormChargeSubsystem::GetCurrentMultiplier() const
{
	return Rules != nullptr ? LanternMultiplier(Rules->Lantern, GetLitLanternCount()) : 1.f;
}

float ULLKStormChargeSubsystem::GetCurrentRate() const
{
	return Rules != nullptr ? Rules->BaselineRatePerSecond * GetCurrentMultiplier() : 0.f;
}

float ULLKStormChargeSubsystem::GetCurrentWardenMultiplier() const
{
	if (Rules == nullptr)
	{
		return 1.f;
	}
	return WardenMultiplierFrom(GetCurrentMultiplier(), Rules->Warden.LightSensitivity);
}

bool ULLKStormChargeSubsystem::GetNextEscalation(FName& OutId, float& OutTargetCharge, float& OutCurrentCharge) const
{
	if (Rules == nullptr)
	{
		return false;
	}

	// Lowest uncrossed threshold first. Not "the next in array order": the
	// data is authored ascending, but a readout that trusted the ordering
	// would start lying the first time somebody inserted a stage.
	const FLLKStormThreshold* Best = nullptr;
	for (const FLLKStormThreshold& Threshold : Rules->Thresholds)
	{
		if (CrossedThresholds.Contains(Threshold.Id))
		{
			continue;
		}
		if (Best == nullptr || Threshold.Value < Best->Value)
		{
			Best = &Threshold;
		}
	}

	if (Best != nullptr)
	{
		OutId = Best->Id;
		OutTargetCharge = Best->Value;
		OutCurrentCharge = Charge;
		return true;
	}

	// Everything crossed, so the Warden is what is left to arrive - and it
	// reads from its OWN meter, row 54. Reporting the thresholds' charge here
	// would make the countdown wrong by the whole of the dampening.
	if (!bWardenConverged)
	{
		OutId = TEXT("WardenConverges");
		OutTargetCharge = Rules->Warden.ChargeValue;
		OutCurrentCharge = WardenCharge;
		return true;
	}

	// Nothing left to arrive. The one state a readout must not render as a
	// countdown to zero.
	return false;
}

float ULLKStormChargeSubsystem::SecondsUntilCharge(float FromCharge, float TargetCharge, int32 LitCount) const
{
	if (Rules == nullptr)
	{
		return TNumericLimits<float>::Max();
	}

	const float Rate = Rules->BaselineRatePerSecond * LanternMultiplier(Rules->Lantern, LitCount);
	if (Rate <= 0.f)
	{
		// Never a division, and never a plausible zero. A baseline tuned to 0
		// means the night does not advance, and "0 seconds away" would say the
		// precise opposite of that.
		return TNumericLimits<float>::Max();
	}

	// Negative when already passed, deliberately: the caller decides whether
	// that reads as "crossed" or as nothing, and clamping here would hide the
	// difference between "due now" and "long gone".
	return (TargetCharge - FromCharge) / Rate;
}

float ULLKStormChargeSubsystem::SecondsUntilWarden(int32 LitCount) const
{
	if (Rules == nullptr)
	{
		return TNumericLimits<float>::Max();
	}

	const float Multiplier = LanternMultiplier(Rules->Lantern, LitCount);
	const float Rate = Rules->BaselineRatePerSecond
		* WardenMultiplierFrom(Multiplier, Rules->Warden.LightSensitivity);
	if (Rate <= 0.f)
	{
		return TNumericLimits<float>::Max();
	}

	return (Rules->Warden.ChargeValue - WardenCharge) / Rate;
}

void ULLKStormChargeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasStormAuthority())
	{
		return;
	}

	Advance(DeltaTime, GetLitLanternCount());
}

void ULLKStormChargeSubsystem::Advance(float DeltaSeconds, int32 LitCount)
{
	if (Rules == nullptr || DeltaSeconds <= 0.f)
	{
		return;
	}

	ElapsedSeconds += DeltaSeconds;

	const float Multiplier = LanternMultiplier(Rules->Lantern, LitCount);
	Charge += Rules->BaselineRatePerSecond * Multiplier * DeltaSeconds;

	// The Warden's own meter, row 54. Derived from the same multiplier but
	// accumulated separately, so that dampening changes when it arrives
	// without ever changing what the thresholds mean.
	const float WardenMultiplier = WardenMultiplierFrom(Multiplier, Rules->Warden.LightSensitivity);
	WardenCharge += Rules->BaselineRatePerSecond * WardenMultiplier * DeltaSeconds;

	AnnounceCrossings();
}

void ULLKStormChargeSubsystem::AnnounceCrossings()
{
	if (Rules == nullptr)
	{
		return;
	}

	for (const FLLKStormThreshold& Threshold : Rules->Thresholds)
	{
		if (Charge >= Threshold.Value && !CrossedThresholds.Contains(Threshold.Id))
		{
			CrossedThresholds.Add(Threshold.Id);
			UE_LOG(LogLLK, Log, TEXT("Storm Charge crossed %s at %.0f (%.0fs elapsed)."),
				*Threshold.Id.ToString(), Charge, ElapsedSeconds);
			OnThresholdCrossed.Broadcast(Threshold.Id, Charge);
		}
	}

	if (!bWardenConverged && WardenCharge >= Rules->Warden.ChargeValue)
	{
		bWardenConverged = true;
		UE_LOG(LogLLK, Log, TEXT("The Warden converges at %.0fs elapsed (Warden meter %.0f)."),
			ElapsedSeconds, WardenCharge);
		OnWardenConverged.Broadcast();
	}
}

void ULLKStormChargeSubsystem::AddSpike(float Amount, FName Reason)
{
	// Row 6, enforced rather than documented. A negative spike is dropped, not
	// applied, so no expression anywhere in the codebase lowers this meter -
	// including an extraction call whose tuning someone later sets negative.
	if (Amount <= 0.f)
	{
		if (Amount < 0.f)
		{
			UE_LOG(LogLLK, Warning,
				TEXT("ULLKStormChargeSubsystem: refused a negative spike of %.0f from %s. ")
				TEXT("Row 6: the meter only ever rises."),
				Amount, *Reason.ToString());
		}
		return;
	}

	Charge += Amount;

	// **The spike hits both meters, and that is an assumption, not data.**
	// `data/storm-charge.json` gives the extraction call 180 and the anchor
	// revive 120, and says nothing about whether the Warden's dampened meter
	// takes them too. Applying them undampened is the reading that matches
	// Chris's words - "a deliberate loud spike: flare, siren, enemies
	// converging, regardless of current Storm Charge" - since the dampening
	// exists to stop a LANTERN from summoning the Warden, and a siren is not a
	// lantern. If he wants the Warden deaf to sirens, this is the line, and it
	// wants a `spikeReachesWarden` key in the data rather than a decision here.
	WardenCharge += Amount;

	UE_LOG(LogLLK, Log, TEXT("Storm Charge spike: +%.0f from %s (now %.0f / Warden %.0f)."),
		Amount, *Reason.ToString(), Charge, WardenCharge);

	// Announced at the moment of the call rather than on the next tick: a
	// spike that crosses a threshold is the game telling the player what their
	// decision just cost, and a frame late is a frame detached from the cause.
	AnnounceCrossings();
}
