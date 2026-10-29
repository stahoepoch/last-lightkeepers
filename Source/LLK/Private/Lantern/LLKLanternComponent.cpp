#include "Lantern/LLKLanternComponent.h"

#include "LLK.h"
#include "Net/UnrealNetwork.h"

ULLKLanternComponent::ULLKLanternComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULLKLanternComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Everyone sees every lantern. This is not a convenience: row 5 makes
	// light reveal value and attract danger, so a crew has to be able to see
	// who is lit in order to argue about it.
	DOREPLIFETIME(ULLKLanternComponent, bLanternOn);
}

void ULLKLanternComponent::BeginPlay()
{
	Super::BeginPlay();
	ResolveRules();
}

void ULLKLanternComponent::ResolveRules()
{
	bForcedOffWhenDowned = bFallbackForcedOffWhenDowned;
	RulesSource = ELLKDataSource::Fallback;

	if (const ULLKLanternData* Data = LanternData.LoadSynchronous())
	{
		bForcedOffWhenDowned = Data->Knockdown.bLanternForcedOffWhenDowned;
		RulesSource = ELLKDataSource::Asset;
	}

	if (RulesSource == ELLKDataSource::Fallback)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKLanternComponent: no ULLKLanternData resolved - running on DECLARED FALLBACK rules ")
			TEXT("(forcedOffWhenDowned=%s)."), bForcedOffWhenDowned ? TEXT("true") : TEXT("false"));
	}
}

void ULLKLanternComponent::ToggleLantern()
{
	SetLanternOn(!bLanternOn);
}

void ULLKLanternComponent::SetLanternOn(bool bNewOn)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	// The client asks. It does not flip the bool and tell the server after,
	// which is the shape that lets a client be lit on its own screen and dark
	// on everyone else's - and this is the one piece of state the storm, the
	// enemies and the loot rules all read.
	if (!Owner->HasAuthority())
	{
		ServerSetLanternOn(bNewOn);
		return;
	}

	ApplyState(bNewOn, ELLKLanternChangeReason::PlayerToggled);
}

void ULLKLanternComponent::ServerSetLanternOn_Implementation(bool bNewOn)
{
	ApplyState(bNewOn, ELLKLanternChangeReason::PlayerToggled);
}

void ULLKLanternComponent::ForceOff(ELLKLanternChangeReason Reason)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	ApplyState(false, Reason);
}

void ULLKLanternComponent::ApplyState(bool bNewOn, ELLKLanternChangeReason Reason)
{
	if (bLanternOn == bNewOn)
	{
		// Idempotent on purpose. A toggle that fires its delegate on every
		// call would make listeners count state changes that did not happen,
		// and the storm charge multiplier is a count of lit lanterns.
		return;
	}

	bLanternOn = bNewOn;
	LastReason = Reason;
	OnLanternStateChanged.Broadcast(bLanternOn, Reason);
}

void ULLKLanternComponent::OnRep_LanternOn()
{
	// The client did not decide this and should not claim it did. Listeners
	// that care about *why* - a HUD explaining a forced-off lantern, say -
	// get the truth rather than a plausible guess.
	OnLanternStateChanged.Broadcast(bLanternOn, ELLKLanternChangeReason::Replicated);
}
