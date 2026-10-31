// The light that can actually reach a relic.
//
// `LLKLightField` is deliberately radial: it models how much light a source
// contributes at a point, independent of the visual shape of the lantern.
// A radial field alone is not permission to read through a gondola. This
// helper applies the map's visibility answer source-by-source before adding
// their contributions. It is kept beside relic code rather than folded into
// the global field because enemies and relics ask different questions: an
// enemy decides whether it can see a keeper; a relic decides whether a lamp
// can physically illuminate it.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/LLKVisibility.h"
#include "Storm/LLKLightField.h"

namespace LLKRelicIllumination
{
	/**
	 * Brightness at a relic from sources with a clear line to it.
	 *
	 * Unknown is deliberately dark. Calling an unbuilt map clear would let a
	 * player identify loot through geometry that the game has failed to query;
	 * treating it as blocked keeps the light gate honest until the map can
	 * answer the question.
	 */
	LLK_API float BrightnessAtVisible(
		const TArray<FLLKLightSource>& Sources,
		const FVector& RelicPosition,
		const ILLKVisibilitySource& Visibility);
}
