#include "Run/LLKExtractionZone.h"

#include "Characters/LLKVitalsComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "Run/LLKExtractionSubsystem.h"

// ---- FLLKZoneOccupancy ---------------------------------------------------

void FLLKZoneOccupancy::Enter(ULLKVitalsComponent* Keeper)
{
	if (Keeper == nullptr)
	{
		return;
	}

	// AddUnique on the weak pointer. Unreal sends a begin-overlap per
	// overlapping body, so a capsule entering a box can report twice - and a
	// keeper counted twice would still be in the zone after walking out of it
	// once, which is a hold that advances with an empty bay.
	Inside.AddUnique(Keeper);
}

void FLLKZoneOccupancy::Leave(ULLKVitalsComponent* Keeper)
{
	if (Keeper == nullptr)
	{
		return;
	}

	Inside.Remove(Keeper);
}

int32 FLLKZoneOccupancy::CountInside() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ULLKVitalsComponent>& Weak : Inside)
	{
		if (Weak.Get() != nullptr)
		{
			++Count;
		}
	}
	return Count;
}

int32 FLLKZoneOccupancy::CountStanding() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ULLKVitalsComponent>& Weak : Inside)
	{
		const ULLKVitalsComponent* Keeper = Weak.Get();
		if (Keeper != nullptr && !Keeper->IsDowned())
		{
			++Count;
		}
	}
	return Count;
}

// ---- ALLKExtractionZone --------------------------------------------------

ALLKExtractionZone::ALLKExtractionZone()
{
	// No tick. The zone is told things by the engine and asked things by the
	// subsystem; it has nothing of its own to do between those.
	PrimaryActorTick.bCanEverTick = false;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);

	// A first guess at a bay mouth, in centimetres via row 61's one authority:
	// about twelve studs wide, eighteen deep and eight high. It is a default on
	// an unplaced actor rather than a tuned value - the real extents come with
	// the greybox's dock, which is why this is `VisibleAnywhere` on the
	// component and editable there rather than a number in `data/`.
	Trigger->SetBoxExtent(FVector(
		LLKUnits::DistanceFromStuds(9.f),
		LLKUnits::DistanceFromStuds(6.f),
		LLKUnits::DistanceFromStuds(4.f)));

	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldStatic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);

	// Replicates its transform and nothing else. The occupancy is server-side
	// truth that feeds a rule; a client knowing who is in the bay is what the
	// extraction snapshot is for, and the snapshot already carries the one
	// thing a readout needs - whether the hold is stalled.
	bReplicates = true;
}

void ALLKExtractionZone::BeginPlay()
{
	Super::BeginPlay();

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ALLKExtractionZone::OnTriggerBeginOverlap);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &ALLKExtractionZone::OnTriggerEndOverlap);

	UWorld* World = GetWorld();
	ULLKExtractionSubsystem* Extraction =
		World != nullptr ? World->GetSubsystem<ULLKExtractionSubsystem>() : nullptr;
	if (Extraction == nullptr)
	{
		return;
	}

	Extraction->RegisterZone(this);

	// A typo in the id is the one mistake placing this actor can make, and it
	// would otherwise show up as an extraction that never completes. Named at
	// BeginPlay, with the authored exits listed, which is the `LLK.CheckMap`
	// discipline at the cheapest possible moment to apply it.
	FLLKExtractionPointRules Unused;
	FName UnusedRequires;
	if (!Extraction->FindExit(ExtractionPointId, Unused, UnusedRequires))
	{
		const TArray<FName> Ids = Extraction->GetExitIds();
		UE_LOG(LogLLK, Warning,
			TEXT("%s names exit '%s', which nothing authored. Authored exits: %s"),
			*GetName(),
			ExtractionPointId.IsNone() ? TEXT("<none>") : *ExtractionPointId.ToString(),
			Ids.IsEmpty()
				? TEXT("none - run Tools/import_data.py")
				: *FString::JoinBy(Ids, TEXT(", "), [](const FName& Id) { return Id.ToString(); }));
	}
}

void ALLKExtractionZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		if (ULLKExtractionSubsystem* Extraction = World->GetSubsystem<ULLKExtractionSubsystem>())
		{
			Extraction->UnregisterZone(this);
		}
	}

	Occupancy.Clear();

	Super::EndPlay(EndPlayReason);
}

void ALLKExtractionZone::OnTriggerBeginOverlap(
	UPrimitiveComponent* /*OverlappedComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComponent*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*Sweep*/)
{
	if (OtherActor == nullptr)
	{
		return;
	}

	// A keeper is an actor with vitals. Not a pawn, not a character class:
	// vitals is what the rule actually needs, because "is this keeper down"
	// is the question the hold asks - and it keeps the zone working for
	// anything a later milestone makes a keeper.
	if (ULLKVitalsComponent* Keeper = OtherActor->FindComponentByClass<ULLKVitalsComponent>())
	{
		Occupancy.Enter(Keeper);
	}
}

void ALLKExtractionZone::OnTriggerEndOverlap(
	UPrimitiveComponent* /*OverlappedComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComponent*/,
	int32 /*OtherBodyIndex*/)
{
	if (OtherActor == nullptr)
	{
		return;
	}

	if (ULLKVitalsComponent* Keeper = OtherActor->FindComponentByClass<ULLKVitalsComponent>())
	{
		Occupancy.Leave(Keeper);
	}
}
