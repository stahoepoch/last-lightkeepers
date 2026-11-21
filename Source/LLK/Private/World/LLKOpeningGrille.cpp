#include "World/LLKOpeningGrille.h"

#include "Characters/LLKCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/PointLight.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Lantern/LLKLanternComponent.h"
#include "Lantern/LLKLanternFeel.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "LLK.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "Components/PointLightComponent.h"

namespace
{
	const FName WitnessTag(TEXT("LLK.Opening.LanternRead"));
	const FName ReleaseTag(TEXT("LLK.Opening.GrilleRelease"));
	const FName SegmentTag(TEXT("LLK.Opening.GrilleSegment"));
	const FName StemTag(TEXT("LLK.Opening.GrilleStem"));
	const FName LinkTag(TEXT("LLK.Opening.GrilleLink"));
	const FName RollerTag(TEXT("LLK.Opening.GrilleRoller"));
	const FName CaptiveTag(TEXT("LLK.Opening.Captive"));
	const FName CaptorTag(TEXT("LLK.Opening.Captor"));
	const FName StormTag(TEXT("LLK.Opening.StormFlash"));
	const FName FallenLanternTag(TEXT("LLK.Opening.FallenLantern"));
	const FName DockGrabTag(TEXT("LLK.DockOpening.Grab"));
	constexpr float DockGrabLeadSeconds = 1.35f;
	constexpr float DockHaulSpeed = 530.f;
	// The bars must be below the child's head before a clean sprint from the
	// counter can reach them. Keep these in one place for the route check.
	constexpr float CaptureLiftStart = 0.05f;
	constexpr float CaptureLiftEnd = 0.60f;
	constexpr float CaptureHaulStart = 0.55f;
	constexpr float CaptureHaulEnd = 1.65f;
	constexpr float CaptureFallStart = 1.65f;
	constexpr float CaptureFallEnd = 2.15f;
	constexpr float CaptureFinish = 2.20f;
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
	TMap<int32, FVector> HaulPoints;
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
		if (Actor->ActorHasTag(CaptiveTag))
		{
			Captives.Add(Actor);
			CaptiveStarts.Add(Actor->GetActorLocation());
		}
		if (Actor->ActorHasTag(CaptorTag))
		{
			Captor = Actor;
			CaptorStart = Actor->GetActorLocation();
		}
		if (Actor->ActorHasTag(StormTag))
		{
			StormFlash = Cast<APointLight>(Actor);
		}
		if (Actor->ActorHasTag(FallenLanternTag))
		{
			FallenLantern = Actor;
		}
		if (Actor->ActorHasTag(DockGrabTag))
		{
			HaulPoints.Add(0, Actor->GetActorLocation());
		}
		if (Actor->ActorHasTag(FName(TEXT("LLK.DockOpening.FarReceiving"))))
		{
			FarReceiving = Actor;
		}
		for (const FName& Tag : Actor->Tags)
		{
			FString IndexString = Tag.ToString();
			if (IndexString.RemoveFromStart(TEXT("LLK.DockOpening.Haul.")) && IndexString.IsNumeric())
			{
				HaulPoints.Add(FCString::Atoi(*IndexString), Actor->GetActorLocation());
			}
		}
	}
	for (int32 Index = 0; Index < HaulPoints.Num(); ++Index)
	{
		const FVector* Point = HaulPoints.Find(Index);
		if (Point == nullptr)
		{
			DockRoute.Reset();
			break;
		}
		DockRoute.Add(*Point);
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
	// Missing scene pieces never strand the player: the normal release still
	// works. A partial cast is logged instead of presenting a false capture.
	bCaptureActive = Captives.Num() == 4 && Captor.IsValid() && Segments.Num() == 45;
	bCaptureFinished = !bCaptureActive;
	if (bCaptureActive)
	{
		if (AActor* Lamp = FallenLantern.Get())
		{
			Lamp->SetActorHiddenInGame(true);
		}
		HaulSound = LoadObject<USoundBase>(nullptr,
			TEXT("/Game/Audio/Sweeper/sweeper-stock-started-v1.sweeper-stock-started-v1"));
		CrashSound = LoadObject<USoundBase>(nullptr,
			TEXT("/Game/Audio/Sweeper/sweeper-stock-filed-v1.sweeper-stock-filed-v1"));
		if (UsesDockOpening())
		{
			StartDockCapture();
			return;
		}
		UAnimSequence* Walk = LoadObject<UAnimSequence>(nullptr,
			TEXT("/Game/Characters/UEFN_Mannequin/Animations/LookAtPOI/LookAtPOI_test_walk_loop.LookAtPOI_test_walk_loop"));
		UAnimSequence* Sprint = LoadObject<UAnimSequence>(nullptr,
			TEXT("/Game/Characters/UEFN_Mannequin/Animations/LookAtPOI/LookAtPOI_test_sprint_loop.LookAtPOI_test_sprint_loop"));
		if (Walk == nullptr || Sprint == nullptr)
		{
			UE_LOG(LogLLK, Error, TEXT("Opening capture gait animation missing; release remains available."));
			bCaptureActive = false;
			bCaptureFinished = true;
		}
		else
		{
			for (int32 Index = 0; Index < Captives.Num(); ++Index)
			{
				ASkeletalMeshActor* Person = Cast<ASkeletalMeshActor>(Captives[Index].Get());
				if (Person == nullptr || Person->GetSkeletalMeshComponent() == nullptr)
				{
					bCaptureActive = false;
					bCaptureFinished = true;
					break;
				}
				const bool bRunning = CaptiveStarts[Index].Y > -2480.f;
				UAnimSequence* Gait = bRunning ? Sprint : Walk;
				Person->GetSkeletalMeshComponent()->PlayAnimation(Gait, true);
				// Four identical mannequins stepping on the same frame read as a
				// factory line. The cast enters on different phases and tempos.
				if (UAnimSingleNodeInstance* Player = Person->GetSkeletalMeshComponent()->GetSingleNodeInstance())
				{
					Player->SetPosition(Gait->GetPlayLength() * 0.19f * Index, false);
					Player->SetPlayRate(0.94f + 0.05f * Index);
				}
			}
		}
	}
	if (!bCaptureActive && (Captives.Num() > 0 || Captor.IsValid()))
	{
		UE_LOG(LogLLK, Error, TEXT("Opening capture incomplete: people=%d captor=%d; grille remains playable."),
			Captives.Num(), Captor.IsValid());
	}
}

void ALLKOpeningGrille::StartDockCapture()
{
	DockRouteDistances.Add(0.f);
	for (int32 Index = 1; Index < DockRoute.Num(); ++Index)
	{
		DockRouteDistances.Add(DockRouteDistances.Last()
			+ FVector::Distance(DockRoute[Index - 1], DockRoute[Index]));
	}
	DockVoiceAttenuation = NewObject<USoundAttenuation>(this);
	DockVoiceAttenuation->Attenuation.bSpatialize = true;
	DockVoiceAttenuation->Attenuation.AttenuationShapeExtents = FVector(340.f, 0.f, 0.f);
	DockVoiceAttenuation->Attenuation.FalloffDistance = 6100.f;
}

FVector ALLKOpeningGrille::SampleDockRoute(float Distance, FVector& Direction) const
{
	for (int32 End = 1; End < DockRoute.Num(); ++End)
	{
		if (Distance <= DockRouteDistances[End] || End == DockRoute.Num() - 1)
		{
			const FVector& Start = DockRoute[End - 1];
			const FVector& Target = DockRoute[End];
			Direction = (Target - Start).GetSafeNormal2D();
			const float Alpha = FMath::Clamp((Distance - DockRouteDistances[End - 1]) /
				FMath::Max(1.f, DockRouteDistances[End] - DockRouteDistances[End - 1]), 0.f, 1.f);
			return FMath::Lerp(Start, Target, Alpha);
		}
	}
	Direction = FVector::ForwardVector;
	return CaptorStart;
}

void ALLKOpeningGrille::TickDockCapture(float DeltaSeconds)
{
	const APlayerController* Player = GetWorld()->GetFirstPlayerController();
	if (Player == nullptr || Player->GetPawn() == nullptr)
	{
		return; // Start the beat after possession, with the player present.
	}
	CaptureSeconds += DeltaSeconds;
	if (CaptureSeconds < DockGrabLeadSeconds)
	{
		return;
	}
	DockGrabSeconds = CaptureSeconds - DockGrabLeadSeconds;
	const float GrabAlpha = FMath::Clamp(DockGrabSeconds / .55f, 0.f, 1.f);
	const float Travel = FMath::Max(0.f, DockGrabSeconds - .36f) * DockHaulSpeed;
	FVector Direction;
	const FVector MachinePoint = SampleDockRoute(Travel, Direction);
	if (AActor* Machine = Captor.Get())
	{
		Machine->SetActorLocationAndRotation(FMath::Lerp(CaptorStart, MachinePoint, GrabAlpha),
			Direction.Rotation());
	}
	if (!bHaulPlayed)
	{
		bHaulPlayed = true;
		if (HaulSound) UGameplayStatics::PlaySoundAtLocation(this, HaulSound, CaptorStart);
		UE_LOG(LogLLK, Log, TEXT("Dock opening: hauler grabbed the family."));
	}
	if (DockGrabSeconds >= .48f)
	{
		if (AActor* Lamp = FallenLantern.Get())
		{
			Lamp->SetActorHiddenInGame(false);
		}
	}
	for (int32 Index = 0; Index < Captives.Num(); ++Index)
	{
		if (AActor* Person = Captives[Index].Get())
		{
			FVector BodyDirection;
			const FVector Track = SampleDockRoute(Travel - 105.f - 83.f * (Index / 2), BodyDirection);
			const FVector Side = FVector::CrossProduct(FVector::UpVector, BodyDirection);
			const float Struggle = FMath::Sin(DockGrabSeconds * 12.f + Index * 1.4f);
			const FVector DragPoint = Track + Side * ((Index % 2 == 0 ? -1.f : 1.f) *
				(55.f + Struggle * 9.f)) + FVector(0.f, 0.f, (21.f + 4.f * Struggle));
			FRotator DragPose = BodyDirection.Rotation();
			DragPose.Yaw += 180.f + 8.f * Struggle;
			DragPose.Pitch = GrabAlpha * (-78.f + 7.f * Struggle);
			DragPose.Roll = 9.f * GrabAlpha * Struggle;
			const float CaptureBlend = FMath::Clamp((DockGrabSeconds - .045f * Index) / .7f, 0.f, 1.f);
			Person->SetActorLocationAndRotation(
				FMath::Lerp(CaptiveStarts[Index], DragPoint, CaptureBlend), DragPose);
		}
	}
	if (!DockScreams.IsEmpty() && DockGrabSeconds >= NextDockScream)
	{
		const int32 Index = DockScreamIndex++;
		USoundBase* Scream = DockScreams[Index % DockScreams.Num()];
		if (Scream && !Captives.IsEmpty())
		{
			if (AActor* Person = Captives[Index % Captives.Num()].Get())
			{
				UGameplayStatics::SpawnSoundAttached(Scream, Person->GetRootComponent(), NAME_None,
					FVector(0.f, 0.f, 50.f), EAttachLocation::KeepRelativeOffset, true, 1.0f,
					.9f + .09f * (Index % 4), 0.f, DockVoiceAttenuation);
				UE_LOG(LogLLK, Display, TEXT("Dock opening: spatial scream %d at %s."),
					Index + 1, *Person->GetActorLocation().ToCompactString());
			}
		}
		NextDockScream += 2.15f + .33f * (Index % 3);
	}
	if (Travel >= DockRouteDistances.Last() + 215.f)
	{
		for (const TWeakObjectPtr<AActor>& Person : Captives)
		{
			if (Person.IsValid()) Person->SetActorHiddenInGame(true);
		}
		if (Captor.IsValid()) Captor->SetActorHiddenInGame(true);
		bCaptureActive = false;
		bCaptureFinished = true;
		UE_LOG(LogLLK, Display, TEXT("Dock opening: capture complete at the far side of Boswell's."));
	}
}

FVector ALLKOpeningGrille::GetReadPoint() const
{
	return Witness.IsValid() ? Witness->GetActorLocation() : GetActorLocation();
}

bool ALLKOpeningGrille::CanFocus(const APawn* Keeper) const
{
	if (bOpen || !bCaptureFinished || Keeper == nullptr || !Witness.IsValid() || !ReleaseBar.IsValid()
		|| Segments.Num() != 45 || Keeper->GetWorld() != GetWorld())
	{
		return false;
	}

	// The pawn location is the capsule centre. The authored eye height is
	// measured from its feet; adding both puts the trace above the child's
	// head and disagrees with the pickup beside this release.
	const FVector Eye = LLKLanternFeel::KeeperEyeWorldPosition(
		Keeper->GetActorLocation());
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
	if (!bFarReceivingReached && FarReceiving.IsValid())
	{
		const APlayerController* Player = GetWorld()->GetFirstPlayerController();
		const APawn* Keeper = Player ? Player->GetPawn() : nullptr;
		if (Keeper && FVector::DistSquared(Keeper->GetActorLocation(),
			FarReceiving->GetActorLocation()) < FMath::Square(260.f))
		{
			bFarReceivingReached = true;
			UE_LOG(LogLLK, Display, TEXT("Dock opening: reached far receiving."));
		}
	}
	if (bCaptureActive && UsesDockOpening())
	{
		TickDockCapture(DeltaSeconds);
		return;
	}
	if (bCaptureActive)
	{
		CaptureSeconds += DeltaSeconds;
		if (APointLight* Flash = StormFlash.Get())
		{
			// Brief storm strokes against the glass, never permanent room fill.
			const bool bLit = (CaptureSeconds > 0.10f && CaptureSeconds < 0.22f)
				|| (CaptureSeconds > 0.92f && CaptureSeconds < 1.04f)
				|| (CaptureSeconds > 1.65f && CaptureSeconds < 1.77f);
			if (UPointLightComponent* Lamp = Cast<UPointLightComponent>(Flash->GetLightComponent()))
			{
				Lamp->SetIntensity(bLit ? 180.f : 0.f);
			}
		}
		if (!bHaulPlayed && CaptureSeconds >= CaptureLiftStart)
		{
			bHaulPlayed = true;
			if (HaulSound) UGameplayStatics::PlaySoundAtLocation(this, HaulSound, CaptorStart);
		}
		if (CaptureSeconds >= 1.20f)
		{
			if (AActor* Lamp = FallenLantern.Get())
			{
				Lamp->SetActorHiddenInGame(false);
			}
		}
		if (!bCrashPlayed && CaptureSeconds >= CaptureFallStart)
		{
			bCrashPlayed = true;
			if (CrashSound) UGameplayStatics::PlaySoundAtLocation(this, CrashSound, CaptorStart);
		}
		// The grille begins closed in the saved map. It lifts just far enough
		// for the four figures, then crashes shut. The child keeps movement
		// and look, but even a straight-line sprint from their saved start
		// arrives after the bars become too low to cross.
		const float Lift = FMath::Clamp((CaptureSeconds - CaptureLiftStart)
			/ (CaptureLiftEnd - CaptureLiftStart), 0.f, 1.f);
		const float Fall = 1.f - FMath::Clamp((CaptureSeconds - CaptureFallStart)
			/ (CaptureFallEnd - CaptureFallStart), 0.f, 1.f);
		const float Height = RaiseCm * FMath::Min(Lift, Fall);
		for (int32 Index = 0; Index < Segments.Num(); ++Index)
		{
			if (AActor* Segment = Segments[Index].Get())
			{
				Segment->SetActorLocation(ClosedLocations[Index] + FVector(0.f, 0.f, Height));
			}
		}
		const float Haul = FMath::Clamp((CaptureSeconds - CaptureHaulStart)
			/ (CaptureHaulEnd - CaptureHaulStart), 0.f, 1.f);
		for (int32 Index = 0; Index < Captives.Num(); ++Index)
		{
			if (AActor* Person = Captives[Index].Get())
			{
				// All four clear the bars but stop inside the glazed vestibule.
				// A fixed displacement sent the nearer people through the
				// exterior glass; their different starting depths need a common
				// destination instead.
				const FVector Start = CaptiveStarts[Index];
				Person->SetActorLocation(FVector(Start.X, FMath::Lerp(Start.Y, -2780.f, Haul), Start.Z));
				Person->SetActorHiddenInGame(CaptureSeconds >= CaptureFallEnd);
			}
		}
		if (AActor* Machine = Captor.Get())
		{
			Machine->SetActorLocation(CaptorStart + FVector(0.f, -180.f * Haul, 0.f));
			Machine->SetActorHiddenInGame(CaptureSeconds >= CaptureFallEnd);
		}
		if (CaptureSeconds >= CaptureFinish)
		{
			bCaptureActive = false;
			bCaptureFinished = true;
			UE_LOG(LogLLK, Log, TEXT("Opening capture completed: four people taken through the south grille."));
		}
		return;
	}
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
