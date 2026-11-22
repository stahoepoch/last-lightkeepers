#include "World/LLKPushableFixture.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Lantern/LLKLanternFeel.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "Net/UnrealNetwork.h"
#include "World/LLKPushFeel.h"

ALLKPushableFixture::ALLKPushableFixture()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false); // `Travel` is the movement, and it replicates itself.

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Body->SetCollisionObjectType(ECC_WorldStatic);
	Body->SetMobility(EComponentMobility::Movable);
}

void ALLKPushableFixture::BeginPlay()
{
	Super::BeginPlay();

	// Captured once. Measuring travel from the live transform would let
	// rounding accumulate into the rail itself, and a rail that drifts is a
	// puzzle whose solution moves between sessions.
	RailOrigin = GetActorLocation();
	WorldAxis = GetActorTransform().TransformVectorNoScale(LocalPushAxis);
	if (!WorldAxis.Normalize())
	{
		// An un-normalisable axis is an authoring mistake that would otherwise
		// present as a fixture nobody can move, with no error anywhere.
		UE_LOG(LogLLK, Warning,
			TEXT("%s has a zero LocalPushAxis and CANNOT BE PUSHED. It will "
				 "focus, prompt, and do nothing. Author an axis."), *GetName());
		WorldAxis = FVector(1.f, 0.f, 0.f);
	}

	if (!bReported)
	{
		bReported = true;
		// Affirmative, for the third time in three days, and for the reason
		// the navmesh taught: a fixture nobody pushed and a fixture that
		// cannot be pushed produce exactly the same empty log.
		UE_LOG(LogLLK, Display,
			TEXT("%s can be pushed: %.1f studs of travel along (%.2f, %.2f, %.2f) "
				 "at %.1f studs/s, which is %.1f s end to end, and it is %.0f studs "
				 "loud the whole way (a sprint is 12.5)."),
			*GetName(), TravelStuds, WorldAxis.X, WorldAxis.Y, WorldAxis.Z,
			LLKPushFeel::PushSpeedStuds,
			LLKPushFeel::PushSpeedStuds > 0.f ? TravelStuds / LLKPushFeel::PushSpeedStuds : -1.f,
			LLKPushFeel::PushNoiseStuds);
	}
}

void ALLKPushableFixture::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALLKPushableFixture, Travel);
}

bool ALLKPushableFixture::CanFocus(const APawn* Keeper) const
{
	if (Keeper == nullptr || Keeper->GetWorld() != GetWorld())
	{
		return false;
	}

	// Eye to body, not foot to foot. The same measurement the lantern pickup
	// makes, and for the same reason: a child bending to shove something is
	// mostly closing a vertical gap, and a foot-to-foot range would let them
	// start a push from behind a counter they never left.
	const FVector Eye = LLKLanternFeel::KeeperEyeWorldPosition(Keeper->GetActorLocation());
	const FVector ToBody = GetPushPoint() - Eye;
	const float Distance = ToBody.Size();
	if (Distance < KINDA_SMALL_NUMBER || Distance > ReachCm)
	{
		return false;
	}

	return FVector::DotProduct(
		Keeper->GetBaseAimRotation().Vector(), ToBody / Distance) >= MinAimDot;
}

FVector ALLKPushableFixture::GetPushPoint() const
{
	// Up in world space, not along the actor's own up: a fixture rotated to
	// face its aisle must still be shoved at hand height rather than at an
	// angle that depends on how it was placed.
	return GetActorLocation()
		+ FVector(0.f, 0.f, LLKUnits::DistanceFromStuds(PushPointHeightStuds));
}

FText ALLKPushableFixture::FocusVerb() const
{
	return NSLOCTEXT("LLK", "PushFixture", "Push");
}

void ALLKPushableFixture::BeginInteract(APawn* Keeper)
{
	if (!HasAuthority() || Keeper == nullptr)
	{
		return;
	}

	Pusher = Keeper;
	UE_LOG(LogLLK, Verbose, TEXT("%s is being pushed by %s."), *GetName(), *Keeper->GetName());
}

void ALLKPushableFixture::EndInteract(APawn* Keeper)
{
	if (!HasAuthority())
	{
		return;
	}

	// Unconditional, and it does not check that `Keeper` is the current
	// pusher. A release always stops the push: the alternative is a fixture
	// that keeps moving because the keeper who let go was not the one the
	// server last recorded, which is a stuck object nobody can explain.
	Pusher = nullptr;
	bMovingThisFrame = false;
}

void ALLKPushableFixture::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	const APawn* Keeper = Pusher.Get();
	if (Keeper == nullptr)
	{
		bMovingThisFrame = false;
		return;
	}

	// **Focus is re-checked every frame, not just at the press.** A keeper who
	// walks away mid-push, or turns to look at something else, has stopped
	// pushing - and a server that only validated the opening request would let
	// a player start a shove and then run for cover while the fixture finished
	// the job and made all the noise for them.
	if (!CanFocus(Keeper))
	{
		Pusher = nullptr;
		bMovingThisFrame = false;
		return;
	}

	const float Before = Travel;
	const float AxisDot =
		FVector::DotProduct(Keeper->GetActorForwardVector(), WorldAxis);
	Travel = LLKPushFeel::AdvancePush(Travel, AxisDot, DeltaSeconds, TravelStuds);
	bMovingThisFrame = LLKPushFeel::IsMoving(Before, Travel);

	if (bMovingThisFrame)
	{
		ApplyTravel();
	}
}

void ALLKPushableFixture::OnRep_Travel()
{
	// Clients place it from the replicated scalar rather than from a
	// replicated transform, so there is exactly one definition of where a
	// fixture is and it is the same arithmetic on both sides.
	ApplyTravel();
}

void ALLKPushableFixture::ApplyTravel()
{
	SetActorLocation(RailOrigin + WorldAxis * LLKUnits::DistanceFromStuds(Travel));
}

bool ALLKPushableFixture::GetCurrentNoise(LLKHearing::FLLKNoise& OutNoise) const
{
	if (!bMovingThisFrame)
	{
		return false;
	}

	OutNoise.Position = GetActorLocation();
	OutNoise.RadiusStuds = LLKPushFeel::PushNoiseStuds;
	return true;
}
