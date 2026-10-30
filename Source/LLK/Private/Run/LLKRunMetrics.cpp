#include "Run/LLKRunMetrics.h"

namespace LLKRunMetrics
{
	FLLKRunSummary Summarise(const TArray<FLLKRunEvent>& Events)
	{
		FLLKRunSummary Summary;
		if (Events.IsEmpty())
		{
			return Summary;
		}

		// The run's length is the last thing that happened, not a stored
		// duration: a run the editor cut short still has a length, and it is
		// the last moment anybody observed.
		Summary.RunSeconds = FMath::Max(0.f, Events.Last().AtSeconds);

		// The lit clock is an interval problem, not a count. Nested lanterns
		// are the normal case - four keepers, four lights, overlapping - so
		// this tracks DEPTH rather than a bool: lit seconds are seconds with
		// at least one lantern burning, however many were.
		int32 LanternDepth = 0;
		float LitSince = 0.f;

		// The same shape for the stall, which cannot nest: one hold, one stall.
		bool bStalled = false;
		float StalledSince = 0.f;

		for (const FLLKRunEvent& Event : Events)
		{
			switch (Event.Kind)
			{
			case ELLKRunEvent::LanternOn:
				if (LanternDepth == 0)
				{
					LitSince = Event.AtSeconds;
				}
				++LanternDepth;
				break;

			case ELLKRunEvent::LanternOff:
				LanternDepth = FMath::Max(0, LanternDepth - 1);
				if (LanternDepth == 0)
				{
					Summary.LitSeconds += FMath::Max(0.f, Event.AtSeconds - LitSince);
				}
				break;

			case ELLKRunEvent::RelicTaken:
				++Summary.RelicsTaken;
				break;

			case ELLKRunEvent::ThresholdCrossed:
				++Summary.ThresholdsCrossed;
				break;

			case ELLKRunEvent::WardenConverged:
				Summary.bWardenArrived = true;
				break;

			case ELLKRunEvent::ExtractionCalled:
				// The FIRST call, kept. A crew that called at 4:00, cancelled
				// and called again at 9:30 had the argument row 3 asks about,
				// and the moment they first decided is the interesting one -
				// `CancelCount` carries the rest of that story.
				if (Summary.ExtractionCalledSeconds < 0.f)
				{
					Summary.ExtractionCalledSeconds = Event.AtSeconds;
				}
				break;

			case ELLKRunEvent::ExtractionCancelled:
				++Summary.CancelCount;

				// A cancelled hold cannot still be stalled. Closed here rather
				// than waiting for a `HoldResumed` that will never come.
				if (bStalled)
				{
					Summary.HoldStallSeconds += FMath::Max(0.f, Event.AtSeconds - StalledSince);
					bStalled = false;
				}
				break;

			case ELLKRunEvent::HoldStalled:
				if (!bStalled)
				{
					bStalled = true;
					StalledSince = Event.AtSeconds;
					++Summary.HoldStallCount;
				}
				break;

			case ELLKRunEvent::HoldResumed:
				if (bStalled)
				{
					Summary.HoldStallSeconds += FMath::Max(0.f, Event.AtSeconds - StalledSince);
					bStalled = false;
				}
				break;

			case ELLKRunEvent::KeeperDowned:
				++Summary.KnockdownCount;
				break;

			case ELLKRunEvent::ExtractionSecured:
				Summary.Outcome = ELLKRunOutcome::Secured;

				// What left the building, as counted by the subsystem at the
				// moment it left. Not derived from `RelicsTaken`, because a
				// keeper can go down and lose a crate on the way (row 59).
				Summary.RelicsSecured = FMath::RoundToInt(Event.Value);
				break;

			case ELLKRunEvent::CrewWiped:
				Summary.Outcome = ELLKRunOutcome::Wiped;

				// Row 8, stated as arithmetic rather than trusted to the
				// caller: a wipe keeps nothing, whatever anybody was carrying.
				Summary.RelicsSecured = 0;
				break;

			default:
				break;
			}
		}

		// Still burning when the run ended: the interval runs to the end.
		if (LanternDepth > 0)
		{
			Summary.LitSeconds += FMath::Max(0.f, Summary.RunSeconds - LitSince);
		}

		// Still stalled when the run ended, which is what a wipe at the dock
		// looks like: the crew went down holding it.
		if (bStalled)
		{
			Summary.HoldStallSeconds += FMath::Max(0.f, Summary.RunSeconds - StalledSince);
		}

		if (Summary.RunSeconds > 0.f)
		{
			Summary.LitFraction = FMath::Clamp(Summary.LitSeconds / Summary.RunSeconds, 0.f, 1.f);

			if (Summary.ExtractionCalledSeconds >= 0.f)
			{
				const float Climax = Summary.RunSeconds - Summary.ExtractionCalledSeconds;
				Summary.ClimaxFraction = FMath::Clamp(Climax / Summary.RunSeconds, 0.f, 1.f);
			}
		}

		return Summary;
	}

	FString Describe(const FLLKRunSummary& Summary)
	{
		// Plain language, because the first reader is Chris after a playtest
		// rather than a spreadsheet. Every line is a number he can act on and
		// none of them is a judgement about the run.
		TArray<FString> Lines;

		Lines.Add(FString::Printf(TEXT("RUN  %s after %.0fs"),
			NameOf(Summary.Outcome), Summary.RunSeconds));

		Lines.Add(FString::Printf(TEXT("  light      %.0fs lit of %.0fs  (%.0f%% of the run)"),
			Summary.LitSeconds, Summary.RunSeconds, Summary.LitFraction * 100.f));

		Lines.Add(FString::Printf(TEXT("  haul       %d taken, %d out"),
			Summary.RelicsTaken, Summary.RelicsSecured));

		if (Summary.ExtractionCalledSeconds >= 0.f)
		{
			Lines.Add(FString::Printf(
				TEXT("  extraction called at %.0fs  (the last %.0f%% of the run)"),
				Summary.ExtractionCalledSeconds, Summary.ClimaxFraction * 100.f));
			Lines.Add(FString::Printf(
				TEXT("  the hold   stalled %d time(s) for %.0fs, cancelled %d time(s)"),
				Summary.HoldStallCount, Summary.HoldStallSeconds, Summary.CancelCount));
		}
		else
		{
			// Said explicitly rather than left as a missing line: a run with
			// no call is the most interesting failure the dock can produce.
			Lines.Add(TEXT("  extraction NEVER CALLED"));
		}

		Lines.Add(FString::Printf(TEXT("  pressure   %d threshold(s) crossed, Warden %s"),
			Summary.ThresholdsCrossed, Summary.bWardenArrived ? TEXT("arrived") : TEXT("did not arrive")));

		Lines.Add(FString::Printf(TEXT("  keepers    %d knockdown(s)"), Summary.KnockdownCount));

		return FString::Join(Lines, TEXT("\n"));
	}

	FString ToJson(const TArray<FLLKRunEvent>& Events, const FLLKRunSummary& Summary)
	{
		FString Json = TEXT("{\n");

		Json += TEXT("  \"summary\": {\n");
		Json += FString::Printf(TEXT("    \"outcome\": \"%s\",\n"), NameOf(Summary.Outcome));
		Json += FString::Printf(TEXT("    \"runSeconds\": %.2f,\n"), Summary.RunSeconds);
		Json += FString::Printf(TEXT("    \"litSeconds\": %.2f,\n"), Summary.LitSeconds);
		Json += FString::Printf(TEXT("    \"litFraction\": %.4f,\n"), Summary.LitFraction);
		Json += FString::Printf(TEXT("    \"relicsTaken\": %d,\n"), Summary.RelicsTaken);
		Json += FString::Printf(TEXT("    \"relicsSecured\": %d,\n"), Summary.RelicsSecured);
		Json += FString::Printf(TEXT("    \"extractionCalledSeconds\": %.2f,\n"),
			Summary.ExtractionCalledSeconds);
		Json += FString::Printf(TEXT("    \"climaxFraction\": %.4f,\n"), Summary.ClimaxFraction);
		Json += FString::Printf(TEXT("    \"cancelCount\": %d,\n"), Summary.CancelCount);
		Json += FString::Printf(TEXT("    \"holdStallSeconds\": %.2f,\n"), Summary.HoldStallSeconds);
		Json += FString::Printf(TEXT("    \"holdStallCount\": %d,\n"), Summary.HoldStallCount);
		Json += FString::Printf(TEXT("    \"knockdownCount\": %d,\n"), Summary.KnockdownCount);
		Json += FString::Printf(TEXT("    \"thresholdsCrossed\": %d,\n"), Summary.ThresholdsCrossed);
		Json += FString::Printf(TEXT("    \"wardenArrived\": %s\n"),
			Summary.bWardenArrived ? TEXT("true") : TEXT("false"));
		Json += TEXT("  },\n");

		// The events too, not just the summary. A metric that surprises
		// somebody has to be checkable against what happened, or the summary
		// is a number nobody can argue with for the wrong reason.
		Json += TEXT("  \"events\": [\n");
		for (int32 Index = 0; Index < Events.Num(); ++Index)
		{
			const FLLKRunEvent& Event = Events[Index];
			Json += FString::Printf(
				TEXT("    {\"at\": %.2f, \"kind\": \"%s\", \"detail\": \"%s\", \"value\": %.2f}%s\n"),
				Event.AtSeconds, NameOf(Event.Kind), *Event.Detail.ToString(), Event.Value,
				Index + 1 < Events.Num() ? TEXT(",") : TEXT(""));
		}
		Json += TEXT("  ]\n}\n");

		return Json;
	}

	const TCHAR* NameOf(const ELLKRunEvent Kind)
	{
		switch (Kind)
		{
		case ELLKRunEvent::RunStarted:          return TEXT("RunStarted");
		case ELLKRunEvent::LanternOn:           return TEXT("LanternOn");
		case ELLKRunEvent::LanternOff:          return TEXT("LanternOff");
		case ELLKRunEvent::LightLevelChanged:   return TEXT("LightLevelChanged");
		case ELLKRunEvent::RelicTaken:          return TEXT("RelicTaken");
		case ELLKRunEvent::ThresholdCrossed:    return TEXT("ThresholdCrossed");
		case ELLKRunEvent::WardenConverged:     return TEXT("WardenConverged");
		case ELLKRunEvent::ExtractionCalled:    return TEXT("ExtractionCalled");
		case ELLKRunEvent::RigArrived:          return TEXT("RigArrived");
		case ELLKRunEvent::ExtractionCancelled: return TEXT("ExtractionCancelled");
		case ELLKRunEvent::HoldStalled:         return TEXT("HoldStalled");
		case ELLKRunEvent::HoldResumed:         return TEXT("HoldResumed");
		case ELLKRunEvent::KeeperDowned:        return TEXT("KeeperDowned");
		case ELLKRunEvent::KeeperUp:            return TEXT("KeeperUp");
		case ELLKRunEvent::ExtractionSecured:   return TEXT("ExtractionSecured");
		case ELLKRunEvent::CrewWiped:           return TEXT("CrewWiped");
		}
		return TEXT("Unknown");
	}

	const TCHAR* NameOf(const ELLKRunOutcome Outcome)
	{
		switch (Outcome)
		{
		case ELLKRunOutcome::Unresolved: return TEXT("Unresolved");
		case ELLKRunOutcome::Secured:    return TEXT("Secured");
		case ELLKRunOutcome::Wiped:      return TEXT("Wiped");
		}
		return TEXT("Unknown");
	}
}
