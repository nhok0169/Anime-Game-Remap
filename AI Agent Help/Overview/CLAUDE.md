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
- This set of files was authored from hands-on, verified work in the C++ core / pybind11 layer
  (the `OrderedMultiMap` / `IfContentPart` / `IfContentPartColouring` subsystem, including a full
  pure-Python-to-C++ migration of the latter — see Architecture's "Two different outcomes for
  porting a class to C++/pybind11" section), plus a much larger, incrementally-built pass through
  the Cython layer (`CyDictTools`/`DictTools` and `CyListTools` — see Architecture's "Cython
  bindings" section and its dedicated gotcha section on exact-type parameter checking for what
  came out of it), plus — separately, later — the Python-side `.ini` graph model and its
  dataflow-analysis-based graph edits (`IniSectionGraph`, `GraphTools`, `CallGraph`, the
  `graphEdits/` strategy family; see [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)). Other
  subsystems (the non-graph `.ini` parsers, the `GIMIFixer` family, the standalone script variant)
  still haven't been exercised to the same depth — verify assumptions there with the usual tools
  rather than trusting this file blindly.

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
