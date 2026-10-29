#include "Characters/LLKCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LLK.h"
#include "Lantern/LLKLanternComponent.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "LLKUnits.h"
#include "Storm/LLKStormChargeSubsystem.h"
#include "Net/UnrealNetwork.h"

ALLKCharacter::ALLKCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Row 44: the camera is the keeper's head, not a setting on a boom. There
	// is no spring arm and no third-person branch to keep working.
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	Lantern = CreateDefaultSubobject<ULLKLanternComponent>(TEXT("Lantern"));

	// M1.3's placeholder, attached to the camera rather than the capsule: a
	// first-person keeper's light comes from where they are looking, and row
	// 44 makes the camera the head. Slightly below and ahead of the eye so the
	// cone is not perfectly centred on the crosshair, which reads as a held
	// object rather than as a headlamp.
	LanternLight = CreateDefaultSubobject<ULLKLanternLightComponent>(TEXT("LanternLight"));
	LanternLight->SetupAttachment(FirstPersonCamera);
	LanternLight->SetRelativeLocation(FVector(20.f, 14.f, -18.f));

	// The pawn turns with the controller; the (absent) mesh does not steer it.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
		Movement->NavAgentProps.bCanCrouch = true;
		// Crouching must not silently un-crouch under a low ceiling into a
		// stand the player did not ask for; keep the capsule honest instead.
		Movement->bCanWalkOffLedgesWhenCrouching = true;
	}
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
	}

	// The character does this wiring, not the lantern component. Its header is
	// explicit that a component which knows about its listeners cannot be
	// tested without them, and this actor is the one thing that owns both the
	// lantern and a world to find the meter in.
	if (Lantern != nullptr)
	{
		if (UWorld* World = GetWorld())
		{
			if (ULLKStormChargeSubsystem* Storm = World->GetSubsystem<ULLKStormChargeSubsystem>())
			{
				Storm->RegisterLantern(Lantern);
			}
		}
	}
}

void ALLKCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// The subsystem holds weak pointers and recounts rather than remembering,
	// so a missed unregister cannot accelerate the storm. Doing it anyway
	// keeps the list from growing by one entry per death across a long night.
	if (Lantern != nullptr)
	{
		if (const UWorld* World = GetWorld())
		{
			if (ULLKStormChargeSubsystem* Storm = World->GetSubsystem<ULLKStormChargeSubsystem>())
			{
				Storm->UnregisterLantern(Lantern);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
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
	}
	ApplyMovementSpeed();
}

void ALLKCharacter::ApplyMovementSpeed()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = bSprinting ? SprintSpeedCms : WalkSpeedCms;
	}
}

void ALLKCharacter::MoveInput(const FVector2D& Axis)
{
	if (Controller == nullptr)
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
		TEXT("fallback (WASD, mouse, shift, ctrl, space, F). Nothing is rebindable until M1.6's assets exist."));
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
	// ACharacter's own jump: already server-authoritative through the movement
	// component, and nothing here needs to second-guess it.
	Jump();
}

void ALLKCharacter::OnJumpStopped()
{
	StopJumping();
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
