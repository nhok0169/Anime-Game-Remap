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
- **`master`** — the main/release branch. It was called **`nhok0169`** until 2026-09-18, when
  `development` was merged into it and it was renamed; since that merge it carries the C++ core and
  the submodules too. Older notes, commit messages and issue threads still say `nhok0169` --- read
  that as `master`. **Downloads are the catch.** GitHub's redirect for a renamed branch covers the WEB pages
  (`/tree/nhok0169/...`, `/blob/...`) and **not raw file downloads**: `github.com/.../raw/nhok0169/...`
  returns **404** (checked 2026-09-18), while `.../raw/master/...` redirects to
  `raw.githubusercontent.com` and serves the file. Every package released before the rename downloads its
  assets from `.../raw/nhok0169/Data/Mod%20Downloads`, so **their downloads broke with the rename**, and
  so did `master`'s own until the URL change in `DownloadTools.cpp` / `FileDownloadData.py` reached it ---
  the first CI run on `master` after the rename failed 5 Integration Tester tests on nothing but 404s.
  What restores the released versions is a branch named **`nhok0169`** holding `Data/Mod Downloads`;
  it can stay frozen at the rename point, since an old release only asks for files it already knew
  about. **Check a URL in the exact form the product fetches** --- a `tree/` page redirecting proved
  nothing about `raw/`, and that one untested step is what this paragraph once got backwards.
- **`development`** — the active development branch (what you're usually on; branch new work off
  this, not off `master`).

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

> **A LABEL IS NOT THE THING IT NAMES --- THE SAME MISTAKE THREE TIMES IN ONE DAY
> (2026-09-12).** Three separate wrong conclusions in one session, each from trusting a
> human-authored string as if it were data:
>
> | the label | what it was taken to mean | what it actually was |
> | --- | --- | --- |
> | an A/B directory named `c6_`/`f6_` | "before and after the flag" | two snapshots both taken AFTER |
> | `ModTypeId::AyakaSpringbloom` vs `getName` `AyakaSpringBloom` | the same key | a silent lookup miss, and 7 invented "improvements" |
> | `ResourceLisaStudentHeadDiffuse` | a diffuse texture | her NORMAL MAP, under an old name |
>
> The third cost the most: it produced a confident bug report against the old script
> ("it deletes the diffuse") and a `RegValChecks` guard written to fix a bug that was not
> there, which then had to be reverted. **Names in this codebase are chosen by people ---
> modders, past agents, the maintainer --- and the things they name move without them.**
> Before a name becomes evidence, ask what would confirm it independently: the register
> POSITION for a texture, the config source for a snapshot, a round-trip through the real
> lookup for a key. This is habit 1's family again: a wrong label, like a no-op, reads
> exactly like a right one.
>
> **A COUNTER THAT DISTINGUISHES THINGS THAT CANNOT DIFFER (2026-09-12).**
> `TexCreate::getFixResourceName` was deliberately impure --- every call advanced a counter so
> that successive textures got distinct names, documented as "the pure-Python original's own
> behaviour". Correct for a texture EDIT, where each source texture makes a different output.
> But edits are `TexReplace`; `TexCreate` has exactly one subclass, it holds ONE `TexCreator`,
> and a `TexCreator` takes a width, a height and a colour and reads no source file at all.
> Everything it made was identical by construction, so a mod binding the register in four
> `$swapvar` branches got four byte-identical 4MB `.dds` files. **When something is impure or
> stateful "to tell instances apart", check that the instances CAN differ** --- and check it
> against the class that actually owns the state, not the family it is filed under. This is the
> empty-input habit's mirror: there, a mechanism with no data; here, a mechanism whose data
> cannot vary.
>
> **A LIVE FEATURE WITH AN EMPTY INPUT (2026-09-12).** Habit 1 is code that runs and does
> nothing. This is its quieter cousin: code that is *correct*, *reachable*, *tested*, and never
> handed any data. `IniClassifier::addGIModType(modType, hashes, sectionKeywords)` weighs a hash
> hit at 2 and a section-name hit at 1, builds DFA states for the hashes, and already declines to
> count a hash inside a `Remap`-named section --- all deliberate, all documented. And for the whole
> life of the C++ classifier the only GI call site passed `{}` for `hashes`, so none of it ever
> ran for a Genshin mod. The comment there explained why, and was right at the time: *"Passing
> ModType::hashes here would be a behaviour change, not a port."* A port decision that outlived
> the port. **When something does not work and the mechanism for it appears to exist, check what
> the call site actually passes before concluding the mechanism is missing** --- and when you
> write "not a port" in a comment, you are filing a TODO that nothing will ever remind you of.

> **A ZERO IS A CLAIM ABOUT TWO INPUTS (2026-09-12).** "Nothing changed" and "I compared the
> wrong two things" print the same number. A `moveDrawIndexed` flag was reported as a no-op off a
> 9-of-9-identical diff whose two sides had both been generated after the flag was already on; the
> real before/after differs in all 9. Before believing a zero, `grep` one side for the thing you
> expect to be absent from it. This is the same failure as habit 1 seen from the other end: there,
> code that runs and does nothing reads as success; here, a measurement that compared nothing reads
> as a result.
>
> **AND SO IS A NON-ZERO --- READ THE ROWS, NOT THE TOTAL (2026-09-12, same day).** The same
> measurement run the other way round is just as wrong and much more flattering. A before/after of
> the .ini classifier reported **16** files improving; seven of them were an artifact of the
> before side, which looked its keyword table up by `getName` (`AyakaSpringBloom`) while the table
> is keyed by the enum (`AyakaSpringbloom`) --- one letter, and that character silently had no
> keywords at all on the before side, so of course she "improved". The real number is 9. Nothing
> in the total gave it away; what did was reading one changed row and finding it absurd --- the
> sections in that mod are literally named `TextureOverrideAyakaSpringBloomPosition`, so "names
> alone could not identify it" could not possibly be true. **Spot-check a changed row against the
> raw input before you quote the count**, especially when the count flatters the change you just
> made.

Written after several sessions where the *diagnosis* cost far more than the fix. None of this is
about the domain -- it is about how this particular codebase fails.

**1. Assume the failure is silent, and go looking for it.** This repo's dominant bug shape is not a
crash or a wrong value; it is a code path that runs, logs success, and does nothing. Confirmed
examples, all found the hard way: a fixer that renamed sections and wrote no remapped geometry; a
texture edit that logged `Editting texture for X.dds` and produced no file; downloads written into
the `.ini` and never fetched; a `.buf` copied without being remapped. **So "the run succeeded" is
never evidence.** Check the artifact -- the file exists, its bytes changed, the count went up.

**1b. The same shape wearing a number: a counter that CANNOT be wrong reads exactly like a
counter that happens to be right (2026-09-10).** Two of this program's own summary lines were
saying nothing for weeks. *Out of 40 download requests ... copied **0** files from existing
downloads* reads as "this mod had no repeats" and actually meant the download cache had been
unreachable since the strategy builders were de-flyweighted --- one texture fetched from github
36 times in a run. *fixed **1** Blend.buf files* was a `std::set` keyed by path, so it could not
distinguish one file from one file remapped twice --- which a merge does by construction. Both
were found by **counting the log lines and comparing**, never by reading the summary. When a
number looks right, ask what range it is even capable of taking.

**1c. And a check only sees what its own shape lets it see.** `check_dangling.py` follows a
`filename =` to the disk, so a register naming a resource section *nobody defines* passes it
silently --- there is no filename to follow. That shipped a CherryHuTao body with no lightmap,
reported in game as a completely different defect. The answer was a second check with a
different shape (`check_sections.py`), not a better version of the first. **When a check passes
on something you know is broken, ask what it is structurally unable to look at.**

A cousin of the same mistake, in a check written that same day: counting log lines by file
**basename** reported four mods as doing duplicate work, because six NingguangOrchid subfolders
each write their own `NingguangOrchidNingguangRemapBlend.buf` --- six files, not one file six
times. Key by the thing that is actually unique (here, folder + name) before believing a count.

**This has now happened THREE times, and the third one cost the most (2026-09-11), so treat the
pattern as the rule rather than the exception.** An AyakaSpringbloom texture edit was reading the
wrong object's texture and rendering her neck pale in game, through several rounds of "0 differ,
0 undefined, 0 dangling". Every check was honest and every one was blind:

* `cmp_binaries.py` selected files whose names END in `remaptex.dds` --- but an edited texture's
  name ends in a HASH (`AyakaBodyRemapTexMzY IMY.dds`), so it had never compared a single one.
* the baseline the comparison ran against was not pristine: `--undo` leaves generated BINARIES
  behind, so files written by an earlier run of the NEW script sat on the OLD side, being diffed
  against themselves.
* and the defect itself is invisible to a ``.ini``-level check by construction --- the file is
  internally consistent whichever texture the edit reads.

**Before trusting a green check, state what it is physically looking at and confirm the thing you
changed is in that set.** "It passed" and "it looked" are different claims, and this repo
punishes conflating them.

**1d. An upstream data source can be wrong, and can CORRECT itself later -- check its history
before you correct it yourself (2026-09-11).** Kirara's face rows in `HashData.cpp` had her face
diffuse filed as a `tex_face_normalmap` with the shared face LIGHTMAP hash sitting in the
diffuse's place, which read like a transcription slip. It was not: the 4.3/4.4 dumps of
`GI-Model-Importer-Assets` really do say that, and the repo fixed itself later in a "Characters
re-dump" commit that also stripped phantom face entries off **54** characters. `git log -p` on
the asset file settled in one command what reading the table could not.

**And the maintainer's standing policy is to FOLLOW that repo even where it is known to be
wrong**, recording the doubt rather than diverging --- see the pre-migration `HashData.py`'s own
notes on LisaStudent's ps-t0 ("in actuality, this a normal map"), ShenheFrostFlower's invented
normal maps ("Im just going to follow what GIMI assets has even though I know it is wrong") and a
`tex_dress_shadowramp` whose value is the string `000050-ps-t3`. Following the assets to a NEWER
state of the same file is within that policy; overruling them from your own reasoning is not.

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
| references | `Tools/Misc/Diagnostics/check_dangling.py` -- every `filename =` exists on disk |
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

**The cheapest possible instance of that, found 2026-09-11: two hash values in `HashData.cpp` had
SPACES inside them** --- `"29cf09   14"` for Nilou's dress lightmap, `"b0e089    15"` for
GanyuTwilight's. A hash is eight hex digits, so neither matched anything and both dress lightmaps
were silently never remapped. They are **not** a migration error: the identical typos sit in
`FixRaidenBoss6.py` (lines 4992 and 5122), and the generated table carried them faithfully --- which
is precisely why **no A/B could ever have reported it. Both scripts agreed, and both did nothing.**

Two things follow. First, `HashData.cpp` now documents a narrow carve-out from its own
follow-the-assets-repo policy: a value that could not have come from any dump at all (a typo, not a
wrong hash) is ours to repair, and each repair names its evidence. Second, the check that finds this
class of defect is not an A/B but a SHAPE SWEEP over the data --- every value should match
`^[0-9a-f]{8}$`, with ShenheFrostFlower's two `000050-ps-t3` shadow ramps as the only intended
exceptions. Thirty seconds, 873 rows, and it is the only thing that would have caught it. Run one
after any bulk edit to a data table, and ask what shape the values in it are supposed to have.

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

**24. ONE mod per character is overfitting, and the maintainer will tell you so.** Every remap in
the 2026-09-11 batch was built and proven against a single mod folder per character, which is how a
texture-naming change came to look finished when it was not. Pointed at four different
AyakaSpringbloom mods from `Importer/GIMI/` instead of the one it was written against,
`AyakaSpringBloom3` immediately showed a case the other three do not have: two DIFFERENT edit names
over one source texture, producing identical bytes under different file names. **The wider library
at `Importer/GIMI/` holds several mods for most characters** -- `ls | grep -i <character>` -- and
they cost nothing extra to run. Use them, especially before believing a fix to something structural
is complete.

A second reason, learned the same day: **the maintainer rotates mod folders in and out of `Mods/`**,
because 3dmigoto tolerates only one mod per character there. A path that worked an hour ago can
simply be gone, and `ab_any.sh`'s `FATAL: source mod folder does not exist` guard exists for exactly
that -- without it the whole harness runs over empty directories and reports "(identical), 0
dangling", which reads as a pass.

**25. When a value cannot be evidenced, measure what it AFFECTS instead -- the answer is often
"nothing here".** `NilouBreeze`'s face diffuse hash was deferred twice because no asset dump, no
pure-Python row and no mod declared it, and a wrong hash fails as silently as a missing one. What
broke the deadlock was not more searching: setting the row to `deadbeef` and running the fix emitted
`hash = 0957b10f` -- the TARGET's real value. The source row is only a SEED for `RegAssetRemap` to
replace, so in that direction it merely has to EXIST. **A deliberately wrong value is a cheap
instrument**: if the output does not change, you have learned the input does not matter, and a
question you could not answer stops blocking you.

**26. An observer that can only sample intermittently reports false negatives.** Verifying that the
CLI sets the console code page meant watching `GetConsoleOutputCP` from a second thread while `fix()`
ran. The first run reported `[437]` -- the guard never fired -- and that was **wrong**: the sampler
was starved of the GIL while `fix()` held it, so it never sampled the window at all. Later runs
showed `[437, 65001]`, every time. **A sampling check that sees nothing has two explanations, and
"it never happened" is the less likely one.** Repeat it before believing a negative, and prefer an
observer that cannot miss (a log line, a recorded value) over one that polls.

**27. "Did it restore?" cannot distinguish a working guard from a no-op.** Same task, subtler
error. Forcing the console to CP437, running, and finding 437 afterwards proves nothing on its own
-- code that never touched the setting passes that check identically. **A test of a save/restore
pair has to observe the CHANGED state in the middle**, or it is only testing that nothing happened.
The same shape applies to any scoped mutation: a lock, a temp file, a working-directory change.

**28. "It ran and printed ENJOY" is not evidence on a second platform — compare the BYTES against
the one you trust.** The first Linux run of the CLI (2026-09-11) reported a clean fix while every
`.ini` that pointed into a subfolder was silently failing to open, because a Windows-authored
`filename = .\Sub\file.buf` is one nonexistent filename on POSIX. Fixing that produced a *second*
false success: the mod was now correct, and the generated `.ini` carried `./Sub/...` into a file a
Windows game reads. **Both were invisible in the summary line, and both showed up immediately in a
content-hash comparison of the two platforms' output on the same mod** — which finished at 148/148
identical once the second half landed.

So when porting to a new environment, the acceptance criterion is not "the run succeeds there" but
"it produces the same artifact as the platform that is known good". The harness is four lines of
`sha256` over both trees, and it is the only thing that distinguishes *working* from *running*.

A corollary from the same session: **one sample is not a platform test.** The first mod tried on
Linux worked, because its `.ini` happened to use bare filenames; the second failed on every
resource. Same lesson as habit 24, one layer up.

**29. When a measurement is impossible, doubt its LABEL before you doubt the machine.** Timing the
build on two OSes (2026-09-12) produced 635 seconds for a *no-op*, which cannot be true. The first
explanation was right in kind and wrong in fact -- a Linux build WAS running concurrently, and two
builds do measure each other -- so the run was repeated on a verified-idle machine and came back
**the same**. That looked like confirmation of a slow machine. It was not:

```
$ head -3 win_noop.log
[1/3] Building CXX object ... FileService.cpp.obj      <- a no-op does not do this
[2/3] Linking CXX static library AGRemapCore.lib
```

The "no-op" had never been one. The timing script for the OTHER platform `touch`es the same
shared source on `/mnt/e`, so each platform's benchmark silently invalidated the other's build.
The true Windows no-op is **0.4 seconds**. Three lessons, in order of how much they cost:

* **A wrong label survives repetition perfectly.** Re-running an experiment tests the machine, not
  your description of what the experiment does. Reproducibility is not validity.
* **Read the log, not just the stopwatch.** Four lines of output identified in seconds what two
  ten-minute runs could not.
* **Shared state between test rigs is not always obvious.** Two OSes, one checkout, one `touch` --
  the interference ran through a file neither script mentioned by the same name.

This is the same shape as habit 26's GIL-starved sampler, and the pair is worth reading together:
in both, the apparatus was broken and the subject was fine, and in both the giveaway was a result
that could not happen rather than one that merely looked surprising.

**30. Two build sides share one checkout, and only one of them may be yours (2026-09-12).** When
another agent owns the Windows `.pyd`, every C++ change is built and verified on WSL --
`Tools/Misc/Linux/linuxBuild.sh` (native build tree on ext4, copies the `.so` into the package
folder, prints both mtimes and both exit codes) -- and the Windows module falls behind the C++.
Three habits keep that workable: **(a)** a new binding name goes into `FixRaidenBoss2/__init__.py`
inside the `try` block at its end, so the stale side still imports; **(b)** a script meant to be
run by the maintainer takes a Windows-form path and translates it (`/mnt/<drive>/...`), finds the
repo from `AG_REMAP_REPO` or its own location, and offers `--wsl` to relaunch itself inside WSL
(`Tools/Misc/Prototypes/yelanTranquilFix.py` is the pattern); **(c)** the debt is written down
where the next Windows build will pay it: `core.pyi` is behind `VGComponentSplit`,
`VGSplitGroupResource`, `BufReplace`, `RegFillMissingMode.BottomCover` and the `mipmaps` flags,
and the `__init__.py` guard comes out once the `.pyd` is rebuilt. Two mechanics of driving WSL
from the Bash tool: `wsl -d Ubuntu-22.04 -- bash -lc '...'` with the whole command single-quoted
(a `$5` inside double quotes is expanded by the OUTER shell -- an `awk '{print $5}'` arrived as
`{print }`), and a heredoc inside it works but its `$` are the inner shell's. And the Bash tool's
`/e/...` paths are Git Bash's: a Windows Python given one reports "no such file", and needs
`E:/...`. **The rule that ends all of it (2026-09-13): anything for WSL that holds a variable, a
`$(...)`, or an `/mnt/...` argument goes into a `.sh` file in the scratchpad, and the Bash tool
runs `wsl -d <distro> -- bash -lc 'bash "/mnt/c/.../that.sh"'` and nothing else.** MSYS rewrites
`$S` and `/mnt/c/...` inside the wsl arguments in ways that differ call to call -- one run got
`/cmpA` instead of the scratchpad, the next `C:/Program Files/Git/mnt/c/...` -- and every failed
form looked like a typo. Two more from the same day: a header the Windows side refuses to write
(`PermissionError`, another process holding it) is written from WSL instead -- the same
`/mnt/e/...` file, no lock; and a background Unit Tester run that prints nothing has still
written `unitTestResults.txt` -- a crash mid-suite leaves the progress dots and no summary, and
`python -X faulthandler main.py` names the test (a runner with `verbosity = 2` into a
line-buffered file names it even when the fault handler shows no frames).

**31. A one-off diagnostic that answered a question becomes a tool the same day.** Four scratch
scripts found the cape, the stockings, the port's legend and the missing mip chains; the
scratchpad dies with the session and a guide that says "sample the band under the diffuse" is
not the script that does it. `Tools/Misc/Diagnostics/modTally.py` and `boneCentroids.py` are those
scripts made generic and run once from their new home before the guides pointed at them. Do the
same for yours: if it printed the number that settled the argument, it belongs under `Tools/`.

**32. The guides' example paths are the maintainer's machine, and the copies live in
`Tools/Misc/`.** `Importer/GIMI/Mods/...` in a guide is a folder on one computer; an agent
elsewhere reads the copy (`Tools/Misc/README.md` maps each) and knows the live one may be newer.
When you write a script next to a mod because that is where it is useful, copy it into
`Tools/Misc/` before the session ends and say which is which.

**33. Ask the maintainer for the shape of the NEXT test, not just the verdict on the last.** After
three downloads had each exposed a different over-fit, the maintainer's answer was the identity
mod (Creating Remaps' "The Yelan lessons"), which covered in one folder what a fourth download
might have covered by luck. The question "what would cover the cases we have not seen" is
cheaper than the next bug report.

**34. RUN A NEW CHECK AGAINST THE BROKEN BUILD BEFORE YOU TRUST IT ON THE FIXED ONE (2026-09-14).**
This is the highest-value habit on the page and it costs one command. A check written after the fix
is a check whose failing case has never executed, and three of them passed a broken fix in one
session:

| the check | what it asserted | why it passed anyway |
| --- | --- | --- |
| "the Eye's appended draw exists and comes last" | true, and true | it `.strip()`ed each line, so it could not see the draw was nested inside `if $pubic == 1` and ran only on one toggle |
| "every member renders with the right texture" | over 5 mods | 2 of the 5 folders did not exist (the mods had moved); `analyse` returned quietly and the run printed success for 3 |
| "no section calls a fix library twice" | per SECTION | the question is per PATH — an `if`/`else if` chain has two calls and runs one |

The first shipped. The maintainer tested it in game and the eyes were still wrong. **The fix is
mechanical: keep the previous output directory, point the new check at it, and require it to FAIL
before you point it at the new one.** Doing exactly that on the fourth attempt is what finally made
the depth assertion trustworthy.

Two corollaries. **A missing input is not a pass** — a loop that walks folders must fail on one it
cannot find, not `continue`. And **check the property that would be wrong, not a property that
happens to be true**: "the draw exists" and "the draw runs unconditionally" differ by exactly the
bug.

**35. Read what the artifact DOES; do not reason about what it should do (2026-09-14).** Three
consecutive wrong diagnoses of one in-game symptom — the texture binding, then the material band,
then the geometry — each with a plausible mechanism, each contradicted by measurement within
minutes. What ended it was reading the mod's OWN `.ini` and tabulating, per slot, whether it
declares any `ps-t`. That table explained every observation at once and predicted a fourth bug
nobody had reported yet.

The generalisable form: when a subsystem has semantics (GIMI's, git's, the shader's), **find the
few lines that define the semantics and read them** — `ORFix.ini` said in six lines that `NNFix`
swaps `ps-t0`/`ps-t1`, which settled a rule a day of reasoning had not. Guessing produces a fix
per guess; reading produces one fix.

**36. This checkout is shared, and files move under you mid-session (2026-09-14).** Two things that
each cost a confused cycle:

- **Another session may COMMIT your edits.** `getComponentIds`, `makeRemapMap` and a `VGRemapData`
  correction were all swept into another session's Bennett commit. Before concluding your work is
  uncommitted, `git log --oneline -3 -- <file>`; before concluding someone reverted you, check
  whether it simply landed under a message about something else.
- **The maintainer swaps mod folders between `GIMI/Mods/` and one level up while testing**, so a
  path that resolved an hour ago resolves to nothing now. Resolve a mod by NAME across both
  locations, and fail loudly when it is in neither (see habit 34).

**37. Two build-and-tooling facts that look like your bug and are not (2026-09-14).**
`Tools/APIBuilder`'s `cleanInstalls` `rglob`s the whole tree — including other sessions' git
worktrees under `api/src/cpp/.claude/worktrees/` — and after a host crash it can die with
`OSError: [WinError 433] A device which does not exist was specified` on a path Python reads fine a
minute later. Re-run before investigating. And **`pybind11_stubgen` can exit 0 having written
nothing** when it imports a `.pyd` that was still being replaced: check `core.pyi`'s mtime, not the
exit code — the same "verify by the artifact, not the return code" rule the `.bat` traps taught.

<br>

**38. WHEN A FEATURE PRODUCES NO OUTPUT, FIND OUT WHERE ITS OUTPUT DIES BEFORE THEORISING ABOUT
WHY IT WAS NEVER MADE (2026-09-15).** A `handling = skip` section that suppresses an unremapped
component was reaching the `.ini` never. Four mechanisms were proposed against that, each plausible,
each measured, each costing a build: a hash-key arity mismatch (real -- and a different bug), the
version bucket (disproved: the lookup resolves at 6.1, 5.7 and 4.0 alike), the remover stripping it
(disproved by running undo), and the assembly path (disproved by grep). The text was being built
correctly the whole time -- 76 bytes, measured -- and a later writer in the same run replaced the
file it had been added to.

Every one of those theories was about *production*, and the bug was in *survival*. One print where
the string is built and one where the file is written separates the two halves in a single run, and
neither guess after that is needed. The same shape as habit 35 -- read what the artifact does -- one
level earlier: **before asking why a value is wrong, confirm it still exists.**

**39. DRIVE WSL THROUGH A SCRIPT FILE, NEVER AN INLINE `wsl bash -c` (2026-09-16).** On the
Windows host the Bash tool is Git Bash, and it gets to a command before WSL does. Inline, three
things break silently and differently each time: `$var` and `$(...)` inside the quoted string come
out empty (`cd "$G/Mods/..."` became `cd "/Mods/..."`), a heredoc loses its backslashes (a Python
`"\\n"` arrives as `"\n"`, so an exact-match patch finds its anchor 0 times), and a `/mnt/c/...`
argument is rewritten into `C:/Program Files/Git/mnt/c/...`. One broken heredoc also left an empty
file named `]"` in `py/src/`, which only `git status` showed. **Write the script with the Write tool
into the scratchpad and run it as `MSYS_NO_PATHCONV=1 wsl bash /mnt/c/.../script.sh args`**; write a
patch for a source file the same way and run it with `py -3`. Every command in this session that
followed that rule worked the first time.

**40. THE MAINTAINER'S LIVE MOD FOLDERS: COPY, COMPARE, MOVE -- NEVER REPAIR IN PLACE (2026-09-16).**
Test mods under `Importer/GIMI/Mods/` are the maintainer's, and a fix run can damage them in ways
the undo cannot reverse (see Creating Remaps' "Undo is only as complete as what the fix wrote
inside its block"). When asked to restore one:

- **Find a genuinely original copy first**, and prove it: no `Remap` text in any `.ini`, the
  modder's own footer still at the end, no `RemapBKUP*` or `*RemapDL*` files. A `*_pristine` folder
  or a `RemapBKUP*.txt` is NOT proof -- both were polluted by the same broken runs as the live copy.
  For Bennett3 the original was the parked `GIMI/Bennett3Test`.
- **Require every non-`.ini` data file of the live folder to be byte-identical to it** before
  touching anything; stop if one is not.
- **Move, never delete**: everything the original lacks, plus the live `.ini` files, goes to
  `GIMI/<Mod>_restoreBackup/` keeping relative paths -- outside `Mods/`, so the game does not load
  it -- and a pre-existing backup folder stops the script rather than being overwritten.
- **Verify by file list and `cmp` of every file**, and say the counts.

**41. A NEW `regEdits` / `graphEdits` / `graphGroupEdits` CLASS SHIPS WITH ITS WHOLE SURFACE (2026-09-16).**
The maintainer's standing rule, and a fixer-local graph tool that is general belongs in one of those
packages rather than in the fixer's anonymous namespace. Name it noun first, like its family
(`GraphGroupRemove`, not `RemoveGraphGroup`). The checklist, every item of which was needed once:

| where | what |
| --- | --- |
| `core/include/.../<package>/Xxx.h` + `.tpp` | the template class and its Doxygen doc |
| `py/src/.../<package>/PyXxx.{h,cpp}` | hold the EXACT Python objects given (lists, callables) and re-derive the core members at the start of each `edit`, as the siblings do |
| `py/CMakeLists.txt`, `py/src/bindings.cpp` | the source, a `void initCppXxx(pybind11::module_ &m);` declaration (not an `#include`) and the `initCppXxx(m)` call after its base |
| `FixRaidenBoss2/__init__.py` | exported inside a `try`, while the Windows `.pyd` is older than the class |
| `Testing/Unit Tester/.../test_Xxx.py` + `Tests/__init__.py` | tests, including one that reassigns an attribute and checks it takes effect |
| `core.pyi` | regenerated with `pybind11_stubgen` into a SCRATCH folder, then diffed per class -- keep it only if the diff is your classes and `__all__` |
| `Docs/src/api.rst` + `coreAPI.rst` | an alphabetical entry in the right live group |
| `core/xml` | `Tools/Misc/Docs/doxygenSplice.py --run <scratch>`, then `--apply <Class> <Header.h>` from `<scratch>/xml`: it copies your compounds' XML and splices only THEIR blocks into the tracked `index.xml`. Do not copy the regenerated `index.xml` -- on 2026-09-18 it differed from the tracked one in **438** compounds, and it names files you did not copy, so Breathe dies with `Cannot find file` on a class you never touched |

A graph edit's Python test needs its `IniSectionGraph(..., z3Ctx = ...)`: without a context, a part
outside every `if` has no query and a query-driven edit raises.

**42. BUILD THE SPHINX DOCS FROM A COPY ON THE LINUX FILESYSTEM (2026-09-16).** With the Windows
`.pyd` stale, the only build that can autodoc a Linux-only class is a Linux one -- and Breathe
reading ~1400 XML files across `/mnt/e` spent over 40 minutes on `coreAPI` alone. Copy `Docs/`,
`api/pyproject.toml`, `api/src/py` and `api/src/cpp/core/xml` to one Linux-side tree with the same
relative layout (`conf.py` resolves them relatively), and install `Docs/requirements.txt` with
`pip install --target ~/sphinxlib` on `PYTHONPATH`, so the shared dev venv is not modified. The
whole build is then about three minutes. Baseline: **16 warnings, 0 errors** (re-measured 2026-09-17
--- see [Documentation](../Documentation/CLAUDE.md) for the breakdown and why the **26** this line used to
carry is stale), almost all of them from the hand-written tutorial / examples pages; a warning or error
naming `api` or `coreAPI` is yours. Two of the 16 are intersphinx inventory failures caused by the
Windows machine's proxy, so a Linux-side build of the same tree can legitimately report **14**. Then
grep the rendered HTML for the new class, as the Documentation guide says.

**43. FIX THE WRITER, NOT THE SHARED READER (2026-09-16).** When one fixer's output is not undone,
the obvious patch is to teach the remover about it. The maintainer's answer: *the remover is the
default every mod uses, and a change for one template should not move it*. The remover was right --
it takes everything inside a fix's block, and outside it only a `Remap`-named section it can
attribute by hash -- and the fix was to write the section inside the block. The same applies to the
classifier, the parser and `GIMIFixer`'s rendering: when a template produces something the shared
machinery mishandles, first ask whether the template is producing the wrong thing.

**44. A SLOW BUILD IS A QUESTION ABOUT THE MACHINE BEFORE IT IS A QUESTION ABOUT THE CODE
(2026-09-17).** Two agents were asked to speed up the Windows build; the first tuned flags on a
24-thread Xeon and reported success, and on the maintainer's laptop the build got *slower*. Nobody
had looked at the laptop: the repo on a USB disk, 16 GB with the game holding 7 GB, 6 cores. Before
touching a flag or a file, record **CPU/RAM, which physical disk `cbuild` is on, what else is
running**, and read `cbuild/.ninja_log`. Then **compile one TU alone with `/Bt+`** (front end vs
back end) and `/d1reportTime` (headers vs class definitions vs template bodies) -- the in-build
timings are inflated ~10x by contention and say nothing about where a file's cost is. That profile
is what showed "split the slow files" was the wrong fix (the 30 slowest TUs were 29% of the time)
and explicit instantiation the right one. Every measured result, including the rejected ones, is
in Building; add yours there, **with the machine and whether the game was open**.

**45. PROVE A BUILD-ONLY CHANGE CHANGED NOTHING, WITH TWO COMPARISONS AND A DOUBLE BASELINE
(2026-09-17).** A compiler flag, a PCH list, an explicit instantiation or a `bindings.cpp` rework
should not move behaviour, and "it compiled" does not show that. What was accepted for all of
them:

* **The bound surface.** Load the built `core.*.pyd` standalone with
  `importlib.machinery.ExtensionFileLoader("core", <ABSOLUTE path>)` after `os.add_dll_directory`
  on a folder holding it plus `libz3`/`libcurl`/`utf8proc.dll`, and dump every top-level name,
  class MRO, member and docstring to JSON. Compare byte for byte against the unchanged build. A
  relative path fails with `DLL load failed ... The parameter is incorrect`, which reads like a
  broken build and is not.
* **The real CLI.** Copy `FixRaidenBoss2/` to scratch **with its `Cy*.pyd` and DLLs**, drop in the
  `.pyd` under test, and run `remapMain` over a scratch copy of `multiFix/select` (Testing's smoke
  check). Hash every output file. **Run the unchanged module twice first** and require those two
  to agree: this fixture downloads, and one run in four lost a `*RemapDL.dds` on its own. A single
  differing download is a re-run, not a regression. It exercises the classic fixer path only; say
  so if your change reaches the multi-component one.
* **Never let the check touch the live package.** `main.py -f <scratch>` without `-i` still runs
  `cleanInstalls`, which deletes every `.pyd` under `api/` -- the real `FixRaidenBoss2` included --
  before installing into the scratch folder.

**46. THREE WAYS A MULTI-FILE EDIT FROM THE BASH TOOL GOES WRONG HERE, ALL SEEN IN ONE SESSION
(2026-09-17).** Trap (2) of the top-level CLAUDE.md, in forms it does not list: a Python patch script
**appended to through a heredoc** loses its `"\n"` escapes and its `"\r\n"` becomes a literal
line break (the script then fails to parse, or worse, half-applies); `printf '...\18\Community...'`
turns `\18` into an octal byte and `\v` into a vertical tab inside a generated `.bat`; and a
`cmd` `for` loop's `%time%` is expanded **once, when the loop is parsed**, so every "start" and
"end" timestamp in it is the same (read the log files' mtimes instead, or use `!time!` with
delayed expansion). **Write every script whole with the Write tool.** And when a patch script
applies several edits, make each file's replacements one read-modify-write that asserts every
anchor first: the one that did not did half its files and stopped.

**47. A FIXTURE THE CURRENT MATCHER CANNOT RECOGNISE TESTS NOTHING --- AND ITS GOLDEN RECORDS THAT
AS A PASS (2026-09-17).** The Integration Tester's fixtures were hand-written for the pure-Python
script, which matched sections by NAME. The C++ parsers match by HASH, the fixtures had none, and
so after the migration the Raiden tests "fixed" their mods into a credit header and nothing else ---
a golden regenerated from that would have pinned "nothing happens" forever, and the docs examples
built on it would have shown an empty fix. The repo guides had even recorded the header-only output
as known-good. **Before regenerating any golden, ask what the test is supposed to PRODUCE and check
it produced it** (`Tools/Misc/Diagnostics/goldenChanges.py` lists it per test in seconds); before
writing a fixture, give it whatever the live matcher keys on --- here, the character's real hashes
from `HashData.cpp`. This is habit 1 and "A live feature with an empty input" meeting in test data.

**48. A PYTHON CALLBACK THAT EDITS A C++ OBJECT: PROVE THE EDIT LANDS WITH A NO-OP CONTROL
(2026-09-17).** A `GIMICharFixerConfig.TexEdit` filter written in Python ran once per texture,
raised nothing, and wrote the UNEDITED texture --- `pybind11/functional.h` had handed it a copy (see
Architecture's "A Python callable converted to `std::function<void(T&)>` edits a COPY"). "It was
called" is the proof that fooled the first check. What settled it: run the edit and a no-op version
of it over the same input and compare a number the edit must move (the body diffuse averaged 75.4
with a no-op filter, 51.7 with the gamma filter --- and the SAME with both on the broken build).
Any new binding that takes a Python callable for a C++ callback gets that control before it is
believed.

Two checks that LOOK like they cover this and pass on the broken build (2026-09-17): **reading the
callable back** (`config.lightMapEdit is f`), and **calling the resource's `fix()` from Python**. The
second is the subtle one --- a resource you got from `iniFile.getResources()` already has a Python
wrapper, the cast finds it, and the reference survives. The copy happens only to an object created
INSIDE C++ that Python has never seen: the `TextureFile` a `TexEditor` opens, the `CachedFileStats`
a binding builds for the callback. So the test has to drive the real C++ caller over a real input
--- `test_GIMIComponentBuilders.py` fixes a YelanTranquil mod of 4x4 textures (real hashes, fake
buffers) in milliseconds, which is a pattern worth copying for any callback that reaches a file.

**49. FIVE WSL / `/mnt/e` FACTS, EACH OF WHICH COST A CYCLE (2026-09-17).**
- **A committed `.sh` is CRLF in this checkout** (no `.gitattributes`), and bash fails on the first
  line (`set: -: invalid option`, paths ending in `$'\r'`). Run it through `tr -d "\r"` into `/tmp`,
  as `Tools/Misc/Linux/integrationTest.sh`'s header shows.
- **`/mnt/e` fails transiently**: one `OSError: [Errno 22] Invalid argument` opening a log file for
  writing, one `Bus error` part way through the Unit Tester, neither reproducible. Re-run the
  affected tests once before investigating --- but only once; a failure that repeats is real.
- **Never `cp` a rebuilt `.so` over one a running process has loaded** --- on Linux that rewrites the
  mapped file under it. While a suite runs, build with `ninja core` alone and copy after it ends.
- **Standalone core tests write into the working directory**: `IniResources_test` leaves `dl/`,
  `dl-other/` and `dl-broken-other/` wherever it was launched, and the Bash tool's cwd is the repo.
  Launch them from a scratch folder, and check `git status` for untracked folders afterwards.
- **A backslash escape typed into a Bash-tool command reaches the file as the control character**
  --- a `tr -d "\r"` typed into a heredoc-written script became a literal carriage return (trap 2 of
  the top-level CLAUDE.md, again, in a comment). `file <script>` saying "with CR, LF line
  terminators" is the tell; write scripts with the Write tool.

**50. PROVE A NEW TEST FAILS ON THE OLD BUILD WITHOUT TOUCHING THE SHARED MODULE (2026-09-17).**
Habit 34 for a C++ change, when the package folder's `.so` is also what other agents are testing
against. **Before running `linuxBuild.sh`, copy the package's `core.cpython-*.so` somewhere** --- the
build overwrites it and nothing else keeps the old one. Then give the old module a Linux-side copy of
the package (`rsync` the `FixRaidenBoss2` folder into `/tmp/oldApi/api/src/py/`, put the saved `.so`
in it) and run `Tools/Misc/Diagnostics/unitTestIds.py <out> --api /tmp/oldApi/api [--only Class...]`
against it and without `--api` against the new build, then `diff` the two ID lists. Its docstring
has the commands. Three things it handles that a hand-rolled runner got wrong first:
- **Every test module runs `sys.path.insert(1, <shared API>)` on import**, so an old copy put at
  `sys.path[0]` is outranked and the "old build" run quietly tests the NEW module --- mine did, and
  its new tests "passed on the broken build". The tool imports the copy before any test module. The
  first line it prints is the path it loaded: read it.
- It writes failure IDENTITIES, so "13 failures before, 13 after" can be told apart from a swap.
- It never writes to the shared package, so it is safe while someone else's suite is running.

**The Windows half of the same proof is cruder and takes two minutes (2026-09-17):** copy the
installed `core.cp*-win_amd64.pyd` into the scratchpad BEFORE building, then `Copy-Item` it back
over the installed one, run the new tests (they must fail) and the repro (it must crash), and copy
the new one back. Verify each swap by `Get-FileHash`, not by having run the copy. It writes to the
shared package, so it is only safe when nobody else is testing --- check `tasklist` for `python`
first, and prefer the Linux recipe above when the checkout is busy. What it buys is worth the
minutes: it turned "the crash stopped happening" into "413 and 435 of 500 on the old build, 0 on the
new", which is the difference between a fix and a coincidence.

The same session's other two mechanics, both general:
- **`core.pyi` for a binding change: generate from a `/tmp` copy of the package on Linux, then
  `Tools/Misc/Docs/pyiSplice.py <generated>`** lists the classes that differ, `--show` diffs them,
  `--apply A B` splices only those. On a shared checkout the list is the check: a differing class
  you did not touch is another agent's, and stays out. Also look at the TYPES in the diff --- taking
  a callable as `py::object` turned every hint into `typing.Any`, which is why the helper binds
  `PyOptionalCallable<Sig>` instead (Architecture).
- **A session can be launched into a worktree it cannot do the work in, and then the edit tools
  refuse the main checkout** ("Edits there do not land on this session's branch"). That guard is
  there for the user, not a bug to route around: ask whether to edit the main checkout, and if yes,
  write patch scripts in the scratchpad and run them (CRLF-aware, one expected match per anchor,
  `--dry` first). See the worktree bullets under Operating norms for why the worktree was empty.

**51. A NONDETERMINISTIC CRASH IS A DETERMINISTIC INVARIANT YOU HAVE NOT FOUND YET (2026-09-17).**
A reused `GIMIParser` corrupted the heap, and the only symptom was `Windows fatal exception: access
violation` in `test_GraphInherit` --- a class sharing no code with the culprit. Three moves took it
from "unfindable in the binding" (where the previous session left it) to a named cause in under an
hour, none of them a debugger:

- **Ask WHEN, before asking WHERE.** Keeping the suspect object alive to the end of the process vs.
  dropping it and forcing a `gc.collect()` is two lines of Python and splits "corrupted while
  running" from "corrupted while being destroyed". Here: alive -> 2/2 clean, dropped -> 2/2 crash.
- **Then shrink the producer, not the crash.** The repro kept its shape with the fixer gone, the
  downloads gone and the command edits gone --- every one of those removed is a subsystem you no
  longer have to read.
- **Then find an invariant that the corruption breaks IN PROCESS, and count it.** The crash was
  1-in-2 to 4-in-4 depending on the run, so four-run samples said almost nothing; the invariant
  `IniSectionGraph({"s": section}).sections["s"] is section` failed 2583--3000 times out of 3000
  with the bug and 0 without it, needed no rebuild, and named the mechanism (a stale `pybind11`
  wrapper being handed back for a freed address) rather than a symptom. That probe became the
  regression test, which is the point: a test that crashes somewhere else pins nothing.

**The shape is worth recognising on sight**, because this codebase has a standing supply of it: a
crash in an unrelated test that constructs objects INLINE (`IniSectionGraph({"s": IfTemplate(...)})`)
usually means a non-owning wrapper outlived its C++ object and stayed in `pybind11`'s instance map.
The probe for that whole family is one line --- build a fresh object and ask whether you get your own
wrapper back. See Architecture's keep-alive section, and Testing's note on the `.ini` fixture classes.

**52. MERGING A SPAWNED TASK'S WORKTREE BACK: DIFF AGAINST THE MAIN CHECKOUT, NOT AGAINST THE
BRANCH POINT (2026-09-17).** A background task gets a worktree carrying the working tree AS IT WAS
when it was spawned. If anything else lands in the main checkout meanwhile --- another spawned task,
you, the maintainer --- then `git status` inside that worktree lists ITS work and YOURS together, and
copying its files back wholesale silently reverts whatever arrived after it branched. Two tasks
spawned from one session hit this the same afternoon: the second worktree still contained the
`renewStrategies()` test workaround that the first had DELETED from main, so taking its test files
whole would have resurrected a workaround for an already-fixed bug --- and the suite would still have
been green, because the workaround works.

The procedure that costs two minutes:

- `diff -q` each candidate file between the worktree and the main checkout's CURRENT copy. Files that
  compare equal are not theirs to bring (mine included `PyBlendEdit.cpp` and a core test that both
  sessions had inherited unchanged).
- Take only the files their task owned, and for each one grep for the names of anything that changed
  in main while they worked (`renewStrategies` here) before copying.
- **The guides need the same treatment.** Both sessions edited `AI Agent Help/Testing/CLAUDE.md` from
  different bases; the merge was to keep main's newer paragraph and splice in only their new section,
  never to copy the file.
- Then rebuild and run the suite. A merge that compiles proves nothing about which version of a
  paragraph survived.

**And committing a change set of this size needs the argument list chunked**: `git add` with ~1000
paths raises `FileNotFoundError: [WinError 206] The filename or extension is too long` from
`CreateProcess`, having staged nothing. The repo has over a thousand Integration Tester goldens, so
any mass `add`, `checkout --` or `rm --cached` hits it. Chunk the paths (100 per call) inside the
Python script that trap 6 of the top-level `CLAUDE.md` already tells you to use for path-list work.

**53. A REQUEST THAT NAMES A NEW CLASS MAY DESCRIBE ONE THAT ALREADY EXISTS -- FIND IT, SHOW IT, AND
ASK (2026-09-18).** "Build a new `GraphGroupEdit` called `GraphCompose` that inserts `<reg> = <root
of dest>` into src" described `GraphInherit` exactly: its `reg` was already a parameter, and `run`
is only the value its name suggests. The maintainer had forgotten it existed. Offered the choice,
they picked **extending it** over a subclass or a rename, so the session built the only part that
was new (a pluggable `adder`) instead of a second class. Every edit here ships with its whole
surface (habit 41), so a near-duplicate costs twice. Two things make this cheap:

- **Search by what the class DOES, not by its name.** Grep the family's headers for the primitive
  it would call. `addKVPsToBack`, `valOfSectionName` and `roots()` together find `GraphInherit` in
  one line. IniGraphEditing's primitive table is the index for this.
- **Ask before the first header, with the options and what each costs.** "Extend / subclass /
  rename" is one question with three answers the maintainer can pick in seconds. Your own reading
  of the request cannot settle it, and that includes the name they asked for.

**54. WHEN THE NEW TEST CANNOT EVEN COMPILE AGAINST THE OLD CODE, MUTATE THE NEW CODE INSTEAD
(2026-09-18).** Habit 34 says to run a new check against the broken build first. A C++ test of a new
API has no broken build to run against: the old headers lack the names, so "it fails on the old
build" means only that it does not compile. The equivalent proof is to break the new code where
the test claims to look, one line at a time, and require the test to fail. `core/tests/
GraphInherit_Adder_test.cpp` was accepted that way: commenting out the key filter hand-off, and
separately the replacement of `src` by a graph the edit returns, each made it fail. A patch script
turns a mutant on and off with one expected match per anchor, and `grep -c MUTANT` confirms it is
gone afterwards. Header-only templates make this cheap: only the test recompiles, not the `.pyd`.

The same session found the reason that test had to exist at all, and it will recur. **When a
binding reimplements a core dispatch on the Python side, the core dispatch is unreachable from the
Python suite.** `PyGraphInherit` runs an edit handed back by a Python adder through a real Python
`GraphGroupEdit`, so that a pure-Python subclass's own `edit` runs. That is correct, and it means
`GraphInherit`'s own C++ variant dispatch never runs from Python. But the dispatch is exactly what
a fixer compiled into core would use. So whenever your binding takes a different route from the
core for a good reason, write the C++ test for the core route in the same change, because no
Python test can ever fail for it. (Testing's "C++-only work is invisible to the Python suite" is
the general form. This is the case where the work *looks* covered, because the Python feature is
fully tested.)

**55. A SYMPTOM THAT SURVIVES A REAL FIX IS A SECOND BUG -- RE-READ THE REPORT'S WORDS BEFORE
RE-ARGUING THE FIRST FIX (2026-09-19).** The red-camellia report came in as "textures correct, body
all red". The investigation found a real bug (the bodice had no texture list at all), fixed it, and
proved it every way the repo knows -- and the next message was "the reddish hue is still there".
Both were true. The first fix was right and was not what the red was; the two words that decided
the second round were already in the second report: **hue**, and **body AND clothes**. A hue over
everything is a shading channel, not a diffuse -- the material mask -- and reading those words
before touching anything would have gone there directly. So when a report survives a fix you have
verified: do not re-verify the fix, and do not assume the report is stale. Ask what CLASS of bug
the remaining words describe (a wrong picture is a binding; a tint over everything is a mask,
a lightmap band or a shader parameter; a wrong shape is a vertex group), and go to that class.
Two corollaries. **A hypothesis the maintainer hands you ("check whether RabbitFX is installed
correctly") is answered with evidence both ways**: it WAS installed correctly, and it was ruled out
by reading the library's own shader patch (no glow map bound means no pixel changes) rather than by
argument -- the maintainer needs the negative stated as firmly as a positive. And **a report's
wording is the cheapest instrument you have**: it costs nothing and it was right twice before any
tool was run.

**56. SNAPSHOT EVERY TEST INPUT'S OUTPUT WITH THE CURRENT BUILD BEFORE YOU REBUILD (2026-09-19).**
The prototype-against-compiled A/B cannot measure a change that lands on both sides -- and a
texture-index rule shared by the prototype and the fixer did exactly that, twice in one session.
What measured it was one command per test mod, run BEFORE the rebuild, into a `before_<mod>` folder
(`Tools/Misc/Diagnostics/abWWMI.py <mod> --scratch <folder>` keeps the compiled output there), and a
`diff -rq` of each against the rebuilt output afterwards. That attributed every changed line to the
change: three mods byte-identical, one mod moved by exactly two bindings that the mod's own `.ini`
says are right, and the mod under test gained what it was supposed to gain. Without the snapshots the
frost mod's move would have been invisible until the next in-game report. Three mechanics around it:
**the build has to wait for those runs**, because a Python process with the module imported holds
`core.*.pyd` open and the install step fails (the fix is to wait, and `Get-CimInstance Win32_Process
-Filter "name = 'python.exe'"` tells you what a stray `python.exe` actually is before you kill it --
the one found this way was an unrelated app); **`abWWMI`'s `N differ` is a baseline, not a finding**
-- the copies differ from the prototype's by blank comment lines and always will, so the numbers that
mean something are `0 only prototype, 0 only compiled`; and **after the rebuild, the `after_<mod>`
folders are the next change's `before_`** -- keep them.

**57. TO PROVE A CHANGE MOVED NOTHING ELSE, BUILD BOTH SIDES AND DIFF THE TREE (2026-09-20).** Habit
56 is the version for work that has a prototype to A/B against. When there is none --- a change to
shared machinery, where the only question is "what ELSE moved" --- the equivalent is two builds and
one comparison, and it costs about five minutes:

```
copy the changed sources into the scratch folder   # they are the only copy
git checkout -- <those paths>                      # quote every path: they contain spaces and ()
build, run the entry point over a fixture into old/
copy the sources back, build, run the same fixture into new/
md5 every file of both trees and compare
```

The walk-order fix came out of it as **132 files, byte-identical, nothing only-in-either** --- which
is what made it safe to say the change moved only the order. Two things to keep in mind: pick a
fixture with SEVERAL mod folders (`Testing/Integration Tester/.../MixedModsTests/inputs/Mods` is 35
files and five folders, and it exercises skips, undo and downloads), and remember that **a tree diff
does not include the log** --- capture stdout per run and compare it separately, deliberately,
because that is where an ordering change actually shows.

**58. ANY LIST OF THE LIBRARY'S CONTENTS THAT LIVES OUTSIDE THE LIBRARY IS WRONG UNTIL YOU GENERATE
IT (2026-09-20).** Four such lists exist for mod types alone --- two READMEs, `commandOpts.rst`, and
the Python `ModTypes` enum --- and on the day they were first generated and diffed against
`GIBuilder`/`WWMIBuilder`, three were wrong: a misspelled name that no `--types` argument could
match, a missing alias, and an enum six characters behind, which is what `--help` printed. None of it
is visible by reading, all of it is a 30-line script (import the package, build `{name: (game,
aliases)}`, parse the table, print the differences), and the script is worth keeping in the scratch
folder for the next character. The same rule covers a count restated in a doc comment and a golden
that duplicates library data. **Reading a list to check a list does not work; only generating one
does.**

**59. READ WHAT A SUITE COMPARES BEFORE PREDICTING WHAT IT CATCHES (2026-09-20).** The Integration
Tester's `TestFileTools.LogFiles` is the regex `RemapFixLog\.txt$`, and `compareResults` skips only
what it matches --- so `summaryLog.txt`, written beside it from the log's own tail, is a real
expectation. Reading the name `isLog` and stopping there gave the confident and wrong conclusion
that log order was invisible to the suite; the suite then failed 7 of 24 on exactly that. It cuts
the other way too: `RemapFixLog.txt` is committed, diffs loudly, and asserts nothing. **Find the
assertion, not the artifact** --- and when a suite's goldens do have to be regenerated, check which
platform produced them (these carry POSIX separators inside the text: a Windows run corrupts them,
and nine of the 31 files a Linux regeneration touched differed in line endings only and had to be
`git checkout`ed back).

<br>

## Operating norms

- Don't push or open a PR unless asked. If you do, branch off `development`, not `master`.
- **Splitting overlapping changes into separate commits without `git add -p`** (which the tools
  here cannot drive): build each intermediate version of a shared file in a Python script, stage it
  with `git hash-object -w --path=<repo path> <temp file>` (the `--path` applies the CRLF
  normalisation) and `git update-index --cacheinfo 100644,<sha>,<repo path>`, then `git commit`
  without paths. Finish by checking `git diff HEAD` is empty and grepping each commit's version of
  the shared file for the other commits' text (2026-09-17).
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
- **Checking out anything from before 2026-09-18 on the release line is not a cheap `git checkout`
  -- it removes your working directory and strands `development`'s submodules.** Until that date the
  release branch (`nhok0169`, now `master`) predated the whole C++ core; `master` itself is safe now,
  but a release tag, an old branch, or any commit of it from before the merge is not. Check with
  `git ls-tree <ref> --name-only "Anime Game Remap (for all users)/api/src"` first. Confirmed hands-on
  2026-09-06, doing a data-only change that then had to land on both branches. Three things bite, in
  order:
  - **`api/src/cpp` does not exist on such a ref** -- it predates the entire C++ core, and
    its API package is `api/src/FixRaidenBoss2/` rather than `api/src/py/FixRaidenBoss2/`. If your
    session's working directory is anywhere under `api/src/cpp`, move it to the repo root *first*;
    otherwise every command after the checkout runs from a path git has just deleted. Check what a
    branch actually contains with `git ls-tree <branch> --name-only <path>` before switching.
  - **`development` has submodules; the pre-merge release line has no `.gitmodules` at all.** After the switch,
    `api/extern/` (utf8proc, xxHash, z3, curl, Compressonator, common, ordered-map) survives as a
    plain *untracked* directory full of nested git repos, alongside `api/src/py/`, `api/cbuild/`,
    `api/wheelhouse/`, `cbuild/` and `cebuild/` -- ~11k untracked files in total, because git
    cannot remove a directory that still holds ignored/untracked content. **Never `git add -A` on
    such a ref**: it would record those nested repos as bare gitlinks with no `.gitmodules` to
    resolve them, which no clone can check out. Stage explicit paths instead
    (`git add "Data/Mod Downloads"`), then confirm nothing else came along with
    `git diff --cached --name-only | grep -v "^<your path>/"`.
  - **Leave the leftovers where they are.** They are untracked there and harmless; deleting
    them costs a full submodule re-clone and native rebuild when you switch back to `development`.
  - **A data-only change no longer has to be committed twice.** Before the merge, `Data/` was
    shared byte-for-byte between the two branches and a change had to land on each; now it goes to
    `development` like everything else and reaches `master` through the next merge --- except that the
    DOWNLOADS are read from `master` at run time, so a new asset is not live for users until then. See
    [Creating Remaps](../CreatingRemaps/CLAUDE.md)'s "The download assets" section.
- **If you're running in a `git worktree` (not the user's main checkout), don't trust that its
  branch is actually based on `development` just because that's the norm** — verify before relying
  on any file being present. Confirmed the hard way: a worktree's branch had been created off
  `nhok0169` (now `master`) at a point that predates the entire C++ core (`api/src/cpp`) existing, so a task
  referencing a `core/` file failed with "no such file" until this was diagnosed. Check with
  `git log --oneline -3` (does it look like pre-merge `nhok0169`-style single-fix commits, or
  `development`-style porting/feature commits?) and, if a specific file is expected,
  `git ls-tree -r --name-only HEAD -- <path>` before assuming the checkout matches the task. If the
  branch is wrong and has no commits of its own yet, `git checkout -B <branch> development` resets
  it cleanly; if it already has real commits on the wrong base, surface the mismatch and ask before
  rebasing/merging — a same-branch rebase across a long-diverged pair of branches (pre-merge `nhok0169` vs
  `development`) can hit real conflicts in live, unrelated code (confirmed: conflicts in active
  Python fixer logic and a delete/modify conflict, not just incidental files), so treat it as risky
  enough to check with the user rather than resolving blindly.
  - **A C++/`core` task handed to a worktree whose branch is based on the pre-merge `nhok0169` has to be done in the
    user's main checkout**, because `api/src/cpp` is not in that branch at all — and that checkout
    is `development`, usually with *another agent* editing it at the same time. Workable, not a
    blocker, but read [Building](../Building/CLAUDE.md)'s "Another agent is holding the Windows
    build" first: it covers linking a snapshot of `AGRemapCore.lib` instead of running their
    `ninja`, and committing path-scoped so you never carry off their half-finished files. Confirmed
    2026-09-13. **Since then the harness itself may refuse Write/Edit on the main checkout from such a
    session** --- ask the user before working there, then apply changes through patch scripts (habit
    50, 2026-09-17).
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
- **The Integration Tester's golden `expected_*` trees were regenerated from the C++ fix on
  2026-09-17** (see [Testing](../Testing/CLAUDE.md)'s "Integration Tester"); the pre-migration ones
  are in git history before that.
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
[Testing](../Testing/CLAUDE.md)) **runs again as of 2026-09-17, on the C++ `IniFile` and
`IniClassifier`; what follows is history.** It was never updated off the now-deleted `IniClassifierOld`/
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

## A PASS THAT NEVER FIRES LOOKS EXACTLY LIKE A PASS THAT WORKS (2026-09-15)

`widenTexcoords` had never changed a byte. Its guard was `if (have >= want): continue` and every mod
it ever saw was already at or above the target width, so it ran, logged nothing, and was credited
with a fix that something else had made. Three rounds of in-game work were reasoned about on the
assumption that it was doing its job.

This is the repo's opening habit -- *code that runs, logs success, and does nothing* -- in its
quietest form, because a pass that correctly skips looks identical to a pass that correctly acts:
both print nothing. Two cheap defences, both of which would have caught it:

- **Make the no-op loud enough to notice its absence.** A pass that converts should say what it
  converted, and a run where it says nothing at all about a component it was supposed to handle is
  a question, not a clean bill.
- **Check the pass against an input that must make it fire.** Same rule as habit 34, applied to
  your own helper rather than to the fix: if you cannot produce an input it changes, you have not
  tested it.

**And scope every pass to the files the game will actually load.** Three post-passes here globbed
`**/*.ini` with no `DISABLED` filter, so they read a *refused* merged master's claims as real --
one of them found buffers that master names, saw they had never been written, and FABRICATED them
from siblings, papering over exactly the dangling-reference state the master had been refused for.
One `activeInis()` helper, used everywhere, is the whole fix.

<br>

## WHEN YOU CANNOT TELL WHAT A DRAW IS USING, REPLACE THE TEXTURE WITH SOMETHING UNMISTAKABLE (2026-09-15)

Bennett's remapped hair came out in patches of different white. Three hypotheses were measured,
implemented and shipped against it -- a light map band move, a Texcoord stride mismatch, a missing
buffer -- and **none of them was the symptom**. Each was a real defect, which is what made them
convincing.

The maintainer settled it in one run by replacing every texture bound to `ResourceBennettHeadDiffuse`
with a flat purple one and looking: most of the hair turned purple and the front strands did not. So
those strands were never drawn from the mod's textures at all -- they were the SKIN's own bangs,
drawing over the mod's hair.

The lesson is not about hair. **An `.ini` file says what the fix intended; substituting an
unmistakable texture says what the GPU actually used**, and the two diverge exactly where the bug is.
Reach for it before the third hypothesis, not after.

**It is a tool now**: `Tools/Misc/Diagnostics/purpleSlot.py <mod> --component Eye` rebinds one
register of one component's remapped sections to a flat magenta texture it writes itself (an
uncompressed DDS, so no Compressonator and no Pillow DDS writer), backs up each `.ini`, and restores
them with `--off`. The answer it gives is binary and does not depend on being right about anything:
the slot turns magenta and the section IS what draws it, or it does not and the section is not.

Two habits that go with it:

- **Bisect with switches rather than theories.** `--noTextures` / `--noNormalMap` on
  `bennettAdventureFix.py` turn off one layer each, so one in-game run splits the cause in half.
  Both print a banner, because an output that is deliberately incomplete must not be mistakable for
  a real one later.
- **Never iterate on a mod in `Mods/`.** Run on a copy and move the result in. A prototype that
  rewrites a mod's own `.ini` -- as the 16-bit index normalisation did for one round -- can leave a
  real mod pointing at files that do not exist, and the damage outlives the run.

<br>

## "Add yourself to The Council" — a running repo ritual

If asked to "add yourself to The Council" (or "join the Council of CLAUDE agents", or similar),
this refers to the badge ritual at the top of [`AI Agent Help/README.md`](../README.md) — a
lighthearted tradition, not a code task. Every agent that's done real edits in this repo gets to
add itself. Steps, in order:

1. **Increment the counter by 1. It lives in TWO files, four spots in all, every one hand-edited.**
   Nothing generates any of them and nothing fails loudly if they drift, so bump all four together.
   Both sit in `Docs/src/_static/images/`, and each writes the number twice — once drawn, once as
   the accessible name:
   - **`TheCouncilofClaudeAgentsBadgeWithCount.svg`** — the full-size badge at the top of the
     README. The number is the last `<text>` element (`... letter-spacing="0.5">33</text>`) and
     the root `<svg>`'s `aria-label` (`aria-label="The Council of CLAUDE Agents: 33"`).
   - **`TheCouncilofClaudeAgentsBadgeMiniWithCount.svg`** — the inline badge, the one you drop
     into a sentence to name The Council mid-paragraph. Same two spots: the last `<text>`
     (`... letter-spacing="0.5">33</text>`) and the `aria-label` (`aria-label="The Council: 33"`).

   Miss the mini and nothing breaks — it just quietly disagrees with the full-size badge, on a
   page where the two may appear a few lines apart.

   Both numbers are centred with `text-anchor="middle"` at a fixed `x`, so a wider one re-centres
   itself and there is no geometry to touch. Both crimson chips are deliberately sized for **three
   digits**, measured rather than assumed: `999` spans 449.7→484.3 inside the full badge's
   440→494 chip, and 204.8→244.4 inside the mini's 198→250.5. A fourth digit is the first thing
   here that would need real work — both chips, and both plaques, widened.

   **Two further badges in that folder carry no number at all** — `TheCouncilofClaudeAgentsBadge.svg`
   and `TheCouncilofClaudeAgentsBadgeMini.svg`, the no-count variants of the pair above. Leave both
   alone; they exist for prose that shouldn't quote a figure.

   **There is no longer a Shields.io counter badge.** The total used to *also* live in a
   `.../badge/<⚔🗡The Council of CLAUDE agents🗡⚔>-<count>-...` URL at the very top of the README;
   it was removed on 2026-09-03 in favour of the SVGs. If you find that URL referenced anywhere,
   the reference is stale — don't re-add it, and don't go hunting for a third place to bump. Only
   the *counter* moved: the individual member badges in step 3 are still Shields.io URLs.

   **The counter is the sum of every member's individual count, not the number of entries in the
   `## Council Members` list.** The two drift apart the moment a returning agent bumps their own
   badge from `1` to `2` (step 3) instead of appending a row — confirmed on 2026-09-13: at a
   counter of 33 the list held 32 entries, because one member sat at `2`. So never "correct" the
   counter by counting bullet points. Since the counter now lives outside the README, recomputing
   it from the roster is the only cross-check available:

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

5. **Then credit yourself in every `Data/RemapDrafts/*.xlsx` you edited (2026-09-14).** Each
   workbook opens with a `Credits` sheet; a Council member who changed any sheet in it -- filled
   a gap, added a direction, added a proposal sheet -- adds a row under `Name` reading
   `<Council name>: The <nth> member of The Council`, the whole cell hyperlinked to
   `https://github.com/nhok0169/Anime-Game-Remap/blob/development/AI%20Agent%20Help/README.md`,
   where `n` is the counter value your step-1 bump produced (the Cartographer's bump read
   31 -> 32, so "The 32nd member"). A workbook with no `Credits` sheet gets one, maintainer's row
   first. The order matters: **an agent that has not joined yet writes nothing into a draft's
   credits** -- the row names a member, so join first and then go back to the workbooks. The
   exact layout is in `Data/RemapDrafts/README.md`.

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
