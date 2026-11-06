#include "UI/LLKExtractionReadout.h"

namespace
{
	/**
	 * The dash Chris wrote, as an escape rather than a literal.
	 *
	 * An em dash typed into a source file depends on the file's encoding
	 * surviving every tool that touches it, and this one is read by Node as
	 * well as by MSVC. `\x2014` cannot be mangled.
	 */
	const TCHAR* Dash = TEXT(" \x2014 ");
}

namespace LLKExtractionReadout
{
	FString FormatSeconds(const float Seconds)
	{
		if (!FMath::IsFinite(Seconds) || Seconds <= 0.f)
		{
			return TEXT("0:00");
		}

		// Rounds UP, which is the whole of the decision here. An eight-second
		// siren that reads 0:07 the instant it starts looks like a bug, and a
		// hold with four tenths of a second left reading 0:00 looks like one
		// too. A countdown shows the second you are in, not the one you have
		// finished.
		const int32 Whole = FMath::CeilToInt(Seconds);
		return FString::Printf(TEXT("%d:%02d"), Whole / 60, Whole % 60);
	}

	FString HumaniseExitId(const FName ExitId)
	{
		if (ExitId.IsNone())
		{
			return FString();
		}

		// A space before each capital that follows a lower-case letter, which
		// turns LoadingDock into Loading Dock and leaves an already-spaced or
		// single-word id alone.
		const FString Raw = ExitId.ToString();
		FString Out;
		Out.Reserve(Raw.Len() + 4);
		for (int32 Index = 0; Index < Raw.Len(); ++Index)
		{
			const TCHAR Ch = Raw[Index];
			if (Index > 0 && FChar::IsUpper(Ch) && FChar::IsLower(Raw[Index - 1]))
			{
				Out.AppendChar(TEXT(' '));
			}
			Out.AppendChar(Ch);
		}
		return Out;
	}

	FString BearingPhrase(const float RelativeYawDegrees)
	{
		if (!FMath::IsFinite(RelativeYawDegrees))
		{
			return FString();
		}

		// Normalised here as well as by the caller. A bearing is the kind of
		// value that arrives as 350 rather than -10 from somewhere nobody
		// remembered, and 350 would land in the last sector and say "ahead
		// right" for something slightly to the left.
		const float Yaw = FRotator::NormalizeAxis(RelativeYawDegrees);
		const float Magnitude = FMath::Abs(Yaw);
		const bool bRight = Yaw > 0.f;

		if (Magnitude <= 22.5f)
		{
			return TEXT("ahead");
		}
		if (Magnitude <= 67.5f)
		{
			return bRight ? TEXT("ahead and right") : TEXT("ahead and left");
		}
		if (Magnitude <= 112.5f)
		{
			return bRight ? TEXT("to your right") : TEXT("to your left");
		}
		if (Magnitude <= 157.5f)
		{
			return bRight ? TEXT("behind you, right") : TEXT("behind you, left");
		}
		return TEXT("behind you");
	}

	FString FormatMetres(const float Centimetres)
	{
		if (!FMath::IsFinite(Centimetres) || Centimetres <= 0.f)
		{
			return FString();
		}

		const int32 Metres = FMath::Max(1, FMath::FloorToInt(Centimetres / 100.f));
		return FString::Printf(TEXT("%d m"), Metres);
	}

	FLLKExtractionReadout Describe(const FLLKExtractionSnapshot& Snapshot, const bool bAtZone,
		const int32 CarriedCount, const FName NearestExitId,
		const float ExitDistanceCm, const float ExitBearingDegrees)
	{
		FLLKExtractionReadout Readout;

		// Before the server has spoken once, a default snapshot is
		// field-by-field identical to a genuine idle run. `bPublished` is what
		// tells them apart, and until it is set the readout says nothing
		// rather than offering a key that may not work yet - the same rule the
		// storm meter follows about drawing a number it does not have.
		if (!Snapshot.bPublished)
		{
			return Readout;
		}

		switch (Snapshot.Phase)
		{
		case ELLKExtractionPhase::Idle:
			// The offer, and only where it can be taken. Nothing centre
			// screen: an idle run has no climax, and a permanent line about
			// extraction is the top-left paragraph by another route.
			if (bAtZone)
			{
				Readout.ZonePrompt = FString::Printf(TEXT("Hold [E]%scall Lanternhold"), Dash);
				return Readout;
			}

			// Not at an exit. The offer above is only visible from inside the
			// bay, so until this line existed a keeper who did not already
			// know where the bay was got nothing at all - which is what
			// "i still can't extract or know where to go or what to do"
			// was. See `FLLKExtractionReadout::Objective` for why this is not
			// a centre line, and for why it is no longer gated on carrying.
			if (!NearestExitId.IsNone())
			{
				// Name, then direction when the caller knows one. The two are
				// built separately so a caller that cannot answer "which way"
				// still answers "where to", rather than the line degrading
				// into a bearing with nothing attached to it.
				const FString Metres = FormatMetres(ExitDistanceCm);
				const FString Bearing = BearingPhrase(ExitBearingDegrees);
				const FString Where = (Metres.IsEmpty() || Bearing.IsEmpty())
					? HumaniseExitId(NearestExitId)
					: FString::Printf(TEXT("%s, %s %s"),
						*HumaniseExitId(NearestExitId), *Metres, *Bearing);

				// Two errands, and the keeper is always on one of them. The
				// empty-handed sentence names the goal the game had never
				// stated anywhere: that a relic is the point, and that the
				// way out exists.
				Readout.Objective = CarriedCount > 0
					? FString::Printf(TEXT("Carrying %d%shold [E] at the %s"),
						CarriedCount, Dash, *Where)
					: FString::Printf(TEXT("Find something worth carrying%sthe way out is the %s"),
						Dash, *Where);
			}
			return Readout;

		case ELLKExtractionPhase::Announcing:
			// An announcement, not a demand: the rig is on the road and there
			// is nothing to hold yet. The countdown is the siren, which is
			// also exactly how long the building has known.
			Readout.Centre = FString::Printf(TEXT("RECOVERY INCOMING%s%s"),
				Dash, *FormatSeconds(Snapshot.SirenSecondsRemaining));
			return Readout;

		case ELLKExtractionPhase::Holding:
			if (Snapshot.bHoldStalled)
			{
				// **No timer on this line, deliberately.** The hold is paused,
				// so a number counting down would be false - and a number
				// frozen on screen reads as a broken HUD rather than as a
				// paused hold. The sentence is the instruction instead.
				//
				// Row 75, in Chris's words: the rig *"holds position while the
				// route is unsafe"*. It is waiting for you, which is why this
				// says holding rather than failing.
				Readout.Centre = FString::Printf(TEXT("RECOVERY HOLDING%sreturn to the dock"), Dash);
			}
			else
			{
				Readout.Centre = FString::Printf(TEXT("HOLD THE DOCK%s%s"),
					Dash, *FormatSeconds(Snapshot.HoldSecondsRemaining));
			}
			Readout.bUrgent = true;
			return Readout;

		case ELLKExtractionPhase::Secured:
			// One word. Row 8's only path to keeping a haul deserves no
			// qualifier, and the count and the payout are the hub's business
			// rather than this frame's.
			Readout.Centre = TEXT("SECURED");
			return Readout;

		case ELLKExtractionPhase::Failed:
			// Nothing. The wipe has its own centre-screen message - "CREW
			// CONTAINED. The night keeps what you carried." - and two of them
			// at once would be the paragraph Chris asked this not to become.
			return Readout;
		}

		return Readout;
	}
}
