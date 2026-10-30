// `lantern.json` -> ULLKLanternData. The keeper's own numbers.
//
// Several values here are deliberately 0 and must not be "fixed" by anyone who
// reads a zero as unset: `knockdown.downedWalkSpeed` is 0 because a downed
// keeper cannot move at all, and `knockdown.bleedOutSeconds` is 0 because
// downed keepers wait indefinitely rather than dying on a timer. That is why
// the family-specific fields on FLLKEnemyDef are TOptional and these are not -
// here, zero IS the value.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LLKLanternData.generated.h"

USTRUCT(BlueprintType)
struct FLLKLanternChargeState
{
	GENERATED_BODY()

	/** 0 = the lantern does not deplete on a timer in v1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge")
	float SecondsOfLightPerFullCharge = 0.f;

	/** True in `lantern.json`. The C++ default must agree with the data: on the
	 *  fallback path the default IS the value, so a default that disagrees is a
	 *  silent disagreement. Armed but never fired while the charge above is 0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge")
	bool bAutoOffAtEmpty = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Charge")
	bool bRechargeAtRelay = false;
};

USTRUCT(BlueprintType)
struct FLLKKnockdownRules
{
	GENERATED_BODY()

	/** 0: a downed keeper cannot crawl. Deliberate - see the header note. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockdown")
	float DownedWalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockdown")
	bool bLanternForcedOffWhenDowned = true;

	/**
	 * False, and load-bearing for row 59. With DownedWalkSpeed at 0, a downed
	 * carrier cannot move and keeps the crate, so a crate cannot be banked
	 * while its carrier is on the floor. That is what physically enforces
	 * "everyone protects the carrier". Setting this true would let a crew
	 * abandon the carrier and still extract the crate.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockdown")
	bool bDropsCarriedCrate = false;

	/** 0 = downed keepers wait indefinitely. The wipe comes from the whole crew going down. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockdown")
	float BleedOutSeconds = 0.f;
};

USTRUCT(BlueprintType)
struct FLLKReviveRules
{
	GENERATED_BODY()

	/** Server-validated hold. Moving out of range cancels it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Revive")
	float TeammateHoldSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Revive")
	float MaxReviveDistance = 0.f;
};

USTRUCT(BlueprintType)
struct FLLKSweeperContactRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	bool bKnockdownOnContact = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	float ContactRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	float ContactCooldownSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	int32 HitsToKnockDown = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contact")
	float HitDecaySeconds = 0.f;
};

/**
 * `light.warmColor` - linear RGB, no alpha.
 *
 * Its own struct rather than `FLinearColor` on purpose. The JSON converter
 * reads an `FLinearColor` from a hex string through `FColor::FromHex`, which
 * silently applies an sRGB-to-linear conversion, and from an object with no
 * alpha key it would leave alpha at 0 - a colour that looks broken in the
 * editor and means nothing to a light. Three declared floats have neither
 * problem, and `data/lantern.json` says plainly that alpha is absent because
 * a light has no opacity.
 */
USTRUCT(BlueprintType)
struct FLLKLightColor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta = (ClampMin = "0", ClampMax = "1"))
	float R = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta = (ClampMin = "0", ClampMax = "1"))
	float G = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Color", meta = (ClampMin = "0", ClampMax = "1"))
	float B = 1.f;

	/** Alpha is always 1: a light has no opacity, so there is nothing to author. */
	FLinearColor ToLinearColor() const { return FLinearColor(R, G, B, 1.f); }
};

/**
 * `light` - the lantern's reach, and the one block here that gameplay other
 * than the lantern will read.
 *
 * Chris, 2026-10-29: "the actual reach must be portable, tunable, and shared
 * by light visibility/enemy logic later". These were C++ constants on
 * `ULLKLanternLightComponent` until that call, which put the most
 * consequential number in the game - how far a keeper can see - somewhere the
 * balance tests could not read it.
 *
 * `RadiusStuds` is the one to read against `data/enemies.json`. With
 * `aggroRadiusLit: 38` and a radius of 30, a lit keeper is noticed from
 * further away than they can see, which is row 5's trade made physical.
 *
 * What is NOT here is as deliberate as what is: the inner cone angle softens
 * the falloff and changes nothing about reach, and mesh placement and
 * cosmetic flicker are `Content/` and Chris's (row 41).
 */
/**
 * One named setting of the lantern. Chris's call, 2026-10-30.
 *
 * *"Build it after the exposure pass as three named states: Hooded, Working,
 * Open... The important implementation rule: each level needs a separate
 * authored 'detectability' value, not just intensity/radius. Otherwise a dim
 * lantern still marks its owner as fully lit."*
 *
 * **Why `Detectability` is authored and not derived, which is the whole point
 * of this struct.** The obvious build is a dial over `Intensity`, and it does
 * nothing: every light threshold in the game is `I * 0.5f`, so "lit" reduces
 * to `d < r` and the intensity cancels out (`docs/LIGHTING-IN-UNREAL.md` has
 * the derivation). The second-obvious build is a dial over `RadiusStuds`, and
 * it does nothing either, for a reason that is invisible until you look for
 * it: `ALLKSweeper` asks for light at the keeper's OWN position, where their
 * own lantern sits at distance zero, so brightness there is `I` against a
 * threshold of `I/2`. A keeper holding a lit lantern is lit at any radius.
 * Hooding would cost sight and buy no concealment.
 *
 * So the thing enemies read is its own number. Nothing computes it, which is
 * what makes a hooded lantern mean less attention rather than only less light.
 *
 * `Intensity` is deliberately NOT here. It stays one value for the whole
 * block, shared by every level, and that is what keeps it a free visual knob:
 * relic legibility cancels it and perception no longer reads it at all. A
 * per-level intensity would make every candela figure a balance change and end
 * the lighting pass being safe to do by eye.
 */
USTRUCT(BlueprintType)
struct FLLKLanternLightLevel
{
	GENERATED_BODY()

	/** `Hooded`, `Working`, `Open`. Named rather than indexed so a log line and a test read the same. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light")
	FName Name;

	/** In STUDS. What `d < r` compares against at this level, so it is this level's reach. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0"))
	float RadiusStuds = 0.f;

	/** The OUTER cone at this level. Hooded is narrow spill, Open is a work light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0", ClampMax = "180"))
	float ConeAngleDegrees = 0.f;

	/**
	 * How much this level marks its holder, from 0 (unlit) to 1 (today's lit).
	 *
	 * Read against `data/enemies.json`: it is the fraction of the way from
	 * `aggroRadiusDark` to `aggroRadiusLit` that a keeper under this light sits.
	 * At 1 a Sweeper notices from 38 studs, at 0 from 12, and the ends are
	 * exactly the two states the game had before levels existed - so a single
	 * level authored at 1 is bit-for-bit today's behaviour.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0", ClampMax = "1"))
	float Detectability = 1.f;

	bool DescribesARealLevel() const
	{
		return RadiusStuds > 0.f && ConeAngleDegrees > 0.f && !Name.IsNone();
	}
};

USTRUCT(BlueprintType)
struct FLLKLanternLightRules
{
	GENERATED_BODY()

	/** In STUDS, like every other spatial value in `data/`. Row 61 converts at the boundary. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0"))
	float RadiusStuds = 0.f;

	/** The OUTER cone: the edge of what is lit, and the half other systems read. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0", ClampMax = "180"))
	float ConeAngleDegrees = 0.f;

	/** Unreal candelas. The one key here with no portable meaning - see the JSON note. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0"))
	float Intensity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light")
	FLLKLightColor WarmColor;

	/**
	 * True in `lantern.json`, and the C++ default must agree: on the fallback
	 * path the default IS the value. Row 5 is about what light reveals, and
	 * revealed geometry with no shadow reads as flat.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light")
	bool bCastsShadows = true;

	/**
	 * The named settings, in order from dimmest to brightest.
	 *
	 * **Empty is a valid and expected state**, and that is deliberate rather
	 * than unfinished: `DA_Lantern` is generated by `Tools/import_data.py`
	 * (Codex's lane) and will carry no levels until `light.levels` is added to
	 * `data/lantern.json` and the importer is taught the key. Until then
	 * `ResolveLevel` synthesises a single level from `RadiusStuds`,
	 * `ConeAngleDegrees` and a detectability of 1 - which is exactly the
	 * behaviour the game had before this struct existed.
	 *
	 * That is the same lesson `DescribesARealLight` below was written for: a
	 * resolved-but-stale asset is more dangerous than a missing one, so the
	 * degenerate path has to be a declared behaviour rather than a surprise.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light")
	TArray<FLLKLanternLightLevel> Levels;

	/**
	 * The level a keeper's lantern starts on, by name.
	 *
	 * Chris: *"Working: current practical lantern."* Resolved by name rather
	 * than by index so that reordering `levels` in the JSON cannot silently
	 * change what a lantern switches on to.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light")
	FName DefaultLevelName = TEXT("Working");

	/**
	 * The level at `Index`, or the synthesised single level described above.
	 *
	 * Clamps rather than fails: an index out of range is a caller bug, and the
	 * safe degradation for a light is to shine at an authored setting rather
	 * than to go dark or to shine at zero.
	 */
	FLLKLanternLightLevel ResolveLevel(int32 Index) const
	{
		if (Levels.Num() > 0)
		{
			return Levels[FMath::Clamp(Index, 0, Levels.Num() - 1)];
		}

		FLLKLanternLightLevel Synthesised;
		Synthesised.Name = TEXT("Working");
		Synthesised.RadiusStuds = RadiusStuds;
		Synthesised.ConeAngleDegrees = ConeAngleDegrees;
		Synthesised.Detectability = 1.f;
		return Synthesised;
	}

	/** Where `DefaultLevelName` sits, or 0 when it names nothing - never -1. */
	int32 DefaultLevelIndex() const
	{
		const int32 Found = Levels.IndexOfByPredicate(
			[this](const FLLKLanternLightLevel& Level) { return Level.Name == DefaultLevelName; });
		return Found == INDEX_NONE ? 0 : Found;
	}

	/** How many settings a keeper can cycle through. Always at least one. */
	int32 LevelCount() const { return FMath::Max(1, Levels.Num()); }

	/**
	 * Whether these values describe a real light.
	 *
	 * **This exists because of a bug found in playtest on 2026-10-29, and the
	 * shape of it is worth more than the fix.** `DA_Lantern` had been
	 * generated a day earlier, before this block existed. So the asset
	 * RESOLVED and supplied zeros - and every "declared fallback that
	 * announces itself" guard in this project protects against an asset that
	 * is MISSING, not one that is stale. The component reported "light from
	 * ULLKLanternData" and shone nothing.
	 *
	 * A resolved-but-degenerate asset is more dangerous than an absent one,
	 * because the absent case is loud and this case looks like data. It had
	 * two consequences from one cause: an invisible lantern, which Chris found
	 * in thirty seconds, and a read threshold of zero that made every relic
	 * legible in pitch darkness, which has no symptom at all and would have
	 * quietly deleted row 53.
	 *
	 * So anything reading this block asks first. Zero radius and zero
	 * intensity are not tunings anybody would author - unlike
	 * `downedWalkSpeed: 0`, where zero IS the value - which is what makes
	 * this test safe to apply here and wrong to apply there.
	 */
	bool DescribesARealLight() const
	{
		return RadiusStuds > 0.f && Intensity > 0.f && ConeAngleDegrees > 0.f;
	}
};

USTRUCT(BlueprintType)
struct FLLKMovementRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CrouchSpeed = 0.f;
};

UCLASS(BlueprintType)
class LLK_API ULLKLanternData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKLanternChargeState Charge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKKnockdownRules Knockdown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKReviveRules Revive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKSweeperContactRules SweeperContact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKLanternLightRules Light;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern")
	FLLKMovementRules Movement;
};
