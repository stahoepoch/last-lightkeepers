#include "Characters/LLKVitalsComponent.h"

#include "LLK.h"
#include "Lantern/LLKLanternComponent.h"
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

void ULLKVitalsComponent::OnRep_Downed()
{
	SecondsDowned = 0.f;

	// The client did not decide this. The lantern is not forced off here
	// either - the server did that and it replicates on its own, so doing it
	// again locally would be a client deciding something about light.
	OnDownedChanged.Broadcast(bDowned, ELLKDownedReason::Replicated);
}
