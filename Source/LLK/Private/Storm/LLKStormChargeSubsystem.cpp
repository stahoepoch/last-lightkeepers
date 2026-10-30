#include "Storm/LLKStormChargeSubsystem.h"

#include "Engine/World.h"
#include "LLK.h"
#include "LLKGameState.h"
#include "Lantern/LLKLanternComponent.h"
#include "Lantern/LLKLanternLightComponent.h"

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
	constexpr float FallbackWardenChargeValue = 700.f;
	constexpr float FallbackPatrolsAlertValue = 270.f;
	constexpr float FallbackPatrolsHuntValue = 480.f;
	constexpr float FallbackSirenSeconds = 8.f;
}

ULLKStormChargeData* ULLKStormChargeSubsystem::MakeFallbackRules(UObject* Outer)
{
	ULLKStormChargeData* Fallback = NewObject<ULLKStormChargeData>(
		Outer != nullptr ? Outer : GetTransientPackage());

	Fallback->BaselineRatePerSecond = FallbackBaselineRate;

	// **The lantern block is deliberately NOT populated here, as of row 72.**
	// The rate does not read it, so a fallback that filled it in would be
	// declaring a curve nothing consults - and the next person to find those
	// four lines would reasonably conclude the curve was still live. The keys
	// remain in `data/storm-charge.json` because deleting values there is
	// Chris's call (row 55); inventing them here is not.

	// Row 54: the Warden is NOT one of the thresholds. Two entries, as in the data.
	// **The effect sentences are part of the fallback, not decoration.**
	// M2.3 prints the authored `effect` beside the stage, and `LLK.Storm.
	// NightStage` caught these two being empty here: a fallback threshold
	// with an id and a value but no consequence text is resolved-but-
	// degenerate, which is the same shape of bug as the lantern that
	// resolved to zero radius and cost an evening. A stage badge with a
	// blank consequence line looks like a threshold that changes nothing,
	// not like data that failed to load.
	//
	// Kept verbatim in step with `data/storm-charge.json`, like every other
	// value in this function.
	FLLKStormThreshold Alert;
	Alert.Id = TEXT("PatrolsAlert");
	Alert.Value = FallbackPatrolsAlertValue;
	Alert.Effect = TEXT("Patrol routes tighten and Sweepers investigate noise further from their spawn.");
	Fallback->Thresholds.Add(Alert);

	FLLKStormThreshold Hunt;
	Hunt.Id = TEXT("PatrolsHunt");
	Hunt.Value = FallbackPatrolsHuntValue;
	Hunt.Effect = TEXT("All patrols actively hunt. Glimmer scan frequency increases.");
	Fallback->Thresholds.Add(Hunt);

	// Row 54 keeps the Warden its own arrival VALUE. Row 72 took away its
	// own meter, so `LightSensitivity` - the dampening that made its second
	// meter rise more slowly than the first - is no longer set here for the
	// same reason the lantern block is not.
	Fallback->Warden.ChargeValue = FallbackWardenChargeValue;

	// The two spikes, because they are meter values and this class applies
	// them. Everything else on the asset - loot payouts, the run's band,
	// the revive timings - is somebody else's row and is left at zero rather
	// than half-copied here, where a stale duplicate would be worse than an
	// obvious hole.
	// Row 73: no spike, so no spike price. The extraction call's real price
	// is the two lines below - everything converges and a siren runs for
	// eight seconds - and it was always the better half of that pricing.
	Fallback->ExtractionCall.SirenSeconds = FallbackSirenSeconds;
	Fallback->ExtractionCall.bConvergeAllPatrols = true;

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
			TEXT("(baseline %.2f/s flat, Warden at %.0f). Run Tools/import_data.py."),
			Rules->BaselineRatePerSecond, Rules->Warden.ChargeValue);
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

void ULLKStormChargeSubsystem::AddDistressPulse(
	const FVector& Position, float Intensity, float RadiusCm, float Seconds)
{
	// **Row 72's replacement price for the Anchor Revive.** It used to cost a
	// permanent spike of 120 on the global meter, which row 72 forbids: a solo
	// keeper getting up cannot make the weather worse. Chris's price instead:
	// *"it consumes the one emergency Anchor for that run and sends a local
	// distress pulse - bright, audible, and attractive to nearby machines -
	// but does not alter the global clock. A solo player gets back up, but
	// wakes up the room."*
	//
	// **Paid in the currency the game already has.** The pulse is a light
	// source in the field for the duration of the revive, so for those seconds
	// the keeper is very likely the brightest thing in the building - which
	// means `FindBrightestPosition` sends the Warden at them and
	// `LLKPerception` notices them, with no new enemy code and no new authored
	// number. Every value comes from the keeper's own lantern rules and from
	// `revive.reviveSeconds`.
	if (Intensity <= 0.f || RadiusCm <= 0.f || Seconds <= 0.f)
	{
		// A pulse with no reach, no brightness or no duration is not a pulse.
		// Refused rather than added as a zero, for the same reason the light
		// field skips a lantern with no radius: an entry that contributes
		// nothing is an entry that looks like it should.
		UE_LOG(LogLLK, Warning,
			TEXT("Distress pulse refused: intensity %.1f, radius %.1fcm, %.2fs."),
			Intensity, RadiusCm, Seconds);
		return;
	}

	FLLKDistressPulse Pulse;
	Pulse.Position = Position;
	Pulse.Intensity = Intensity;
	Pulse.RadiusCm = RadiusCm;
	Pulse.SecondsRemaining = Seconds;
	DistressPulses.Add(Pulse);

	UE_LOG(LogLLK, Log,
		TEXT("Distress pulse at %s: %.0f intensity, %.0fcm, %.1fs. The room is awake."),
		*Position.ToCompactString(), Intensity, RadiusCm, Seconds);

	OnDistressPulse.Broadcast(Position, Seconds);
}

void ULLKStormChargeSubsystem::ExpireDistressPulses(float DeltaSeconds)
{
	for (int32 Index = DistressPulses.Num() - 1; Index >= 0; --Index)
	{
		DistressPulses[Index].SecondsRemaining -= DeltaSeconds;
		if (DistressPulses[Index].SecondsRemaining <= 0.f)
		{
			DistressPulses.RemoveAtSwap(Index);
		}
	}
}

TArray<FLLKLightSource> ULLKStormChargeSubsystem::GetLitLightSources() const
{
	TArray<FLLKLightSource> Sources;

	// Pulses first, so a revive's light is in the field on the same frame it
	// was spent. They are sources exactly like a lantern is: nothing
	// downstream knows the difference, which is what makes "wakes up the room"
	// a mechanical fact rather than a sentence in a design document.
	for (const FLLKDistressPulse& Pulse : DistressPulses)
	{
		FLLKLightSource Source;
		Source.Position = Pulse.Position;
		Source.Intensity = Pulse.Intensity;
		Source.RadiusCm = Pulse.RadiusCm;
		Sources.Add(Source);
	}

	for (const TWeakObjectPtr<ULLKLanternComponent>& Weak : Lanterns)
	{
		const ULLKLanternComponent* Lantern = Weak.Get();
		if (Lantern == nullptr || !Lantern->IsLanternOn())
		{
			// An unlit lantern contributes nothing, and a destroyed one
			// resolves to null - the same no-ghost-light property the count
			// relies on, for the same reason.
			continue;
		}

		const AActor* Owner = Lantern->GetOwner();
		if (Owner == nullptr)
		{
			continue;
		}

		// The LIGHT component, not the lantern component, because the light is
		// where the brightness and the reach are - and where the light
		// actually is in the world, which on a first-person keeper is the
		// camera rather than the capsule's centre.
		const ULLKLanternLightComponent* Light =
			Owner->FindComponentByClass<ULLKLanternLightComponent>();
		if (Light == nullptr)
		{
			// A lantern whose owner has no light component is lit as far as
			// the storm is concerned - it still costs charge - but it has no
			// position or reach to offer the field. Counted by
			// GetLitLanternCount and absent here, deliberately: inventing an
			// intensity for it would put a made-up number into enemy
			// targeting.
			continue;
		}

		FLLKLightSource Source;
		// **The honest anchor, not the component's transform.** Chris found
		// this before it shipped: the light component is both the visible cone
		// and the position this function reports, so once the cone sways,
		// GetComponentLocation() is a position the keeper never occupied -
		// and this value decides storm accrual, relic legibility and what the
		// Warden converges on. GetGameplayLightLocation derives from the
		// owner's capsule and an authored eye height, with no cosmetic term in
		// it. LLK.Lantern.Feel asserts that no sway can change it.
		Source.Position = Light->GetGameplayLightLocation();
		Source.Intensity = Light->GetResolvedRules().Intensity;
		// Already in centimetres: the light converted it through LLKUnits
		// when it resolved, and converting again here would be the second
		// place spatial values are handled.
		Source.RadiusCm = Light->AttenuationRadius;
		Sources.Add(Source);
	}

	return Sources;
}

bool ULLKStormChargeSubsystem::GetBrightestLitPosition(FVector& OutPosition, float& OutBrightness) const
{
	return LLKLightField::FindBrightestPosition(GetLitLightSources(), OutPosition, OutBrightness);
}

float ULLKStormChargeSubsystem::GetCurrentRate() const
{
	// **Flat, and that is the decision.** Row 72: the pressure clock is the
	// night arriving and nothing a player does changes its rate. There used to
	// be a lantern multiplier here, on a diminishing curve, and it was the
	// single largest lever in the game - a dark crew's night ran nearly five
	// times longer than a brightly lit one's.
	//
	// It is gone because it could not be explained. Weather does not care
	// about your lamp, and a meter that said "the storm is arriving" while
	// your lantern accelerated it was two claims that could not both be true.
	// Light still costs - locally, through being noticed - which is what
	// LLKPerception and the light field are for.
	return Rules != nullptr ? Rules->BaselineRatePerSecond : 0.f;
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

	// Everything crossed, so the Warden is what is left to arrive - and as of
	// row 72 it is a threshold on the one clock rather than a second meter,
	// so the countdown reads from the same charge every other threshold does.
	if (!bWardenConverged)
	{
		OutId = TEXT("WardenConverges");
		OutTargetCharge = Rules->Warden.ChargeValue;
		OutCurrentCharge = Charge;
		return true;
	}

	// Nothing left to arrive. The one state a readout must not render as a
	// countdown to zero.
	return false;
}

float ULLKStormChargeSubsystem::SecondsUntilCharge(float FromCharge, float TargetCharge) const
{
	if (Rules == nullptr)
	{
		return TNumericLimits<float>::Max();
	}

	const float Rate = Rules->BaselineRatePerSecond;
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

float ULLKStormChargeSubsystem::SecondsUntilWarden() const
{
	// **Derived from the one clock, not accumulated beside it.** Row 72 says
	// one pressure clock per run, never two, and this is where that bites:
	// WardenCharge used to be a second float advancing at its own light-
	// sensitive rate. With the rate flat, a second meter would have advanced
	// at exactly the same speed as the first - two copies of one fact, and a
	// thing that can disagree with itself across a network for no benefit.
	//
	// So the Warden's arrival is a threshold on the clock like any other. Row
	// 54 still holds: its arrival is its own event with its own value, and
	// nothing about it is perception.
	return Rules != nullptr
		? SecondsUntilCharge(Charge, Rules->Warden.ChargeValue)
		: TNumericLimits<float>::Max();
}

FLLKStormSnapshot ULLKStormChargeSubsystem::BuildSnapshot() const
{
	FLLKStormSnapshot Snapshot;
	Snapshot.bPublished = true;
	Snapshot.Charge = Charge;
	Snapshot.ElapsedSeconds = ElapsedSeconds;
	// A crew is at most four; uint8 is not a micro-optimisation so much as a
	// statement that a count which needed more would be a bug.
	Snapshot.LitLanternCount = static_cast<uint8>(FMath::Clamp(GetLitLanternCount(), 0, 255));
	Snapshot.bWardenConverged = bWardenConverged;
	Snapshot.RulesSource = RulesSource;

	FName NextId;
	float Target = 0.f;
	float From = 0.f;
	Snapshot.bHasNextEscalation = GetNextEscalation(NextId, Target, From);
	if (Snapshot.bHasNextEscalation)
	{
		Snapshot.NextEscalationId = NextId;
		Snapshot.NextTargetCharge = Target;
		// Whichever meter GetNextEscalation measured against - the thresholds'
		// or the Warden's. Carried rather than re-derived on the client.
		Snapshot.NextFromCharge = From;
	}

	return Snapshot;
}

float ULLKStormChargeSubsystem::SecondsUntilSnapshotEscalation(
	const FLLKStormSnapshot& Snapshot) const
{
	if (!Snapshot.bHasNextEscalation)
	{
		return TNumericLimits<float>::Max();
	}

	// One branch, where there used to be two. The Warden's countdown was
	// measured against its own dampened meter; with one clock it is measured
	// against the clock, like everything else. Which also means a client and
	// a server can no longer disagree about the Warden by the width of a
	// multiplier neither of them replicated.
	return SecondsUntilCharge(Snapshot.NextFromCharge, Snapshot.NextTargetCharge);
}

void ULLKStormChargeSubsystem::PublishSnapshot()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	ALLKGameState* GameState = World->GetGameState<ALLKGameState>();
	if (GameState == nullptr)
	{
		if (!bWarnedAboutMissingGameState)
		{
			bWarnedAboutMissingGameState = true;
			// Once, loudly. A GameMode that forgets this class produces a
			// client HUD showing nothing, which looks exactly like a storm
			// that never advances - and that is a long evening.
			UE_LOG(LogLLK, Warning,
				TEXT("ULLKStormChargeSubsystem: no ALLKGameState in this world, so the storm will not ")
				TEXT("replicate to clients. Set GameStateClass to ALLKGameState or a subclass."));
		}
		return;
	}

	GameState->PublishStormSnapshot(BuildSnapshot());
}

void ULLKStormChargeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasStormAuthority())
	{
		return;
	}

	Advance(DeltaTime);

	// Throttled. The meter moves at most about four units a second and the
	// readout renders whole seconds, so sixty packets a second would change
	// the displayed number six times. A client's projection is computed from
	// static rules between snapshots, so it stays smooth either way.
	SecondsSinceSnapshot += DeltaTime;
	if (SecondsSinceSnapshot >= SnapshotIntervalSeconds)
	{
		SecondsSinceSnapshot = 0.f;
		PublishSnapshot();
	}
}

void ULLKStormChargeSubsystem::Advance(float DeltaSeconds)
{
	if (Rules == nullptr || DeltaSeconds <= 0.f)
	{
		return;
	}

	ElapsedSeconds += DeltaSeconds;
	Charge += Rules->BaselineRatePerSecond * DeltaSeconds;

	// Pulses expire on the same tick that advances the clock, so a pulse
	// cannot outlive the run or be left lit by a missed update.
	ExpireDistressPulses(DeltaSeconds);

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

	if (!bWardenConverged && Charge >= Rules->Warden.ChargeValue)
	{
		bWardenConverged = true;
		UE_LOG(LogLLK, Log, TEXT("The Warden converges at %.0fs elapsed (pressure %.0f)."),
			ElapsedSeconds, Charge);
		OnWardenConverged.Broadcast();
	}
}

FLLKNightStage ULLKStormChargeSubsystem::GetNightStage() const
{
	return StageFrom(Rules, Charge, bWardenConverged);
}

FLLKNightStage ULLKStormChargeSubsystem::StageFrom(
	const ULLKStormChargeData* InRules, float InCharge, bool bInWardenConverged)
{
	FLLKNightStage Stage;
	Stage.bWardenConverged = bInWardenConverged;

	if (InRules == nullptr)
	{
		// Calm, with a Count of zero. Deliberately not a guess at two
		// thresholds: a readout can tell "no rules" from "calm night" by the
		// count, and an enemy asking IsAtLeast gets the safe answer either
		// way.
		return Stage;
	}

	Stage.Count = static_cast<uint8>(FMath::Min(InRules->Thresholds.Num(), 255));

	// Counted against the authored values rather than against the announced
	// set, which is what makes this runnable on a client that has no
	// CrossedThresholds of its own. The two cannot disagree: a threshold is
	// announced exactly when the charge passes it, and row 6 means the charge
	// never comes back down to un-pass it.
	//
	// Highest-valued threshold wins the Id rather than last-in-array, because
	// nothing orders `Thresholds` and a data file sorted descending would
	// otherwise name the wrong stage while counting the right number of them.
	float HighestCrossedValue = -1.f;
	for (const FLLKStormThreshold& Threshold : InRules->Thresholds)
	{
		if (InCharge < Threshold.Value)
		{
			continue;
		}

		++Stage.Index;
		if (Threshold.Value > HighestCrossedValue)
		{
			HighestCrossedValue = Threshold.Value;
			Stage.Id = Threshold.Id;
		}
	}

	return Stage;
}

FString ULLKStormChargeSubsystem::GetThresholdEffect(FName ThresholdId) const
{
	if (Rules == nullptr || ThresholdId.IsNone())
	{
		return FString();
	}

	for (const FLLKStormThreshold& Threshold : Rules->Thresholds)
	{
		if (Threshold.Id == ThresholdId)
		{
			return Threshold.Effect;
		}
	}

	// Unknown id. Empty rather than invented - a badge with made-up text
	// would read as authored consequence.
	return FString();
}

// **The spike machinery is gone, as of row 73.** There used to be
// `GetSpikeAmount`, `AddSpike`, `ApplySpike` and `AddSpikeAmountForTest` here,
// plus an `ELLKStormSpike` allowlist of causes that a crew had chosen and been
// priced for. Chris closed the last of them on 2026-10-29: *"Clock means
// weather alone. Remove extractionCall.stormChargeSpike too. Calling
// extraction already has the right price: siren, visible activity, and patrol
// convergence. A radio call cannot accelerate the weather."*
//
// So `Advance` is now the only thing in the project that writes `Charge`, and
// `Advance` takes nothing but a delta. Row 6 - the meter only ever rises -
// stops needing to be enforced against a negative spike, because there is no
// longer an expression anywhere that adds to the meter at all. Row 64 line
// three stops needing an allowlist, because there is nothing to allow.
//
// `tests/escalation.test.mjs` guards the absence rather than the allowlist:
// the strong form of "escalation never comes from failing" is that escalation
// comes from nothing a player does.
