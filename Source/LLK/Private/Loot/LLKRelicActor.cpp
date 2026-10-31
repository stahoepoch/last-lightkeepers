#include "Loot/LLKRelicActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Loot/LLKRelicComponent.h"
#include "UObject/ConstructorHelpers.h"

ALLKRelicActor::ALLKRelicActor()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	SetRootComponent(Visual);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetGenerateOverlapEvents(false);
	Visual->SetRenderCustomDepth(true);
	Visual->SetWorldScale3D(FVector(0.22f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		Visual->SetStaticMesh(Cube.Object);
	}

	Relic = CreateDefaultSubobject<ULLKRelicComponent>(TEXT("Relic"));
}

void ALLKRelicActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyDefinition();
}

void ALLKRelicActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyDefinition();
}

void ALLKRelicActor::ApplyDefinition()
{
	if (Relic == nullptr)
	{
		return;
	}

	Relic->Visibility = Visibility;
	Relic->SetRelicIdentity(
		RelicId,
		RelicName.IsEmpty() ? FText::FromName(RelicId) : RelicName,
		Value);
}
