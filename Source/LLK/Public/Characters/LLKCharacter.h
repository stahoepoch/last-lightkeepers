// The keeper. M1.1.
//
// **Row 44 is LOCKED: first person is not a camera setting here.** There is
// no third-person boom on this class and none is kept working. The lantern,
// hands, carry states, tools and the downed crawl are all first-person
// systems, and the saving that buys is no full-body player animation in the
// first slice beyond what co-op players see of each other.
//
// **Server-authoritative from the first commit**, because `docs/ROADMAP.md`
// lists "replication retrofitted late" as a named risk with an owner. Sprint
// is a replicated server decision, never a client-set speed. Crouch uses
// ACharacter's own replicated crouch rather than a second mechanism beside it.
//
// **No Content dependency.** This class runs with no mesh, no Blueprint and
// no DataAsset: a capsule, a camera and declared fallback speeds. That is
// deliberate - `Content/` is Chris's alone (row 41), and a gameplay class
// that cannot be compiled or tested without an artist is a class that blocks.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/LLKLanternData.h"
#include "LLKCharacter.generated.h"

class UCameraComponent;

/**
 * Where a keeper's movement numbers came from, so "it works" can never
 * silently mean "the asset was missing and you are walking on defaults".
 */
UENUM(BlueprintType)
enum class ELLKSpeedSource : uint8
{
	/** Nothing resolved yet. */
	Unresolved,
	/** Read from a ULLKLanternData generated out of `data/lantern.json`. */
	LanternData,
	/** The asset was not set or failed to load; the declared fallbacks are in use. */
	Fallback
};

UCLASS()
class LLK_API ALLKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALLKCharacter();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---------------------------------------------------------------- intent
	// M1.6 (Enhanced Input, Codex's row) binds to these. They are the seam
	// between "a key was pressed" and "the server decided", and they are
	// deliberately not Input Actions themselves - nothing here is bound to a
	// key, so the binding scheme can be rebuilt without touching movement.

	/** Axis: X forward/back, Y right/left, each -1..1. */
	void MoveInput(const FVector2D& Axis);

	/** Axis: X yaw, Y pitch, in degrees for this frame. */
	void LookInput(const FVector2D& Axis);

	/** Asks to sprint. The server decides; the client never sets its own speed. */
	UFUNCTION(BlueprintCallable, Category = "LLK|Movement")
	void SetSprinting(bool bWantsToSprint);

	/** Asks to crouch. Routes to ACharacter's replicated crouch. */
	UFUNCTION(BlueprintCallable, Category = "LLK|Movement")
	void SetCrouching(bool bWantsToCrouch);

	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	bool IsSprinting() const { return bSprinting; }

	/** Which path the speeds came from. Read this in tests rather than trusting a log line. */
	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	ELLKSpeedSource GetSpeedSource() const { return SpeedSource; }

	/** Walk speed in cm/s, after the stud conversion. */
	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetWalkSpeed() const { return WalkSpeedCms; }

	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetSprintSpeed() const { return SprintSpeedCms; }

protected:
	/**
	 * The generated asset built from `data/lantern.json` by M0.5's writer.
	 *
	 * Soft and unset by default on purpose. `Content/Data/` is generated and
	 * not committed, so a clean clone has no asset here until the importer
	 * runs - and a character that refuses to exist in that state would make
	 * every clean checkout un-runnable. Unset means fallbacks, loudly.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	TSoftObjectPtr<ULLKLanternData> LanternData;

	// The fallbacks, in STUDS, matching `data/lantern.json` at the time of
	// writing. They are duplicated values and that is a real cost, so they
	// are declared rather than hidden: GetSpeedSource() reports when they are
	// in use, and the log says so at BeginPlay. A copy nobody can detect is
	// the problem; a copy that announces itself is a fallback.
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data", meta = (ClampMin = "0"))
	float FallbackWalkStuds = 16.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data", meta = (ClampMin = "0"))
	float FallbackSprintStuds = 23.f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data", meta = (ClampMin = "0"))
	float FallbackCrouchStuds = 9.f;

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bWantsToSprint);

	UFUNCTION()
	void OnRep_Sprinting();

	/** Resolves speeds from the asset or the fallbacks, then applies them. */
	void ResolveSpeeds();

	/** Pushes the current speed to the movement component. Server and clients. */
	void ApplyMovementSpeed();

	UPROPERTY(VisibleAnywhere, Category = "LLK|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(ReplicatedUsing = OnRep_Sprinting)
	bool bSprinting = false;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Movement")
	ELLKSpeedSource SpeedSource = ELLKSpeedSource::Unresolved;

	float WalkSpeedCms = 0.f;
	float SprintSpeedCms = 0.f;
	float CrouchSpeedCms = 0.f;
};
