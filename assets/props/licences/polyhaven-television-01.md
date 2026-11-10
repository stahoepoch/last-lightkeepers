# Licence record — Poly Haven, Television 01

Kept per `docs/ASSET-INTAKE.md` step 2: the licence record lives here, outside
the cooked build. No `assets/MANIFEST.md` row yet — step 2 says one is owed
when game content starts referencing the pack, and nothing references this.

| Field | Value |
| --- | --- |
| Asset | `Television_01` |
| Source | https://polyhaven.com/a/Television_01 |
| Author | **Gabriel Radić** (`{"Gabriel Radić": "All"}`, from the site's own API) |
| Licence | **CC0 1.0** — site-wide, no attribution required |
| Published | 2020-03-30 (`date_published` 1585605600) |
| Measured size | **600.0 × 470.8 × 456.8 mm** (the API's `dimensions` field) |
| Categories | props, electronics, collection: the_shed |
| Retrieved | 2026-11-06 |
| Need it serves | A real 1990s CRT for the teens area. See `docs/FINDINGS-FIXTURE-SOURCING.md` §6 |
| Cleared to ship | **No.** Temporary placeholder under `DECISIONS.md` row 24 |

## The grant, quoted from https://polyhaven.com/license

> All assets (HDRIs, textures and 3D models) on this site are the original work
> of Poly Haven staff, or artists who willingly and directly donate/sell their
> work to Poly Haven. Our assets are all licensed as CC0, which is effectively
> Public Domain even in jurisdictions that do not support the Public Domain.

> You can use our assets for any purpose, including commercial work. You do not
> need to give credit or attribution when using them (although it is
> appreciated). You can redistribute them, share them around, include them when
> sharing your own work, or even in a product you sell.

Retrieved 2026-11-06. The curated chain of title — a named author per asset,
donating directly — is why this source was taken and the contribution-library
alternatives were not. See `FINDINGS-FIXTURE-SOURCING.md` §4.

**CC0 does not satisfy row 24.** Row 24 is *"everything we ship, we own"*, and
CC0 makes a work unowned rather than ours. This is a greybox placeholder with a
clean record, in the same bucket as the Game Animation Sample row in
`assets/MANIFEST.md`.

## Files, as downloaded

Outside the repository, per the intake convention:
`C:/Users/chris/Documents/LLK_AssetIntake_5_8/_downloads/polyhaven-television-01/`

| File | Bytes | SHA-256 |
| --- | --- | --- |
| `Television_01_2k.fbx` | 79,228 | `082bd7d2589e4f02af198c0325074eb6af9e46dd073a1a7fdfa7894e8dbb4206` |
| `Television_01_diff_2k.jpg` | 329,921 | `4e785a2f878f7bcafac7c823ef5734e8a1e2b848f6b36e67ba4fa43d6743cc42` |
| `Television_01_nor_dx_2k.jpg` | 439,645 | `c0d68e8e0c0959a769c0a06ecb535aefa494da43669fa18956f805e7951dc502` |
| `Television_01_arm_2k.jpg` | 691,103 | `7ac7cbe0877a8c78478dd61e5c5a20cff9fed9da61fa34419c66037b605ce733` |

1.5 MB in total. `arm` is the packed AO / roughness / metallic map.

**Normal-map trap.** The FBX's own material references name `nor_gl` — the
OpenGL normal — plus separate roughness and metallic EXRs, none of which were
downloaded. `nor_dx` is the one Unreal wants. Pointing UE at a GL normal
inverts the green channel; it does not fail the import and it is not obvious on
a dark prop, it simply lights backwards.

Nothing has been imported. `ASSET-INTAKE.md` step 4 requires Unreal's
**Asset Actions → Migrate** into `/Game/ThirdParty/PolyHaven_Television_01/`,
not a file copy, and step 7 says to check Git LFS before pushing — `*.fbx` and
`*.tga` are LFS patterns. ~~The LFS pre-push path is currently failing~~ —
**no longer true: X6 closed 2026-11-05 by `ea928d3`, and `0c0c921` pushed a
230 KB `.umap` clean on 2026-11-09.** The transfer path works.
See `docs/HANDOFF-CODEX-LFS-LOCK-VERIFY.md`.
