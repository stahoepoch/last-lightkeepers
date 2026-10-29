#include "Characters/LLKCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LLK.h"
#include "LLKUnits.h"
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
}

void ALLKCharacter::ResolveSpeeds()
{
	float WalkStuds = FallbackWalkStuds;
	float SprintStuds = FallbackSprintStuds;
	float CrouchStuds = FallbackCrouchStuds;
	SpeedSource = ELLKSpeedSource::Fallback;

	// Synchronous load is correct here and not laziness: these are three
	// floats the pawn cannot move without, and a frame of walking at the
	// wrong speed would be a frame of gameplay built on a default.
	if (const ULLKLanternData* Data = LanternData.LoadSynchronous())
	{
		WalkStuds = Data->Movement.WalkSpeed;
		SprintStuds = Data->Movement.SprintSpeed;
		CrouchStuds = Data->Movement.CrouchSpeed;
		SpeedSource = ELLKSpeedSource::LanternData;
	}

	WalkSpeedCms = LLKUnits::SpeedFromStuds(WalkStuds);
	SprintSpeedCms = LLKUnits::SpeedFromStuds(SprintStuds);
	CrouchSpeedCms = LLKUnits::SpeedFromStuds(CrouchStuds);

	if (SpeedSource == ELLKSpeedSource::Fallback)
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
