// The six families, loaded once and queryable. M2.4's first half.
//
// **Why this has no fallback, when every other data reader in the project
// does.** `ULLKStormChargeSubsystem::MakeFallbackRules` and the lantern's
// declared defaults exist because a missing asset must not stop the game
// being playable, and because a silent fallback is how "gameplay reads
// `data/`" quietly stops being true. Both of those arguments reverse here.
//
// An enemy running on invented radii is worse than an enemy that does not
// spawn. A Sweeper with a fallback `aggroRadiusLit` would hunt keepers on a
// number nobody authored, and the player would be learning a lantern rule the
// data does not contain - which is precisely the lesson this game is built to
// teach correctly. The lantern taught this the hard way in the other
// direction: `DA_Lantern` resolving to zero values produced a light that was
// present, legal and invisible, and cost an evening to find. The lesson was
// not "always have a fallback", it was **"never let a degenerate resolution
// pass for a real one"** - and for perception the honest degenerate answer is
// no roster at all.
//
// So: `FindFamily` returns nullptr when the table is missing, `GetSource`
// says `Unresolved` loudly, and callers get nothing rather than something
// plausible. `LLK.Enemies.Roster` asserts the nullptr path rather than
// treating it as an edge case.
//
// The perception rule itself takes an `FLLKEnemyDef&` directly and never
// touches this class, so it stays testable with no world, no asset and no
// roster - which is also why a missing table costs the test suite nothing.

#pragma once

#include "CoreMinimal.h"
#include "Data/LLKDataTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "LLKEnemyRosterSubsystem.generated.h"

class UDataTable;
struct FLLKEnemyDef;

UCLASS()
class LLK_API ULLKEnemyRosterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * The authored row for a family, or nullptr.
	 *
	 * Matched on the row's own `Family` field rather than on its row name.
	 * The importer writes both from `enemies.json`, so they agree today;
	 * matching the field means a renamed row keeps working and a row whose
	 * name and family disagree is found by `LLK.Enemies.Roster` rather than
	 * by a player wondering why one enemy behaves like another.
	 */
	const FLLKEnemyDef* FindFamily(ELLKEnemyFamily Family) const;

	/** Every row the table actually contained, in table order. */
	TArray<const FLLKEnemyDef*> GetAllFamilies() const;

	/** Where the roster came from. Read this rather than trusting a log line. */
	UFUNCTION(BlueprintPure, Category = "LLK|Enemies")
	ELLKDataSource GetSource() const { return Source; }

	/**
	 * How far this family notices a keeper in this light, in studs.
	 *
	 * The convenience `LLKStoreFloorPlan`'s sightline rule wants: it is
	 * written against `Sweepers.aggroRadiusLit` being 38 and currently
	 * restates the number in a comment. Returns 0 for an unknown family,
	 * which is the same "no plausible guess" rule as everything else here.
	 */
	float NoticeRangeStuds(ELLKEnemyFamily Family, float Brightness, float LitThreshold) const;

#if WITH_DEV_AUTOMATION_TESTS
	/** Injects a table so a test is not at the mercy of whether Content/Data was generated. */
	void SetTableForTest(UDataTable* InTable)
	{
		Table = InTable;
		Source = InTable != nullptr ? ELLKDataSource::Asset : ELLKDataSource::Unresolved;
	}
#endif

protected:
	/** Loads `/Game/Data/DT_Enemies`, or stays Unresolved and says so once. */
	void ResolveRoster();

	UPROPERTY()
	TObjectPtr<UDataTable> Table;

	ELLKDataSource Source = ELLKDataSource::Unresolved;
};
