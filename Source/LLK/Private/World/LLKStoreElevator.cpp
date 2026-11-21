#include "World/LLKStoreElevator.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ALLKStoreElevator::ALLKStoreElevator()
{
	bReplicates = true;
	CabTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CabTrigger"));
	SetRootComponent(CabTrigger);
	CabTrigger->SetBoxExtent(FVector(95.f, 95.f, 115.f));
	CabTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CabTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	CabTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CabTrigger->SetGenerateOverlapEvents(true);

	CabVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CabVisual"));
	CabVisual->SetupAttachment(CabTrigger);
	CabVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		CabVisual->SetStaticMesh(Cube.Object);
		CabVisual->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.04f));
	}
}

void ALLKStoreElevator::BeginPlay()
{
	Super::BeginPlay();
	CabTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALLKStoreElevator::OnCabOverlap);
	if (LowerStop.IsNearlyZero())
	{
		LowerStop = GetActorLocation();
	}
}

bool ALLKStoreElevator::CanUse(const APawn* Pawn) const
{
	return HasAuthority() && !bTravelling && Pawn != nullptr && Pawn->GetWorld() == GetWorld();
}

void ALLKStoreElevator::OnCabOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!CanUse(Pawn))
	{
		return;
	}

	Passenger = Pawn;
	const float ToLower = FVector::DistSquared(Pawn->GetActorLocation(), LowerStop);
	const float ToUpper = FVector::DistSquared(Pawn->GetActorLocation(), UpperStop);
	PendingStop = ToLower <= ToUpper ? UpperStop : LowerStop;
	bTravelling = true;
	Pawn->DisableInput(nullptr);
	GetWorldTimerManager().SetTimer(TravelTimer, this, &ALLKStoreElevator::CompleteTravel,
		TravelSeconds, false);
}

void ALLKStoreElevator::CompleteTravel()
{
	APawn* Pawn = Passenger.Get();
	if (Pawn != nullptr)
	{
		Pawn->SetActorLocation(PendingStop, false, nullptr, ETeleportType::TeleportPhysics);
		Pawn->EnableInput(nullptr);
	}
	Passenger.Reset();
	bTravelling = false;
}

void ALLKStoreElevator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALLKStoreElevator, bTravelling);
}
