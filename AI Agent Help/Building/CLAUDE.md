# Building

How to compile the C++ core, the pybind11 bindings, and the Cython extensions. See
[Overview](../Overview/CLAUDE.md) for repo layout and why the compiled binaries matter, and
[Testing](../Testing/CLAUDE.md) for why a rebuild has to happen before tests will reflect a
native-code change.

## Prerequisites (Windows/MSVC, the dev environment this file assumes)
- Python 3.9 at `py -3` (the committed `.pyd` is `core.cp39-win_amd64.pyd` — building with a
  different Python version produces a differently-named file and won't overwrite the tracked one;
  ask before intentionally changing the pinned dev Python version).
- Visual Studio (MSVC) with the C++ toolchain, CMake, Ninja.
- The MSVC dev environment must be initialized in-shell first:
  ```bash
  call "<path-to-VS>\VC\Auxiliary\Build\vcvarsall.bat" x64
  ```
  Find the exact path once with `find "/c/Program Files/Microsoft Visual Studio" -iname vcvarsall.bat`
  (installation path/version varies by machine). Everything below assumes this has been run in
  the same shell.
  - **If you're an AI agent driving this through a tool whose shell state doesn't persist between
    separate tool calls** (env vars set in one call are gone by the next, even though the working
    directory may persist) — `vcvarsall.bat` and the actual build command must happen inside one
    single invocation. Inlining `cmd //c '"...\vcvarsall.bat" x64 && py -3 main.py'` directly as a
    Bash-tool command tends to break on the nested quoting (the outer single-quote/inner
    double-quote mix gets mangled going through Git Bash). The reliable pattern: write a small
    `.bat` file to the scratchpad with `call "...\vcvarsall.bat" x64`, an errorlevel check, then
    `cd /d` into `Tools/APIBuilder` and the actual `py -3 main.py ...` line — then invoke just that
    one `.bat` path via `cmd //c <path>` (unquoted if the scratchpad path has no spaces, which it
    won't). Run it via the tool's background mode and tail the log; don't try to poll for
    completion, wait for the completion notification instead.

## Full build (core + pybind11 + Cython + docs XML)
From `Tools/APIBuilder`:
```bash
py -3 main.py -d
```
- No `-e` flag = `dev` env mode: builds `AGRemapCore`, the `core` pybind11 module, and the
  Cython extensions, then installs everything into `api/src/py/FixRaidenBoss2/` (default
  `--installFolder`).
- `-d`/`--addDocs` also runs Doxygen over `core/include` as part of the build (needed before a
  docs rebuild picks up any C++-side doc-comment changes — see
  [Documentation](../Documentation/CLAUDE.md)).
- Build artifacts land in `cbuild/` (CMake build dir), external deps in `cext/`/`cebuild/`, all
  at the repo root — these are safe to delete and let the next build regenerate
  (`-b /`, `-pir /`, `-p /` to do that explicitly; `*` instead of `/` nukes every suffixed
  variant too).
- `-e core` builds only the C++ core as a static lib for external C++ consumption (no Python
  bindings) — not what you want for a normal Python-visible feature.
- `-s`/`--skipBuild` reinstalls without recompiling; `-i`/`--installKeep` preserves the previous
  install instead of overwriting.
- Run `py -3 main.py -h` for the full flag list; it's authoritative over this summary.

Run it in the background and tail the log rather than blocking — a full rebuild (with docs) takes
noticeably longer than a small edit-compile-test loop, and `-d` additionally shells out to
Doxygen/plantuml/mermaid.

## Fast iteration on C++-core-only changes
If you're only touching `core/include` or `core/src` (no pybind11-visible API change), you don't
need the full `main.py -d` cycle every time:
- A pybind11 rebuild (`main.py` without `-d`) is enough to get a working `.pyd` for testing;
  add `-d` back before your final doc-verification pass.
- For doc-comment-only changes, skip the C++ recompile entirely and just regenerate Doxygen XML:
  ```bash
  cd "Anime Game Remap (for all users)/api/src/cpp/core"
  doxygen Doxyfile
  ```
  then rebuild Sphinx — Sphinx/Breathe reads from the *generated* `core/xml/`, not from the
  headers directly, so this step is required before a Sphinx rebuild will reflect your header
  comment edit. Full details in [Documentation](../Documentation/CLAUDE.md).

## Verifying a build/binding change in Python directly
Don't just trust that it compiled — a pybind11 registration typo (wrong base class, wrong
holder, wrong constructor signature) fails at import/runtime, not compile time. This applies
equally to a Cython (`api/src/cy`) change — same "compiles fine, breaks on import" risk, same
verification approach:
```bash
py -3 -c "
import sys; sys.path.insert(0, r'Anime Game Remap (for all users)\api\src\py')
import FixRaidenBoss2 as FRB
# exercise the thing you just added, e.g.:
# print(isinstance(FRB.CppIfContentPart(), FRB.CppIfTemplatePart))
"
```

**Run this from the PowerShell tool, not the Bash tool.** Confirmed on this machine: importing
*any* freshly-built native extension (`core.pyd`, `CyDictTools.pyd`, `CyListTools.pyd` — not
specific to one module, and not specific to a fresh build either; it reproduced on a `.pyd` last
built weeks earlier too) through the Bash tool's Git Bash fails with
`ImportError: DLL load failed while importing X: The parameter is incorrect`, while the exact same
file imports cleanly from a native PowerShell invocation of the same `py -3 -c "..."` line. This
is a Git-Bash/MSYS environment quirk (most likely DLL search-path handling), not a sign the build
is broken — don't waste time treating it as a regression to fix. Also note: if you write the
verification snippet to a script file and run `py -3 <path>` instead of `-c "..."`, Python adds
*the script's own directory* to `sys.path`, not the current working directory — either `cd` into
`api/src/py` first, set `$env:PYTHONPATH` to that directory, or keep using inline `-c "..."` with
an explicit `sys.path.insert`.

If the change touches an `IOrderedMultiMap` virtual method, this quick check isn't enough by
itself — calling a pure-Python subclass's method directly from Python never crosses the pybind11
trampoline, so it can't catch an arity mismatch that only shows up when a C++ caller invokes it
through the interface pointer. See [Architecture](../Architecture/CLAUDE.md) for why that
specific gap matters and how to actually exercise it.

If the change introduces a brand-new pybind11-bound class (not just a new method on an existing
one), also exercise `copy.copy()`/`copy.deepcopy()` on an instance if anything in the codebase
deep-copies that type — a fresh `py::class_<...>` doesn't support either by default, and this
won't show up from "does it import and does the method I added work" alone. See
[Architecture](../Architecture/CLAUDE.md)'s note on this.

## Cython pieces
`api/src/cy` has its own small CMakeLists, built automatically as part of the same top-level
`api/CMakeLists.txt` orchestration (skipped only in `core`/`core_sdk` env mode). No separate
step needed — the same `py -3 main.py` invocation above rebuilds Cython sources alongside the
C++ core/pybind11 pieces. See [Architecture](../Architecture/CLAUDE.md)'s "Cython bindings"
section for the source-layout/wrapper-class conventions to follow when adding a new method here
(verified via one hands-on pass adding `CyDictTools.getVal`).
