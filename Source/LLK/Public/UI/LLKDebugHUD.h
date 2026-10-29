// Storm Charge, on screen. M1.7.
//
// **This is not the HUD.** The real one is M3 and Chris's, and it will be a
// widget with art. This is a debug readout drawn with `DrawText`: no UMG, no
// widget Blueprint, no `Content/` of any kind, deleted the day the real one
// lands.
//
// **Why it is worth building before the real one.** Chris, 2026-10-29: the
// HUD "turns the lantern from 'a cool light' into the game's actual risk
// decision." Right now a player can hold F and nothing visibly answers. The
// meter is rising 2.5x faster, which is the entire premise of the game, and
// it is invisible.
//
// **So the primary number here is TIME, not charge.** A meter climbing from
// 336 to 480 teaches nothing - it is two numbers with no stakes. "Patrols
// hunt in 2:24" teaches, and watching that figure jump to 0:58 the instant
// the lantern comes on teaches the whole of row 5 in one frame, without a
// line of tutorial text. Row 43 makes the store teach by being legible; this
// is the same principle applied to a number instead of a room.
//
// That is also why the lit-lantern count and the multiplier are shown beside
// it rather than instead of it: the projection is the consequence, and those
// two are the cause. A player who sees only the consequence learns to fear
// the meter. A player who sees both learns what they did.
//
// **It refuses to show a number it does not have.** The subsystem accrues on
// authority only, so a client holds an honest zero. Rendering that as a meter
// would be a readout that lies, which is worse than no readout - so on a
// client it says so instead.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LLKDebugHUD.generated.h"

class ULLKStormChargeSubsystem;

UCLASS()
class LLK_API ALLKDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALLKDebugHUD();

	virtual void DrawHUD() override;

	/** mm:ss, or a dash when there is nothing to count down to. */
	static FString FormatSeconds(float Seconds);

protected:
	/** One line, left-aligned, advancing a cursor. Keeps DrawHUD readable. */
	void DrawLine(const FString& Text, const FLinearColor& Colour, float& CursorY, float Scale = 1.f);

	void DrawStormPanel(ULLKStormChargeSubsystem* Storm, float& CursorY);

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug")
	FVector2D PanelOrigin = FVector2D(24.f, 24.f);

	UPROPERTY(EditDefaultsOnly, Category = "LLK|Debug")
	float LineHeight = 20.f;
};
