// The ground floor, as numbers. A disposable scaffold, not the Galleria.
//
// **What this is for.** Chris, 2026-10-29: *"build the ground-floor
// department-store scaffold in code, not as a final Unreal map... It is
// disposable greybox, not your final Galleria. You do not need to model,
// furnish, or build it. You play it and say 'this feels too square', 'this
// ramp feels wrong', 'I need more department-store density'."*
//
// So every number here is in **studs**, named, and commented with why it is
// that number - because the whole point is that he changes them by eye and
// tells me. A plan with unexplained numbers is a plan nobody can argue with.
//
// **What this is NOT.** It cannot be art-passed. A code-built floor is a
// scaffold for finding the fun; when the licensed kit and the authored map
// arrive, this gets deleted exactly like the twelve-pillar arena did. It buys
// time to answer "is this fun" before anybody spends money or an artist's
// week on a floor plan that might be wrong.
//
// **The three numbers everything else is tuned against**, all from `data/`:
//
//   - The lantern reaches **30 studs** (`lantern.json` `light.radiusStuds`).
//   - A lit keeper is noticed from **38 studs** (`enemies.json`
//     `Sweepers.aggroRadiusLit`), further than they can see.
//   - A dark keeper is noticed from **12 studs** (`aggroRadiusDark`).
//
// Which gives the floor its one hard rule: **no unbroken sightline longer
// than 38 studs.** Past that a keeper can be hunted by something they had no
// way of seeing, which is not tension, it is unfairness - and
// `tests/balance.test.mjs` already enforces the relationship those two
// numbers have to each other.
//
// **And the heights are row 63's prep.** The jump apex is 56cm
// (`ALLKCharacter`), so a 3.2-stud counter at ~90cm **cannot** be jumped and
// must eventually be mantled, while a 1.8-stud service ledge at ~50cm can be
// cleared. That contrast is deliberate: it is the difference a vault has to
// justify, and it wants to exist in the geometry before the vault is built
// against it.

#pragma once

#include "CoreMinimal.h"
#include "LLKStoreFloorPlan.generated.h"

/**
 * The ground floor's dimensions, in STUDS.
 *
 * Studs throughout, converted once at the boundary by `LLKUnits` (row 61).
 * A harness is exactly where raw centimetres creep in, and this one is big
 * enough that a single missed conversion would be a floor a hundred times too
 * small rather than something anybody noticed.
 */
USTRUCT(BlueprintType)
struct FLLKStoreFloorPlan
{
	GENERATED_BODY()

	// ---- the shell ------------------------------------------------------

	/** Depth, street wall to back wall. 120 studs is about 34 metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shell", meta = (ClampMin = "40"))
	float DepthStuds = 120.f;

	/** Width. 90 studs is about 25 metres - a real department store floor plate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shell", meta = (ClampMin = "40"))
	float WidthStuds = 90.f;

	/**
	 * Wall height. 14 studs is nearly 4 metres.
	 *
	 * Retail ceilings are high, and it matters here for a specific reason:
	 * the lantern is a cone pointing where the keeper looks, so a low ceiling
	 * bounces light back and flattens the trade. A tall room lets the dark
	 * stay dark overhead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shell", meta = (ClampMin = "6"))
	float WallHeightStuds = 14.f;

	// ---- the rule the layout exists to obey ------------------------------

	/**
	 * The longest sightline the floor is allowed to contain.
	 *
	 * Must stay under `Sweepers.aggroRadiusLit` (38). Past that a lit keeper
	 * can be hunted by something beyond their own vision with no warning,
	 * which reads as the game cheating rather than as danger. 34 leaves a
	 * margin without making the floor feel like a maze of small rooms.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sightlines", meta = (ClampMin = "12"))
	float MaxSightlineStuds = 34.f;

	/**
	 * How many times the shop-floor concourse is interrupted.
	 *
	 * **This number exists because the first version of this plan lied.**
	 * `LongestStraightStuds` used to halve the depth twice - once for "broken
	 * at the midpoint", once for no geometric reason at all - and reported 35
	 * studs for a concourse that actually ran 60 clear from the street wall to
	 * the mirrored columns. `LLK.Harness.StoreFloorPlan` failed on its first
	 * run and the layout was wrong, not the test.
	 *
	 * So the breaks are counted, the builder places a blocker at each one,
	 * and the longest leg is derived from the count rather than asserted. At
	 * 3 breaks across 120 studs of depth the legs are 30 studs - inside the
	 * 34 limit, and inside the 38 a lit keeper is noticed from.
	 *
	 * Each break is eye-height or taller. A counter at 90cm does not break a
	 * sightline; a gondola run at 1.7m does.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sightlines", meta = (ClampMin = "1"))
	int32 ConcourseBreaks = 3;

	/**
	 * How many times the service corridor jogs.
	 *
	 * A corridor the full depth of the building is a 120-stud sightline,
	 * which is the worst offender on the floor and the easiest to miss
	 * because a corridor is *supposed* to be long. Three jogs give 30-stud
	 * legs, so somebody looking down it sees a corner rather than the way
	 * out.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sightlines", meta = (ClampMin = "1"))
	int32 CorridorJogs = 3;

	// ---- the departments -------------------------------------------------

	/** Cosmetics: glass counters in a grid. Short sightlines, baseline loot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Departments", meta = (ClampMin = "1"))
	int32 CosmeticsRows = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Departments", meta = (ClampMin = "1"))
	int32 CosmeticsColumns = 4;

	/**
	 * Jewellery: locked glass cases, and where the relics are.
	 *
	 * `THE-STORE.md`: *"Small valuable things were kept in locked glass on the
	 * ground floor of every department store that ever existed, and the locks
	 * are the hold."*
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Departments", meta = (ClampMin = "1"))
	int32 JewelleryCases = 6;

	// ---- the heights that teach ------------------------------------------

	/**
	 * Counter height. 3.2 studs is about 90cm - a real shop counter.
	 *
	 * Deliberately ABOVE the 56cm jump apex, so a counter must be mantled and
	 * cannot be hopped. Row 63's vault needs something it is the only answer
	 * to, or it is a slower way to do something you could already do.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heights", meta = (ClampMin = "0.5"))
	float CounterHeightStuds = 3.2f;

	/**
	 * Service ledge. 1.8 studs is about 50cm - just under the jump apex.
	 *
	 * The contrast is the point: one obstacle you clear and one you cannot,
	 * in the same building, so the difference is learnable by trying.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heights", meta = (ClampMin = "0.2"))
	float ServiceLedgeHeightStuds = 1.8f;

	/** Shelving. 6 studs is about 1.7m - above eye level, so it breaks sightlines. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heights", meta = (ClampMin = "1"))
	float ShelfHeightStuds = 6.f;

	// ---- the escalator well, which is the landmark -----------------------

	/**
	 * How high the dead escalator climbs. 7 studs is about 2 metres.
	 *
	 * `THE-STORE.md` gives every floor *"one landmark object that exists
	 * nowhere else in the building, and which is how players name the place
	 * out loud."* On the ground floor that is the escalator well, and it does
	 * double duty here as Chris's inclined transition.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escalator", meta = (ClampMin = "1"))
	float MezzanineHeightStuds = 7.f;

	/**
	 * Ramp run. 20 studs against a 7-stud rise is about 19 degrees.
	 *
	 * Gentle on purpose - Chris asked for *"a gentle inclined plane"*, and
	 * Unreal walks up to 44 degrees by default, so this is comfortably
	 * walkable while still reading as a climb. Shorten the run to steepen it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escalator", meta = (ClampMin = "4"))
	float RampRunStuds = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escalator", meta = (ClampMin = "2"))
	float RampWidthStuds = 8.f;

	// ---- the service route ----------------------------------------------

	/**
	 * Back-of-house corridor width. 8 studs is about 2.2m.
	 *
	 * Narrow enough that a lantern fills it and a keeper cannot be flanked,
	 * which is what makes it an escape rather than a shortcut. It jogs
	 * halfway along so the straight run stays under the sightline rule.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Service", meta = (ClampMin = "3"))
	float ServiceCorridorWidthStuds = 8.f;

	/** Gaps in the internal wall. Two doors, so the route is a choice and not a funnel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Service", meta = (ClampMin = "2"))
	float ServiceDoorWidthStuds = 5.f;

	/** The longest straight this plan actually produces, for the test to check. */
	float LongestStraightStuds() const;

	/** True when the plan obeys its own sightline rule. */
	bool ObeysSightlineRule() const { return LongestStraightStuds() <= MaxSightlineStuds; }

	/** Ramp angle in degrees, derived rather than authored. */
	float RampAngleDegrees() const;
};
