// `workshop.json` -> ULLKWorkshopRulesData. Holds `currency` only; `upgrades`
// is a DataTable of FLLKUpgradeDef.
//
// This was the one top-level key in the eight files with no home in the
// contract, which would have made the importer hard-error on it under rule 2.
// Added to `docs/DATA-PIPELINE.md` on 2026-10-28.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LLKWorkshopRulesData.generated.h"

UCLASS(BlueprintType)
class LLK_API ULLKWorkshopRulesData : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Which loot tiers can be spent. Today: Supplies, Components.
	 *
	 * These are FNames rather than a UENUM, and that is a deliberate
	 * refinement of DATA-PIPELINE.md rule 4 rather than a shortcut - see the
	 * note added there. Rule 4 says enum strings "parse strictly", which for
	 * `lantern.curve` means a real UENUM because its vocabulary lives only in
	 * code. This vocabulary does not: it is the row names of the loot tier
	 * table, checked by the importer against `loot.json` -> `tiers`. A UENUM
	 * here would be a second copy of a list the data already owns, and the
	 * two could drift. The closed set is enforced at import; it is simply
	 * enforced against the data rather than against C++.
	 *
	 * Relics are deliberately absent. Row 53 makes them the reward, and
	 * tests/balance.test.mjs asserts no relic appears in a progression cost.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Workshop")
	TArray<FName> Currency;

	bool IsCurrency(FName Tier) const { return Currency.Contains(Tier); }
};
