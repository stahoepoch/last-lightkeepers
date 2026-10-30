#include "Loot/LLKBurden.h"

// Named rather than anonymous: unity builds merge anonymous namespaces across
// files, and `LLKExtraction.cpp` has its own `Finite`. This project has paid
// for that lesson once already, in the M2.5 feel slice.
namespace LLKBurdenDetail
{
	/**
	 * The rail that is not the design.
	 *
	 * A keeper who cannot move is never the right answer to a keeper who is
	 * carrying too much: the authored cap is 0.12, and anything approaching
	 * this number means the data is wrong rather than the player greedy.
	 */
	constexpr float HardestPenalty = 0.9f;

	float Finite(float Value)
	{
		return FMath::IsFinite(Value) ? Value : 0.f;
	}
}

using namespace LLKBurdenDetail;

namespace LLKBurden
{
	float PenaltyFraction(const int32 Points, const FLLKBurdenRules& Rules)
	{
		if (Points <= 0)
		{
			// Empty-handed is full speed, and that is a rule rather than an
			// arithmetic accident: a crew that has banked nothing should move
			// exactly as the authored speeds say, so the numbers in
			// `lantern.json` mean what they say for a keeper on the way in.
			return 0.f;
		}

		const float PerPoint = FMath::Max(0.f, Finite(Rules.SpeedPenaltyPerPoint));
		if (PerPoint <= 0.f)
		{
			// Unauthored, not free-by-mistake: a per-point penalty of zero is
			// a deliberate "burden is off", which is what every run before
			// today was playing.
			return 0.f;
		}

		const float Raw = PerPoint * static_cast<float>(Points);

		// The authored cap, when there is one. A cap of zero means no cap
		// rather than no movement - the opposite reading would make an
		// unfilled field the harshest possible setting.
		const float AuthoredCap = FMath::Max(0.f, Finite(Rules.MaxSpeedPenalty));
		const float Capped = AuthoredCap > 0.f ? FMath::Min(Raw, AuthoredCap) : Raw;

		return FMath::Clamp(Capped, 0.f, HardestPenalty);
	}

	float SpeedMultiplier(const int32 Points, const FLLKBurdenRules& Rules)
	{
		return 1.f - PenaltyFraction(Points, Rules);
	}

	int32 PointsUntilCapped(const FLLKBurdenRules& Rules)
	{
		const float PerPoint = FMath::Max(0.f, Finite(Rules.SpeedPenaltyPerPoint));
		const float Cap = FMath::Max(0.f, Finite(Rules.MaxSpeedPenalty));
		if (PerPoint <= 0.f || Cap <= 0.f)
		{
			return 0;
		}

		// Ceil, because the point that REACHES the cap is the last one that
		// costs anything. At 0.04 and 0.12 that is three, and the fourth
		// relic is the first free one.
		return FMath::CeilToInt(Cap / PerPoint);
	}
}
