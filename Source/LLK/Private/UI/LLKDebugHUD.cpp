#include "UI/LLKDebugHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "Characters/LLKCharacter.h"
#include "Characters/LLKVitalsComponent.h"
#include "Data/LLKEnemyDef.h"
#include "Enemies/LLKEnemyRosterSubsystem.h"
#include "Enemies/LLKPerception.h"
#include "Enemies/LLKSweeper.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Loot/LLKInteractorComponent.h"
#include "LLKGameState.h"
#include "LLKUnits.h"
#include "Loot/LLKBurden.h"
#include "Loot/LLKRelicComponent.h"
#include "Run/LLKExtractionZone.h"
#include "Storm/LLKZonePressureSubsystem.h"
#include "UI/LLKExtractionReadout.h"

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
		0,
		TEXT("Draw the M1.7 Storm Pressure debug readout. 1 = on, 0 = off (default). World prompts remain visible."),
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

void ALLKDebugHUD::BindSweepers()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Once a second, not every frame. Iterating every actor in the level at
	// frame rate to find pawns that almost never appear is the kind of cost
	// that hides in a debug class and shows up in a profile.
	const float Now = World->GetTimeSeconds();
	if (Now < NextSweeperScanSeconds)
	{
		return;
	}
	NextSweeperScanSeconds = Now + 1.f;

	for (TActorIterator<ALLKSweeper> It(World); It; ++It)
	{
		const FObjectKey Key(*It);
		if (BoundSweepers.Contains(Key))
		{
			continue;
		}
		BoundSweepers.Add(Key);

		It->OnTelegraph.AddDynamic(this, &ALLKDebugHUD::OnSweeperTelegraph);
		It->OnStockStarted.AddDynamic(this, &ALLKDebugHUD::OnSweeperStockStarted);
		It->OnStockInterrupted.AddDynamic(this, &ALLKDebugHUD::OnSweeperStockInterrupted);
		It->OnStockFiled.AddDynamic(this, &ALLKDebugHUD::OnSweeperStockFiled);
	}
}

void ALLKDebugHUD::OnSweeperTelegraph(FVector)
{
	// Row 16's announcement, which until now happened silently. This is not
	// the sound the row asks for - it is proof the broadcast fires, and a
	// stand-in a playtest can react to.
	ShowNotice(TEXT("something has noticed you"), ColourUrgent);
}

void ALLKDebugHUD::OnSweeperStockStarted(FVector)
{
	// `ColourCause` is the right one by its own name: something is happening
	// because of what the player left on the floor.
	ShowNotice(TEXT("a machine is dealing with something of yours"), ColourCause);
}

void ALLKDebugHUD::OnSweeperStockInterrupted(FVector)
{
	// **The one the player caused on purpose.** They lit up to save their
	// crate, and without this the success of that decision looked exactly
	// like nothing happening.
	ShowNotice(TEXT("it saw you, and left your crate where it was"), ColourHeading);
}

void ALLKDebugHUD::OnSweeperStockFiled(FVector)
{
	ShowNotice(TEXT("your crate has been put somewhere it belongs"), ColourUrgent);
}

void ALLKDebugHUD::ShowNotice(const FString& Text, const FLinearColor& Colour)
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	NoticeText = Text;
	NoticeColour = Colour;
	NoticeExpirySeconds = World->GetTimeSeconds() + NoticeSeconds;
}

void ALLKDebugHUD::DrawBeatNotice()
{
	const UWorld* World = GetWorld();
	if (World == nullptr || Canvas == nullptr || NoticeText.IsEmpty())
	{
		return;
	}

	if (World->GetTimeSeconds() >= NoticeExpirySeconds)
	{
		NoticeText.Reset();
		return;
	}

	// Low on the screen and away from the top-left panel, because this is for
	// the player rather than for whoever is reading the diagnostics.
	float CursorY = Canvas->SizeY * 0.72f;
	DrawLine(NoticeText, NoticeColour, CursorY);
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

	const UWorld* World = GetWorld();
	if (World == nullptr || Canvas == nullptr)
	{
		return;
	}

	// These are the playable world's language, not the top-left diagnostic
	// panel. A player still needs an object prompt, a rescue prompt and the
	// extraction state with LLK.ShowStormHUD disabled.
	BindSweepers();

	DrawWorldPrompt();
	DrawExtraction();
	DrawBeatNotice();

	if (CVarShowStormHUD.GetValueOnGameThread() == 0)
	{
		return;
	}

	// Seven lines plus the spacer, at the current scale. Measured rather than
	// drawn-then-resized, because the canvas is immediate-mode and the panel
	// has to go down before the text does.
	// Seven storm lines, a spacer, two relic lines, and the haul's one when
	// there is a haul. Measured for the tallest case rather than the current
	// one: a panel that grows and shrinks as a keeper picks things up is a
	// panel that flickers.
	DrawBacking(LineHeight * GetUIScale() * 11.8f);

	float CursorY = PanelOrigin.Y * GetUIScale();

	// The subsystem, for the ARITHMETIC only - it holds the rules, which are
	// static data present on every machine. None of its meters are read here.
	const ULLKZonePressureSubsystem* Storm = World->GetSubsystem<ULLKZonePressureSubsystem>();
	if (Storm == nullptr)
	{
		// Each missing piece names itself. On screen they all look like "no
		// HUD", and they have three different fixes.
		DrawLine(TEXT("STORM  no ULLKZonePressureSubsystem in this world"), ColourUrgent, CursorY);
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
	DrawBurdenLine(CursorY);
	DrawRelicPanel(CursorY);

}

void ALLKDebugHUD::DrawExtraction()
{
	const UWorld* World = GetWorld();
	if (Canvas == nullptr || World == nullptr)
	{
		return;
	}

	const ALLKGameState* GameState = World->GetGameState<ALLKGameState>();
	if (GameState == nullptr)
	{
		return;
	}

	const ALLKCharacter* Character = Cast<ALLKCharacter>(GetOwningPawn());
	const ULLKInteractorComponent* Interactor =
		Character != nullptr ? Character->GetInteractor() : nullptr;

	// **The prompt and the key agree because they read the same focus.** The
	// interactor resolves the zone last of its three - a casualty, then a
	// relic, then the bay - and `BeginInteract` branches in that order, so the
	// prompt is suppressed in exactly the cases where E would do something
	// else. A prompt offering a key that lifts a brooch instead is the bug
	// this whole component was built to avoid.
	const bool bAtZone = Interactor != nullptr
		&& Interactor->GetFocusedZone() != nullptr
		&& Interactor->GetFocusedCasualty() == nullptr
		&& Interactor->GetFocused() == nullptr;

	// **The nearest authored exit, for the objective line.** Found here rather
	// than passed through the snapshot, because which exit is closest is a
	// fact about where THIS keeper is standing - the same reason `bAtZone` is
	// answered locally. Zones that name an exit nothing authored already
	// report themselves at BeginPlay, so an id reaching this point is one
	// `data/loot.json` knows.
	const int32 CarriedCount = Interactor != nullptr ? Interactor->GetCarriedCount() : 0;
	FName NearestExitId = NAME_None;
	FVector NearestExitLocation = FVector::ZeroVector;

	// **Searched whether or not the keeper is carrying anything.** It used to
	// be gated on `CarriedCount > 0`, which is why a keeper who had just
	// spawned was told nothing: no relic, no search, no exit, no line. The
	// gate's stated reason - that extraction refuses an empty-handed call -
	// is not in `ELLKExtractionRefusal`, which has seven reasons and none
	// about carrying.
	if (Character != nullptr)
	{
		const FVector From = Character->GetActorLocation();
		float NearestSquared = TNumericLimits<float>::Max();
		for (TActorIterator<ALLKExtractionZone> It(const_cast<UWorld*>(World)); It; ++It)
		{
			if (It->ExtractionPointId.IsNone())
			{
				continue;
			}

			const float DistanceSquared = FVector::DistSquared(It->GetActorLocation(), From);
			if (DistanceSquared < NearestSquared)
			{
				NearestSquared = DistanceSquared;
				NearestExitId = It->ExtractionPointId;
				NearestExitLocation = It->GetActorLocation();
			}
		}
	}

	// ---- which way, not just where -------------------------------------
	//
	// A name is only navigation to someone who already knows the building.
	// The bearing is relative to where this keeper is LOOKING rather than to
	// the world, and it is computed here because the view rotation lives
	// here - `LLKExtractionReadout` stays a pure function of numbers.
	//
	// Horizontal distance deliberately. The dock header sits at 260-300 cm
	// and a keeper's eye at 152, so the straight-line distance to a zone
	// overhead reads longer than the walk, and the walk is the thing being
	// described.
	float ExitDistanceCm = -1.f;
	float ExitBearingDegrees = 0.f;
	if (!NearestExitId.IsNone() && Character != nullptr)
	{
		const FVector To = NearestExitLocation - Character->GetActorLocation();
		ExitDistanceCm = To.Size2D();

		// `GetBaseAimRotation` rather than the controller's control rotation:
		// it resolves to exactly that for a locally controlled pawn, needs no
		// null check and no extra include, and it is also correct for a pawn
		// this machine does not control - which the split-screen and
		// spectator cases would otherwise get wrong in a way nobody would
		// see until someone tried them.
		const float ViewYaw = Character->GetBaseAimRotation().Yaw;
		ExitBearingDegrees = FRotator::NormalizeAxis(
			FMath::RadiansToDegrees(FMath::Atan2(To.Y, To.X)) - ViewYaw);
	}

	const FLLKExtractionReadout Readout = LLKExtractionReadout::Describe(
		GameState->GetExtractionSnapshot(), bAtZone, CarriedCount, NearestExitId,
		ExitDistanceCm, ExitBearingDegrees);
	if (!Readout.HasAnything())
	{
		return;
	}

	UFont* Font = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
	if (Font == nullptr)
	{
		return;
	}

	const float Scale = GetUIScale();

	// ---- the offer, at the zone ----------------------------------------
	//
	// Projected from the zone's own position, so it belongs to the dock the
	// way the relic bracket belongs to the shelf.
	if (!Readout.ZonePrompt.IsEmpty() && Interactor != nullptr)
	{
		const ALLKExtractionZone* Zone = Interactor->GetFocusedZone();
		const FVector Screen = Zone != nullptr ? Canvas->Project(Zone->GetActorLocation()) : FVector::ZeroVector;
		if (Zone != nullptr && Screen.Z > 0.f)
		{
			float W = 0.f;
			float H = 0.f;
			const float S = 1.2f * Scale;
			Canvas->TextSize(Font, Readout.ZonePrompt, W, H, S, S);

			FCanvasTextItem Item(
				FVector2D(Screen.X - W * 0.5f, Screen.Y),
				FText::FromString(Readout.ZonePrompt), Font, ColourCause);
			Item.Scale = FVector2D(S, S);
			Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.95f));
			Canvas->DrawItem(Item);
		}
	}

	// ---- the event, centre screen ---------------------------------------
	if (!Readout.Centre.IsEmpty())
	{
		// Above the middle rather than on it, so it is not under the crosshair
		// a keeper is aiming a lantern with. Below the wipe message's line,
		// which is the only other thing allowed to speak here - and the two
		// never overlap, because a failed extraction says nothing.
		const float CentreX = Canvas->SizeX * 0.5f;
		const float Y = Canvas->SizeY * 0.3f;

		float W = 0.f;
		float H = 0.f;
		const float S = 2.1f * Scale;
		Canvas->TextSize(Font, Readout.Centre, W, H, S, S);

		FCanvasTextItem Item(FVector2D(CentreX - W * 0.5f, Y),
			FText::FromString(Readout.Centre), Font,
			Readout.bUrgent ? ColourUrgent : ColourCause);
		Item.Scale = FVector2D(S, S);
		Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.95f));
		Canvas->DrawItem(Item);
	}

	// ---- the errand, low and quiet --------------------------------------
	//
	// Bottom-centre and at the zone prompt's size rather than the centre
	// line's, because this is a standing reminder and not an event. It is in
	// the lower third so it does not compete with the crosshair or the shelf
	// brackets, and it leaves centre screen free for the climax that `Centre`
	// is reserved for.
	if (!Readout.Objective.IsEmpty())
	{
		float W = 0.f;
		float H = 0.f;
		const float S = 1.2f * Scale;
		Canvas->TextSize(Font, Readout.Objective, W, H, S, S);

		FCanvasTextItem Item(
			FVector2D(Canvas->SizeX * 0.5f - W * 0.5f, Canvas->SizeY * 0.86f),
			FText::FromString(Readout.Objective), Font, ColourQuiet);
		Item.Scale = FVector2D(S, S);
		Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.95f));
		Canvas->DrawItem(Item);
	}
}

float ALLKDebugHUD::DrawProgressBar(
	const FVector2D& At, float Width, float Fraction, const FLinearColor& Colour) const
{
	if (Canvas == nullptr)
	{
		return 0.f;
	}

	const float Scale = GetUIScale();
	const float Height = 7.f * Scale;
	const float Clamped = FMath::Clamp(Fraction, 0.f, 1.f);

	// Track first, then fill. The empty track is what makes a part-filled bar
	// legible as "part way" rather than as a short line.
	FCanvasTileItem Track(At, FVector2D(Width, Height), FLinearColor(0.f, 0.f, 0.f, 0.55f));
	Track.BlendMode = SE_BLEND_AlphaBlend;
	Canvas->DrawItem(Track);

	if (Clamped > 0.f)
	{
		FCanvasTileItem Fill(At, FVector2D(Width * Clamped, Height), Colour);
		Fill.BlendMode = SE_BLEND_AlphaBlend;
		Canvas->DrawItem(Fill);
	}

	return Height;
}

void ALLKDebugHUD::DrawCasualtyPrompt(const ULLKVitalsComponent* Casualty)
{
	const AActor* Body = Casualty != nullptr ? Casualty->GetOwner() : nullptr;
	if (Body == nullptr || Canvas == nullptr)
	{
		return;
	}

	const FVector Screen = Canvas->Project(Body->GetActorLocation());
	if (Screen.Z <= 0.f)
	{
		return;
	}

	const float Scale = GetUIScale();
	const float Half = BracketHalfSize * Scale * 1.6f;
	const FVector2D Centre(Screen.X, Screen.Y);

	// Red, and wider than a relic's bracket. A person is not a pickup, and
	// the thing a crew must find fastest should not look like a brooch.
	const bool bHolding = Casualty->IsBeingRevived();
	const FLinearColor Colour = bHolding ? ColourCause : ColourUrgent;

	const float Arm = Half * 0.45f;
	const float Thickness = FMath::Max(2.f, 3.f * Scale);
	const TArray<FVector2D> Corners = {
		FVector2D(-Half, -Half), FVector2D(Half, -Half),
		FVector2D(-Half, Half), FVector2D(Half, Half)};
	for (const FVector2D& Corner : Corners)
	{
		const FVector2D CornerAt = Centre + Corner;
		const float SignX = Corner.X < 0.f ? 1.f : -1.f;
		const float SignY = Corner.Y < 0.f ? 1.f : -1.f;
		Canvas->K2_DrawLine(CornerAt, CornerAt + FVector2D(Arm * SignX, 0.f), Thickness, Colour);
		Canvas->K2_DrawLine(CornerAt, CornerAt + FVector2D(0.f, Arm * SignY), Thickness, Colour);
	}

	UFont* Font = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
	if (Font == nullptr)
	{
		return;
	}

	// "hold E" rather than "[E]", because a tap does nothing here and a
	// prompt that does not say so teaches the wrong gesture.
	const FString Label = bHolding
		? FString::Printf(TEXT("reviving...  %.0f%%"), Casualty->GetReviveProgress() * 100.f)
		: FString(TEXT("hold [E]  -  revive"));

	float TextWidth = 0.f;
	float TextHeight = 0.f;
	const float LabelScale = 1.2f * Scale;
	Canvas->TextSize(Font, Label, TextWidth, TextHeight, LabelScale, LabelScale);

	const float LabelY = Centre.Y + Half + 6.f * Scale;
	FCanvasTextItem Item(FVector2D(Centre.X - TextWidth * 0.5f, LabelY),
		FText::FromString(Label), Font, Colour);
	Item.Scale = FVector2D(LabelScale, LabelScale);
	Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.9f));
	Canvas->DrawItem(Item);

	// The bar only while a hold is live. Drawn under the label and the same
	// width as the bracket, so progress reads as belonging to that body and
	// not to the screen.
	if (bHolding)
	{
		DrawProgressBar(
			FVector2D(Centre.X - Half, LabelY + TextHeight * LabelScale + 4.f * Scale),
			Half * 2.f, Casualty->GetReviveProgress(), ColourCause);
	}
}

void ALLKDebugHUD::DrawDownedPanel(const ULLKVitalsComponent* Own)
{
	if (Own == nullptr || Canvas == nullptr)
	{
		return;
	}

	UFont* Font = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
	if (Font == nullptr)
	{
		return;
	}

	const float Scale = GetUIScale();
	const float CentreX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.62f;

	const auto Centred = [&](const FString& Text, const FLinearColor& Colour, float TextScale)
	{
		float W = 0.f;
		float H = 0.f;
		const float S = TextScale * Scale;
		Canvas->TextSize(Font, Text, W, H, S, S);
		FCanvasTextItem Item(FVector2D(CentreX - W * 0.5f, Y), FText::FromString(Text), Font, Colour);
		Item.Scale = FVector2D(S, S);
		Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.9f));
		Canvas->DrawItem(Item);
		Y += H * S + 6.f * Scale;
	};

	Centred(TEXT("CONTAINED"), ColourUrgent, 1.8f);

	// Being rescued: say so and show how far, so a downed player knows to
	// stay put rather than wondering whether anything is happening.
	if (Own->IsBeingRevived())
	{
		Centred(FString::Printf(TEXT("someone is getting you up  -  %.0f%%"),
			Own->GetReviveProgress() * 100.f), ColourCause, 1.1f);
		DrawProgressBar(FVector2D(CentreX - 150.f * Scale, Y), 300.f * Scale,
			Own->GetReviveProgress(), ColourCause);
		return;
	}

	// **The indefinite wait, stated.** `bleedOutSeconds` is 0, so a downed
	// keeper waits forever - which is correct and reads as a hang if nobody
	// says so. Telling them they are safe but stuck is the difference between
	// a rule and a bug.
	if (Own->IsAnchorReviveAvailable())
	{
		Centred(FString::Printf(TEXT("hold [E]  -  anchor revive  (%d left)"),
			Own->GetAnchorChargesLeft()), ColourCause, 1.2f);
		Centred(TEXT("it will cost the night"), ColourQuiet, 1.0f);
	}
	else
	{
		Centred(TEXT("wait for the crew"), ColourQuiet, 1.2f);
		Centred(TEXT("you keep what you are carrying, and there is no timer"),
			ColourQuiet, 1.0f);
	}
}

void ALLKDebugHUD::DrawWorldPrompt()
{
	// Named RunWorld because a `FVector World` position already lives further
	// down this function - the collision is the kind of thing that compiles
	// into something subtly wrong in a language with more coercion.
	const UWorld* RunWorld = GetWorld();
	if (Canvas == nullptr || RunWorld == nullptr)
	{
		return;
	}

	const APawn* Keeper = GetOwningPawn();
	const ALLKCharacter* Character = Cast<ALLKCharacter>(Keeper);
	if (Character == nullptr)
	{
		return;
	}

	// **The same focus the take key uses.** Not a second search - the prompt
	// describing one relic while E lifts another is the bug this reads from a
	// single source to avoid.
	const ULLKInteractorComponent* Interactor = Character->GetInteractor();
	if (Interactor == nullptr)
	{
		return;
	}

	// **The run being over outranks everything.** Chris's words, and they are
	// the whole consequence in two lines: the night keeps what you carried.
	// Drawn before the downed panel, because a contained crew is no longer
	// waiting for anybody and telling them to is worse than telling them
	// nothing.
	const ALLKGameState* RunGameState = RunWorld->GetGameState<ALLKGameState>();
	if (RunGameState != nullptr && RunGameState->HasCrewWiped())
	{
		UFont* Font = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
		if (Font == nullptr)
		{
			return;
		}

		const float Scale = GetUIScale();
		const float CentreX = Canvas->SizeX * 0.5f;
		float Y = Canvas->SizeY * 0.42f;

		const auto Centred = [&](const FString& Text, const FLinearColor& Colour, float TextScale)
		{
			float W = 0.f;
			float H = 0.f;
			const float S = TextScale * Scale;
			Canvas->TextSize(Font, Text, W, H, S, S);
			FCanvasTextItem Item(FVector2D(CentreX - W * 0.5f, Y), FText::FromString(Text), Font, Colour);
			Item.Scale = FVector2D(S, S);
			Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.95f));
			Canvas->DrawItem(Item);
			Y += H * S + 10.f * Scale;
		};

		Centred(TEXT("CREW CONTAINED"), ColourUrgent, 2.4f);
		Centred(TEXT("The night keeps what you carried."), ColourQuiet, 1.3f);
		return;
	}

	// **And the half that did not exist until 2026-11-03.** The losing end of
	// the night got a full-screen panel that outranks everything; the winning
	// end got nothing at all, so a player who did the whole loop was told
	// only by a small readout that said SECURED and then fell silent.
	//
	// That was the same asymmetry `8307a86` fixed one layer down, where a
	// wipe resolved the run and securing the haul changed no state. This is
	// the part the player actually sees.
	//
	// The second line is `GAME-THESIS.md`'s own words. Its stated emotional
	// promise is "We got something home", and the one moment that promise is
	// kept is this one - so the screen says it rather than paraphrasing it.
	if (RunGameState != nullptr && RunGameState->HasRunSecured())
	{
		UFont* Font = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
		if (Font == nullptr)
		{
			return;
		}

		const float Scale = GetUIScale();
		const float CentreX = Canvas->SizeX * 0.5f;
		float Y = Canvas->SizeY * 0.42f;

		const auto CentredWin = [&](const FString& Text, const FLinearColor& Colour, float TextScale)
		{
			float W = 0.f;
			float H = 0.f;
			const float S = TextScale * Scale;
			Canvas->TextSize(Font, Text, W, H, S, S);
			FCanvasTextItem Item(FVector2D(CentreX - W * 0.5f, Y), FText::FromString(Text), Font, Colour);
			Item.Scale = FVector2D(S, S);
			Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.95f));
			Canvas->DrawItem(Item);
			Y += H * S + 10.f * Scale;
		};

		CentredWin(TEXT("HAUL IS OUT"), ColourHeading, 2.4f);
		CentredWin(TEXT("You got something home."), ColourQuiet, 1.3f);
		return;
	}

	// **This keeper's own condition first.** A player on the floor needs to
	// know what they can do about it before they need to know about a shelf.
	const ULLKVitalsComponent* Own = Character->GetVitals();
	if (Own != nullptr && Own->IsDowned())
	{
		DrawDownedPanel(Own);
		return;
	}

	// A body outranks loot. The interactor already resolves it that way, so
	// the prompt and the key agree without a second rule here.
	if (const ULLKVitalsComponent* Casualty = Interactor->GetFocusedCasualty())
	{
		DrawCasualtyPrompt(Casualty);
		return;
	}

	const ULLKRelicComponent* Relic = Interactor->GetFocused();
	if (Relic == nullptr || Relic->IsTaken())
	{
		return;
	}

	const AActor* RelicActor = Relic->GetOwner();
	if (RelicActor == nullptr)
	{
		return;
	}

	const FVector World = RelicActor->GetActorLocation();
	const FVector Screen = Canvas->Project(World);
	if (Screen.Z <= 0.f)
	{
		// Behind the camera. Project still returns a point, and drawing it
		// would put a marker on screen for something at the keeper's back.
		return;
	}

	const float Scale = GetUIScale();
	const float Half = BracketHalfSize * Scale;
	const FVector2D Centre(Screen.X, Screen.Y);

	// Colour carries the state, so the marker answers "can I have it" before
	// any text is read: warm when it is yours to take, red when the light is
	// what is missing, grey when you have not read it yet.
	FLinearColor Colour = ColourQuiet;
	FString Label;
	switch (Relic->GetInteractBlock())
	{
	case ELLKRelicInteractBlock::None:
		Colour = ColourCause;
		Label = FString::Printf(TEXT("[Hold E]  Inspect%s  -  %s"),
			Relic->IsCurrentlyLit() ? TEXT("") : TEXT(" slowly"),
			*Relic->GetDisplayName().ToString());
		break;
	case ELLKRelicInteractBlock::NeedsLight:
		Colour = ColourUrgent;
		Label = FString::Printf(TEXT("%s  -  needs light"), *Relic->GetDisplayName().ToString());
		break;
	case ELLKRelicInteractBlock::Unidentified:
		Colour = ColourQuiet;
		Label = FString::Printf(TEXT("[Hold E]  Search slowly  -  %s"), *Relic->GetDisplayName().ToString());
		break;
	}

	// Four corner brackets rather than a full box: a closed rectangle reads
	// as UI chrome sitting in front of the world, and corners read as
	// something the world is doing. Cheap stand-in for the outline material.
	const float Arm = Half * 0.45f;
	const float Thickness = FMath::Max(1.f, 2.f * Scale);
	const TArray<FVector2D> Corners = {
		FVector2D(-Half, -Half), FVector2D(Half, -Half),
		FVector2D(-Half, Half), FVector2D(Half, Half)};

	for (const FVector2D& Corner : Corners)
	{
		const FVector2D At = Centre + Corner;
		const float SignX = Corner.X < 0.f ? 1.f : -1.f;
		const float SignY = Corner.Y < 0.f ? 1.f : -1.f;
		Canvas->K2_DrawLine(At, At + FVector2D(Arm * SignX, 0.f), Thickness, Colour);
		Canvas->K2_DrawLine(At, At + FVector2D(0.f, Arm * SignY), Thickness, Colour);
	}

	// The label under the bracket, centred, with the same shadow the panel
	// uses so it survives a lit pillar behind it.
	UFont* Font = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
	if (Font == nullptr)
	{
		return;
	}

	float TextWidth = 0.f;
	float TextHeight = 0.f;
	const float LabelScale = 1.1f * Scale;
	Canvas->TextSize(Font, Label, TextWidth, TextHeight, LabelScale, LabelScale);

	FCanvasTextItem Item(
		FVector2D(Centre.X - TextWidth * 0.5f, Centre.Y + Half + 6.f * Scale),
		FText::FromString(Label), Font, Colour);
	Item.Scale = FVector2D(LabelScale, LabelScale);
	Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.9f));
	Canvas->DrawItem(Item);

	if (Interactor->GetTakingRelic() == Relic)
	{
		const float Progress = Interactor->GetRelicTakeProgress();
		const FString Status = FString::Printf(TEXT("INSPECTING  %.0f%%"), Progress * 100.f);
		float StatusWidth = 0.f;
		float StatusHeight = 0.f;
		Canvas->TextSize(Font, Status, StatusWidth, StatusHeight, LabelScale, LabelScale);
		FCanvasTextItem StatusItem(
			FVector2D(Centre.X - StatusWidth * 0.5f, Centre.Y + Half + TextHeight + 12.f * Scale),
			FText::FromString(Status), Font, ColourCause);
		StatusItem.Scale = FVector2D(LabelScale, LabelScale);
		StatusItem.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.9f));
		Canvas->DrawItem(StatusItem);
		DrawProgressBar(
			FVector2D(Centre.X - Half, Centre.Y + Half + TextHeight + StatusHeight + 18.f * Scale),
			Half * 2.f, Progress, ColourCause);
	}
}

void ALLKDebugHUD::DrawBurdenLine(float& CursorY)
{
	const ALLKCharacter* Character = Cast<ALLKCharacter>(GetOwningPawn());
	const ULLKInteractorComponent* Interactor =
		Character != nullptr ? Character->GetInteractor() : nullptr;
	if (Interactor == nullptr)
	{
		return;
	}

	const int32 Carried = Interactor->GetCarriedCount();
	if (Carried <= 0)
	{
		return;
	}

	const FLLKBurdenRules& Rules = Character->GetBurdenRules();
	const float Penalty = LLKBurden::PenaltyFraction(Carried, Rules);
	const int32 CapAt = LLKBurden::PointsUntilCapped(Rules);

	// The three things that line can say, in the order a player needs them:
	// what you have, what it costs, and whether the next one is free.
	FString Line = FString::Printf(TEXT("HAUL  carrying %d"), Carried);
	if (Penalty > 0.f)
	{
		Line += FString::Printf(TEXT("  -  %.0f%% slower"), Penalty * 100.f);
	}
	else
	{
		// Burden is authored off, which every run before M3 was playing. Said
		// rather than left looking like a zero somebody forgot to fill in.
		Line += TEXT("  -  no burden authored");
	}

	if (CapAt > 0 && Carried >= CapAt)
	{
		Line += TEXT("  -  the next one is free");
	}

	DrawLine(Line, Penalty > 0.f ? ColourUrgent : ColourQuiet, CursorY, 1.1f);
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

	// And the affordance. Illumination changes the speed of a held inspection,
	// rather than making an identified object abruptly untakeable.
	FString Affordance;
	FLinearColor AffordanceColour = ColourQuiet;
	switch (Nearest->GetInteractBlock())
	{
	case ELLKRelicInteractBlock::None:
		Affordance = FString::Printf(TEXT("  hold to inspect%s%s"),
			Nearest->IsCurrentlyLit() ? TEXT("") : TEXT(" slowly in dark"),
			Nearest->GetReadableValue() >= 0
				? *FString::Printf(TEXT("  -  worth %d"), Nearest->GetReadableValue()) : TEXT(""));
		AffordanceColour = ColourCause;
		break;
	case ELLKRelicInteractBlock::NeedsLight:
		Affordance = TEXT("  needs light");
		AffordanceColour = ColourUrgent;
		break;
	case ELLKRelicInteractBlock::Unidentified:
		Affordance = TEXT("  hold to search slowly in dark");
		break;
	}
	DrawLine(Affordance, AffordanceColour, CursorY);
}

void ALLKDebugHUD::DrawStormPanel(
	const ULLKZonePressureSubsystem* Storm, const FLLKZonePressureSnapshot& Snapshot, float& CursorY)
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

	// ---- which stage the night is in, M2.3 ----------------------------
	//
	// **Computed here, not replicated.** The stage is a function of the
	// charge and the shared rules, so a client derives its own from the
	// snapshot it already has and cannot disagree with the server about it.
	// Replicating a stage field would put a second copy of the same fact on
	// the wire, and two copies of one fact is a badge that can contradict the
	// countdown printed underneath it.
	const FLLKNightStage Stage =
		ULLKZonePressureSubsystem::StageFrom(Storm->GetRules(), Snapshot.Pressure, Snapshot.bWardenConverged);

	// The authored sentence, not a word of ours. `zone-pressure.json` carries
	// an `effect` per threshold and M2.6 is judged on whether a player can
	// name what changed - so the words they read are the words Chris tunes.
	const FString Effect = Storm->GetThresholdEffect(Stage.Id);

	if (Stage.bWardenConverged)
	{
		DrawLine(TEXT("THE WARDEN IS HERE"), ColourUrgent, CursorY, 1.3f);
	}
	else if (Stage.IsCalm())
	{
		DrawLine(
			FString::Printf(TEXT("NIGHT  calm  (0 of %d)"), Stage.Count),
			ColourQuiet, CursorY, 1.3f);
	}
	else
	{
		DrawLine(
			FString::Printf(TEXT("NIGHT  %s  (%d of %d)"),
				*Stage.Id.ToString(), Stage.Index, Stage.Count),
			ColourUrgent, CursorY, 1.3f);
	}

	if (!Effect.IsEmpty())
	{
		DrawLine(FString::Printf(TEXT("  %s"), *Effect), ColourCause, CursorY);
	}

	CursorY += LineHeight * 0.4f;

	// ---- the consequence, first and largest ---------------------------
	//
	// Time, not charge. This is the line the whole readout exists for: it is
	// what changes the moment the lantern comes on, and it is what makes the
	// lantern a decision instead of a light.
	if (Snapshot.bHasNextEscalation)
	{
		const bool bWardenNext = Snapshot.NextEscalationId == TEXT("WardenConverges");

		// Projected here rather than replicated. Both machines run this on
		// the same ULLKZonePressureData, so the countdown agrees without the
		// server sending one - and it keeps ticking smoothly between the
		// 10Hz snapshots instead of stepping.
		const float Seconds = Storm->SecondsUntilSnapshotEscalation(Snapshot);

		DrawLine(
			FString::Printf(TEXT("%s in %s"),
				*Snapshot.NextEscalationId.ToString(), *FormatSeconds(Seconds)),
			bWardenNext ? ColourUrgent : ColourHeading, CursorY, 1.6f);

		// **The counterfactual used to live here and row 72 moved it.** This
		// line read "2:24 dark" against "0:58 lit", which was row 5 with no
		// tutorial text - and it was only true while the lantern accelerated
		// the clock. It does not any more, so the same sentence would now be
		// a lie told in two identical numbers.
		//
		// The cost did not disappear; it moved to where it can be seen. It is
		// printed below as the distance you can be noticed from, which is the
		// same teaching device aimed at the thing that actually changed.
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
	// The rate is one number now and it is the same for everybody, so the
	// interesting line is no longer "what is light costing the clock" but
	// "what is light costing you". Read off the authored Sweeper rather than
	// restated: 38 studs lit against 12 dark is the largest consequence of
	// the lantern in the data, and it is the roster's first real consumer.
	float NoticedLit = 0.f;
	float NoticedDark = 0.f;
	const UWorld* PanelWorld = Storm->GetWorld();
	if (const ULLKEnemyRosterSubsystem* Roster = PanelWorld != nullptr
			? PanelWorld->GetSubsystem<ULLKEnemyRosterSubsystem>() : nullptr)
	{
		if (const FLLKEnemyDef* Sweeper = Roster->FindFamily(ELLKEnemyFamily::Sweepers))
		{
			// The two ends of the range, asked for by detectability rather
			// than by a brightness: 1 is a fully open lantern and 0 is a dark
			// crew. Row 80 made everything between them reachable too, so this
			// readout now shows the ends of a span and not two states.
			NoticedLit = LLKPerception::NoticeRangeStuds(*Sweeper, 1.f);
			NoticedDark = LLKPerception::NoticeRangeStuds(*Sweeper, 0.f);
		}
	}

	DrawLine(
		FString::Printf(TEXT("lanterns lit  %d       rate  %.2f/s       (light does not change it)"),
			Lit,
			Storm->GetRules() != nullptr ? Storm->GetRules()->BaselineRatePerSecond : 0.f),
		Lit > 0 ? ColourCause : ColourQuiet, CursorY);

	if (NoticedLit > 0.f && NoticedDark > 0.f)
	{
		DrawLine(
			FString::Printf(TEXT("  a Sweeper notices you from %.0f studs lit, %.0f dark"),
				NoticedLit, NoticedDark),
			Lit > 0 ? ColourCause : ColourQuiet, CursorY);
	}

	// ---- the two meters, last, because they are the trivia -------------
	DrawLine(
		FString::Printf(TEXT("charge  %.0f%s       elapsed  %s"),
			Snapshot.Pressure,
			Snapshot.bHasNextEscalation
				? *FString::Printf(TEXT(" / %.0f"), Snapshot.NextTargetPressure) : TEXT(""),
			*FormatSeconds(Snapshot.ElapsedSeconds)),
		ColourQuiet, CursorY);

	// Row 54 still gives the Warden its own arrival value; row 72 took away
	// its second meter. So this reads the one clock against the Warden's
	// threshold - the same arithmetic every other countdown on this panel
	// uses, which is the point of there being one clock.
	DrawLine(
		FString::Printf(TEXT("warden  arrives  %s%s"),
			*FormatSeconds(Storm->SecondsUntilWarden()),
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
				? TEXT("DA_ZonePressure") : TEXT("DECLARED FALLBACKS"),
			Storm->IsAccruing() ? TEXT("") : TEXT("  (server's, replicated)")),
		Snapshot.RulesSource == ELLKDataSource::Asset ? ColourQuiet : ColourUrgent, CursorY);
}
