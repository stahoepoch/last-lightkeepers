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


Delete this patch once it has landed.

---

## Editor scripts, which are not patches and should not be deleted

`dress-boswells-circuit.py` is a different kind of thing from the patch above,
and the difference matters because the last line of this file says to delete a
patch once it has landed.

It **has** landed — it is the pass that shipped as `d399e40`. It lives here
because it is re-runnable by design and because it only existed in a
session-specific scratchpad, which is the same mistake this README already
records one step further along: the work was not gitignored and had not
vanished, it was simply somewhere the next writer could not reach.

So the rule for this directory is two-sided:

- **A patch** is unlanded work waiting for a build. Delete it once it lands.
- **An editor script** is a pass that needs a viewport, kept so the next
  sitting re-runs it rather than rewriting it. It carries its own run recipe
  in its header. Keep it until the map it edits stops existing.
