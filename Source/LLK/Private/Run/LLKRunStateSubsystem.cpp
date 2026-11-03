#include "Run/LLKRunStateSubsystem.h"

#include "Characters/LLKVitalsComponent.h"
#include "Engine/World.h"
#include "LLK.h"
#include "LLKGameState.h"
#include "Loot/LLKInteractorComponent.h"

bool ULLKRunStateSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// Game and PIE only, as with the storm. An editor preview world declaring
	// a crew wipe would be a strange thing to debug.
	const UWorld* World = Cast<UWorld>(Outer);
	return World != nullptr && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId ULLKRunStateSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULLKRunStateSubsystem, STATGROUP_Tickables);
}

bool ULLKRunStateSubsystem::HasRunAuthority() const
{
	if (bHasForcedAuthority)
	{
		return bForcedAuthority;
	}

	const UWorld* World = GetWorld();
	return World != nullptr && World->GetNetMode() != NM_Client;
}

void ULLKRunStateSubsystem::RegisterKeeper(ULLKVitalsComponent* Keeper)
{
	if (Keeper == nullptr)
	{
		return;
	}

	// AddUnique, because a keeper counted twice would be a keeper who can
	// still act after they cannot.
	Keepers.AddUnique(Keeper);
}

void ULLKRunStateSubsystem::UnregisterKeeper(ULLKVitalsComponent* Keeper)
{
	Keepers.Remove(Keeper);
}

int32 ULLKRunStateSubsystem::CountKeepers() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ULLKVitalsComponent>& Weak : Keepers)
	{
		if (Weak.Get() != nullptr)
		{
			++Count;
		}
	}
	return Count;
}

TArray<ULLKVitalsComponent*> ULLKRunStateSubsystem::GetKeepers() const
{
	TArray<ULLKVitalsComponent*> Alive;
	Alive.Reserve(Keepers.Num());
	for (const TWeakObjectPtr<ULLKVitalsComponent>& Weak : Keepers)
	{
		if (ULLKVitalsComponent* Keeper = Weak.Get())
		{
			Alive.Add(Keeper);
		}
	}
	return Alive;
}

int32 ULLKRunStateSubsystem::CountKeepersWhoCanAct() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ULLKVitalsComponent>& Weak : Keepers)
	{
		const ULLKVitalsComponent* Keeper = Weak.Get();
		if (Keeper == nullptr)
		{
			// Destroyed mid-run. Recounted rather than tracked, the same
			// no-ghost rule the lit-lantern count follows.
			continue;
		}

		// On their feet: obviously can act.
		if (!Keeper->IsDowned())
		{
			++Count;
			continue;
		}

		// Down, but holding their own way back up. Chris's rule 2: a solo
		// keeper's first knockdown is not a wipe while the anchor revive is
		// still available, because continuing is still their choice to make.
		if (Keeper->IsAnchorReviveAvailable())
		{
			++Count;
			continue;
		}

		// Down and halfway through using it. Counting standing bodies would
		// end the run in the middle of the eight seconds that save it.
		if (Keeper->IsSelfReviving())
		{
			++Count;
		}
	}
	return Count;
}

void ULLKRunStateSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCrewWiped || !HasRunAuthority())
	{
		return;
	}

	SecondsSinceEvaluate += DeltaTime;
	if (SecondsSinceEvaluate < EvaluateIntervalSeconds)
	{
		return;
	}
	SecondsSinceEvaluate = 0.f;

	Evaluate();
}

void ULLKRunStateSubsystem::Evaluate()
{
	if (bCrewWiped || !HasRunAuthority())
	{
		return;
	}

	// An empty roll is not a wipe. Before any keeper has registered - the
	// first frames of a level - zero keepers who can act would otherwise end
	// the run before it began.
	if (CountKeepers() <= 0)
	{
		return;
	}

	if (CountKeepersWhoCanAct() > 0)
	{
		return;
	}

	// Idempotent by this flag, and the flag is why: row 8's haul resolution,
	// the end screen and any future scoring all hang off one event, and each
	// of them firing twice is a different bug.
	bCrewWiped = true;

	UE_LOG(LogLLK, Log,
		TEXT("CREW CONTAINED. %d keeper(s) in the run and none able to act. The night keeps the haul."),
		CountKeepers());

	ResolveHaul();

	if (UWorld* World = GetWorld())
	{
		if (ALLKGameState* GameState = World->GetGameState<ALLKGameState>())
		{
			GameState->PublishCrewWiped();
		}
	}

	OnCrewWiped.Broadcast();
}

void ULLKRunStateSubsystem::ResolveHaul()
{
	// Row 8: only extraction secures loot, and a wipe loses the whole haul.
	// Done here rather than left to a listener, because "the night keeps what
	// you carried" is the consequence that makes the whole risk real - and a
	// consequence hanging off a delegate nobody bound would be the dangling
	// seam this project keeps finding.
	int32 Lost = 0;
	for (const TWeakObjectPtr<ULLKVitalsComponent>& Weak : Keepers)
	{
		const ULLKVitalsComponent* Keeper = Weak.Get();
		const AActor* Owner = Keeper != nullptr ? Keeper->GetOwner() : nullptr;
		if (Owner == nullptr)
		{
			continue;
		}

		if (ULLKInteractorComponent* Carried = Owner->FindComponentByClass<ULLKInteractorComponent>())
		{
			Lost += Carried->GetCarriedCount();
			Carried->ClearCarriedOnWipe();
		}
	}

	UE_LOG(LogLLK, Log, TEXT("The night kept %d carried item(s)."), Lost);
}

void ULLKRunStateSubsystem::RaiseFloorAlert(const float Seconds)
{
	const UWorld* World = GetWorld();
	if (World == nullptr || Seconds <= 0.f || !FMath::IsFinite(Seconds))
	{
		return;
	}

	// Extend, never shorten. A second machine filing while the first alert is
	// still running must not reset the clock to something smaller - the floor
	// gets more attentive, not less, as faults accumulate.
	FloorAlertExpirySeconds =
		FMath::Max(FloorAlertExpirySeconds, World->GetTimeSeconds() + Seconds);

	UE_LOG(LogLLK, Log,
		TEXT("The floor is alerted for %.1fs: a machine filed a fault and every ")
		TEXT("Sweeper is now noticing from its swarm-call distance."),
		Seconds);
}

bool ULLKRunStateSubsystem::IsFloorAlerted() const
{
	return GetFloorAlertSecondsRemaining() > 0.f;
}

float ULLKRunStateSubsystem::GetFloorAlertSecondsRemaining() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr || FloorAlertExpirySeconds <= 0.f)
	{
		return 0.f;
	}

	return FMath::Max(0.f, FloorAlertExpirySeconds - World->GetTimeSeconds());
}
