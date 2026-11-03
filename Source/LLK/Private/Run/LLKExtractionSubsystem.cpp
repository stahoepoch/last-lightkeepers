#include "Run/LLKExtractionSubsystem.h"

#include "Characters/LLKVitalsComponent.h"
#include "Data/LLKLootRulesData.h"
#include "Data/LLKZonePressureData.h"
#include "Engine/World.h"
#include "LLK.h"
#include "LLKGameState.h"
#include "Lantern/LLKLanternLightComponent.h"
#include "Loot/LLKInteractorComponent.h"
#include "Run/LLKExtractionZone.h"
#include "Run/LLKRunStateSubsystem.h"
#include "Storm/LLKZonePressureSubsystem.h"

bool ULLKExtractionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// Game and PIE, as with the storm and the run state. An editor preview
	// world announcing an extraction would be a strange thing to debug.
	const UWorld* World = Cast<UWorld>(Outer);
	return World != nullptr && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void ULLKExtractionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResolveRules();
}

TStatId ULLKExtractionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULLKExtractionSubsystem, STATGROUP_Tickables);
}

void ULLKExtractionSubsystem::ResolveRules()
{
	LootRules = ::LoadObject<ULLKLootRulesData>(nullptr, TEXT("/Game/Data/DA_LootRules.DA_LootRules"));
	if (LootRules == nullptr)
	{
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKExtractionSubsystem: no /Game/Data/DA_LootRules - there are no exits in this run. ")
			TEXT("Run Tools/import_data.py."));
	}

	// Two of `extractionCall`'s three fields, and the omission is the rule.
	// Row 73: the call cannot move the clock, so the spike beside these is
	// never read and `FLLKExtractionAnnounce` has nowhere to put it.
	if (const ULLKZonePressureData* Storm =
			::LoadObject<ULLKZonePressureData>(nullptr, TEXT("/Game/Data/DA_ZonePressure.DA_ZonePressure")))
	{
		Announce.SirenSeconds = Storm->ExtractionCall.SirenSeconds;
		Announce.bConvergeAllPatrols = Storm->ExtractionCall.bConvergeAllPatrols;
	}
	else
	{
		// Declared fallback, matching `zone-pressure.json`'s authored values,
		// so a missing asset gives a run with a real siren rather than an
		// extraction that completes in silence the instant it is called.
		Announce.SirenSeconds = 8.f;
		Announce.bConvergeAllPatrols = true;
		UE_LOG(LogLLK, Warning,
			TEXT("ULLKExtractionSubsystem: no DA_ZonePressure - DECLARED FALLBACK siren of %.0fs."),
			Announce.SirenSeconds);
	}
}

bool ULLKExtractionSubsystem::HasExtractionAuthority() const
{
	if (bHasForcedAuthority)
	{
		return bForcedAuthority;
	}

	const UWorld* World = GetWorld();
	return World != nullptr && World->GetNetMode() != NM_Client;
}

ULLKRunStateSubsystem* ULLKExtractionSubsystem::GetRunState() const
{
	const UWorld* World = GetWorld();
	return World != nullptr ? World->GetSubsystem<ULLKRunStateSubsystem>() : nullptr;
}

TArray<FName> ULLKExtractionSubsystem::GetExitIds() const
{
	TArray<FName> Ids;
	if (LootRules == nullptr)
	{
		return Ids;
	}

	for (const FLLKExtractionPoint& Point : LootRules->Extraction)
	{
		if (!Point.Id.IsNone())
		{
			Ids.Add(Point.Id);
		}
	}
	return Ids;
}

bool ULLKExtractionSubsystem::FindExit(
	const FName PointId, FLLKExtractionPointRules& OutPoint, FName& OutRequires) const
{
	if (LootRules == nullptr)
	{
		return false;
	}

	for (const FLLKExtractionPoint& Point : LootRules->Extraction)
	{
		if (Point.Id != PointId)
		{
			continue;
		}

		OutPoint.Id = Point.Id;
		OutPoint.HoldSeconds = Point.ArrivalSeconds;
		OutPoint.PayoutModifier = Point.PayoutModifier;
		OutRequires = Point.Requires;
		return true;
	}

	return false;
}

void ULLKExtractionSubsystem::SetRequirementSatisfied(const FName Requirement, const bool bSatisfied)
{
	if (Requirement.IsNone())
	{
		return;
	}

	if (bSatisfied)
	{
		SatisfiedRequirements.Add(Requirement);
	}
	else
	{
		SatisfiedRequirements.Remove(Requirement);
	}
}

bool ULLKExtractionSubsystem::IsRequirementSatisfied(const FName Requirement) const
{
	// An exit with no requirement is always satisfied, which is the dock.
	return Requirement.IsNone() || SatisfiedRequirements.Contains(Requirement);
}

ELLKExtractionRefusal ULLKExtractionSubsystem::CallExtraction(const FName PointId, AActor* Caller)
{
	if (!HasExtractionAuthority())
	{
		// Not a refusal code: a client asking is not a crew being told no, and
		// giving it a reason would imply the server considered the request.
		UE_LOG(LogLLK, Warning, TEXT("CallExtraction ignored: no authority."));
		return ELLKExtractionRefusal::None;
	}

	FLLKExtractionPointRules Point;
	FName Requires;
	if (!FindExit(PointId, Point, Requires))
	{
		UE_LOG(LogLLK, Warning,
			TEXT("EXTRACTION REFUSED (%s): '%s' is not an authored exit. Exits: %s"),
			LLKExtraction::NameOf(ELLKExtractionRefusal::NoSuchPoint),
			*PointId.ToString(),
			GetExitIds().IsEmpty() ? TEXT("none") : *FString::JoinBy(
				GetExitIds(), TEXT(", "), [](const FName& Id) { return Id.ToString(); }));
		return ELLKExtractionRefusal::NoSuchPoint;
	}

	const ULLKRunStateSubsystem* Run = GetRunState();
	const bool bRunIsOver = Run != nullptr && Run->IsRunOver();

	ELLKExtractionRefusal Refusal = ELLKExtractionRefusal::None;
	const FLLKExtractionState Next = LLKExtraction::Call(
		State, Point, Announce, IsRequirementSatisfied(Requires), bRunIsOver, Refusal);

	if (Refusal != ELLKExtractionRefusal::None)
	{
		UE_LOG(LogLLK, Warning, TEXT("EXTRACTION REFUSED (%s): %s%s"),
			LLKExtraction::NameOf(Refusal),
			*PointId.ToString(),
			Refusal == ELLKExtractionRefusal::RequirementNotMet
				? *FString::Printf(TEXT(" needs %s"), *Requires.ToString())
				: TEXT(""));
		return Refusal;
	}

	State = Next;
	bHoldStalled = false;
	bWarnedAboutMissingZone = false;

	UE_LOG(LogLLK, Log,
		TEXT("EXTRACTION CALLED at %s. Siren %.0fs, then hold %.0fs. Call %d this run."),
		*State.PointId.ToString(), State.SirenSecondsRemaining, State.HoldSecondsRequired,
		State.CallCount);

	// Row 74's convergence, in the currency the game already has. The rig
	// arrives with its lamps lit and the dock's own bay lamps and shutter
	// motors come on with it, which makes the boarding route the brightest
	// thing in a building full of machines that watch for light - so the
	// convergence is a light source in the field rather than a flag an AI
	// consults. The Warden already converges on the brightest position (rows 7
	// and 54) and `LLKPerception` already notices a lit keeper from 38 studs
	// instead of 12; both of those now point at the dock without learning that
	// extraction exists.
	//
	// **It is re-armed every tick rather than lit once for a computed
	// duration**, which is row 75's pause rule showing up somewhere it was not
	// obvious. Chris: *"the siren, rig lights, and converged patrols persist,
	// so stepping away buys survival time, not a free reset or a safer
	// retry."* A pulse of siren-plus-hold seconds would have expired during a
	// long pause and left the dock dark while the extraction was still
	// underway - which would have made hiding the safe play and inverted the
	// rule it was supposed to serve.
	//
	// **The brightness is the one unauthored number here**, and the header
	// says whose it is. The caller's own lantern stands in for the rig.
	if (Announce.bConvergeAllPatrols)
	{
		RigIntensity = 0.f;
		RigRadiusCm = 0.f;
		RigLightFallback = Caller;

		if (const ULLKLanternLightComponent* Light = Caller != nullptr
				? Caller->FindComponentByClass<ULLKLanternLightComponent>()
				: nullptr)
		{
			RigIntensity = Light->GetResolvedRules().Intensity;
			RigRadiusCm = Light->AttenuationRadius;
		}

		RefreshRigLight();
	}

	OnExtractionCalled.Broadcast(State.PointId, State.SirenSecondsRemaining);
	PublishSnapshot();
	return ELLKExtractionRefusal::None;
}

bool ULLKExtractionSubsystem::CancelExtraction()
{
	if (!HasExtractionAuthority())
	{
		return false;
	}

	const FName Was = State.PointId;

	bool bCancelled = false;
	const FLLKExtractionState Next = LLKExtraction::Cancel(State, bCancelled);
	if (!bCancelled)
	{
		return false;
	}

	State = Next;
	bHoldStalled = false;

	UE_LOG(LogLLK, Log,
		TEXT("EXTRACTION CANCELLED at %s. The siren is already spent - %d call(s) and %d cancel(s) this run."),
		*Was.ToString(), State.CallCount, State.CancelCount);

	OnExtractionCancelled.Broadcast(Was);
	PublishSnapshot();
	return true;
}

void ULLKExtractionSubsystem::RegisterZone(ALLKExtractionZone* Zone)
{
	if (Zone == nullptr)
	{
		return;
	}

	if (Zone->ExtractionPointId.IsNone())
	{
		UE_LOG(LogLLK, Warning,
			TEXT("%s has no ExtractionPointId, so no exit can use it. Set it to one of: %s"),
			*Zone->GetName(),
			GetExitIds().IsEmpty()
				? TEXT("none - run Tools/import_data.py")
				: *FString::JoinBy(GetExitIds(), TEXT(", "),
					[](const FName& Id) { return Id.ToString(); }));
		return;
	}

	if (const ALLKExtractionZone* Existing = FindZone(Zone->ExtractionPointId))
	{
		if (Existing != Zone)
		{
			// Refused rather than replaced. Replacing would make the answer
			// depend on which zone began play last, which is not a thing
			// anybody placing two of them could reason about.
			UE_LOG(LogLLK, Warning,
				TEXT("%s is a second zone for %s - %s already is. Ignoring the new one: one exit, ")
				TEXT("one boarding route, or the hold could be advanced from two places."),
				*Zone->GetName(), *Zone->ExtractionPointId.ToString(), *Existing->GetName());
		}
		return;
	}

	Zones.Add(Zone);

	UE_LOG(LogLLK, Log, TEXT("Extraction zone registered for %s: %s."),
		*Zone->ExtractionPointId.ToString(), *Zone->GetName());
}

void ULLKExtractionSubsystem::UnregisterZone(ALLKExtractionZone* Zone)
{
	Zones.RemoveAll([Zone](const TWeakObjectPtr<ALLKExtractionZone>& Candidate)
	{
		return !Candidate.IsValid() || Candidate.Get() == Zone;
	});
}

ALLKExtractionZone* ULLKExtractionSubsystem::FindZone(const FName PointId) const
{
	if (PointId.IsNone())
	{
		return nullptr;
	}

	for (const TWeakObjectPtr<ALLKExtractionZone>& Weak : Zones)
	{
		ALLKExtractionZone* Zone = Weak.Get();
		if (Zone != nullptr && Zone->ExtractionPointId == PointId)
		{
			return Zone;
		}
	}
	return nullptr;
}

TOptional<int32> ULLKExtractionSubsystem::ObserveBoardingRoute() const
{
	// Nobody measured. The distinction this `TOptional` exists for: no dock in
	// the map is a different fact from a dock with nobody standing in it, and
	// only one of them is the crew's fault.
	const ALLKExtractionZone* Zone = FindZone(State.PointId);
	if (Zone == nullptr)
	{
		return TOptional<int32>();
	}

	// `CountKeepersStanding`, not `CountKeepersInside`. Row 75: a downed
	// keeper does not hold the route, because row 74 is explicit that no
	// driver comes into the building to collect anybody. The zone reports both
	// numbers and this is the one that is a rule.
	return Zone->CountKeepersStanding();
}

void ULLKExtractionSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasExtractionAuthority())
	{
		return;
	}

	const ULLKRunStateSubsystem* Run = GetRunState();
	const bool bCrewWiped = Run != nullptr && Run->HasCrewWiped();

	// Idle and nothing to report: no state machine, no snapshot, no cost.
	if (State.Phase == ELLKExtractionPhase::Idle && !bCrewWiped)
	{
		return;
	}

	if (State.IsTerminal())
	{
		return;
	}

	FLLKExtractionInput Input;
	Input.DeltaSeconds = DeltaTime;
	Input.bCrewWiped = bCrewWiped;
	if (State.Phase == ELLKExtractionPhase::Holding)
	{
		Input.KeepersOnBoardingRoute = ObserveBoardingRoute();
	}

	Apply(LLKExtraction::Advance(State, Input));

	// While it is underway at all - announcing or holding, moving or stalled.
	// A stalled hold is the case that matters: the operation is still running,
	// so the dock is still lit and the building is still interested.
	if (State.IsUnderway())
	{
		RefreshRigLight();
	}

	SecondsSinceSnapshot += DeltaTime;
	if (SecondsSinceSnapshot >= SnapshotIntervalSeconds)
	{
		SecondsSinceSnapshot = 0.f;
		PublishSnapshot();
	}
}

void ULLKExtractionSubsystem::Apply(const FLLKExtractionStep& Step)
{
	State = Step.State;
	bHoldStalled = Step.bHoldStalled;

	if (Step.bRigArrived)
	{
		UE_LOG(LogLLK, Log,
			TEXT("THE RIG IS AT %s. Hold the boarding route for %.0fs."),
			*State.PointId.ToString(), State.HoldSecondsRequired);
		OnRigArrived.Broadcast(State.PointId);
	}

	// The honest failure the greybox needs. Said once per call and by name,
	// because an extraction that never finishes and never explains itself is
	// indistinguishable from a bug in the state machine - which is exactly the
	// confusion `LLK.CheckMap` was written to prevent for enemies.
	if (Step.bHoldStalled && !bWarnedAboutMissingZone
		&& State.Phase == ELLKExtractionPhase::Holding
		&& FindZone(State.PointId) == nullptr)
	{
		bWarnedAboutMissingZone = true;
		UE_LOG(LogLLK, Warning,
			TEXT("EXTRACTION CANNOT COMPLETE: no ALLKExtractionZone registered for %s, so the hold ")
			TEXT("will never advance. The dock is map work - see DECISIONS.md rows 74 and 75 and ")
			TEXT("docs/MAP-READINESS.md."),
			*State.PointId.ToString());
	}

	if (Step.bSecured)
	{
		ResolveSecuredHaul();

		// **The run now has a success terminus.** Until 2026-11-03 this
		// branch logged, credited telemetry and changed no run state, so
		// `bRunIsOver` below was `HasCrewWiped()` alone - a run could be lost
		// and could not be won. Row 8 makes extraction the only thing that
		// secures loot, which makes this the winning end of the night.
		if (ULLKRunStateSubsystem* Run = GetRunState())
		{
			Run->MarkRunSecured();
		}

		PublishSnapshot();
		return;
	}

	if (Step.bFailed)
	{
		UE_LOG(LogLLK, Log,
			TEXT("EXTRACTION FAILED. The crew is contained and the night keeps the haul (row 8)."));
		OnExtractionFailed.Broadcast();
		PublishSnapshot();
	}
}

void ULLKExtractionSubsystem::RefreshRigLight()
{
	if (!Announce.bConvergeAllPatrols || !(RigIntensity > 0.f) || !(RigRadiusCm > 0.f))
	{
		// No convergence authored, or a caller with no lantern to borrow
		// brightness from. `AddOrRefreshDistressPulse` would refuse it anyway;
		// refused silently here rather than warning once a frame.
		return;
	}

	UWorld* World = GetWorld();
	ULLKZonePressureSubsystem* Storm = World != nullptr
		? World->GetSubsystem<ULLKZonePressureSubsystem>() : nullptr;
	if (Storm == nullptr)
	{
		return;
	}

	// At the dock, not at the caller: the light is the operation, and the
	// operation is at the bay whether the crew is standing in it or not. The
	// caller's position is a fallback for a map with no route registered,
	// which cannot really extract anyway.
	FVector Position;
	if (const ALLKExtractionZone* Zone = FindZone(State.PointId))
	{
		Position = Zone->GetActorLocation();
	}
	else if (const AActor* Fallback = RigLightFallback.Get())
	{
		Position = Fallback->GetActorLocation();
	}
	else
	{
		return;
	}

	Storm->AddOrRefreshDistressPulse(
		RigLightSourceName(), Position, RigIntensity, RigRadiusCm, RigLightRefreshSeconds);
}

FName ULLKExtractionSubsystem::RigLightSourceName()
{
	// One name for the whole run, because there is one rig and one dock cycle
	// at a time. Two concurrent extractions would need two, and row 75's
	// AlreadyUnderway refusal is what guarantees there is never a second.
	static const FName Name(TEXT("ExtractionRig"));
	return Name;
}

void ULLKExtractionSubsystem::ResolveSecuredHaul()
{
	// Counted here rather than left to a listener, for the reason
	// `ULLKRunStateSubsystem::ResolveHaul` gives about the wipe: the two halves
	// of row 8 - extraction secures, everything else loses - should not be one
	// function and one delegate somebody forgot to bind.
	//
	// **What this deliberately does NOT do is bank anything.** There is no
	// Lanternhold vault in the build yet, and inventing one here would be a
	// second owner of the haul. The count and the payout go out as an event,
	// and the first real consumer is the hub work in a later milestone.
	int32 Secured = 0;
	if (const ULLKRunStateSubsystem* Run = GetRunState())
	{
		for (const ULLKVitalsComponent* Keeper : Run->GetKeepers())
		{
			const AActor* Owner = Keeper != nullptr ? Keeper->GetOwner() : nullptr;
			if (Owner == nullptr)
			{
				continue;
			}

			if (const ULLKInteractorComponent* Carried =
					Owner->FindComponentByClass<ULLKInteractorComponent>())
			{
				Secured += Carried->GetCarriedCount();
			}
		}
	}

	UE_LOG(LogLLK, Log,
		TEXT("EXTRACTED at %s. %d item(s) out at a payout of %.2fx."),
		*State.PointId.ToString(), Secured, State.PayoutModifier);

	OnExtractionSecured.Broadcast(State.PointId, State.PayoutModifier, Secured);
}

void ULLKExtractionSubsystem::PublishSnapshot()
{
	UWorld* World = GetWorld();
	if (World == nullptr || !HasExtractionAuthority())
	{
		return;
	}

	ALLKGameState* GameState = World->GetGameState<ALLKGameState>();
	if (GameState == nullptr)
	{
		// Quietly, unlike the storm's one-time warning: the storm's whole
		// readout is dead without a GameState, while extraction still runs
		// correctly on the server with nobody listening.
		return;
	}

	FLLKExtractionSnapshot Snapshot;
	Snapshot.Phase = State.Phase;
	Snapshot.PointId = State.PointId;
	Snapshot.SirenSecondsRemaining = State.SirenSecondsRemaining;
	Snapshot.HoldSecondsRemaining = State.HoldSecondsRemaining;
	Snapshot.HoldSecondsRequired = State.HoldSecondsRequired;
	Snapshot.bHoldStalled = bHoldStalled;
	Snapshot.bPublished = true;

	GameState->PublishExtractionSnapshot(Snapshot);
}

// ---- console commands ----------------------------------------------------
//
// The same affordance `LLK.CheckMap` has, and for the same reason: there is no
// input binding for extraction yet (Enhanced Input is Codex's lane and the
// dock has no interactable), so without these the state machine could only be
// exercised by a test. A rule nobody can play is a rule nobody can feel.

static ULLKExtractionSubsystem* FindExtraction(UWorld* World)
{
	return World != nullptr ? World->GetSubsystem<ULLKExtractionSubsystem>() : nullptr;
}

static FAutoConsoleCommandWithWorldAndArgs GLLKCallExtraction(
	TEXT("LLK.CallExtraction"),
	TEXT("Calls extraction at the named exit, e.g. LLK.CallExtraction LoadingDock. Server only."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			ULLKExtractionSubsystem* Extraction = FindExtraction(World);
			if (Extraction == nullptr)
			{
				UE_LOG(LogLLK, Warning, TEXT("No extraction subsystem in this world."));
				return;
			}

			if (Args.Num() < 1)
			{
				const TArray<FName> Ids = Extraction->GetExitIds();
				UE_LOG(LogLLK, Display, TEXT("Usage: LLK.CallExtraction <exit>. Authored exits: %s"),
					Ids.IsEmpty() ? TEXT("none - run Tools/import_data.py") : *FString::JoinBy(
						Ids, TEXT(", "), [](const FName& Id) { return Id.ToString(); }));
				return;
			}

			APawn* Caller = nullptr;
			if (const APlayerController* Controller = World->GetFirstPlayerController())
			{
				Caller = Controller->GetPawn();
			}

			Extraction->CallExtraction(FName(*Args[0]), Caller);
		}));

static FAutoConsoleCommandWithWorld GLLKCancelExtraction(
	TEXT("LLK.CancelExtraction"),
	TEXT("Calls off an extraction that has not resolved. The siren is not refunded."),
	FConsoleCommandWithWorldDelegate::CreateLambda(
		[](UWorld* World)
		{
			if (ULLKExtractionSubsystem* Extraction = FindExtraction(World))
			{
				if (!Extraction->CancelExtraction())
				{
					UE_LOG(LogLLK, Display, TEXT("Nothing to cancel."));
				}
			}
		}));

static FAutoConsoleCommandWithWorldAndArgs GLLKSatisfyRequirement(
	TEXT("LLK.Extraction.Satisfy"),
	TEXT("Marks an exit requirement satisfied by hand, e.g. SubBasementBreakerRestored. "
		 "Stands in for the restoration system until there is one."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			ULLKExtractionSubsystem* Extraction = FindExtraction(World);
			if (Extraction == nullptr || Args.Num() < 1)
			{
				UE_LOG(LogLLK, Display, TEXT("Usage: LLK.Extraction.Satisfy <requirement> [0|1]"));
				return;
			}

			const bool bSatisfied = Args.Num() < 2 || Args[1] != TEXT("0");
			Extraction->SetRequirementSatisfied(FName(*Args[0]), bSatisfied);
			UE_LOG(LogLLK, Display, TEXT("%s is now %s."),
				*Args[0], bSatisfied ? TEXT("satisfied") : TEXT("unsatisfied"));
		}));

static FAutoConsoleCommandWithWorld GLLKExtractionStatus(
	TEXT("LLK.Extraction.Status"),
	TEXT("Prints the extraction state machine's current phase and hold."),
	FConsoleCommandWithWorldDelegate::CreateLambda(
		[](UWorld* World)
		{
			const ULLKExtractionSubsystem* Extraction = FindExtraction(World);
			if (Extraction == nullptr)
			{
				return;
			}

			const FLLKExtractionState& State = Extraction->GetState();
			UE_LOG(LogLLK, Display,
				TEXT("Extraction: %s at %s. Siren %.1fs, hold %.1f/%.1fs (%.0f%%)%s. %d call(s), %d cancel(s)."),
				LLKExtraction::NameOf(State.Phase),
				State.PointId.IsNone() ? TEXT("nowhere") : *State.PointId.ToString(),
				State.SirenSecondsRemaining,
				State.HoldSecondsRequired - State.HoldSecondsRemaining,
				State.HoldSecondsRequired,
				LLKExtraction::HoldFraction(State) * 100.f,
				Extraction->WasHoldStalledLastStep() ? TEXT(" STALLED") : TEXT(""),
				State.CallCount, State.CancelCount);
		}));
