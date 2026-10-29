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
#include "InputActionValue.h"
#include "Data/LLKDataTypes.h"
#include "Data/LLKLanternData.h"
#include "LLKCharacter.generated.h"

class UCameraComponent;
class ULLKLanternComponent;
class UInputAction;
class UInputMappingContext;

/**
 * The four actions the keeper needs, and the context that maps keys to them.
 *
 * One struct rather than five loose pointers so that "do we have input?" is a
 * single question with a single answer. A half-bound keeper - move but no
 * look, say - is worse than an unbound one, because it looks like a physics
 * bug rather than a missing asset.
 */
USTRUCT()
struct FLLKInputBindings
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UInputMappingContext> Context = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> Move = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> Look = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> Sprint = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> CrouchToggle = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> Jump = nullptr;

	bool IsComplete() const
	{
		return Context != nullptr && Move != nullptr && Look != nullptr
			&& Sprint != nullptr && CrouchToggle != nullptr && Jump != nullptr;
	}
};

/** Where the keeper's input bindings came from. Mirrors ELLKDataSource. */
UENUM(BlueprintType)
enum class ELLKInputSource : uint8
{
	/** Nothing resolved yet. */
	Unresolved,
	/** Real Input Action assets from `Content/`. These always win. */
	MappingContextAsset,
	/** Built in code because the assets do not exist yet. A stopgap, by design. */
	TransientFallback
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
	ELLKDataSource GetSpeedSource() const { return SpeedSource; }

	/** Walk speed in cm/s, after the stud conversion. */
	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetWalkSpeed() const { return WalkSpeedCms; }

	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetSprintSpeed() const { return SprintSpeedCms; }

	/** Which path the input bindings came from. Assets win; the fallback announces itself. */
	UFUNCTION(BlueprintPure, Category = "LLK|Input")
	ELLKInputSource GetInputSource() const { return InputSource; }

	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	ULLKLanternComponent* GetLantern() const { return Lantern; }

	/**
	 * Builds the stopgap bindings in code: WASD, mouse look, shift, ctrl.
	 *
	 * **This exists so the keeper can be walked before anyone authors a
	 * `.uasset`**, because `Content/` is Chris's alone (row 41) and M1.6's
	 * real Input Actions are not made yet. `UInputAction` and
	 * `UInputMappingContext` are plain `UDataAsset` subclasses, so nothing
	 * about Enhanced Input requires them to come off disk.
	 *
	 * Static and outer-parameterised so a test can build the table and
	 * inspect it without a world, a pawn or a running game.
	 */
	static FLLKInputBindings BuildTransientBindings(UObject* Outer);

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

	/**
	 * The keeper's lantern state. M1.2 owns on/off; the light itself is M1.3
	 * and Chris's, so nothing here creates or references a ULightComponent.
	 */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<ULLKLanternComponent> Lantern;

	/**
	 * Real assets, if they exist. Unset by default - `Content/` is Chris's,
	 * and M1.6 fills these in. All five must resolve or none are used: a
	 * partial asset set falls back wholesale rather than mixing two sources.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputMappingContext> MappingContextAsset;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> MoveActionAsset;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> LookActionAsset;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> SprintActionAsset;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> CrouchActionAsset;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> JumpActionAsset;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

	/** Picks the asset path if it is complete, the code path otherwise. */
	void ResolveInputBindings();

	/** Pushes the resolved context into the local player's input subsystem. */
	void ApplyMappingContext();

	void OnMoveInput(const FInputActionValue& Value);
	void OnLookInput(const FInputActionValue& Value);
	void OnSprintStarted();
	void OnSprintStopped();
	void OnCrouchStarted();
	void OnCrouchStopped();
	void OnJumpStarted();
	void OnJumpStopped();

	UPROPERTY()
	FLLKInputBindings ActiveBindings;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Input")
	ELLKInputSource InputSource = ELLKInputSource::Unresolved;

	UPROPERTY(ReplicatedUsing = OnRep_Sprinting)
	bool bSprinting = false;

	UPROPERTY(VisibleInstanceOnly, Category = "LLK|Movement")
	ELLKDataSource SpeedSource = ELLKDataSource::Unresolved;

	float WalkSpeedCms = 0.f;
	float SprintSpeedCms = 0.f;
	float CrouchSpeedCms = 0.f;
};
