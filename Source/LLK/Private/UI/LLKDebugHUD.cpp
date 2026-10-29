#include "UI/LLKDebugHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "LLKGameState.h"
#include "LLKUnits.h"
#include "Loot/LLKRelicComponent.h"
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

float ALLKDebugHUD::GetUIScale() const
{
	if (Canvas == nullptr || Canvas->SizeY <= 0)
	{
		return 1.f;
	}

	// Against 1080p, and never smaller: on a 4K monitor this is 2x, which is
	// what stops the readout shrinking as the screen gets bigger. Clamped at
	// the bottom so a small window gets a cramped panel rather than an
	// illegible one.
	return FMath::Max(1.f, static_cast<float>(Canvas->SizeY) / 1080.f);
}

void ALLKDebugHUD::DrawBacking(float Height) const
{
	if (Canvas == nullptr)
	{
		return;
	}

	const float Scale = GetUIScale();
	const float Pad = 14.f * Scale;

	// Drawn first, so the text lands on top. Without it the readout is
	// unreadable over a lit pillar and barely readable over a dark floor,
	// which is most of this harness.
	FCanvasTileItem Tile(
		FVector2D(PanelOrigin.X * Scale - Pad, PanelOrigin.Y * Scale - Pad),
		FVector2D(560.f * Scale + Pad * 2.f, Height + Pad * 2.f),
		BackingColour);
	Tile.BlendMode = SE_BLEND_AlphaBlend;
	Canvas->DrawItem(Tile);
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

	const float UIScale = GetUIScale();
	const float Final = Scale * TextScale * UIScale;

	FCanvasTextItem Item(FVector2D(PanelOrigin.X * UIScale, CursorY), FText::FromString(Text), Font, Colour);
	Item.Scale = FVector2D(Final, Final);
	// Kept even with the backing panel: the shadow is what holds the thinner
	// glyphs together at a glance, which is the only way this gets read.
	Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.9f));
	Canvas->DrawItem(Item);

	CursorY += LineHeight * Scale * UIScale;
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

	// Seven lines plus the spacer, at the current scale. Measured rather than
	// drawn-then-resized, because the canvas is immediate-mode and the panel
	// has to go down before the text does.
	// Seven storm lines, a spacer, and two relic lines.
	DrawBacking(LineHeight * GetUIScale() * 10.6f);

	float CursorY = PanelOrigin.Y * GetUIScale();

	// The subsystem, for the ARITHMETIC only - it holds the rules, which are
	// static data present on every machine. None of its meters are read here.
	const ULLKStormChargeSubsystem* Storm = World->GetSubsystem<ULLKStormChargeSubsystem>();
	if (Storm == nullptr)
	{
		// Each missing piece names itself. On screen they all look like "no
		// HUD", and they have three different fixes.
		DrawLine(TEXT("STORM  no ULLKStormChargeSubsystem in this world"), ColourUrgent, CursorY);
		return;
	}

	const ALLKGameState* GameState = World->GetGameState<ALLKGameState>();
	if (GameState == nullptr)
	{
		DrawLine(TEXT("STORM  no ALLKGameState - set GameStateClass to ALLKGameState"),
			ColourUrgent, CursorY);
		return;
	}

	DrawStormPanel(Storm, GameState->GetStormSnapshot(), CursorY);

	CursorY += LineHeight * GetUIScale() * 0.6f;
	DrawRelicPanel(CursorY);
}

void ALLKDebugHUD::DrawRelicPanel(float& CursorY)
{
	const APawn* Keeper = GetOwningPawn();
	const UWorld* World = GetWorld();
	if (Keeper == nullptr || World == nullptr)
	{
		return;
	}

	const float SearchRadius = LLKUnits::DistanceFromStuds(RelicSearchStuds);
	const FVector From = Keeper->GetActorLocation();

	// Nearest, by iterating the relics rather than by an overlap query: this
	// is a debug readout in a harness with a few dozen actors, and a sphere
	// trace would need a collision channel that the loot props deliberately
	// do not have.
	const ULLKRelicComponent* Nearest = nullptr;
	float NearestDistanceSquared = SearchRadius * SearchRadius;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		const ULLKRelicComponent* Relic = It->FindComponentByClass<ULLKRelicComponent>();
		if (Relic == nullptr)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(From, It->GetActorLocation());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			Nearest = Relic;
		}
	}

	if (Nearest == nullptr)
	{
		DrawLine(TEXT("no loot within reach"), ColourQuiet, CursorY);
		return;
	}

	// The name, which is the visible half of row 53. In the dark this reads
	// "Something on the shelf" and the point is that it reads SOMETHING.
	DrawLine(
		FString::Printf(TEXT("%s"), *Nearest->GetDisplayName().ToString()),
		Nearest->IsLegible() ? ColourCause : ColourQuiet, CursorY, 1.25f);

	// And the affordance, which is the second channel. "needs light" is the
	// line that teaches: a keeper reaching for a relic they already read, and
	// being told why they cannot have it yet.
	FString Affordance;
	FLinearColor AffordanceColour = ColourQuiet;
	switch (Nearest->GetInteractBlock())
	{
	case ELLKRelicInteractBlock::None:
		Affordance = FString::Printf(TEXT("  can take it%s"),
			Nearest->GetReadableValue() >= 0
				? *FString::Printf(TEXT("  -  worth %d"), Nearest->GetReadableValue()) : TEXT(""));
		AffordanceColour = ColourCause;
		break;
	case ELLKRelicInteractBlock::NeedsLight:
		Affordance = TEXT("  needs light");
		AffordanceColour = ColourUrgent;
		break;
	case ELLKRelicInteractBlock::Unidentified:
		Affordance = TEXT("  unreadable in the dark");
		break;
	}
	DrawLine(Affordance, AffordanceColour, CursorY);
}

void ALLKDebugHUD::DrawStormPanel(
	const ULLKStormChargeSubsystem* Storm, const FLLKStormSnapshot& Snapshot, float& CursorY)
{
	// Nothing has arrived yet. A default snapshot is field-for-field identical
	// to a genuine all-zero start of night, and those mean opposite things, so
	// this is the flag and not a zero check.
	if (!Snapshot.bPublished)
	{
		DrawLine(TEXT("STORM  waiting for the server's first snapshot"), ColourQuiet, CursorY);
		return;
	}

	// Everything below reads the SNAPSHOT, on the server as well as on a
	// client. The server could read its own meters directly and deliberately
	// does not: one path means both screens show the same thing for the same
	// reason, which is the first thing M1.9's two-machine test needs.
	const int32 Lit = Snapshot.LitLanternCount;
	const float Multiplier = Snapshot.Multiplier;

	// ---- the consequence, first and largest ---------------------------
	//
	// Time, not charge. This is the line the whole readout exists for: it is
	// what changes the moment the lantern comes on, and it is what makes the
	// lantern a decision instead of a light.
	if (Snapshot.bHasNextEscalation)
	{
		const bool bWardenNext = Snapshot.NextEscalationId == TEXT("WardenConverges");

		// Projected here rather than replicated. Both machines run this on
		// the same ULLKStormChargeData, so the countdown agrees without the
		// server sending one - and it keeps ticking smoothly between the
		// 10Hz snapshots instead of stepping.
		const float Seconds = Storm->SecondsUntilSnapshotEscalation(Snapshot, Lit);

		DrawLine(
			FString::Printf(TEXT("%s in %s"),
				*Snapshot.NextEscalationId.ToString(), *FormatSeconds(Seconds)),
			bWardenNext ? ColourUrgent : ColourHeading, CursorY, 1.6f);

		// The counterfactual, which is the actual teaching. One line that
		// says what the other choice costs - "and 0:58 lit" beside "2:24
		// dark" is row 5 without a word of tutorial text.
		const int32 Counterfactual = Lit > 0 ? 0 : 1;
		const float OtherSeconds = Storm->SecondsUntilSnapshotEscalation(Snapshot, Counterfactual);

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
			Lit, Multiplier,
			// Rate from the snapshot's multiplier, not from a live recount:
			// a client has no lanterns registered, so GetCurrentRate() there
			// would print the baseline while the multiplier beside it said
			// x2.50 - two numbers from two machines on one line.
			Storm->GetRules() != nullptr
				? Storm->GetRules()->BaselineRatePerSecond * Multiplier : 0.f),
		Lit > 0 ? ColourCause : ColourQuiet, CursorY);

	// ---- the two meters, last, because they are the trivia -------------
	DrawLine(
		FString::Printf(TEXT("charge  %.0f%s       elapsed  %s"),
			Snapshot.Charge,
			Snapshot.bHasNextEscalation
				? *FString::Printf(TEXT(" / %.0f"), Snapshot.NextTargetCharge) : TEXT(""),
			*FormatSeconds(Snapshot.ElapsedSeconds)),
		ColourQuiet, CursorY);

	// Row 54: the Warden's own dampened meter, shown separately because it is
	// a different cost for the same light. Seeing x2.50 beside x1.45 is the
	// clearest statement of the dampening anybody will get.
	DrawLine(
		FString::Printf(TEXT("warden  %.0f       x%.2f       arrives  %s%s"),
			Snapshot.WardenCharge,
			Snapshot.WardenMultiplier,
			*FormatSeconds(Storm->SecondsUntilWardenFrom(Snapshot.WardenCharge, Lit)),
			Snapshot.bWardenConverged ? TEXT("  (here)") : TEXT("")),
		Snapshot.bWardenConverged ? ColourUrgent : ColourQuiet, CursorY);

	// Which data it read. The fallback path matches the asset today, so this
	// is the only way to tell them apart from inside the game - and "it was
	// on fallbacks the whole time" has already cost this project an evening.
	// The SERVER's source, carried on the snapshot. A client printing its own
	// would be reporting which data it loaded, not which data the night is
	// actually running on - and those can differ, which is exactly the
	// mismatch worth seeing on a two-machine test.
	DrawLine(
		FString::Printf(TEXT("source  %s%s"),
			Snapshot.RulesSource == ELLKDataSource::Asset
				? TEXT("DA_StormCharge") : TEXT("DECLARED FALLBACKS"),
			Storm->IsAccruing() ? TEXT("") : TEXT("  (server's, replicated)")),
		Snapshot.RulesSource == ELLKDataSource::Asset ? ColourQuiet : ColourUrgent, CursorY);
}
