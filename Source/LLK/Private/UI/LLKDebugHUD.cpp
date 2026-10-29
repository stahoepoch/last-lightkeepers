#include "UI/LLKDebugHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "Storm/LLKStormChargeSubsystem.h"

namespace
{
	/**
	 * Off switch, because a debug readout welded on is a debug readout that
	 * ends up in a screenshot. A console variable rather than a key: a key
	 * would mean a seventh input action, and M1.6's asset set is already
	 * all-or-nothing on six.
	 *
	 *   LLK.ShowStormHUD 0
	 */
	static TAutoConsoleVariable<int32> CVarShowStormHUD(
		TEXT("LLK.ShowStormHUD"),
		1,
		TEXT("Draw the M1.7 Storm Charge debug readout. 1 = on (default), 0 = off."),
		ECVF_Default);

	// Warm, matching the lantern, for the things the player controls. Cold for
	// the storm, which they do not. The colours are the same argument the
	// harness lighting makes: the keeper's light is the one human thing.
	const FLinearColor ColourHeading(0.75f, 0.80f, 0.90f, 1.f);
	const FLinearColor ColourUrgent(1.00f, 0.45f, 0.35f, 1.f);
	const FLinearColor ColourCause(1.00f, 0.82f, 0.56f, 1.f);
	const FLinearColor ColourQuiet(0.55f, 0.58f, 0.62f, 1.f);
}

ALLKDebugHUD::ALLKDebugHUD()
{
	PrimaryActorTick.bCanEverTick = false;
}

FString ALLKDebugHUD::FormatSeconds(float Seconds)
{
	// Nothing left to arrive, or a rate of zero. Rendering either as 0:00
	// would say the opposite of what is true - that it is due immediately
	// rather than never - so it gets a dash and no false precision.
	if (!FMath::IsFinite(Seconds) || Seconds >= TNumericLimits<float>::Max() * 0.5f)
	{
		return TEXT("--:--");
	}

	// Already passed. Shown rather than clamped, because "due now" and "long
	// since gone" are different states and a readout that flattens them hides
	// a stage that failed to fire.
	if (Seconds < 0.f)
	{
		return TEXT("now");
	}

	const int32 Total = FMath::FloorToInt(Seconds);
	return FString::Printf(TEXT("%d:%02d"), Total / 60, Total % 60);
}

void ALLKDebugHUD::DrawLine(const FString& Text, const FLinearColor& Colour, float& CursorY, float Scale)
{
	if (Canvas == nullptr)
	{
		return;
	}

	UFont* Font = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
	if (Font == nullptr)
	{
		return;
	}

	FCanvasTextItem Item(FVector2D(PanelOrigin.X, CursorY), FText::FromString(Text), Font, Colour);
	Item.Scale = FVector2D(Scale, Scale);
	// A readout over a dark floor and a lit pillar has no single legible
	// colour; the shadow is what makes it readable over both.
	Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.85f));
	Canvas->DrawItem(Item);

	CursorY += LineHeight * Scale;
}

void ALLKDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (CVarShowStormHUD.GetValueOnGameThread() == 0)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr || Canvas == nullptr)
	{
		return;
	}

	float CursorY = PanelOrigin.Y;

	ULLKStormChargeSubsystem* Storm = World->GetSubsystem<ULLKStormChargeSubsystem>();
	if (Storm == nullptr)
	{
		// Says which thing is missing. "No HUD" and "no subsystem" look
		// identical on screen otherwise, and they have different fixes.
		DrawLine(TEXT("STORM  no ULLKStormChargeSubsystem in this world"), ColourUrgent, CursorY);
		return;
	}

	DrawStormPanel(Storm, CursorY);
}

void ALLKDebugHUD::DrawStormPanel(ULLKStormChargeSubsystem* Storm, float& CursorY)
{
	// The client case, stated rather than papered over. The meter accrues on
	// authority only, so a client's zero is honest - and rendering it as a
	// meter would make this readout lie about the one number everything reads.
	if (!Storm->IsAccruing())
	{
		DrawLine(TEXT("STORM  not replicated to clients yet - this is the server's number"),
			ColourQuiet, CursorY);
		return;
	}

	const int32 Lit = Storm->GetLitLanternCount();
	const float Multiplier = Storm->GetCurrentMultiplier();

	// ---- the consequence, first and largest ---------------------------
	//
	// Time, not charge. This is the line the whole readout exists for: it is
	// what changes the moment the lantern comes on, and it is what makes the
	// lantern a decision instead of a light.
	FName NextId;
	float Target = 0.f;
	float Current = 0.f;
	if (Storm->GetNextEscalation(NextId, Target, Current))
	{
		const bool bWardenNext = NextId == TEXT("WardenConverges");
		const float Seconds = bWardenNext
			? Storm->SecondsUntilWarden(Lit)
			: Storm->SecondsUntilCharge(Current, Target, Lit);

		DrawLine(
			FString::Printf(TEXT("%s in %s"), *NextId.ToString(), *FormatSeconds(Seconds)),
			bWardenNext ? ColourUrgent : ColourHeading, CursorY, 1.6f);

		// The counterfactual, which is the actual teaching. One line that
		// says what the other choice costs - "and 0:58 lit" beside "2:24
		// dark" is row 5 without a word of tutorial text.
		const int32 Counterfactual = Lit > 0 ? 0 : 1;
		const float OtherSeconds = bWardenNext
			? Storm->SecondsUntilWarden(Counterfactual)
			: Storm->SecondsUntilCharge(Current, Target, Counterfactual);

		DrawLine(
			FString::Printf(TEXT("  %s if you %s"),
				*FormatSeconds(OtherSeconds),
				Lit > 0 ? TEXT("put every lantern out") : TEXT("light one lantern")),
			ColourCause, CursorY);
	}
	else
	{
		DrawLine(TEXT("the night has nothing left to send"), ColourUrgent, CursorY, 1.6f);
	}

	CursorY += LineHeight * 0.4f;

	// ---- the cause, beside it -----------------------------------------
	//
	// A player shown only the consequence learns to fear the meter. Shown
	// both, they learn what they did.
	DrawLine(
		FString::Printf(TEXT("lanterns lit  %d       multiplier  x%.2f       rate  %.2f/s"),
			Lit, Multiplier, Storm->GetCurrentRate()),
		Lit > 0 ? ColourCause : ColourQuiet, CursorY);

	// ---- the two meters, last, because they are the trivia -------------
	DrawLine(
		FString::Printf(TEXT("charge  %.0f%s       elapsed  %s"),
			Storm->GetCharge(),
			Target > 0.f ? *FString::Printf(TEXT(" / %.0f"), Target) : TEXT(""),
			*FormatSeconds(Storm->GetElapsedSeconds())),
		ColourQuiet, CursorY);

	// Row 54: the Warden's own dampened meter, shown separately because it is
	// a different cost for the same light. Seeing x2.50 beside x1.45 is the
	// clearest statement of the dampening anybody will get.
	DrawLine(
		FString::Printf(TEXT("warden  %.0f       x%.2f       arrives  %s%s"),
			Storm->GetWardenCharge(),
			Storm->GetCurrentWardenMultiplier(),
			*FormatSeconds(Storm->SecondsUntilWarden(Lit)),
			Storm->HasWardenConverged() ? TEXT("  (here)") : TEXT("")),
		Storm->HasWardenConverged() ? ColourUrgent : ColourQuiet, CursorY);

	// Which data it read. The fallback path matches the asset today, so this
	// is the only way to tell them apart from inside the game - and "it was
	// on fallbacks the whole time" has already cost this project an evening.
	DrawLine(
		FString::Printf(TEXT("source  %s"),
			Storm->GetRulesSource() == ELLKDataSource::Asset
				? TEXT("DA_StormCharge") : TEXT("DECLARED FALLBACKS")),
		Storm->GetRulesSource() == ELLKDataSource::Asset ? ColourQuiet : ColourUrgent, CursorY);
}
