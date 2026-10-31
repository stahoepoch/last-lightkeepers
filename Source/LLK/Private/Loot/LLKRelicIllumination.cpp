#include "Loot/LLKRelicIllumination.h"

float LLKRelicIllumination::BrightnessAtVisible(
	const TArray<FLLKLightSource>& Sources,
	const FVector& RelicPosition,
	const ILLKVisibilitySource& Visibility)
{
	// Keep the default allocator because LLKLightField deliberately accepts
	// the project-standard TArray signature. The roster is tiny, but avoiding
	// an allocator-specific overload is more valuable than a micro-optimisation
	// in a 10 Hz relic check.
	TArray<FLLKLightSource> VisibleSources;
	VisibleSources.Reserve(Sources.Num());

	for (const FLLKLightSource& Source : Sources)
	{
		// A source and relic at exactly the same point are unobstructed by
		// definition. More importantly, asking a physics trace that question
		// lets the relic's own collision answer its own light and makes the
		// result depend on collision setup rather than on the rule.
		const ELLKVisibility Sightline = FVector::DistSquared(Source.Position, RelicPosition)
			<= KINDA_SMALL_NUMBER
			? ELLKVisibility::Clear
			: Visibility.VisibilityBetween(Source.Position, RelicPosition);

		if (Sightline == ELLKVisibility::Clear)
		{
			VisibleSources.Add(Source);
		}
	}

	return LLKLightField::BrightnessAt(VisibleSources, RelicPosition);
}
