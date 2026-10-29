"""Enemy telegraph cues for Last Lightkeepers — synthesised from scratch.

Run:  python assets/audio/make_cues.py

Every sound this writes is generated here from arithmetic. Nothing is sampled,
nothing is downloaded, nothing is borrowed. That matters for two reasons:

  1. Licensing. docs/ASSETS.md requires that everything we ship, we own. A
     Marketplace rbxassetid cannot port to a desktop build because the file
     lives on Roblox's servers under someone else's terms. These are ours.

  2. Portability. The master is not the .wav - it is this script. Text, in the
     repo, diffable, and re-renderable at any sample rate or length when the
     game is rebuilt in another engine. A .wav is just today's render of it.

The brief is an accessibility requirement from the handoff, not polish:
telegraph every enemy with sound BEFORE sight, and make the cues tellable apart
by ear alone. Pitch-shifting one sample does not satisfy that, which is why
each cue below is built from a different mechanism rather than a different
knob on the same one.

Stdlib only - no numpy, no install step.
"""

import math
import random
import struct
import wave
from pathlib import Path

RATE = 44100
OUT = Path(__file__).parent

# Deterministic: the same script always renders the same files, so a rebuild is
# never a silent change.
random.seed(1889)


# ---------------------------------------------------------------- primitives

def silence(seconds):
    return [0.0] * int(RATE * seconds)


def sine(freq, seconds, phase=0.0):
    n = int(RATE * seconds)
    step = 2 * math.pi * freq / RATE
    return [math.sin(phase + step * i) for i in range(n)]


def sweep(f0, f1, seconds, curve=1.0):
    """Frequency glide. curve > 1 holds low then rushes up at the end."""
    n = int(RATE * seconds)
    out = []
    phase = 0.0
    for i in range(n):
        t = (i / n) ** curve
        freq = f0 + (f1 - f0) * t
        phase += 2 * math.pi * freq / RATE
        out.append(math.sin(phase))
    return out


def noise(seconds):
    return [random.uniform(-1.0, 1.0) for _ in range(int(RATE * seconds))]


def lowpass(buf, cutoff):
    """One-pole filter. Crude, and exactly right for this job."""
    out = []
    a = math.exp(-2 * math.pi * cutoff / RATE)
    prev = 0.0
    for s in buf:
        prev = s * (1 - a) + prev * a
        out.append(prev)
    return out


def highpass(buf, cutoff):
    low = lowpass(buf, cutoff)
    return [s - l for s, l in zip(buf, low)]


def env(buf, attack, decay, hold=0.0):
    """Attack / hold / decay in seconds, applied over the whole buffer."""
    n = len(buf)
    a = max(1, int(RATE * attack))
    h = int(RATE * hold)
    d = max(1, int(RATE * decay))
    out = []
    for i, s in enumerate(buf):
        if i < a:
            g = i / a
        elif i < a + h:
            g = 1.0
        else:
            k = (i - a - h) / d
            g = max(0.0, 1.0 - k)
        out.append(s * g)
    return out


def mix(*buffers):
    n = max(len(b) for b in buffers)
    out = [0.0] * n
    for b in buffers:
        for i, s in enumerate(b):
            out[i] += s
    return out


def at(base, offset_seconds, buf):
    """Place buf into base starting at an offset, extending base if needed."""
    start = int(RATE * offset_seconds)
    need = start + len(buf)
    if len(base) < need:
        base = base + [0.0] * (need - len(base))
    for i, s in enumerate(buf):
        base[start + i] += s
    return base


def normalise(buf, peak=0.82):
    hi = max(abs(s) for s in buf) or 1.0
    k = peak / hi
    return [s * k for s in buf]


def write(name, buf):
    buf = normalise(buf)
    path = OUT / name
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        frames = b"".join(
            struct.pack("<h", int(max(-1.0, min(1.0, s)) * 32767)) for s in buf
        )
        w.writeframes(frames)
    print(f"  {name}  {len(buf) / RATE:.2f}s")


# -------------------------------------------------------------------- cues

def sweeper():
    """Many small things moving, low and scuttling.

    Mechanism: dozens of tiny filtered-noise grains at irregular intervals.
    The identity is RATE OF EVENTS - a crowd, not a thing.
    """
    out = silence(1.1)
    t = 0.0
    while t < 1.0:
        grain = env(lowpass(noise(0.035), random.uniform(700, 1800)), 0.002, 0.03)
        pan = random.uniform(0.35, 1.0)
        out = at(out, t, [s * pan for s in grain])
        t += random.uniform(0.018, 0.055)
    # A low floor so it reads as mass rather than static.
    out = mix(out, env(lowpass(noise(1.1), 180), 0.15, 0.5, 0.3))
    return out


def stocker():
    """Slow, heavy, one source.

    Mechanism: a single low impact plus a descending servo groan. The identity
    is WEIGHT and singularity - one big thing, taking its time.
    """
    thump = env(mix(sine(48, 0.9), [0.4 * s for s in sine(72, 0.9)]), 0.004, 0.85)
    body = env(lowpass(noise(0.25), 240), 0.002, 0.24)
    groan = env(sweep(190, 95, 1.3, curve=0.7), 0.08, 1.1)
    groan = [0.28 * s for s in groan]
    out = mix(thump, body, groan)
    return at(out, 0.0, [])


def glimmer():
    """A scan tone whose 2-second window is audible.

    This is the one that matters most. The handoff gives the player two seconds
    to go dark before a tag lands, and a single hit cannot convey "you have
    time, and it is running out".

    Mechanism: a tone rising across exactly 2.0s, with ticks that ACCELERATE -
    spaced by a geometric series so the gaps audibly close. You can hear how
    much window is left without looking at anything.
    """
    seconds = 2.0
    tone = env(sweep(420, 1180, seconds, curve=1.6), 0.05, 0.25, hold=seconds - 0.3)
    tone = [0.5 * s for s in tone]

    out = list(tone)
    # Ticks at closing intervals: slow, slow, quicker, quicker, urgent.
    #
    # Placed by formula rather than by accumulating a shrinking gap. A gap that
    # shrinks geometrically sums to a finite limit - 0.34/(1-0.74) is about
    # 1.31s - so a `while t < 1.96` loop over it never terminates. It spun for
    # several minutes before I caught it. This spans the full window by
    # construction and cannot run away.
    ticks = 9
    for i in range(ticks):
        t = seconds * ((i / ticks) ** 0.62)
        tick = env(highpass(noise(0.012), 2600), 0.0005, 0.011)
        out = at(out, t, [0.75 * s for s in tick])
    # The tag landing: a short, flat, final note. No ambiguity that it is over.
    out = at(out, seconds, env(sine(1480, 0.14), 0.002, 0.13))
    return out


def window_walker():
    """Stillness breaking.

    Mechanism: near-silence, then a dry crack, then a rigid scrape. The
    identity is the SILENCE FIRST - something that was furniture a moment ago.
    """
    out = silence(0.12)
    crack = env(highpass(noise(0.05), 1500), 0.0004, 0.05)
    out = at(out, 0.12, crack)
    # Joint turning over: a creak that is pitched, not noisy.
    creak = env(sweep(300, 215, 0.55, curve=0.5), 0.02, 0.5)
    creak = [0.45 * s for s in creak]
    out = at(out, 0.17, creak)
    # One weighted step, dulled.
    out = at(out, 0.62, env(lowpass(noise(0.2), 320), 0.004, 0.2))
    return out


def ficus():
    """The only thing in the building that is alive.

    Mechanism: dense foliage rustle that SWELLS and never resolves into
    separate events, plus a fibrous stem creak that drifts in pitch. The
    identity is ORGANIC MASS - no impact, no metal, no servo, nothing
    mechanical anywhere in it.

    That is the point rather than a flourish. `DECISIONS.md` row 47 makes
    every other enemy a machine finishing the last job it was given, and their
    cues are built from machine parts: a servo groan, a rigid joint, a horn.
    The Ficus was landscaping and it has grown (`docs/THE-NIGHT.md`). A player
    who can hear one thing in the store that is not a machine has learned a
    fact about the world by ear - which is how row 47 says it must be learned,
    by watching the building work, never from a tooltip.

    Three things separate it from the Sweeper, its only noise-based neighbour:
    the grains are roughly five times denser, so they fuse into texture rather
    than a countable crowd; they are bright instead of lowpassed; and the cue
    is one swell instead of a steady rate.

    Length is 1.2s because `data/enemies.json` says so. The shape is slow
    because the Ficus moves at 6, the slowest in the roster - the cue must not
    imply something that can rush you.
    """
    seconds = 1.2
    out = silence(seconds)

    # Foliage: many short bright grains, dense enough to merge. Leaves are
    # papery and high; the Sweeper's grains are dull and low, deliberately.
    t = 0.0
    while t < seconds - 0.06:
        grain = env(highpass(noise(0.012), 1100), 0.001, 0.011)
        out = at(out, t, [g * random.uniform(0.5, 1.0) for g in grain])
        t += random.uniform(0.003, 0.007)

    # One swell over the whole thing: all of it moves at once, then settles.
    # The attack is fast but not a transient - a crack is the Window Walker.
    out = env(out, 0.02, 0.62, hold=0.18)

    # Stem taking weight. Pitched, but inharmonic and drifting rather than the
    # clean glide of a servo or a hinge, so it never reads as a mechanism.
    creak = mix(
        env(sweep(132, 108, 0.75, curve=0.8), 0.12, 0.6),
        [0.42 * g for g in env(sweep(194, 171, 0.75, curve=1.3), 0.18, 0.55)],
    )
    out = at(out, 0.32, [0.33 * g for g in creak])

    # Soil and pot settling: low, soft, with no impact anywhere in it.
    out = mix(out, env(lowpass(noise(seconds), 150), 0.25, 0.55, 0.2))
    return out


def warden():
    """Unmistakable, and audible from much further than the rest.

    Mechanism: a deep two-tone horn with a slow beat between near-unison
    partials, plus a long tail. The identity is LENGTH and LOWNESS - nothing
    else in the game occupies this register or lasts this long.
    """
    seconds = 3.2
    # Two close fundamentals beating against each other: an unsettled interval.
    a = sine(41, seconds)
    b = sine(43.6, seconds)
    c = [0.35 * s for s in sine(82, seconds)]
    d = [0.18 * s for s in sine(123, seconds)]
    horn = env(mix(a, b, c, d), 0.55, 1.6, hold=1.0)

    # Tail: a few decaying echoes, so it reads as distance.
    out = list(horn)
    for i, (delay, gain) in enumerate(((0.38, 0.33), (0.77, 0.17), (1.2, 0.08))):
        out = at(out, delay, [gain * s for s in horn])
    return lowpass(out, 900)


def extraction_siren():
    """A public alarm, not an enemy voice.

    Mechanism: alternating rising/falling two-note sweeps over a mechanical
    low pulse. It has to travel a long way through the mall and be immediately
    distinct from a Glimmer's *single* rising scan.
    """
    out = silence(4.0)
    for start, rising in ((0.0, True), (1.0, False), (2.0, True), (3.0, False)):
        tone = sweep(610, 910, 0.82) if rising else sweep(910, 610, 0.82)
        tone = [0.48 * s for s in env(tone, 0.035, 0.12, hold=0.66)]
        out = at(out, start, tone)
    pulse = [0.16 * s for s in env(sine(72, 4.0), 0.01, 0.3, hold=3.5)]
    return mix(out, pulse)


# -------------------------------------------------------------------- main

CUES = {
    "sweeper-telegraph-v1.wav": sweeper,
    "stocker-telegraph-v1.wav": stocker,
    "glimmer-scan-v1.wav": glimmer,
    "window-walker-wake-v1.wav": window_walker,
    "ficus-wake-v1.wav": ficus,
    "warden-approach-v1.wav": warden,
    "extraction-siren-v1.wav": extraction_siren,
}


def main():
    print("Rendering enemy cues:")
    for name, fn in CUES.items():
        write(name, fn())
    print("\nAll original, and all mono - positional audio wants a point source,")
    print("not a stereo pair (docs/ASSETS.md). Import into Content/ and record")
    print("the rows in assets/MANIFEST.md before anything references them.")


if __name__ == "__main__":
    main()
