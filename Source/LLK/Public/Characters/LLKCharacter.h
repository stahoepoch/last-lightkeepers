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
// **Gameplay does not depend on Content.** This class still runs from a
// capsule, camera and declared fallback speeds if an optional presentation
// asset is absent. The temporary keeper mesh is a co-op visual only; losing
// it must never prevent a playable build or change a lantern calculation.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Data/LLKDataTypes.h"
#include "Characters/LLKVitalsComponent.h"
#include "Data/LLKLanternData.h"
#include "Data/LLKLootRulesData.h"
#include "LLKCharacter.generated.h"

class UCameraComponent;
class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class ULLKLanternComponent;
class ULLKLanternLightComponent;
class ULLKInteractorComponent;
class ULLKVitalsComponent;
class UInputAction;
class UInputMappingContext;
enum class ELLKLanternChangeReason : uint8;

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

	virtual void Tick(float DeltaSeconds) override;
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

	/**
	 * What carrying a haul costs this keeper, as a multiplier on every speed.
	 *
	 * 1.0 empty-handed. The rule is `LLKBurden`'s and the points are the
	 * interactor's relic count; this is the seam, so a readout and the
	 * movement path cannot disagree about how slow a keeper is.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetBurdenMultiplier() const;

	/** The authored burden rules this pawn resolved, for a readout to explain. */
	const FLLKBurdenRules& GetBurdenRules() const { return BurdenRules; }

	/**
	 * A Sweeper clipped the keeper's feet. Server-authoritative; a repeat
	 * contact restarts the window rather than stacking.
	 *
	 * **It costs distance and tempo, never input.** No camera lock, no input
	 * suppression, nothing to wait out - a control removal the player cannot
	 * act through is a knockdown with a shorter timer, and the knockdown is
	 * already the authored consequence of three landed contacts. This is the
	 * cost of the first two.
	 */
	void ApplyStumble();

	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	bool IsStumbling() const { return bStumbling; }

	/**
	 * The stumble's speed scale, composing with burden rather than replacing
	 * it: a burdened keeper who is clipped pays both, which is the same
	 * argument that applies burden to every speed.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetStumbleMultiplier() const;

	/** Launch speed in cm/s, after the stud conversion. */
	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetJumpVelocity() const;

	/**
	 * Apex height in cm, from the jump speed and gravity actually applied.
	 *
	 * Published because it is the number with design meaning: row 63 needs a
	 * counter to be un-jumpable, and `LLK.Movement.JumpWeight` asserts that
	 * against this rather than against the two inputs separately.
	 */
	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetJumpApexCm() const;

	/** Seconds from launch to landing on flat ground. The float, measured. */
	UFUNCTION(BlueprintPure, Category = "LLK|Movement")
	float GetJumpAirtimeSeconds() const;

	/** Which path the input bindings came from. Assets win; the fallback announces itself. */
	UFUNCTION(BlueprintPure, Category = "LLK|Input")
	ELLKInputSource GetInputSource() const { return InputSource; }

	UFUNCTION(BlueprintPure, Category = "LLK|Lantern")
	ULLKLanternComponent* GetLantern() const { return Lantern; }

	/** What the keeper is looking at and whether they can take it. The HUD reads this. */
	UFUNCTION(BlueprintPure, Category = "LLK|Interact")
	ULLKInteractorComponent* GetInteractor() const { return Interactor; }

	/** Standing or on the floor. M2.1. */
	UFUNCTION(BlueprintPure, Category = "LLK|Vitals")
	ULLKVitalsComponent* GetVitals() const { return Vitals; }

	UFUNCTION(BlueprintPure, Category = "LLK|Vitals")
	bool IsDowned() const;

	/**
	 * Console commands, so knockdown can be tested before M2.2 exists.
	 *
	 * `LLKDown` and `LLKUp` from the console. There is no way back up until
	 * M2.2 lands the revive, and a state you can enter and not leave is
	 * untestable by hand - which for a feature whose whole proof is "a human
	 * looked at it" would mean shipping it unlooked-at.
	 *
	 * Server-routed like everything else: the exec runs on the client that
	 * typed it and asks, so these cannot be used to stand up in a co-op game
	 * where the server disagrees.
	 */
	UFUNCTION(Exec)
	void LLKDown();

	UFUNCTION(Exec)
	void LLKUp();

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

	/**
	 * Jump launch speed in STUDS/s, and the gravity it falls under.
	 *
	 * **Chris, 2026-10-29, playing the harness: "the jump feels like gravity
	 * is a little light, i float a little much."** He is right, and two
	 * things were compounding. Unreal's defaults are `JumpZVelocity` 420 with
	 * `GravityScale` 1 against -980 gravity, which is a 90cm apex and 0.86
	 * seconds of airtime. And our keeper walks at 448cm/s rather than
	 * Unreal's default 600, so less ground passes per second of air and the
	 * hang feels proportionally longer than the same jump would in a stock
	 * project. The defaults are tuned for a shooter; this is not one.
	 *
	 * **The arithmetic, so these can be argued with rather than guessed at.**
	 * Apex is `v^2 / (2 * 980 * G)` and airtime is `2v / (980 * G)`. At 13.2
	 * studs/s (369.6cm/s) and G 1.25 that is a **56cm apex over 0.60s**,
	 * against the stock 90cm over 0.86s.
	 *
	 * **These are Codex's numbers, not mine, and his were better.** He
	 * proposed `JumpZVelocity` 370 with `GravityScale` 1.25; I had reached
	 * 392 at 1.6, which is a 49cm apex over 0.50s. Both clear the row 63 bar
	 * below, but his warning was the right one - *"without turning jumps into
	 * tiny hops"* - and mine was heading that way. A smaller departure from
	 * stock is also the safer first move when the thing being tuned is feel,
	 * and the difference between 0.50s and 0.60s is for Chris to judge by
	 * playing rather than for either of us to argue about. 13.2 studs is 369.6
	 * cm/s: his 370, expressed in the unit row 61 requires, and within half a
	 * centimetre per second of it.
	 *
	 * **And it protects row 63, which matters more than the feel.** A 90cm
	 * apex clears a 90cm counter, so the stock jump already did the vault's
	 * job - and a vault you do not need is not "desperate and physical", it
	 * is a slower way to do something you could already do. At 49cm a counter
	 * has to be mantled. The traversal work now has a reason to exist, and
	 * that was true before anybody noticed the floating.
	 *
	 * **These are not in `data/` yet and should be.** `Tools/` is Codex's
	 * lane, and a new key in `lantern.json` needs his importer validator to
	 * know about it or the data suite fails on an unrecognised key. Requested
	 * in `NOTES-FOR-CODEX.md`; declared here until then, and deliberately not
	 * added to `FLLKMovementRules` - a reflected field the generated asset
	 * has no value for would resolve to zero and beat this fallback, which is
	 * exactly the stale-asset bug that cost an evening earlier today.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Movement", meta = (ClampMin = "0"))
	float JumpStuds = 13.2f;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Movement", meta = (ClampMin = "0.1"))
	float GravityScale = 1.25f;

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bWantsToSprint);

	UFUNCTION(Server, Reliable)
	void ServerSetDownedForDebug(bool bDown);

	/** Re-applies speed and un-sprints when the keeper goes down or gets up. */
	UFUNCTION()
	void OnDownedChanged(bool bIsDowned, ELLKDownedReason Reason);

	UFUNCTION()
	void OnRep_Sprinting();

	/** Resolves speeds from the asset or the fallbacks, then applies them. */
	void ResolveSpeeds();

	/** Pushes the current speed to the movement component. Server and clients. */
	void ApplyMovementSpeed();

	/** Clears the stumble and re-applies the speed. Server-side, on a timer. */
	void EndStumble();

	UFUNCTION()
	void OnRep_Stumbling();

	/**
	 * The stumble's two numbers, declared here rather than authored in
	 * `data/lantern.json` because they are **Chris's to set and he has not
	 * set them**. Declared rather than hidden, for the reason the speed
	 * fallbacks above give: a copy nobody can detect is the problem; a copy
	 * that announces itself is a fallback. They move to
	 * `sweeperContact` the moment he has a feel for them.
	 *
	 * Both are reasoned against numbers that *are* authored, not chosen:
	 *
	 * - **0.7** is a 30% cost, against burden's `maxSpeedPenalty` of 0.12.
	 *   Being clipped by a maintenance machine should read as worse than
	 *   carrying a full haul, and two and a half times is enough to feel
	 *   without being a stop.
	 * - **1.0 s** sits under the authored `contactCooldownSeconds` of 1.5, and
	 *   that bound is the load-bearing half. A stumble that outlasts the
	 *   cooldown would be re-applied before it expired, so repeated contact
	 *   would become a permanent slow that nothing in the code says is
	 *   permanent. Under the cooldown, each contact is a separate, visible
	 *   cost. At the authored walk of 16 studs/s it is about 4.8 studs of
	 *   ground - real, and survivable.
	 */
	/**
	 * **Retuned 2026-11-02 on Chris's "tune it", and derived rather than
	 * picked.** 0.7 for 1.0s was the first pass; the arithmetic says it did
	 * not bite.
	 *
	 * Sprinting is free, and deliberately so - `data/lantern.json`:
	 * *"Movement is a readability and route-choice tool, not a stamina
	 * minigame. Sprint gets you to an exit or away from a Sweeper."* So at
	 * 0.7 a stumbled sprint is 451 cm/s against the Sweeper's 392, and
	 * sprinting defeated the stumble outright at no cost. It only taxed
	 * players who did not think to sprint, which punishes not knowing rather
	 * than choosing.
	 *
	 * At **0.55**, against the authored speeds:
	 *
	 *   - stumbled walk is 246 cm/s, which is **below the deliberate crouch
	 *     of 252**. Reduced to less than a careful crouch is unmistakably
	 *     "tripped" rather than "slightly slowed", which is the thing Chris
	 *     described.
	 *   - stumbled sprint is 354 against the Sweeper's 392, so a contact
	 *     costs you ground **whatever you do** for as long as it lasts.
	 *   - avoiding the third hit becomes the skill, which is the whole point
	 *     of the ladder.
	 *
	 * **What this knowingly trades**: it makes sprint temporarily
	 * insufficient, which is a 1.2-second exception to "sprint gets you away
	 * from a Sweeper" rather than a reversal of it. Flagged because it is the
	 * one place this value argues with an authored note, and that is Chris's
	 * to overrule.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data", meta = (ClampMin = "0", ClampMax = "1"))
	float FallbackStumbleMultiplier = 0.55f;

	/**
	 * 1.2s, still safely under the 1.5s contact cooldown.
	 *
	 * That bound is the load-bearing half and it is unchanged: a stumble that
	 * outlasted the cooldown would be re-applied before expiring, and
	 * repeated contact would become a permanent slow nothing in the code
	 * calls permanent. 0.3s of full speed between contacts is the recovery
	 * window, and it is what keeps this a cost rather than a spiral.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Data", meta = (ClampMin = "0"))
	float FallbackStumbleSeconds = 1.2f;

	/** Replicated so an owning client's movement agrees with the server's. */
	UPROPERTY(ReplicatedUsing = OnRep_Stumbling)
	bool bStumbling = false;

	FTimerHandle StumbleTimer;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	/** Stable hand-space origin shared by the rendered lantern and game light field. */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<USceneComponent> LanternAnchor;

	/**
	 * Co-op-only presentation anchor, mounted on the keeper mesh's right palm.
	 * It intentionally never feeds the gameplay light field: animation and mesh
	 * replacement are presentation concerns, while LanternAnchor remains the
	 * stable source for interaction and perception.
	 */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<USceneComponent> RemoteLanternAnchor;

	/** Temporary glass chamber of the carried lantern. Replaced by authored art later. */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> LanternGlass;

	/** Temporary hard-surface silhouette: two caps, cage rails and a carry handle. */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> LanternTop;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> LanternBase;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> LanternCageLeft;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> LanternCageRight;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> LanternHandle;

	/** The same temporary silhouette, visible to teammates from the palm socket. */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> RemoteLanternGlass;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> RemoteLanternTop;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> RemoteLanternBase;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> RemoteLanternCageLeft;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> RemoteLanternCageRight;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UStaticMeshComponent> RemoteLanternHandle;

	/** Cosmetic near-field glow only. It is never exposed to the gameplay light field. */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UPointLightComponent> LanternCoreGlow;

	UPROPERTY(VisibleAnywhere, Category = "LLK|Lantern")
	TObjectPtr<UPointLightComponent> RemoteLanternCoreGlow;

	/**
	 * Purely visual elapsed time for the local held instrument. The gameplay
	 * light remains rooted at LanternAnchor, so this may never feed sight,
	 * relic inspection, or enemy perception.
	 */
	float LanternPresentationSeconds = 0.f;

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

	/** Matches cosmetic core glow to the replicated lantern state. */
	UFUNCTION()
	void OnLanternVisualStateChanged(bool bIsOn, ELLKLanternChangeReason Reason);

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
	 * Standing or contained. Row 47's machines put people on the floor rather
	 * than killing them, so this is the keeper's whole condition - there is no
	 * health here and no death.
	 */
	UPROPERTY(VisibleAnywhere, Category = "LLK|Vitals")
	TObjectPtr<ULLKVitalsComponent> Vitals;

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
	void OnInteractReleased();

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

	/**
	 * `loot.json`'s `burden` block, resolved once beside the speeds.
	 *
	 * Read here rather than in the interactor because this is where a speed
	 * is decided, and row 55 keeps the values Chris's: 0.04 per relic to a cap
	 * of 0.12, which is his playtest call that *"ordinary loot must not make a
	 * raid feel sluggish."*
	 */
	FLLKBurdenRules BurdenRules;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Movement")
	TSoftObjectPtr<ULLKLootRulesData> LootRules =
		TSoftObjectPtr<ULLKLootRulesData>(FSoftObjectPath(TEXT("/Game/Data/DA_LootRules.DA_LootRules")));

	/** Re-applies the speeds when the haul changes. Bound to the interactor. */
	UFUNCTION()
	void OnCarriedCountChanged(int32 CarriedCount);
};
