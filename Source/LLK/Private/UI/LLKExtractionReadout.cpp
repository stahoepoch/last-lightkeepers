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

	FLLKExtractionReadout Describe(const FLLKExtractionSnapshot& Snapshot, const bool bAtZone)
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
