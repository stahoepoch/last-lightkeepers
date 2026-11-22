#include "World/LLKPowerSubsystem.h"

#include "Components/LightComponent.h"
#include "Engine/Light.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "LLK.h"

const FName ULLKPowerSubsystem::MainsTag(TEXT("LLK_Mains"));
const FName ULLKPowerSubsystem::EmergencyTag(TEXT("LLK_Emergency"));

void ULLKPowerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	MainsCount = 0;
	EmergencyCount = 0;
	for (TActorIterator<ALight> It(&InWorld); It; ++It)
	{
		const ALight* Light = *It;
		if (Light == nullptr)
		{
			continue;
		}
		if (Light->ActorHasTag(MainsTag))
		{
			++MainsCount;
		}
		else if (Light->ActorHasTag(EmergencyTag))
		{
			++EmergencyCount;
		}
	}

	// Emergency is the state every chapter opens in, and it is asserted here
	// rather than assumed: a map saved with the mains visible would otherwise
	// start lit and nothing would say so.
	bPowerOn = false;
	ApplyToLights();

	// **Affirmative, for the fourth time in three days, and this one earns it
	// most.** A map with no mains lights at all and a map with the power
	// correctly off look exactly the same at boot - black departments either
	// way - so "nothing complained" would be worthless. This says what the
	// building has and which state it is in.
	if (MainsCount > 0)
	{
		UE_LOG(LogLLK, Display,
			TEXT("Boswell's is on emergency power: %d emergency light(s) lit, "
				 "%d mains light(s) dark and waiting for someone to throw the "
				 "breaker. Switching them on is heard %.0f studs away."),
			EmergencyCount, MainsCount, PowerOnNoiseStuds);
	}
	else
	{
		UE_LOG(LogLLK, Warning,
			TEXT("%s has NO mains lighting: no light in it is tagged %s, so the "
				 "power can never be turned on here and restoring it would "
				 "change nothing a player could see. Tag the mains layer."),
			*InWorld.GetMapName(), *MainsTag.ToString());
	}
}

bool ULLKPowerSubsystem::SetPowerOn(const bool bOn)
{
	if (bOn == bPowerOn)
	{
		return false;
	}

	bPowerOn = bOn;
	ApplyToLights();

	UE_LOG(LogLLK, Log,
		TEXT("The mains are %s. %d light(s) moved."),
		bPowerOn ? TEXT("ON - Boswell's is lit") : TEXT("off again"), MainsCount);

	OnPowerChanged.Broadcast(bPowerOn);
	return true;
}

void ULLKPowerSubsystem::ApplyToLights()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<ALight> It(World); It; ++It)
	{
		ALight* Light = *It;
		if (Light == nullptr || !Light->ActorHasTag(MainsTag))
		{
			continue;
		}

		// Visibility rather than destroying or re-spawning: the lights stay
		// where the level author put them, and the only thing that changes is
		// whether they are on - which is what a breaker does.
		if (ULightComponent* Component = Light->GetLightComponent())
		{
			Component->SetVisibility(bPowerOn);
		}
	}
}

// ---------------------------------------------------------------------------
// `LLK.Power 0|1` - flip the mains from the console.
//
// For looking at the two states without having to walk to the breaker, which
// is how both of this chapter's lighting screenshots were taken. It is a
// developer instrument and NOT the mechanic: the breaker is the mechanic, and
// this deliberately does not emit the noise the breaker does, so nobody can
// use it to test the cost and conclude there isn't one.
// ---------------------------------------------------------------------------
static FAutoConsoleCommandWithWorldAndArgs GLLKPowerCommand(
	TEXT("LLK.Power"),
	TEXT("LLK.Power 0|1 - switch Boswell's mains off or on. Developer tool: "
		 "makes no noise, unlike the breaker."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
		[](const TArray<FString>& Args, UWorld* World)
		{
			if (World == nullptr)
			{
				return;
			}

			ULLKPowerSubsystem* Power = World->GetSubsystem<ULLKPowerSubsystem>();
			if (Power == nullptr)
			{
				UE_LOG(LogLLK, Warning, TEXT("LLK.Power: no power subsystem in this world."));
				return;
			}

			if (Args.Num() == 0)
			{
				UE_LOG(LogLLK, Display,
					TEXT("LLK.Power: mains are %s, %d light(s) on the circuit."),
					Power->IsPowerOn() ? TEXT("ON") : TEXT("off"),
					Power->GetMainsLightCount());
				return;
			}

			const bool bOn = Args[0] != TEXT("0");
			const bool bChanged = Power->SetPowerOn(bOn);
			UE_LOG(LogLLK, Display,
				TEXT("LLK.Power: mains now %s%s."),
				Power->IsPowerOn() ? TEXT("ON") : TEXT("off"),
				bChanged ? TEXT("") : TEXT(" (already were)"));
		}));
