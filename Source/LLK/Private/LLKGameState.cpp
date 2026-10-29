#include "LLKGameState.h"

#include "Net/UnrealNetwork.h"

ALLKGameState::ALLKGameState()
{
	// No tick. This class stores a struct the subsystem pushes; giving it a
	// tick would invite it to start deriving storm values, and then two
	// places would know how the night escalates.
	PrimaryActorTick.bCanEverTick = false;
}

void ALLKGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Everyone, unconditionally. Row 5 makes light reveal value and attract
	// danger, which is an argument a crew has to be able to have out loud -
	// and they cannot argue about a meter only one of them can see.
	DOREPLIFETIME(ALLKGameState, StormSnapshot);
}

void ALLKGameState::PublishStormSnapshot(const FLLKStormSnapshot& InSnapshot)
{
	// Authority-checked here rather than trusted to the caller. The subsystem
	// already checks, so this is the second lock on the same door - and it is
	// the door that matters, because this struct is what every client believes
	// about the night.
	if (!HasAuthority())
	{
		return;
	}

	StormSnapshot = InSnapshot;
}
