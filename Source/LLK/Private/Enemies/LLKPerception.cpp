#include "Enemies/LLKPerception.h"

#include "Data/LLKEnemyDef.h"
#include "LLKUnits.h"
#include "Storm/LLKLightField.h"

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
		/**
		 * A reach, or zero when the value could not have been authored.
		 *
		 * **This exists because the guard used to live in one place only.**
		 * `WithinStuds` refused a non-positive radius, so `SenseReaching`
		 * correctly noticed nobody with a negative one - but
		 * `NoticeRangeStuds` returned the raw number, and handed callers
		 * -38 studs to size a sightline against. `LLK.Enemies.
		 * PerceptionFailures` caught it on its first run.
		 *
		 * One helper, used by both, so the two answers cannot disagree about
		 * what counts as a usable radius again. Zero and negative are the
		 * same thing here: a value no designer wrote, from a row that did not
		 * import, and `tests/enemies.test.mjs` forbids both in the authored
		 * file so this is a guard rather than a behaviour.
		 */
		float ReachOrZero(float RadiusStuds)
		{
			return RadiusStuds > 0.f ? RadiusStuds : 0.f;
		}

		bool WithinStuds(const FVector& From, const FVector& To, float RadiusStuds)
		{
			if (ReachOrZero(RadiusStuds) <= 0.f)
			{
				return false;
			}

			const float RadiusCm = LLKUnits::DistanceFromStuds(RadiusStuds);
			return FVector::DistSquared(From, To) <= RadiusCm * RadiusCm;
		}
	}

	float AggroRadiusAt(float DarkStuds, float LitStuds, float Detectability)
	{
		// Both ends through ReachOrZero, so a family that lost one radius to a
		// failed import cannot have the other one silently stand in for it.
		// Zero at either end still produces a usable number here, and
		// SenseReaching's own guard is what refuses to act on it.
		const float Dark = ReachOrZero(DarkStuds);
		const float Lit = ReachOrZero(LitStuds);
		return FMath::Lerp(Dark, Lit, FMath::Clamp(Detectability, 0.f, 1.f));
	}

	ELLKNoticeSense SenseReaching(
		const FLLKEnemyDef& Enemy,
		const FLLKPerceiver& Perceiver,
		const FLLKPerceivedKeeper& Keeper)
	{
		// Marked by the FIELD, not by the keeper's own switch. The header says
		// why at length; the short version is that a dark keeper standing in
		// a teammate's cone is lit, and anything else makes light stop being
		// a shared cost.
		//
		// `bLit` is now a floor rather than a threshold, and it is only read
		// by the senses that gate on "is a lantern burning at all"
		// (`scanRequiresActiveLantern`, `wakeRequiresActiveLantern`). The
		// Sweeper below does not use it: its reach is continuous in
		// detectability, which is the whole of row 80.
		const float Detectability = FMath::Clamp(Keeper.Detectability, 0.f, 1.f);
		const bool bLit = Detectability >= LLKLightField::DetectabilityFloor;

		// ---- Sweepers: proximity, with light deciding how far -----------
		//
		// Both radii required. A family with only one of them is a data bug
		// rather than a half-working sense, and defaulting the missing one
		// would invent the single number this game teaches hardest.
		if (Enemy.AggroRadiusDark.IsSet() && Enemy.AggroRadiusLit.IsSet())
		{
			// **Lerped, not chosen.** The two authored radii are the ends of a
			// range and detectability is the position on it, so a hooded
			// lantern is noticed from between the two. At 0 and 1 this returns
			// exactly the two values the switch used to pick, which is why
			// `data/enemies.json` needed no change for any of this.
			const float Radius = AggroRadiusAt(
				Enemy.AggroRadiusDark.GetValue(), Enemy.AggroRadiusLit.GetValue(), Detectability);
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
		// Row 7 and row 54: it converges on the brightest position when its
		// own meter fills, and giving it a radius here would be the first
		// step toward making it a boss.
		//
		// **Both conditions, and the second one is a correction.** The first
		// version of this returned `StormAuthority` for any family with no
		// reaching sense, on the reasoning that the rule should describe the
		// data's contract rather than name an enum value. That was wrong in a
		// way only a malformed row reveals: a Sweeper that lost one of its
		// two radii also has no reaching sense, and it would have reported
		// `StormAuthority` - which reads as "this thing converges on the
		// brightest keeper regardless of distance", the most dangerous answer
		// available. Broken data must degrade to noticing NOTHING, never to
		// behaving like the one enemy that cannot be escaped.
		//
		// So the Warden is named. It is the one family whose senselessness is
		// authored rather than accidental, and `tests/enemies.test.mjs`
		// asserts that it stays that way.
		const bool bHasAnySense =
			(Enemy.AggroRadiusDark.IsSet() && Enemy.AggroRadiusLit.IsSet())
			|| (Enemy.SightRange.IsSet() && Enemy.SightConeDegrees.IsSet())
			|| Enemy.ScanRange.IsSet()
			|| Enemy.WakeRange.IsSet();

		if (!bHasAnySense && Enemy.Family == ELLKEnemyFamily::Warden)
		{
			return ELLKNoticeSense::StormAuthority;
		}

		// Everything else with nothing to notice with notices nothing. A
		// family whose senses failed to import is a family that does not act.
		return ELLKNoticeSense::None;
	}

	FLLKNotice Notice(
		const FLLKEnemyDef& Enemy,
		const FLLKPerceiver& Perceiver,
		const FLLKPerceivedKeeper& Keeper,
		const ILLKVisibilitySource* Visibility)
	{
		FLLKNotice Result;
		Result.Sense = SenseReaching(Enemy, Perceiver, Keeper);

		// Nothing reached, so there is nothing for a wall to interrupt. Not
		// asking saves a trace per enemy per keeper per tick for the common
		// case, which is the case that runs most.
		if (Result.Sense == ELLKNoticeSense::None)
		{
			return Result;
		}

		// The Warden. Row 7 and row 54: its arrival is storm-driven, so no
		// geometry is consulted and none can stop it. Visibility stays
		// Unknown because nobody asked, which is the truth.
		if (Result.Sense == ELLKNoticeSense::StormAuthority)
		{
			Result.bNoticed = true;
			return Result;
		}

		if (Visibility == nullptr)
		{
			// An error rather than a default, and loud rather than silent.
			// The alternative - assuming a clear line - is an enemy that
			// hunts through walls in exactly the build where somebody forgot
			// to wire the source up.
			UE_LOG(LogTemp, Error,
				TEXT("LLKPerception::Notice called with no visibility source. ")
				TEXT("Noticing nothing. See docs/MAP-READINESS.md."));
			return Result;
		}

		Result.Visibility = Visibility->VisibilityBetween(Perceiver.Position, Keeper.Position);

		// Clear and only Clear. Unknown does not notice - see FLLKNotice::
		// IsUnverified for why it is not quietly folded into Blocked.
		Result.bNoticed = Result.Visibility == ELLKVisibility::Clear;
		return Result;
	}

	float NoticeRangeStuds(const FLLKEnemyDef& Enemy, float Detectability)
	{
		const float Marked = FMath::Clamp(Detectability, 0.f, 1.f);
		const bool bLit = Marked >= LLKLightField::DetectabilityFloor;

		// Every return goes through ReachOrZero, so a radius this function
		// reports is always one SenseReaching would actually act on - and for
		// the Sweeper it is the same lerp SenseReaching uses, not a restatement
		// of it, so a debug readout cannot disagree with the behaviour.
		if (Enemy.AggroRadiusDark.IsSet() && Enemy.AggroRadiusLit.IsSet())
		{
			return AggroRadiusAt(
				Enemy.AggroRadiusDark.GetValue(), Enemy.AggroRadiusLit.GetValue(), Marked);
		}
		if (Enemy.SightRange.IsSet() && Enemy.SightConeDegrees.IsSet())
		{
			return ReachOrZero(Enemy.SightRange.GetValue());
		}
		if (Enemy.ScanRange.IsSet())
		{
			return Enemy.bScanRequiresActiveLantern.Get(true) && !bLit
				? 0.f : ReachOrZero(Enemy.ScanRange.GetValue());
		}
		if (Enemy.WakeRange.IsSet())
		{
			return Enemy.bWakeRequiresActiveLantern.Get(true) && !bLit
				? 0.f : ReachOrZero(Enemy.WakeRange.GetValue());
		}

		// The Warden, or a family whose senses are not authored. Zero rather
		// than a guess: a caller sizing a sightline against this must not be
		// handed a plausible number nobody wrote down.
		return 0.f;
	}
}
