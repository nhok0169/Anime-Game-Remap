# Building

How to compile the C++ core, the pybind11 bindings, and the Cython extensions. See
[Overview](../Overview/CLAUDE.md) for repo layout and why the compiled binaries matter, and
[Testing](../Testing/CLAUDE.md) for why a rebuild has to happen before tests will reflect a
native-code change.

## Prerequisites (Windows/MSVC, the dev environment this file assumes)

> **If you're setting up from a fresh clone, read [Setup](../Setup/CLAUDE.md) first** — it was
> written from an actual cold-start run and corrects several items in this section (there is no
> committed `.pyd`, so the Python version isn't pinned by one and 3.13 works fine; Ninja and CMake
> come with the VS "C++ CMake tools" component rather than being separate installs; `pybind11`
> must be 3.x, and 3.0.4 specifically to avoid `core.pyi` churn; `-d` needs Doxygen but not
> PlantUML/Java).
>
> **This whole file describes the Windows/MSVC build only.** The project also builds on Linux
> (WSL2 / Ubuntu 24.04, GCC 13, Ninja, a pip-installed CMake) — that toolchain, its build-folder
> `lin` suffix convention, and the traps of sharing one checkout between both OSes live in
> [Setup](../Setup/CLAUDE.md)'s Linux section. Nothing below is Linux-specific, and the MSVC-only
> details (`vcvarsall.bat`, `/std:c++latest`, `cl` link lines) do not carry over.
>
> One rule spans both platforms and is worth internalising before your first build: **`APIBuilder`
> invokes `cmake` and `doxygen` by bare name**, so they must be on `PATH` *in the shell that runs
> it* — selecting a particular Python interpreter does not help, and running a venv's
> `bin/python` by absolute path does not activate that venv. Check with
> `command -v cmake ninja doxygen` (or `where` on Windows) first.

- **Read the Python version off the machine — this bullet has now been wrong in both directions.**
  `py -0p` lists what is installed; `cbuild/CMakeCache.txt`'s
  `FIND_PACKAGE_MESSAGE_DETAILS_Python` line says what the existing build tree was configured
  against. As of **2026-09-07** those agree on **3.13** (`py -0p` lists 3.13 as the default plus
  3.11 and 3.7; the installed module is `core.cp313-win_amd64.pyd`; the cache reads `v3.13.1`).
  Successive revisions of this file have asserted 3.9, then 3.13, then 3.9, and now 3.13 again —
  **four flips.** Do not read the number above as fact either; it is a dated observation and the
  next agent's `py -0p` is the authority. Treat every `cp313` in the rest of this file as
  illustrative of the *shape* of the filename, not of the version you will actually see. Nothing
  is committed for any version (see Overview); the only thing that matters is that the interpreter
  running the tests matches the one the `.pyd` was built for. Ask before changing it.
- **`python` and `py -3` are not necessarily the same interpreter here.** Measured 2026-09-10:
  bare `python` on `PATH` is **3.9.13** (a WindowsApps entry ahead of it in `PATH` resolves there),
  while `py -3` is **3.9.3** at `AppData\Local\Programs\Python\Python39\python.exe` --- which is
  the one `cbuild` was configured against. They share the `cp39` ABI so a `.pyd` loads under either,
  which is exactly why the difference goes unnoticed. When something spawns python, have it spawn
  `sys.executable` rather than a bare name; `Utils/pipeline/Stage.py` used to get this wrong and ran
  every CI pipeline stage under the other interpreter.
- Visual Studio (MSVC) with the C++ toolchain, CMake, Ninja.
- The MSVC dev environment must be initialized in-shell first:
  ```bash
  call "<path-to-VS>\VC\Auxiliary\Build\vcvarsall.bat" x64
  ```
  Find the exact path once with
  `find "/c/Program Files/Microsoft Visual Studio" "/c/Program Files (x86)/Microsoft Visual Studio" -maxdepth 6 -iname vcvarsall.bat`
  — and actually run it, because this path keeps moving. As of **2026-09-07** the only VS 18 on
  this machine is the **BuildTools** install under `Program Files (x86)`:
  `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat`,
  and `Program Files\Microsoft Visual Studio\18` does not exist. That is the reverse of what the
  2026-09-06 revision recorded, which was itself the reverse of the one before it. **This path and
  the Python version have now flipped back and forth three times between them. Run the `find` — do
  not paste either value out of this file, this sentence included.** Everything below assumes
  `vcvarsall.bat` has been run in the same shell.
  - **If you're an AI agent driving this through a tool whose shell state doesn't persist between
    separate tool calls** (env vars set in one call are gone by the next, even though the working
    directory may persist) — `vcvarsall.bat` and the actual build command must happen inside one
    single invocation. Inlining `cmd //c '"...\vcvarsall.bat" x64 && py -3 main.py'` directly as a
    Bash-tool command tends to break on the nested quoting (the outer single-quote/inner
    double-quote mix gets mangled going through Git Bash). The reliable pattern: write a small
    `.bat` file to the scratchpad with `call "...\vcvarsall.bat" x64`, an errorlevel check, then
    `cd /d` into `Tools/APIBuilder` and the actual `py -3 main.py ...` line — then invoke just that
    one `.bat` path via `cmd //c <path>` (unquoted if the scratchpad path has no spaces, which it
    won't). Inside such a `.bat`, invoke a freshly-built `.exe` by its **full path** too — a bare
    `logger_test.exe` right after `cd /d` into its own folder still failed with `is not recognized`
    while the file was plainly there. Run it via the tool's background mode and tail the log; don't try to poll for
    completion, wait for the completion notification instead.
  - **Pass that `.bat` as a full *Windows* path, not a bare filename — even after `cd`-ing into the
    directory that holds it.** `cd "$SP" && cmd //c build_core.bat` fails with
    `'build_core.bat' is not recognized as an internal or external command`, because the Git-Bash
    cwd isn't what `cmd` resolves against. Convert explicitly:
    `cmd //c "$(cygpath -w "$SP/build_core.bat")"`. The error message reads like a missing file, so
    it's easy to waste time re-checking that the `.bat` was written correctly when the path form is
    the actual problem.
  - **The simplest route that is confirmed to work (2026-09-03) is to skip the Bash tool for the
    launch entirely and use the PowerShell tool:**
    `cmd /c "C:\...\scratchpad\build_core.bat" 2>&1 | Out-File -Encoding utf8 <log>` with
    `run_in_background: true` for a `ninja` build, or in the foreground with a long timeout for a
    standalone `cl` test compile. Two failure modes to recognise on sight, both of which *report
    success*: a hand-typed Windows path given to the Bash tool's `cmd //c` (even with doubled
    backslashes) arrives as `'C:UsersAlexX...build_core.bat' is not recognized` -- the batch never
    ran, and the harness still says exit 0 -- and PowerShell's
    `Start-Process cmd.exe -ArgumentList '/c', '"<bat>" > <log>'` returns a PID immediately but never
    writes the log. In both cases the *old* `.pyd` stays installed, and every test you run afterwards
    is quietly exercising stale code. Check `core.cp313-win_amd64.pyd`'s mtime before trusting any
    result. Data point for pacing, **re-measured 2026-09-08 after the build-speed work below**:
    touching a widely-included core header and running `ninja` to completion is about **2 minutes**,
    and a one-line change to a single `core/src/*.cpp` is about **8 seconds**. Both were far worse
    before (137.9s and 57.2s); if you are seeing the old figures, check which options `cbuild` was
    configured with -- see "Build speed" below.

### The CIPipeline builds the API now, with `-d`

`Tools/CIPipeline`'s first stage runs `APIBuilder -d` (added 2026-09-10), so a pipeline run needs
everything a build needs --- `cmake`, `ninja` and `doxygen` on `PATH`, and the MSVC environment
initialized in the same shell --- and **regenerates the tracked `core/xml` and `core.pyi` every
time**, around 966 files. The rules below about which of those to keep apply unchanged; for a
tooling change the answer is neither. See [Tools](../Tools/CLAUDE.md).

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

> **The entry point is `main.py`, and nothing else is.** `Tools/APIBuilder/APIBuilder/` is a
> package, not a CLI: `py -3 -m APIBuilder.APIBuilder <flags>` **exits 0 having built nothing**,
> and `py -3 APIBuilder/APIBuilder.py` dies on `attempted relative import with no known parent
> package`. The silent one cost a full session — a stale `.pyd` kept serving old bindings while
> every "build" reported success, and the failure only surfaced hours later when a rename made a
> symbol genuinely disappear. **Verify a build by the `.pyd`'s mtime, never by its exit code.**
>
> Related trap: `-pb`/`-pi` are `--makePreBuild`/`--makePreInstall`, which are about *external
> dependencies* (z3, Compressonator). They are not "build the project" flags, and passing one
> instead of running a plain `main.py` builds none of your code.

`py -3 main.py` (no `-d`) is the normal edit-compile-test build: it compiles `AGRemapCore`, the
`core` pybind11 module and the Cython extensions, **and runs CMake's install step**, which is what
copies `core.cp313-win_amd64.pyd` into `api/src/py/FixRaidenBoss2/`. You never have to stage that
file by hand.

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
  - **The committed `core/xml` is itself far out of date, so "keep it" does not mean "commit
    whatever Doxygen just produced".** Measured 2026-09-06: a single clean `doxygen Doxyfile` run
    (Doxygen 1.17.0, matching the pin) produced **147 modified files plus 31 brand-new ones**, on a
    change that touched exactly one header's doc comments. Practically none of that is yours —
    it's accumulated drift from every earlier session that (correctly, per the rule above) discarded
    the XML. Committing all of it buries your actual change under ~178 unrelated files and makes the
    diff unreviewable. **Keep only the XML files for the header(s) you actually edited** and revert
    the rest; the kept files' remaining noise is just `<location line="...">` shifts from your own
    insertion, which is fine. Concretely, for `TextureFile.h` that meant keeping
    `_texture_file_8h.xml`, `_texture_file_8cpp.xml` and
    `class_a_g_remap_core_1_1_texture_file.xml` and `git checkout --`-ing the other 144 (plus
    `git clean` for the 31 untracked ones, which belong to *other* classes that were never
    regenerated). Resyncing `core/xml` repo-wide is a legitimate task — just make it its own commit,
    not a passenger on a feature.
  - Judge a Doxygen run by its log, not its exit code: piping `doxygen Doxyfile` through
    PowerShell's `Select-String` reported a non-zero exit while the same run redirected to a file
    exited 0. Redirect to a log and grep it for `warning:` lines mentioning *your* file.
- Build artifacts land in `cbuild/` (CMake build dir), external deps in `cext/`/`cebuild/`, all
  at the repo root — these are safe to delete and let the next build regenerate
  (`-b /`, `-pir /`, `-p /` to do that explicitly; `*` instead of `/` nukes every suffixed
  variant too).
- `-e core` builds only the C++ core as a static lib for external C++ consumption (no Python
  bindings) — not what you want for a normal Python-visible feature.
- `-s`/`--skipBuild` reinstalls without recompiling; `-i`/`--installKeep` preserves the previous
  install instead of overwriting.
- `-c`/`--addCredits` rewrites the credits boiler plate in every API source file that has the
  `##### Credits`/`##### EndCredits` keywords -- all four layers, `.py`/`.pyx`/`.h`/`.tpp`/`.cpp`.
  It touches *only* the text between those two keywords, and a file without them is skipped
  entirely, so it cannot add credits to a brand-new file. It rewrites nothing when the block is
  already correct, so `-c -s -i` (credits only, no compile, keep the installed `.pyd`) is a safe
  two-second check that costs nothing when there is nothing to do. See
  [Overview](../Overview/CLAUDE.md)'s "Every source file in `api/src` carries a credits block".
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
  `.pyd` out yourself before testing: `cbuild/src/cpp/py/core.cp313-win_amd64.pyd` →
  `api/src/py/FixRaidenBoss2/core.cp313-win_amd64.pyd` (and `cbuild/src/cy/cython/Cy*.pyd` →
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

## Build speed: five switches, and what each one actually measured

Added 2026-09-08 after profiling the build end to end. **Every number here is a stopwatch on this
machine** (Xeon w5-3423, 12 cores / 24 threads, 31GB, MSVC 14.50, Ninja) --- re-measure rather than
trust them if the hardware or toolset moved, the same way you re-check the Python version and the
`vcvarsall.bat` path.

Where a rebuild's time went **before** any of this existed, for the two cases that matter:

| what you changed | wall time |
| --- | --- |
| one `.cpp` under `core/src` | 57.2s |
| one widely-included header under `core/include` (107 rebuild edges) | 137.9s |

Almost all of that was **two serial things**, not the parallel compile:

- **The LTCG link, 56.0s, single-threaded, on every build no matter how small.** That was the whole
  of the 57.2s floor: a one-line `.cpp` change compiled in about a second and then waited a minute
  for the link.
- **`VGRemapData.cpp`, 80.4s in one translation unit**, which gated `AGRemapCore.lib` while the
  other 45 recompiled files sat finished. It is one function holding a single static initializer of
  52 `VGRemap` rows built from 5,229 nested `{a, b}` pair initializers, and MSVC's optimizer is
  superlinear in the size of that one expression (38.6s at `/O2`, 37.7s at `/O1`, **5.5s at `/Od`**,
  measured standalone). `HashData.cpp` looks similar and is *not* affected (9.4s vs 9.0s), so don't
  generalise this to `data/`.

Today those are 4.6s and 14.2s, and the floor for a one-`.cpp` change is **8.1s**.

### The switches

| option | default | what it does |
| --- | --- | --- |
| `AGREMAP_ENABLE_LTO` | **OFF** for `python_dev`, ON for `cibuildwheel`/`core_sdk` | pybind11 links `pybind11::lto` unless told otherwise, which on MSVC is `/GL` + `-LTCG` |
| `AGREMAP_PCH` | ON | precompiled header for the `core` pybind module |
| `AGREMAP_PCH_LIB` | ON | precompiled header for `AGRemapCore` |
| `AGREMAP_SCCACHE` | OFF (but **ON in this machine's `cbuild`**) | routes every compile through sccache; forces both PCHs off |
| `AGREMAP_UNITY_BUILD` | OFF | jumbo-compiles `AGRemapCore` in batches of 16. Wired up, measured, and **rejected** --- see below |

**Shipped wheels are unaffected by all of this.** The LTO default keys off `BUILD_MODE`, and the
other four are off or irrelevant in the wheel path.

### `AGREMAP_SCCACHE` and the precompiled headers are mutually exclusive --- do not run both

sccache **will not cache a compilation that uses a PCH.** It does not warn; it reports the
compilations under `Non-cacheable reasons: /Fp` in `sccache --show-stats` and pays its own overhead
for nothing. That is why turning `AGREMAP_SCCACHE` on turns both PCH options off in
`api/CMakeLists.txt`. Running both is strictly the worst configuration available.

Pick per workload; the trade is real in both directions:

| | first-ever compile of that content | content seen before |
| --- | --- | --- |
| PCH | **112.9s** | 112.9s |
| sccache | 135.5s | **11.3s** |

- **PCH** wins when you are editing in one long-lived tree, where every compile is of something you
  just typed and can never be a cache hit. Worth about 13-17%.
- **sccache** wins, by roughly 10x, whenever content repeats: undoing an edit, switching branches
  and back, a wiped `cbuild/`, or **a fresh `git worktree`** --- and that last one is where this
  repo's long builds actually live. Rebuilding both targets from zero objects took **19.7s at a
  100% hit rate**, against ~183s of real compiling. The cold pass that filled the cache covered all
  637 compiles including curl and Compressonator, and cost 112MiB against sccache's 10GiB
  default. Flipping the whole tree between the two configurations afterwards --- which changes
  every compile flag and so invalidates every object --- rebuilt all 637 in **32s at a 100% hit
  rate**.

Cache hits **survive being in a different checkout**: the same source compiled through a second
path to the tree, with entirely different `-I` arguments, still hit. So a new worktree gets the
main checkout's cache for free, which is the single biggest lever in this file.

sccache costs about 8% on a miss (5.8s vs 6.2s on one binding source), and needs `sccache` on
`PATH` --- `winget install Mozilla.sccache`. Configure fails loudly if it is missing rather than
silently skipping.

```bash
# switch this tree to sccache (cache variable -- it sticks; main.py picks it up afterwards)
cmake -G Ninja -B cbuild -DAGREMAP_SCCACHE=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=cext/z3
# ...and back to precompiled headers
cmake -G Ninja -B cbuild -DAGREMAP_SCCACHE=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=cext/z3
```

### Two traps that make a "speed-up" silently a slow-down

**Any source with per-file `COMPILE_OPTIONS` must also be excluded from the PCH and from any unity
batch.** `VGRemapData.cpp` compiles at `/Od` while the PCH is built at `/O2`. MSVC does not reject
the mismatch --- it just stops being able to use the PCH, and the file pays for the whole
force-included header set alone: **14.2s excluded, 144.2s included**, worse than before `/Od`
existed, and enough on its own to turn the PCH into a net regression (a header change went 132.6s
to 150.5s until `SKIP_PRECOMPILE_HEADERS` was added). Unity batches drop such flags outright, which
is the same bug with no warning at all.

**Never put a `core/include` header in either PCH.** A PCH is rebuilt, and every consumer of it
recompiled, whenever anything inside it changes --- so this project's own headers would turn a
one-header edit into a guaranteed full-target rebuild, i.e. exactly the case the PCH exists to make
cheaper. Both lists are third-party and standard-library headers only, deliberately.

### Measuring this yourself: read `cbuild/.ninja_log`, not a stopwatch

Ninja records `start_ms  end_ms  mtime  output  hash` per edge, so the whole critical path is
already on disk. Summing the durations and comparing against the wall span is what showed the build
was reaching only **12x parallelism on 24 threads** and that 136 of 138 seconds were those two
serial items. Do that before optimising anything --- both of the changes that mattered were
invisible in a per-file average.

**And beware of how you fake the edit.** Touching a file's *mtime* makes ninja rebuild it, which is
a fine way to measure compile work --- but it is worthless for anything cache-related, because the
content is unchanged and sccache keys on content, so you measure a guaranteed 100% hit. Change the
bytes when a compiler cache is in play.

### Rejected after measuring: `AGREMAP_UNITY_BUILD`

Jumbo-compiling `AGRemapCore` in batches of 16 is implemented and works, and it is **off**, because
on this machine it measured as a net loss (all rows with `/Od` already applied):

| | one `.cpp` | one header |
| --- | --- | --- |
| no unity, no LTO | **8.6s** | 129.5s |
| unity, no LTO | 25.5s | 123.6s |

It buys ~6s on a header change and costs ~17s on the far more common single-file change, because 24
hardware threads against ~150 translation units were never short of parallelism --- batching mostly
converts parallel work into serial work, and makes ninja rebuild all 16 files when you touch one.
That inverts on a low-core machine, which is why the switch is still there.

If you do turn it on, note that **17 files under `core/src` must stay excluded** (already listed in
`core/CMakeLists.txt`): they share anonymous-namespace helper names with each other --- `ModObj`,
`BlendHashKey`, `IbHashKey` and `prototypeRepo` in four files each, `buildRows` in three, 15 more in
two --- and a batch turns each into a redefinition error. `py/src` has 21 such clashes of its own,
which is why the `core` pybind target is not unity-built at all.

A scan for these that only looks for `namespace {` at *file scope* reports zero clashes and is
wrong: nearly all of them sit inside `namespace AGRemapCore { ... }`. Brace-count instead.

## Run long builds in the **background** --- a foreground timeout kills the link and leaves you a half-built tree

> **Corrected 2026-09-08.** This section used to open by saying the final
> `Linking CXX shared module ... core.cp313-win_amd64.pyd` step "can take well over ten minutes on
> this machine". That has not been true for a while and is not true now: measured four times, the
> link took **54-56s** while pybind11's LTO was on, and takes **4.6s** with it off, which is the
> `python_dev` default since the build-speed work (see "Build speed" above). Don't build
> background-and-poll scaffolding around a five-second step.

The *habits* below still earn their keep, because a full rebuild is still minutes long and the
failure mode is silent. If the tool call running a build hits its timeout the link is killed: the
log stops after the last `Building CXX object` line, `.obj` files are all present, and **no `.pyd`
is produced** --- so a `cp` of the build output afterwards silently installs the *previous* build
and you test stale code.

Two habits that avoid it:

- Launch long builds with `run_in_background`, then poll the log for the terminal line rather than
  blocking on the command:
  ```bash
  until grep -qE "Linking CXX shared module|FAILED|ninja: build stopped" "$LOG"; do sleep 25; done
  grep -E "error C|FAILED" "$LOG" | head
  ```
- **Check for that line before copying the `.pyd`.** "No `error C` in the log" is not the same as
  "the build finished" --- a killed build has no errors either. If the log's last line is a
  `Building CXX object`, re-run the build; ninja will pick up where it stopped.

When only `AGRemapCore.lib` is needed (a standalone `core/tests/*.cpp` run, or a pure-core change
you are not yet testing from Python), build just that target --- it is a fraction of the time:

```bash
ninja AGRemapCore    # instead of: ninja core
```

**A data-table change is not the 8-second case, and the install step removes the old `.pyd`
first (2026-09-09).** Editing only `core/src/data/VGRemapData.cpp` rebuilt every `py/` object and
relinked `core.pyd` for about 30 minutes on this machine (`link.exe` climbing past 1.4 GB, so LTCG
was on for that link -- check what the build directory was configured with before assuming the
python_dev defaults), and `core.cp39-win_amd64.pyd` was gone from `FixRaidenBoss2/` from the start
of the install step until the link finished. Two consequences: run it in the background and wait
for the notification, and **run nothing that imports the API meanwhile** -- a test, the finder's
benchmark, a notebook cell -- or it either fails to import or holds the file the copy needs.

**So batch every edit into ONE pass before rebuilding -- comment-only header tidy-ups included
(2026-09-10).** The note above is the general case rather than a quirk of `VGRemapData.cpp`: the
`.pyd` relink dominates, so a one-line change to a single `core/src/*.cpp` costs about the same
here (~15 minutes measured) as a change to a widely-included header. The 8-second figure in "Build
speed" is `ninja AGRemapCore` -- the static library on its own -- and stops applying the moment the
build has to produce something Python can import.

Landing four characters cost three of those rebuilds, and one was avoidable: a missing blank line
and a stale `Stub for ...` doc sentence, both spotted while the first build was already running.
Read your own diff **before** starting a build rather than while waiting for it, and write the
tidy-ups into the same patch script as the real change.

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

**Nearly every tracked text file in this repo is CRLF** (`core.autocrlf=true`; `file` reports
"with CRLF line terminators" for `CMakeLists.txt`, `bindings.cpp`, `__init__.py`, `Docs/src/*.rst`,
these `.md` files, the test package). A Python patch script that matches exact strings containing
`\n` against such a file finds **0** occurrences, and its assert reads like a drifted anchor rather
than a line-ending problem — two whole patch runs were lost to this in one session. Read bytes,
`crlf = b"\r\n" in raw`, `.replace("\r\n", "\n")` before matching, re-expand on write. Files the
Write tool creates are LF, which git normalises on commit — mixing is harmless, matching is not.
Relatedly, right after regenerating `core/xml` `git status` shows ~680 modified files until git
renormalises them (a `git stash`/`pop` or any `git add` does it); `git diff --ignore-space-at-eol
--stat` shows the real content changes underneath.

**The Bash tool's heredocs eat backslash escapes, which silently corrupts any patch script that
writes C++ or Python string literals.** A `<<'PY' ... PY` block is quoted against *variable*
expansion, not against backslash processing on the way in --- so a `"\n"` you wrote inside the
heredoc arrives in the file as a real, literal newline. In a `.cpp` that surfaces as a wall of
`error C2001: newline in string literal` (pointing at lines you never touched, since every following
line shifts); in a `.py` it usually just produces wrong output with no error at all. This has been
hit repeatedly, across multiple sessions, and costs a full write-diagnose-repair cycle every time.

Two reliable ways around it, in order of preference:

1. **Write the patch script to the scratchpad with the `Write` tool and run it by path.** The
   `Write` tool does no escape processing, so what you typed is what lands on disk. This is the
   default --- it also gives you a re-runnable artifact if the patch needs a second pass.
2. If you must inline it, build the escapes at runtime rather than writing them literally:
   `BS = chr(92); NL = BS + "n"`, then concatenate.

The same applies to `\t`, `\\`, and `\"`. Note this is a *different* failure from the PowerShell
one below --- different tool, different mechanism --- so avoiding one does not protect you from the
other.

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
   <repo>/cbuild/curl/lib/libcurl_imp.lib ^
   <repo>/cbuild/Compressonator/cmp_compressonatorlib/CMP_Compressonator.lib ^
   <repo>/cbuild/Compressonator/cmp_framework/CMP_Framework.lib ^
   <repo>/cbuild/Compressonator/cmp_core/CMP_Core.lib ^
   <repo>/cbuild/Compressonator/cmp_core/CMP_Core_SSE.lib ^
   <repo>/cbuild/Compressonator/cmp_core/CMP_Core_AVX.lib ^
   <repo>/cbuild/Compressonator/cmp_core/CMP_Core_AVX512.lib ole32.lib
```

Three details on that line are load-bearing:
- **The six Compressonator libs, plus `ole32.lib`.** Needed the moment anything in the binary
  reaches `TextureFile` -- which now includes every fixer with a texture edit, since
  `TexEditorReplace` pulls in `RemapTexEditResource`. Without them you get seven `LNK2019`s naming
  `CMP_LoadTexture`/`CMP_ConvertMipTexture`/... and one for `CoInitializeEx`, which is what
  `ole32.lib` answers (Compressonator's DDS plugin constructor calls it). The failure appears when
  you add a texture edit to a character, not when you touch the tests.
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

### Rebuild `AGRemapCore.lib` BEFORE the test, or a header change crashes it with no output

The line above compiles the test against `core/include` and links the **already-built** static
library. So if you changed a header — added a member to a class, changed a struct — and did not
run `ninja` first, the test's idea of that type and the library's disagree, and nothing warns you:
the compile succeeds, the link succeeds, and the `.exe` dies with

```
RUN_EXIT=-1073741819        # 0xC0000005, an access violation
```

**and prints not one line first**, so it does not even look like a test failure. Adding three
members to `FileDownload` produced exactly this on 2026-09-10. The rule is simply: `ninja` first,
every time, and treat a no-output access violation from a standalone test as a stale-library
mismatch until proven otherwise rather than as a bug in the test.

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

## Manual fallback: `ninja` directly
Only when you deliberately want to skip the install step (e.g. rebuilding just to run the
standalone `core/tests/*.cpp` executables, which link the static lib rather than the `.pyd`):
```bash
# from a shell where vcvarsall.bat x64 has already been called -- tool shells do not persist it,
# so keep a one-shot .bat that calls vcvarsall and then ninja
cd <repo-root>/cbuild && ninja
```
Then copy `cbuild/src/cpp/py/core.cp313-win_amd64.pyd` over
`api/src/py/FixRaidenBoss2/core.cp313-win_amd64.pyd` yourself, because nothing else will.

If you also need the stub by hand:
```bash
PYTHONPATH=<api/src/py> py -3 -m pybind11_stubgen FixRaidenBoss2.core -o <api/src/py> --root-suffix ""
```
**Run that from Bash, not PowerShell** — PowerShell swallows the empty-string argument and
stubgen fails with `--root-suffix: expected one argument`. Check `pybind11.__version__` is 3.0.4
first or the regenerated stub churns against the committed one.


## Build hygiene: three ways a "successful" build leaves you testing something else

All three cost a debugging cycle in one session. None of them is exotic; they are what happens when
you drive `ninja` yourself instead of going through `main.py`.

**1. Never run two builds against the same `cbuild/` at once.** Ninja does not lock its build
directory. Two overlapping runs fight over the same `.obj` paths and MSVC reports
`fatal error C1083: Cannot open compiler generated file: ... Permission denied` — which reads like a
missing header or a broken toolchain and is neither. If you background a build, **wait for it** (or
poll `tasklist` for `ninja`/`cl.exe`/`link.exe`) before starting the next.

**2. `NINJA_EXIT=0` is not "the build landed".** The install step is a separate `copy`, and it fails
when something holds the destination `.pyd` open — most often **a `FixRaidenBoss*.py` run still
sitting at `== Press ENTER to exit ==`**, which keeps the module loaded indefinitely. The symptom is
`0 file(s) copied.` and nothing else. Always
`echo COPY_EXIT=%errorlevel%` + `if errorlevel 1 exit /b 1` after the copy, and verify by the
destination `.pyd`'s mtime before trusting a suite result. Ask the user to close the run rather than
killing their process.

**And if you need the build NOW: Windows will not let you overwrite a loaded DLL, but it will let
you RENAME one.** So rename the locked file aside and copy the fresh one into its place — the
process that has it open keeps its handle to the renamed file and is undisturbed:

```powershell
Rename-Item $live ("core.cp39-win_amd64.pyd.locked-" + (Get-Date -Format "HHmmss"))
Copy-Item "<cbuild>\src\cpp\py\core.cp39-win_amd64.pyd" $live
```

Delete the renamed leftover once the user's run exits (it is untracked but NOT gitignored, so it
shows up in `git status` until you do). Check `Get-CimInstance Win32_Process` for the command line
before assuming a lock is yours — the one that cost a build on 2026-09-10 was the maintainer's own
`FixRaidenBoss7.py -s CherryHutao1` window, and killing it would have thrown away what they were
looking at.

**3. A partially-written `.pyd` looks like a real one.** A link that is still running (or was killed
mid-way) leaves a file at the destination — one session found a 2,097,152-byte `core.pyd` next to
the real 9.8MB one. **Verify by size *and* mtime**, not existence.

A build script that ends in `echo BUILD_OK` only after every step, and a wait loop that greps for
`BUILD_OK|NINJA_EXIT=[1-9]|COPY_EXIT=[1-9]|FAILED:`, removes all three. Grep the log for
`error C`/`FAILED` **alongside** the exit codes — a per-target failure does not always change the
overall exit code.

## A `compile_check` over a few `.cpp` files does not cover a template you changed

Spot-compiling the translation units you edited is a fast inner loop, and it misses anything in a
`.tpp` that those TUs do not instantiate. Editing `GIMIFixer.tpp` compiled clean against four
`data/*.cpp` files and then failed in `IniFixBuilder.cpp` and `bindings.cpp`, which instantiate more
of it. Treat a spot-compile as a syntax check; the full `ninja` is the real one.
