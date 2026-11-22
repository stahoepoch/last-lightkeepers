#include "World/LLKBreakerPanel.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Lantern/LLKLanternFeel.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "Net/UnrealNetwork.h"
#include "World/LLKPowerSubsystem.h"

ALLKBreakerPanel::ALLKBreakerPanel()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Body->SetCollisionObjectType(ECC_WorldStatic);
}

void ALLKBreakerPanel::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALLKBreakerPanel, bThrown);
}

void ALLKBreakerPanel::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogLLK, Display,
		TEXT("%s is a breaker the keeper can reach: handle at %.0f cm, "
			 "%.0f cm of reach, and throwing it is heard %.0f studs away."),
		*GetName(), LLKUnits::DistanceFromStuds(HandleHeightStuds), ReachCm,
		ULLKPowerSubsystem::PowerOnNoiseStuds);
}

FVector ALLKBreakerPanel::GetHandlePoint() const
{
	// World up, not the actor's own: a panel rotated to sit flat on a service
	// corridor wall must still present its handle at the authored height.
	return GetActorLocation()
		+ FVector(0.f, 0.f, LLKUnits::DistanceFromStuds(HandleHeightStuds));
}

bool ALLKBreakerPanel::CanFocus(const APawn* Keeper) const
{
	// A thrown breaker is not interactable. It stops prompting rather than
	// prompting and refusing, because a prompt that does nothing teaches a
	// player that prompts are unreliable.
	if (bThrown || Keeper == nullptr || Keeper->GetWorld() != GetWorld())
	{
		return false;
	}

	const FVector Eye = LLKLanternFeel::KeeperEyeWorldPosition(Keeper->GetActorLocation());
	const FVector ToHandle = GetHandlePoint() - Eye;
	const float Distance = ToHandle.Size();
	if (Distance < KINDA_SMALL_NUMBER || Distance > ReachCm)
	{
		return false;
	}

	return FVector::DotProduct(
		Keeper->GetBaseAimRotation().Vector(), ToHandle / Distance) >= MinAimDot;
}

FText ALLKBreakerPanel::FocusVerb() const
{
	return NSLOCTEXT("LLK", "ThrowBreaker", "Throw the breaker");
}

void ALLKBreakerPanel::BeginInteract(APawn* Keeper)
{
	if (!HasAuthority() || bThrown)
	{
		return;
	}

	UWorld* World = GetWorld();
	ULLKPowerSubsystem* Power =
		World != nullptr ? World->GetSubsystem<ULLKPowerSubsystem>() : nullptr;
	if (Power == nullptr)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("%s was thrown and there is no power subsystem to tell, so "
				 "nothing happened. The breaker is a prop."), *GetName());
		return;
	}

	bThrown = true;
	NoiseRemaining = NoiseSeconds;
	const bool bChanged = Power->SetPowerOn(true);

	UE_LOG(LogLLK, Log,
		TEXT("%s thrown by %s. Mains %s, and the building is %.0f studs loud "
			 "for the next %.0f s."),
		*GetName(), Keeper != nullptr ? *Keeper->GetName() : TEXT("nobody"),
		bChanged ? TEXT("came on") : TEXT("were already on"),
		ULLKPowerSubsystem::PowerOnNoiseStuds, NoiseSeconds);
}

void ALLKBreakerPanel::EndInteract(APawn* Keeper)
{
	// Nothing to cancel. Throwing a breaker is not a hold, and the release
	// arrives unconditionally, so this is deliberately empty rather than
	// absent - the interface requires an answer and "nothing happens" is one.
}

void ALLKBreakerPanel::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && NoiseRemaining > 0.f)
	{
		NoiseRemaining = FMath::Max(0.f, NoiseRemaining - DeltaSeconds);
	}
}

bool ALLKBreakerPanel::GetCurrentNoise(LLKHearing::FLLKNoise& OutNoise) const
{
	if (NoiseRemaining <= 0.f)
	{
		return false;
	}

	OutNoise.Position = GetActorLocation();
	OutNoise.RadiusStuds = ULLKPowerSubsystem::PowerOnNoiseStuds;
	return true;
}
