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
  - **Pass that `.bat` as a full *Windows* path, not a bare filename — even after `cd`-ing into the
    directory that holds it.** `cd "$SP" && cmd //c build_core.bat` fails with
    `'build_core.bat' is not recognized as an internal or external command`, because the Git-Bash
    cwd isn't what `cmd` resolves against. Convert explicitly:
    `cmd //c "$(cygpath -w "$SP/build_core.bat")"`. The error message reads like a missing file, so
    it's easy to waste time re-checking that the `.bat` was written correctly when the path form is
    the actual problem.

## `api/extern/*` are git submodules — empty in a fresh `git worktree`

`z3`, `ordered-map`, `utf8proc`, `xxHash`, `Compressonator` under `api/extern/` (referenced by
`.gitmodules`, needed by `api/CMakeLists.txt`/`core/CMakeLists.txt`) are git submodules. `git
worktree add` does **not** run `git submodule update --init` for you — a fresh worktree's
`api/extern/*` directories exist but are empty, even though the submodules' git data already lives
in the repo's shared `.git/modules/...` (populated by whichever checkout, usually the user's main
one, did the original `git submodule update --init --recursive`).

`Compressonator` (~53MB, AMD's GPUOpen texture-compression library, backing `TextureFile`'s
default `TexEngine.Compressonator` engine — see [Texture Editing](../TextureEditing/CLAUDE.md))
is the newest of these and, unlike the other four (header/source-only, folded straight into
`AGRemapCore`'s own sources), is itself a real nested CMake subproject (`core/CMakeLists.txt`
`add_subdirectory`s into it and consumes `CMP_Compressonator`/`cmp_core`/`cmp_framework` as
separate build targets) — so a first build after populating it takes noticeably longer than the
other submodules, and its own `CMakeLists.txt` inside `extern/Compressonator` occasionally needs a
platform-variable workaround from the parent `core/CMakeLists.txt` (see that file's comments
immediately around its `add_subdirectory` call before assuming a Compressonator-side CMake error
is this project's own bug).

**Don't run `git submodule update --init` to fix this in a worktree** — confirmed it does not reuse
the shared `.git/modules` data automatically; it attempts a fresh network clone from each
submodule's GitHub URL instead, which is slow and, on this machine, fails outright
(`SSL certificate problem: unable to get local issuer certificate` — this environment's outbound
HTTPS goes through a TLS-inspecting proxy, e.g. Norton AV, whose CA isn't in git's default trust
store; `curl -v https://github.com` showing "unknown CA" right after the server's TLS certificate
is the tell). **Instead, just copy the already-populated directories from the main checkout**
(fast, no network, no cert wrangling — confirmed working via `robocopy <main-checkout>/api/extern/X
<worktree>/api/extern/X /E /XD .git` for the original four, ~40MB total; the same approach applies
to `Compressonator`, just a bigger copy at ~53MB):
```bash
robocopy "<main-checkout-path>/Anime Game Remap (for all users)/api/extern/z3" "<worktree-path>/Anime Game Remap (for all users)/api/extern/z3" /E /XD .git
# repeat for ordered-map, utf8proc, xxHash, Compressonator
```
(If you do need a real fetch — e.g. no populated main checkout exists to copy from — point git at
the interception CA first: `GIT_SSL_CAINFO=<path-to-CA> git submodule update --init --recursive`;
find the CA path via `$env:NODE_EXTRA_CA_CERTS` in PowerShell, which pointed at
`C:\ProgramData\Norton\Antivirus\wscert.pem` on this machine — don't assume the same path on a
different machine.)

Note `extern/uni-algo` is referenced by `api/CMakeLists.txt` as an interface include dir but
nothing under `api/src/cpp` actually `#include`s anything from it (verified by grep) and it isn't
even a live submodule in `.gitmodules` anymore (only a stale, orphaned git-dir under
`.git/modules/extern/uni-algo` remains) — don't go looking for it or treat its absence as a build
blocker.

## `z3` builds from source — but its *installed* output is copyable, so you rarely need to

`main.py -d` alone does **not** build `z3` — it goes straight to `buildAPI()`, which expects an
already-*installed* z3 under `<repo-root>/cext/z3` (passed as `CMAKE_PREFIX_PATH`) and fails
`find_package(Z3 CONFIG REQUIRED)` immediately (`Could not find a package configuration file...`)
if that's missing. Building z3 from source yourself is a separate, slow, explicit step
(`preBuildExterns`/`preInstallExterns` in `APIBuilder.py`, wired to `-pb`/`-pi` — not part of a
plain `-d` run) — a large C++ project, noticeably the slowest part of a build if you actually have
to do it.

**You almost never have to.** `cext/z3` (the *installed* tree — `bin/`/`include/`/`lib/`, ~30MB)
is a normal, relocatable `cmake --install` output, unlike `cbuild/` (the CMake+Ninja build tree
itself, which bakes in absolute paths and doesn't survive being copied elsewhere). If the main
checkout (or any other build tree) already has a populated `<repo-root>/cext/z3`, just
`robocopy`/copy that whole folder to the same `cext/z3` path relative to your worktree's repo root
— confirmed this lets `cmake -G Ninja -B cbuild ... -DCMAKE_PREFIX_PATH=.../cext/z3` succeed and
proceed straight into compiling `AGRemapCore`/pybind11/Cython, skipping the from-source z3 build
entirely. Only fall back to the real `-pb -pi` build-from-source path if no populated `cext/z3`
exists anywhere to copy from.

Either way, run the actual build in the background and tail the log rather than waiting on it
synchronously — see the guidance below.

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
  [Documentation](../Documentation/CLAUDE.md)). It also regenerates `core.pyi` (the pybind11 stub)
  to match the actual current binding surface. **Both `core/xml/*` and `core.pyi` are tracked
  files** — running `-d` purely to get a working `.pyd` for verifying an unrelated change (e.g.
  confirming a bugfix through the real Python binding) leaves these regenerated/modified as a
  side effect, showing up in `git status` as noise unrelated to your actual change. If a merge
  brought in new C++ classes since these were last regenerated (a real scenario, not
  hypothetical — hit this after merging in a commit that added several new pybind-bound classes),
  the diff can be large (hundreds of lines). Discard it after verifying, unless updating docs/stubs
  was actually part of the task: `git checkout -- <path>/core/xml <path>/core.pyi` (add `git clean
  -fd <path>/core/xml` too, since Doxygen can add brand-new XML files for brand-new classes, which
  `checkout` alone won't remove).
  - **Decide `core/xml`'s fate by whether you touched a `core/include/*.h`/`.tpp` doc comment —
    Doxygen never looks at `py/src/*.cpp` at all.** A pybind11-only docstring change (a
    `py::doc(R"doc(...)doc")` string in a `.cpp` under `py/src/`, e.g. adding/editing a class or
    method description) doesn't touch anything Doxygen processes, so `core/xml`'s regenerated
    content is guaranteed to be pure incidental noise from that specific `-d` run — discard it
    unconditionally, no case-by-case judgment needed. `core.pyi` is the opposite: keep it whenever
    you touched the pybind binding surface at all (new class/method/docstring), since it's the one
    artifact that actually reflects pybind11 registrations, not Doxygen's C++ header sweep.
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
- **If a `cbuild/` directory already exists at the repo root** (left over from a previous session
  in this same worktree, or copied/inherited somehow), it's a real configured CMake+Ninja build
  tree — `CMAKE_HOME_DIRECTORY` in its `CMakeCache.txt` tells you which repo checkout it's
  configured against (worth checking before trusting it, if you're not sure which worktree left it
  there). Driving it directly with `ninja` from inside is noticeably faster than a full
  `main.py -d` cycle for a code-only change, since it skips APIBuilder's own orchestration
  overhead entirely: `cd cbuild && ninja core` builds only the pybind11 module (target name `core`,
  matching `pybind11_add_module(core ...)` in `py/CMakeLists.txt`) plus its `AGRemapCore` static-lib
  dependency — same one-shot-`vcvarsall`-invocation constraint as above applies (plain `ninja` run
  through a shell that never sourced `vcvarsall.bat` fails immediately with `fatal error C1083:
  Cannot open include file: 'optional'`/`'vector'` — misleading, since it looks like a missing
  header rather than a missing dev-environment, but it's really just `cl.exe` running with no
  `INCLUDE` env var set). **This bypasses APIBuilder's install step** — copy the freshly-built
  `.pyd` out yourself before testing: `cbuild/src/cpp/py/core.cp39-win_amd64.pyd` →
  `api/src/py/FixRaidenBoss2/core.cp39-win_amd64.pyd` (and `cbuild/src/cy/cython/Cy*.pyd` →
  the same destination, if a Cython change is also in play). Don't assume every worktree has a
  `cbuild/` to reuse this way — it's an opportunistic shortcut, not the normal path; fall back to
  `main.py` (which configures one from scratch if needed) when there isn't one already there.
- **The two things `-d` adds on top of the compile — the `core.pyi` stub and the Doxygen XML — can
  each be run standalone**, so the `ninja core` shortcut above doesn't force you back into a full
  `main.py -d` cycle just to refresh them. Doxygen is covered in
  [Documentation](../Documentation/CLAUDE.md) (**read its note on wiping `core/xml` first** — a
  dirty-directory rerun can emit a corrupt `index.xml` that crashes the next Sphinx build). The
  stub is what `APIBuilder.buildDocs()` shells out to, and it needs `PYTHONPATH` pointing at the
  installed package:
  ```bash
  cd "Anime Game Remap (for all users)/api/src/py"
  PYTHONPATH=. py -3 -m pybind11_stubgen FixRaidenBoss2.core -o . "--root-suffix="
  ```
  **From the PowerShell tool, write it as `"--root-suffix="`, not `--root-suffix ""`** — PowerShell
  drops a standalone empty-string argument before `argparse` ever sees it, and you get
  `error: argument --root-suffix: expected one argument`, which reads like a bad flag rather than a
  quoting problem. The `pybind11_stubgen - [ERROR] ... Invalid expression 'AGRemapCore::Xxx'` lines
  it prints for a handful of pre-existing signatures are baseline noise, not your change failing;
  check that the classes you added actually appear in `core.pyi` instead.

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
# print(isinstance(FRB.IfContentPart(), FRB.IfTemplatePart))
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

## Fast path: compiling a standalone `core/` regression test without the full pipeline

If you only need to sanity-check pure C++ logic in `AGRemapCore` (no pybind11-visible behavior to
verify), you don't need `z3`, a CMake configure, or even the full `core/CMakeLists.txt` source
list — most of `core/`'s dependency footprint per-subsystem is much smaller than the whole
library's. Confirmed for the trie subsystem (`BaseTrie`/`BaseAhoCorasickDFA`): with
`TrieVal = std::unordered_set<int>` (a plain std container, not `tsl::ordered_set`), it needs
**zero** of `z3`/`ordered-map`/`xxHash` — the only real external dependency is `utf8proc` (pulled
in transitively for grapheme iteration via `StringTools.cpp`/`GraphemeIterator.cpp`). Grep the
headers you actually need for `#include "AGRemapCore/..."` chains to work out the real minimal set
for a different subsystem — don't assume the whole `core/CMakeLists.txt` source list is required.

A standalone MSVC compile that worked for this (after `vcvarsall.bat x64`, and after making sure
`api/extern/utf8proc` is populated per the section above):
```bash
cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC \
   /I "<core>/include" /I "<utf8proc-src>" \
   your_test.cpp \
   "<utf8proc-src>/utf8proc.c" \
   "<core>/src/tools/StringTools.cpp" "<core>/src/tools/StringHash.cpp" \
   "<core>/src/tools/grapheme/GraphemeIterator.cpp" "<core>/src/tools/grapheme/GraphemeRange.cpp" \
   /Fe:test.exe
```
Two gotchas that cost real time to work out:
- **`/std:c++23` is not a recognized flag on the MSVC version installed here** (silently ignored
  with `D9002` and falls back to an older default, which then fails to compile C++20/23 code this
  project actually uses, e.g. `unordered_map::contains`) — use `/std:c++latest` instead. Check
  `cl /?`'s `/std:` line if compiling on a different machine/toolset in case this has changed.
- **`utf8proc.c` fails with `C2491: definition of dllimport ... not allowed`** unless you define
  `UTF8PROC_STATIC` — its headers assume a DLL build by default.

Confirmed minimal file set for a second subsystem, the tokenizer/parsing layer
(`tools/parsing/*`, e.g. reproducing a `BaseTokenizer`/`IfPredTokenizer` bug): `StringHash.cpp`,
`StringTools.cpp`, `tools/grapheme/GraphemeIterator.cpp`, `tools/grapheme/GraphemeRange.cpp`,
`tools/parsing/Token.cpp`, `tools/parsing/ParseContext.cpp`, `tools/parsing/SyntaxErr.cpp`,
`tools/parsing/BaseTokenizer.cpp`, `tools/parsing/FilteredTokenizer.cpp` (only if the tokenizer
under test derives from it, e.g. `IfPredTokenizer`/`SympyTokenizer` do), the tokenizer's own
`.cpp`, and `tools/idGenerator/UuidIdGenerator.cpp` (pulled in by the DFA machinery) — plus
`utf8proc.c` per the grapheme dependency above. Also zero `z3`/`ordered-map`/`xxHash` needed. `/std:c++20`
also worked here (as an alternative to `/std:c++latest` above) on the same MSVC install.

**A third subsystem, anything touching `Z3Context`/`Z3Predicate`/`IfPredZ3Generator`/
`Z3IfPredGenerator`/`IfPredPart` directly, genuinely does need `z3`** (unlike the two subsystems
above) — but you almost never need to *build* it yourself, since `cext/z3` is normally already a
populated, installed tree. The extra flags on top of the base recipe above: `/I "<cext>/z3/include"`
at compile time, plus `/link /LIBPATH:"<cext>/z3/lib" libz3.lib` at link time (a standalone `cl`
invocation needs `/link` as its own trailing section — everything after it is linker args, not
compiler args, and it must come after all the `.cpp` sources). The private header
`core/src/tools/z3/Z3Internal.h` (see [Architecture](../Architecture/CLAUDE.md)'s section on
wrapping a third-party library like Z3 for why it exists) needs its own `/I "<core>/src"`, on top of
the usual `/I "<core>/include"`, since it's deliberately not under `include/` and won't resolve
through the normal include path. **The built `.exe` won't run without `libz3.dll` next to it** —
`cext/z3/bin/libz3.dll` needs copying alongside the compiled `.exe` (or onto `PATH`) before invoking
it; the compile and link steps give no indication this is missing, only a failure to launch the
`.exe` does.

**Diagnosing a reported "crash" this way: check whether it's actually an uncaught C++ exception
before assuming memory corruption.** On this Windows/MSVC setup, a `throw` that nothing catches
(e.g. a repro's bare `main()` with no `try`/`catch` around a call that legitimately raises
`AGRemapCore::SyntaxErr` or similar) surfaces as process exit code `-1073740791` / `0xC0000409`
(`STATUS_STACK_BUFFER_OVERRUN`) via the UCRT's fail-fast path for an unhandled exception — this
*looks* exactly like a real stack-smash/buffer-overrun crash from the exit code alone, but isn't
one. Confirmed by wrapping the identical repro call in `try { ... } catch (const SyntaxErr &e) {
printf("%s", e.what()); }` and getting a clean, catchable exception with a sensible message
instead of a crash. Do this wrap-and-catch check first, before spending time hunting for an
out-of-bounds access — if it turns out to be a clean catchable exception, the real bug (if there is
one) is almost always "this code path should never have thrown in the first place for this input,"
a logic bug, not a memory-safety one.

This is a genuinely useful pattern for a targeted regression test that exercises a real lifetime/UB
bug (reference/pointer dangling, use-after-free) empirically — a plain C++ binary run under a
debugger-free, ASan-free MSVC build is still a legitimate confirmation tool for "does the memory
get reused if I disturb the stack between producing and reading a value", and doesn't require the
full pipeline to be running. **Don't assume a Python-side test can substitute for or duplicate
this, though — check first whether the specific buggy method is even reachable from Python at
all.** A pybind11 wrapper method often does its own independent thing rather than forwarding to the
exact C++ overload you fixed (e.g. calling a sibling `*Ptr` overload and building its own safe
by-value return, or having an *earlier-registered, identically-signatured* `.def` of the same
Python method name silently shadow the one you'd expect to reach) — see
[Architecture](../Architecture/CLAUDE.md)'s note on the `getKVP`/`getMaximal` dangling-reference bug
for a concrete case where grepping the real call sites (`grep -n "methodName(" api/src/cpp/py`)
revealed the Python-visible methods never called the buggy C++ methods at all, making a Python-side
verification pass useless for that particular bug, no matter how thorough.

**A crash repro that redirects stdout to a file (e.g. `cmd //c script.bat > log.txt` from
PowerShell, or piping a `cl`-built `.exe`'s output) can lose every `printf`/`std::cout` line if the
process crashes, because stdout is fully *block*-buffered (not line-buffered) once it isn't a real
console** — the buffer never gets flushed on a hard crash (access violation, `abort()`), so a log
file that should show "got this far" progress markers comes back empty or truncated right before the
actual crash point, actively misleading you about where the fault is. Fix: add
`std::setvbuf(stdout, nullptr, _IONBF, 0);` as the very first line of `main()` in any standalone
crash-repro/diagnostic `.cpp` — this one line converts stdout to fully unbuffered, so every line
before the crash is guaranteed to actually reach the file. This is cheap enough to just always add
to a throwaway repro `main()`, rather than debugging it only after getting bitten once.

**Constructing a PowerShell command string with `-c "..."`/`Invoke-Expression`, then having
PowerShell itself re-parse a path containing this repo's own directory name, breaks**: `"Anime Game
Remap (for all users)"` has literal parentheses in it, which PowerShell's parser treats as
expression-grouping syntax when they show up unquoted inside a larger constructed/interpolated
command string, not just as path characters — this is a real, repeatedly-hit gotcha specific to this
repo's own folder name, not a hypothetical. Avoid it entirely by writing the verification/repro
script out to a real `.py`/`.ps1` file (with `Write`) and invoking it by path (`py -3 <path>`)
instead of building up a `-c "..."` one-liner that embeds the repo path.
test for it actively misleading (it would pass regardless of whether the C++ bug was fixed) rather
than just redundant. Confirm reachability before writing or trusting that kind of test; a
standalone `.cpp` like this one isn't wired into any build target here, so nothing will compile/run
it for you automatically — treat it as a manual verification artifact, and say so if you leave one
in the repo, e.g. under a `core/tests/` directory with build instructions in a header comment.

**`core/tests/` is a standing, user-approved scratch location for exactly this** — confirmed
directly with the maintainer, not just inferred. Nothing under it is wired into
`core/CMakeLists.txt`/`py/CMakeLists.txt` (both use explicit, non-glob source lists — verified by
reading both files directly), so a normal build never touches it regardless of what accumulates
there. Feel free to drop temporary standalone verification `.cpp` files here during C++ core work
without asking first. One caveat: the maintainer is planning a dedicated, real unit tester for the
core later — once that exists, the temporary files sitting here will need to be migrated into it,
not left behind as a second, informal test suite; don't delete or treat them as superseded without
checking first once that tester exists.

### When the hand-picked source list stops working: link `AGRemapCore.lib` instead

The source-list approach above scales only while the test's dependency cone stays small. It stops
working the moment a test touches `IniFile::parse`/`IniFile::fix`, which reach `getIfTemplates` and
so drag in the entire Z3/`IfTemplate` half of the core — you get a wall of `LNK2019` on
`Z3Context`, `Z3Predicate`, `IfTemplatePart`, `IfPredPart`. **Don't try to grow the source list to
cover that.** Link the already-built static library instead (`cd cbuild && ninja AGRemapCore` first):

```bash
cl /std:c++latest /EHsc /nologo /MD ^
   /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include /I <repo>/cext/z3/include ^
   Foo_test.cpp /Fe:test.exe ^
   /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
   <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
   <repo>/cbuild/utf8proc/utf8proc.lib ^
   <repo>/cext/z3/lib/libz3.lib ^
   <repo>/cbuild/curl/lib/libcurl_imp.lib
```

Three details on that line are load-bearing:
- **The three `/NODEFAULTLIB` flags.** `AGRemapCore.lib` is built against the DLL CRT (`/MD`) while
  `utf8proc.lib`/`libz3.lib` carry `/DEFAULTLIB` directives for the *static* one. Without them the
  link dies in a wall of `LNK2005 ... already defined in libcpmt.lib(cout.obj)`; switching to `/MT`
  instead just yields the mirror-image `LNK2019 __imp_?...@std@@` on `basic_streambuf`/
  `basic_ostream`. Neither error mentions a CRT mismatch, so both read as a missing library.
- **`utf8proc.lib`, not `utf8proc_static.lib`, and drop `/DUTF8PROC_STATIC`.** The core links the
  shared one; the static variant gives `unresolved external __imp_utf8proc_map`.
- Copy `libz3.dll` next to `test.exe` before running it.

`vcvarsall.bat` may print a harmless `'vswhere.exe' is not recognized as an internal or external
command` line — ignore it, the environment still sets up correctly.

**The recipes written into `core/tests/*.cpp` headers are not all current.** Several predate this
problem and still list a source set that no longer links — `IniFile_classify_test.cpp` is one, since
`IniFile`'s destructor alone now reaches `Z3Context::~Z3Context`. Treat a test file's header recipe
as a hint, not a contract, and fall back to the static-lib line above.

## Migrating a class's associated literal *project data* (not its algorithmic code) into C++

Distinct from porting a class's logic (covered throughout
[Architecture](../Architecture/CLAUDE.md)): some classes (`Hashes`, `Indices`, and similarly
`VertexCounts`/`VGRemaps`) are thin engines wrapped around a large, hand-authored literal data
table (`HashData.py`'s per-character-per-version hash strings, `IndexData.py`'s vertex-start
indices, ...) — genshin-character content data, not code. Whether to migrate the *data* into C++
too (as opposed to just the class/engine sitting on top of it) is a real, separate design
question from porting the class itself, worth surfacing to the user explicitly rather than
defaulting either way — moving frequently-updated content data into compiled C++ trades "edit a
Python dict, no recompile" for "edit C++ source, recompile, and — for this project specifically —
go through the full PR/rebuild/PyPI-release ordeal for every future game-patch update," which is a
real cost some maintainers accept and others don't.

If the answer is yes, the data itself is genuinely correctness-critical (a single wrong hex digit
in a hash string is a silent, hard-to-notice bug, not a compile error) and large enough that
hand-transcribing it is not an acceptable risk. The pattern that worked, done twice now
(`HashData`/`IndexData`), both migrations verified byte-for-byte identical before being wired into
anything real:

1. **Write a one-off Python generator script** (scratchpad, not committed) that `import`s the
   *real, live* Python dict (executing the actual module — never hand-copy the literal text) and
   walks it recursively, both (a) emitting a new C++ source file with the flattened
   `{{"col0", "col1", ...}, "value"},` rows as a `static const std::vector<std::pair<...>>`
   literal — grouped/commented by the walk's own natural boundaries (e.g. a comment per top-level
   version, per name) so the generated file stays visually scannable and diffable against the
   original, not just correct — and (b) dumping the exact same flattened rows to a JSON "golden"
   file for step 3. Preserve the original's iteration order (Python 3.7+ dicts already do, so a
   plain recursive walk is enough) rather than re-sorting.
2. **Compile a tiny standalone C++ program** (see "Fast path: compiling a standalone `core/`
   regression test" above for the `cl` invocation shape) that `#include`s only the new data
   file and prints every row in the same flat shape (a naive `printf`-based JSON dump is fine —
   this data has no embedded quotes/backslashes to escape).
3. **Diff the two JSON dumps programmatically** (row count, exact per-row equality in original
   order, and a set-equality check as a second, order-independent cross-check) — not a manual
   read-through, and not "looks right." This is the step that actually catches a transcription
   bug, and it caught nothing here (both migrations came back byte-identical on the first try) —
   but treat that as confirmation the process works, not a reason to skip it next time.
4. Only after this passes, wire the new data file into `py/CMakeLists.txt` and whatever binding
   class consumes it. Don't skip straight to step 4 "since the generator script looked right" —
   the whole point is that a script bug is just as capable of silently corrupting data as a typo
   would be; the round-trip diff is what actually proves correctness, not the generation method.

**Don't silently "fix" what looks like a data bug found this way.** The generator will faithfully
reproduce whatever the live source actually contains, bugs included — this project's real
`HashData.py` had two hash strings with stray embedded whitespace (`"29cf09   14"`,
`"b0e089    15"`), almost certainly pre-existing copy/paste typos, unrelated to the migration
itself. Preserve them exactly in the migrated data (that's what "verified identical" means) and
flag the suspected bug separately for the user to confirm and fix deliberately, rather than
quietly correcting it as part of an unrelated migration.

**Check for other public entry points that expose the same raw data independently of the class
being ported**, before assuming the class itself is the only consumer. `HashData`/`IndexData`
were each reachable two ways beyond `Hashes`/`Indices` themselves: a directly re-exported
module-level name (`FixRaidenBoss2.HashData`) *and* a `DeferredEnum`-based registry
(`ModData.Hashes.value`) — both need to keep returning the exact same nested-dict shape after the
literal data moves into C++, or it's a real breaking change to documented public API. The fix that
avoided a second copy of the data existing anywhere: add a genuinely reusable export/reconstruction
capability to the C++ side once (`ModDictAssets::forEachEntry` → a new pybind `toNestedDict()`
method rebuilding the original nesting, re-inserting the version column at its original position),
then rewrite the old Python data module (`HashData.py`) to a 3-line "reconstruct once from the live
C++ instance at import time" shim instead of deleting it outright — this keeps every existing
import path (`from .data.HashData import HashData`, `ModData.Hashes`) working unchanged, with the
C++ table as the one real source of truth.

## A new shared-lib dependency needs its own `install(FILES ...)` line — `target_link_libraries` alone isn't enough

`AGRemapCore` links several externs as **shared** libraries on Windows (produce a `.dll`, not just a
`.lib`): `z3::libz3`, `utf8proc`, and `CURL::libcurl` (see `core/CMakeLists.txt`'s
`target_link_libraries(AGRemapCore PRIVATE ...)` block). `Compressonator`'s `CMP_Compressonator`/
`CMP_Framework` are the exception — they build as **static** libs here, so they need no DLL install
step at all; don't assume every extern in that list needs the same treatment without checking
whether it actually produced a `.dll` under `cbuild/` first.

Each shared extern's `.dll` has to be explicitly copied next to `core.pyd` via
`py/CMakeLists.txt`'s `if (WIN32) install(FILES $<TARGET_FILE:...> DESTINATION FixRaidenBoss2) endif()`
block — CMake does **not** do this automatically just because `AGRemapCore` links against the
target. Confirmed hitting this directly: that block only listed `z3::libz3`, so `utf8proc.dll` and
`libcurl.dll` were silently never installed even though `main.py -d`/`cmake --install` "succeeded"
with no error. The only symptom on the Python side was
`ImportError: DLL load failed while importing core: The specified module could not be found` —
generic, doesn't name the missing DLL, and easy to mistake for `core.pyd` itself being broken/stale
rather than one of its *dependencies* being absent.

**If this resurfaces (a new shared extern added later, or this install block regresses):**
1. Diagnose with `dumpbin /dependents <path-to-core.pyd>` (needs `vcvarsall.bat` sourced first, per
   above) to list every DLL `core.pyd` actually imports, then check which of those aren't present
   next to it in `FixRaidenBoss2/`. **Run it via the Bash tool with
   `MSYS2_ARG_CONV_EXCL="*" dumpbin.exe /dependents "<path>"`** — Git Bash otherwise mangles the
   single-dash `/dependents` flag into a path (`Not a file: .../dependents`).
2. The fix is a one-line addition to the `if (WIN32) install(FILES ...)` block in
   `py/CMakeLists.txt`: add `$<TARGET_FILE:<newTargetName>>` alongside the existing entries — same
   pattern as `z3::libz3`/`utf8proc`/`CURL::libcurl`. Use the exact target name from
   `core/CMakeLists.txt`'s `target_link_libraries` line (namespaced alias like `CURL::libcurl`, or
   plain like `utf8proc`, whichever that extern actually exports).
3. After editing, a plain `cmake --install <buildFolder> --prefix api/src/py` (no recompile needed
   if no source changed) picks up the new install rule immediately — CMake auto-reconfigures because
   `CMakeLists.txt`'s timestamp changed. Verify by checking the `-- Installing: .../FixRaidenBoss2/<dll>`
   lines in its output, then re-run the "Verifying a build/binding change in Python directly"
   import check above (from PowerShell) to confirm the `ImportError` is actually gone.

## Cython pieces
`api/src/cy` has its own small CMakeLists, built automatically as part of the same top-level
`api/CMakeLists.txt` orchestration (skipped only in `core`/`core_sdk` env mode). No separate
step needed — the same `py -3 main.py` invocation above rebuilds Cython sources alongside the
C++ core/pybind11 pieces. See [Architecture](../Architecture/CLAUDE.md)'s "Cython bindings"
section for the source-layout/wrapper-class conventions to follow when adding a new method here
(verified via one hands-on pass adding `CyDictTools.getVal`).

## Adding a brand-new source file — registration is never automatic

None of the three build layers discover new files by scanning a directory; each has an explicit
source list that a brand-new file needs adding to by hand, or it's silently just never compiled
(no error — the build succeeds without it):
- **`core/CMakeLists.txt`**: a new `core/src/.../Xxx.cpp` needs its own line in
  `add_library(AGRemapCore STATIC ...)`'s source list.
- **`py/CMakeLists.txt`**: a new `py/src/.../PyXxx.cpp` needs its own line in
  `pybind11_add_module(core ...)`'s source list, *and* `PyXxx.h`'s `initCppXxx(m)` needs an
  explicit `#include` + call added inside `PYBIND11_MODULE(core, m) { ... }` in `bindings.cpp` —
  adding the `.cpp` to CMake without wiring the `init` call compiles and links fine, the new
  class/method is just silently absent from the Python-visible module.
- **`cy/CMakeLists.txt`**: a new `cy/src/.../Xxx.pyx` needs its own
  `add_cython_module(CyXxx src/tools/Xxx.pyx)` line.

On top of all three: a **brand-new pybind11-bound class or Cython class** (not just a new method
on an existing one) additionally needs registering in `api/src/py/FixRaidenBoss2/__init__.py` —
both a `from .core import Xxx` (or `from .CyXxx import CyXxx`) line *and* an entry in that file's
`__all__` list — or it's unreachable as `FRB.Xxx` even though the build succeeded and the `.pyd`
installed correctly. The failure mode is worth knowing since it isn't the plain `AttributeError`
you'd expect from "forgot to export it": for a Cython module specifically, Python already
auto-registers the *submodule* itself as a package attribute on import elsewhere, so `FRB.CyXxx`
silently resolves to `<module 'FixRaidenBoss2.CyXxx' ...>` instead of the class, and calling it
raises `TypeError: 'module' object is not callable`. Confirmed by hitting this directly while
adding `CyHashTools`.

**This same two-places rule also applies to a plain pure-Python class under `model/...` that
already exists on disk and is already fully implemented** — a completed class is not necessarily
registered. Confirmed hitting exactly this for `BaseIniGraphPartEdit`
(`model/strategies/iniFixers/BaseIniGraphPartEdit.py`): the class itself had a real, working
implementation, but no `from .model...BaseIniGraphPartEdit import BaseIniGraphPartEdit` line
existed anywhere in `__init__.py` at all, so `FRB.BaseIniGraphPartEdit` raised `AttributeError`
until both lines were added by hand. Don't assume "the file exists and looks done" means "it's
reachable as `FRB.Xxx`" — grep `__init__.py` for the class name (both the import line and its
`__all__` entry) before relying on it, especially when completing a stub whose sibling classes
were registered at a different time than the stub itself was scaffolded.
