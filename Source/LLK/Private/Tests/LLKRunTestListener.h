// An event counter for LLKCrewWipeTest.
//
// **This exists because Codex read the test and found it was not testing what
// it claimed.** The "fires once" section asserted that the wipe FLAG stays
// true across repeated evaluation, and then counted the flag rather than the
// delegate. A flag that cannot un-set and an event that fires once are
// different properties, and the one that matters is the second: row 8's haul
// resolution and the end screen both hang off the event, and each firing
// twice is a different bug.
//
// A UCLASS in its own header for the same reason the lantern's and the
// storm's listeners are: `OnCrewWiped` is BlueprintAssignable, deliberately,
// because the end screen is `Content/` and Chris's. A dynamic delegate has no
// `AddLambda` - which is exactly why the first version of that test reached
// for the flag instead.

#pragma once

#include "CoreMinimal.h"
#include "LLKRunTestListener.generated.h"

UCLASS()
class ULLKRunTestListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnWiped()
	{
		++Count;
	}

	int32 Count = 0;
};
