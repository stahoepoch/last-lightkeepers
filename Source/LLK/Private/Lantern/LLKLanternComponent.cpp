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
	DOREPLIFETIME(ULLKLanternComponent, bHasLantern);
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

void ULLKLanternComponent::GrantLantern()
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || bHasLantern)
	{
		return;
	}

	bHasLantern = true;
	UE_LOG(LogLLK, Log, TEXT("Lantern taken: the keeper now has one."));
	OnLanternPossessionChanged.Broadcast(true);
}

void ULLKLanternComponent::RevokeLantern()
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || !bHasLantern)
	{
		return;
	}

	// Off BEFORE the possession flag drops. The other order would leave
	// bLanternOn true on a keeper with no lantern, and the storm counts lit
	// lanterns rather than asking who is holding them - so a light nobody
	// holds would go on pressuring the room forever.
	ApplyState(false, ELLKLanternChangeReason::NotPossessed);

	bHasLantern = false;
	UE_LOG(LogLLK, Log, TEXT("Lantern lost: the keeper no longer has one."));
	OnLanternPossessionChanged.Broadcast(false);
}

void ULLKLanternComponent::OnRep_HasLantern()
{
	OnLanternPossessionChanged.Broadcast(bHasLantern);
}

void ULLKLanternComponent::ApplyState(bool bNewOn, ELLKLanternChangeReason Reason)
{
	// **Possession gates the one place the lantern can turn on.** Nothing
	// downstream checks it, and nothing downstream needs to: the storm's lit
	// count, the enemies' light field, relic legibility and the telemetry
	// all ask IsLanternOn(), and this makes "on without a lantern" an
	// unreachable state rather than a case each of them has to remember.
	//
	// Logged rather than silent. A keeper pressing the key and getting
	// nothing is exactly the shape of the 2026-10-29 evening spent on a
	// lantern that was toggling fine and was too dim to see - the fix for
	// that was to make every state change say so, and a refusal is a state
	// change that did not happen.
	if (bNewOn && !bHasLantern)
	{
		UE_LOG(LogLLK, Log,
			TEXT("Lantern stayed off: the keeper does not have one."));
		return;
	}

	if (bLanternOn == bNewOn)
	{
		// Idempotent on purpose. A toggle that fires its delegate on every
		// call would make listeners count state changes that did not happen,
		// and the zone-pressure readout is a count of lit lanterns.
		return;
	}

	bLanternOn = bNewOn;
	LastReason = Reason;

	// **Logged because the absence of this cost an evening.** 2026-10-29:
	// Chris reported the lantern not toggling. It was toggling; the light was
	// simply too dim to see, and there was no way to tell those two apart
	// from outside. A state change that nothing records is a state change
	// nobody can debug, and this is the one piece of state the storm, the
	// enemies and the loot rules all read.
	UE_LOG(LogLLK, Log, TEXT("Lantern %s (%s)."),
		bLanternOn ? TEXT("ON") : TEXT("OFF"),
		*UEnum::GetValueAsString(Reason));

	OnLanternStateChanged.Broadcast(bLanternOn, Reason);
}

void ULLKLanternComponent::OnRep_LanternOn()
{
	// The client did not decide this and should not claim it did. Listeners
	// that care about *why* - a HUD explaining a forced-off lantern, say -
	// get the truth rather than a plausible guess.
	OnLanternStateChanged.Broadcast(bLanternOn, ELLKLanternChangeReason::Replicated);
}
