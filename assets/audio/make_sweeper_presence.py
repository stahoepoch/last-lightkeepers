"""The Sweeper's motor, as a seamless loop — synthesised from scratch.

Run:  python assets/audio/make_sweeper_presence.py

Why this exists
---------------
`ALLKSweeper` played one-shot cues at a fixed location and was then **silent
while it moved**. Measured on the opening map, a Sweeper that notices the
keeper walks 11962 cm of path — 71.2 s at 168 cm/s — and in all that time it
makes no sound at all. `DECISIONS.md` row 16 requires every enemy to announce
itself with sound before sight, and one telegraph does satisfy the letter of
that. It does not let a player hear a machine *coming*, which is the whole of
choosing between hide, detour and sprint (`ROADMAP.md` slice 3).

So this is not a cue. It is a presence: a continuous loop attached to the
Sweeper, spatialised and attenuated, that the runtime varies by state. One
asset, because volume and pitch at playback are enough to tell a patrolling
machine from a pursuing one, and two assets would be two things to keep in
agreement.

House rules, same as `make_cues.py`
-----------------------------------
Everything here is arithmetic. Nothing is sampled, downloaded or borrowed, so
`docs/ASSETS.md`'s "everything we ship, we own" holds. The master is this
script, not the .wav — text, diffable, re-renderable at any rate. Stdlib only.

Seamlessness is a requirement, not a nicety
-------------------------------------------
A loop with a discontinuity at the join clicks once per period, and a click
every four seconds is a worse tell than silence. Two things guarantee the
join here:

  1. Every periodic layer completes a whole number of cycles in the loop
     length. At 4.0 s: the 58 Hz motor runs 232 cycles, the 7.25 Hz rotor
     modulation 29, the castor 5. No layer is cut mid-cycle.
  2. The one aperiodic layer (rotor noise) is rendered LONGER than the loop
     and wrap-crossfaded onto itself, so its own tail becomes its own head.

`verify_loop()` at the bottom asserts the join rather than trusting the
reasoning, and fails the render if it drifts.
"""

import math
import random
import struct
import wave
from pathlib import Path

RATE = 44100
OUT = Path(__file__).parent

LOOP_SECONDS = 4.0

# Deterministic, so a rebuild is never a silent change.
random.seed(1889)

MOTOR_HZ = 58.0      # 232 cycles in 4.0 s
ROTOR_AM_HZ = 7.25   # 29 cycles
CASTOR_HZ = 1.25     # 5 ticks
WRAP_FADE = 0.25     # seconds of wrap-crossfade on the noise layer


def _samples(seconds):
    return int(round(RATE * seconds))


def motor(n):
    """The drive: a fundamental with the odd harmonics a loaded motor makes.

    Weighted toward the low end so it survives attenuation over distance as a
    felt rumble rather than a thin buzz. The slight inharmonicity on the
    fourth partial keeps it from reading as a test tone.
    """
    partials = ((1.0, 1.00), (2.0, 0.34), (3.0, 0.17), (4.07, 0.09), (6.0, 0.05))
    out = [0.0] * n
    for ratio, weight in partials:
        step = 2.0 * math.pi * MOTOR_HZ * ratio / RATE
        for i in range(n):
            out[i] += weight * math.sin(step * i)
    peak = max(abs(v) for v in out) or 1.0
    return [v / peak for v in out]


def rotor(n):
    """Brush head: band-limited noise, amplitude-modulated at the rotor rate.

    Rendered long and wrap-crossfaded so the noise has no join. The modulation
    is applied AFTER the wrap, on the final length, so it stays periodic.
    """
    fade = _samples(WRAP_FADE)
    raw = [random.uniform(-1.0, 1.0) for _ in range(n + fade)]

    # Two-pole smoothing, twice, to take the top off. White noise reads as
    # tape hiss; a brush head is a band, not a hiss.
    for _ in range(2):
        a = 0.0
        for i in range(len(raw)):
            a += 0.12 * (raw[i] - a)
            raw[i] = a

    # Wrap-crossfade: the tail becomes the head, so sample n-1 meets sample 0
    # across material that was already continuous.
    wrapped = raw[:n]
    for i in range(fade):
        t = i / fade
        wrapped[i] = wrapped[i] * t + raw[n + i] * (1.0 - t)

    peak = max(abs(v) for v in wrapped) or 1.0
    wrapped = [v / peak for v in wrapped]

    step = 2.0 * math.pi * ROTOR_AM_HZ / RATE
    return [v * (0.55 + 0.45 * math.sin(step * i)) for i, v in enumerate(wrapped)]


def castor(n):
    """A wheel that wants oil: one short decaying squeak per revolution.

    This is the layer that makes the machine read as *travelling* rather than
    running on the spot, and it is the part a player can count to judge
    distance changes by ear.
    """
    out = [0.0] * n
    period = _samples(1.0 / CASTOR_HZ)
    length = _samples(0.11)
    for tick in range(int(round(n / period))):
        start = tick * period
        # Alternating pitch: a wheel does not squeak identically every turn.
        freq = 1180.0 if tick % 2 == 0 else 1310.0
        step = 2.0 * math.pi * freq / RATE
        for i in range(length):
            where = start + i
            if where >= n:
                break  # Never writes past the loop, so the join stays clean.
            env = math.exp(-9.0 * i / length) * (1.0 - math.exp(-260.0 * i / length))
            out[where] += env * math.sin(step * i) * 0.5
    return out


def mix(n):
    m = motor(n)
    r = rotor(n)
    c = castor(n)
    out = [0.62 * m[i] + 0.26 * r[i] + 0.20 * c[i] for i in range(n)]
    peak = max(abs(v) for v in out) or 1.0
    # Leave headroom: the runtime sets volume per state and a loop rendered at
    # full scale gives it nowhere to go up.
    return [0.72 * v / peak for v in out]


def verify_loop(samples, name):
    """Assert the join rather than trusting the arithmetic above.

    The step across the join must be no larger than the steps the waveform
    already takes inside itself. If a layer is ever cut mid-cycle this fails
    at render time, which is the only time it is cheap to fix.
    """
    join = abs(samples[0] - samples[-1])
    inside = max(abs(samples[i + 1] - samples[i]) for i in range(len(samples) - 1))
    if join > inside:
        raise AssertionError(
            f"{name}: the loop join steps {join:.5f}, larger than the largest "
            f"step inside the waveform ({inside:.5f}). A layer is being cut "
            f"mid-cycle; check that every rate divides LOOP_SECONDS."
        )
    return join, inside


def write_wav(path, samples):
    with wave.open(str(path), "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(RATE)
        f.writeframes(b"".join(
            struct.pack("<h", int(max(-1.0, min(1.0, v)) * 32767)) for v in samples))


def main():
    n = _samples(LOOP_SECONDS)
    samples = mix(n)
    join, inside = verify_loop(samples, "sweeper-presence-v1")
    path = OUT / "sweeper-presence-v1.wav"
    write_wav(path, samples)
    print(f"wrote {path.name}: {LOOP_SECONDS:.1f}s, {n} frames at {RATE} Hz")
    print(f"loop join step {join:.6f} vs largest internal step {inside:.6f} - seamless")


if __name__ == "__main__":
    main()
