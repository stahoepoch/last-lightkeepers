#include "Storm/LLKLightField.h"

namespace LLKLightField
{
	float BrightnessAt(const TArray<FLLKLightSource>& Sources, const FVector& Point)
	{
		float Total = 0.f;
		for (const FLLKLightSource& Source : Sources)
		{
			if (Source.Intensity <= 0.f || Source.RadiusCm <= 0.f)
			{
				// A lantern with no reach or no brightness is not a light.
				// Skipped rather than divided by.
				continue;
			}

			const float DistanceSquared = FVector::DistSquared(Source.Position, Point);
			const float RadiusSquared = Source.RadiusCm * Source.RadiusCm;
			Total += Source.Intensity / (1.f + DistanceSquared / RadiusSquared);
		}
		return Total;
	}

	bool FindBrightestPosition(
		const TArray<FLLKLightSource>& Sources, FVector& OutPosition, float& OutBrightness)
	{
		bool bFound = false;
		float Best = 0.f;
		FVector BestPosition = FVector::ZeroVector;

		for (const FLLKLightSource& Candidate : Sources)
		{
			if (Candidate.Intensity <= 0.f || Candidate.RadiusCm <= 0.f)
			{
				continue;
			}

			// Evaluated against every source including itself, which is what
			// makes a cluster win: a keeper standing with two others is lit by
			// three lanterns, and a keeper alone is lit by one.
			const float Brightness = BrightnessAt(Sources, Candidate.Position);

			// Strictly greater, so an exact tie keeps the earlier index. Two
			// keepers in precisely the same spot is rare but a server and a
			// client must still name the same one.
			if (!bFound || Brightness > Best)
			{
				bFound = true;
				Best = Brightness;
				BestPosition = Candidate.Position;
			}
		}

		if (!bFound)
		{
			// Out parameters left alone. Nothing is lit, and writing a
			// plausible zero here is how (0,0,0) becomes somewhere an enemy
			// walks to all night.
			return false;
		}

		OutPosition = BestPosition;
		OutBrightness = Best;
		return true;
	}
}
