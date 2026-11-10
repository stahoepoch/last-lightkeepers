# Handoff — Codex — X6, the LFS lock check: three things the filing does not have

> **CLOSED. X6 closed on 2026-11-05 by `ea928d3`**, which carried the three
> Sweeper cue wavs — *"Uploading LFS objects: 100% (3/3), 249 KB, done."* —
> and that is the push this document asked for. `docs/HANDOFFS.md` row X6 has
> the full closure. Nothing below is a live problem. It is kept as the record
> of the diagnosis, because this file is **why** the closing test was changed
> from a text-only push to one carrying a real object.
>
> **Further evidence, 2026-11-09.** `0c0c921` pushed
> `Content/LLK_Boswells_Anchor.umap` — *"Uploading LFS objects: 100% (1/1),
> 230 KB, done."* `git lfs push --dry-run` reports nothing outstanding and
> `git lfs locks` exits 0 with no rows. So both the verify call and the
> transfer path are working, now on two independent pushes.
>
> **And a correction I owe this file.** On 2026-11-09 I wrote, in
> `0c0c921`'s own commit message and twice in conversation, that that push
> was *"the first LFS object this repository has committed since X6"* and
> that it closed X6's test. **Both are wrong.** `ea928d3` did it four days
> earlier, X6 was already struck through in `docs/HANDOFFS.md`, and the row
> says so in the sentence that closes it. I planned against a document
> instead of reading the ledger that exists precisely so nobody does that —
> which is, exactly, the failure `HANDOFFS.md`'s own preamble was written to
> fix. The commit message is pushed and is not being rewritten; this is the
> correction.
>
> **The live row is X8, not this one:** a fresh clone has none of the four
> `git lfs install` hooks, because they are untracked. That is Codex's, it is
> §2 below, and a second machine does not hit a loud failed push — it hits
> LFS objects that silently never upload.

**Status:** nothing changed. No config set, no hook touched, no `--no-verify`.
**Lane:** LFS config, `.gitattributes` and `.gitignore` are yours. Everything
below is read-only verification from this checkout.
**Reads with:** `docs/HANDOFFS.md` row **X6**, which is the filing. This does
not replace it; it corrects one word of its diagnosis and adds a stake and a
closing test it does not have.

Chris asked for this after choosing to leave the fix with you rather than have
me set the config.

## What X6 has right

`git push` dies in `git-lfs pre-push`, deterministically on `d5f2943`, on a
commit of four text files with zero LFS objects, and `--no-verify` is the wrong
answer because the same hook is what uploads real LFS objects. All confirmed.
The stack from `git lfs logs last` puts the failure in
`locking.(*httpLockClient).SearchVerifiable` → `lfshttp.handleResponse`, so it
is the **verify** call that fails, nothing to do with transfer.

Environment, for the record: git-lfs **3.6.0**, git **2.47.1.windows.1**,
endpoint `https://github.com/stahoepoch/last-lightkeepers.git/info/lfs`
(auth=basic), and `locksverify` is **unset** — not false, unset, so git-lfs
decides per response rather than following a policy anyone chose.

## 1. The remote did not say it lacks the locking API. It returned an error

X6 reads the message *"Remote origin does not support the Git LFS locking
API"* as the remote not implementing it. That sentence is git-lfs's **generic
interpretation of any failed verify**, and the `locksverify false` line is the
generic advice attached to it. Both are printed without looking at what came
back.

Queried read-only, no push involved:

```
$ git lfs locks
Error while retrieving locks: Server error:
  https://github.com/stahoepoch/last-lightkeepers.git/info/lfs/locks?refspec=refs%2Fheads%2Fmaster
```

**"Server error"**, not "not found" and not "not implemented". GitHub does
implement the LFS locking API on this plan, so the likely reading is a
transient fault at the remote rather than a missing feature — which matters
because the two have different fixes. A missing feature needs the config line.
A 500 needs a retry.

**So try the push again before configuring anything.** It costs one command and
it is the only fix that leaves nothing behind.

## 2. "Blocked for everyone" is too strong, in two different ways

**It was not blocking me.** `fed5a41` pushed clean through this identical hook
at 21:36 on 2026-11-04 — `a0bd1e2..fed5a41`, three commits, no `--no-verify`.
X6 notes `bf29d37` and `e27ca26` going through the same day and infers
something changed. The narrower reading is that nothing changed in the repo at
all: with `locksverify` unset, each push asks the remote and takes the answer
it gets, so a remote that is erroring intermittently produces exactly this —
some pushes through, some not, no local cause.

**And the hook is not in the repository.** `git ls-files` says:

```
pre-commit     TRACKED
pre-push       untracked
post-commit    untracked
post-merge     untracked
post-checkout  untracked
```

`core.hooksPath` is `.githooks`, the lighting guard in `pre-commit` is
committed, and the four hooks `git lfs install` generated are **not**. Two
consequences:

- **Git history cannot answer X6's own question.** "Something changed at the
  remote or in the hook" — the hook is untracked, so there is no history of it
  to check. Whatever you conclude about the hook comes from the file on disk,
  not from the log.
- **A fresh clone of this repo has no LFS hooks at all.** It gets the lighting
  guard and nothing else. So "for everyone" is true of this working copy and
  false of the next one, and anyone cloning to a second machine will not hit
  X6 — they will hit the quieter problem of LFS objects that never upload.

That second one is a bigger exposure than X6 and it is in your lane. I have not
touched it.

## 3. What is actually at stake, and why X6's closing test is not enough

X6 closes when *"a push of a text-only commit succeeds without `--no-verify`"*.
That proves the lock verify stopped failing. It does not prove the transfer
path works, and the transfer path is the one with the cost attached.

LFS is in heavy real use here:

- `.gitattributes` has **15** `filter=lfs` patterns — `*.uasset`, `*.umap`,
  `*.ubulk`, `*.uexp`, `*.upack`, `*.fbx`, `*.psd`, `*.tga`, `*.exr`, `*.hdr`,
  `*.wav`, `*.ogg`, `*.mp3`, `*.mp4`.
- `git lfs ls-files` reports **220** objects currently tracked.
- **Three untracked `*.wav` files are sitting in `assets/audio/` right now** —
  `sweeper-stock-filed-v1.wav`, `sweeper-stock-interrupted-v1.wav`,
  `sweeper-stock-started-v1.wav`. `*.wav` is an LFS pattern, so they become LFS
  objects the moment someone commits them, and `pre-push` is what uploads them.

So the test that actually closes this is **a push carrying a real LFS object**,
not a text-only one. One of those three wavs will do it, once whoever owns them
commits them. A text-only push passing while the transfer path is broken is the
same shape as every entry in `TOOLING-TRAPS.md`: a true answer to a nearby
question.

## Recommended order

1. **Retry the push.** Nothing changes, and an intermittent 500 clears itself.
2. If it fails again across a day, set the line git-lfs suggests:
   `git config lfs.https://github.com/stahoepoch/last-lightkeepers.git/info/lfs.locksverify false`.
   It is per-remote and disables **only** lock verification, which nothing here
   uses — but it is permanent and silent, so it belongs in a commit that says
   so rather than in someone's local config.
3. Either way, close X6 on a push that carries an LFS object.
4. Separately, decide whether the four `git lfs install` hooks should be
   tracked. That is the real repo-wide question this turned up.

## Out of scope

- **Do not use `--no-verify`.** X6 is right: the same hook uploads LFS objects,
  and a commit whose objects never upload is a broken clone for everyone else
  rather than a failed push for one person.
- **Do not delete `.githooks/pre-push`.** Same reason, permanently.
- **Do not change `.gitattributes`.** Nothing here suggests the patterns are
  wrong, and row 33's warning about `-text` on binary assets still stands.

## Two commits waiting

~~`d5f2943` and `53163bb` are on `master` in this shared checkout, two ahead
of `origin/master`.~~ **Both went out on 2026-11-05**, with `d183234` and
`933d6cb`, on a retry with nothing changed — which is the evidence for the
transient-fault reading in §1 over the config line git-lfs suggested. As of
`0c0c921` the checkout is in sync with `origin/master` and there is nothing
waiting.
