// The remaining DataTable row types, one per keyed collection in `data/`.
//
//   loot.json     -> tiers      -> FLLKLootTierDef
//   ledger.json   -> rooms      -> FLLKRoomDef
//   ledger.json   -> rungs      -> FLLKRungDef
//   relics.json   -> relics     -> FLLKRelicDef
//   workshop.json -> upgrades   -> FLLKUpgradeDef
//   town.json     -> landmarks  -> FLLKLandmarkDef
//
// Row names come from the JSON: the object key for `tiers`, and the `id`
// field for every array-shaped collection. `id` is therefore NOT repeated as
// a field - the row name is the id, and storing it twice invites the two to
// disagree.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/LLKDataTypes.h"
#include "LLKTableRows.generated.h"

/** `loot.json` -> `tiers.*`. Row name is the tier key (Supplies, Relics, ...). */
USTRUCT(BlueprintType)
struct FLLKLootTierDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FString> Examples;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	ELLKLootVisibility Visibility = ELLKLootVisibility::Always;

	/**
	 * What this tier pays into. The JSON allows a string OR an array of
	 * strings; the runtime always sees an array. Normalising is the asset
	 * writer's job, not every reader's - see DATA-PIPELINE.md.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FName> Feeds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	int32 CarrySlots = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	float PickupSeconds = 0.f;

	/** Points of burden. The speed penalty per point lives in ULLKLootRulesData. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	int32 Burden = 0;

	/** Optional: author's note on where this tier should be placed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FString SourceHint;

	/** Optional, and true only for RestorationCrates today. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	bool bBlocksCombatWhileCarried = false;
};

/** `ledger.json` -> `rooms.*`. A sealed room in Lanternhold and its crate cost. */
USTRUCT(BlueprintType)
struct FLLKRoomDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	FString Room;

	/** Restoration crates to open it. Row 59: an extracted crate pays every keeper who extracts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	int32 Crates = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	FString Sealed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	FString Restored;

	/** The rung this room unlocks. Must name a row in the rungs table. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	FName Rung;
};

/**
 * `ledger.json` -> `rungs.*.restoration`. Present on some rungs only, so the
 * whole block is optional rather than each field being separately defaulted.
 */
USTRUCT(BlueprintType)
struct FLLKRungRestoration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restoration")
	FName RequiresRoom;

	/** Loot tier name -> amount. Keys must name rows in the loot tier table. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restoration")
	TMap<FName, int32> Salvage;

	/**
	 * Enemy families this rung wakes even for a crew running dark. Row 5 says
	 * light is the primary cost; this is the list of things that cost you
	 * anyway, which is why restoring something is never purely a gain.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restoration")
	TArray<FName> WakesWithoutLantern;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restoration")
	float UnpoweredSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restoration")
	float PoweredSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restoration")
	float UnpoweredFalseStartChance = 0.f;
};

/** `ledger.json` -> `rungs.*`. */
USTRUCT(BlueprintType)
struct FLLKRungDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	FString Job;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	FString Opens;

	/** What restoring this wakes up. Row 38: expansion arrives as more danger. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	FString Wakes;

	/** Unset on rungs that are not restoration contracts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledger")
	TOptional<FLLKRungRestoration> Restoration;
};

/** `relics.json` -> `relics.*`. Row 53: visible in darkness, unreadable without light. */
USTRUCT(BlueprintType)
struct FLLKRelicDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FString Found;

	/** The reward itself. Reading it is what the lantern buys. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relic")
	FString Text;
};

/** `workshop.json` -> `upgrades.*`. */
USTRUCT(BlueprintType)
struct FLLKUpgradeDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workshop")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workshop")
	FString Blurb;

	/**
	 * Currency name -> amount. Keys are loot tier names and the importer
	 * rejects any that is not in `workshop.json` -> `currency`, which is
	 * itself checked against the loot tiers. Relics are deliberately not a
	 * currency: row 53 makes them the reward, not the fuel.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workshop")
	TMap<FName, int32> Cost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workshop")
	float RelicBurdenMultiplier = 1.f;

	/** Optional: an upgrade or rung that must exist first. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workshop")
	FName Requires;
};

/** `town.json` -> `landmarks.*`. The long tail - 85 of the 105 crates. */
USTRUCT(BlueprintType)
struct FLLKLandmarkDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town")
	int32 Crates = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Town")
	FString Restored;
};
