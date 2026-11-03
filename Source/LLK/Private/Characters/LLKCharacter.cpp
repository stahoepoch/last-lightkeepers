#include "Characters/LLKCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LLK.h"
#include "Lantern/LLKLanternComponent.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "Characters/LLKVitalsComponent.h"
#include "Loot/LLKInteractorComponent.h"
#include "Loot/LLKBurden.h"
#include "Run/LLKRunStateSubsystem.h"
#include "LLKUnits.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "Net/UnrealNetwork.h"

ALLKCharacter::ALLKCharacter()
{
	// Only drives the visible local lantern assembly below. Gameplay still
	// samples LanternAnchor, which remains stable beneath the camera.
	PrimaryActorTick.bCanEverTick = true;

	// Row 44: the camera is the keeper's head, not a setting on a boom. There
	// is no spring arm and no third-person branch to keep working.
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// The UEFN mannequin is intentionally a co-op presentation body for now,
	// not a dishonest substitute for dedicated first-person arms. The owning
	// keeper never sees a torso clipping through their camera, while another
	// keeper can finally see an actual body carrying the shared lantern.
	//
	// This is optional presentation: movement, interaction and the gameplay
	// light field still run if the asset cannot resolve in a clean code-only
	// checkout. A proper first-person arm rig will replace only the local view.
	const ConstructorHelpers::FObjectFinder<USkeletalMesh> KeeperBody(
		TEXT("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin.SKM_UEFN_Mannequin"));
	if (KeeperBody.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(KeeperBody.Object);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -89.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		GetMesh()->SetOwnerNoSee(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// One honest carried anchor: the mesh and the rendered cone live here, and
	// the light component reports this transform to gameplay. Cosmetic sway is
	// applied below this component, so a hand-feel pass cannot quietly move a
	// relic-read or enemy-perception source.
	LanternAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("LanternAnchor"));
	LanternAnchor->SetupAttachment(FirstPersonCamera);
	// A hand lantern belongs low and to the side of the view.  The old pose was
	// almost eye-height and centred, so the temporary model read as a helmet
	// lamp while blocking the scene it was meant to illuminate.
	LanternAnchor->SetRelativeLocation(FVector(42.f, 25.f, -19.f));

	// This exists only so another keeper sees an instrument in a hand instead
	// of a light emerging from a camera. It must stay separate from
	// LanternAnchor: the latter is the stable gameplay source, and mesh pose
	// must never alter a relic read or an enemy's perception calculation.
	RemoteLanternAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("RemoteLanternAnchor"));
	RemoteLanternAnchor->SetupAttachment(GetMesh(), TEXT("palm_r_Socket"));
	RemoteLanternAnchor->SetRelativeLocation(FVector(6.f, 0.f, -12.f));
	RemoteLanternAnchor->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	Lantern = CreateDefaultSubobject<ULLKLanternComponent>(TEXT("Lantern"));

	// M1.3's placeholder, attached to the camera rather than the capsule: a
	// first-person keeper's light comes from where they are looking, and row
	// 44 makes the camera the head. Slightly below and ahead of the eye so the
	// cone is not perfectly centred on the crosshair, which reads as a held
	// object rather than as a headlamp.
	LanternLight = CreateDefaultSubobject<ULLKLanternLightComponent>(TEXT("LanternLight"));
	LanternLight->SetupAttachment(LanternAnchor);

	// A visible instrument before authored art exists. This is intentionally
	// simple hard-surface assembly, not a disguised final asset: the player
	// needs a base, glass chamber, cage and handle in frame so the light reads
	// as carried equipment rather than a camera flashlight. Every part remains
	// below LanternAnchor, the one position gameplay reads.
	const ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	const ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	// The core material has always existed, but the temporary lantern assembly
	// never assigned it to its glass chamber.  That left a point light floating
	// inside an untextured engine cylinder: technically illuminated, but not an
	// object the eye could believe was supplying the light.
	const ConstructorHelpers::FObjectFinder<UMaterialInterface> LanternCoreMaterial(
		TEXT("/Game/LLK/Materials/MI_LLK_LanternCore_Working.MI_LLK_LanternCore_Working"));

	auto ConfigureLanternPart = [](UStaticMeshComponent* Part, USceneComponent* Parent,
		const FVector& Location, const FVector& Scale, UStaticMesh* Shape, bool bOnlyOwner)
	{
		Part->SetupAttachment(Parent);
		Part->SetRelativeLocation(Location);
		Part->SetRelativeScale3D(Scale);
		Part->SetStaticMesh(Shape);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetOnlyOwnerSee(bOnlyOwner);
		Part->SetOwnerNoSee(!bOnlyOwner);
		// The carried source must not cast a shadow through the player's own
		// camera. The world light remains shadow-casting.
		Part->SetCastShadow(false);
	};

	LanternGlass = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternGlass_Temporary"));
	LanternTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternTop_Temporary"));
	LanternBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternBase_Temporary"));
	LanternCageLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternCageLeft_Temporary"));
	LanternCageRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternCageRight_Temporary"));
	LanternHandle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternHandle_Temporary"));

	RemoteLanternGlass = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteLanternGlass_Temporary"));
	RemoteLanternTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteLanternTop_Temporary"));
	RemoteLanternBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteLanternBase_Temporary"));
	RemoteLanternCageLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteLanternCageLeft_Temporary"));
	RemoteLanternCageRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteLanternCageRight_Temporary"));
	RemoteLanternHandle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteLanternHandle_Temporary"));

	auto BuildLanternSilhouette = [&ConfigureLanternPart, Cylinder, Cube, LanternCoreMaterial](USceneComponent* Parent,
		UStaticMeshComponent* Glass, UStaticMeshComponent* Top, UStaticMeshComponent* Base,
		UStaticMeshComponent* CageLeft, UStaticMeshComponent* CageRight, UStaticMeshComponent* Handle,
		bool bOnlyOwner)
	{
		// Engine primitives are one metre across.  These dimensions make a
		// roughly 18 cm wide, 32 cm tall service lantern rather than a barrel.
		ConfigureLanternPart(Glass, Parent, FVector(0.f, 0.f, -4.f), FVector(0.08f, 0.08f, 0.13f), Cylinder.Object, bOnlyOwner);
		if (LanternCoreMaterial.Succeeded())
		{
			Glass->SetMaterial(0, LanternCoreMaterial.Object);
		}
		ConfigureLanternPart(Top, Parent, FVector(0.f, 0.f, 4.f), FVector(0.10f, 0.10f, 0.03f), Cylinder.Object, bOnlyOwner);
		ConfigureLanternPart(Base, Parent, FVector(0.f, 0.f, -12.f), FVector(0.10f, 0.10f, 0.035f), Cylinder.Object, bOnlyOwner);
		ConfigureLanternPart(CageLeft, Parent, FVector(0.f, -7.f, -4.f), FVector(0.017f, 0.015f, 0.145f), Cube.Object, bOnlyOwner);
		ConfigureLanternPart(CageRight, Parent, FVector(0.f, 7.f, -4.f), FVector(0.017f, 0.015f, 0.145f), Cube.Object, bOnlyOwner);
		ConfigureLanternPart(Handle, Parent, FVector(0.f, 0.f, 14.f), FVector(0.05f, 0.02f, 0.016f), Cube.Object, bOnlyOwner);
	};

	BuildLanternSilhouette(LanternAnchor, LanternGlass, LanternTop, LanternBase,
		LanternCageLeft, LanternCageRight, LanternHandle, true);
	BuildLanternSilhouette(RemoteLanternAnchor, RemoteLanternGlass, RemoteLanternTop, RemoteLanternBase,
		RemoteLanternCageLeft, RemoteLanternCageRight, RemoteLanternHandle, false);

	LanternCoreGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("LanternCoreGlow_Temporary"));
	LanternCoreGlow->SetupAttachment(LanternAnchor);
	LanternCoreGlow->SetRelativeLocation(FVector(0.f, 0.f, -4.f));
	LanternCoreGlow->IntensityUnits = ELightUnits::Candelas;
	LanternCoreGlow->SetIntensity(15.f);
	LanternCoreGlow->SetAttenuationRadius(160.f);
	LanternCoreGlow->SetLightColor(FLinearColor(1.f, 0.45f, 0.08f));
	LanternCoreGlow->SetCastShadows(false);
	LanternCoreGlow->SetVisibility(false);

	RemoteLanternCoreGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("RemoteLanternCoreGlow_Temporary"));
	RemoteLanternCoreGlow->SetupAttachment(RemoteLanternAnchor);
	RemoteLanternCoreGlow->SetRelativeLocation(FVector(0.f, 0.f, -4.f));
	RemoteLanternCoreGlow->IntensityUnits = ELightUnits::Candelas;
	RemoteLanternCoreGlow->SetIntensity(15.f);
	RemoteLanternCoreGlow->SetAttenuationRadius(160.f);
	RemoteLanternCoreGlow->SetLightColor(FLinearColor(1.f, 0.45f, 0.08f));
	RemoteLanternCoreGlow->SetCastShadows(false);
	RemoteLanternCoreGlow->SetVisibility(false);

	Interactor = CreateDefaultSubobject<ULLKInteractorComponent>(TEXT("Interactor"));
	Vitals = CreateDefaultSubobject<ULLKVitalsComponent>(TEXT("Vitals"));

	// The pawn turns with the controller; the (absent) mesh does not steer it.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		// **Set in the CONSTRUCTOR, not at BeginPlay, and that was a real bug.**
		// These first went into ResolveSpeeds, which runs at BeginPlay - so the
		// class default object still reported Unreal's stock 420 at gravity 1,
		// the editor showed values the game did not use, and
		// LLK.Movement.JumpWeight failed every assertion while the running game
		// was correct. Static tuning belongs where the component is made; only
		// data-resolved values belong in ResolveSpeeds.
		Movement->JumpZVelocity = LLKUnits::SpeedFromStuds(JumpStuds);
		Movement->GravityScale = GravityScale;

		Movement->bOrientRotationToMovement = false;
		Movement->NavAgentProps.bCanCrouch = true;
		// Crouching must not silently un-crouch under a low ceiling into a
		// stand the player did not ask for; keep the capsule honest instead.
		Movement->bCanWalkOffLedgesWhenCrouching = true;
	}
}

void ALLKCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// A deliberately small walk response makes the temporary hard-surface rig
	// read as carried equipment. It is presentation-only: no component in this
	// function is read by ULLKLanternLightComponent or the gameplay light field.
	if (!IsLocallyControlled())
	{
		return;
	}

	LanternPresentationSeconds += DeltaSeconds;
	const float Speed01 = GetVelocity().Size2D() / 448.f;
	const float Amount = FMath::Clamp(Speed01, 0.f, 1.f);
	const float Side = FMath::Sin(LanternPresentationSeconds * 7.f) * 1.2f * Amount;
	const float Lift = FMath::Abs(FMath::Cos(LanternPresentationSeconds * 7.f)) * 1.6f * Amount;

	auto SetPresentationLocation = [Side, Lift](UStaticMeshComponent* Part, const FVector& Rest)
	{
		if (Part != nullptr)
		{
			Part->SetRelativeLocation(Rest + FVector(0.f, Side, Lift));
		}
	};

	SetPresentationLocation(LanternGlass, FVector(0.f, 0.f, -4.f));
	SetPresentationLocation(LanternTop, FVector(0.f, 0.f, 4.f));
	SetPresentationLocation(LanternBase, FVector(0.f, 0.f, -12.f));
	SetPresentationLocation(LanternCageLeft, FVector(0.f, -7.f, -4.f));
	SetPresentationLocation(LanternCageRight, FVector(0.f, 7.f, -4.f));
	SetPresentationLocation(LanternHandle, FVector(0.f, 0.f, 14.f));
}

void ALLKCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Everyone sees everyone's sprint: co-op players read each other's state,
	// and a lantern-lit crew needs to see who is about to outrun them.
	DOREPLIFETIME(ALLKCharacter, bSprinting);
}

void ALLKCharacter::BeginPlay()
{
	Super::BeginPlay();
	ResolveSpeeds();

	// The light subscribes to the lantern; the lantern knows nothing about
	// the light. Done here rather than in the constructor because a CDO has
	// no delegates worth binding and because the lantern may already have
	// replicated a state in by now, which WatchLantern matches.
	if (LanternLight != nullptr && Lantern != nullptr)
	{
		// One asset, pushed down rather than set twice. The light keeps its own
		// LanternData so it can live on an actor that is not a keeper, but a
		// keeper's light must read the same data its speeds did - two soft
		// pointers a designer can set independently is a light that disagrees
		// with the movement about which file it came from.
		LanternLight->SetLanternData(LanternData);
		LanternLight->WatchLantern(Lantern);
		Lantern->OnLanternStateChanged.AddDynamic(this, &ALLKCharacter::OnLanternVisualStateChanged);
		OnLanternVisualStateChanged(Lantern->IsLanternOn(), ELLKLanternChangeReason::Replicated);
	}

	if (Interactor != nullptr)
	{
		// Burden, bound the same way everything else here is: the component
		// announces, the character decides what it means for movement. The
		// interactor knows nothing about speed.
		Interactor->OnCarriedCountChanged.AddDynamic(this, &ALLKCharacter::OnCarriedCountChanged);
	}

	// Once at the start, so the crouch speed and the walk speed are burdened
	// from the first frame rather than from the first time something changed.
	// `ResolveSpeeds` sets the authored numbers; this is where the haul is
	// taken off them, and a keeper who loads in already carrying - which a
	// rejoin will be - must not be fast until they pick something up.
	ApplyMovementSpeed();

	if (Vitals != nullptr)
	{
		// Speed and sprint have to react to the state, and the character owns
		// both. The component decides whether the keeper is down; what that
		// does to movement is movement's business.
		Vitals->OnDownedChanged.AddDynamic(this, &ALLKCharacter::OnDownedChanged);

		// The character does the registering, as with the lantern and the
		// storm: the vitals component stays ignorant of who is watching it.
		if (UWorld* World = GetWorld())
		{
			if (ULLKRunStateSubsystem* RunState = World->GetSubsystem<ULLKRunStateSubsystem>())
			{
				RunState->RegisterKeeper(Vitals);
			}
		}
	}

	// The character does this wiring, not the lantern component. Its header is
	// explicit that a component which knows about its listeners cannot be
	// tested without them, and this actor is the one thing that owns both the
	// lantern and a world to find the meter in.
	if (Lantern != nullptr)
	{
		if (UWorld* World = GetWorld())
		{
			if (ULLKZonePressureSubsystem* Storm = World->GetSubsystem<ULLKZonePressureSubsystem>())
			{
				Storm->RegisterLantern(Lantern);
			}
		}
	}
}

void ALLKCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Lantern != nullptr)
	{
		Lantern->OnLanternStateChanged.RemoveDynamic(this, &ALLKCharacter::OnLanternVisualStateChanged);
	}

	// The subsystem holds weak pointers and recounts rather than remembering,
	// so a missed unregister cannot accelerate the storm. Doing it anyway
	// keeps the list from growing by one entry per death across a long night.
	if (Lantern != nullptr)
	{
		if (const UWorld* World = GetWorld())
		{
			if (ULLKZonePressureSubsystem* Storm = World->GetSubsystem<ULLKZonePressureSubsystem>())
			{
				Storm->UnregisterLantern(Lantern);
			}
		}
	}

	if (Vitals != nullptr)
	{
		if (const UWorld* World = GetWorld())
		{
			if (ULLKRunStateSubsystem* RunState = World->GetSubsystem<ULLKRunStateSubsystem>())
			{
				// A keeper who leaves is not a keeper who cannot act. Without
				// this, one player quitting a four-player run would leave a
				// stale entry that never stands up and never wipes.
				RunState->UnregisterKeeper(Vitals);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ALLKCharacter::OnLanternVisualStateChanged(bool bIsOn, ELLKLanternChangeReason Reason)
{
	// Cosmetic only. The gameplay field is still owned by LanternLight and
	// resolves through LanternAnchor; this tiny fill cannot make a relic
	// legible or change an enemy's reach.
	// Point lights do not have mesh-style owner visibility flags. Resolve this
	// locally instead so an owner gets only the first-person glow while every
	// other client sees only the hand-mounted glow on this keeper.
	const bool bOwnsThisKeeper = IsLocallyControlled();
	if (LanternCoreGlow != nullptr)
	{
		LanternCoreGlow->SetVisibility(bIsOn && bOwnsThisKeeper);
	}

	if (RemoteLanternCoreGlow != nullptr)
	{
		RemoteLanternCoreGlow->SetVisibility(bIsOn && !bOwnsThisKeeper);
	}
}

void ALLKCharacter::ResolveSpeeds()
{
	float WalkStuds = FallbackWalkStuds;
	float SprintStuds = FallbackSprintStuds;
	float CrouchStuds = FallbackCrouchStuds;
	SpeedSource = ELLKDataSource::Fallback;

	// Synchronous load is correct here and not laziness: these are three
	// floats the pawn cannot move without, and a frame of walking at the
	// wrong speed would be a frame of gameplay built on a default.
	if (const ULLKLanternData* Data = LanternData.LoadSynchronous())
	{
		WalkStuds = Data->Movement.WalkSpeed;
		SprintStuds = Data->Movement.SprintSpeed;
		CrouchStuds = Data->Movement.CrouchSpeed;
		SpeedSource = ELLKDataSource::Asset;
	}

	// Burden's own rules, from `loot.json` rather than from the lantern's
	// file, because carrying is loot's business. A missing asset leaves the
	// struct's defaults, which are zeros - burden off, which is exactly what
	// every run before M3 was playing, and the relic component already warns
	// loudly about the same asset being absent.
	if (const ULLKLootRulesData* Loot = LootRules.LoadSynchronous())
	{
		BurdenRules = Loot->Burden;
	}

	WalkSpeedCms = LLKUnits::SpeedFromStuds(WalkStuds);
	SprintSpeedCms = LLKUnits::SpeedFromStuds(SprintStuds);
	CrouchSpeedCms = LLKUnits::SpeedFromStuds(CrouchStuds);

	if (SpeedSource == ELLKDataSource::Fallback)
	{
		// Loud on purpose. `docs/DATA-PIPELINE.md` exists so gameplay reads
		// the data rather than a constant that happens to match it today, and
		// a quiet fallback is exactly how that promise stops being true.
		UE_LOG(LogLLK, Warning,
			TEXT("ALLKCharacter: no ULLKLanternData resolved - running on DECLARED FALLBACK speeds ")
			TEXT("(walk %.0f, sprint %.0f, crouch %.0f cm/s). Run Tools/import_data.py and set LanternData."),
			WalkSpeedCms, SprintSpeedCms, CrouchSpeedCms);
	}
	else
	{
		UE_LOG(LogLLK, Log,
			TEXT("ALLKCharacter: speeds from ULLKLanternData - walk %.0f, sprint %.0f, crouch %.0f cm/s."),
			WalkSpeedCms, SprintSpeedCms, CrouchSpeedCms);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeedCrouched = CrouchSpeedCms;

		// Re-applied here so an instance override still lands, and logged so
		// the numbers Chris is reacting to are in the file he sends me.
		Movement->JumpZVelocity = LLKUnits::SpeedFromStuds(JumpStuds);
		Movement->GravityScale = GravityScale;

		UE_LOG(LogLLK, Log,
			TEXT("ALLKCharacter: jump %.0f cm/s at gravity x%.2f - %.0fcm apex over %.2fs."),
			Movement->JumpZVelocity, GravityScale, GetJumpApexCm(), GetJumpAirtimeSeconds());
	}
	ApplyMovementSpeed();
}

float ALLKCharacter::GetJumpVelocity() const
{
	return LLKUnits::SpeedFromStuds(JumpStuds);
}

float ALLKCharacter::GetJumpApexCm() const
{
	// Read from the movement component where possible rather than recomputed,
	// so this reports what the keeper actually does and not what the
	// properties say they should. The two differ the moment anything else
	// touches gravity.
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	const float Gravity = Movement != nullptr
		? FMath::Abs(Movement->GetGravityZ()) : 980.f * GravityScale;
	const float Velocity = Movement != nullptr ? Movement->JumpZVelocity : GetJumpVelocity();

	if (Gravity <= 0.f)
	{
		return 0.f;
	}
	return (Velocity * Velocity) / (2.f * Gravity);
}

float ALLKCharacter::GetJumpAirtimeSeconds() const
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	const float Gravity = Movement != nullptr
		? FMath::Abs(Movement->GetGravityZ()) : 980.f * GravityScale;
	const float Velocity = Movement != nullptr ? Movement->JumpZVelocity : GetJumpVelocity();

	if (Gravity <= 0.f)
	{
		return 0.f;
	}
	return (2.f * Velocity) / Gravity;
}

bool ALLKCharacter::IsDowned() const
{
	return Vitals != nullptr && Vitals->IsDowned();
}

float ALLKCharacter::GetBurdenMultiplier() const
{
	// The points are relics carried. Chris's note in `loot.json` is narrower
	// than the key names: *"Only relics impose a small movement penalty;
	// supplies, components, and restoration crates do not."* The interactor's
	// count is a count of relics taken, so it is already the right number.
	const int32 Points = Interactor != nullptr ? Interactor->GetCarriedCount() : 0;
	return LLKBurden::SpeedMultiplier(Points, BurdenRules);
}

void ALLKCharacter::OnCarriedCountChanged(int32 CarriedCount)
{
	// The haul changed, so the speed has. Event-driven rather than polled:
	// this happens a handful of times in a run, and `ApplyMovementSpeed` is
	// the one place a speed is ever set.
	ApplyMovementSpeed();
}

void ALLKCharacter::ApplyMovementSpeed()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		// Applied to every speed rather than to walking alone, so a burdened
		// keeper cannot sprint away from a cost they accepted - and applied
		// here, in the single function that assigns a speed, so there is no
		// path to a movement value that forgot about the haul.
		const float Burden = GetBurdenMultiplier();
		Movement->MaxWalkSpeedCrouched = CrouchSpeedCms * Burden;

		if (IsDowned())
		{
			// `downedWalkSpeed: 0` converted through LLKUnits like every other
			// speed. At zero this is a keeper who cannot crawl, which is the
			// authored value and the thing that pins a carried crate to a
			// body (row 59).
			Movement->MaxWalkSpeed = LLKUnits::SpeedFromStuds(Vitals->GetDownedWalkStuds());
			return;
		}

		Movement->MaxWalkSpeed = (bSprinting ? SprintSpeedCms : WalkSpeedCms) * Burden;
	}
}

void ALLKCharacter::OnDownedChanged(bool bIsDowned, ELLKDownedReason Reason)
{
	if (bIsDowned && bSprinting)
	{
		// Sprint is a replicated intent and it does not survive the floor.
		// Left set, a keeper revived later would stand up already sprinting
		// without having asked to.
		bSprinting = false;
	}

	if (bIsDowned && bIsCrouched)
	{
		// ACharacter's own crouch, released so the capsule is not left short
		// under a keeper who is no longer choosing anything.
		UnCrouch();
	}

	ApplyMovementSpeed();
}

void ALLKCharacter::LLKDown()
{
	ServerSetDownedForDebug(true);
}

void ALLKCharacter::LLKUp()
{
	ServerSetDownedForDebug(false);
}

void ALLKCharacter::ServerSetDownedForDebug_Implementation(bool bDown)
{
	if (Vitals == nullptr)
	{
		return;
	}

	if (bDown)
	{
		Vitals->GoDown(ELLKDownedReason::Debug);
	}
	else
	{
		Vitals->StandUp(ELLKDownedReason::Debug);
	}
}

void ALLKCharacter::MoveInput(const FVector2D& Axis)
{
	if (Controller == nullptr)
	{
		return;
	}

	// Gated here rather than relying on a zero speed alone. MaxWalkSpeed of
	// zero stops the keeper travelling, but AddMovementInput still turns the
	// capsule and still reads as the game accepting input it is going to
	// ignore - which feels like a bug rather than like being pinned.
	if (IsDowned())
	{
		return;
	}

	// Yaw only: pitch must never tilt the floor the keeper walks on.
	const FRotator YawOnly(0.f, Controller->GetControlRotation().Yaw, 0.f);
	if (!FMath::IsNearlyZero(Axis.X))
	{
		AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X), Axis.X);
	}
	if (!FMath::IsNearlyZero(Axis.Y))
	{
		AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y), Axis.Y);
	}
}

void ALLKCharacter::LookInput(const FVector2D& Axis)
{
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ALLKCharacter::SetSprinting(bool bWantsToSprint)
{
	if (bWantsToSprint && IsDowned())
	{
		return;
	}

	// The client asks. It does not decide, and it does not set its own speed
	// first and tell the server afterwards - that is the shape that makes a
	// speed hack a one-line change.
	if (!HasAuthority())
	{
		ServerSetSprinting(bWantsToSprint);
		return;
	}

	if (bSprinting != bWantsToSprint)
	{
		bSprinting = bWantsToSprint;
		ApplyMovementSpeed();
	}
}

void ALLKCharacter::ServerSetSprinting_Implementation(bool bWantsToSprint)
{
	SetSprinting(bWantsToSprint);
}

void ALLKCharacter::OnRep_Sprinting()
{
	ApplyMovementSpeed();
}

void ALLKCharacter::SetCrouching(bool bWantsToCrouch)
{
	if (IsDowned())
	{
		return;
	}

	// ACharacter's crouch is already server-authoritative and replicated via
	// bIsCrouched. Building a second crouch beside it would mean two sources
	// of truth for one piece of state, which is the bug this project keeps
	// finding in its own paper trail.
	if (bWantsToCrouch)
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

// ------------------------------------------------------------------- input
//
// Assets win whenever they exist. The code path below is a stopgap so the
// keeper can be walked tonight without anybody writing into `Content/`, and
// it is built to be deleted: when M1.6's Input Actions land, they resolve,
// `InputSource` reports `MappingContextAsset`, and none of this runs.

FLLKInputBindings ALLKCharacter::BuildTransientBindings(UObject* Outer)
{
	FLLKInputBindings Bindings;

	const auto MakeAction = [Outer](const TCHAR* Name, EInputActionValueType Type)
	{
		UInputAction* Action = NewObject<UInputAction>(Outer, Name);
		Action->ValueType = Type;
		return Action;
	};

	Bindings.Move = MakeAction(TEXT("IA_LLK_Move_Transient"), EInputActionValueType::Axis2D);
	Bindings.Look = MakeAction(TEXT("IA_LLK_Look_Transient"), EInputActionValueType::Axis2D);
	Bindings.Sprint = MakeAction(TEXT("IA_LLK_Sprint_Transient"), EInputActionValueType::Boolean);
	Bindings.CrouchToggle = MakeAction(TEXT("IA_LLK_Crouch_Transient"), EInputActionValueType::Boolean);
	Bindings.Jump = MakeAction(TEXT("IA_LLK_Jump_Transient"), EInputActionValueType::Boolean);
	Bindings.LanternToggle = MakeAction(TEXT("IA_LLK_Lantern_Transient"), EInputActionValueType::Boolean);
	Bindings.Interact = MakeAction(TEXT("IA_LLK_Interact_Transient"), EInputActionValueType::Boolean);

	Bindings.Context = NewObject<UInputMappingContext>(Outer, TEXT("IMC_LLK_Transient"));

	// A key press gives 1.0 on X. Forward is X, right is Y - the same axes
	// ALLKCharacter::MoveInput expects - so S negates X, and A/D swizzle into
	// Y with A negated.
	const auto Negate = [Outer](bool bX, bool bY)
	{
		UInputModifierNegate* Modifier = NewObject<UInputModifierNegate>(Outer);
		Modifier->bX = bX;
		Modifier->bY = bY;
		Modifier->bZ = false;
		return Modifier;
	};
	const auto Swizzle = [Outer]()
	{
		UInputModifierSwizzleAxis* Modifier = NewObject<UInputModifierSwizzleAxis>(Outer);
		Modifier->Order = EInputAxisSwizzle::YXZ;
		return Modifier;
	};

	Bindings.Context->MapKey(Bindings.Move, EKeys::W);
	Bindings.Context->MapKey(Bindings.Move, EKeys::S).Modifiers.Add(Negate(true, false));

	FEnhancedActionKeyMapping& Right = Bindings.Context->MapKey(Bindings.Move, EKeys::D);
	Right.Modifiers.Add(Swizzle());

	FEnhancedActionKeyMapping& Left = Bindings.Context->MapKey(Bindings.Move, EKeys::A);
	Left.Modifiers.Add(Swizzle());
	// Swizzle runs first, moving the key value from X onto Y. Negating X
	// after that made A numerically identical to D.
	Left.Modifiers.Add(Negate(false, true));

	// Mouse2D is (dX, dY). Pitch is negated so that pushing the mouse forward
	// looks up, which is the convention every other game has already taught
	// the player.
	FEnhancedActionKeyMapping& Look = Bindings.Context->MapKey(Bindings.Look, EKeys::Mouse2D);
	Look.Modifiers.Add(Negate(false, true));

	Bindings.Context->MapKey(Bindings.Sprint, EKeys::LeftShift);
	Bindings.Context->MapKey(Bindings.CrouchToggle, EKeys::LeftControl);
	Bindings.Context->MapKey(Bindings.Jump, EKeys::SpaceBar);
	// F, because every game the player has already played put the flashlight
	// there. The lantern is the one key in this scheme that is not a movement
	// verb, and it is the one the whole game is named after.
	Bindings.Context->MapKey(Bindings.LanternToggle, EKeys::F);
	// E, because every game the player has already played put "use the thing
	// in front of you" there. Chris's reference was Arc Raiders, which does
	// the same.
	Bindings.Context->MapKey(Bindings.Interact, EKeys::E);

	return Bindings;
}

void ALLKCharacter::ResolveInputBindings()
{
	FLLKInputBindings FromAssets;
	FromAssets.Context = MappingContextAsset.LoadSynchronous();
	FromAssets.Move = MoveActionAsset.LoadSynchronous();
	FromAssets.Look = LookActionAsset.LoadSynchronous();
	FromAssets.Sprint = SprintActionAsset.LoadSynchronous();
	FromAssets.CrouchToggle = CrouchActionAsset.LoadSynchronous();
	FromAssets.Jump = JumpActionAsset.LoadSynchronous();
	FromAssets.LanternToggle = LanternActionAsset.LoadSynchronous();
	FromAssets.Interact = InteractActionAsset.LoadSynchronous();

	if (FromAssets.IsComplete())
	{
		ActiveBindings = FromAssets;
		InputSource = ELLKInputSource::MappingContextAsset;
		UE_LOG(LogLLK, Log, TEXT("ALLKCharacter: input from Input Action assets."));
		return;
	}

	ActiveBindings = BuildTransientBindings(this);
	InputSource = ELLKInputSource::TransientFallback;
	UE_LOG(LogLLK, Warning,
		TEXT("ALLKCharacter: no complete Input Action asset set - running on the TRANSIENT in-code ")
		TEXT("fallback (WASD, mouse, shift, ctrl, space, F, E). Nothing is rebindable until M1.6's ")
		TEXT("assets exist - and the authored set now needs a SEVENTH action, IA_LLK_Interact."));
}

void ALLKCharacter::ApplyMappingContext()
{
	if (!ActiveBindings.IsComplete())
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(ActiveBindings.Context, 0);
	}
}

void ALLKCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (InputSource == ELLKInputSource::Unresolved)
	{
		ResolveInputBindings();
	}
	ApplyMappingContext();
}

void ALLKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (InputSource == ELLKInputSource::Unresolved)
	{
		ResolveInputBindings();
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (Input == nullptr || !ActiveBindings.IsComplete())
	{
		// Loud rather than a dead keeper nobody can explain.
		UE_LOG(LogLLK, Error,
			TEXT("ALLKCharacter: no UEnhancedInputComponent or incomplete bindings - the keeper will not move."));
		return;
	}

	Input->BindAction(ActiveBindings.Move, ETriggerEvent::Triggered, this, &ALLKCharacter::OnMoveInput);
	Input->BindAction(ActiveBindings.Look, ETriggerEvent::Triggered, this, &ALLKCharacter::OnLookInput);
	Input->BindAction(ActiveBindings.Sprint, ETriggerEvent::Started, this, &ALLKCharacter::OnSprintStarted);
	Input->BindAction(ActiveBindings.Sprint, ETriggerEvent::Completed, this, &ALLKCharacter::OnSprintStopped);
	Input->BindAction(ActiveBindings.CrouchToggle, ETriggerEvent::Started, this, &ALLKCharacter::OnCrouchStarted);
	Input->BindAction(ActiveBindings.CrouchToggle, ETriggerEvent::Completed, this, &ALLKCharacter::OnCrouchStopped);
	Input->BindAction(ActiveBindings.Jump, ETriggerEvent::Started, this, &ALLKCharacter::OnJumpStarted);
	Input->BindAction(ActiveBindings.Jump, ETriggerEvent::Completed, this, &ALLKCharacter::OnJumpStopped);
	// Started only. A lantern you have to hold down to keep lit would make
	// row 5's trade a matter of finger stamina instead of a decision.
	Input->BindAction(ActiveBindings.LanternToggle, ETriggerEvent::Started, this, &ALLKCharacter::OnLanternToggled);
	Input->BindAction(ActiveBindings.Interact, ETriggerEvent::Started, this, &ALLKCharacter::OnInteract);
	// Released as well as pressed, because a revive is a HOLD: letting go has
	// to cancel it, which is what makes reviving a commitment of attention
	// rather than a tap.
	Input->BindAction(ActiveBindings.Interact, ETriggerEvent::Completed, this, &ALLKCharacter::OnInteractReleased);
}

void ALLKCharacter::OnMoveInput(const FInputActionValue& Value)
{
	MoveInput(Value.Get<FVector2D>());
}

void ALLKCharacter::OnLookInput(const FInputActionValue& Value)
{
	LookInput(Value.Get<FVector2D>());
}

void ALLKCharacter::OnSprintStarted()
{
	SetSprinting(true);
}

void ALLKCharacter::OnSprintStopped()
{
	SetSprinting(false);
}

void ALLKCharacter::OnCrouchStarted()
{
	SetCrouching(true);
}

void ALLKCharacter::OnCrouchStopped()
{
	SetCrouching(false);
}

void ALLKCharacter::OnJumpStarted()
{
	if (IsDowned())
	{
		return;
	}

	// ACharacter's own jump: already server-authoritative through the movement
	// component, and nothing here needs to second-guess it.
	Jump();
}

void ALLKCharacter::OnJumpStopped()
{
	StopJumping();
}

void ALLKCharacter::OnInteract()
{
	// **No longer gated on being downed, and that is M2.2's doing.** A keeper
	// on the floor still cannot take things off shelves - the interactor
	// refuses that - but they CAN spend their own anchor revive, which is the
	// one action a contained keeper has. The gate moved from "a downed keeper
	// does nothing" to "a downed keeper may do exactly one thing", and it
	// lives in the interactor where it can tell the two apart.
	if (Interactor != nullptr)
	{
		Interactor->BeginInteract();
	}
}

void ALLKCharacter::OnInteractReleased()
{
	if (Interactor != nullptr)
	{
		Interactor->EndInteract();
	}
}

void ALLKCharacter::OnLanternToggled()
{
	if (Lantern != nullptr)
	{
		// Asks. The server decides and replicates, and the placeholder light
		// turns on because it heard the broadcast - not because this key was
		// pressed on this machine.
		Lantern->ToggleLantern();
	}
}
