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
class ULLKLanternLightComponent;
class ULLKInteractorComponent;
class UInputAction;
class UInputMappingContext;

/**
 * The actions the keeper needs, and the context that maps keys to them.
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

	UPROPERTY()
	TObjectPtr<UInputAction> LanternToggle = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> Interact = nullptr;

	bool IsComplete() const
	{
		return Context != nullptr && Move != nullptr && Look != nullptr
			&& Sprint != nullptr && CrouchToggle != nullptr && Jump != nullptr
			&& LanternToggle != nullptr && Interact != nullptr;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	/** What the keeper is looking at and whether they can take it. The HUD reads this. */
	UFUNCTION(BlueprintPure, Category = "LLK|Interact")
	ULLKInteractorComponent* GetInteractor() const { return Interactor; }

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
	 * Defaults to the importer's output path. A soft pointer that resolves to
	 * nothing simply fails, so a clean clone with no `Content/Data/` still
	 * runs - it just announces the fallback. That is strictly better than
	 * leaving this unset, which was the original shape: the assets existed on
	 * disk and nothing pointed at them, so the game ran on declared fallbacks
	 * that happened to match while reporting that it had read no data.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data")
	TSoftObjectPtr<ULLKLanternData> LanternData =
		TSoftObjectPtr<ULLKLanternData>(FSoftObjectPath(TEXT("/Game/Data/DA_Lantern.DA_Lantern")));

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
	 * The keeper's lantern state. M1.2 owns on/off and owns no light.
	 */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<ULLKLanternComponent> Lantern;

	/**
	 * M1.3's placeholder light, which only listens.
	 *
	 * The real lantern - mesh, materials, the sound of it striking - is M1.3
	 * proper and Chris's (`Content/` is his, row 41). This is a code-built
	 * spot light so that the keeper's light can be seen and the subscription
	 * shape can be proven before any of that exists, which is exactly what
	 * M1.3's proof line asks for. It is attached to the camera, not the
	 * capsule: a first-person keeper's light comes from where they are
	 * looking, and row 44 makes the camera the head.
	 */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<ULLKLanternLightComponent> LanternLight;

	/**
	 * Resolves what the keeper is looking at, once, for everyone.
	 *
	 * On the character rather than in the HUD because the prompt and the take
	 * key have to agree about which object they mean. Two searches disagree
	 * the moment two relics are nearly equidistant, and then the game takes
	 * something the player was not looking at.
	 */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Interact")
	TObjectPtr<ULLKInteractorComponent> Interactor;

	/**
	 * The real Input Action assets, pointed at the paths M1.6 authors.
	 *
	 * **Pointed rather than left unset, on Codex's handoff for `98ff8b2`.**
	 * The paths below are exactly what his editor validator enforces, and a
	 * soft pointer that resolves to nothing simply fails - so until Chris
	 * creates them in `/Game/Input` these all resolve null, the set is
	 * incomplete, and the transient code bindings carry on exactly as before.
	 * The moment the assets exist the real rebindable and gamepad route
	 * activates with no code change, which is the point of doing this now
	 * rather than on the day.
	 *
	 * **All SIX must resolve or none are used.** A partial set falls back
	 * wholesale rather than mixing two sources, because a keeper with
	 * authored movement and a code-bound lantern is a keeper whose rebinding
	 * screen lies about one key. The count moved from five to six when the
	 * lantern was added; `IsComplete()` is what enforces it.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputMappingContext> MappingContextAsset =
		TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath(TEXT("/Game/Input/IMC_LLK.IMC_LLK")));

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> MoveActionAsset =
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_LLK_Move.IA_LLK_Move")));

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> LookActionAsset =
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_LLK_Look.IA_LLK_Look")));

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> SprintActionAsset =
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_LLK_Sprint.IA_LLK_Sprint")));

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> CrouchActionAsset =
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_LLK_Crouch.IA_LLK_Crouch")));

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> JumpActionAsset =
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_LLK_Jump.IA_LLK_Jump")));

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> LanternActionAsset =
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_LLK_Lantern.IA_LLK_Lantern")));

	/**
	 * **A SEVENTH action, which the authored contract does not know about yet.**
	 *
	 * Codex's validator in `98ff8b2` enforces six: Move, Look, Sprint, Crouch,
	 * Jump, Lantern. Interact makes seven, and his validator and the assets in
	 * `/Game/Input` both need the extra entry. Until they have it this path
	 * resolves null, the set is incomplete, and the whole scheme falls back to
	 * the transient bindings - which is the existing all-or-nothing rule
	 * working as intended rather than a new problem.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Input")
	TSoftObjectPtr<UInputAction> InteractActionAsset =
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_LLK_Interact.IA_LLK_Interact")));

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
	void OnLanternToggled();
	void OnInteract();

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
