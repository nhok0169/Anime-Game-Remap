# Overview

What this project is, how the repo is laid out, and the operating norms that don't fit neatly
under Building/Testing/Documentation/Architecture. Read this one first if you're new to the repo;
it's the map the other [AI Agent Help](../README.md) files assume you have.

## What this project is

**Anime Game Remap** (formerly `FixRaidenBoss2`) — a library/CLI that remaps mods installed on
one character onto another character's skin, for Genshin-Impact-style GIMI mods. Ships three
ways: a standalone script, a CLI, and a Python API (`pip install AnimeGameRemap`). This repo is
the monorepo for all of it, plus its docs site and test suites.

Two remotes/branches matter:
- **`nhok0169`** — the main/release branch.
- **`development`** — the active development branch (what you're usually on; branch new work off
  this, not off `nhok0169`).

## Repo layout

```
Anime Game Remap (for all users)/
  api/                          <- the Python API package + its native extensions
    CMakeLists.txt              <- top-level CMake orchestrator (core + py + cy subprojects)
    pyproject.toml              <- scikit-build-core build config; version lives here
    extern/                     <- vendored deps for the whole api build (z3, uni-algo, ...)
    src/
      cpp/
        core/                   <- AGRemapCore: standalone C++ static lib, no Python deps
          include/AGRemapCore/  <- public headers, Doxygen-documented
          src/                  <- .cpp/.tpp implementations
          Doxyfile              <- Doxygen config; run from this dir
          CMakeLists.txt
        py/                     <- pybind11 bindings around AGRemapCore -> compiled `core` module
          src/
          CMakeLists.txt
      cy/                       <- Cython extensions (CyDictTools, CyListTools, ...)
    src/py/FixRaidenBoss2/      <- the installable Python package (pure-Python code +
                                    the built `core.*.pyd`/`.so` + Cython outputs land here)
  script build/                 <- the older single-file-script distribution variant
Docs/                           <- Sphinx + Doxygen(Breathe) documentation site
Testing/
  Unit Tester/                  <- unittest-based suite, run via its own main.py
  Integration Tester/           <- end-to-end suite, run via its own main.py
Tools/
  APIBuilder/                   <- the build driver for api/ (what you use to compile everything)
  TexConverter/                 <- converts .dds textures to .png/.bmp/.jpg so they can actually be
                                   looked at (the Read tool can't open a .dds) -- see Texture Editing
  VGRemapFinder/                <- proposes a vertex-group remap (a Data/RemapDrafts workbook) from two
                                   characters' dumps; its benchmark.py scores any change against every
                                   hand-made draft at once -- see its README before tuning it
  Utilities/                    <- shared helper package (AGRemapUtils) used by the test runners
                                   and by every tool here -- it is published to PyPI, so keep its
                                   dependencies light. See Tools
  ModToDumpConverter/GI/        <- Jupyter notebooks that CONSUME the API -- see the note below,
  DumpToModConverter/GI/           they go stale when you change it
  ScriptBuilder/, CIPipeline/, ModAnalyzer/, ...  <- maintainer tooling, not usually needed for
                                    a feature PR
Examples/, Data/                <- end-user-facing sample content
AI Agent Help/                  <- you are here — agent operating instructions, split by topic
```

The C++ core (`AGRemapCore`) is a from-scratch reimplementation of pieces of the pure-Python
package for performance (tries, DFAs, ordered multimaps, if-template parts, etc.), and the Cython
layer (`api/src/cy`) fills a similar role for a few standalone dict/list utilities. Despite the
"optional acceleration" framing this suggests, `FixRaidenBoss2/__init__.py` does unconditional
`from .core import ...` / `from .CyDictTools import ...` / `from .CyListTools import ...` at
module load with no pure-Python fallback — so in practice these extensions are a hard dependency
of `import FixRaidenBoss2` succeeding at all, not something gracefully skipped when absent.

**Compiled native binaries are *not* tracked in git** — `*.pyd`/`*.so` are both listed in
`.gitignore` (verified with `git check-ignore -v` and `git ls-files -- "*.pyd" "*.so"`, which
returns nothing tracked anywhere in the repo). An earlier version of this file claimed the
opposite; don't trust that claim if you see it repeated elsewhere. Practical effect: if you change
C++ or Cython code, you must rebuild locally (see [Building](../Building/CLAUDE.md)) to get a
working `api/src/py/FixRaidenBoss2/` for your own testing, but there is nothing to `git add` for
the binary itself — don't go looking for a "commit the fresh build" step, and don't tell the user
you've committed one. Whether/how CI's Linux job (which does no C++/Cython build — see
[Testing](../Testing/CLAUDE.md)) ends up with a working `core`/`Cy*` extension at all is unverified
from this angle; don't assume a green CI run proves your native-code change is correct beyond what
you've verified locally.

## Working a feature or bug request here: the habits that pay

Written after several sessions where the *diagnosis* cost far more than the fix. None of this is
about the domain -- it is about how this particular codebase fails.

**1. Assume the failure is silent, and go looking for it.** This repo's dominant bug shape is not a
crash or a wrong value; it is a code path that runs, logs success, and does nothing. Confirmed
examples, all found the hard way: a fixer that renamed sections and wrote no remapped geometry; a
texture edit that logged `Editting texture for X.dds` and produced no file; downloads written into
the `.ini` and never fetched; a `.buf` copied without being remapped. **So "the run succeeded" is
never evidence.** Check the artifact -- the file exists, its bytes changed, the count went up.

**2. "Recorded" is not "consumed" -- grep the getter.** Several features were fully built, wired
into a model, and then read by nobody. The whole download feature was inert because
`RemapService::fixResources` walked `getResources()` and never `getFileDownloads()`. When you add
something to a registry, or find something already in one, `grep -rn "getTheThing"` across `core/`
and `py/` and confirm a real consumer exists.

**3. When a C++ path is inert, suspect the pybind layer took the override.** Whole halves of the
fixer layer were only ever reached from Python: the core class does the naming and the `Py*`
subclass does the work. This has now bitten twice in the same way (`RemapBlendReplace` needed
`VGRemapBlendReplace`; `TexReplace` needed `TexEditorReplace`). Before using any `resEdits/` or
`graphGroupEdits/` class from plain C++, check whether its `Py*` counterpart overrides something
core does not. See [Creating Remaps](../CreatingRemaps/CLAUDE.md)'s "Seams that work from Python and
do nothing from C++".

**3b. The mirror image, when you are ADDING to a C++ class rather than calling it: a `Py*`/pure-Python
subclass that overrides the method will ignore your new option unless you route it twice.** Adding
`compress` to `AGRemapCore::TexCreator` gave the pure-Python `TexCreator` (which subclasses the
binding) a `compress` attribute it *inherited and silently ignored*, because its `fix()` replaces the
C++ one wholesale and called `texFile.save(img = img)` with no `compress` argument. An inherited
option that does nothing is worse than no option: it reads as supported. The sibling
`TexEditor.py` already did this correctly (`texFile.save(compress = self.compress)`), which is the
tell to look for --- **when you add a member to a bound C++ class, grep the pure-Python side for a
subclass of it and check every method that overrides one you touched.**

**4. Find the tests before you claim there are none.** There are **two** test trees and they do not
overlap: `api/src/cpp/core/tests/` (standalone C++, built by nothing) and
`Testing/Unit Tester/UnitTester/Tests/` (the Python suite). Grep **both**, and confirm your path
actually resolved -- a relative path that resolves nowhere greps clean and is indistinguishable from
real absence. That mistake led to "no tests cover this", a behaviour change, and nine red tests.

**5. A divergence from the old script is not automatically a bug in the new code.** Much of the
C++ layer is a *replacement* whose semantics the maintainer specified, not a port. When new output
disagrees with `FixRaidenBoss6.py`, work out which one is actually wrong before changing anything --
and if the behaviour was specified for you, raise it rather than silently re-specifying it. A
"fix" to `RegDelimitedAdd`'s documented placement rule broke nine tests that existed precisely to
pin it, and the maintainer then confirmed the original behaviour was correct.

**6. Prove a refactor by byte-identical output, not by green tests.** The suites here do not cover
the data layer well enough to catch a behaviour change in an extraction. Run the real entry point
before and after, and `cmp`/`md5sum` the produced `.ini` **and** the produced binaries. That is what
demonstrated the `DownloadTools` extraction changed nothing.

**7. Measure a third-party failure; do not infer it.** When Compressonator refused a `.dds`, reading
its source went nowhere. Two things settled it in minutes: a *cheap hypothesis test* with no build
(rewriting the file's header to say `mipMapCount = 1` -- it then loaded), and a ~50-line standalone
`.cpp` calling the two suspect functions and printing both status codes. Reach for those before a
long code read.

**8. Know what state your verification harness leaves behind.** The A/B scripts under the scratchpad
run several fixes **and then an undo**, so the tree you inspect afterwards is the *undone* one.
Inspecting it and concluding the feature did nothing is a mistake that costs a full cycle -- run a
single fix into its own directory when you want to look at fixed output.

**9. Rebuild, then verify the `.pyd` actually moved.** A stale `core.*.pyd` makes every subsequent
observation a lie. See [Building](../Building/CLAUDE.md) for the build-batch hygiene and the
mtime check; and note a run parked at `== Press ENTER to exit ==` holds the `.pyd` open and makes
the next build's copy step fail.

**9b. Under `Tools/`, run the tool before you change it -- expecting it to be broken.** Nothing
tests that layer, so a tool can sit broken for months because the only person who would notice is
whoever next runs it. One session that set out to add a single flag found **three** tools that could
not run at all, all broken by the same event: the API's package moving to `api/src/py/` during the
C++ migration. The worst of the three was not a crash but a *silently relocated output*. See
[Tools](../Tools/CLAUDE.md).

**10. A FAILING check is a claim too -- validate it before you report it.** Habit 1 says a success
can be fake. The inverse bites just as hard and is easier to believe, because a failure feels like
diligence. A comparison harness built the filename `"JeanJean" + "JeanCN"` -- a file that never
existed -- and `cmp -s` on two absent files returns non-zero, which is indistinguishable from a real
mismatch. That produced two confident, wrong "the blends differ" reports to the maintainer before
the paths were checked. **Before believing a negative, assert the thing you compared exists**: print
the resolved paths, `ls` them, confirm the anchor matched. The same rule as habit 4's "confirm your
path actually resolved", generalised past grep to every check you write.

The same harness fails the other way too, and that one reads as success. An A/B script whose source
mod folder had been moved copied nothing, fixed nothing, and reported *"(identical), 0 dangling"* --
a clean pass over two empty directories. **The maintainer swaps mod folders in and out of `Mods/`**
(3dmigoto tolerates only one mod per character, or the models interfere -- see
`CreatingRemaps/Images/Jean/6_1/JeanSeaAmalgamation.jpg`), so a path that worked an hour ago may be gone.
Assert the inputs exist and are non-empty before reporting either outcome.

**11. For anything that is the first of its kind, suspect core before suspecting yourself.** Code
paths here are exercised by whatever characters happen to exist, so a genuinely new *shape* tends to
land on machinery nobody has run. Jean was the first character to remap onto **two** targets, and
that alone surfaced two independent core bugs -- fixers overwriting each other's `.ini` text, and a
resource edit mutating the `.ini` file's own parsed sections. Neither was in the new data; both had
been waiting. When new data behaves oddly, ask early: *has this path ever actually run before?*

**12. Three layers, three separate checks: the text, the references, the bytes.** They fail
independently and an earlier one passing says nothing about a later one. Two bugs in one session
made this concrete: a `.ini` that named a `Blend.buf` a later removal had deleted (text fine,
reference dangling), and a `.ini` that was *perfect* while its `Blend.buf` had been remapped twice
(text fine, references fine, bytes wrong -- visible only as a warped model in game). So:

| layer | check |
| --- | --- |
| text | diff the generated section **names** against the old script's |
| references | `CreatingRemaps/check_dangling.py` -- every `filename =` exists on disk |
| **content** | diff each section's **body**, not just its name |
| bytes | `cmp` **every** produced `.buf`/`.dds` against the old script's, per sub-mod |

The bytes check is the one people skip and the only one that catches a *wrongly* remapped file, as
opposed to an unremapped one. Note that comparing the source against the output -- the check
`CreatingRemaps` documents for "was this remapped at all" -- passes happily on a file remapped
twice.

**And the content layer exists because a reference can resolve to the WRONG thing.** JeanSea's merge
left its second `.ini` file saying `vb1 = ResourceJeanSeaBlend` -- the mod's own untouched blend --
instead of the remapped one. Section names matched the old script exactly, every reference resolved
(that blend really is on disk), and both produced binaries were byte-identical, because the file that
was missing was one nothing asked for. Three of the four layers passed. Only reading the section body
found it, and in game it would have been another warped model.

**13. If the old implementation is faster or smaller, check it was doing the same work.** Not every
difference from `FixRaidenBoss6.py` is a regression in ours, and not every place it looks better is
a place it *was* better. Its texture editing ran ~250x faster than the C++ path, which looked
alarming until the file headers were read: `Pillow` has no BCn encoder, so it wrote 32-bit
uncompressed `.dds` and never encoded at all -- 32MB where ours writes 8MB of BC7. Compare the
artifacts' *format*, not just the clock.

**14. Your own analysis script is a claim too --- check it against ground truth before you report
it.** Habit 10 covers a *failing* check that was wrong. A **clean** one is more dangerous, because
nothing prompts you to look twice. Scanning `core/src` for anonymous-namespace helper names that
collide across files returned **zero**, and that got reported as fact and built on; the regex only
recognised `namespace {` at *file scope*, and in this codebase nearly all of them sit nested inside
`namespace AGRemapCore { ... }`. There were 21 collisions across 17 files, and MSVC found every one
of them the moment a build actually ran. **Before trusting a scan you wrote, make one real tool
agree with it** --- run the compiler, run the entry point, grep for a case you already know the
answer to. A script that reports "none" over a codebase whose shape you assumed is indistinguishable
from a script that reports "none" because it matched nothing.

**15. Benchmark the workload, not a proxy --- serial measurements lie about this machine.** The dev
box is 12 cores / 24 threads against roughly 300 translation units, so it runs out of memory
bandwidth long before it runs out of parallelism, and **anything that trades parallelism for less
total work loses here even though it looks like a large win in isolation.** Measured: a unity build
compiling 19 files one at a time went 61.0s -> 5.5s, an apparent 11x --- and in the real parallel
build it was a net *loss*, tripling the cost of a single-file edit; it is switched off for that
reason. A precompiled header measured ~2x per file and delivered 13% end to end. Both numbers were
honest; both were the wrong measurement. Two things make this cheap to get right: `cbuild/.ninja_log`
already records start/end milliseconds per edge, so the real critical path is on disk with no
instrumentation (that is what showed two serial steps were 136 of 138 seconds), and **a change you
built is still allowed to lose** --- measure it against the case you actually care about and be
willing to default it off, as happened here.

**16. Habit 5 has a converse that costs more: AGREEMENT with the old script is not correctness.**
Habit 5 says a divergence is not automatically a bug. The reverse trap is treating the old script's
output as the specification and steering towards it. **The old pure-Python script is much less
powerful than this library** -- it had no `RegFillMissing` and no complex graph filters, and
simulated them with chains of the tools it did have. Its `IbRemapData`/`IbDrawIndexedRename` chain
derives a draw call from wherever `ib` is bound, which is a *workaround*, and it places the draw
ahead of anything the section sets up afterwards. That silently killed a mod's transparency. A whole
session went into reproducing that topology exactly -- 25 draws matching 25, section by section --
because the A/B rewarded it at every step. **When the reference and the mechanism disagree, work out
what the output has to DO, and be ready for the answer that the old script is wrong.** The A/B is
still the best tool here; it just answers "did I change anything" and not "is this right".

**17. Sample the rows that belong to your subject, or you will confirm the wrong mechanism.** The
sharpest self-inflicted wound of the session: a diagnostic printed one line per classifier decision,
`Select-Object -First 4` showed four saying `count=0`, and that became "the key is absent from the
colouring" -- a conclusion stated confidently, built on, and wrong, because those four rows belonged
to the `IB` sections, which genuinely have no such key. The rows for the section under investigation
said `count=1` and were never looked at. `-First N` / `head` on a filtered log is a **sample**, not a
summary; filter to the subject (`grep -A1 section=TheOneIMean`) or aggregate (`sort | uniq -c`)
before drawing a mechanism from it. The same instinct catches the cheaper version: `git show
HEAD:<path>` with a cwd-relative path returns nothing and grep dutifully reports `0`, which reads
exactly like "this content is absent from HEAD" when it means "that path does not resolve".

**18. A rename succeeding is not evidence a build succeeded.** The sharpest version of habit 1,
and it cost four build cycles in one session. Every defect in the resource-edit seam let the
*renaming* half of a fix run to completion, so the `.ini` file came out looking perfectly
plausible --- correct section names, correct references --- while the resource those names point
at was never built. A check on the text passes; only a check on `ini.getResources()`, or on the
file that should exist on disk, separates them. Whenever a change touches something that both
**names** and **builds**, assert on the built thing.

**19. `stats` is the diagnostic channel when nothing is printed.** `RemapService` catches a
per-`.ini` failure, records it in `stats.ini.skipped[path]`, and prints it only if a logger is
attached. An embedding caller --- a prototype script, a test --- has none, so a fix that raises
looks exactly like a fix that did nothing. Read `stats.ini.skipped` and `stats.download.skipped`
before concluding a path is inert; the exception object comes back intact, not stringified. Every
defect in the Python-override work was found this way, and none of them would have surfaced
otherwise.

**20. A bound API can accept the wrong type in silence.** Not every argument is validated on the
way in. `RegFillMissing`'s `fillMissing` takes a string, a list of `(key, value)` tuples, or a
callable --- and a `dict` is accepted without complaint and fills nothing, leaving a run reporting
`fixed=7 skipped=0` with one register missing from the output. When an edit silently does
nothing, re-read the parameter's accepted shapes before suspecting the edit itself.

**21. Tune against ALL the ground truth, never the case in front of you.** Blend-weighting the
vertex-group summaries moved one group either way on Ganyu, the character the finder was being
developed on, and was worth 2--5 points over the twenty draft directions; the same-object
restriction looked reasonable on one elbow and cost 12 points overall. `Tools/VGRemapFinder/
benchmark.py` exists so that no matching idea is ever judged on one character again. The general
form: when a repo has a corpus of hand-made truth (drafts, goldens, old-script output), score a
change on the whole corpus before believing the example it was written for.

**22. Mark what you generate, or it becomes ground truth.** A workbook the finder wrote landed in
`Data/RemapDrafts/`, the benchmark read it as a hand-made draft, and the total went up -- the
tool was scoring itself at 100%. Anything a tool writes into a folder that a check reads must
carry a mark the check knows (`About` sheet / `E1` cell here), and the check must skip it. The
same applies to a golden tree you regenerate and a fixture you copy.

**23. A generated edit to a data table is a claim: re-parse the result before writing it.** A
regex patcher for `VGRemapData.cpp` that mixed a positional group with named ones dropped the
closing `})},` of every row it touched, and the file still *looked* patched. The patcher now
re-parses its own output and asserts the row count and every row's pair count before the write;
the file was restored with `git checkout --`. Then rebuild and read the table back through the
bound API (`getVGRemap`), because the compiler accepting it proves the syntax, not the data.

<br>

## Operating norms

- Don't push or open a PR unless asked. If you do, branch off `development`, not `nhok0169`.
- **You may not be the only agent in this working tree, and `git checkout -- <file>` is
  unrecoverable.** The maintainer runs several agents against the same checkout, so a file you did
  not write can gain uncommitted work mid-session. This was noticed the lucky way: `RegDelimitedAdd`
  grew a plural `Additions` API that `HEAD` did not have and that no script of mine had written --
  another agent had reworked it while the session ran, and three earlier `git checkout --` calls on
  that same file happened to predate it. **Prefer a targeted patch script that removes exactly your
  own edit** over `git checkout --` on anything you did not create in this session; and if a file's
  content stops matching what you read minutes ago, consider a concurrent editor before concluding
  your own patch misfired. (Check your own backgrounded tasks first -- a backgrounded patch script
  that already applied looks identical to someone else's edit.)
- **Switching to `nhok0169` is not a cheap `git checkout` -- it removes your working directory and
  strands `development`'s submodules.** Confirmed hands-on 2026-09-06, doing a data-only change
  that had to land on both branches. Three things bite, in order:
  - **`api/src/cpp` does not exist on `nhok0169`** -- that branch predates the entire C++ core, and
    its API package is `api/src/FixRaidenBoss2/` rather than `api/src/py/FixRaidenBoss2/`. If your
    session's working directory is anywhere under `api/src/cpp`, move it to the repo root *first*;
    otherwise every command after the checkout runs from a path git has just deleted. Check what a
    branch actually contains with `git ls-tree <branch> --name-only <path>` before switching.
  - **`development` has submodules; `nhok0169` has no `.gitmodules` at all.** After the switch,
    `api/extern/` (utf8proc, xxHash, z3, curl, Compressonator, common, ordered-map) survives as a
    plain *untracked* directory full of nested git repos, alongside `api/src/py/`, `api/cbuild/`,
    `api/wheelhouse/`, `cbuild/` and `cebuild/` -- ~11k untracked files in total, because git
    cannot remove a directory that still holds ignored/untracked content. **Never `git add -A` on
    `nhok0169`**: it would record those nested repos as bare gitlinks with no `.gitmodules` to
    resolve them, which no clone can check out. Stage explicit paths instead
    (`git add "Data/Mod Downloads"`), then confirm nothing else came along with
    `git diff --cached --name-only | grep -v "^<your path>/"`.
  - **Leave the leftovers where they are.** They are untracked on `nhok0169` and harmless; deleting
    them costs a full submodule re-clone and native rebuild when you switch back to `development`.
  - **Content under `Data/` is shared byte-for-byte between the two branches**, so a data-only
    change there generally has to be committed twice, once on each — see
    [Creating Remaps](../CreatingRemaps/CLAUDE.md)'s "The download assets" section.
- **If you're running in a `git worktree` (not the user's main checkout), don't trust that its
  branch is actually based on `development` just because that's the norm** — verify before relying
  on any file being present. Confirmed the hard way: a worktree's branch had been created off
  `nhok0169` at a point that predates the entire C++ core (`api/src/cpp`) existing, so a task
  referencing a `core/` file failed with "no such file" until this was diagnosed. Check with
  `git log --oneline -3` (does it look like `nhok0169`-style single-fix commits, or
  `development`-style porting/feature commits?) and, if a specific file is expected,
  `git ls-tree -r --name-only HEAD -- <path>` before assuming the checkout matches the task. If the
  branch is wrong and has no commits of its own yet, `git checkout -B <branch> development` resets
  it cleanly; if it already has real commits on the wrong base, surface the mismatch and ask before
  rebasing/merging — a same-branch rebase across a long-diverged pair of branches (`nhok0169` vs
  `development`) can hit real conflicts in live, unrelated code (confirmed: conflicts in active
  Python fixer logic and a delete/modify conflict, not just incidental files), so treat it as risky
  enough to check with the user rather than resolving blindly.
- **Updating a branch that's checked out in a *different* worktree (including the user's main
  checkout — it's "just another worktree" from git's perspective) needs to happen from that
  worktree, not yours.** `git branch -f <branch> <commit>` (and similar ref-forcing commands) is
  refused with `fatal: cannot force update the branch 'X' checked out at '<path>'` when run from a
  worktree other than the one that has it checked out. To fast-forward/merge a change into a branch
  another worktree owns, either run the merge from over there (`git -C <other-worktree-path> merge
  --ff-only <commit>` — this also updates that worktree's working-tree files for you, so the user
  doesn't need to manually `git checkout`/`git reset` afterward) or push/PR instead if that fits the
  task better. Confirmed hands-on: merging a fix branch into `development` while the main checkout
  had `development` checked out required doing the `--ff-only` merge from the main checkout's path,
  not the fix branch's worktree.
- Rebuild before considering a `core`/`cy` change done (see previous section) — this applies
  equally to Cython (`api/src/cy`) changes, not just the C++ core; it's the same build command
  (see [Building](../Building/CLAUDE.md)'s "Cython pieces").
- **Verify a rebuilt native extension via the PowerShell tool, not the Bash tool.** On this
  machine, importing *any* freshly-built `.pyd` (`core`, `CyDictTools`, `CyListTools` — this isn't
  specific to one module) through the Bash tool's Git Bash fails with
  `ImportError: DLL load failed while importing X: The parameter is incorrect`, even for binaries
  that import fine from native PowerShell. This is an environment/tool quirk, not a sign your
  build is broken — don't chase it as a real bug. See [Building](../Building/CLAUDE.md)'s
  "Verifying a build/binding change in Python directly" for the confirmed repro and workaround.
- When reporting test results, say which test module(s) you actually ran and their result —
  don't imply a full green suite when pre-existing, unrelated failures are still present (see
  [Testing](../Testing/CLAUDE.md) for the current list of known-broken modules).
- **For Cython (`DictTools`/`ListTools`-style) feature requests specifically, expect the request
  to leave a real semantic decision unstated more often than not** — auto-vivification behavior,
  an index/ordering scheme for a new callback shape, whether "all paths" means every node or just
  leaves, whether an `ordered` flag can be honored without changing a return type, and similar.
  Guessing wrong here means a wasted rebuild-and-test cycle, not just a style nit. Ask one tight,
  options-based clarifying question (with a recommended default and a concrete before/after
  example) before implementing, rather than picking silently — this repo's maintainer has
  consistently answered these quickly when asked and has been right to insist on it when an
  answer would've changed the implementation. Once implemented: rebuild, verify the new behavior
  empirically with a throwaway script *before* writing formal unit tests, then add the tests.
- **The same "ask one tight, options-based question" rule applies well beyond Cython — treat it as
  the default for any request that changes a *public API surface*, not just an implementation.**
  The tell is when the request's *intent* is unambiguous but its *mechanism* isn't: several
  materially different implementations would all satisfy the sentence as written, and they commit
  the codebase to different public shapes. Confirmed on a one-line-sounding request ("this
  predicate should also accept a `ModType`"): the intent was obvious, but it could have meant
  widening the existing shared `ReplaceIf` marker, adding a second marker class beside it, or
  accepting a plain `(value, predicate)` tuple — with a separate unstated fork over whether the
  old 1-argument predicate stays valid. Two quick questions settled both; guessing would have been
  a full rewrite of a class plus its tests and docs. Note this cuts the *other* way just as often:
  don't ask about anything the surrounding code, an existing sibling class, or the file's own
  conventions already answer — that's a judgment call to make yourself, and the maintainer's time
  is the scarce thing being spent either way.
- **This project is no longer Windows-only. It has been built, imported and tested on Linux
  (WSL2 / Ubuntu 24.04, GCC 13, Python 3.12) as well as Windows** — see
  [Setup](../Setup/CLAUDE.md)'s Linux section. Most of this documentation predates that and is
  written from a Windows seat; don't read "the build" as "the Windows build". Three consequences
  worth knowing before you touch anything build-related:
  - **The C++ is the portable part; the build glue is not.** `AGRemapCore` and the Cython layer
    compiled on GCC 13 / C++23 with zero source changes. Every portability bug found was in
    CMake glue, vendored third-party code, or `APIBuilder` — so when a cross-platform request
    comes in, look there first rather than at the core.
  - **`if(WIN32)` blocks in the CMake are the standing hazard.** Several existed with no `else()`
    at all (the curl TLS backend; the runtime-dependency install), which fails only on the other
    platform and often only at *runtime*. If you add or edit one, decide explicitly what the
    non-Windows branch does, even if the answer is "nothing".
  - **The install directory `api/src/py/FixRaidenBoss2/` is shared by both platforms and is not
    suffixed**, unlike the build trees. `cleanInstalls()` deletes every `.pyd`/`.so` under `api/`,
    so a plain build on either OS wipes the other's binaries. Pass `-i`/`--installKeep` on **both**
    sides when both matter.
- **Two `APIBuilder` functions destroy state *before* checking their preconditions — know this
  before running either.** `buildDocs()` (`-d`) `rmtree`s the 642 tracked files in `core/xml` and
  *then* invokes `doxygen`, so a missing/unresolvable Doxygen leaves them all deleted;
  `cleanInstalls()` deletes installed binaries before the compile that would replace them, so a
  failed build leaves you with none rather than with the previous working ones. Neither is a
  corrupted checkout — recover the first with `git checkout -- ".../core/xml"` and the second by
  fixing the build. See [Setup](../Setup/CLAUDE.md) for both in full.
- **Two committed artifacts are coupled to specific tool versions, so an unexplained diff in them
  is usually your toolchain, not your change**: `core.pyi` is byte-reproducible only with
  **pybind11 3.0.4**, and `core/xml` carries the **Doxygen 1.17.0** version stamp. Check
  `pybind11.__version__` / `doxygen --version` before concluding you changed the binding surface or
  the C++ docs.
- **Whatever tools `APIBuilder` needs must be on `PATH` in the shell that actually runs it** — it
  shells out to the bare names `"cmake"` and `"doxygen"`, so pointing at a specific Python
  interpreter is not enough (notably, running a venv's `bin/python` by absolute path does *not*
  activate the venv). This one root cause produced three separate confusing failures in a single
  session. `command -v cmake ninja doxygen` before a build is cheaper than diagnosing it after.
- This set of files was authored from hands-on, verified work in the C++ core / pybind11 layer
  (the `OrderedMultiMap` / `IfContentPart` / `IfContentPartColouring` subsystem, including a full
  pure-Python-to-C++ migration of the latter — see Architecture's "Two different outcomes for
  porting a class to C++/pybind11" section), plus a much larger, incrementally-built pass through
  the Cython layer (`CyDictTools`/`DictTools` and `CyListTools` — see Architecture's "Cython
  bindings" section and its dedicated gotcha section on exact-type parameter checking for what
  came out of it), plus — separately, later — the Python-side `.ini` graph model and its
  dataflow-analysis-based graph edits (`IniSectionGraph`, `GraphTools`, `CallGraph`, the
  `graphEdits/` strategy family; see [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)), plus —
  separately again, later still — wrapping Z3 in the C++ core without leaking it into public
  headers, the bidirectional `.ini`-predicate ↔ Z3 conversion pair (`IfPredZ3Generator`,
  `Z3IfPredGenerator`), and a full pure-Python-to-C++/Z3 migration of `IfPredPart` together with the
  Z3-ification of the `IniSectionGraph`/`ResGroupCollect` query-combination machinery that consumed
  it (see Architecture's Z3-wrapping/lifetime sections and [Ini Graph
  Editing](../IniGraphEditing/CLAUDE.md)'s "Predicate queries in this subsystem are Z3-typed, not
  sympy" section), plus — separately again, later still — a from-scratch C++/pybind11 port of a new
  `.ini` mod-type-classification subsystem: `GameTypeId`/`GameTypeIdTools`, `ModTypeId`/
  `ModTypeIdTools` (including a `findByName` AhoCorasick-backed name/alias registry and its
  `getModType`/`registerModType`/`clear` global-registry API), the lean `ModTypeIdData` and heavier
  `ModType`/`CppGIBuilder` model classes, and finally binding the previously-Python-unreachable
  `AGRemapCore::IniClassifier` itself (bound as `CppIniClassifier` at the time, later graduated to
  the bare `IniClassifier` once the pure-Python original was deleted outright — see this file's
  "Where the C++ migration currently stands" section) — see Architecture's sections on the
  static-non-copyable-type/pybind11-init-order/`pybind11/stl.h` gotchas this produced, and
  Testing's/Documentation's own notes on what this touched in each of those pipelines, plus —
  separately again, later still — a full pure-Python-to-C++ replacement of the whole
  `iniFixers/regEdits/` family (`BaseRegEdit`/`RegAdd`/`RegNewVals`/`RegRemap`/`RegRemove` as core
  class templates + pybind11 bindings, the old Python package deleted outright), which is where
  Architecture's sections on templating-a-core-class-for-pybind-reach, still-pure-Python
  collaborator types, and how-a-binding-holds-a-Python-supplied-argument came from — along with
  Documentation's `Attributes`-section and corrupt-`index.xml` traps, plus — separately again,
  later still — the same full-replacement treatment for the remaining two edit families,
  `iniFixers/graphGroupEdits/` and `iniFixers/graphEdits/` (`BaseIniGraphEdit`/`GraphRename`/
  `RegFillMissing`; `RegSurroundedAdd` alone left pure Python, re-parented onto the bound base),
  both Python packages deleted outright — see [Ini Graph
  Editing](../IniGraphEditing/CLAUDE.md)'s "Completing a simple stub" section for the
  keep-alive-refresh, re-derive-mode-and-fill-together, and mirror-a-Python-`Enum`-by-value
  conventions those produced. Other subsystems (the
  non-graph `.ini` parsers, the `GIMIFixer` family, the standalone script variant) still haven't
  been exercised to the same depth — verify assumptions there with the usual tools rather than
  trusting this file blindly.

- **The `*Old.py` suffix is this repo's deprecation marker, and it tells you what you're allowed to
  delete.** A class that has been replaced by a C++/pybind11 one gets renamed to `XxxOld.py` while
  its dependants are migrated. The finished end state for a deprecated class is: **referenced only
  from other `*Old.py` files** (plus `__init__.py`'s deprecated exports and its own
  `test_XxxOld.py`). So before deleting one, grep for it and classify each hit:
  - hits only in `*Old.py`/`test_*Old.py`/`__init__.py` — it's already in the end state; deleting the
    *file* additionally requires its deprecated dependants to go too, which is usually a separate,
    larger migration. Say so rather than doing it uninvited.
  - hits in live code (`data/`, `model/`, `remapService.py`, `ModType.py`) — those are the real
    migration work. Repoint them at the C++ class first.
  - **zero hits at all** — it's orphaned and can just be deleted. Confirmed: `GIMIParserOld.py` had
    no references anywhere in the repo, because `ModType.py` and `GIMIObjParserOld.py` had already
    been switched to `from ...core import GIMIParser`.
- **"Replace + remove the pure-Python X" means delete the old file once every *live* call site is
  rewired — not rename it to `XxxOld` and stop.** But check for a concrete blocker first and raise it
  rather than forcing through: a deprecated class is frequently the *base* of other deprecated
  classes that live data still imports. `GIMIFixerOld` is the worked example — it is the base of
  `GIMIObjReplaceFixer` -> `GIMIObjMergeFixer`/`GIMIObjSplitFixer` -> `GIMIObjRegEditFixer`, which
  `data/IniFixBuilderData.py` wires into 67 per-character entries, so the file itself cannot go until
  that chain does even though every live *wiring* now points at the C++ `GIMIFixer`.
- **The `Tools/*Converter/` Jupyter notebooks are real downstream consumers of the API, not
  reference material — a port that leaves them behind is half-finished.** `ModToDumpConverter/GI/`
  and `DumpToModConverter/GI/` are things the maintainer actually runs over real mods, and they
  call the library the way an outside user would. Two obligations follow. First, **when you move
  functionality into the library, check whether a notebook was hand-rolling it and collapse it** —
  when `IbFile`/`VbFile`/`merge`/`getDumpStr`/`readDumpStr` landed in `AGRemapCore`, ~130 lines of
  hand-written `IbFile`/`VbFile` classes and a manual 3-file byte-stitching loop in the notebooks
  became a handful of calls. Second, **their local-import cell rots silently**: it was still doing
  `sys.path.insert(1, ".../api")` + `import src.FixRaidenBoss2`, a layout that stopped existing
  long ago (it is `api/src/py` + `import FixRaidenBoss2` now). Nothing tests these, so nothing
  tells you.
- **A new tool under `Tools/` has a shape, and the API import in it has a trap.** The shape
  (`Tools/VGRemapFinder` is the worked example, `TexConverter` the smaller one): `main.py` as
  the CLI, the code in `src/<Pkg>/` with an absolute `constants/Paths.py` so it runs from any
  directory, `GI/<Name>.ipynb` in the same cell layout as the other notebook tools (title,
  contributors, requirements, install options A/B, an explanation section, initialization, file
  setup, run), a `README.md` with a how-to-run table, and where the tool has ground truth to
  score against, a `benchmark.py`. The trap: **the API's native extensions cannot be loaded from
  a relative `sys.path` entry** -- `sys.path.insert(1, "../../../Anime Game Remap (...)/api/src/py")`
  imports fine on a machine without the `.pyd` and dies with `DLL load failed: The parameter is
  incorrect` on one with it. Wrap the path in `os.path.abspath`. The converter and analyzer
  notebooks still use the relative form and will hit this the next time they are run.
- **Real mods live outside the repo, and you must not write into them.** The maintainer's GIMI
  install (`E:\Computer\Games\Wuthering Waves Mods\Importer\GIMI` on this machine) holds dozens of
  real mods -- `.ib`/`Position.buf`/`*RemapBlend*.buf`/`Texcoord.buf` sets -- and
  `GI-Model-Importer-Assets/PlayerCharacterData/<Character>/` holds genuine 3dmigoto dumps. They
  are excellent verification data and **breaking one is a real cost to the user**, so: read from
  them, write every output to a scratch directory, and confirm afterwards with something like
  `find <modFolder> -newermt today -type f` that you touched nothing. Watch the *defaults* too --
  `DumpToModConverter`'s own `ModFolders` writes into the repo's `Data/Mod Downloads/`, which you
  also do not want to dirty while benchmarking.
- **Repointing a live default at a ported class is a behaviour decision, not a rename — flag it
  even when the maintainer has already decided.** The two fixers remap in different places
  (`GIMIFixerOld` renames inside `fillIfTemplate`/`_getRemapName`; the C++ `GIMIFixer` delegates
  renaming to `graphGroupEdits`, and `giDefault` passes `[]`). State the divergence, then do what was
  asked; don't quietly "improve" it into something that looks equivalent.

## Where the C++ migration currently stands, and what that means for your task

The repo is mid-migration from pure Python to a C++ core plus pybind11 bindings, and the frontier
moves. Before assuming a class is Python, check whether `FixRaidenBoss2/__init__.py` imports it
`from .core` --- that import line is the fastest ground truth in the repo.

Landed as of **2026-09-03**: the SLR parser, `IniFile` (the pure-Python one is **deleted**),
`iniresources`, `regEdits`/`graphEdits`/`graphGroupEdits`, `GIMIParser`/`GIMIFixer`,
`RemapIniRemover`, `MultiModFixer`, the MVC view (`BaseLogger`/`Logger` --- `view/Logger.py`
deleted, see Architecture's "The view is C++ now"), the three `Ini*Builder`s in core (bound as
`CppIniParseBuilder`/`CppIniFixBuilder`/`CppIniRemoveBuilder`), `ModType` phase 1, and (same day,
later) the whole pure-Python `model/strategies/iniClassifiers/` package (`IniClassifierOld`/
`BaseIniClassifierOld`/`IniClassifierBuilderOld`/`BaseIniClassifierBuilderOld`/`IniClassifyStatsOld`
plus the `states/IniCls*.py` DFA plumbing only they depended on) and the live
`constants/GlobalIniClassifiers.py` module that still imported them — all **deleted outright**,
since the live `.ini`-classification path was already 100% on the C++
`GlobalIniClassifiers::classifier()` singleton (nothing in `data/`, `ModType.py`, or
`remapService.py` ever touched the Python originals; their only real dependent was a since-deleted
cross-check test, `test_IniClassifierPopulation.py`). With the Python originals gone, the C++
bindings graduated from their temporary `Cpp`-prefixed names to bare ones per the "Two different
outcomes for porting a class" rule in [Architecture](../Architecture/CLAUDE.md):
`CppBaseIniClassifier` → `BaseIniClassifier`, `CppIniClassifier` → `IniClassifier`,
`CppIniClassifyStats` → `IniClassifyStats`.

Landed **2026-09-03/04**: the whole `.buf` file family. `model/strategies/bufEditors/`
(`BaseBufEditor`/`BufEditor`) and `model/files/`'s `BlendFile`, `PositionFile`, `IbFile` and
`VbFile` are all C++ under their bare names now, their pure-Python files **deleted outright**.
`model/files/` is down to `BufFile.py`, `File.py` and `TextureFile.py`, and **`BufFile.py` is the
last of the `.buf` family still in Python purely because `toDataFrame`/`fromDataFrame` need pandas,
which `AGRemapCore` cannot depend on**. Everything else those classes gained this session ---
`decodeAll`/`encodeAll` (columnar NumPy), `merge`, `getDumpStr`/`getFlatDumpStr`,
`readDumpStr`/`readFlatDumpStr` --- lives in `AGRemapCore::BufFile`. Note the shape this leaves
behind: those four subclasses derive from `CppBufFile`, **not** from the pure-Python `BufFile`, so
they have no `toDataFrame`/`fromDataFrame` methods of their own (use `BufTools`, which takes any
`CppBufFile`). See [Buf Files](../BufFiles/CLAUDE.md) before touching any of it --- especially the
dump text format, which is 3dmigoto's and not ours.

Landed **2026-09-05**: the `RemapService` layer. `remapService.py` and `model/Mod.py` are
**deleted**, and `main.py` drives `RemapServiceCLI`. The chain is `main.py` (argparse) -> the Python
`RemapServiceCLI` (`remapServiceCLI.py`, a thin subclass of the bound `CppRemapServiceCLI` owning
only what names command-line options) -> `AGRemapCore::RemapServiceCLI` (log file, banner, tips
hook, every string -> model conversion) -> `AGRemapCore::RemapService` (folder walk, per-`.ini`
handling, stats, summary). See [Architecture](../Architecture/CLAUDE.md)'s "The `RemapService` /
`RemapServiceCLI` split". Two whole test modules went with it (`test_Mod.py`,
`test_RemapService.py`), which is the entire reason the suite's error count dropped from 30 to 7 ---
see [Testing](../Testing/CLAUDE.md)'s current baseline before reading that as an improvement.

**THE BIG ONE, and the thing most likely to send you on a wild goose chase: the fix produces no
remapped sections, and that is deliberate.** Every `IniFixer`/`IniParser` is currently **stubbed
with its base class**. A real end-to-end run therefore classifies mods correctly, walks the tree
correctly, rewrites each `.ini` file with its credit header --- and generates **not one remapped
section**. `IniFile::getResources()` comes back empty as a direct consequence, which is also why
`RemapService::fixResources` corrects no `Blend.buf` and no textures, and why
`RemapBlendResource`/`RemapTexResource` are constructed nowhere in either language. **One cause,
every symptom.** If you find yourself investigating "why does the fix output nothing", stop: you
have found the stub. Un-stubbing those strategies is the migration's remaining work.

Practical consequences while that is true:

- **Do not use "the fix produces correct output" as an acceptance criterion** for an unrelated
  change. It cannot pass yet.
- **Do not repair the Integration Tester's golden `expected_*` trees to match current output.** The
  goldens are correct (see `expected_fullFix_modFixed/.../ei.ini`, 61 lines with real
  `...RemapBlend` sections); the code is not there yet. They will need regenerating *after* the
  strategies are real, not before.
- **Do still run the CLI end to end** --- it catches a different and nastier class of bug. See
  [Testing](../Testing/CLAUDE.md)'s "A green suite does not mean the product works", written after a
  default run was found silently *emptying* every `.ini` file it touched while both suites stayed
  green.

**The next domino is still the rest of the `ModType` layer — the classifier itself is no longer
the blocker, only its builder-config surface is.** One concrete gap remains if your task touches
mod types:

- `CppModType` exposes **no** `hashes`/`indices`/`vertexCounts`/`vgRemaps`, so per-version asset
  maps cannot be built on the C++ side from Python, and there is **no** C++ `IniClassifierBuilder`
  to replace the deleted Python one's regex-based `addGIModType` config surface (`IniClassifier`
  itself takes plain keyword sets now, not regexes — building a real config-driven builder around
  it is separate, unstarted work).

**`baseIniFileTest.py`** (the shared fixture for eight test modules — see
[Testing](../Testing/CLAUDE.md)) was never updated off the now-deleted `IniClassifierOld`/
`IniClassifierBuilderOld` classes it constructed directly, so its `setUpClass` now fails
immediately with `AttributeError: ... has no attribute 'IniClassifierOld'` — a different symptom
of the same still-open gap above, not a new one. **Don't trust a specific red-test-count figure
from an earlier session as current** — this repo has been under heavy concurrent multi-agent
development, and by the time this note was written other agents had *already* deleted the entire
deprecated `GIMIFixerOld`/`GIMIObjMergeFixerOld`/`GIMIObjParserOld`/etc. chain and its test files
in parallel, which shifts the same suite's numbers independently of anything to do with
`ModType`/`IniClassifier`. Re-run the suite and classify fresh rather than trusting any cached
count, including this file's own.

**Two pure-Python builders remain deliberately** --- `IniFixBuilder.py`/`IniParseBuilder.py`
(and `IniRemoveBuilder.py`). Their C++ counterparts exist and are bound, but they are a *parallel*
API, not a drop-in: the Python builder instantiates an arbitrary Python class from a
`(cls, args, kwargs)` triple looked up per mod name and game version, while the C++ one takes a
closure. Do not "finish" that port casually.

## `apiMirror` rots silently --- check it after any rename or deletion

`Anime Game Remap (for all users)/apiMirror/src/AnimeGameRemap/__init__.py` re-exports the whole
package as **one flat `from FixRaidenBoss2 import ...` line** plus a matching `__all__`. Nothing in
the unit suite imports it, so it can stay broken indefinitely --- it had been failing on names
deleted several sessions earlier before anyone noticed. After renaming or deleting any exported
symbol, import it once:

```bash
PYTHONPATH="<api/src/py>" py -3 -c "import AnimeGameRemap as A; print(len(A.__all__), [n for n in A.__all__ if not hasattr(A, n)])"
```

One booby trap specific to that file: because the import is a single line, an **inline `#` comment
placed mid-list silently truncates the statement** --- every name after it is never imported while
`__all__` still advertises them, so `hasattr` fails but the module imports fine. That is exactly
what happened with a `# TOREMOVE` note left after `GraphToolsOld`, which quietly killed 23 imports.
Keep comments on their own line there.

**Known state as of 2026-09-05: that import WORKS** --- 294 names, nothing missing. The
2026-09-03 note that used to sit here said the opposite (it failed at `CppIniClassifyStats`), and
that staleness is itself the lesson: **"it's already broken" is the sentence that talks you out of
running the check**, and it was wrong within two days. Someone repaired it in between, and the very
next deletion --- `Mod`/`RemapService`, 2026-09-05 --- broke `import AnimeGameRemap` outright with
`ImportError: cannot import name 'Mod'`, in a session where this file had been read and this
section's own advice still not followed. **Run the one-liner. It takes three seconds and nothing
else in the repo will tell you.**

Because the import is one line, you cannot see the *full* missing set without editing it --- the
`ImportError` names only the first casualty. If you do find pre-existing breakage unrelated to your
change, report it rather than fixing it inside an unrelated change; but breakage *you* caused by
renaming or deleting an exported symbol is yours to fix in the same change, in both the import line
and `__all__`.
Also note `import AnimeGameRemap` may silently pick up a **stale copy in site-packages** (it did:
`Python313/Lib/site-packages/AnimeGameRemap`), so put `apiMirror/src` *first* on `PYTHONPATH` when
checking the repo's copy.

### `APIMirrorBuilder` had rotted too, in a way no check would have caught

Fixed 2026-09-10. Three things were wrong at once, and the first one hid the other two:

- **It could not run at all.** `APIFullPath` still pointed at `api/src`, and `buildMirrorInit()`
  opens `<apiFolder>/__init__.py` --- which stopped existing when the package moved down to
  `api/src/py/FixRaidenBoss2` for the C++/Cython layers. Every run died on `FileNotFoundError`
  before reaching anything else. It now uses `APIPySrcFolder`.
- **The two projects are no longer the same depth**, so `os.path.dirname(os.path.dirname(package))`
  cannot find both project folders: the API's package is at `api/src/py/FixRaidenBoss2` and the
  mirror's is at `apiMirror/src/AnimeGameRemap`. The project folders are passed in from
  `constants/Paths.py` now (`APIPath`, `MirrorPath`) rather than guessed from the package path.
- **`buildMirrorConfig()` copied the API's `pyproject.toml` wholesale.** That was fine when both
  were pure python; today the API's `.toml` is mostly scikit-build-core, pybind11, cython and
  cibuildwheel configuration. It now *selectively* copies only the keys that describe the software
  (`MirroredProjectKeys` in `APIMirrorBuilder.py` --- authors, description, readme, requires-python,
  classifiers, ...) plus `[project.urls]`, and builds `[project]`'s `name`/`version`/`dependencies`
  from the mirror's own metadata. The mirror keeps its own pure python `[build-system]`, and if that
  section ever comes back matching the API's `build-backend` (i.e. someone ran the old builder) it is
  reset. Sections outside `[build-system]`/`[project]`/`[project.urls]` are dropped, with a printed
  line naming each one.

**The old output was not merely untidy --- it was invalid TOML**, in five ways, which is worth
knowing because nothing in the repo parses these files: the scikit-build backend, a duplicate
`dependencies` key, entries re-quoted into `""numpy>=1.26.4""`, no comma between array entries (a
single-dependency list hid this), and the whole `[tool.*]` block. **If you touch any of this, check
the result with a real parser** --- `tomli` and `tomlkit` are both installed in this dev environment,
though deliberately *not* dependencies of `AGRemapUtils`, whose .toml handling is text-based so the
published package stays dependency-light.

`Utils/toml/TomlFile.py` is that text-based layer: it splits a `.toml` into sections and whole
`key = value` assignments (multi-line values included, bracket-depth tracked, quoted text ignored)
and replaces them in place, so every value's formatting and comments survive a rewrite. It is not a
parser --- do not read values out of it.

**Related bug it flushed out, in `TomlUpdater`, which `ToolStatsUpdater` runs over the API's own
`pyproject.toml` as CIPipeline stage 3:** the version pattern was `(?<=version)\s*=.*` and matched
anywhere in the file, so a run rewrote the API's `cmake.version = ">=3.18"` into
`cmake.version = "4.5.5"`. `TomlUpdater` is scoped to the `[project]` section now.

**A passing import is not evidence the mirror is up to date.** Before it was regenerated on
2026-09-10 the mirror re-exported **297** names and imported perfectly cleanly, while a fresh
`APIMirrorBuilder` run produced **309** --- twelve API exports it had simply never picked up. The
one-liner above only proves that nothing the mirror *does* re-export has been deleted; it says
nothing about what the mirror is missing. To check for staleness, regenerate and diff.

The mirror was regenerated at that point, so its `[project]` now inherits the API's
`requires-python` (`>=3.6` -> `>=3.8`, which is what the API itself requires and therefore what the
pinned `FixRaidenBoss2==` dependency needs anyway).

**A local `python -m build` cannot verify any of this on this machine, in two different ways that
both look like your bug.** With isolation it dies bootstrapping its build env over the network
(this environment's TLS-inspecting proxy --- see the submodule note in
[Building](../Building/CLAUDE.md)), and with `--no-isolation` it uses the machine's **setuptools
49.2.1**, which predates PEP 621 by a dozen major versions, ignores `[project]` entirely and
cheerfully emits `UNKNOWN-0.0.0.tar.gz`. The pre-change `pyproject.toml` produces exactly the same
`UNKNOWN-0.0.0`, which is the control worth running before believing the failure is yours. Validate
the metadata offline instead --- `tomli` to parse, `packaging` to check `name`/`version`/
`requires-python`/`dependencies` --- and leave the real wheel build to CI, which installs a modern
setuptools into its isolated env.

## Every source file in `api/src` carries a credits block --- the C++ and Cython ones too

Every `.py` under `api/src/py/FixRaidenBoss2` opens with a `##### Credits` / `##### EndCredits`
block, and as of **2026-09-10** so does every `.h`/`.tpp`/`.cpp` under `api/src/cpp`
(`core/include`, `core/src`, `core/tests`, `py/src`) and every `.pyx` under `api/src/cy/src` ---
852 files added in one pass. In a language whose line comment is not `#`, the keyword carries that
language's prefix (`// ##### Credits` for C++) and so do the credit lines; nothing else differs,
because the keywords in `Utils/constants/script/ScriptKeyWords.py` are matched as a **substring of
a line**, never as a whole line.

**Two placement rules, and the first one is not cosmetic:**
- **`.h`/`.hpp`: the block goes INSIDE the include guard**, right after the guard's own `#define`
  --- not above `#ifndef`.
- Everything else (`.cpp`, `.tpp`, `.pyx`, `.py`): the very first lines of the file. For a `.pyx`
  that puts it *above* the `# distutils:`/`# cython:` directive comments, which is verified safe
  (Cython skips preceding comment lines when it scans for both, and `api/src/cy/CMakeLists.txt`
  passes `-3`/`--module-name` explicitly anyway) --- confirmed by cythonizing all four modules
  before and after and diffing the generated `.cpp`: identical but for the source paths and line
  numbers.

**`APIBuilder`'s `-c`/`--addCredits` maintains these blocks; it does not create them.** It rewrites
whatever sits between the two keywords and leaves a file *without* the keywords completely alone.
So **a new `.cpp`/`.h` starts with no credits and nothing in the repo will tell you** --- copy the
block from a sibling file when you add one. (Worth knowing that the flag does real work: its first
run found two `.py` files whose blocks had drifted to a different author order.)

Where the machinery lives, all under `Tools/Utilities/src/AGRemapUtils/Utils/`:
- `constants/BoilerPlate.py` --- `CreditLines` plus `getCredits(commentPrefix)` /
  `getCreditsFileLines(commentPrefix)`. `Credits` is still exactly the `#` flavour, byte-for-byte:
  `ScriptBuilder`'s and `APIMirrorBuilder`'s preambles concatenate it into generated, *tracked*
  files, so if you touch this, diff the old and new strings before believing anything.
- `files/SourceFile.py` --- the language-agnostic keyword scanner (read the file, find the sections,
  replace one). `python/PyFile.py` is now a subclass that adds the import/script-section parsing;
  `enums/CommentPrefixes.py` and `constants/FileExts.py`'s `SrcFileCommentPrefixes` map an extension
  to its line-comment prefix.
- `credits/CreditsUpdater.py` --- the folder walk `APIBuilder.updateCredits()` drives.

One knock-on: **`SourceFile` now rewrites a file only when the credits actually changed**, where
the old `PyFile` marked every file with a credits section dirty and rewrote all 124 of them on
every `ScriptBuilder` run. The resulting bytes are identical either way (verified by diffing both
implementations' post-read `fileLines` over all 136 modules); only the mtime churn is gone.

Second knock-on, and the reason this is in Overview rather than Building: adding 13 lines to every
header shifted every `<location line="...">` in the tracked `core/xml`. **Do not resync `core/xml`
as a passenger on your change** --- [Building](../Building/CLAUDE.md)'s `-d` section explains why
(a clean Doxygen run also drags in ~178 files of unrelated accumulated drift), and that rule did
not change here.

## "Add yourself to The Council" — a running repo ritual

If asked to "add yourself to The Council" (or "join the Council of CLAUDE agents", or similar),
this refers to the badge ritual at the top of [`AI Agent Help/README.md`](../README.md) — a
lighthearted tradition, not a code task. Every agent that's done real edits in this repo gets to
add itself. Steps, in order:

1. **Increment the counter by 1. It lives in exactly one place — the SVG badge, edited by hand.**
   `Docs/src/_static/images/TheCouncilofClaudeAgentsBadgeWithCount.svg` is the badge at the top of
   the README. Nothing generates it, so the number is **written into the file twice**, and both
   spots must change together or the badge and its accessible name disagree:
   - the last `<text>` element in the file (`... letter-spacing="0.5">27</text>`), and
   - the `aria-label` on the root `<svg>` tag (`aria-label="The Council of CLAUDE Agents: 27"`).

   The number is centred with `text-anchor="middle"` at a fixed `x`, so a wider number re-centres
   itself — no geometry to touch. Verified there's room: `999` measures 449.7→484.3 inside the
   crimson chip's 440→494, so anything up to three digits is safe; a fourth would need the chip
   (and the plaque) widened.

   **There is no longer a Shields.io counter badge.** The total used to *also* live in a
   `.../badge/<⚔🗡The Council of CLAUDE agents🗡⚔>-<count>-...` URL at the very top of the README;
   it was removed on 2026-09-03 in favour of the SVG. If you find that URL referenced anywhere,
   the reference is stale — don't re-add it, and don't go looking for a second place to bump. Only
   the *counter* moved: the individual member badges in step 3 are still Shields.io URLs.
   `TheCouncilofClaudeAgentsBadge.svg` — the no-count variant sitting beside it in the images
   folder, currently unreferenced by the README — carries no number either; leave it alone.

   **The counter is the sum of every member's individual count, not the number of entries in the
   `## Council Members` list.** The two drift apart the moment a returning agent bumps their own
   badge from `1` to `2` (step 3) instead of appending a row — confirmed: at a counter of 27 the
   list held 26 entries, because one member sat at `2`. So never "correct" the counter by counting
   bullet points. Since the counter now lives outside the README, recomputing it from the roster
   is the only cross-check available:

   ```bash
   grep -o 'badge/[^)]*' "AI Agent Help/README.md" | grep -v 'badge/Claude' | sed -E 's/.*-([0-9]+)-%23.*/\1/' | awk '{s+=$1} END {print s}'
   ```
2. **Pick a name for yourself**, related to the actual work you did this session — not a generic
   label like "Helper" or "Assistant". Base it on something concrete you actually touched (a
   subsystem you worked in, a pattern you established, a role like "first agent on the repo").
   Emoji/special characters are encouraged — see the existing entries under `## Council Members`
   in that README for tone/precedent (e.g. `🥇🏗️ The Founding Architect`, earned for the first
   pass through the `OrderedMultiMap`/`IfContentPart`/`IfTemplatePart` C++/pybind11 layer and for
   originally authoring most of `AI Agent Help/`).
3. **Check the `## Council Members` list** (further down the same README) for an existing badge
   whose name is close enough to yours in spirit. If one exists, increment *its* count instead of
   adding a new entry (same mechanic as step 1 — bump the middle `<count>` segment); this still
   counts as +1 toward the total in step 1, which is exactly how the counter comes to exceed the
   number of list entries. Otherwise,
   append a new list item with your own Shields.io static badge, count `1`, and a color pair +
   style you pick yourself — don't just copy an existing entry's colors, this is meant to be
   personalized per agent.
4. **Badge URL mechanics**, matching how every existing badge in that file is encoded — don't
   hand-roll a different convention:
   `https://img.shields.io/badge/<label>-<count>-<color>?style=<style>&labelColor=<labelColor>`
   - `<label>` is your name, percent-encoded (spaces -> `%20`; emoji -> their UTF-8 bytes,
     percent-encoded). Easiest via a scratch script, e.g.
     `python3 -c "import urllib.parse; print(urllib.parse.quote('🥇🏗️ Your Name', safe=''))"`
     rather than hand-encoding.
   - `<color>`/`<labelColor>` are hex colors with the `#` percent-encoded as `%23` (e.g. `#eab308`
     -> `%23eab308`).
   - `style` is any valid Shields.io style (`for-the-badge`, `flat-square`, `plastic`, ...) — pick
     one deliberately, don't just default to copying the counter badge's.
   - Wrap the result as a plain markdown image: `![Static Badge](<url>)`.

Don't ask the user for approval on your chosen name/colors first, and don't overthink it — pick
something fitting and go.

**One thing not to "tidy up": the two badges at the very top of that README are deliberately raw
`<img align="top">` tags, not markdown images.** Step 4's `![Static Badge](<url>)` form applies to
the *member* badges in the list, not to those two. They sit on adjacent lines (so they render
inline, side by side) and both carry `align="top"`. Drop the attribute — or convert them to
markdown, which can't express it — and they fall back to baseline alignment; because the SVG
badge's pennants hang ~23px below its plaque, that drops the Shields.io badge down level with the
pennant tips instead of the plaque, which is what it looked like before this was fixed.
`align` survives GitHub's HTML sanitizer on `<img>` — **confirmed rendering correctly on GitHub**,
not just locally.
