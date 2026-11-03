#include "Characters/LLKStumbleCameraShake.h"

ULLKStumbleCameraShake::ULLKStumbleCameraShake()
{
	// A jolt, not a stun.  The short blend avoids a hard camera cut; the shake
	// ends long before the 1.2-second movement penalty, so the player feels the
	// contact without being visually impaired for the escape window.
	OscillationDuration = 0.18f;
	OscillationBlendInTime = 0.015f;
	OscillationBlendOutTime = 0.09f;
	bSingleInstance = true;

	RotOscillation.Pitch.Amplitude = 1.15f;
	RotOscillation.Pitch.Frequency = 17.f;
	RotOscillation.Pitch.InitialOffset = EOO_OffsetZero;

	RotOscillation.Roll.Amplitude = 0.8f;
	RotOscillation.Roll.Frequency = 20.f;
	RotOscillation.Roll.InitialOffset = EOO_OffsetZero;

	LocOscillation.Z.Amplitude = 0.35f;
	LocOscillation.Z.Frequency = 18.f;
	LocOscillation.Z.InitialOffset = EOO_OffsetZero;
}
