#include "Enemies/LLKSweeper.h"

#include "Characters/LLKCharacter.h"
#include "Characters/LLKStumbleCameraShake.h"
#include "Characters/LLKVitalsComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/LLKEnemyDef.h"
#include "Data/LLKLanternData.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Enemies/LLKEnemyRosterSubsystem.h"
#include "Enemies/LLKHearing.h"
#include "Enemies/LLKNoiseSource.h"
#include "Enemies/LLKPerception.h"
#include "Enemies/LLKVisibility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "LLK.h"
#include "Loot/LLKRelicComponent.h"
#include "LLKUnits.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Run/LLKRunStateSubsystem.h"
#include "Storm/LLKLightField.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "World/LLKPushFeel.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Enemies/LLKSweeperPresence.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

const FName ALLKSweeper::PatrolPointTag(TEXT("LLK.Patrol"));
const FName ALLKSweeper::StockBayTag(TEXT("LLK.StockBay"));

namespace
{
	/**
	 * How long the floor stays alerted when a machine files a fault and the
	 * enemy authored no `losesTargetAfterSeconds` of its own.
	 *
	 * Sweepers do not author that key - only Stockers do - so this is the
	 * value they actually get. Same precedent as the brain's fallbacks:
	 * declared here rather than invented in `enemies.json`, because every
	 * number in that file is Chris's. Three seconds is "quickly", which was
	 * his word for it.
	 */
	constexpr float FallbackFloorAlertSeconds = 3.f;
}

ALLKSweeper::ALLKSweeper()
{
	PrimaryActorTick.bCanEverTick = true;

	// **No capsule size, no gravity, no acceleration, and no world collision
	// tuning.** The map owns the Sweeper's physical scale against shelves and
	// doorways. Keeper contact is different: it is a gameplay event measured by
	// the brain, never a reason to make the machine a moving wall.
	//
	// The one movement value set here is the authored speed, and it is set in
	// BeginPlay rather than the constructor because it comes from a data asset.
	bReplicates = true;
	SetReplicateMovement(true);

	// A Sweeper may clip a keeper's feet, apply its authored stumble, then
	// search their last-known position. It must never trap them in an aisle by
	// blocking the Pawn channel. The capsule still blocks WorldStatic, so this
	// does not let it walk through gondolas or walls; it only makes keeper
	// contact an overlap, resolved by the brain's range/cooldown rules.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// A brain without a body was technically moving through the aisle, but the
	// player only received its debug text. This deliberately crude maintenance
	// cart makes the first encounter legible until authored enemy art replaces
	// it. It is presentation only: collision remains ACharacter's capsule.
	const ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	const ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	const ConstructorHelpers::FObjectFinder<UMaterialInterface> BeaconMaterial(
		TEXT("/Game/LLK/Materials/MI_LLK_LanternCore_Working.MI_LLK_LanternCore_Working"));
	const ConstructorHelpers::FObjectFinder<USoundBase> TemporaryCue(
		TEXT("/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"));
	const ConstructorHelpers::FObjectFinder<USoundBase> StockStartedCue(
		TEXT("/Game/Audio/Sweeper/sweeper-stock-started-v1.sweeper-stock-started-v1"));
	const ConstructorHelpers::FObjectFinder<USoundBase> StockInterruptedCue(
		TEXT("/Game/Audio/Sweeper/sweeper-stock-interrupted-v1.sweeper-stock-interrupted-v1"));
	const ConstructorHelpers::FObjectFinder<USoundBase> StockFiledCue(
		TEXT("/Game/Audio/Sweeper/sweeper-stock-filed-v1.sweeper-stock-filed-v1"));

	Chassis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SweeperChassis_Temporary"));
	Chassis->SetupAttachment(GetCapsuleComponent());
	Chassis->SetStaticMesh(Cube.Object);
	Chassis->SetRelativeLocation(FVector(0.f, 0.f, -48.f));
	Chassis->SetRelativeScale3D(FVector(0.48f, 0.34f, 0.26f));
	Chassis->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// **NoCollision is not enough, and this cost an evening of play.**
	//
	// Navigation relevance is independent of collision enablement in Unreal.
	// A decorative mesh with `ECollisionEnabled::NoCollision` still carves the
	// navmesh unless this is set, and `RecastNavMesh-Default` in Boswell's runs
	// `RuntimeGeneration = Dynamic` - so this chassis punched a nav hole
	// centred on the Sweeper, which then FOLLOWED it. It could never path out
	// of a hole it was carrying.
	//
	// The symptom was not a stuck pawn, which would have been obvious. The
	// Sweeper noticed the crate correctly (perception does not consult the
	// navmesh), began the filing job, could not take a step, burnt the
	// `losesTargetAfterSeconds` approach budget, gave up, re-noticed, and
	// looped - one abandon warning every three seconds while standing
	// perfectly still. Measured 2026-11-02, from Chris's "sweeper didn't move
	// anywhere or do anything".
	//
	// Every decorative component on a moving pawn needs this line. Nothing in
	// the project set it anywhere before now, so the default - true - applied
	// to all of them. `tests/wiring.test.mjs` guards it.
	Chassis->SetCanEverAffectNavigation(false);

	BeaconHousing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SweeperBeaconHousing_Temporary"));
	BeaconHousing->SetupAttachment(GetCapsuleComponent());
	BeaconHousing->SetStaticMesh(Cylinder.Object);
	BeaconHousing->SetRelativeLocation(FVector(0.f, 0.f, -7.f));
	BeaconHousing->SetRelativeScale3D(FVector(0.12f, 0.12f, 0.08f));
	BeaconHousing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeaconHousing->SetCanEverAffectNavigation(false);
	if (BeaconMaterial.Succeeded())
	{
		BeaconHousing->SetMaterial(0, BeaconMaterial.Object);
	}

	// A cue has to be spatial before it can be a telegraph.  A global UI blip
	// tells a player that *something* happened; a bounded source tells them
	// where a machine has committed.  The range uses the authored bright
	// Sweeper reach, not a second hidden gameplay radius.
	CueAttenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("SweeperCueAttenuation"));
	CueAttenuation->Attenuation.bAttenuate = true;
	CueAttenuation->Attenuation.bSpatialize = true;
	CueAttenuation->Attenuation.DistanceAlgorithm = EAttenuationDistanceModel::Logarithmic;
	CueAttenuation->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	CueAttenuation->Attenuation.AttenuationShapeExtents = FVector(LLKUnits::DistanceFromStuds(38.f), 0.f, 0.f);
	CueAttenuation->Attenuation.FalloffDistance = LLKUnits::DistanceFromStuds(12.f);

	// The motor. Reaches further than a cue on purpose - see
	// PresenceAttenuation in the header for why hearing a machine must never
	// imply being noticed by it.
	PresenceAttenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("SweeperPresenceAttenuation"));
	PresenceAttenuation->Attenuation.bAttenuate = true;
	PresenceAttenuation->Attenuation.bSpatialize = true;
	PresenceAttenuation->Attenuation.DistanceAlgorithm = EAttenuationDistanceModel::Logarithmic;
	PresenceAttenuation->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	PresenceAttenuation->Attenuation.AttenuationShapeExtents =
		FVector(LLKUnits::DistanceFromStuds(60.f), 0.f, 0.f);
	PresenceAttenuation->Attenuation.FalloffDistance = LLKUnits::DistanceFromStuds(42.f);

	Presence = CreateDefaultSubobject<UAudioComponent>(TEXT("SweeperPresence"));
	Presence->SetupAttachment(RootComponent);
	// Started in BeginPlay instead, and only on authority: a client's Sweeper
	// moves because movement replicates, so it needs the sound, but a
	// commandlet or a test world should not have a motor running in it.
	Presence->bAutoActivate = false;
	Presence->bAllowSpatialization = true;
	Presence->AttenuationSettings = PresenceAttenuation;
	// A machine heard through a gondola is still a machine. Occlusion would be
	// the right refinement later; muffling it to nothing would give back the
	// defect this exists to remove.
	Presence->SetVolumeMultiplier(0.f);

	static ConstructorHelpers::FObjectFinder<USoundBase> PresenceLoop(
		TEXT("/Game/Audio/Sweeper/sweeper-presence-v1.sweeper-presence-v1"));
	if (PresenceLoop.Succeeded())
	{
		PresenceSound = PresenceLoop.Object;
	}

	if (TemporaryCue.Succeeded())
	{
		TemporaryCueSound = TemporaryCue.Object;
	}
	if (StockStartedCue.Succeeded())
	{
		StockStartedCueSound = StockStartedCue.Object;
	}
	if (StockInterruptedCue.Succeeded())
	{
		StockInterruptedCueSound = StockInterruptedCue.Object;
	}
	if (StockFiledCue.Succeeded())
	{
		StockFiledCueSound = StockFiledCue.Object;
	}
}

bool ALLKSweeper::HasEnemyAuthority() const
{
	return HasAuthority();
}

void ALLKSweeper::BeginPlay()
{
	Super::BeginPlay();

	// Dynamic instances belong to a live actor, not the class default object.
	// The static material is already assigned in the constructor so this is
	// harmless if a clean checkout lacks the optional presentation asset.
	if (BeaconHousing != nullptr)
	{
		StateLensMaterial = BeaconHousing->CreateAndSetMaterialInstanceDynamic(0);
	}

	// The motor runs for the whole of the machine's life, from silence. It is
	// started here rather than auto-activated so a world without the asset,
	// or a headless one, is exactly as quiet as it was before this existed.
	if (Presence != nullptr && PresenceSound != nullptr)
	{
		Presence->SetSound(PresenceSound);
		Presence->SetVolumeMultiplier(0.f);
		Presence->Play();
		bVoiceStarted = true;
		// Affirmative, not the absence of a warning. The navmesh taught this
		// project that "nothing complained" is not evidence: a null sound, a
		// component that never played and a working motor all look identical
		// in a log that only speaks up on failure. So this says what loaded
		// and what the player will hear.
		UE_LOG(LogLLK, Display,
			TEXT("%s has a motor: %s, running at %.0f%% volume on patrol and %.0f%% in pursuit, ")
			TEXT("silent while it telegraphs, audible to %.0f cm."),
			*GetName(), *PresenceSound->GetName(),
			LLKSweeperPresence::PatrolVolume * 100.f,
			LLKSweeperPresence::PursueVolume * 100.f,
			PresenceAttenuation != nullptr
				? PresenceAttenuation->Attenuation.AttenuationShapeExtents.X
				: 0.f);
	}
	else if (Presence != nullptr)
	{
		// Named, because a silent Sweeper is the defect this component was
		// added to remove and it would otherwise look identical to a working
		// one in every log.
		UE_LOG(LogLLK, Warning,
			TEXT("%s has no presence loop: /Game/Audio/Sweeper/sweeper-presence-v1 did not load, ")
			TEXT("so this machine is inaudible while it moves. Render it with ")
			TEXT("assets/audio/make_sweeper_presence.py and import it."),
			*GetName());
	}

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

void ALLKSweeper::ConsiderNoise(
	FLLKSweeperSenses& Senses, const LLKHearing::FLLKNoise& Noise)
{
	const FVector Ear = GetActorLocation();
	if (!LLKHearing::NoiseReaching(Noise, Ear))
	{
		return;
	}

	// **Most salient wins, and salience is excess reach** - how far past this
	// machine the sound carried. `LLKPushFeel::Salience` states the argument;
	// the consequence is the one a player needs, which is that a deliberate
	// loud thing beats their own footsteps. Nearest would have made a pusher's
	// own steps outrank the fixture they are pushing, because they are
	// standing next to it, and the distraction would never work.
	const float Excess = LLKPushFeel::Salience(Noise, Ear);
	if (Senses.HeardNoisePosition.IsSet() && Excess <= HeardSalienceStuds)
	{
		return;
	}

	HeardSalienceStuds = Excess;
	Senses.HeardNoisePosition = Noise.Position;
	Senses.DistanceToHeardStuds = LLKHearing::DistanceStuds(Noise, Ear);
}

FLLKSweeperSenses ALLKSweeper::Observe(const float DeltaSeconds)
{
	FLLKSweeperSenses Senses;
	Senses.DeltaSeconds = DeltaSeconds;
	// Reset per frame. A running best that survived the tick would ratchet:
	// the loudest sound ever heard would silence every quieter one forever.
	HeardSalienceStuds = 0.f;
	Senses.bHasPatrolPoint = NextPatrolPoint().IsSet();
	Senses.bAtPatrolPoint = IsAtCurrentPatrolPoint();

	const UWorld* World = GetWorld();
	if (World == nullptr || !bHasDef)
	{
		return Senses;
	}

	// **Before the keeper, because indifference does not depend on one.** The
	// early returns below exist for "there is nobody to chase", and a Sweeper
	// filing stock is doing the job it would be doing in an empty building.
	// Resolving this after them would have made the whole behaviour contingent
	// on a keeper being present and upright, which is the opposite of row 47.
	PendingShelve = nullptr;
	if (const TOptional<FVector> Bay = NearestStockBay())
	{
		Senses.bHasStockBay = true;

		float MisplacedStuds = 0.f;
		if (AActor* Misplaced = FindMisplacedItem(MisplacedStuds))
		{
			Senses.MisplacedItemPosition = Misplaced->GetActorLocation();
			Senses.DistanceToMisplacedStuds = MisplacedStuds;
			PendingShelve = Misplaced;
			PendingBay = *Bay;
		}
	}

	// **Before the keeper, for the same reason the stocking job is.** A
	// fixture grinding across tile in an empty building is a sound, and row
	// 47's machine is indifferent to whether anybody is there to have made
	// it. Resolving this after the keeper early-returns would have made the
	// whole sense contingent on a keeper being present and upright.
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		const ILLKNoiseSource* Source = Cast<ILLKNoiseSource>(*It);
		LLKHearing::FLLKNoise SourceNoise;
		if (Source != nullptr && Source->GetCurrentNoise(SourceNoise))
		{
			ConsiderNoise(Senses, SourceNoise);
		}
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

	// The keeper's own noise is NOT computed here any more. `ALLKCharacter`
	// is an `ILLKNoiseSource`, so the loop above already asked it - one code
	// path for "what can I hear", which is what let the climb fall silently
	// through a gap between two of them. See the character's
	// `GetCurrentNoise` for the argument.


	// **Detectability at the keeper, not "is their lantern on".** The light
	// field is the one definition of how marked a spot is, and a keeper
	// standing in a teammate's cone with their own lantern off is marked -
	// which is what makes light a shared cost.
	//
	// **This read brightness and a derived threshold until row 80.** Both are
	// gone, and the reason is worth keeping: the threshold was half the
	// lantern's intensity, a keeper's own lantern is at distance zero, so
	// every lit keeper cleared it by a factor of two no matter how dim or how
	// small their light was. A hooded setting could not have meant anything.
	// Detectability is authored per level, so now it can.
	float Detectability = 0.f;
	if (const ULLKZonePressureSubsystem* Pressure = World->GetSubsystem<ULLKZonePressureSubsystem>())
	{
		Detectability = LLKLightField::DetectabilityAt(
			Pressure->GetLitLightSources(), Senses.KeeperPosition);
	}

	// **While the floor is alerted, every Sweeper notices from further.**
	// Chris's authored `swarmCallRadius` of 45, against 12 dark and 38 lit -
	// and it applies to machines that never touched the keeper, because the
	// alert is a record the building keeps rather than a grudge one machine
	// holds. `max`, never a replacement: a lit keeper at 38 does not become
	// harder to notice because the floor got more attentive.
	FLLKEnemyDef Alerted = Def;
	if (const ULLKRunStateSubsystem* Run = World->GetSubsystem<ULLKRunStateSubsystem>())
	{
		if (Run->IsFloorAlerted() && Def.SwarmCallRadius.IsSet())
		{
			const float Call = *Def.SwarmCallRadius;
			Alerted.AggroRadiusDark = FMath::Max(Def.AggroRadiusDark.Get(0.f), Call);
			Alerted.AggroRadiusLit = FMath::Max(Def.AggroRadiusLit.Get(0.f), Call);
		}
	}

	FLLKPerceiver Perceiver;
	Perceiver.Position = GetActorLocation();
	Perceiver.Facing = GetActorForwardVector();

	// **The Sweeper looks out of its head, not its waist.**
	//
	// `GetActorLocation()` is the capsule centre, and this class takes
	// `ACharacter`'s default capsule deliberately - see the constructor, "the
	// map owns the Sweeper's physical scale". That puts the centre 88 cm up on
	// a machine 176 cm tall, and until 2026-11-23 the sight trace started
	// there. It did not show, because Boswell's ground floor had thirty-six
	// objects on it that blocked `ECC_Visibility`; the clothing the player
	// hides behind was visual-only. Once that dressing became cover the floor
	// had 1,114, and `Tools/audit_boswells_sweeper_sight.py` measured what the
	// waist was costing: from two of the three patrol points the Sweeper could
	// see **nothing at all**, against 45% of what was in range from a proper
	// eye.
	//
	// The inset is 8 cm below the crown rather than at it, because a sensor
	// sits on a machine and not floating at its exact apex - and because a
	// trace starting on the top plane of its own capsule is a trace that can
	// graze geometry the machine is standing against.
	//
	// Read from the capsule rather than typed, so a Sweeper scaled by a level
	// author or a future authored body keeps looking out of its own head.
	if (const UCapsuleComponent* Body = GetCapsuleComponent())
	{
		constexpr float EyeInsetCm = 8.f;
		const float Rise = FMath::Max(0.f, Body->GetScaledCapsuleHalfHeight() - EyeInsetCm);
		Perceiver.EyePosition = Perceiver.Position + FVector(0.f, 0.f, Rise);
	}

	FLLKPerceivedKeeper Perceived;
	Perceived.Position = Senses.KeeperPosition;
	Perceived.Detectability = Detectability;

	// The trace, through the seam M2.5 built for exactly this: perception asks
	// a visibility source and never learns that a `UWorld` exists. `Unknown`
	// from a failed trace never notices, which is why a Sweeper cannot hunt
	// through geometry nobody could measure.
	const FLLKWorldVisibility Visibility(World, this);

	Senses.Notice = LLKPerception::Notice(Alerted, Perceiver, Perceived, &Visibility);

	return Senses;
}

FVector ALLKSweeper::RestingPlaceAt(const FVector& Bay, const AActor* Thing) const
{
	// **Why this is not just `Bay`.** The filed crate used to be placed at the
	// tagged actor's exact origin, which worked because somebody computed the
	// bay's Z by hand so a 100 cm relic with a centred origin would land its
	// base on the floor. That is correct until the floor moves - and on
	// 2026-11-02 the greybox was rebuilt under a bay that stayed exactly where
	// it was. `SetActorLocation` does not sweep, so a crate would be teleported
	// into a gondola without complaint.
	//
	// A filed thing that is inside a shelf is not recoverable, and
	// `WORLD-SYSTEMS-BOARD.md` is explicit that a shelved thing must stay in
	// the world with a recoverable destination. So: keep the bay's XY, which
	// is the authored decision, and find the floor for the Z, which is a fact
	// about geometry that nobody should be maintaining by hand.
	const UWorld* World = GetWorld();
	if (World == nullptr || Thing == nullptr)
	{
		return Bay;
	}

	FVector Origin = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	Thing->GetActorBounds(true, Origin, Extent);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(LLKStockBayDrop), false, this);
	Params.AddIgnoredActor(Thing);

	const FVector From(Bay.X, Bay.Y, Bay.Z + DropProbeHeightCm);
	const FVector To(Bay.X, Bay.Y, Bay.Z - DropProbeHeightCm);

	if (!World->LineTraceSingleByChannel(Hit, From, To, ECC_WorldStatic, Params))
	{
		// No floor found under the bay at all. Falling back to the authored
		// point is strictly no worse than before this function existed, and a
		// warning is better than a crate quietly inside the scenery.
		UE_LOG(LogLLK, Warning,
			TEXT("%s found no floor under the stock bay at %s. Filing to the authored ")
			TEXT("point, which may be inside geometry - see docs/MAP-READINESS.md."),
			*GetName(), *Bay.ToCompactString());
		return Bay;
	}

	// Centre-origin actors need half their height above the floor to rest on
	// it rather than in it.
	return FVector(Bay.X, Bay.Y, Hit.ImpactPoint.Z + Extent.Z);
}

bool ALLKSweeper::IsAtCurrentPatrolPoint() const
{
	const TOptional<FVector> Point = NextPatrolPoint();
	if (!Point.IsSet())
	{
		// No route is not the same as standing on one. A map with no patrol
		// points must never report arrival, or a Sweeper would dwell forever
		// on a point that does not exist.
		return false;
	}

	// 2D on purpose, like the original check: a Sweeper standing under a
	// marker somebody floated a metre off the floor has still arrived.
	const float ToleranceCm = LLKUnits::DistanceFromStuds(ArriveToleranceStuds);
	return FVector::DistSquared2D(GetActorLocation(), *Point) <= ToleranceCm * ToleranceCm;
}

TOptional<FVector> ALLKSweeper::NearestStockBay() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return TOptional<FVector>();
	}

	TOptional<FVector> Nearest;
	float NearestSquared = TNumericLimits<float>::Max();
	for (TActorIterator<AActor> It(const_cast<UWorld*>(World)); It; ++It)
	{
		if (!It->ActorHasTag(StockBayTag))
		{
			continue;
		}

		const float DistanceSquared =
			FVector::DistSquared(It->GetActorLocation(), GetActorLocation());
		if (DistanceSquared < NearestSquared)
		{
			NearestSquared = DistanceSquared;
			Nearest = It->GetActorLocation();
		}
	}

	return Nearest;
}

AActor* ALLKSweeper::FindMisplacedItem(float& OutDistanceStuds) const
{
	OutDistanceStuds = 0.f;

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	AActor* Nearest = nullptr;
	float NearestSquared = TNumericLimits<float>::Max();
	for (TActorIterator<AActor> It(const_cast<UWorld*>(World)); It; ++It)
	{
		const ULLKRelicComponent* Relic = It->FindComponentByClass<ULLKRelicComponent>();
		if (Relic == nullptr || Relic->IsTaken())
		{
			continue;
		}

		// Already filed by this Sweeper. Not by any Sweeper: a second one
		// finding the same relic in its own lane is a legitimate second work
		// order, and the keeper having to watch two machines pass it along is
		// better horror than a relic becoming immune.
		if (ShelvedAlready.Contains(FObjectKey(*It)))
		{
			continue;
		}

		const float DistanceSquared =
			FVector::DistSquared(It->GetActorLocation(), GetActorLocation());
		if (DistanceSquared < NearestSquared)
		{
			NearestSquared = DistanceSquared;
			Nearest = *It;
		}
	}

	if (Nearest != nullptr)
	{
		OutDistanceStuds = FMath::Sqrt(NearestSquared) / LLKUnits::StudsToCm;
	}

	return Nearest;
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
		MulticastPlayCue(ELLKSweeperCue::Telegraph, GetActorLocation());
	}

	if (Step.bLostTheKeeper)
	{
		UE_LOG(LogLLK, Log, TEXT("%s lost the keeper and went back to its route."), *GetName());
	}

	if (Step.bDwellStarted)
	{
		UE_LOG(LogLLK, Verbose, TEXT("%s reached a patrol point and stopped."), *GetName());
	}

	if (Step.bAdvancePatrolPoint)
	{
		// The brain says the pause is spent. This is the whole of the pawn's
		// involvement in the route now: take the next one.
		PatrolIndex = PatrolIndex == INDEX_NONE ? 1 : PatrolIndex + 1;
	}

	if (Step.bStockStarted)
	{
		UE_LOG(LogLLK, Log, TEXT("%s began filing misplaced stock."), *GetName());
		OnStockStarted.Broadcast(GetActorLocation());
		MulticastPlayCue(ELLKSweeperCue::StockStarted, GetActorLocation());
	}

	if (Step.bStockInterrupted)
	{
		UE_LOG(LogLLK, Log, TEXT("%s noticed a keeper and abandoned the filing."), *GetName());
		OnStockInterrupted.Broadcast(GetActorLocation());
		MulticastPlayCue(ELLKSweeperCue::StockInterrupted, GetActorLocation());
	}

	if (Step.bStockAbandoned)
	{
		// Not a cue - a map problem. An unreachable relic should be findable
		// as a warning rather than as a Sweeper that quietly stopped filing.
		UE_LOG(LogLLK, Warning,
			TEXT("%s could not reach the misplaced stock it noticed and gave up. ")
			TEXT("Check the navmesh between it and the relic - see docs/MAP-READINESS.md."),
			*GetName());
	}

	if (Step.bItemShelved)
	{
		// **Relocated, never destroyed.** `TakeByServer` is the player's verb;
		// calling it here would remove the relic from the world and delete the
		// run's objective. The thing stays in the world, at an authored place,
		// and the cost to the player is the route and the time to go get it.
		if (AActor* Shelved = PendingShelve.Get())
		{
			ShelvedAlready.Add(FObjectKey(Shelved));
			Shelved->SetActorLocation(
				RestingPlaceAt(PendingBay, Shelved), false, nullptr, ETeleportType::TeleportPhysics);

			UE_LOG(LogLLK, Log, TEXT("%s filed %s into the stock bay. It is still there to be found."),
				*GetName(), *Shelved->GetName());

			// At the bay, not at the Sweeper: the cue should point a player at
			// where their crate went.
			OnStockFiled.Broadcast(PendingBay);
			MulticastPlayCue(ELLKSweeperCue::StockFiled, PendingBay);
		}

		PendingShelve = nullptr;
	}

	if (Step.bFloorAlerted)
	{
		// **It reports you; it does not punish you.** Row 47's answer to
		// being bumped three times.
		//
		// The duration is `losesTargetAfterSeconds` rather than a new number:
		// it already means "how long before it gives up on something it
		// cannot resolve", which is the same shape as a floor forgetting a
		// fault. Chris's call was that it decays quickly, with no clearing
		// action in the first version - see
		// docs/PROPOSAL-SWEEPER-CONTACT.md.
		if (UWorld* World = GetWorld())
		{
			if (ULLKRunStateSubsystem* Run = World->GetSubsystem<ULLKRunStateSubsystem>())
			{
				Run->RaiseFloorAlert(
					Def.LosesTargetAfterSeconds.Get(FallbackFloorAlertSeconds));
			}
		}
	}

	if (Step.bSlowLanded && Target != nullptr)
	{
		if (ALLKCharacter* Keeper = Cast<ALLKCharacter>(Target->GetOwner()))
		{
			Keeper->ApplyStumble();
		}

		UE_LOG(LogLLK, Log, TEXT("%s clipped a keeper's feet: slow."), *GetName());
	}

	if (Step.bStumbleLanded && Target != nullptr)
	{
		// Wired 2026-11-02. This was a log line for long enough that
		// `tests/cues.test.mjs` had to name it as the class of defect its own
		// census cannot see: produced by the brain, asserted twice in the
		// brain's test, read once by a UE_LOG, and never felt by anybody.
		//
		// No impulse. The movement component's own push may already supply
		// the shove, and whether there should be a scripted one on top is
		// Chris's call and checkable in play first. The speed cost is the
		// part the rule names.
		if (ALLKCharacter* Keeper = Cast<ALLKCharacter>(Target->GetOwner()))
		{
			Keeper->ApplyStumble();

			// A stumble costs distance and tempo; it never removes input.  The
			// controller's client RPC plays only for the keeper who was clipped,
			// including a listen-server host, so another player cannot have their
			// view shaken by somebody else's contact.
			if (APlayerController* PlayerController = Cast<APlayerController>(Keeper->GetController()))
			{
				PlayerController->ClientStartCameraShake(ULLKStumbleCameraShake::StaticClass());
			}
		}

		UE_LOG(LogLLK, Log, TEXT("%s clipped a keeper's feet."), *GetName());
	}

	if (Step.bFallLanded && Target != nullptr)
	{
		if (ALLKCharacter* Keeper = Cast<ALLKCharacter>(Target->GetOwner()))
		{
			Keeper->ApplyFall();
			if (APlayerController* PlayerController = Cast<APlayerController>(Keeper->GetController()))
			{
				PlayerController->ClientStartCameraShake(ULLKStumbleCameraShake::StaticClass());
			}
		}

		UE_LOG(LogLLK, Log, TEXT("%s knocked a keeper down briefly, then disengaged."), *GetName());
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

void ALLKSweeper::MulticastPlayCue_Implementation(
	const ELLKSweeperCue Cue, const FVector_NetQuantize CueLocation)
{
	if (TemporaryCueSound == nullptr || CueAttenuation == nullptr)
	{
		return;
	}

	// The state machine owns when a sound happens.  These are spatial authored
	// sounds that tell a player what committed, interrupted, or completed.
	// The ping remains a deliberately plain fallback for telegraphing and for a
	// missing imported asset; it must not make a missing content reference mute
	// an otherwise readable encounter.
	USoundBase* CueSound = TemporaryCueSound;
	float Volume = 0.15f;
	float Pitch = 0.40f;
	switch (Cue)
	{
	case ELLKSweeperCue::Telegraph:
		Volume = 0.32f;
		Pitch = 0.36f;
		break;
	case ELLKSweeperCue::StockStarted:
		CueSound = StockStartedCueSound != nullptr ? StockStartedCueSound : TemporaryCueSound;
		Volume = 0.16f;
		Pitch = 1.f;
		break;
	case ELLKSweeperCue::StockInterrupted:
		CueSound = StockInterruptedCueSound != nullptr ? StockInterruptedCueSound : TemporaryCueSound;
		Volume = 0.25f;
		Pitch = 1.f;
		break;
	case ELLKSweeperCue::StockFiled:
		CueSound = StockFiledCueSound != nullptr ? StockFiledCueSound : TemporaryCueSound;
		Volume = 0.20f;
		Pitch = 1.f;
		break;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this, CueSound, CueLocation, GetActorRotation(), Volume, Pitch,
		0.f, CueAttenuation, nullptr, this);
}

void ALLKSweeper::UpdatePresentation()
{
	if (BeaconHousing == nullptr)
	{
		return;
	}

	const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 12.f);
	float CoreEmissive = 0.5f;
	float Scale = 1.f;
	switch (Mind.State)
	{
	case ELLKSweeperState::Telegraph:
	case ELLKSweeperState::Pursue:
		CoreEmissive = 0.7f;
		Scale = 1.f + Pulse * 0.25f;
		break;

	case ELLKSweeperState::Stock:
		CoreEmissive = 0.6f;
		Scale = 1.f + Pulse * 0.12f;
		break;

	default:
		break;
	}

	// The lab measured visible Lumen bounce only from about 5 upward. Keeping
	// this lens under one makes it a readable machine cue, never free aisle
	// illumination or a competing relic-reading source.
	if (StateLensMaterial != nullptr)
	{
		StateLensMaterial->SetScalarParameterValue(TEXT("CoreEmissive"), CoreEmissive);
	}
	BeaconHousing->SetRelativeScale3D(FVector(0.12f * Scale, 0.12f * Scale, 0.08f));
}

void ALLKSweeper::UpdateVoice(const float DeltaSeconds)
{
	if (Presence == nullptr || !bVoiceStarted)
	{
		return;
	}

	// Not `Target`: that name is the keeper this Sweeper is hunting.
	const LLKSweeperPresence::FLLKVoice Wanted = LLKSweeperPresence::VoiceFor(Mind.State);

	// The telegraph's silence has to be immediate. Gliding into it would play
	// the announcement over a decaying motor, which is the one thing
	// LLKSweeperPresence.h says this must not do.
	const bool bImmediate = Mind.State == ELLKSweeperState::Telegraph
		&& VoicedState != ELLKSweeperState::Telegraph;

	Voice = LLKSweeperPresence::GlideToward(Voice, Wanted, DeltaSeconds, bImmediate);
	VoicedState = Mind.State;

	Presence->SetVolumeMultiplier(Voice.Volume);
	Presence->SetPitchMultiplier(Voice.Pitch);
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

	// Logged here rather than in `Act`, which is given the decision and not
	// the senses: the distance is the whole value of the line. Without it a
	// playtest can see that something made the machine leave its route and
	// cannot tell whether the player caused it from across the store or from
	// the next aisle.
	if (Step.bNoiseInvestigated)
	{
		UE_LOG(LogLLK, Log,
			TEXT("%s HEARD something %.1f studs away and left its route to look. ")
			TEXT("It has not seen anybody."),
			*GetName(), Senses.DistanceToHeardStuds);
	}

	Mind = Step.Mind;
	Act(Step);
	UpdatePresentation();
	UpdateVoice(DeltaSeconds);

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

	case ELLKSweeperMove::MisplacedItem:
		// Resolved in Observe this same tick, so it cannot be stale. If the
		// sense went away the brain has already left Stock and this arm is
		// unreachable.
		if (Senses.MisplacedItemPosition.IsSet())
		{
			CurrentDestination = *Senses.MisplacedItemPosition;
			bHasDestination = true;
			StepToward(CurrentDestination, DeltaSeconds);
		}
		else
		{
			bHasDestination = false;
		}
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

		// **Arrival is now measured, not decided.** This block used to
		// advance `PatrolIndex` itself the moment it got within tolerance,
		// and the comment it carried admitted the shape of the problem: "the
		// only thing this class decides about a route is 'that one is
		// done'." That one decision was enough to make a dwell impossible —
		// the route moved on before the brain was ever consulted, so a
		// Sweeper could never pause anywhere.
		//
		// The measurement stays here, because tolerance is a fact about
		// geometry. The decision moved to the brain, which answers it with
		// `bAdvancePatrolPoint` once the dwell is spent.
		if (!IsAtCurrentPatrolPoint())
		{
			StepToward(CurrentDestination, DeltaSeconds);
		}
		break;
	}
	}
}
