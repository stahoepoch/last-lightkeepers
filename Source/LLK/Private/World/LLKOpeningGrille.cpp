#include "World/LLKOpeningGrille.h"

#include "Characters/LLKCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Lantern/LLKLanternComponent.h"
#include "Lantern/LLKLanternFeel.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "LLK.h"
#include "Net/UnrealNetwork.h"

namespace
{
	const FName WitnessTag(TEXT("LLK.Opening.LanternRead"));
	const FName ReleaseTag(TEXT("LLK.Opening.GrilleRelease"));
	const FName SegmentTag(TEXT("LLK.Opening.GrilleSegment"));
	const FName StemTag(TEXT("LLK.Opening.GrilleStem"));
	const FName LinkTag(TEXT("LLK.Opening.GrilleLink"));
	const FName RollerTag(TEXT("LLK.Opening.GrilleRoller"));
}

ALLKOpeningGrille::ALLKOpeningGrille()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ALLKOpeningGrille::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALLKOpeningGrille, bOpen);
}

void ALLKOpeningGrille::BeginPlay()
{
	Super::BeginPlay();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor->ActorHasTag(WitnessTag))
		{
			Witness = Actor;
		}
		if (Actor->ActorHasTag(ReleaseTag))
		{
			ReleaseBar = Actor;
			ClosedBarLocation = Actor->GetActorLocation();
		}
		if (Actor->ActorHasTag(StemTag))
		{
			ReleaseStem = Actor;
			ClosedStemLocation = Actor->GetActorLocation();
		}
		if (Actor->ActorHasTag(LinkTag))
		{
			ChainLink = Actor;
			ClosedLinkLocation = Actor->GetActorLocation();
		}
		if (Actor->ActorHasTag(RollerTag))
		{
			HeaderRoller = Actor;
			ClosedRollerRotation = Actor->GetActorRotation();
		}
		if (Actor->ActorHasTag(SegmentTag))
		{
			Segments.Add(Actor);
			ClosedLocations.Add(Actor->GetActorLocation());
		}
	}
	if (!Witness.IsValid() || !ReleaseBar.IsValid() || Segments.Num() != 45)
	{
		UE_LOG(LogLLK, Error, TEXT("Opening grille incomplete: witness=%d release=%d segments=%d (expected 45)"),
			Witness.IsValid(), ReleaseBar.IsValid(), Segments.Num());
	}
	if (!ReleaseStem.IsValid() || !ChainLink.IsValid() || !HeaderRoller.IsValid())
	{
		UE_LOG(LogLLK, Warning, TEXT("Opening grille release hardware is incomplete; the bars can still rise."));
	}
}

FVector ALLKOpeningGrille::GetReadPoint() const
{
	return Witness.IsValid() ? Witness->GetActorLocation() : GetActorLocation();
}

bool ALLKOpeningGrille::CanFocus(const APawn* Keeper) const
{
	if (bOpen || Keeper == nullptr || !Witness.IsValid() || !ReleaseBar.IsValid()
		|| Segments.Num() != 45 || Keeper->GetWorld() != GetWorld())
	{
		return false;
	}

	const FVector Eye = LLKLanternFeel::GameplayLightPosition(
		Keeper->GetActorLocation(), LLKLanternFeel::KeeperEyeHeightCm);
	const FVector Point = GetReadPoint();
	const FVector ToMark = Point - Eye;
	const float Distance = ToMark.Size();
	if (Distance < KINDA_SMALL_NUMBER || Distance > ReadReachCm
		|| FVector::DotProduct(Keeper->GetBaseAimRotation().Vector(), ToMark / Distance) < ReadAimDot)
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LLKOpeningReleaseRead), false, Keeper);
	FHitResult Hit;
	return !GetWorld()->LineTraceSingleByChannel(Hit, Eye, Point, ECC_Visibility, Params)
		|| Hit.GetActor() == Witness.Get();
}

bool ALLKOpeningGrille::CanRelease(const APawn* Keeper) const
{
	if (!CanFocus(Keeper))
	{
		return false;
	}
	const ULLKLanternComponent* Lantern = Keeper->FindComponentByClass<ULLKLanternComponent>();
	const ULLKLanternLightComponent* Beam = Keeper->FindComponentByClass<ULLKLanternLightComponent>();
	if (Lantern == nullptr || !Lantern->HasLantern() || !Lantern->IsLanternOn()
		|| Beam == nullptr || Beam->GetAttachParent() == nullptr)
	{
		return false;
	}

	// Read the stable hand mount, never cosmetic lantern sway. The mark must
	// actually fall in the carried beam, in reach and without a blocker. This
	// is the opening's light puzzle, not a global "lamp on" switch.
	const FVector Source = Beam->GetGameplayLightLocation();
	const FVector ToMark = GetReadPoint() - Source;
	const float Distance = ToMark.Size();
	if (Distance < KINDA_SMALL_NUMBER || Distance > Beam->GetGameplayRadiusCm())
	{
		return false;
	}
	const float HalfAngleRadians = FMath::DegreesToRadians(Beam->GetEffectiveOuterConeDegrees());
	if (FVector::DotProduct(Beam->GetAttachParent()->GetForwardVector(), ToMark / Distance)
		< FMath::Cos(HalfAngleRadians))
	{
		return false;
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(LLKOpeningBeamRead), false, Keeper);
	FHitResult Hit;
	return !GetWorld()->LineTraceSingleByChannel(Hit, Source, GetReadPoint(), ECC_Visibility, Params)
		|| Hit.GetActor() == Witness.Get();
}

bool ALLKOpeningGrille::TryRelease(const APawn* Keeper)
{
	if (!HasAuthority() || !CanRelease(Keeper))
	{
		return false;
	}
	bOpen = true;
	UE_LOG(LogLLK, Log, TEXT("Opening grille released with the lantern; raising %d segments."), Segments.Num());
	return true;
}

void ALLKOpeningGrille::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bOpen || OpenAlpha >= 1.f)
	{
		return;
	}
	OpenAlpha = FMath::Min(1.f, OpenAlpha + DeltaSeconds / RaiseSeconds);
	const float Eased = FMath::InterpEaseInOut(0.f, 1.f, OpenAlpha, 2.f);
	// The player's hand moves the same physical bar the machine moved. The
	// stem follows it, the chain link draws upward, and the roller turns while
	// the bars lift. These are presentation pieces; collision stays on the bars.
	if (AActor* Bar = ReleaseBar.Get())
	{
		Bar->SetActorLocation(ClosedBarLocation + FVector(0.f, 9.f, -23.f) * Eased);
	}
	if (AActor* Stem = ReleaseStem.Get())
	{
		Stem->SetActorLocation(ClosedStemLocation + FVector(0.f, 0.f, -20.f) * Eased);
	}
	if (AActor* Link = ChainLink.Get())
	{
		Link->SetActorLocation(ClosedLinkLocation + FVector(0.f, 0.f, 55.f) * Eased);
	}
	if (AActor* Roller = HeaderRoller.Get())
	{
		Roller->SetActorRotation(ClosedRollerRotation + FRotator(0.f, 0.f, 360.f * Eased));
	}
	for (int32 Index = 0; Index < Segments.Num(); ++Index)
	{
		AActor* Segment = Segments[Index].Get();
		if (Segment == nullptr)
		{
			continue;
		}
		Segment->SetActorLocation(ClosedLocations[Index] + FVector(0.f, 0.f, RaiseCm * Eased));
		if (OpenAlpha >= 1.f)
		{
			if (UStaticMeshComponent* Body = Segment->FindComponentByClass<UStaticMeshComponent>())
			{
				Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}
