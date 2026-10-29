#include "Harness/LLKStoreFloorBuilder.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "Loot/LLKRelicComponent.h"

// ---------------------------------------------------------------- the plan
float FLLKStoreFloorPlan::RampAngleDegrees() const
{
	if (RampRunStuds <= 0.f)
	{
		return 90.f;
	}
	return FMath::RadiansToDegrees(FMath::Atan2(MezzanineHeightStuds, RampRunStuds));
}

float FLLKStoreFloorPlan::LongestStraightStuds() const
{
	// **Derived from the break COUNT, because the first version was derived
	// from wishful thinking.** It halved the depth twice and reported 35
	// studs for a concourse that ran 60 clear. The test caught it on its
	// first run; the layout was wrong, not the test.
	//
	// N breaks across the depth gives N+1 legs. Whichever of the two runs -
	// shop floor or service corridor - has the longer leg is the longest
	// sightline the floor actually contains.
	const float ConcourseLeg = DepthStuds / FMath::Max(1, ConcourseBreaks + 1);
	const float CorridorLeg = DepthStuds / FMath::Max(1, CorridorJogs + 1);
	return FMath::Max(ConcourseLeg, CorridorLeg);
}

namespace
{
	/** Engine primitives only - see the harness header. Null-checked everywhere. */
	AStaticMeshActor* SpawnBox(UWorld* World, UStaticMesh* Mesh,
		const FVector& CentreCm, const FVector& SizeCm, int32& BlockCount,
		const FRotator& Rotation = FRotator::ZeroRotator)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(CentreCm, Rotation, Params);
		if (Actor == nullptr)
		{
			return nullptr;
		}

		Actor->SetMobility(EComponentMobility::Movable);
		if (UStaticMeshComponent* Component = Actor->GetStaticMeshComponent())
		{
			if (Mesh != nullptr)
			{
				Component->SetStaticMesh(Mesh);
			}
			// The engine cube is 100cm, so scale is size/100.
			Component->SetWorldScale3D(SizeCm / 100.f);
			// Collision regardless of whether the mesh loaded. An invisible
			// wall is a bad scaffold; a wall you walk through is no scaffold.
			Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Component->SetCollisionProfileName(TEXT("BlockAll"));
		}
		++BlockCount;
		return Actor;
	}

	/** One piece of loot on a surface, with an identity and a visibility tier. */
	void SpawnLoot(UWorld* World, UStaticMesh* Mesh, const FVector& AtCm,
		ELLKLootVisibility Visibility, FName Id, const FString& Name, int32 Value, int32& BlockCount)
	{
		AStaticMeshActor* Actor = SpawnBox(World, Mesh, AtCm, FVector(22.f, 22.f, 22.f), BlockCount);
		if (Actor == nullptr)
		{
			return;
		}

		// Loot is not furniture. A relic you can bump off a case is a
		// distraction from the thing being tested.
		if (UStaticMeshComponent* Component = Actor->GetStaticMeshComponent())
		{
			Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		Actor->SetReplicates(true);

		ULLKRelicComponent* Relic = NewObject<ULLKRelicComponent>(Actor);
		if (Relic == nullptr)
		{
			return;
		}
		Relic->Visibility = Visibility;
		Relic->RegisterComponent();
		Relic->SetRelicIdentity(Id, FText::FromString(Name), Value);
	}

	const TCHAR* RelicNames[] = {
		TEXT("Engraved pocket watch"),
		TEXT("Enamel brooch"),
		TEXT("Silver locket"),
		TEXT("Signet ring, initialled"),
		TEXT("Pearl drop earrings"),
		TEXT("Gold cigarette case"),
	};

	const TCHAR* BaselineNames[] = {
		TEXT("Bandage roll"),
		TEXT("Tin of pastilles"),
		TEXT("Hand mirror"),
		TEXT("Bottle of scent"),
	};
}

namespace LLKStoreFloor
{
	FBuildResult BuildGroundFloor(
		UWorld* World, UStaticMesh* Cube, const FVector& Origin, const FLLKStoreFloorPlan& Plan)
	{
		FBuildResult Result;
		if (World == nullptr)
		{
			return Result;
		}

		// Everything below is in centimetres, converted ONCE here from the
		// plan's studs. Row 61: one conversion authority, and a builder this
		// size is exactly where a second one would hide.
		const float Depth = LLKUnits::DistanceFromStuds(Plan.DepthStuds);
		const float Width = LLKUnits::DistanceFromStuds(Plan.WidthStuds);
		const float WallH = LLKUnits::DistanceFromStuds(Plan.WallHeightStuds);
		const float CounterH = LLKUnits::DistanceFromStuds(Plan.CounterHeightStuds);
		const float ShelfH = LLKUnits::DistanceFromStuds(Plan.ShelfHeightStuds);
		const float LedgeH = LLKUnits::DistanceFromStuds(Plan.ServiceLedgeHeightStuds);
		const float MezzH = LLKUnits::DistanceFromStuds(Plan.MezzanineHeightStuds);
		const float RampRun = LLKUnits::DistanceFromStuds(Plan.RampRunStuds);
		const float RampW = LLKUnits::DistanceFromStuds(Plan.RampWidthStuds);
		const float CorridorW = LLKUnits::DistanceFromStuds(Plan.ServiceCorridorWidthStuds);
		const float DoorW = LLKUnits::DistanceFromStuds(Plan.ServiceDoorWidthStuds);

		const float HalfDepth = Depth * 0.5f;
		const float HalfWidth = Width * 0.5f;
		const float Thickness = LLKUnits::DistanceFromStuds(1.f);

		// ---- 1. the slab ------------------------------------------------
		//
		// One continuous box with its top at the origin's Z. Chris asked for
		// "solid continuous floor and no fall-throughs", so it is a single
		// piece rather than tiles that can gap at a seam.
		SpawnBox(World, Cube,
			Origin + FVector(0.f, 0.f, -Thickness * 0.5f),
			FVector(Depth, Width, Thickness), Result.Blocks);

		// ---- 2. the shell ------------------------------------------------
		const float WallZ = Origin.Z + WallH * 0.5f;

		// Back and both sides, solid.
		SpawnBox(World, Cube, FVector(Origin.X + HalfDepth, Origin.Y, WallZ),
			FVector(Thickness, Width, WallH), Result.Blocks);
		SpawnBox(World, Cube, FVector(Origin.X, Origin.Y - HalfWidth, WallZ),
			FVector(Depth, Thickness, WallH), Result.Blocks);
		SpawnBox(World, Cube, FVector(Origin.X, Origin.Y + HalfWidth, WallZ),
			FVector(Depth, Thickness, WallH), Result.Blocks);

		// The street front, in three piers with glass between them. Sealed:
		// `THE-STORE.md` has the street entrances shut with the water still
		// against them, which is what makes the entrance read as where you
		// came from rather than as a way out.
		const float StreetX = Origin.X - HalfDepth;
		const float PierWidth = Width / 5.f;
		for (int32 Pier = -2; Pier <= 2; Pier += 2)
		{
			SpawnBox(World, Cube,
				FVector(StreetX, Origin.Y + Pier * PierWidth, WallZ),
				FVector(Thickness, PierWidth, WallH), Result.Blocks);
		}
		// The glass itself - solid, and low enough to see the water over.
		for (int32 Bay = -1; Bay <= 1; Bay += 2)
		{
			SpawnBox(World, Cube,
				FVector(StreetX, Origin.Y + Bay * PierWidth, WallZ),
				FVector(Thickness * 0.4f, PierWidth, WallH), Result.Blocks);
		}

		// Spawn just inside, facing in. 6 studs clear of the glass so the
		// first frame is the shop and not a wall.
		Result.SpawnPoint = FVector(
			StreetX + LLKUnits::DistanceFromStuds(6.f), Origin.Y,
			Origin.Z + LLKUnits::DistanceFromStuds(4.f));
		Result.ExtractionPoint = Result.SpawnPoint;

		// ---- 3. cosmetics: the counter grid ------------------------------
		//
		// Lesson one of the ground floor - there are things here and you can
		// carry them - taught with the lantern OFF, which is why the loot on
		// these is `Always` visible.
		const float GridStartX = StreetX + LLKUnits::DistanceFromStuds(16.f);
		const float CounterLen = LLKUnits::DistanceFromStuds(8.f);
		const float CounterDepth = LLKUnits::DistanceFromStuds(2.5f);
		const float GridPitchX = LLKUnits::DistanceFromStuds(12.f);
		const float GridPitchY = Width / (Plan.CosmeticsColumns + 1);

		int32 BaselineIndex = 0;
		for (int32 Row = 0; Row < Plan.CosmeticsRows; ++Row)
		{
			for (int32 Col = 0; Col < Plan.CosmeticsColumns; ++Col)
			{
				const float X = GridStartX + Row * GridPitchX;
				const float Y = Origin.Y - HalfWidth + GridPitchY * (Col + 1);

				SpawnBox(World, Cube, FVector(X, Y, Origin.Z + CounterH * 0.5f),
					FVector(CounterDepth, CounterLen, CounterH), Result.Blocks);

				// Baseline loot on top, every other counter. Something
				// readable in the dark is what lesson two needs to contrast
				// against; without it there is no "before".
				if (((Row + Col) % 2) == 0)
				{
					const int32 Which = BaselineIndex % UE_ARRAY_COUNT(BaselineNames);
					SpawnLoot(World, Cube,
						FVector(X, Y, Origin.Z + CounterH + LLKUnits::DistanceFromStuds(0.5f)),
						ELLKLootVisibility::Always,
						FName(*FString::Printf(TEXT("Baseline_%d_%d"), Row, Col)),
						BaselineNames[Which], 5, Result.Blocks);
					++Result.BaselineLoot;
					++BaselineIndex;
				}
			}
		}

		// ---- 4. the sightline breaks -------------------------------------
		//
		// **Not decoration, and the count is load-bearing.** Without these
		// the concourse runs the whole depth of the floor - 120 studs against
		// a 38-stud notice radius - so a lit keeper could be hunted by
		// something four times beyond their own vision.
		//
		// Every break is eye-height or taller, which is the part that is easy
		// to get wrong: a counter at 90cm does not break a sightline, it just
		// looks like it might.
		//
		// The first break is the mirrored columns, which also carry lesson
		// two's second channel - a lit keeper sees their own light arrive on a
		// mirrored face somewhere they are not standing. The rest are gondola
		// runs, which is what a shop floor interrupts itself with anyway.
		const float ColumnSize = LLKUnits::DistanceFromStuds(3.f);
		const float BreakPitch = Depth / (Plan.ConcourseBreaks + 1);
		for (int32 Break = 1; Break <= Plan.ConcourseBreaks; ++Break)
		{
			const float BreakX = Origin.X - HalfDepth + BreakPitch * Break;

			if (Break == (Plan.ConcourseBreaks + 1) / 2)
			{
				// The mirrored columns, mid-floor: a pair with a gap between
				// them, so the break is a thing you walk around rather than a
				// wall that ends the room.
				for (int32 Side = -1; Side <= 1; Side += 2)
				{
					SpawnBox(World, Cube,
						FVector(BreakX, Origin.Y + Side * LLKUnits::DistanceFromStuds(7.f),
							Origin.Z + WallH * 0.5f),
						FVector(ColumnSize, ColumnSize, WallH), Result.Blocks);
				}
				continue;
			}

			// A gondola run with a gap at one end, alternating sides, so the
			// route through the floor weaves instead of running straight.
			const float RunLength = Width * 0.62f;
			const float Offset = (Break % 2 == 0) ? 1.f : -1.f;
			SpawnBox(World, Cube,
				FVector(BreakX,
					Origin.Y + Offset * (HalfWidth - RunLength * 0.5f),
					Origin.Z + ShelfH * 0.5f),
				FVector(LLKUnits::DistanceFromStuds(2.f), RunLength, ShelfH), Result.Blocks);
		}

		// ---- 5. jewellery: the locked cases ------------------------------
		//
		// Relic-dense and deeper in, so reaching them is a decision. Row 53:
		// visible but unreadable until lit.
		const float CaseStartX = Origin.X + LLKUnits::DistanceFromStuds(10.f);
		const float CasePitch = LLKUnits::DistanceFromStuds(9.f);
		for (int32 Case = 0; Case < Plan.JewelleryCases; ++Case)
		{
			const float X = CaseStartX + (Case % 3) * CasePitch;
			const float Y = Origin.Y - HalfWidth + LLKUnits::DistanceFromStuds(10.f)
				+ (Case / 3) * CasePitch;

			SpawnBox(World, Cube, FVector(X, Y, Origin.Z + CounterH * 0.5f),
				FVector(CounterDepth, CounterLen * 0.7f, CounterH), Result.Blocks);

			const int32 Which = Case % UE_ARRAY_COUNT(RelicNames);
			SpawnLoot(World, Cube,
				FVector(X, Y, Origin.Z + CounterH + LLKUnits::DistanceFromStuds(0.5f)),
				ELLKLootVisibility::LanternOnly,
				FName(*FString::Printf(TEXT("Relic_%d"), Case)),
				RelicNames[Which], 300 + Case * 60, Result.Blocks);
			++Result.Relics;
		}

		// ---- 6. the escalator well: the landmark and the incline ---------
		const float RampStartX = Origin.X + LLKUnits::DistanceFromStuds(18.f);
		const float RampCentreX = RampStartX + RampRun * 0.5f;
		const float RampCentreZ = Origin.Z + MezzH * 0.5f;
		const float RampLength = FMath::Sqrt(RampRun * RampRun + MezzH * MezzH);

		// Rotated about Y so it climbs along +X. Negative pitch raises the
		// far end in Unreal's left-handed frame.
		SpawnBox(World, Cube,
			FVector(RampCentreX, Origin.Y + LLKUnits::DistanceFromStuds(24.f), RampCentreZ),
			FVector(RampLength, RampW, Thickness),
			Result.Blocks,
			FRotator(-Plan.RampAngleDegrees(), 0.f, 0.f));

		// The mezzanine stub it climbs to. Somewhere you can see the floor
		// from, and be seen from - a light-trade spot rather than a safe one.
		const float MezzX = RampStartX + RampRun + LLKUnits::DistanceFromStuds(6.f);
		SpawnBox(World, Cube,
			FVector(MezzX, Origin.Y + LLKUnits::DistanceFromStuds(24.f), Origin.Z + MezzH),
			FVector(LLKUnits::DistanceFromStuds(12.f), RampW, Thickness), Result.Blocks);

		// ---- 7. the service route ----------------------------------------
		//
		// Behind an internal wall with two doors, jogging at the midpoint so
		// no straight run exceeds the sightline rule. Narrow enough that a
		// lantern fills it, which is what makes it an escape and not a
		// shortcut.
		const float CorridorY = Origin.Y + HalfWidth - CorridorW * 0.5f - Thickness;
		const float WallY = CorridorY - CorridorW * 0.5f;

		// Two wall runs with a door gap between them, twice.
		const float SegmentLength = (Depth - DoorW * 2.f) / 3.f;
		for (int32 Segment = 0; Segment < 3; ++Segment)
		{
			const float SegX = Origin.X - HalfDepth
				+ SegmentLength * (Segment + 0.5f) + DoorW * Segment;
			SpawnBox(World, Cube, FVector(SegX, WallY, WallZ),
				FVector(SegmentLength, Thickness, WallH), Result.Blocks);
		}

		// The jogs. A corridor the full depth of the building is the worst
		// sightline on the floor and the easiest to miss, because a corridor
		// is supposed to be long. Each stub reaches most of the way across so
		// somebody looking down it sees a corner, not the way out.
		const float JogPitch = Depth / (Plan.CorridorJogs + 1);
		for (int32 Jog = 1; Jog <= Plan.CorridorJogs; ++Jog)
		{
			const float JogX = Origin.X - HalfDepth + JogPitch * Jog;
			const float Side = (Jog % 2 == 0) ? 1.f : -1.f;
			SpawnBox(World, Cube,
				FVector(JogX, CorridorY + Side * CorridorW * 0.25f, WallZ),
				FVector(Thickness, CorridorW * 0.6f, WallH), Result.Blocks);
		}

		// The service ledge - under the jump apex, unlike the counters. The
		// contrast is the teaching, and it is row 63's tuning reference.
		SpawnBox(World, Cube,
			FVector(Origin.X - LLKUnits::DistanceFromStuds(20.f), CorridorY, Origin.Z + LedgeH * 0.5f),
			FVector(LLKUnits::DistanceFromStuds(8.f), CorridorW * 0.8f, LedgeH), Result.Blocks);

		// Shelving in the back of house, because a stockroom is shelves.
		for (int32 Shelf = 0; Shelf < 3; ++Shelf)
		{
			SpawnBox(World, Cube,
				FVector(Origin.X + LLKUnits::DistanceFromStuds(10.f + Shelf * 9.f),
					CorridorY, Origin.Z + ShelfH * 0.5f),
				FVector(LLKUnits::DistanceFromStuds(2.f), CorridorW * 0.7f, ShelfH), Result.Blocks);
		}

		// ---- 8. the authored points M2.6 and M2.2 need -------------------
		//
		// Authored rather than generated, for row 63's reason about climb
		// points: a machine that can appear anywhere is a machine the player
		// cannot learn. These are places a keeper arrives at, not starts in.
		Result.EnemySpawnPoints.Add(FVector(Origin.X + HalfDepth
			- LLKUnits::DistanceFromStuds(8.f), CorridorY, Origin.Z + LLKUnits::DistanceFromStuds(4.f)));
		Result.EnemySpawnPoints.Add(FVector(CaseStartX + CasePitch * 2.f,
			Origin.Y - HalfWidth + LLKUnits::DistanceFromStuds(8.f),
			Origin.Z + LLKUnits::DistanceFromStuds(4.f)));
		Result.EnemySpawnPoints.Add(FVector(MezzX,
			Origin.Y + LLKUnits::DistanceFromStuds(24.f),
			Origin.Z + MezzH + LLKUnits::DistanceFromStuds(4.f)));

		// Somewhere a downed keeper can plausibly fall and still be reached -
		// the revive needs 12 studs of clearance around a body, and a
		// casualty wedged between two counters is a rescue nobody can finish.
		Result.RescueTestPoints.Add(FVector(Origin.X, Origin.Y,
			Origin.Z + LLKUnits::DistanceFromStuds(2.f)));
		Result.RescueTestPoints.Add(FVector(Origin.X - LLKUnits::DistanceFromStuds(20.f),
			CorridorY, Origin.Z + LLKUnits::DistanceFromStuds(2.f)));

		UE_LOG(LogLLK, Log,
			TEXT("LLKStoreFloor: ground floor built - %.0f x %.0f studs, %d blocks, %d relics, ")
			TEXT("%d baseline, ramp %.1f deg, longest straight %.0f studs (limit %.0f). ")
			TEXT("Disposable greybox; the authored store replaces all of it."),
			Plan.DepthStuds, Plan.WidthStuds, Result.Blocks, Result.Relics, Result.BaselineLoot,
			Plan.RampAngleDegrees(), Plan.LongestStraightStuds(), Plan.MaxSightlineStuds);

		if (!Plan.ObeysSightlineRule())
		{
			UE_LOG(LogLLK, Warning,
				TEXT("LLKStoreFloor: longest straight %.0f studs exceeds the %.0f-stud limit. ")
				TEXT("A lit keeper is noticed from 38 studs, so a longer sightline means being ")
				TEXT("hunted by something there was no way to see."),
				Plan.LongestStraightStuds(), Plan.MaxSightlineStuds);
		}

		return Result;
	}
}
