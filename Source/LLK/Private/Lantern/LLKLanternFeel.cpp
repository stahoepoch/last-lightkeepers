#include "Lantern/LLKLanternFeel.h"

namespace LLKLanternFeel
{
	// 88cm capsule half-height plus the camera's authored 64cm. See the header
	// for why this is the only declaration of it in the project.
	const float KeeperEyeHeightCm = 152.f;

	namespace
	{
		/** A finite number, or the fallback. Hitches and NaNs arrive eventually. */
		float Finite(float Value, float Fallback = 0.f)
		{
			return FMath::IsFinite(Value) ? Value : Fallback;
		}

		/**
		 * Frame-rate independent exponential approach.
		 *
		 * `PerSecond` is the fraction of the remaining distance closed in one
		 * second, so 0.88 means 88% per second at any frame rate. Written out
		 * rather than lerped by `Delta * Rate`, which is the version that
		 * behaves differently at 30 and 144fps and is the reason feel tuned on
		 * one machine stops feeling the same on another.
		 */
		float ApproachAlpha(float PerSecond, float DeltaSeconds)
		{
			const float Clamped = FMath::Clamp(Finite(PerSecond), 0.f, 1.f);
			if (Clamped <= 0.f || DeltaSeconds <= 0.f)
			{
				return 0.f;
			}
			if (Clamped >= 1.f)
			{
				// "All of it in a second" is still a rate, and at a 2-second
				// frame it must not overshoot into instability.
				return FMath::Min(1.f, DeltaSeconds);
			}
			return 1.f - FMath::Pow(1.f - Clamped, DeltaSeconds);
		}
	}

	FVector GameplayLightPosition(const FVector& CapsuleLocation, float EyeHeightCm)
	{
		// Two inputs, both honest, neither cosmetic. This is the whole of
		// guarantee 3 and it is deliberately this boring: there is no
		// parameter here that a sway could be passed through.
		return CapsuleLocation + FVector(0.f, 0.f, Finite(EyeHeightCm));
	}

	FVector ClampedOffset(
		const FVector& DesiredOffsetCm, float FreeDistanceCm, const FLLKLanternFeelRules& Rules)
	{
		const float Ceiling = FMath::Max(0.f, Finite(Rules.MaxOffsetCm));

		FVector Desired(
			Finite(DesiredOffsetCm.X), Finite(DesiredOffsetCm.Y), Finite(DesiredOffsetCm.Z));

		// The hard rail first, so everything below works on a bounded vector.
		if (Desired.SizeSquared() > Ceiling * Ceiling)
		{
			Desired = Desired.GetSafeNormal() * Ceiling;
		}

		if (FreeDistanceCm < 0.f)
		{
			// Nobody traced. Collapse to the anchor rather than trusting the
			// offset: an unmeasured frame shows the player the truth instead
			// of a guess, which is the same direction ELLKVisibility::Unknown
			// resolves in and for the same reason.
			return FVector::ZeroVector;
		}

		const float Clearance = FMath::Max(0.f, Finite(Rules.WallClearanceCm));
		const float Allowed = FMath::Max(0.f, Finite(FreeDistanceCm) - Clearance);
		const float Length = Desired.Size();

		if (Length <= Allowed || Length <= KINDA_SMALL_NUMBER)
		{
			return Desired;
		}

		// Shortened along its own direction rather than stopped dead, so a
		// cone pressed against a shelf eases back instead of snapping to the
		// anchor. The direction is what the player reads as a held object.
		return Desired.GetSafeNormal() * Allowed;
	}

	FLLKLanternFeelState AdvanceFeel(
		const FLLKLanternFeelState& State,
		const FLLKLanternFeelInput& Input,
		const FLLKLanternFeelRules& Rules)
	{
		FLLKLanternFeelState Next = State;

		const float Delta = FMath::Max(0.f, Finite(Input.DeltaSeconds));
		if (Delta <= 0.f)
		{
			// A zero or negative frame advances nothing. It still runs the
			// clamp below, so a state that arrived out of bounds comes back
			// in bounds rather than being preserved.
			Next.OffsetCm = ClampedOffset(State.OffsetCm, Input.FreeDistanceCm, Rules);
			return Next;
		}

		const float Speed = FMath::Max(0.f, Finite(Input.SpeedCm));

		// ---- the trail: the cone lags the look ---------------------------
		//
		// Computed as a lag ANGLE rather than as a remembered rotation, so
		// nothing here can be mistaken for the look itself. Guarantee 1 is in
		// the signature - Input.Look is const and no rotation is returned for
		// the camera - and this keeps it true in the body as well.
		const float TrailAlpha = ApproachAlpha(Rules.TrailPerSecond, Delta);
		const float MaxTrail = FMath::Max(0.f, Finite(Rules.MaxTrailDegrees));

		const FRotator LookDelta = (Input.Look - State.TrailDegrees).GetNormalized();
		FRotator Trail = State.TrailDegrees + LookDelta * TrailAlpha;

		// Expressed and clamped relative to the live look, so the cone can
		// never be further behind than MaxTrailDegrees however long the frame.
		FRotator Lag = (Input.Look - Trail).GetNormalized();
		Lag.Pitch = FMath::Clamp(Lag.Pitch, -MaxTrail, MaxTrail);
		Lag.Yaw = FMath::Clamp(Lag.Yaw, -MaxTrail, MaxTrail);
		Lag.Roll = FMath::Clamp(Lag.Roll, -MaxTrail, MaxTrail);
		Next.TrailDegrees = (Input.Look - Lag).GetNormalized();

		// ---- the sway: evidence of a hand --------------------------------
		//
		// Phase advances with distance travelled rather than with time, so a
		// standing keeper's lantern is still and a walking keeper's keeps step
		// with their feet. A time-driven sway on a stationary player is the
		// thing that reads as a wobbling camera rather than as a body.
		const float Hertz = FMath::Max(0.1f, Finite(Rules.SwayHertz, 1.6f));
		const float WalkSpeedCm = 448.f; // 16 studs/s, data/lantern.json movement.walkSpeed
		const float SpeedFraction = FMath::Clamp(Speed / WalkSpeedCm, 0.f, 2.f);
		Next.SwayPhase = FMath::Fmod(
			State.SwayPhase + Delta * Hertz * SpeedFraction * 2.f * PI, 2.f * PI);

		const float Amplitude = FMath::Max(0.f,
			Finite(Input.bSprinting ? Rules.SprintSwayCm : Rules.WalkSwayCm))
			* FMath::Min(1.f, SpeedFraction);

		// A figure-of-eight: horizontal at the sway rate, vertical at twice
		// it, which is what a carried object does because the body rises once
		// per step and shifts side to side once per pair of steps.
		const FVector Sway(
			0.f,
			FMath::Sin(Next.SwayPhase) * Amplitude,
			FMath::Sin(Next.SwayPhase * 2.f) * Amplitude * 0.5f);

		// ---- the landing kick -------------------------------------------
		const float Impact = FMath::Max(0.f, Finite(Input.LandingImpactSpeed));
		if (Impact > 0.f)
		{
			const float FullAt = FMath::Max(1.f, Finite(Rules.FullKickImpactSpeed, 600.f));
			const float Kick = FMath::Min(1.f, Impact / FullAt)
				* FMath::Max(0.f, Finite(Rules.MaxLandingKickCm));

			// The hardest of the recent impacts wins rather than the sum, so a
			// stair descent does not accumulate into a dive.
			Next.KickCm = FMath::Max(Next.KickCm, Kick);
		}

		Next.KickCm *= 1.f - ApproachAlpha(Rules.KickRecoveryPerSecond, Delta);
		if (Next.KickCm < 0.01f)
		{
			Next.KickCm = 0.f;
		}

		// ---- and the one bounded result ---------------------------------
		const FVector Desired = Sway - FVector(0.f, 0.f, Next.KickCm);
		Next.OffsetCm = ClampedOffset(Desired, Input.FreeDistanceCm, Rules);
		return Next;
	}
}
