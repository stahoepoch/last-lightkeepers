#include "Characters/LLKVitalsComponent.h"

#include "LLK.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Lantern/LLKLanternComponent.h"
#include "LLKUnits.h"
#include "Net/UnrealNetwork.h"

ULLKVitalsComponent::ULLKVitalsComponent()
{
	// Ticks only to count seconds on the floor, which a revive prompt and the
	// HUD both read. Cheap, and it stops when nobody is down.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void ULLKVitalsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Everyone. A crew has to be able to see who is on the floor without being
	// told - row 59 makes protecting the carrier the job, and you cannot
	// protect somebody whose state only they can see.
	DOREPLIFETIME(ULLKVitalsComponent, bDowned);

	// The rescue is crew information, not private. Row 59 makes protecting a
	// downed carrier the job, and a crew cannot coordinate a rescue none of
	// them can see the progress of.
	DOREPLIFETIME(ULLKVitalsComponent, ReviveSeconds);
	DOREPLIFETIME(ULLKVitalsComponent, Rescuer);
	DOREPLIFETIME(ULLKVitalsComponent, AnchorChargesLeft);
}

void ULLKVitalsComponent::BeginPlay()
{
	Super::BeginPlay();
	ResolveRules();
}

void ULLKVitalsComponent::ResolveRules()
{
	DownedWalkStuds = FallbackDownedWalkStuds;
	BleedOutSeconds = FallbackBleedOutSeconds;
	bLanternForcedOffWhenDowned = bFallbackLanternForcedOff;
	bDropsCarriedCrate = bFallbackDropsCarriedCrate;
	RulesSource = ELLKDataSource::Fallback;

	if (const ULLKLanternData* Data = LanternData.LoadSynchronous())
	{
		// **Taken as authored, with no zero check, and that is deliberate.**
		// The light block gets `DescribesARealLight` because a radius of zero
		// is not a tuning anybody would write, so zero there means the asset
		// predates the field. Here zero IS the design: a downed keeper cannot
		// crawl and has no bleed-out, both documented twice in lantern.json.
		// Validating these the same way would quietly replace Chris's
		// authored rules with a fallback that happens to match today.
		TeammateHoldSeconds = Data->Revive.TeammateHoldSeconds;
		MaxReviveDistanceStuds = Data->Revive.MaxReviveDistance;
		DownedWalkStuds = Data->Knockdown.DownedWalkSpeed;
		BleedOutSeconds = Data->Knockdown.BleedOutSeconds;
		bLanternForcedOffWhenDowned = Data->Knockdown.bLanternForcedOffWhenDowned;
		bDropsCarriedCrate = Data->Knockdown.bDropsCarriedCrate;
		RulesSource = ELLKDataSource::Asset;
	}

	if (RulesSource == ELLKDataSource::Fallback)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKVitalsComponent: no ULLKLanternData resolved - running on DECLARED FALLBACK ")
			TEXT("knockdown rules (downed walk %.0f studs, bleed-out %.0fs, lantern forced off %s, ")
			TEXT("drops crate %s)."),
			DownedWalkStuds, BleedOutSeconds,
			bLanternForcedOffWhenDowned ? TEXT("yes") : TEXT("no"),
			bDropsCarriedCrate ? TEXT("yes") : TEXT("no"));
	}

	// Row 59 depends on this pair and nothing else enforces it, so it is
	// stated where somebody changing the data will see it. With a walk speed
	// of zero and no drop, a crate is pinned to a downed body and cannot be
	// banked while its carrier is on the floor - which is what makes
	// "everyone protects the carrier" a rule rather than an encouragement.
	if (bDropsCarriedCrate)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKVitalsComponent: dropsCarriedCrate is TRUE, so a crew can abandon a downed ")
			TEXT("carrier and still extract the crate. DECISIONS.md row 59 depends on it being false."));
	}
}

void ULLKVitalsComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bDowned)
	{
		return;
	}

	TickDowned(DeltaTime);
	TickRevive(DeltaTime);
}

void ULLKVitalsComponent::TickDowned(float DeltaSeconds)
{
	SecondsDowned += DeltaSeconds;

	// **Armed and never fired.** `bleedOutSeconds` is 0 in the data, which
	// means a downed keeper waits indefinitely and the wipe comes from the
	// whole crew going down rather than from a clock. Implemented anyway so
	// the key means something if Chris ever sets it - the same shape as
	// `autoOffAtEmpty`, which is also real and also never triggers today.
	if (BleedOutSeconds <= 0.f)
	{
		return;
	}

	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	if (SecondsDowned >= BleedOutSeconds)
	{
		// Deliberately not implemented as death. Row 47 contains people; it
		// does not kill them. What a bleed-out would mean in this game is a
		// design question nobody has answered, so it logs and stops rather
		// than inventing an answer.
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKVitalsComponent: bleedOutSeconds of %.0f elapsed, and there is no behaviour ")
			TEXT("for it. The machines contain rather than kill (row 47), so what expiry MEANS is ")
			TEXT("undecided. Set bleedOutSeconds back to 0 or decide the rule."),
			BleedOutSeconds);
		BleedOutSeconds = 0.f;
	}
}

void ULLKVitalsComponent::GoDown(ELLKDownedReason Reason)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	ApplyDowned(true, Reason);
}

void ULLKVitalsComponent::StandUp(ELLKDownedReason Reason)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	ApplyDowned(false, Reason);
}

void ULLKVitalsComponent::ApplyDowned(bool bNewDowned, ELLKDownedReason Reason)
{
	if (bDowned == bNewDowned)
	{
		// Idempotent. A second machine reaching an already-downed keeper is
		// not an event: the HUD counts downed keepers and a revive prompt
		// reads the clock, and both would move for something that did not
		// happen.
		return;
	}

	bDowned = bNewDowned;
	SecondsDowned = 0.f;

	if (!bDowned)
	{
		// Standing up clears any hold, so a rescuer who kept holding through
		// somebody else's successful rescue does not bank progress toward
		// the next knockdown.
		ReviveSeconds = 0.f;
		Rescuer.Reset();
		AnchorSecondsHeld = 0.f;
	}

	if (bDowned && bLanternForcedOffWhenDowned)
	{
		// **The caller `ForceOff` has been waiting for since M1.2.** The
		// reason entry `ForcedOffWhenDowned` was written the same day and has
		// been unused ever since, which is how a seam becomes decoration.
		// It goes through ForceOff rather than SetLanternOn so the reason
		// survives into the delegate: a HUD explaining a lantern the keeper
		// did not switch off needs to know the difference.
		if (AActor* Owner = GetOwner())
		{
			if (ULLKLanternComponent* Lantern = Owner->FindComponentByClass<ULLKLanternComponent>())
			{
				Lantern->ForceOff(ELLKLanternChangeReason::ForcedOffWhenDowned);
			}
		}
	}

	UE_LOG(LogLLK, Log, TEXT("Keeper %s (%s)."),
		bDowned ? TEXT("DOWN") : TEXT("up"),
		*UEnum::GetValueAsString(Reason));

	OnDownedChanged.Broadcast(bDowned, Reason);
}

void ULLKVitalsComponent::BeginRevive(AActor* InRescuer)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || InRescuer == nullptr || !bDowned)
	{
		return;
	}

	if (Rescuer.Get() == InRescuer)
	{
		// Already holding. Idempotent, because the client re-sends on key
		// repeat and a restart would make the hold unfinishable.
		return;
	}

	Rescuer = InRescuer;
	ReviveSeconds = KINDA_SMALL_NUMBER;
}

void ULLKVitalsComponent::CancelRevive(ELLKReviveCancelReason Reason)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	if (ReviveSeconds <= 0.f)
	{
		return;
	}

	ReviveSeconds = 0.f;
	Rescuer.Reset();
	OnReviveCancelled.Broadcast(Reason);
}

void ULLKVitalsComponent::TickRevive(float DeltaSeconds)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	// ---- the solo keeper's self-rescue, which runs on its own clock -------
	if (AnchorSecondsHeld > 0.f)
	{
		AnchorSecondsHeld += DeltaSeconds;
		if (AnchorSecondsHeld >= AnchorReviveSeconds)
		{
			AnchorSecondsHeld = 0.f;
			StandUp(ELLKDownedReason::Revived);
		}
		return;
	}

	if (ReviveSeconds <= 0.f)
	{
		return;
	}

	const AActor* Holder = Rescuer.Get();
	if (Holder == nullptr)
	{
		CancelRevive(ELLKReviveCancelReason::RescuerDowned);
		return;
	}

	// **Validated against the rescuer's real position every tick, not once at
	// the start.** `revive.maxReviveDistance` only means something if walking
	// away actually stops it - that is the commitment row 59 asks for, and
	// checking only on the first frame would make the rule decorative.
	const float MaxDistance = LLKUnits::DistanceFromStuds(MaxReviveDistanceStuds);
	if (FVector::DistSquared(Owner->GetActorLocation(), Holder->GetActorLocation())
		> MaxDistance * MaxDistance)
	{
		CancelRevive(ELLKReviveCancelReason::OutOfRange);
		return;
	}

	// A rescuer who goes down themselves stops rescuing. The co-op crisis
	// compounding, which is the decision Chris's fourth pillar is about.
	if (const ULLKVitalsComponent* HolderVitals = Holder->FindComponentByClass<ULLKVitalsComponent>())
	{
		if (HolderVitals->IsDowned())
		{
			CancelRevive(ELLKReviveCancelReason::RescuerDowned);
			return;
		}
	}

	ReviveSeconds += DeltaSeconds;
	if (ReviveSeconds >= TeammateHoldSeconds)
	{
		ReviveSeconds = 0.f;
		Rescuer.Reset();
		StandUp(ELLKDownedReason::Revived);
	}
}

float ULLKVitalsComponent::GetReviveProgress() const
{
	if (AnchorSecondsHeld > 0.f && AnchorReviveSeconds > 0.f)
	{
		return FMath::Clamp(AnchorSecondsHeld / AnchorReviveSeconds, 0.f, 1.f);
	}

	if (ReviveSeconds <= 0.f || TeammateHoldSeconds <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(ReviveSeconds / TeammateHoldSeconds, 0.f, 1.f);
}

bool ULLKVitalsComponent::IsSoloRun() const
{
	if (bHasForcedSolo)
	{
		return bForcedSoloForTest;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World != nullptr ? World->GetGameState() : nullptr;
	if (GameState == nullptr)
	{
		// Unknown rather than assumed solo. Row 9 grants the anchor revive to
		// a lone keeper; granting it because we could not tell would hand a
		// crew a free rescue, which breaks the other half of the same rule.
		return false;
	}

	return GameState->PlayerArray.Num() <= 1;
}

bool ULLKVitalsComponent::IsAnchorReviveAvailable() const
{
	if (!bDowned || AnchorChargesLeft <= 0)
	{
		return false;
	}

	// Row 9, LOCKED: solo only. "Granting it to a crew breaks the other half
	// of the same rule - a whole crew down must wipe rather than wait
	// indefinitely for someone to press R."
	return !bAnchorSoloOnly || IsSoloRun();
}

bool ULLKVitalsComponent::TryAnchorRevive()
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return false;
	}

	if (!IsAnchorReviveAvailable() || AnchorSecondsHeld > 0.f)
	{
		return false;
	}

	--AnchorChargesLeft;
	AnchorSecondsHeld = KINDA_SMALL_NUMBER;

	// A teammate hold in progress is abandoned - you cannot be rescued and
	// rescue yourself at once, and the charge is already spent.
	CancelRevive(ELLKReviveCancelReason::NoLongerNeeded);

	// **This component does NOT pay the price, and that is the guard's doing.**
	// My first version spiked the meter from here, and
	// `tests/escalation.test.mjs` failed - correctly. The typed enum stops
	// anybody NAMING failure as a cause, but the file check stops the
	// knockdown path from spiking at all, including by misusing a legitimate
	// cause: `OnDowned -> AddSpike(ExtractionCall)` would satisfy the enum
	// and still be exactly the rule being broken.
	//
	// Which forced a better split. Spending the anchor revive is a player
	// ACTION with a price, not a vital state, so the chooser pays: this
	// returns true and `ULLKInteractorComponent` applies the spike. Vitals
	// never mentions the storm, and the guard keeps its teeth.
	UE_LOG(LogLLK, Log, TEXT("Anchor revive spent. %d left."), AnchorChargesLeft);
	return true;
}

void ULLKVitalsComponent::OnRep_Downed()
{
	SecondsDowned = 0.f;

	// The client did not decide this. The lantern is not forced off here
	// either - the server did that and it replicates on its own, so doing it
	// again locally would be a client deciding something about light.
	OnDownedChanged.Broadcast(bDowned, ELLKDownedReason::Replicated);
}
