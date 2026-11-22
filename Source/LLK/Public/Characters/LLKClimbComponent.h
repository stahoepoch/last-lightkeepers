// Finding a ledge, and getting the keeper onto it.
//
// `LLKClimbFeel` holds the rules and the argument for them. This is the half
// that needs a world: two traces to find a surface, a height check against
// what the movement component can already step over, a room check so the
// keeper never lands inside something, and a server-driven move.
//
// **Why a component and not a function on the character.** `ALLKCharacter` is
// already 1,100 lines carrying movement, sprint, crouch, burden, stumble, the
// lantern and the input fallback. A verb that owns its own state - am I
// climbing, where to, how far through - belongs beside them rather than
// inside them, for the reason `ULLKInteractorComponent` exists: the thing with
// its own state and its own tick is its own object.
//
// **Server-authoritative, and the client is told.** A client that could decide
// it had finished a mantle could place itself on top of anything. The server
// runs the traces, owns the progress and moves the capsule; movement
// replication carries the result, which is what it is already for.
//
// **It is an `ILLKNoiseSource`, which is the whole reason the exposure is
// real.** A mantle moves the capsule directly, so the keeper's velocity-
// derived noise would read as silence for the entire climb - the most
// helpless second in the chapter would be the quietest. That is the shape of
// defect this project keeps finding, so the climb reports its own sound.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Enemies/LLKNoiseSource.h"

#include "LLKClimbComponent.generated.h"

class ACharacter;

/** A surface the keeper could get onto, as the traces found it. */
USTRUCT(BlueprintType)
struct FLLKLedge
{
	GENERATED_BODY()

	/** Where the capsule ends up standing. Already room-checked. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Climb")
	FVector CapsuleDestination = FVector::ZeroVector;

	/** How far above the keeper's feet the surface is, in cm. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Climb")
	float HeightCm = 0.f;

	/** Whether anything was found at all. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Climb")
	bool bFound = false;
};

UCLASS(ClassGroup = (LLK), meta = (BlueprintSpawnableComponent))
class LLK_API ULLKClimbComponent : public UActorComponent, public ILLKNoiseSource
{
	GENERATED_BODY()

public:
	ULLKClimbComponent();

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- ILLKNoiseSource --------------------------------------------------

	virtual bool GetCurrentNoise(LLKHearing::FLLKNoise& OutNoise) const override;

	/**
	 * Look for a ledge in front of the keeper.
	 *
	 * Const and side-effect free, so the HUD may call it to show a prompt and
	 * the server may call it to validate, and the two cannot disagree about
	 * what is in front of the player. The same contract
	 * `ULLKInteractorComponent::ResolveLanternPickup` states.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Climb")
	FLLKLedge FindLedge() const;

	/**
	 * Try to start a climb. Returns whether one started.
	 *
	 * **The return value is what makes jump still work.** The character tries
	 * a mantle first and jumps when this says no, so one key does the
	 * obvious thing in both cases and no new input action is needed - which
	 * also keeps this out of `Config/` and out of Codex's lane.
	 */
	UFUNCTION(BlueprintCallable, Category = "LLK|Climb")
	bool TryClimb();

	/** Is a mantle running right now. Replicated, so a client can suppress input. */
	UFUNCTION(BlueprintPure, Category = "LLK|Climb")
	bool IsClimbing() const { return bClimbing; }

	/** 0..1 through the current mantle. */
	UFUNCTION(BlueprintPure, Category = "LLK|Climb")
	float GetClimbAlpha() const { return Alpha; }

	/**
	 * How far in front of the keeper a ledge can be, in cm.
	 *
	 * Their capsule radius plus a hand's reach past it, derived from the body
	 * rather than chosen: a keeper has to be standing at the thing, not near
	 * it. Public so the boot report can say what it is.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Climb")
	static float ForwardReachCm();

protected:
	UFUNCTION(Server, Reliable)
	void ServerStartClimb();

	/** The character this is on, or null. */
	ACharacter* Keeper() const;

	/** Server: advances the mantle and finishes it. */
	void TickClimb(float DeltaSeconds);

	UPROPERTY(Replicated)
	bool bClimbing = false;

	UPROPERTY(Replicated)
	float Alpha = 0.f;

	/** Server only: where the mantle started and ends, and how long it takes. */
	FVector ClimbStart = FVector::ZeroVector;
	FVector ClimbEnd = FVector::ZeroVector;
	float ClimbDuration = 0.f;
};
