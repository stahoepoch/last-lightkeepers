// Compiles the generated-data headers and asserts what their shapes promise.
//
// Headers alone would be parsed by UHT but never type-checked as a translation
// unit, so this file exists to make the compiler read them. The static_asserts
// are not ceremony: each one is a claim `docs/DATA-PIPELINE.md` makes about the
// C++ side, and a claim in a document that nothing checks is a claim that drifts.

#include <type_traits>

#include "Data/LLKDataTypes.h"
#include "Data/LLKEnemyDef.h"
#include "Data/LLKTableRows.h"
#include "Data/LLKStormChargeData.h"
#include "Data/LLKLanternData.h"
#include "Data/LLKLootRulesData.h"
#include "Data/LLKWorkshopRulesData.h"

// Every row type must actually be a DataTable row, or the asset writer cannot
// build a table from it and the failure appears at import time instead of here.
static_assert(TIsDerivedFrom<FLLKEnemyDef, FTableRowBase>::IsDerived, "FLLKEnemyDef must be a DataTable row");
static_assert(TIsDerivedFrom<FLLKLootTierDef, FTableRowBase>::IsDerived, "FLLKLootTierDef must be a DataTable row");
static_assert(TIsDerivedFrom<FLLKRoomDef, FTableRowBase>::IsDerived, "FLLKRoomDef must be a DataTable row");
static_assert(TIsDerivedFrom<FLLKRungDef, FTableRowBase>::IsDerived, "FLLKRungDef must be a DataTable row");
static_assert(TIsDerivedFrom<FLLKRelicDef, FTableRowBase>::IsDerived, "FLLKRelicDef must be a DataTable row");
static_assert(TIsDerivedFrom<FLLKUpgradeDef, FTableRowBase>::IsDerived, "FLLKUpgradeDef must be a DataTable row");
static_assert(TIsDerivedFrom<FLLKLandmarkDef, FTableRowBase>::IsDerived, "FLLKLandmarkDef must be a DataTable row");

// Row 12 is LOCKED on the diminishing curve, and says switching back to the
// retired shape must never need a code change. That only holds if the retired
// value stays a real enum entry rather than a comment.
static_assert(static_cast<uint8>(ELLKLanternCurve::LinearCapped) != static_cast<uint8>(ELLKLanternCurve::Diminishing),
	"Row 12: linearCapped must remain a real enum entry so the retired curve stays implemented");

// Row 7: the Warden has no health, which is why FLLKEnemyDef::Health is
// optional rather than defaulted. If this ever becomes a plain float, a zero
// would read as "already dead" to any damage routine.
static_assert(std::is_same_v<decltype(FLLKEnemyDef::Health), TOptional<float>>,
	"Row 7: Health must stay optional so the Warden can have none rather than zero");

namespace LLKDataContract
{
	/** Row 6 in one place: the meter only ever rises, so a multiplier is never below 1. */
	float ClampMultiplier(float Multiplier)
	{
		return Multiplier < 1.f ? 1.f : Multiplier;
	}
}
