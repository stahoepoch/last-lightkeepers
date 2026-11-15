#include "Lantern/LLKLanternPickup.h"

#include "Characters/LLKCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "LLK.h"
#include "Lantern/LLKLanternComponent.h"
#include "Net/UnrealNetwork.h"

ALLKLanternPickup::ALLKLanternPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);

	// No collision response of its own. Whatever route ends up taking this
	// lantern - a prompt, a trigger, a scripted beat - will do its own
	// query, and a pickup that also blocks the player is a pickup you can
	// get stuck on in a doorway.
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALLKLanternPickup::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALLKLanternPickup, bTaken);
}

bool ALLKLanternPickup::GrantTo(ALLKCharacter* Keeper)
{
	if (!HasAuthority())
	{
		// Deliberately not routed to the server from here. Who may take the
		// lantern is a decision for whatever calls this, and a pickup that
		// silently forwards a client's request is a pickup any client can
		// fire. The caller has authority or it does not.
		UE_LOG(LogLLK, Warning,
			TEXT("LanternPickup: GrantTo called without authority; ignored."));
		return false;
	}

	if (bTaken)
	{
		return false;
	}

	if (Keeper == nullptr)
	{
		UE_LOG(LogLLK, Warning, TEXT("LanternPickup: no keeper to grant to."));
		return false;
	}

	ULLKLanternComponent* Lantern = Keeper->FindComponentByClass<ULLKLanternComponent>();
	if (Lantern == nullptr)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("LanternPickup: %s has no lantern component, so there is "
				 "nothing to give it to."), *Keeper->GetName());
		return false;
	}

	if (Lantern->HasLantern())
	{
		// One instrument. Not an error, and not a success either - this is
		// the case the return value exists for.
		return false;
	}

	Lantern->GrantLantern();

	bTaken = true;
	OnRep_Taken();

	UE_LOG(LogLLK, Log, TEXT("LanternPickup: %s took the lantern."),
		*Keeper->GetName());
	return true;
}

void ALLKLanternPickup::OnRep_Taken()
{
	// Hidden rather than destroyed. The opening may want the lantern to
	// stop being on the floor without the actor leaving the level - a beat
	// that returns to the same spot, a shot that frames where it was - and
	// an actor that destroyed itself cannot do either. Whoever authors the
	// opening can destroy it if they would rather.
	if (Body != nullptr)
	{
		Body->SetVisibility(!bTaken);
	}
}
