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
  Utilities/                    <- shared helper package used by the test runners
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

## Operating norms

- Don't push or open a PR unless asked. If you do, branch off `development`, not `nhok0169`.
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
`RemapIniRemover`, `MultiModFixer`, the three `Ini*Builder`s in core (bound as
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

## "Add yourself to The Council" — a running repo ritual

If asked to "add yourself to The Council" (or "join the Council of CLAUDE agents", or similar),
this refers to the badge ritual at the top of [`AI Agent Help/README.md`](../README.md) — a
lighthearted tradition, not a code task. Every agent that's done real edits in this repo gets to
add itself. Steps, in order:

1. **Increment the counter badge** at the very top of that file (the one reading
   "⚔🗡The Council of CLAUDE agents🗡⚔") by 1. Its Shields.io URL shape is
   `.../badge/<label>-<count>-<color>?style=...&labelColor=...` — only the `<count>` segment
   changes here.
2. **Pick a name for yourself**, related to the actual work you did this session — not a generic
   label like "Helper" or "Assistant". Base it on something concrete you actually touched (a
   subsystem you worked in, a pattern you established, a role like "first agent on the repo").
   Emoji/special characters are encouraged — see the existing entries under `## Council Members`
   in that README for tone/precedent (e.g. `🥇🏗️ The Founding Architect`, earned for the first
   pass through the `OrderedMultiMap`/`IfContentPart`/`IfTemplatePart` C++/pybind11 layer and for
   originally authoring most of `AI Agent Help/`).
3. **Check the `## Council Members` list** (further down the same README) for an existing badge
   whose name is close enough to yours in spirit. If one exists, increment *its* count instead of
   adding a new entry (same mechanic as step 1 — bump the middle `<count>` segment). Otherwise,
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
