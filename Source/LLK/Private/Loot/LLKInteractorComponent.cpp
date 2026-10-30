#include "Loot/LLKInteractorComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "Characters/LLKVitalsComponent.h"
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

	// A body outranks loot. Resolved first so the prompt never flickers
	// between a fallen crewmate and a brooch on the shelf behind them.
	FocusedCasualty = FindCasualty();

	ULLKRelicComponent* Found = FocusedCasualty != nullptr ? nullptr : FindFocus();
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

ULLKVitalsComponent* ULLKInteractorComponent::FindCasualty() const
{
	const APawn* Keeper = Cast<APawn>(GetOwner());
	const UWorld* World = GetWorld();
	if (Keeper == nullptr || World == nullptr)
	{
		return nullptr;
	}

	// A keeper on the floor cannot rescue anybody, including a crewmate who
	// fell beside them.
	if (const ULLKVitalsComponent* Own = Keeper->FindComponentByClass<ULLKVitalsComponent>())
	{
		if (Own->IsDowned())
		{
			return nullptr;
		}
	}

	const UCameraComponent* Camera = Keeper->FindComponentByClass<UCameraComponent>();
	const FVector EyeLocation = Camera != nullptr
		? Camera->GetComponentLocation() : Keeper->GetActorLocation();
	const FVector ViewDirection = Camera != nullptr
		? Camera->GetForwardVector() : Keeper->GetActorForwardVector();

	// Reach is the revive rule own distance, not the loot reach: the data says
	// 12 studs for a rescue and 9 for a shelf, and they are different numbers
	// for different reasons.
	ULLKVitalsComponent* Best = nullptr;
	float BestScore = -1.f;

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		if (*It == Keeper)
		{
			continue;
		}

		ULLKVitalsComponent* Vitals = It->FindComponentByClass<ULLKVitalsComponent>();
		if (Vitals == nullptr || !Vitals->IsDowned())
		{
			continue;
		}

		const float MaxDistance = LLKUnits::DistanceFromStuds(Vitals->GetMaxReviveDistanceStuds());
		const FVector ToCasualty = It->GetActorLocation() - EyeLocation;
		if (ToCasualty.SizeSquared() > MaxDistance * MaxDistance)
		{
			continue;
		}

		const float Aim = FVector::DotProduct(ToCasualty.GetSafeNormal(), ViewDirection);
		if (Aim < MinAimDot)
		{
			continue;
		}

		if (Aim > BestScore)
		{
			BestScore = Aim;
			Best = Vitals;
		}
	}

	return Best;
}

void ULLKInteractorComponent::BeginInteract()
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	// A downed keeper only available action is their own anchor revive.
	if (const ULLKVitalsComponent* Own = Owner->FindComponentByClass<ULLKVitalsComponent>())
	{
		if (Own->IsDowned())
		{
			ServerAnchorRevive();
			return;
		}
	}

	if (FocusedCasualty != nullptr)
	{
		Holding = FocusedCasualty;
		ServerBeginRevive(FocusedCasualty);
		return;
	}

	TryTakeFocused();
}

void ULLKInteractorComponent::EndInteract()
{
	if (Holding != nullptr)
	{
		ServerCancelRevive(Holding);
		Holding = nullptr;
	}
}

void ULLKInteractorComponent::ServerBeginRevive_Implementation(ULLKVitalsComponent* Target)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || Target == nullptr)
	{
		return;
	}

	// Re-checked here because the client asked based on its own view. The
	// per-tick range validation in TickRevive is what enforces the
	// commitment; this only refuses an opening request that was never valid.
	if (const ULLKVitalsComponent* Own = Owner->FindComponentByClass<ULLKVitalsComponent>())
	{
		if (Own->IsDowned())
		{
			return;
		}
	}

	Target->BeginRevive(Owner);
}

void ULLKInteractorComponent::ServerCancelRevive_Implementation(ULLKVitalsComponent* Target)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || Target == nullptr)
	{
		return;
	}

	// Only the keeper actually holding may cancel, or a third party could
	// interrupt somebody else rescue by tapping E.
	if (Target->GetRescuer() == Owner)
	{
		Target->CancelRevive(ELLKReviveCancelReason::Released);
	}
}

void ULLKInteractorComponent::ServerAnchorRevive_Implementation()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	ULLKVitalsComponent* Own = Owner->FindComponentByClass<ULLKVitalsComponent>();
	if (Own == nullptr || !Own->TryAnchorRevive())
	{
		return;
	}

	// **The price, re-priced by row 72.** It used to be
	// `AddSpike(AnchorRevive)` - a permanent 120 on the global meter - and
	// that is now forbidden: the clock is the night arriving and a keeper
	// getting up cannot make the weather worse. Chris's replacement, 2026-10-29:
	// *"it consumes the one emergency Anchor for that run and sends a local
	// distress pulse - bright, audible, and attractive to nearby machines -
	// but does not alter the global clock. A solo player gets back up, but
	// wakes up the room."*
	//
	// So the price is paid in the currency the game already has. The pulse is
	// a light source in the field for as long as the revive took, which makes
	// the keeper almost certainly the brightest thing in the building - and
	// the brightest thing is what the Warden converges on (row 7, row 54) and
	// what `LLKPerception` notices from 38 studs rather than 12. No new
	// number was authored for this: the brightness and reach are the keeper's
	// own lantern's, and the duration is the revive's own.
	if (const UWorld* World = GetWorld())
	{
		if (ULLKZonePressureSubsystem* Storm = World->GetSubsystem<ULLKZonePressureSubsystem>())
		{
			float Intensity = 0.f;
			float RadiusCm = 0.f;
			if (const ULLKLanternLightComponent* Light =
					Owner->FindComponentByClass<ULLKLanternLightComponent>())
			{
				Intensity = Light->GetResolvedRules().Intensity;
				RadiusCm = Light->AttenuationRadius;
			}

			// Deliberately not defaulted when the keeper has no light
			// component. AddDistressPulse refuses a pulse with no reach and
			// says so, which is the same rule the light field follows about a
			// lantern with no radius: an entry that contributes nothing must
			// not look like one that does.
			Storm->AddDistressPulse(
				Owner->GetActorLocation(), Intensity, RadiusCm, Own->GetAnchorReviveSeconds());
		}
	}
}

void ULLKInteractorComponent::ClearCarriedOnWipe()
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	CarriedCount = 0;
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
