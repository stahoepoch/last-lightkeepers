// "Is this map ready to have an enemy put in it." M2.5 support.
//
// **The thing this exists to prevent is one specific afternoon.** An enemy is
// placed in the greybox, it stands still, and the next four hours go into the
// behaviour tree - because an inert enemy looks identical whether its brain is
// broken, its navmesh was never built, the nav bounds volume stops short of
// the back of house, or the floor it is standing on has no collision for a
// trace to hit. Three of those four are map problems that a check can name in
// a line, and naming them is cheaper than any amount of debugging that starts
// from the wrong premise.
//
// `docs/MAP-READINESS.md` is the contract in prose, with a row per check code
// and the reason each one is on the list. This file is that contract as code.
//
// **Two halves, on purpose.** `Evaluate` is pure - observations in, findings
// out, no `UWorld` - so the judgement is testable without a level, and
// `LLK.Harness.MapReadiness` tests it with hand-built observations that no
// real map would produce. `Observe` is the half that touches the world, and it
// is deliberately thin: it measures and records, and decides nothing.
//
// **Chris owns the map (row 41) and this never changes it.** Nothing here
// spawns, moves, repairs or saves anything. It reports.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/LLKVisibility.h"
#include "LLKMapReadiness.generated.h"

struct FLLKStoreFloorPlan;
class UWorld;

/**
 * What a map can be missing.
 *
 * One value per row of `docs/MAP-READINESS.md`, named the same, so a log line
 * and the document are searchable by the same word. A finding is never a bare
 * sentence for that reason: sentences get reworded and then the document and
 * the code disagree about what the failure is called.
 */
UENUM(BlueprintType)
enum class ELLKMapCheck : uint8
{
	/** No `APlayerStart`. Nothing to measure anything else from. */
	NoPlayerSpawn,

	/** No actors tagged `LLK.EnemySpawn`. Nowhere for M2.6 to put the Sweeper. */
	NoEnemySpawnPoints,

	/** Nothing solid under the keeper spawn. Requirement 1. */
	FloorHasNoCollision,

	/** No `NavMeshBoundsVolume`. Requirement 2: no volume, no navmesh, ever. */
	NavBoundsMissing,

	/** A volume but no built nav data. Usually a level that was not saved. */
	NavDataMissing,

	/**
	 * Nav data exists and has not finished generating. Requirement 3a.
	 *
	 * Distinct from `NavDataMissing`, and the distinction is the fix: missing
	 * means no nav data actor at all, usually a level saved before the volume
	 * was added. Not built means the actor is there and generation has not
	 * completed - which also happens to a check that runs too early, since
	 * navmesh generation is asynchronous and "the level is loaded" is not
	 * "the navmesh is ready".
	 *
	 * Only reported when a nav data actor exists. On a map with none, this
	 * and `NavDataMissing` would be the same failure said twice.
	 */
	NavmeshNotBuilt,

	/** A spawn point outside every nav bounds volume. Requirement 4. */
	NavBoundsDoesNotCoverSpawn,

	/**
	 * The keeper spawn does not project onto navigable space. Requirement 4a.
	 *
	 * **Different from `NavBoundsDoesNotCoverSpawn`, and they look identical
	 * in the viewport.** That one is a box test against the volumes' bounds
	 * and answers even when the navmesh is not built. This one is a
	 * projection onto the generated mesh, and it catches what a box cannot: a
	 * `PlayerStart` sunk into the floor, floating above it, or standing on a
	 * counter. Both are kept because they fail on different days and are
	 * fixed by different moves.
	 */
	SpawnOffNavmesh,

	/** Somebody asked for a route and the answer was no. */
	NoRouteFromSpawn,

	/**
	 * Nobody asked for a route.
	 *
	 * **Reported as a finding, not treated as a pass.** A navmesh path query
	 * needs the engine's `NavigationSystem` module and
	 * `Source/LLK/LLK.Build.cs` is Codex's lane, so until that one line
	 * lands this appears on every clean map. That is correct and is meant to
	 * be annoying: a check that returns "fine" when it did not run is worse
	 * than no check, and `ULLKEnemyRosterSubsystem` already set the
	 * precedent that a missing input degrades to inert rather than to
	 * invented.
	 */
	RouteUnverified,

	/** No sight-blocking geometry anywhere near the spawn. Requirement 6. */
	NoOccludingGeometry,

	/** A sightline longer than the plan's limit. Requirement 7. */
	SightlineTooLong,

	/** Not exactly two actors tagged `LLK.TestLane.Open`. */
	TestLaneOpenMissing,

	/** The open lane reported blocked, so a `Clear` answer is unproven. */
	TestLaneOpenBlocked,

	/** Not exactly two actors tagged `LLK.TestLane.Blocked`. */
	TestLaneBlockedMissing,

	/** The blocked lane reported clear, so a `Blocked` answer is unproven. */
	TestLaneBlockedClear
};

/** One thing that is wrong, and where. */
USTRUCT(BlueprintType)
struct FLLKMapFinding
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	ELLKMapCheck Check = ELLKMapCheck::NoPlayerSpawn;

	/**
	 * The measurement that produced the finding, for the log line.
	 *
	 * Numbers, not advice. "longest sightline 61.4 studs, limit 34" tells
	 * Chris where to put a shelf; "consider adding occlusion" does not.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	FString Detail;
};

/** Everything that is wrong, or nothing. */
USTRUCT(BlueprintType)
struct FLLKMapReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	TArray<FLLKMapFinding> Findings;

	bool IsReady() const { return Findings.IsEmpty(); }

	/** True when this exact check is among the findings. For tests. */
	bool Has(ELLKMapCheck Check) const;

	/** One line per finding, or one line saying the map is ready. */
	FString Describe() const;
};

/**
 * What was measured in the world, with nothing concluded from it.
 *
 * **`TOptional` where nobody may have looked**, which is the same distinction
 * `FLLKEnemyDef` draws between an absent field and a zero one. A route that
 * was not queried is not a route that failed, and a struct that cannot tell
 * those apart forces `Evaluate` to guess.
 */
USTRUCT(BlueprintType)
struct FLLKMapObservations
{
	GENERATED_BODY()

	// ---- the things that must exist --------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	bool bHasPlayerSpawn = false;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	int32 EnemySpawnPoints = 0;

	/** A downward trace from above the keeper spawn hit something solid. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	bool bFloorUnderSpawn = false;

	// ---- navigation -------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	int32 NavBoundsVolumes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	int32 NavDataActors = 0;

	/**
	 * Spawn points (keeper or enemy) that fell outside every nav bounds
	 * volume. Counted rather than listed, with the first one's position in
	 * the finding's detail - a list of twelve positions in a log line is a
	 * line nobody reads.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	int32 SpawnPointsOutsideNavBounds = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	FVector FirstSpawnPointOutsideNavBounds = FVector::ZeroVector;

	/**
	 * Whether the navmesh has finished generating.
	 *
	 * Unset means nobody asked. All three of these fields are filled by the
	 * same seam in `Observe`, so unset on one is normally unset on all three,
	 * and `RouteUnverified` is the single finding that covers that - rather
	 * than three codes all saying the same thing about the same missing
	 * query.
	 */
	TOptional<bool> NavigationBuilt;

	/**
	 * Whether the keeper spawn projects onto navigable space.
	 *
	 * Unset means nobody asked. **Not inferable from the box test**: a point
	 * inside a nav bounds volume can still be off-mesh, which is the whole
	 * reason this is a separate question.
	 */
	TOptional<bool> SpawnOnNavmesh;

	/**
	 * Whether a walkable route reaches every enemy spawn from the keeper
	 * spawn. **Unset means nobody asked**, which `Evaluate` reports as
	 * `RouteUnverified` rather than silently accepting.
	 */
	TOptional<bool> RouteFromSpawnReaches;

	// ---- sightlines and occlusion ----------------------------------------

	/** How many directions were traced from the keeper spawn at eye height. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	int32 ProbedDirections = 0;

	/** How many of them hit something inside the lit notice range. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	int32 BlockedDirections = 0;

	/** The longest unblocked one, in studs. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	float LongestSightlineStuds = 0.f;

	/** The plan's limit, carried along so `Evaluate` needs no plan. */
	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	float MaxSightlineStuds = 34.f;

	// ---- the two test lanes ----------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	int32 OpenLaneEnds = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LLK|Map")
	int32 BlockedLaneEnds = 0;

	/** Unset when the lane was not traced because it did not have two ends. */
	TOptional<ELLKVisibility> OpenLaneResult;
	TOptional<ELLKVisibility> BlockedLaneResult;
};

namespace LLKMapReadiness
{
	/** Actor tag marking a place M2.6's enemies may start. */
	LLK_API extern const FName EnemySpawnTag;

	/** Actor tags marking the two ends of each line-of-sight test lane. */
	LLK_API extern const FName TestLaneOpenTag;
	LLK_API extern const FName TestLaneBlockedTag;

	/**
	 * A keeper's eye, 152cm above the floor.
	 *
	 * The camera sits 64cm above the capsule's centre
	 * (`LLKCharacter.cpp`) and the capsule's default half-height is 88cm.
	 * Every sightline trace runs at this height, which makes it the number a
	 * sight break has to beat - and the reason the floor plan's shelving is 6
	 * studs (168cm) rather than 5 (140cm).
	 */
	LLK_API extern const float EyeHeightCm;

	/**
	 * The judgement. Pure: observations in, findings out, no world.
	 *
	 * Order matters in one place only - `NoPlayerSpawn` short-circuits the
	 * measurements that are taken from the spawn, because reporting a 0-stud
	 * sightline measured from nowhere is noise on top of a real failure.
	 */
	LLK_API FLLKMapReport Evaluate(const FLLKMapObservations& Observations);

	/**
	 * The measuring. Traces, iterates actors, concludes nothing.
	 *
	 * Read-only with respect to the world. Takes a mutable UWorld only because
	 * NavigationSystem's query API requires one. Safe to call in PIE, in a
	 * packaged build, and from a commandlet.
	 */
	LLK_API FLLKMapObservations Observe(UWorld* World, const FLLKStoreFloorPlan& Plan);

	/** Both halves, for the console command and for anything else that wants it. */
	LLK_API FLLKMapReport Check(UWorld* World, const FLLKStoreFloorPlan& Plan);

	/** The code's name, which is the same word `docs/MAP-READINESS.md` uses. */
	LLK_API FString NameOf(ELLKMapCheck Check);
}
