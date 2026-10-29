// The roster loads, is queryable, and refuses to invent anything. M2.4.
//
// **The property under test is mostly the absence of a fallback**, which is
// unusual enough in this project to need asserting rather than assuming.
// Every other data reader here declares defaults so the game stays playable
// without `Content/Data`. The roster deliberately does not, because an enemy
// hunting on radii nobody authored teaches the player a lantern rule the data
// does not contain - and that is the one thing this game cannot afford to get
// wrong, since the lantern rule IS the game.
//
// So a missing table must produce nullptr and an `Unresolved` source, not a
// plausible-looking Sweeper. The lantern taught the opposite failure in
// 2026-10: `DA_Lantern` resolved to zero values and produced a light that was
// present, legal and invisible. The lesson both times is the same one -
// **never let a degenerate resolution pass for a real one** - and here the
// honest degenerate answer is nothing at all.

#include "Data/LLKEnemyDef.h"
#include "Enemies/LLKEnemyRosterSubsystem.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKEnemyRosterTest,
	"LLK.Enemies.Roster",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKEnemyRosterTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK enemy roster test reached RunTest."));

	ULLKEnemyRosterSubsystem* Roster = NewObject<ULLKEnemyRosterSubsystem>(GetTransientPackage());

	// ---- 1. no table means nothing, loudly ------------------------------
	//
	// A fresh subsystem has not resolved, which is exactly a world whose
	// DT_Enemies is missing.
	TestEqual(TEXT("an unresolved roster says so"),
		Roster->GetSource(), ELLKDataSource::Unresolved);
	TestNull(TEXT("and finds no family"),
		Roster->FindFamily(ELLKEnemyFamily::Sweepers));
	TestEqual(TEXT("and lists none"), Roster->GetAllFamilies().Num(), 0);

	// The number that matters: a missing roster must not hand a caller a
	// radius. `LLKStoreFloorPlan` will size sightlines against this.
	TestEqual(TEXT("and offers no notice range to size a sightline against"),
		Roster->NoticeRangeStuds(ELLKEnemyFamily::Sweepers, 999.f, 1.f), 0.f);

	// ---- 2. a table is queried by FAMILY, not by row name ---------------
	//
	// Built here rather than loaded, so this runs whether or not
	// `Content/Data` has been generated - and so the row name can be made to
	// disagree with the family on purpose, which is the case the header
	// claims to handle.
	UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
	Table->RowStruct = FLLKEnemyDef::StaticStruct();

	FLLKEnemyDef Sweeper;
	Sweeper.Family = ELLKEnemyFamily::Sweepers;
	Sweeper.AggroRadiusDark = 12.f;
	Sweeper.AggroRadiusLit = 38.f;
	Sweeper.AudioTelegraphSeconds = 1.f;
	// Named NOTHING like its family, deliberately.
	Table->AddRow(TEXT("row_the_first"), Sweeper);

	FLLKEnemyDef Warden;
	Warden.Family = ELLKEnemyFamily::Warden;
	Warden.bKillable = false;
	Warden.AudioTelegraphSeconds = 3.f;
	Table->AddRow(TEXT("row_the_second"), Warden);

	Roster->SetTableForTest(Table);

	TestEqual(TEXT("a resolved roster says it read an asset"),
		Roster->GetSource(), ELLKDataSource::Asset);
	TestEqual(TEXT("and lists every row it contained"), Roster->GetAllFamilies().Num(), 2);

	const FLLKEnemyDef* FoundSweeper = Roster->FindFamily(ELLKEnemyFamily::Sweepers);
	if (!TestNotNull(TEXT("the Sweeper row is found despite its row name"), FoundSweeper))
	{
		return false;
	}
	TestEqual(TEXT("and it is the row with that family, not merely some row"),
		FoundSweeper->Family, ELLKEnemyFamily::Sweepers);
	TestTrue(TEXT("carrying the radii that were authored on it"),
		FoundSweeper->AggroRadiusLit.IsSet() && FoundSweeper->AggroRadiusLit.GetValue() == 38.f);

	// A family the table does not contain is still nullptr, not the first row
	// it happened to iterate - which would make one enemy behave as another.
	TestNull(TEXT("a family absent from the table is nullptr, never a near-miss row"),
		Roster->FindFamily(ELLKEnemyFamily::Glimmers));
	TestEqual(TEXT("and its notice range is zero rather than a neighbour's"),
		Roster->NoticeRangeStuds(ELLKEnemyFamily::Glimmers, 999.f, 1.f), 0.f);

	// ---- 3. the roster's range agrees with the rule ---------------------
	//
	// Row 7 through the roster rather than through the rule, since this is
	// the path a spawner will take.
	TestEqual(TEXT("a lit keeper is noticed from the Sweeper's lit radius"),
		Roster->NoticeRangeStuds(ELLKEnemyFamily::Sweepers, 200.f, 125.f), 38.f);
	TestEqual(TEXT("a dark one from the dark radius"),
		Roster->NoticeRangeStuds(ELLKEnemyFamily::Sweepers, 10.f, 125.f), 12.f);
	TestEqual(TEXT("and the Warden from neither"),
		Roster->NoticeRangeStuds(ELLKEnemyFamily::Warden, 200.f, 125.f), 0.f);

	// ---- 4. losing the table loses the roster, not half of it -----------
	//
	// A world torn down mid-run must not leave a subsystem answering from a
	// dangling table.
	Roster->SetTableForTest(nullptr);
	TestEqual(TEXT("clearing the table returns the roster to Unresolved"),
		Roster->GetSource(), ELLKDataSource::Unresolved);
	TestNull(TEXT("and it finds nothing again"),
		Roster->FindFamily(ELLKEnemyFamily::Sweepers));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
