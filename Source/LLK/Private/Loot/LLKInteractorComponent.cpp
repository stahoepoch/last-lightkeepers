#include "Loot/LLKInteractorComponent.h"
#include "Lantern/LLKLanternFeel.h"
#include "Lantern/LLKLanternPickup.h"

#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "Characters/LLKCharacter.h"
#include "Characters/LLKVitalsComponent.h"
#include "Loot/LLKRelicComponent.h"
#include "Run/LLKExtractionSubsystem.h"
#include "Components/BoxComponent.h"
#include "Run/LLKExtractionZone.h"
#include "Net/UnrealNetwork.h"
#include "World/LLKOpeningGrille.h"

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
	DOREPLIFETIME_CONDITION(ULLKInteractorComponent, TakingRelic, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ULLKInteractorComponent, RelicTakeProgressSeconds, COND_OwnerOnly);
}

void ULLKInteractorComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// **Before the search interval, not after it.** Focus is resolved a few
	// times a second because a prompt does not need 60Hz, but a hold that only
	// advanced on those frames would take as long as the interval made it -
	// the hold is a duration the player feels, so it gets every frame.
	TickExtractionCall(DeltaTime);
	TickRelicTake(DeltaTime);

	SecondsSinceSearch += DeltaTime;
	if (SecondsSinceSearch < SearchIntervalSeconds)
	{
		return;
	}
	SecondsSinceSearch = 0.f;

	// A body outranks loot. Resolved first so the prompt never flickers
	// between a fallen crewmate and a brooch on the shelf behind them.
	FocusedCasualty = FindCasualty();

	// Lowest priority of the three, and resolved last for that reason: a
	// keeper standing in the bay reaching for a brooch on the dock floor takes
	// the brooch. Looking away from it is how they call.
	FocusedZone = FindZone();

	// Resolved on the same beat as the others so the prompt cannot describe
	// one frame's world while E acts on another's.
	FocusedLantern = ResolveLanternPickup();
	FocusedOpeningGrille = ResolveOpeningGrille();

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

	// The zone is checked after a casualty and after loot, and `TryTakeFocused`
	// is a no-op with nothing focused, so the order below reads as: a body,
	// then a thing you are looking at, then the bay you are standing in.
	// The lantern joins this gate rather than being bolted on after it. The
	// zone prompt is suppressed in exactly the cases where E would do
	// something else, and a fourth thing E can do is a fourth case - C13 in
	// HANDOFFS is explicit that the fix for a hidden prompt is never to
	// reorder these, so the new focus is added to the condition instead.
	if (FocusedCasualty == nullptr && Focused == nullptr
		&& FocusedLantern == nullptr && FocusedOpeningGrille == nullptr
		&& FocusedZone != nullptr)
	{
		ServerBeginExtractionCall(FocusedZone);
		return;
	}

	if (FocusedCasualty != nullptr)
	{
		Holding = FocusedCasualty;
		ServerBeginRevive(FocusedCasualty);
		return;
	}

	// Above loot, below a body. It is the one instrument and a gate on the
	// opening; a brooch can wait the half second it takes to look away.
	if (FocusedLantern != nullptr)
	{
		ServerTakeLantern(FocusedLantern);
		return;
	}

	if (FocusedOpeningGrille != nullptr)
	{
		ServerReleaseOpeningGrille(FocusedOpeningGrille);
		return;
	}

	TryTakeFocused();
}

ALLKOpeningGrille* ULLKInteractorComponent::ResolveOpeningGrille() const
{
	const APawn* Keeper = Cast<APawn>(GetOwner());
	if (Keeper == nullptr || GetWorld() == nullptr)
	{
		return nullptr;
	}
	for (TActorIterator<ALLKOpeningGrille> It(GetWorld()); It; ++It)
	{
		if (It->CanFocus(Keeper))
		{
			return *It;
		}
	}
	return nullptr;
}

void ULLKInteractorComponent::ServerReleaseOpeningGrille_Implementation(ALLKOpeningGrille* Candidate)
{
	// The same resolver runs on both sides; the client does not grant itself a door.
	if (Candidate != nullptr && ResolveOpeningGrille() == Candidate)
	{
		Candidate->TryRelease(Cast<APawn>(GetOwner()));
	}
}

ALLKLanternPickup* ULLKInteractorComponent::ResolveLanternPickup() const
{
	const AActor* Owner = GetOwner();
	const APawn* Pawn = Cast<APawn>(Owner);
	if (Pawn == nullptr)
	{
		return nullptr;
	}

	UWorld* World = Pawn->GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	// The project's one authority on where a keeper's eye is.
	//
	// This used to read `GameplayLightPosition(GetActorLocation(),
	// KeeperEyeHeightCm)`, copying the form the light component uses, and
	// that form double-counts the capsule half-height: the eye landed at
	// 197.7 cm for a 145 cm boy and the lantern, 9 cm off the tile, was
	// 189 cm away vertically against a 150 cm reach. E could not have
	// worked anywhere. Measured 2026-11-15, after the census printed the
	// pickup's real height.
	const FVector Eye =
		LLKLanternFeel::KeeperEyeWorldPosition(Pawn->GetActorLocation());

	// GetBaseAimRotation, not the actor's forward: the pawn turns with the
	// controller in yaw only, and a child reaching for something on the
	// floor is looking almost straight down. Aim including pitch is also
	// what the server has for a remote pawn.
	//
	// **Its pitch is quantised to a byte off the owning client**, about 1.4
	// degrees a step, so at the exact edge of the aim cone the server can
	// disagree with the client that asked. That fails CLOSED - the grant is
	// refused and the player presses again - which is the right direction
	// for a disagreement about whether someone was looking at something.
	const FVector Aim = Pawn->GetBaseAimRotation().Vector();

	ALLKLanternPickup* Best = nullptr;
	float BestDot = LanternMinAimDot;

	for (ALLKLanternPickup* Pickup : ALLKLanternPickup::GetLivePickups(World))
	{
		if (Pickup == nullptr || Pickup->IsTaken())
		{
			continue;
		}

		const FVector Point = Pickup->GetInteractionPoint();
		const FVector ToPickup = Point - Eye;
		const float Distance = ToPickup.Size();
		if (Distance > LanternReachCm || Distance <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const float Dot = FVector::DotProduct(Aim, ToPickup / Distance);
		if (Dot < BestDot)
		{
			continue;
		}

		// Clear line of sight. The body is NoCollision, so it cannot block
		// this itself - any blocking hit between the eye and the lantern is
		// something else in the way, which is exactly the question.
		FCollisionQueryParams Params(SCENE_QUERY_STAT(LLKLanternReach), false, Pawn);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Eye, Point, ECC_Visibility, Params))
		{
			continue;
		}

		Best = Pickup;
		BestDot = Dot;
	}

	return Best;
}

void ULLKInteractorComponent::ServerTakeLantern_Implementation(
	ALLKLanternPickup* Candidate)
{
	// **Recomputed, not trusted.** The client sends what it resolved; the
	// server resolves again from its own view of reach, aim and
	// obstruction, and grants only if the two name the same lantern. A
	// client that asks for a pickup across the room, through a wall, or
	// while facing away gets nothing, and so does one that asks for a
	// different lantern than the one it is being shown a prompt for.
	ALLKLanternPickup* Mine = ResolveLanternPickup();
	if (Mine == nullptr || Mine != Candidate)
	{
		return;
	}

	ALLKCharacter* Keeper = Cast<ALLKCharacter>(GetOwner());
	if (Keeper == nullptr)
	{
		return;
	}

	// The return value is the point of it. GrantTo refuses a keeper who
	// already has one, a pickup already taken, and a missing component -
	// and a caller that ignored the bool would clear the prompt and leave
	// the boy empty-handed with nothing logged.
	if (!Pickup_GrantAndReport(Mine, Keeper))
	{
		return;
	}

	FocusedLantern = nullptr;
}

bool ULLKInteractorComponent::Pickup_GrantAndReport(
	ALLKLanternPickup* Pickup, ALLKCharacter* Keeper)
{
	if (Pickup->GrantTo(Keeper))
	{
		return true;
	}

	UE_LOG(LogLLK, Warning,
		TEXT("Lantern pickup refused for %s; the prompt and the grant "
			 "disagreed about whether it could be taken."),
		*Keeper->GetName());
	return false;
}

void ULLKInteractorComponent::EndInteract()
{
	if (Holding != nullptr)
	{
		ServerCancelRevive(Holding);
		Holding = nullptr;
	}

	// Unconditional: the server knows whether there was a call hold to drop,
	// and a client tracking that separately is a second copy of the same fact
	// that can disagree with the first.
	ServerCancelExtractionCall();
	ServerCancelRelicTake();
}

void ULLKInteractorComponent::ServerBeginExtractionCall_Implementation(ALLKExtractionZone* Zone)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || Zone == nullptr)
	{
		return;
	}

	// Re-checked on the server because the client asked based on its own view,
	// exactly as the revive is. The per-frame validation in TickExtractionCall
	// is what enforces the commitment; this refuses an opening request that
	// was never valid.
	const ULLKVitalsComponent* Own = Owner->FindComponentByClass<ULLKVitalsComponent>();
	if (Own == nullptr || Own->IsDowned())
	{
		return;
	}

	if (Zone->CountKeepersStanding() <= 0)
	{
		// The keeper asking is not in the bay by the server's reckoning. A
		// client cannot be trusted about where it is standing, and the zone's
		// own occupancy is the one answer the hold already depends on.
		return;
	}

	CallingZone = Zone;
	CallHeldSeconds = 0.f;
}

void ULLKInteractorComponent::ServerCancelExtractionCall_Implementation()
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	CallingZone = nullptr;
	CallHeldSeconds = 0.f;
}

void ULLKInteractorComponent::TickExtractionCall(float DeltaSeconds)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || CallingZone == nullptr)
	{
		return;
	}

	// **Three ways the hold drops, and all three are the keeper's own doing.**
	// Walking out of the bay, going down, or the zone's level unloading under
	// them. A hold that survived any of those would be a recovery rig
	// summoned by somebody who was not there to ask for it.
	const ULLKVitalsComponent* Own = Owner->FindComponentByClass<ULLKVitalsComponent>();
	if (Own == nullptr || Own->IsDowned() || CallingZone->CountKeepersStanding() <= 0)
	{
		CallingZone = nullptr;
		CallHeldSeconds = 0.f;
		return;
	}

	CallHeldSeconds += FMath::Max(0.f, DeltaSeconds);
	if (CallHeldSeconds < CallHoldSeconds)
	{
		return;
	}

	// Cleared before the call, not after: `CallExtraction` logs, broadcasts
	// and lights the dock, and a re-entrant path back into this function
	// would otherwise find a completed hold still sitting here.
	const FName PointId = CallingZone->ExtractionPointId;
	CallingZone = nullptr;
	CallHeldSeconds = 0.f;

	const UWorld* World = GetWorld();
	ULLKExtractionSubsystem* Extraction =
		World != nullptr ? World->GetSubsystem<ULLKExtractionSubsystem>() : nullptr;
	if (Extraction == nullptr)
	{
		return;
	}

	// The subsystem decides, including refusing. The caller is passed for the
	// rig's light, which borrows this keeper's own lantern until a rig
	// brightness is authored (row 75).
	Extraction->CallExtraction(PointId, GetOwner());
}

ALLKExtractionZone* ULLKInteractorComponent::FindZone() const
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (Owner == nullptr || World == nullptr)
	{
		return nullptr;
	}

	const ULLKExtractionSubsystem* Extraction = World->GetSubsystem<ULLKExtractionSubsystem>();
	if (Extraction == nullptr)
	{
		return nullptr;
	}

	// **By overlap and not by aim**, because standing in a bay is not
	// something you point at - which is also why this is the one focus with no
	// reach and no dot product.
	//
	// Resolved from the zones' own triggers on this machine. The zone actor
	// replicates, so a client has its box and can answer "am I in it" for its
	// own pawn; the server still decides whether a hold may start, so a client
	// that got this wrong would only see a prompt it could not use.
	for (const FName& PointId : Extraction->GetExitIds())
	{
		ALLKExtractionZone* Zone = Extraction->FindZone(PointId);
		const UPrimitiveComponent* Trigger = Zone != nullptr ? Zone->GetTrigger() : nullptr;
		if (Trigger != nullptr && Trigger->IsOverlappingActor(Owner))
		{
			return Zone;
		}
	}

	return nullptr;
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
				// The gameplay reach, not the attenuation radius. Row 80 lets a
				// lantern-level crossfade lerp the visual radius, and
				// LLKLanternLevelTest pins it as low as a QUARTER of the gameplay
				// value mid-fade - so a pulse read off it would wake the room by
				// however far the fade had got, and stop charging row 9's price
				// silently and only sometimes. Same seam GetLitLightSources was
				// moved across at row 80; this was the call site it missed.
				RadiusCm = Light->GetGameplayRadiusCm();
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

	SetCarriedCount(0);
}

void ULLKInteractorComponent::SetCarriedCount(const int32 NewCount)
{
	const AActor* Owner = GetOwner();
	if (Owner == nullptr || !Owner->HasAuthority() || NewCount == CarriedCount)
	{
		return;
	}

	CarriedCount = NewCount;

	// Announced on the server too, not only through the OnRep. A listen
	// server's own keeper never receives its own replication, so a burden that
	// only updated in `OnRep_CarriedCount` would be the one keeper in the crew
	// who carries a haul for free - and that keeper is whoever is hosting,
	// which is Chris on every playtest.
	OnCarriedCountChanged.Broadcast(CarriedCount);
}

void ULLKInteractorComponent::OnRep_CarriedCount()
{
	OnCarriedCountChanged.Broadcast(CarriedCount);
}

void ULLKInteractorComponent::TryTakeFocused()
{
	if (Focused == nullptr)
	{
		return;
	}

	// Asked, never done. The client has already been told by the prompt what
	// it is looking at, but the server owns both the hold and the shelf.
	ServerBeginRelicTake(Focused);
}

float ULLKInteractorComponent::AdvanceRelicTakeProgress(
	float CurrentProgress, float DeltaSeconds, bool bLit, float LitSeconds, float DarkSeconds)
{
	const float RequiredSeconds = bLit ? LitSeconds : DarkSeconds;
	return CurrentProgress + FMath::Max(0.f, DeltaSeconds) / FMath::Max(RequiredSeconds, KINDA_SMALL_NUMBER);
}

bool ULLKInteractorComponent::IsRelicInServerReach(const ULLKRelicComponent* Relic) const
{
	const AActor* Keeper = GetOwner();
	const AActor* RelicActor = Relic != nullptr ? Relic->GetOwner() : nullptr;
	const float Reach = LLKUnits::DistanceFromStuds(ReachStuds) * 1.5f;
	return Keeper != nullptr && RelicActor != nullptr
		&& FVector::DistSquared(Keeper->GetActorLocation(), RelicActor->GetActorLocation()) <= Reach * Reach;
}

void ULLKInteractorComponent::ServerBeginRelicTake_Implementation(ULLKRelicComponent* Relic)
{
	const AActor* Keeper = GetOwner();
	if (Keeper == nullptr || !Keeper->HasAuthority() || Relic == nullptr || !Relic->CanBeTaken()
		|| !IsRelicInServerReach(Relic))
	{
		return;
	}

	// A different relic is a different action. Starting it replaces the old
	// hold; temporary loss of light does not, which is the deliberate rule.
	if (TakingRelic != Relic)
	{
		TakingRelic = Relic;
		RelicTakeProgressSeconds = 0.f;
	}
}

void ULLKInteractorComponent::ServerCancelRelicTake_Implementation()
{
	TakingRelic = nullptr;
	RelicTakeProgressSeconds = 0.f;
}

void ULLKInteractorComponent::TickRelicTake(float DeltaSeconds)
{
	if (TakingRelic == nullptr)
	{
		return;
	}

	const AActor* Keeper = GetOwner();
	if (Keeper == nullptr || !Keeper->HasAuthority() || !TakingRelic->CanBeTaken()
		|| !IsRelicInServerReach(TakingRelic))
	{
		ServerCancelRelicTake_Implementation();
		return;
	}

	RelicTakeProgressSeconds = AdvanceRelicTakeProgress(
		RelicTakeProgressSeconds, DeltaSeconds, TakingRelic->IsCurrentlyLit(),
		RelicTakeSecondsLit, RelicTakeSecondsDark);
	if (RelicTakeProgressSeconds < 1.f)
	{
		return;
	}

	ULLKRelicComponent* Completed = TakingRelic;
	ServerCancelRelicTake_Implementation();
	ServerTake_Implementation(Completed);
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

	// The attempt, and the reason, recorded before anything acts on either.
	//
	// **Here rather than in `TryTakeFocused` because the client's prompt is a
	// courtesy and this machine's answer is the one that happened.** Broadcast
	// for successes too, with `Block` as `None`, so a listener counting
	// refusals has the same denominator the player experienced.
	//
	// Announced ahead of the range check deliberately: a take refused for
	// range is not a legibility failure, and recording `None` here with no
	// `RelicTaken` after it leaves that visible in the event list rather than
	// filing a reach from across the room as a rule the player misunderstood.
	const bool bCanTake = Relic->CanBeTaken();
	OnInteractAttempted.Broadcast(
		Relic, bCanTake ? ELLKRelicInteractBlock::None : Relic->GetInteractBlock());

	// Re-checked here rather than trusted from the call. The client decided
	// to ask based on what its own prompt said; whether it may is this
	// machine's question, and the answer includes the live-light gate.
	if (!bCanTake)
	{
		UE_LOG(LogLLK, Verbose,
			TEXT("ULLKInteractorComponent: refused a take - %s"),
			Relic->IsLegible() ? TEXT("not lit") : TEXT("not identified"));
		return;
	}

	// Range re-checked too, against the server's own view of where the keeper
	// is. Without it a client could ask for a relic on another floor.
	if (!IsRelicInServerReach(Relic))
	{
		return;
	}

	if (!Relic->TakeByServer())
	{
		return;
	}

	SetCarriedCount(CarriedCount + 1);

	// Carry slots and the haul are `loot.json`'s rules and a later row; this
	// is a count, and it says so rather than pretending to be an inventory.
	// Burden now reads it - `ALLKCharacter` turns this number into a speed
	// through `LLKBurden`, and nothing else about the count has changed.
	UE_LOG(LogLLK, Log, TEXT("Took %s. Carrying %d."),
		*Relic->GetDisplayName().ToString(), CarriedCount);

	if (Focused == Relic)
	{
		Focused = nullptr;
		OnFocusChanged.Broadcast(nullptr);
	}
}
