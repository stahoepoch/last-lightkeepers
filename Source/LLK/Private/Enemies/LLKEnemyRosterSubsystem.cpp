#include "Enemies/LLKEnemyRosterSubsystem.h"

#include "Data/LLKEnemyDef.h"
#include "Enemies/LLKPerception.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "LLK.h"

bool ULLKEnemyRosterSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// Game and PIE only, as with the storm and the run state. An editor
	// preview world loading the enemy roster would be a strange thing to
	// debug, and the roster is read by gameplay rather than by tooling.
	const UWorld* World = Cast<UWorld>(Outer);
	return World != nullptr
		&& (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void ULLKEnemyRosterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResolveRoster();
}

void ULLKEnemyRosterSubsystem::ResolveRoster()
{
	// Generated from `data/enemies.json` by `Tools/import_data.py`, and owned
	// by nobody - `Content/Data/` is the one exception to row 41.
	Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Enemies.DT_Enemies"));

	if (Table == nullptr)
	{
		Source = ELLKDataSource::Unresolved;
		// Warning, not an error, and said once: a world with no enemies yet
		// is the normal state of this project today. But it must be visible,
		// because "the enemies did nothing" and "the enemies never loaded"
		// look identical from inside the game.
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKEnemyRosterSubsystem: /Game/Data/DT_Enemies not found. No family will notice "
				 "anything, deliberately - there is no fallback roster, because an enemy hunting on "
				 "invented radii is worse than an enemy that does nothing."));
		return;
	}

	Source = ELLKDataSource::Asset;
	UE_LOG(LogLLK, Log,
		TEXT("ULLKEnemyRosterSubsystem: DT_Enemies loaded, %d famil%s."),
		Table->GetRowMap().Num(), Table->GetRowMap().Num() == 1 ? TEXT("y") : TEXT("ies"));
}

const FLLKEnemyDef* ULLKEnemyRosterSubsystem::FindFamily(ELLKEnemyFamily Family) const
{
	if (Table == nullptr)
	{
		return nullptr;
	}

	// Matched on the row's own Family field, not on its row name - see the
	// header. The table is six rows, so the scan costs nothing and removes a
	// naming assumption that would fail silently.
	for (const TPair<FName, uint8*>& Row : Table->GetRowMap())
	{
		const FLLKEnemyDef* Def = reinterpret_cast<const FLLKEnemyDef*>(Row.Value);
		if (Def != nullptr && Def->Family == Family)
		{
			return Def;
		}
	}

	return nullptr;
}

TArray<const FLLKEnemyDef*> ULLKEnemyRosterSubsystem::GetAllFamilies() const
{
	TArray<const FLLKEnemyDef*> Families;
	if (Table == nullptr)
	{
		return Families;
	}

	for (const TPair<FName, uint8*>& Row : Table->GetRowMap())
	{
		if (const FLLKEnemyDef* Def = reinterpret_cast<const FLLKEnemyDef*>(Row.Value))
		{
			Families.Add(Def);
		}
	}
	return Families;
}

float ULLKEnemyRosterSubsystem::NoticeRangeStuds(
	ELLKEnemyFamily Family, float Brightness, float LitThreshold) const
{
	const FLLKEnemyDef* Def = FindFamily(Family);
	return Def != nullptr
		? LLKPerception::NoticeRangeStuds(*Def, Brightness, LitThreshold)
		: 0.f;
}
