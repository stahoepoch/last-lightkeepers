#include "Enemies/LLKSweeper.h"

#include "Characters/LLKVitalsComponent.h"
#include "Data/LLKEnemyDef.h"
#include "Data/LLKLanternData.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Enemies/LLKEnemyRosterSubsystem.h"
#include "Enemies/LLKPerception.h"
#include "Enemies/LLKVisibility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LLK.h"
#include "LLKUnits.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Run/LLKRunStateSubsystem.h"
#include "Storm/LLKLightField.h"
#include "Storm/LLKZonePressureSubsystem.h"

const FName ALLKSweeper::PatrolPointTag(TEXT("LLK.Patrol"));

ALLKSweeper::ALLKSweeper()
{
	PrimaryActorTick.bCanEverTick = true;

	// **No capsule size, no gravity, no acceleration, no collision channels.**
	// Chris: *"Do not invent patrol points, route shapes, collision tuning, or
	// spawn spacing."* A Sweeper's real dimensions are a question about
	// doorways and shelf gaps, so the defaults stand until a map asks.
	//
	// The one movement value set here is the authored speed, and it is set in
	// BeginPlay rather than the constructor because it comes from a data asset.
	bReplicates = true;
	SetReplicateMovement(true);
}

bool ALLKSweeper::HasEnemyAuthority() const
{
	return HasAuthority();
}

void ALLKSweeper::BeginPlay()
{
	Super::BeginPlay();

	// `enemies.json`'s Sweeper row, through the roster that already reads it.
	// Nothing about the family is duplicated here - the aggro radii, the
	// telegraph and the speed are all Chris's values (row 55), and this class
	// only converts the one it needs into centimetres.
	if (!bHasDef)
	{
		if (const UWorld* World = GetWorld())
		{
			if (const ULLKEnemyRosterSubsystem* Roster = World->GetSubsystem<ULLKEnemyRosterSubsystem>())
			{
				if (const FLLKEnemyDef* Found = Roster->FindFamily(ELLKEnemyFamily::Sweepers))
				{
					Def = *Found;
					bHasDef = true;
				}
			}
		}
	}

	if (!bHasDef)
	{
		// Loud, because a Sweeper with no def has no telegraph and no aggro
		// radii: it would stand still forever and look like a broken pawn
		// rather than like missing data.
		UE_LOG(LogLLK, Warning,
			TEXT("%s: no Sweepers row in the enemy roster. It will not notice anybody. ")
			TEXT("Run Tools/import_data.py."),
			*GetName());
		return;
	}

	if (const ULLKLanternData* Data = LanternData.LoadSynchronous())
	{
		ContactRules = Data->SweeperContact;
	}
	else
	{
		UE_LOG(LogLLK, Warning,
			TEXT("%s: no ULLKLanternData, so sweeperContact is unauthored - it will chase and ")
			TEXT("never knock anybody down. Run Tools/import_data.py."),
			*GetName());
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = LLKUnits::SpeedFromStuds(Def.MoveSpeed);
	}

	UE_LOG(LogLLK, Log,
		TEXT("%s awake. Speed %.0f cm/s, telegraph %.1fs, notices a dark keeper at %.0f studs and a lit one at %.0f."),
		*GetName(), LLKUnits::SpeedFromStuds(Def.MoveSpeed), Def.AudioTelegraphSeconds,
		Def.AggroRadiusDark.Get(0.f), Def.AggroRadiusLit.Get(0.f));
}

ULLKVitalsComponent* ALLKSweeper::FindNearestKeeper(float& OutDistanceStuds) const
{
	OutDistanceStuds = 0.f;

	const UWorld* World = GetWorld();
	const ULLKRunStateSubsystem* Run = World != nullptr
		? World->GetSubsystem<ULLKRunStateSubsystem>() : nullptr;
	if (Run == nullptr)
	{
		return nullptr;
	}

	// **The run's own roll, not an actor iterator.** The keepers in the run are
	// already registered somewhere, and a second definition of "who is in this
	// raid" is the kind of duplicate that disagrees with the first the moment
	// somebody disconnects.
	ULLKVitalsComponent* Nearest = nullptr;
	float NearestSquared = TNumericLimits<float>::Max();

	for (ULLKVitalsComponent* Keeper : Run->GetKeepers())
	{
		const AActor* KeeperOwner = Keeper != nullptr ? Keeper->GetOwner() : nullptr;
		if (KeeperOwner == nullptr)
		{
			continue;
		}

		const float DistanceSquared =
			FVector::DistSquared(KeeperOwner->GetActorLocation(), GetActorLocation());
		if (DistanceSquared < NearestSquared)
		{
			NearestSquared = DistanceSquared;
			Nearest = Keeper;
		}
	}

	if (Nearest != nullptr)
	{
		OutDistanceStuds = FMath::Sqrt(NearestSquared) / LLKUnits::StudsToCm;
	}

	return Nearest;
}

FLLKSweeperSenses ALLKSweeper::Observe(const float DeltaSeconds)
{
	FLLKSweeperSenses Senses;
	Senses.DeltaSeconds = DeltaSeconds;
	Senses.bHasPatrolPoint = NextPatrolPoint().IsSet();

	const UWorld* World = GetWorld();
	if (World == nullptr || !bHasDef)
	{
		return Senses;
	}

	float DistanceStuds = 0.f;
	ULLKVitalsComponent* Keeper = FindNearestKeeper(DistanceStuds);
	const AActor* KeeperActor = Keeper != nullptr ? Keeper->GetOwner() : nullptr;
	if (KeeperActor == nullptr)
	{
		Target = nullptr;
		return Senses;
	}

	// A downed keeper is not a target. Row 9 gives them their own fate and a
	// Sweeper standing over a body it cannot knock down any further would be
	// a chase with no end condition.
	if (Keeper->IsDowned())
	{
		Target = nullptr;
		return Senses;
	}

	Target = Keeper;
	Senses.KeeperPosition = KeeperActor->GetActorLocation();
	Senses.DistanceToKeeperStuds = DistanceStuds;

	// **Brightness at the keeper, not "is their lantern on".** The light field
	// is the one definition of how lit a spot is, and a keeper standing in a
	// teammate's cone with their own lantern off is lit - which is what makes
	// light a shared cost.
	float Brightness = 0.f;
	float LanternIntensity = 0.f;
	if (const ULLKZonePressureSubsystem* Pressure = World->GetSubsystem<ULLKZonePressureSubsystem>())
	{
		const TArray<FLLKLightSource> Sources = Pressure->GetLitLightSources();
		Brightness = LLKLightField::BrightnessAt(Sources, Senses.KeeperPosition);

		// The threshold is derived from a lantern's own authored intensity, so
		// the brightest source present stands in for it. One definition of
		// "lit enough to notice", shared with the relics (row 53).
		for (const FLLKLightSource& Source : Sources)
		{
			LanternIntensity = FMath::Max(LanternIntensity, Source.Intensity);
		}
	}

	FLLKPerceiver Perceiver;
	Perceiver.Position = GetActorLocation();
	Perceiver.Facing = GetActorForwardVector();

	FLLKPerceivedKeeper Perceived;
	Perceived.Position = Senses.KeeperPosition;
	Perceived.Brightness = Brightness;

	// The trace, through the seam M2.5 built for exactly this: perception asks
	// a visibility source and never learns that a `UWorld` exists. `Unknown`
	// from a failed trace never notices, which is why a Sweeper cannot hunt
	// through geometry nobody could measure.
	const FLLKWorldVisibility Visibility(World, this);

	Senses.Notice = LLKPerception::Notice(
		Def, Perceiver, Perceived, LLKPerception::LitThresholdFrom(LanternIntensity), &Visibility);

	return Senses;
}

TOptional<FVector> ALLKSweeper::NextPatrolPoint() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return TOptional<FVector>();
	}

	// **Whatever the map tagged, in the order the map made them.** No route
	// shape, no spacing, no generation: if Chris puts three empty actors in a
	// line and tags them, that is the route. The index walks the list and
	// wraps, which is the least opinionated thing a patrol can be - and if a
	// floor ever wants a real route shape, that belongs in the map rather than
	// here.
	TArray<AActor*> Points;
	for (TActorIterator<AActor> It(const_cast<UWorld*>(World)); It; ++It)
	{
		if (It->ActorHasTag(PatrolPointTag))
		{
			Points.Add(*It);
		}
	}

	if (Points.IsEmpty())
	{
		return TOptional<FVector>();
	}

	const int32 Index = PatrolIndex == INDEX_NONE ? 0 : PatrolIndex % Points.Num();
	return Points[Index]->GetActorLocation();
}

void ALLKSweeper::StepToward(const FVector& Destination, const float DeltaSeconds)
{
	UWorld* World = GetWorld();
	UNavigationSystemV1* Nav = World != nullptr ? UNavigationSystemV1::GetCurrent(World) : nullptr;
	if (Nav == nullptr)
	{
		// No navigation at all. Reported by `LLK.CheckMap` as
		// `NavBoundsMissing` rather than warned about again here; a Sweeper
		// that walked in a straight line through shelving instead would be a
		// worse answer than one that stands still.
		return;
	}

	// Queried every time rather than cached, because the brain's destination
	// can be the keeper, who moves. A path this short is cheap, and caching it
	// would be the beginning of the path-following component this file
	// deliberately does not have.
	const UNavigationPath* Path = Nav->FindPathToLocationSynchronously(
		World, GetActorLocation(), Destination, this);
	if (Path == nullptr || !Path->IsValid() || Path->PathPoints.Num() < 2)
	{
		return;
	}

	// The second point: the first is where we are standing.
	const FVector Next = Path->PathPoints[1];
	FVector Direction = Next - GetActorLocation();
	Direction.Z = 0.f;

	if (Direction.SizeSquared() <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	Direction.Normalize();
	AddMovementInput(Direction, 1.f);

	// Faces where it is going. A Sweeper's facing matters to nobody's
	// perception - `LitProximity` is omnidirectional - but a machine walking
	// sideways reads as broken, and row 16's whole point is that an enemy is
	// legible before it reaches you.
	SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
}

void ALLKSweeper::Act(const FLLKSweeperStep& Step)
{
	if (Step.bTelegraphStarted)
	{
		// Row 16. The sound itself is Chris's asset; this is the cue and the
		// log line, so a playtest can see the announcement happen even before
		// anything is audible.
		UE_LOG(LogLLK, Log, TEXT("%s TELEGRAPHS. %.1fs before it moves."),
			*GetName(), Def.AudioTelegraphSeconds);
		OnTelegraph.Broadcast(GetActorLocation());
	}

	if (Step.bLostTheKeeper)
	{
		UE_LOG(LogLLK, Log, TEXT("%s lost the keeper and went back to its route."), *GetName());
	}

	if (Step.bKnockedDown && Target != nullptr)
	{
		// The authored three hits landed inside the decay window. The vitals
		// component owns what being down means - row 64 keeps the knockdown
		// path away from the clock, and this call is the whole of a Sweeper's
		// involvement in it.
		UE_LOG(LogLLK, Log, TEXT("%s put a keeper down."), *GetName());
		Target->GoDown(ELLKDownedReason::Contained);
	}
}

void ALLKSweeper::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Server only. A client's Sweeper walks because movement replicates, not
	// because it decided to - a client that could conclude it had caught you
	// could disagree with the server about whether you are down.
	if (!HasEnemyAuthority() || !bHasDef)
	{
		return;
	}

	const FLLKSweeperSenses Senses = Observe(DeltaSeconds);
	const FLLKSweeperStep Step = LLKSweeperBrain::Advance(Mind, Senses, Def, ContactRules);

	Mind = Step.Mind;
	Act(Step);

	switch (Step.Move)
	{
	case ELLKSweeperMove::Hold:
		bHasDestination = false;
		break;

	case ELLKSweeperMove::Keeper:
		CurrentDestination = Senses.KeeperPosition;
		bHasDestination = true;
		StepToward(CurrentDestination, DeltaSeconds);
		break;

	case ELLKSweeperMove::LastKnown:
		CurrentDestination = Mind.LastKnownKeeperPosition;
		bHasDestination = true;
		StepToward(CurrentDestination, DeltaSeconds);
		break;

	case ELLKSweeperMove::PatrolPoint:
	{
		const TOptional<FVector> Point = NextPatrolPoint();
		if (!Point.IsSet())
		{
			if (!bWarnedAboutNoPatrol)
			{
				bWarnedAboutNoPatrol = true;
				UE_LOG(LogLLK, Warning,
					TEXT("%s has nowhere to patrol: no actor in this map is tagged %s. ")
					TEXT("It will notice a keeper and chase, but it will not walk a route. ")
					TEXT("The route is map work - see docs/MAP-READINESS.md."),
					*GetName(), *PatrolPointTag.ToString());
			}
			break;
		}

		CurrentDestination = Point.GetValue();
		bHasDestination = true;

		// Arrived: take the next point. The only thing this class decides
		// about a route is "that one is done", which is why the tolerance is
		// here and the shape is not.
		const float ToleranceCm = LLKUnits::DistanceFromStuds(ArriveToleranceStuds);
		if (FVector::DistSquared2D(GetActorLocation(), CurrentDestination) <= ToleranceCm * ToleranceCm)
		{
			PatrolIndex = PatrolIndex == INDEX_NONE ? 1 : PatrolIndex + 1;
			break;
		}

		StepToward(CurrentDestination, DeltaSeconds);
		break;
	}
	}
}
