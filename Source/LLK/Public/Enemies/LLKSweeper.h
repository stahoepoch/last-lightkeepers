// A Sweeper with a body. M3's first encounter.
//
// **The thin half of `LLKSweeperBrain`**: it measures, it moves, and it
// concludes nothing. Every decision belongs to the brain, which is pure and
// tested with no world; this class is the gatherer, the same split
// `LLKMapReadiness::Observe` and `ULLKExtractionSubsystem` already use.
//
// **Chris's scope, 2026-10-30, and it is the shape of the whole file:** *"Use
// the authored Sweeper speed and state machine. Do not invent patrol points,
// route shapes, collision tuning, or spawn spacing. The dock map should author
// those."* So:
//
//   - The speed is `enemies.json`'s `moveSpeed: 14`, converted at the boundary
//     through `LLKUnits` (row 61). No other movement number is set here.
//   - The capsule is `ACharacter`'s default. A Sweeper's real size is a
//     question about doorways and shelf gaps, which is map work.
//   - Patrol points are actors the MAP tags `LLK.Patrol`. This class never
//     invents one, never generates a route, and holds still when there are
//     none - which is today's state, and it says so by name once rather than
//     standing there silently.
//
// **No `AIController`, and that is a lane decision rather than a technical
// one.** `AAIController` and its path following live in `AIModule`, which
// would mean adding a dependency to `LLK.Build.cs` - Codex's file
// (`docs/LANES.md`). `NavigationSystem` is already a dependency because the
// map-readiness checks use it, so this follows a path it queries itself and
// steers with `AddMovementInput`. If path following ever needs the real thing -
// crowd avoidance, for one - that is a one-line handoff rather than a rewrite,
// because the brain would not change.
//
// **What it does on the server only.** Perception, the brain and the knockdown
// all run on authority. A client's copy walks because its movement replicates,
// not because it decided anything - an enemy that decided locally would be an
// enemy that could disagree with the server about whether it had caught you.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKLanternData.h"
#include "Enemies/LLKSweeperBrain.h"
#include "GameFramework/Character.h"
#include "LLKSweeper.generated.h"

class ULLKEnemyRosterSubsystem;
class ULLKVitalsComponent;

/**
 * Row 16's cue, for whatever plays it.
 *
 * Fires on the frame the telegraph begins, on the server. `assets/audio/
 * sweeper-telegraph-v1.wav` exists and nothing plays it yet: a `USoundBase` is
 * `Content/` and Chris's alone (row 41), so this is the seam that waits for it.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLKSweeperTelegraph, FVector, Position);

UCLASS(Blueprintable)
class LLK_API ALLKSweeper : public ACharacter
{
	GENERATED_BODY()

public:
	ALLKSweeper();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "LLK|Enemies")
	ELLKSweeperState GetState() const { return Mind.State; }

	UFUNCTION(BlueprintPure, Category = "LLK|Enemies")
	const FLLKSweeperMind& GetMind() const { return Mind; }

	/** The keeper it is hunting, or null. Server-side truth. */
	UFUNCTION(BlueprintPure, Category = "LLK|Enemies")
	ULLKVitalsComponent* GetTarget() const { return Target; }

	UPROPERTY(BlueprintAssignable, Category = "LLK|Enemies")
	FLLKSweeperTelegraph OnTelegraph;

	/**
	 * The tag a map puts on a patrol point.
	 *
	 * An actor tag rather than a route asset, for the reason
	 * `docs/MAP-READINESS.md` already gives for `LLK.EnemySpawn`: a tag needs
	 * no `.uasset`, no Blueprint and no C++ edit to author, so a greybox can
	 * grow a route out of empty actors and this class never has to know what
	 * shape it is.
	 */
	static const FName PatrolPointTag;

#if WITH_DEV_AUTOMATION_TESTS
	void SetMindForTest(const FLLKSweeperMind& InMind) { Mind = InMind; }
	void SetDefForTest(const FLLKEnemyDef& InDef) { Def = InDef; bHasDef = true; }
#endif

protected:
	/** Measures the world into `FLLKSweeperSenses`. Concludes nothing. */
	FLLKSweeperSenses Observe(float DeltaSeconds);

	/** Picks the keeper to ask about: the nearest one that is still in the run. */
	ULLKVitalsComponent* FindNearestKeeper(float& OutDistanceStuds) const;

	/** Acts on the brain's verdict. The only function that moves this pawn. */
	void Act(const FLLKSweeperStep& Step);

	/** Walks toward a world position along a queried nav path. */
	void StepToward(const FVector& Destination, float DeltaSeconds);

	/** The next authored patrol point, or unset when the map has none. */
	TOptional<FVector> NextPatrolPoint() const;

	bool HasEnemyAuthority() const;

	/** `enemies.json`'s Sweeper row, resolved once from the roster. */
	FLLKEnemyDef Def;

	/**
	 * `lantern.json`'s `sweeperContact`, resolved once beside the def.
	 *
	 * In the lantern's file rather than the enemy's because it is the rule
	 * about a KEEPER being touched - three hits, a cooldown and a decay - and
	 * row 55 keeps all four values Chris's. Unresolved leaves the struct's
	 * zeros, which is a Sweeper that chases and never knocks anybody down:
	 * wrong, and loud about it rather than silent.
	 */
	FLLKSweeperContactRules ContactRules;

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Enemies")
	TSoftObjectPtr<ULLKLanternData> LanternData =
		TSoftObjectPtr<ULLKLanternData>(FSoftObjectPath(TEXT("/Game/Data/DA_Lantern.DA_Lantern")));

	bool bHasDef = false;

	FLLKSweeperMind Mind;

	UPROPERTY()
	TObjectPtr<ULLKVitalsComponent> Target;

	/** Where it is walking, and how long it has been walking there. */
	FVector CurrentDestination = FVector::ZeroVector;

	bool bHasDestination = false;

	/** The patrol point it is heading for, so it does not re-pick every frame. */
	int32 PatrolIndex = INDEX_NONE;

	/**
	 * How close counts as arrived, in STUDS. Converted at the boundary.
	 *
	 * Not a route shape and not spacing: it is the tolerance a nav path needs
	 * so a Sweeper does not orbit its own destination. Three studs is under a
	 * metre.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "LLK|Enemies", meta = (ClampMin = "0.5"))
	float ArriveToleranceStuds = 3.f;

	/** Said once rather than every frame, when the map has nothing to walk to. */
	bool bWarnedAboutNoPatrol = false;

	bool bWarnedAboutNoDef = false;
};
