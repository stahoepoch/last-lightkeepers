#include "Loot/LLKInteractorComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "Loot/LLKRelicComponent.h"
#include "Net/UnrealNetwork.h"

ULLKInteractorComponent::ULLKInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void ULLKInteractorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Everyone, not just the owner. A crew argues about who is carrying what,
	// and row 59 makes an extracted crate pay every keeper who extracts - so
	// what each other is holding is crew information, not private.
	DOREPLIFETIME(ULLKInteractorComponent, CarriedCount);
}

void ULLKInteractorComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SecondsSinceSearch += DeltaTime;
	if (SecondsSinceSearch < SearchIntervalSeconds)
	{
		return;
	}
	SecondsSinceSearch = 0.f;

	ULLKRelicComponent* Found = FindFocus();
	if (Found == Focused)
	{
		return;
	}

	Focused = Found;
	// Broadcast on change only. A prompt rebuilt twelve times a second is a
	// prompt that flickers, which is the same lesson the affordance channel
	// learned an hour ago.
	OnFocusChanged.Broadcast(Focused);
}

ULLKRelicComponent* ULLKInteractorComponent::FindFocus() const
{
	const APawn* Keeper = Cast<APawn>(GetOwner());
	const UWorld* World = GetWorld();
	if (Keeper == nullptr || World == nullptr)
	{
		return nullptr;
	}

	// From the eye and along the view, not from the capsule and along the
	// body. Row 44 makes the camera the keeper's head, so where they are
	// looking is the only reading of "what do they mean" that matches what
	// they can see.
	const UCameraComponent* Camera = Keeper->FindComponentByClass<UCameraComponent>();
	const FVector EyeLocation = Camera != nullptr ? Camera->GetComponentLocation() : Keeper->GetActorLocation();
	const FVector ViewDirection = Camera != nullptr
		? Camera->GetForwardVector() : Keeper->GetActorForwardVector();

	const float Reach = LLKUnits::DistanceFromStuds(ReachStuds);
	const float ReachSquared = Reach * Reach;

	ULLKRelicComponent* Best = nullptr;
	float BestScore = -1.f;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		ULLKRelicComponent* Relic = It->FindComponentByClass<ULLKRelicComponent>();
		if (Relic == nullptr)
		{
			continue;
		}

		const FVector ToRelic = It->GetActorLocation() - EyeLocation;
		if (ToRelic.SizeSquared() > ReachSquared)
		{
			continue;
		}

		// Aim as well as reach. Nearest-alone picks the relic behind your
		// shoulder when you are stood between two shelves, and then the
		// prompt describes one thing while E takes another.
		const FVector Direction = ToRelic.GetSafeNormal();
		const float Aim = FVector::DotProduct(Direction, ViewDirection);
		if (Aim < MinAimDot)
		{
			continue;
		}

		// Most directly looked-at wins, rather than nearest. Between a relic
		// at arm's length off to the side and one dead ahead a little
		// further, the one you are looking at is the one you mean.
		if (Aim > BestScore)
		{
			BestScore = Aim;
			Best = Relic;
		}
	}

	return Best;
}

void ULLKInteractorComponent::TryTakeFocused()
{
	if (Focused == nullptr)
	{
		return;
	}

	// Asked, never done. The client has already been told by the prompt
	// whether this will work, but the prompt is a courtesy and the server is
	// the authority - row 62's light gate on taking is only a rule if the
	// machine that owns the shelf enforces it.
	ServerTake(Focused);
}

void ULLKInteractorComponent::ServerTake_Implementation(ULLKRelicComponent* Relic)
{
	if (Relic == nullptr)
	{
		return;
	}

	const AActor* Keeper = GetOwner();
	if (Keeper == nullptr || !Keeper->HasAuthority())
	{
		return;
	}

	// Re-checked here rather than trusted from the call. The client decided
	// to ask based on what its own prompt said; whether it may is this
	// machine's question, and the answer includes the live-light gate.
	if (!Relic->CanBeTaken())
	{
		UE_LOG(LogLLK, Verbose,
			TEXT("ULLKInteractorComponent: refused a take - %s"),
			Relic->IsLegible() ? TEXT("not lit") : TEXT("not identified"));
		return;
	}

	// Range re-checked too, against the server's own view of where the keeper
	// is. Without it a client could ask for a relic on another floor.
	const float Reach = LLKUnits::DistanceFromStuds(ReachStuds) * 1.5f;
	const AActor* RelicActor = Relic->GetOwner();
	if (RelicActor == nullptr
		|| FVector::DistSquared(Keeper->GetActorLocation(), RelicActor->GetActorLocation()) > Reach * Reach)
	{
		return;
	}

	if (!Relic->TakeByServer())
	{
		return;
	}

	++CarriedCount;

	// Burden, carry slots and the haul are `loot.json`'s rules and a later
	// row; this is a count, and it says so rather than pretending to be an
	// inventory.
	UE_LOG(LogLLK, Log, TEXT("Took %s. Carrying %d."),
		*Relic->GetDisplayName().ToString(), CarriedCount);

	if (Focused == Relic)
	{
		Focused = nullptr;
		OnFocusChanged.Broadcast(nullptr);
	}
}
