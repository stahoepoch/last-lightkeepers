#include "Characters/LLKClimbComponent.h"

#include "Characters/LLKClimbFeel.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Lantern/LLKLanternFeel.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "Net/UnrealNetwork.h"

ULLKClimbComponent::ULLKClimbComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void ULLKClimbComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULLKClimbComponent, bClimbing);
	DOREPLIFETIME(ULLKClimbComponent, Alpha);
}

float ULLKClimbComponent::ForwardReachCm()
{
	// The capsule's own radius plus half of it again as a hand's reach. Not a
	// chosen distance: a keeper must be standing AT the counter rather than
	// near it, and the body is what decides what "at" means.
	return LLKLanternFeel::KeeperCapsuleRadiusCm * 1.5f;
}

ACharacter* ULLKClimbComponent::Keeper() const
{
	return Cast<ACharacter>(GetOwner());
}

FLLKLedge ULLKClimbComponent::FindLedge() const
{
	FLLKLedge Ledge;

	const ACharacter* Character = Keeper();
	const UWorld* World = GetWorld();
	if (Character == nullptr || World == nullptr)
	{
		return Ledge;
	}

	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (Capsule == nullptr || Movement == nullptr)
	{
		return Ledge;
	}

	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float Radius = Capsule->GetScaledCapsuleRadius();
	const FVector Centre = Character->GetActorLocation();
	const FVector Feet = Centre - FVector(0.f, 0.f, HalfHeight);
	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	// **Trace down, not forward-then-down.** A forward trace to find a face
	// first would miss the whole class of surface this verb is for: a counter
	// has a lip, a gondola has a kick plate, and a shelf has nothing solid at
	// the height a chest-high forward trace would sweep. What matters is
	// whether there is a floor to stand on just in front, so the probe starts
	// above the highest reachable ledge and comes straight down.
	const float Ceiling = LLKClimbFeel::MaxLedgeHeightCm();
	const FVector Probe = Feet + Forward * ForwardReachCm();
	const FVector From = Probe + FVector(0.f, 0.f, Ceiling + 1.f);
	const FVector To = Probe;

	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, Params))
	{
		return Ledge;
	}

	const float HeightCm = Hit.ImpactPoint.Z - Feet.Z;
	if (!LLKClimbFeel::IsClimbable(HeightCm, Movement->MaxStepHeight))
	{
		return Ledge;
	}

	// **Room for the body, checked before the climb rather than discovered
	// during it.** A mantle onto a surface with a shelf 40 cm above it would
	// finish with the capsule inside the shelf, and the engine's response to
	// that is to eject the keeper somewhere neither of us chose. This is the
	// same class of defect as the crate in a recess: a correct move onto a
	// place that will not hold it.
	const FVector Destination = Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight + 1.f);
	if (World->OverlapBlockingTestByChannel(
			Destination, FQuat::Identity, ECC_Pawn,
			FCollisionShape::MakeCapsule(Radius, HalfHeight), Params))
	{
		return Ledge;
	}

	Ledge.CapsuleDestination = Destination;
	Ledge.HeightCm = HeightCm;
	Ledge.bFound = true;
	return Ledge;
}

bool ULLKClimbComponent::TryClimb()
{
	if (bClimbing)
	{
		return true;
	}

	// The client asks only when it can see a reason to, so a jump in open
	// floor costs no RPC at all. The server does not trust this: it runs the
	// same traces again in `ServerStartClimb`.
	if (!FindLedge().bFound)
	{
		return false;
	}

	ServerStartClimb();
	return true;
}

void ULLKClimbComponent::ServerStartClimb_Implementation()
{
	ACharacter* Character = Keeper();
	if (Character == nullptr || !Character->HasAuthority() || bClimbing)
	{
		return;
	}

	// Re-traced on the server. A client that could name its own destination
	// could name the top of anything, which is the rule every other server
	// entry point in this project already states.
	const FLLKLedge Ledge = FindLedge();
	if (!Ledge.bFound)
	{
		return;
	}

	ClimbStart = Character->GetActorLocation();
	ClimbEnd = Ledge.CapsuleDestination;
	ClimbDuration = LLKClimbFeel::ClimbSeconds(Ledge.HeightCm);
	Alpha = 0.f;
	bClimbing = true;

	// **Flying for the duration, deliberately.** Walking movement would fight
	// the mantle every frame - gravity pulling the capsule back down the face
	// of the counter while the interpolation pulls it up - and the keeper
	// would arrive late, or not at all, depending on frame rate.
	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->SetMovementMode(MOVE_Flying);
	}

	UE_LOG(LogLLK, Log,
		TEXT("%s is climbing a %.0f cm ledge: %.2f s at %.0f studs/s, and %.0f studs loud."),
		*Character->GetName(), Ledge.HeightCm, ClimbDuration,
		LLKClimbFeel::ClimbStudsPerSecond, LLKClimbFeel::ClimbNoiseStuds);
}

void ULLKClimbComponent::TickComponent(
	const float DeltaTime, const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AActor* Owner = GetOwner();
	if (Owner != nullptr && Owner->HasAuthority())
	{
		TickClimb(DeltaTime);
	}
}

void ULLKClimbComponent::TickClimb(const float DeltaSeconds)
{
	if (!bClimbing)
	{
		return;
	}

	ACharacter* Character = Keeper();
	if (Character == nullptr)
	{
		bClimbing = false;
		return;
	}

	// A zero duration would divide by nothing and finish instantly; a ledge
	// of zero height cannot reach here, but a retune to a huge climb speed
	// could, and a teleport is worse than a fast climb.
	Alpha = ClimbDuration > KINDA_SMALL_NUMBER
		? FMath::Clamp(Alpha + DeltaSeconds / ClimbDuration, 0.f, 1.f)
		: 1.f;

	Character->SetActorLocation(
		LLKClimbFeel::MantlePosition(ClimbStart, ClimbEnd, Alpha),
		false, nullptr, ETeleportType::TeleportPhysics);

	if (Alpha < 1.f)
	{
		return;
	}

	bClimbing = false;
	Alpha = 0.f;
	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}
}

bool ULLKClimbComponent::GetCurrentNoise(LLKHearing::FLLKNoise& OutNoise) const
{
	if (!bClimbing)
	{
		return false;
	}

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	// **The reason this component is a noise source at all.** The mantle moves
	// the capsule with `SetActorLocation`, so the keeper's velocity is zero
	// throughout and `LLKHearing::NoiseFromMovement` would report silence for
	// the whole climb. The most exposed second in the chapter would be the
	// quietest thing in it, which is exactly the shape of defect this project
	// keeps finding: a correct system over an absent thing.
	OutNoise.Position = Owner->GetActorLocation();
	OutNoise.RadiusStuds = LLKClimbFeel::ClimbNoiseStuds;
	return true;
}
