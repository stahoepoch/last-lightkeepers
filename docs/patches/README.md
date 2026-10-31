# docs/patches — work handed between writers

A patch lives here when it is finished but cannot be committed yet: usually
because it could not be built, and landing unbuilt code on a branch somebody
else is working from is the collision row 27 exists to prevent.

**Handoffs go in the repository.** The first version of the patch below was
written to this session group's shared notes folder, which is outside the
checkout and not visible to another writer's process. It had not vanished and
it was not gitignored — it was simply somewhere Codex could not reach, and
"look in the notes folder" is not a path. Anything one writer needs to hand
another belongs under version control, named, with the recipe beside it.

---

## `roster-sense-audit-UNBUILT.patch`

Claude's, cut 2026-10-31 against `6997aad`. **175 lines, five files, never
compiled.** Verified to apply cleanly to `HEAD` as of this commit — no commit
has touched those five files since it was cut.

Step 2 of the enemy-roster repair sequence: the importer fix (`1ae1db9`) makes
the data arrive, and this makes a future failure announce itself instead of
hiding, which is what let a blind roster survive six playtests.

It lifts `SenseReaching`'s internal `bHasAnySense` expression into
`LLKPerception::HasReachingSense(const FLLKEnemyDef&)` — one definition called
by both the rule and the audit, because two copies would drift and the audit
would then be reporting on a rule nobody runs. Then
`ULLKEnemyRosterSubsystem::AuditSenses` logs one `Error` at load naming every
non-Warden family with no reaching sense. It repairs nothing and must not: the
numbers are not there to recover, and inventing radii would be worse than
noticing nothing.

Section 7 of `LLKPerceptionFailureTest.cpp` comes with it — healthy rows true,
unimported rows false, half a sense false, the Warden false by authorship
(row 7), and the limit pinned: zeroed radii count as **present**, so the audit
answers true while `SenseReaching` still correctly notices nothing. The audit
catches a field that never arrived and cannot catch one that arrived wrong.

### To land it

```
git apply docs/patches/roster-sense-audit-UNBUILT.patch
```

Then build with the editor closed, and **read Unreal's printed `Result:` line,
not the exit code** — see `docs/TOOLING-TRAPS.md` for why that is not a
preference.

To prove the new assertions red first, change `HasReachingSense` to
`return true;` and expect eight failures in `LLK.Enemies.PerceptionFailures`.
If that test reports `Success` with the mutation in place, the binary is stale
and nothing you are reading came from your build.

Delete this patch once it has landed.
