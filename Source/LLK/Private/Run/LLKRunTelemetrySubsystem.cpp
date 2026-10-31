#include "Run/LLKRunTelemetrySubsystem.h"

#include "Characters/LLKVitalsComponent.h"
#include "Engine/World.h"
#include "HAL/PlatformFileManager.h"
#include "LLK.h"
#include "LLKGameState.h"
#include "Lantern/LLKLanternComponent.h"
#include "EngineUtils.h"
#include "Loot/LLKInteractorComponent.h"
#include "Loot/LLKRelicComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Run/LLKExtractionSubsystem.h"
#include "Run/LLKRunStateSubsystem.h"
#include "Storm/LLKZonePressureSubsystem.h"

bool ULLKRunTelemetrySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	const UWorld* World = Cast<UWorld>(Outer);
	return World != nullptr && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void ULLKRunTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Recorded even on a client, where it will be the only event: the run's
	// timeline starts here either way, and an authority check that silenced
	// this would make a client's empty file indistinguishable from a crash.
	Record(ELLKRunEvent::RunStarted);
}

void ULLKRunTelemetrySubsystem::Deinitialize()
{
	// A run that ends because the editor stopped is still a run, and the
	// numbers are the whole point of having been there. Written here rather
	// than only on an outcome, because "they quit at 3:40 having never called
	// extraction" is one of the more useful things a playtest can produce.
	if (!bWritten && Events.Num() > 1 && HasTelemetryAuthority())
	{
		WriteToDisk();
	}

	Super::Deinitialize();
}

TStatId ULLKRunTelemetrySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULLKRunTelemetrySubsystem, STATGROUP_Tickables);
}

bool ULLKRunTelemetrySubsystem::HasTelemetryAuthority() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->GetNetMode() != NM_Client;
}

void ULLKRunTelemetrySubsystem::Record(const ELLKRunEvent Kind, const FName Detail, const float Value)
{
	FLLKRunEvent Event;
	Event.Kind = Kind;

	// **The subsystem stamps the time, not the caller.** Every handler here is
	// a delegate somebody else broadcast, and a timestamp passed in would be a
	// timestamp somebody could get wrong - the run's timeline is this class's
	// one piece of authority.
	Event.AtSeconds = RunSeconds;
	Event.Detail = Detail;
	Event.Value = Value;
	Events.Add(Event);
}

void ULLKRunTelemetrySubsystem::BindToWhatExists()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// The subsystems exist for the whole run, so once is enough.
	if (!bBoundToSubsystems)
	{
		bool bBoundAll = true;

		if (ULLKZonePressureSubsystem* Pressure = World->GetSubsystem<ULLKZonePressureSubsystem>())
		{
			Pressure->OnThresholdCrossed.AddDynamic(this, &ULLKRunTelemetrySubsystem::OnThresholdCrossed);
			Pressure->OnWardenConverged.AddDynamic(this, &ULLKRunTelemetrySubsystem::OnWardenConverged);
		}
		else
		{
			bBoundAll = false;
		}

		if (ULLKRunStateSubsystem* Run = World->GetSubsystem<ULLKRunStateSubsystem>())
		{
			Run->OnCrewWiped.AddDynamic(this, &ULLKRunTelemetrySubsystem::OnCrewWiped);
		}
		else
		{
			bBoundAll = false;
		}

		if (ULLKExtractionSubsystem* Extraction = World->GetSubsystem<ULLKExtractionSubsystem>())
		{
			Extraction->OnExtractionCalled.AddDynamic(this, &ULLKRunTelemetrySubsystem::OnExtractionCalled);
			Extraction->OnRigArrived.AddDynamic(this, &ULLKRunTelemetrySubsystem::OnRigArrived);
			Extraction->OnExtractionCancelled.AddDynamic(this, &ULLKRunTelemetrySubsystem::OnExtractionCancelled);
			Extraction->OnExtractionSecured.AddDynamic(this, &ULLKRunTelemetrySubsystem::OnExtractionSecured);
		}
		else
		{
			bBoundAll = false;
		}

		bBoundToSubsystems = bBoundAll;
	}

	// **Keepers arrive late and can arrive at any time**, which is why this
	// runs on an interval rather than once at Initialize: a pawn possessed two
	// seconds into a run would otherwise contribute nothing, and a four-player
	// playtest would quietly measure one lantern.
	const ULLKRunStateSubsystem* Run = World->GetSubsystem<ULLKRunStateSubsystem>();
	if (Run == nullptr)
	{
		return;
	}

	for (ULLKVitalsComponent* Keeper : Run->GetKeepers())
	{
		if (Keeper == nullptr || BoundKeepers.Contains(Keeper))
		{
			continue;
		}

		BoundKeepers.Add(Keeper);
		Keeper->OnDownedChanged.AddDynamic(this, &ULLKRunTelemetrySubsystem::OnDownedChanged);

		const AActor* Owner = Keeper->GetOwner();
		if (Owner == nullptr)
		{
			continue;
		}

		if (ULLKLanternComponent* Lantern = Owner->FindComponentByClass<ULLKLanternComponent>())
		{
			if (!BoundLanterns.Contains(Lantern))
			{
				BoundLanterns.Add(Lantern);
				Lantern->OnLanternStateChanged.AddDynamic(
					this, &ULLKRunTelemetrySubsystem::OnLanternStateChanged);
			}
		}

		if (ULLKInteractorComponent* Interactor = Owner->FindComponentByClass<ULLKInteractorComponent>())
		{
			if (!BoundInteractors.Contains(Interactor))
			{
				BoundInteractors.Add(Interactor);
				LastCarried.Add(Interactor, Interactor->GetCarriedCount());
				Interactor->OnCarriedCountChanged.AddDynamic(
					this, &ULLKRunTelemetrySubsystem::OnCarriedCountChanged);
				Interactor->OnInteractAttempted.AddDynamic(
					this, &ULLKRunTelemetrySubsystem::OnInteractAttempted);
			}
		}
	}

	// **The relics, found on this same sweep rather than at Initialize.** The
	// harness spawns the store floor during BeginPlay and the authored map's
	// loot will stream in with World Partition, so a one-shot search at
	// startup would find an empty room. Discovered here and polled in Tick;
	// the iteration is the discovery, never the measurement.
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		ULLKRelicComponent* Relic = It->FindComponentByClass<ULLKRelicComponent>();
		if (Relic == nullptr || TakeableSince.Contains(Relic))
		{
			continue;
		}

		// Negative means "not takeable as far as this instrument knows", which
		// is the right starting assumption: a relic already lit when the
		// instrument found it has a dwell nobody observed the beginning of,
		// and inventing one would be a measurement of when this code ran.
		TakeableSince.Add(Relic, -1.f);
	}
}

void ULLKRunTelemetrySubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunSeconds += DeltaTime;

	SecondsSinceBind += DeltaTime;
	if (SecondsSinceBind >= BindIntervalSeconds)
	{
		SecondsSinceBind = 0.f;
		BindToWhatExists();
	}

	if (!HasTelemetryAuthority())
	{
		return;
	}

	// Before the game state is fetched, because this needs no game state and a
	// run in a world without one must still answer row 43's question.
	PollAbandonedRelics(DeltaTime);

	// **The other thing polled rather than listened to.** A stall is a condition
	// - the hold is running and not advancing - and nothing broadcasts it,
	// because the extraction subsystem publishes it in the snapshot for a
	// readout to draw. Polling the edges turns row 75's pause into two
	// numbers: how often, and for how long.
	const UWorld* World = GetWorld();
	const ALLKGameState* GameState = World != nullptr ? World->GetGameState<ALLKGameState>() : nullptr;
	if (GameState == nullptr)
	{
		return;
	}

	const FLLKExtractionSnapshot& Snapshot = GameState->GetExtractionSnapshot();
	const bool bStalled = Snapshot.bPublished
		&& Snapshot.Phase == ELLKExtractionPhase::Holding
		&& Snapshot.bHoldStalled;

	if (bStalled != bWasStalled)
	{
		bWasStalled = bStalled;
		Record(bStalled ? ELLKRunEvent::HoldStalled : ELLKRunEvent::HoldResumed, Snapshot.PointId);
	}
}

void ULLKRunTelemetrySubsystem::OnThresholdCrossed(const FName ThresholdId, const float Pressure)
{
	Record(ELLKRunEvent::ThresholdCrossed, ThresholdId, Pressure);
}

void ULLKRunTelemetrySubsystem::OnWardenConverged()
{
	Record(ELLKRunEvent::WardenConverged);
}

void ULLKRunTelemetrySubsystem::OnCrewWiped()
{
	if (bResolved)
	{
		// A run has one ending. A wipe after a secure would make row 8's
		// arithmetic - a wipe keeps nothing - erase a haul that already left.
		return;
	}

	bResolved = true;
	Record(ELLKRunEvent::CrewWiped);
	WriteToDisk();
}

void ULLKRunTelemetrySubsystem::OnExtractionCalled(const FName PointId, const float SirenSeconds)
{
	Record(ELLKRunEvent::ExtractionCalled, PointId, SirenSeconds);
}

void ULLKRunTelemetrySubsystem::OnRigArrived(const FName PointId)
{
	Record(ELLKRunEvent::RigArrived, PointId);
}

void ULLKRunTelemetrySubsystem::OnExtractionCancelled(const FName PointId)
{
	Record(ELLKRunEvent::ExtractionCancelled, PointId);
}

void ULLKRunTelemetrySubsystem::OnExtractionSecured(
	const FName PointId, const float PayoutModifier, const int32 ItemsSecured)
{
	if (bResolved)
	{
		return;
	}

	bResolved = true;
	Record(ELLKRunEvent::ExtractionSecured, PointId, static_cast<float>(ItemsSecured));
	WriteToDisk();
}

void ULLKRunTelemetrySubsystem::OnDownedChanged(const bool bIsDowned, ELLKDownedReason Reason)
{
	Record(bIsDowned ? ELLKRunEvent::KeeperDowned : ELLKRunEvent::KeeperUp);
}

void ULLKRunTelemetrySubsystem::OnLanternStateChanged(const bool bIsOn, ELLKLanternChangeReason Reason)
{
	// Every keeper's lantern, counted as depth by the metrics layer: lit
	// seconds are seconds with at least one light burning, which is the
	// question row 3 asks. Who was holding it is not recorded.
	Record(bIsOn ? ELLKRunEvent::LanternOn : ELLKRunEvent::LanternOff);
}

void ULLKRunTelemetrySubsystem::OnCarriedCountChanged(const int32 CarriedCount)
{
	// **Only an increase is a take.** The count also drops - a wipe clears it
	// (row 8) - and recording that as a relic event would make a crew that
	// lost everything look like a crew that looted twice as much.
	//
	// Per-interactor, because four keepers share this handler and one crew
	// member putting something down must not read as another picking it up.
	int32 Increases = 0;
	for (TPair<TWeakObjectPtr<ULLKInteractorComponent>, int32>& Pair : LastCarried)
	{
		const ULLKInteractorComponent* Interactor = Pair.Key.Get();
		if (Interactor == nullptr)
		{
			continue;
		}

		const int32 Now = Interactor->GetCarriedCount();
		if (Now > Pair.Value)
		{
			Increases += Now - Pair.Value;
		}
		Pair.Value = Now;
	}

	for (int32 Taken = 0; Taken < Increases; ++Taken)
	{
		Record(ELLKRunEvent::RelicTaken);
	}
}

FString ULLKRunTelemetrySubsystem::WriteToDisk()
{
	const FLLKRunSummary Summary = LLKRunMetrics::Summarise(Events);

	// Logged whatever happens with the file. A playtest where the write failed
	// should still leave its numbers somewhere, and the log is somewhere.
	UE_LOG(LogLLK, Log, TEXT("\n%s"), *LLKRunMetrics::Describe(Summary));

	const FString Directory = FPaths::ProjectSavedDir() / TEXT("LLKTelemetry");
	IPlatformFile& Platform = FPlatformFileManager::Get().GetPlatformFile();
	if (!Platform.DirectoryExists(*Directory))
	{
		Platform.CreateDirectoryTree(*Directory);
	}

	// Wall clock in the NAME only - the events inside are all relative to the
	// run's own start, so a file says when it was recorded without any event
	// carrying a date. Nothing here identifies a machine or a person.
	const FString Path = Directory / FString::Printf(TEXT("run-%s.json"),
		*FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S")));

	if (!FFileHelper::SaveStringToFile(LLKRunMetrics::ToJson(Events, Summary), *Path))
	{
		UE_LOG(LogLLK, Warning, TEXT("Could not write run telemetry to %s."), *Path);
		return FString();
	}

	bWritten = true;
	UE_LOG(LogLLK, Log, TEXT("Run telemetry written to %s"), *Path);
	return Path;
}

// ---- console commands ----------------------------------------------------

static FAutoConsoleCommandWithWorld GLLKTelemetryShow(
	TEXT("LLK.Telemetry.Show"),
	TEXT("Prints what this run has been so far. Changes nothing."),
	FConsoleCommandWithWorldDelegate::CreateLambda(
		[](UWorld* World)
		{
			const ULLKRunTelemetrySubsystem* Telemetry =
				World != nullptr ? World->GetSubsystem<ULLKRunTelemetrySubsystem>() : nullptr;
			if (Telemetry == nullptr)
			{
				UE_LOG(LogLLK, Warning, TEXT("No telemetry subsystem in this world."));
				return;
			}

			UE_LOG(LogLLK, Display, TEXT("\n%s\n(%d event(s) recorded)"),
				*LLKRunMetrics::Describe(Telemetry->Summarise()), Telemetry->CountEvents());
		}));

static FAutoConsoleCommandWithWorld GLLKTelemetryWrite(
	TEXT("LLK.Telemetry.Write"),
	TEXT("Writes this run to Saved/LLKTelemetry/ without waiting for it to end."),
	FConsoleCommandWithWorldDelegate::CreateLambda(
		[](UWorld* World)
		{
			if (ULLKRunTelemetrySubsystem* Telemetry =
					World != nullptr ? World->GetSubsystem<ULLKRunTelemetrySubsystem>() : nullptr)
			{
				const FString Path = Telemetry->WriteToDisk();
				UE_LOG(LogLLK, Display, TEXT("%s"),
					Path.IsEmpty() ? TEXT("Nothing written.") : *Path);
			}
		}));

FName ULLKRunTelemetrySubsystem::NameOfBlock(const ELLKRelicInteractBlock Block)
{
	// Spelled out rather than taken from the reflected enum, and the reason is
	// that these strings end up in a run file that `tests/` reads: a rename of
	// the enumerator must not silently change the shape of last month's data.
	switch (Block)
	{
	case ELLKRelicInteractBlock::None:         return TEXT("None");
	case ELLKRelicInteractBlock::Unidentified: return TEXT("Unidentified");
	case ELLKRelicInteractBlock::NeedsLight:   return TEXT("NeedsLight");
	}

	// Not NAME_None: the arithmetic counts an unrecognised reason as a refusal
	// rather than a success, and "Unrecognised" says so in the file where
	// NAME_None would read as a missing field.
	return TEXT("Unrecognised");
}

void ULLKRunTelemetrySubsystem::OnInteractAttempted(
	ULLKRelicComponent* Relic, const ELLKRelicInteractBlock Block)
{
	// The relic is in the signature because a later question will want to know
	// WHICH relic kept being refused, not only how often. Unused here, and
	// deliberately not recorded: the id of a relic a player failed to read is
	// the one piece of this event that is about the object rather than the
	// rule, and the rule is what row 43 is judged on.
	(void)Relic;

	Record(ELLKRunEvent::InteractAttempted, NameOfBlock(Block));
}

void ULLKRunTelemetrySubsystem::PollAbandonedRelics(const float DeltaSeconds)
{
	SecondsSinceAbandonPoll += DeltaSeconds;
	if (SecondsSinceAbandonPoll < AbandonPollSeconds)
	{
		return;
	}
	SecondsSinceAbandonPoll = 0.f;

	for (TPair<TWeakObjectPtr<ULLKRelicComponent>, float>& Pair : TakeableSince)
	{
		ULLKRelicComponent* Relic = Pair.Key.Get();
		if (Relic == nullptr)
		{
			continue;
		}

		// `CanBeTaken` rather than a raw light sample: it is the same answer
		// the prompt and the outline are drawn from, and since `ab9818f` its
		// light half is debounced over two samples. An instrument timing a
		// raw trace would manufacture an abandonment every time a keeper stood
		// at a shelf edge, which is exactly where the hooded reading loop
		// happens.
		const bool bTakeableNow = Relic->CanBeTaken();
		const float Since = Pair.Value;

		if (bTakeableNow)
		{
			if (Since < 0.f)
			{
				Pair.Value = RunSeconds;
			}
			continue;
		}

		if (Since < 0.f)
		{
			// Was not available and still is not. Nothing happened.
			continue;
		}

		Pair.Value = -1.f;

		if (Relic->IsTaken())
		{
			// It stopped being takeable because somebody took it. That is a
			// `RelicTaken`, already recorded by the carried count, and
			// counting it here as well would make every successful pickup
			// read as a relic the player failed to understand.
			continue;
		}

		const float Dwell = RunSeconds - Since;
		if (Dwell >= AbandonDwellSeconds)
		{
			Record(ELLKRunEvent::RelicAbandoned, Relic->GetRelicId(), Dwell);
		}
	}
}
