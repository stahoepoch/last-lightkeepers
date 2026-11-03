// A short, input-safe acknowledgement that the keeper was clipped.
//
// A stumble already costs movement speed.  This class gives that cost a
// readable physical channel without changing controls, possession, or camera
// ownership.  It is native so every client has the same effect and no content
// asset is required for the first playable pass.

#pragma once

#include "CoreMinimal.h"
#include "Shakes/LegacyCameraShake.h"
#include "LLKStumbleCameraShake.generated.h"

UCLASS()
class LLK_API ULLKStumbleCameraShake : public ULegacyCameraShake
{
	GENERATED_BODY()

public:
	ULLKStumbleCameraShake();
};
