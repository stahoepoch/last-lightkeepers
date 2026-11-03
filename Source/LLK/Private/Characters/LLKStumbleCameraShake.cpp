#include "Characters/LLKStumbleCameraShake.h"

ULLKStumbleCameraShake::ULLKStumbleCameraShake()
{
	// A jolt, not a stun.  The short blend avoids a hard camera cut; the shake
	// ends long before the 1.2-second movement penalty, so the player feels the
	// contact without being visually impaired for the escape window.
	// **Raised 2026-11-03, deliberately toward the perceptible end.**
	//
	// The first pass was 1.15 degrees of pitch at 17 Hz, and the failure it
	// was built to correct was Chris feeling nothing at all. 1.15 is footstep
	// territory - a hit response is usually two to five - and 17 to 20 Hz
	// reads as machine *vibration* rather than as a lurch. A trip is not a
	// buzz: it is fewer, bigger oscillations.
	//
	// So amplitude roughly trebles and frequency drops by a third, and the
	// duration goes to 0.26s so that at 11 Hz there are about three clean
	// swings instead of three cycles of a rattle. Roll carries most of the
	// "tripped" read, which is why it moves the furthest proportionally -
	// but it stays under three degrees because roll is the axis that makes
	// people queasy, and this fires every 2.5 seconds under sustained
	// contact.
	//
	// **Overshooting on purpose.** We already under-shot once and the cost
	// was a mechanic nobody could detect. Coming down from too much is one
	// playtest; creeping up from too little is several.
	OscillationDuration = 0.26f;
	OscillationBlendInTime = 0.015f;
	OscillationBlendOutTime = 0.12f;
	bSingleInstance = true;

	RotOscillation.Pitch.Amplitude = 3.2f;
	RotOscillation.Pitch.Frequency = 11.f;
	RotOscillation.Pitch.InitialOffset = EOO_OffsetZero;

	RotOscillation.Roll.Amplitude = 2.4f;
	RotOscillation.Roll.Frequency = 9.f;
	RotOscillation.Roll.InitialOffset = EOO_OffsetZero;

	LocOscillation.Z.Amplitude = 1.4f;
	LocOscillation.Z.Frequency = 12.f;
	LocOscillation.Z.InitialOffset = EOO_OffsetZero;
}
