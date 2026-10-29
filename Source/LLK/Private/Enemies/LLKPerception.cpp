#include "Enemies/LLKPerception.h"

#include "Data/LLKEnemyDef.h"
#include "LLKUnits.h"

namespace LLKPerception
{
	namespace
	{
		/**
		 * True when `Keeper` is inside a stud radius of `Perceiver`.
		 *
		 * **Row 61's boundary, and this is the file where it bites.** Every
		 * radius in `data/enemies.json` is in studs; every `FVector` in a
		 * running world is in centimetres. Comparing them directly would make
		 * every enemy notice from a twenty-eighth of its intended range,
		 * which reads as enemies that do not work rather than as a unit bug.
		 * So the conversion happens here, once, through `LLKUnits`.
		 *
		 * Squared distance, so no square root runs per enemy per keeper per
		 * tick to answer a question that does not need the actual distance.
		 */
		bool WithinStuds(const FVector& From, const FVector& To, float RadiusStuds)
		{
			if (RadiusStuds <= 0.f)
			{
				// A radius nobody authored, or one authored as zero. Either
				// way it reaches nothing - and a test asserts that no family
				// in the data authors a zero for a sense it has, so this is a
				// guard rather than a behaviour.
				return false;
			}

			const float RadiusCm = LLKUnits::DistanceFromStuds(RadiusStuds);
			return FVector::DistSquared(From, To) <= RadiusCm * RadiusCm;
		}
	}

	float LitThresholdFrom(float LanternIntensity)
	{
		// Half the lantern's intensity: one lantern at the edge of its own
		// reach, since BrightnessAt falls to I/2 at distance r. The same
		// derivation ULLKRelicComponent uses, on purpose - see the header.
		return FMath::Max(0.f, LanternIntensity) * 0.5f;
	}

	ELLKNoticeSense SenseReaching(
		const FLLKEnemyDef& Enemy,
		const FLLKPerceiver& Perceiver,
		const FLLKPerceivedKeeper& Keeper,
		float LitThreshold)
	{
		// Lit by the FIELD, not by the keeper's own switch. The header says
		// why at length; the short version is that a dark keeper standing in
		// a teammate's cone is lit, and anything else makes light stop being
		// a shared cost.
		const bool bLit = Keeper.Brightness > LitThreshold;

		// ---- Sweepers: proximity, with light deciding how far -----------
		//
		// Both radii required. A family with only one of them is a data bug
		// rather than a half-working sense, and defaulting the missing one
		// would invent the single number this game teaches hardest.
		if (Enemy.AggroRadiusDark.IsSet() && Enemy.AggroRadiusLit.IsSet())
		{
			const float Radius = bLit
				? Enemy.AggroRadiusLit.GetValue()
				: Enemy.AggroRadiusDark.GetValue();
			if (WithinStuds(Perceiver.Position, Keeper.Position, Radius))
			{
				return ELLKNoticeSense::LitProximity;
			}
		}

		// ---- Stockers: looking, and only where they are facing ----------
		//
		// **The cone is what makes this a sight sense rather than a radius.**
		// Only Stockers author `sightConeDegrees`, and only Stockers'
		// behaviour line says "strong line of sight". WindowWalkers and Ficus
		// carry a `sightRange` with no cone, and that absence is meaningful:
		// their range is for following a target they already woke to, not for
		// spotting one. Treating a cone-less sightRange as a sight sense
		// would make every mannequin in the building an omnidirectional
		// watchman, which is the opposite of what "remain still until a
		// nearby active lantern catches them" describes.
		if (Enemy.SightRange.IsSet() && Enemy.SightConeDegrees.IsSet())
		{
			if (WithinStuds(Perceiver.Position, Keeper.Position, Enemy.SightRange.GetValue()))
			{
				const FVector ToKeeper = Keeper.Position - Perceiver.Position;
				const FVector Facing = Perceiver.Facing.GetSafeNormal();

				// A keeper standing exactly on the enemy has no direction to
				// be in. Noticed rather than missed: at zero distance the
				// cone is not the question.
				if (ToKeeper.IsNearlyZero() || Facing.IsNearlyZero())
				{
					return ELLKNoticeSense::SightCone;
				}

				// The authored angle is the cone's FULL width, so the test is
				// against half of it either side of the facing direction.
				// Getting this wrong doubles or halves every Stocker's vision
				// and would be invisible in a log.
				const float HalfAngleRadians =
					FMath::DegreesToRadians(Enemy.SightConeDegrees.GetValue() * 0.5f);
				const float Dot = FVector::DotProduct(Facing, ToKeeper.GetSafeNormal());
				if (Dot >= FMath::Cos(HalfAngleRadians))
				{
					return ELLKNoticeSense::SightCone;
				}
			}
		}

		// ---- Glimmers: scanning, and only what is lit -------------------
		//
		// `scanRequiresActiveLantern` defaults to TRUE when absent rather
		// than false. Row 5: a dark crew is not scanned. If the flag ever
		// goes missing from the data, the safe failure is a Glimmer that
		// scans less, not one that scans a crew who did everything right.
		if (Enemy.ScanRange.IsSet())
		{
			const bool bNeedsLight = Enemy.bScanRequiresActiveLantern.Get(true);
			if ((bLit || !bNeedsLight)
				&& WithinStuds(Perceiver.Position, Keeper.Position, Enemy.ScanRange.GetValue()))
			{
				return ELLKNoticeSense::Scan;
			}
		}

		// ---- WindowWalkers and Ficus: furniture until the light arrives --
		if (Enemy.WakeRange.IsSet())
		{
			const bool bNeedsLight = Enemy.bWakeRequiresActiveLantern.Get(true);
			if ((bLit || !bNeedsLight)
				&& WithinStuds(Perceiver.Position, Keeper.Position, Enemy.WakeRange.GetValue()))
			{
				return ELLKNoticeSense::Wake;
			}
		}

		// ---- the Warden: not perception --------------------------------
		//
		// Recognised by having no reaching sense at all rather than by
		// `Family == Warden`, so the rule is about the contract the data
		// describes rather than about one enum value somebody could add a
		// radius to. Row 7 and row 54: it converges on the brightest
		// position when its own meter fills, and giving it a radius here
		// would be the first step toward making it a boss.
		const bool bHasAnySense =
			(Enemy.AggroRadiusDark.IsSet() && Enemy.AggroRadiusLit.IsSet())
			|| (Enemy.SightRange.IsSet() && Enemy.SightConeDegrees.IsSet())
			|| Enemy.ScanRange.IsSet()
			|| Enemy.WakeRange.IsSet();

		if (!bHasAnySense)
		{
			return ELLKNoticeSense::StormAuthority;
		}

		return ELLKNoticeSense::None;
	}

	float NoticeRangeStuds(const FLLKEnemyDef& Enemy, float Brightness, float LitThreshold)
	{
		const bool bLit = Brightness > LitThreshold;

		if (Enemy.AggroRadiusDark.IsSet() && Enemy.AggroRadiusLit.IsSet())
		{
			return bLit ? Enemy.AggroRadiusLit.GetValue() : Enemy.AggroRadiusDark.GetValue();
		}
		if (Enemy.SightRange.IsSet() && Enemy.SightConeDegrees.IsSet())
		{
			return Enemy.SightRange.GetValue();
		}
		if (Enemy.ScanRange.IsSet())
		{
			return Enemy.bScanRequiresActiveLantern.Get(true) && !bLit
				? 0.f : Enemy.ScanRange.GetValue();
		}
		if (Enemy.WakeRange.IsSet())
		{
			return Enemy.bWakeRequiresActiveLantern.Get(true) && !bLit
				? 0.f : Enemy.WakeRange.GetValue();
		}

		// The Warden, or a family whose senses are not authored. Zero rather
		// than a guess: a caller sizing a sightline against this must not be
		// handed a plausible number nobody wrote down.
		return 0.f;
	}
}
