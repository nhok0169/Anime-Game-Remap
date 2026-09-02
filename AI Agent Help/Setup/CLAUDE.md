# Setup

Getting from *"a Windows box with git and Python on it"* to a working, importable, locally-built
API. [Building](../Building/CLAUDE.md) assumes this bootstrap has already happened and tells you
how to *drive* the build day-to-day; this file is the one-time setup that comes before it. See
[Overview](../Overview/CLAUDE.md) for the repo layout every path below assumes.

**Everything in this file was executed end-to-end on 2026-08-30**, on a Windows 11 checkout that
had no `cbuild/`, `cext/`, or `cebuild/` at all — i.e. a genuine cold start, including building
`z3` from source. Versions, timings, exit codes and failure modes below are what that run actually
produced. Where this file and [Building](../Building/CLAUDE.md) disagree, this file is the newer
observation; the specific corrections are collected in the last section rather than left for you
to trip over.

Reference machine, for calibrating the timings: Intel Xeon w5-3423 (12C/24T), 31 GB RAM,
Windows 11 Pro for Workstations.

## The end state you're aiming for

A `git status`-clean checkout in which this works:

```bash
py -3 -c "import FixRaidenBoss2 as FRB; print(FRB.OrderedMultiMap)"
```

with `PYTHONPATH` (or `sys.path`) pointing at `Anime Game Remap (for all users)/api/src/py`.

Concretely, that means these landed in `api/src/py/FixRaidenBoss2/`:

| File | From |
| --- | --- |
| `core.cp3XX-win_amd64.pyd` | the pybind11 module wrapping `AGRemapCore` |
| `CyDictTools` / `CyListTools` / `CyHashTools` / `CyAlgo` `.cp3XX-win_amd64.pyd` | the Cython layer |
| `libz3.dll`, `libcurl.dll`, `utf8proc.dll` | runtime deps loaded via `__init__.py`'s `os.add_dll_directory` |

`FixRaidenBoss2/__init__.py` does unconditional `from .core import ...` / `from .CyDictTools
import ...` with no pure-Python fallback, so **until this build succeeds, `import FixRaidenBoss2`
fails outright** — there is no "works, just slower" degraded mode. Nothing in that list is tracked
in git (`*.pyd`/`*.so` in the root `.gitignore`, `*.dll` in `Anime Game Remap (for all
users)/.gitignore`), so this step is unavoidable for every fresh clone.

## Prerequisites

| Tool | Confirmed working | Needed for | Notes |
| --- | --- | --- | --- |
| Git | 2.49.0.windows.1 | clone + submodules | needs `http.sslBackend=schannel` here, see below |
| Python | **3.13.1** (x64) | everything | `requires-python = ">=3.8"`; see "Which Python" below |
| VS Build Tools | 2026 (18.4), MSVC 19.50 | the C++/Cython compile | supplies `cl`, **and** `ninja` + `cmake` |
| CMake | 4.0.2 | configure/build/install | VS's bundled one also works |
| Ninja | 1.12.1 (VS-bundled) | the generator APIBuilder hard-codes | **no separate install needed** |
| Doxygen | **1.17.0** | **only** `main.py -d` | version matters — see below; install route has a trap |

Python packages (into whichever interpreter CMake will pick up):

```bash
py -3 -m pip install "pybind11==3.0.4" cython "numpy>=1.26.4" "pybind11-stubgen>=2.5.5"
```

(the exact `pybind11` pin is deliberate — see the next two sections)

`Tools/APIBuilder/requirements.txt` lists only `pybind11-stubgen` and `numpy` — it is **not** the
full set. `pybind11` and `cython` are equally required (both are consumed by
`api/CMakeLists.txt` / `src/cy/CMakeLists.txt` via `execute_process` against the found
interpreter), they're just declared in `api/pyproject.toml`'s `[build-system].requires` instead,
which nothing in the `main.py` path reads.

### `pybind11` must be 3.x — the `>=2.10` in `pyproject.toml` is stale

Confirmed the hard way by deliberately downgrading and rebuilding: **pybind11 2.13.6 does not
compile this codebase.** The failure is immediate and unambiguous:

```
PyIOrderedMultiMap.h(51): error C2039: 'trampoline_self_life_support': is not a member of 'pybind11'
PyIOrderedMultiMap.cpp(249): error C2039: 'smart_holder': is not a member of 'pybind11'
```

`py::smart_holder` and `py::trampoline_self_life_support` are pybind11 **3.x** APIs (in 2.x they
only ever existed on the separate `smart_holder` branch), and this repo's bindings depend on them
pervasively — see [Architecture](../Architecture/CLAUDE.md)'s notes on holders and trampoline
lifetimes for why. `api/pyproject.toml` still says `pybind11>=2.10`; that bound is wrong and a
literal `pip install "pybind11>=2.10"` can resolve to something that cannot build the project.
Don't treat the `pyproject.toml` bound as authoritative.

### ...and 3.0.4 specifically, if you don't want `core.pyi` churn

Within 3.x the choice still matters, because the stub is rendered from signature strings baked
into `core.pyd` at *compile* time by the pybind11 headers — so a different 3.x patch release
re-renders parts of `core.pyi` with no binding having actually changed. Measured on the same tree,
same everything else:

| pybind11 | `git diff` on `core.pyi` after `main.py -d` |
| --- | --- |
| 2.13.6 | *build fails* (`smart_holder`/`trampoline_self_life_support`) |
| **3.0.4** | **none — byte-identical to the committed stub** |
| 3.1.0 | 24 insertions / 8 deletions, all cosmetic |

The 3.1.0 diff is entirely `typing.SupportsInt | typing.SupportsIndex` collapsing to `int` in
callback parameter types, plus enums gaining `@typing.overload`ed `__eq__`/`__ne__`. It is not a
real API-surface change — but it *looks* like one in review, and it reappears on every `-d` run.
`3.0.4` is therefore the pin worth using until someone deliberately regenerates the committed stub
on a newer pybind11. If you see an unexplained `core.pyi` diff, check `pybind11.__version__`
before concluding you changed the binding surface.

Note the failure mode if you hit this accidentally: `main.py` runs `cleanInstalls()` (which deletes
every `.pyd` under `api/`) **before** it compiles, so a failed build doesn't leave you with the
previous working binaries — it leaves you with none. `import FixRaidenBoss2` breaks until you get a
successful build. Don't interpret that as the failed build having corrupted something.

### Which Python

CMake picks the interpreter itself — `find_package(Python REQUIRED COMPONENTS Interpreter
Development)` — and nothing in APIBuilder passes `-DPython_EXECUTABLE`. With CMake's default
`VERSION` find-strategy it takes the **highest** version it can find, not the first on `PATH`. On a
machine with 3.13/3.11/3.7 installed it selected 3.13.1, producing `core.cp313-win_amd64.pyd`.

That matters because the interpreter CMake picks must be the same one you later run tests with —
`py -3` here — or the `.pyd` won't be importable. If you have several Pythons and want a specific
one, pin it explicitly rather than hoping; there's no APIBuilder flag for it, so either configure
`cbuild/` by hand with `-DPython_EXECUTABLE=...` or make the intended interpreter the highest
version visible.

**Corollary that costs an hour if you miss it: do not invoke a bare `python`.** On Windows that
frequently resolves to the Microsoft-Store/`WindowsApps` shim, which is a *different* interpreter
with a *different* `site-packages` from the one CMake built against. Importing the package through
it fails with

```
ValueError: numpy.dtype size changed, may indicate binary incompatibility.
            Expected 96 from C header, got 88 from PyObject
```

raised from inside `CyDictTools` --- which looks exactly like a broken Cython build, and sends you
rebuilding something that was never broken. It is just the wrong `numpy` (2.x on the shim, 1.26.4
where the build happened). **The interpreter CMake selected is the only one that can import this
package**; read it out of `cbuild/CMakeCache.txt` (grep `_Python_EXECUTABLE` --- note the leading
underscore, it is an `INTERNAL` entry) or off the `-- Found Python:` line a configure prints, and
drive everything --- tests, `pybind11_stubgen`, Sphinx, throwaway probe
scripts --- through that absolute path. `py -3` is fine only when it resolves to that same one.

This also applies to a script that *itself* re-launches Python: a helper run under the shim inherits
its environment, so `subprocess.run([<correct python>, ...])` from inside it can still fail. Run the
helper with the correct interpreter in the first place.

Whichever you choose needs its **development files** present (`include/Python.h` and
`libs/pythonXY.lib`) — `COMPONENTS Development` in `src/cy/CMakeLists.txt` requires the full
development component, not just `Development.Module`. A standard python.org installer ships these;
a Microsoft-Store Python does not.

### Visual Studio is the only large install — and it gives you Ninja and CMake for free

The confirmed-working install is **Visual Studio Build Tools 2026** (the standalone build tools,
no IDE) with exactly these components:

| Component ID | What it provides |
| --- | --- |
| `Microsoft.VisualStudio.Component.VC.Tools.x86.x64` | the MSVC C++ toolset (`cl.exe`) |
| `Microsoft.VisualStudio.Component.VC.CMake.Project` | "C++ CMake tools for Windows" — **the bundled `cmake` + `ninja`** |
| `Microsoft.VisualStudio.Component.Windows11SDK.26100` | the Windows SDK |

```bash
vs_BuildTools.exe --quiet --wait --norestart \
  --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
  --add Microsoft.VisualStudio.Component.VC.CMake.Project \
  --add Microsoft.VisualStudio.Component.Windows11SDK.26100
```

**Don't install Ninja separately.** [Building](../Building/CLAUDE.md) lists "CMake, Ninja" beside
Visual Studio in a way that reads as three independent installs; in practice the second component
above puts both on `PATH` the moment `vcvarsall.bat` runs. Verified directly, in a shell whose only
setup was `vcvarsall.bat x64`:

```
--- ninja ---
C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe
--- cmake ---
C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
--- cl ---
C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe
```

A standalone Ninja (`winget install Ninja-build.Ninja`) is a harmless fallback if the VS CMake
component is missing, but it isn't part of the happy path. If you do install it that way, note
winget drops it in `%LOCALAPPDATA%\Microsoft\WinGet\Links` and only edits the *persisted* `PATH` —
already-running shells (including a long-lived agent session's tool processes) keep the stale one
and will report `ninja NOT FOUND` until they're restarted. Prepend the `Links` directory inside
your build script rather than debugging a "failed" install that actually succeeded.

`vcvarsall.bat` printing `'vswhere.exe' is not recognized as an internal or external command` is
harmless — the environment still initializes correctly (`[vcvarsall.bat] Environment initialized
for: 'x64'` follows it). Ignore it; it is not a broken install.

### Doxygen — needed only for `-d`, and the obvious install route hangs

Only `main.py -d` (the docs/stub pass) needs Doxygen. The plain build has no such dependency, so
skip this entirely if you're not touching docs — but if you *do* pass `-d`, make sure Doxygen is
genuinely resolvable first. Running `-d` without it **deletes 642 tracked files**; see the warning
under "Step 2 — build" below.

**`winget install DimitriVanHeesch.Doxygen` hangs indefinitely in a non-interactive shell.**
Confirmed: it downloads the ~70 MB installer to `%TEMP%\WinGet\DimitriVanHeesch.Doxygen.<ver>\`
and then sits there forever, producing *zero* output — no error, no progress, no visible installer
or `consent.exe` child process — until killed, having installed nothing (no
`C:\Program Files\doxygen`). `--accept-package-agreements --accept-source-agreements
--disable-interactivity` does not help. Running the downloaded installer directly with NSIS-style
`/S /D=<dir>` also fails (exit code 2 — it isn't NSIS). If you kick this off from an agent tool
call, it will burn the whole timeout and then report success when the process is killed, since
that's the *wrapper's* exit code, not winget's.

The exact blocker wasn't isolated — it's some prompt winget can't surface here (elevation for the
installer, or a source agreement: a later `winget list` against the same package failed outright
with `One or more of the source agreements were not agreed to`). Don't spend time diagnosing which;
the portable zip below sidesteps the whole package manager.

Use the portable zip instead — no admin, no installer:

```bash
curl.exe -sSL --fail -o "%TEMP%\doxygen.zip" \
  https://github.com/doxygen/doxygen/releases/download/Release_1_17_0/doxygen-1.17.0.windows.x64.bin.zip
```
then extract to e.g. `%LOCALAPPDATA%\Programs\doxygen` and put that directory on `PATH`. The zip is
a flat `doxygen.exe` + `doxyindexer.exe` + `doxywizard.exe` + `libclang.dll`.

**Take 1.17.0 specifically, not "whatever's current".** Doxygen stamps its own version into every
generated file (`<doxygenindex ... version="1.17.0">`), and the committed `core/xml` was generated
by **1.17.0** — so a mismatched Doxygen rewrites that stamp across all 642 files on top of the
normal churn. Building with 1.14.0 was measurably worse than 1.17.0 on that axis. This is the same
kind of version-coupling as the `pybind11` 3.0.4 / `core.pyi` pairing above: the toolchain version
is effectively part of the committed artifact. (winget currently offers 1.18.0 — another reason to
take the pinned zip rather than the package manager.)

`curl.exe` succeeding here is worth noting on its own: Windows' bundled `curl.exe` uses schannel
and therefore the Windows certificate store, so it transparently trusts this machine's
TLS-inspecting proxy CA. Tools that ship their own CA bundle are the ones that break on that (see
the git note below).

### What you explicitly do *not* need

- **Graphviz / `dot`** — `core/Doxyfile` sets `HAVE_DOT = NO`.
- **Java / PlantUML** — `PLANTUML_JAR_PATH`, `PLANTUML_CFG_FILE` and `MERMAID_PATH` are all empty
  in that Doxyfile. Doxygen still prints a `Running plantuml with JAVA...` line during the `-d`
  run; with no jar configured it's a no-op and the run exits 0 regardless. Don't go install a JDK
  because of that line — [Building](../Building/CLAUDE.md)'s "`-d` additionally shells out to
  Doxygen/plantuml/mermaid" overstates it.
- **A separate Ninja or CMake**, per above.
- **Python 3.9 specifically**, per "Which Python" above.
- **Sphinx and friends** (`Docs/requirements.txt`) — only for rendering the docs *site*, which is
  a separate pipeline; see [Documentation](../Documentation/CLAUDE.md).

## Step 1 — clone, with submodules

```bash
git clone --recurse-submodules https://github.com/nhok0169/Anime-Game-Remap.git
```

All six `api/extern/*` submodules in `.gitmodules` are load-bearing (`z3` 31 MB,
`Compressonator` 49 MB, `curl` 18 MB, `xxHash` 5 MB, `utf8proc` 3 MB, `ordered-map` <1 MB —
~106 MB total). An existing clone that predates them: `git submodule update --init --recursive`.

`extern/uni-algo` is referenced by `api/CMakeLists.txt` as an interface include directory but
nothing `#include`s from it and it isn't in `.gitmodules` — its absence is not a problem.

**On git + TLS-inspecting proxies:** [Building](../Building/CLAUDE.md) records submodule fetches
failing here with `SSL certificate problem: unable to get local issuer certificate`, and prescribes
copying populated `extern/` directories between checkouts to avoid the network entirely. That is
**not** the current state of this machine — `git ls-remote https://github.com/Z3Prover/z3.git`
succeeds, because `http.sslBackend` is set to `schannel`, which validates against the Windows
certificate store (where the interception CA already lives). If you hit the cert error on a
different machine, setting that backend is the cleaner fix than the copy workaround:

```bash
git config --global http.sslBackend schannel
```

The copy trick (`robocopy <other-checkout>/api/extern/X <here>/api/extern/X /E /XD .git`) remains
useful for a fresh `git worktree`, which never populates submodules regardless of network health.

## Step 2 — build

APIBuilder must run with its **own directory as the working directory** (`constants/Paths.py`
resolves `Tools/Utilities` via the relative path `../Utilities/...`), and inside a shell where
`vcvarsall.bat` has already run. Since environment changes don't survive between separate agent
tool calls, put both in one `.bat` and invoke that:

```bat
@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 exit /b 1
set "PATH=%LOCALAPPDATA%\Programs\doxygen;%PATH%"
cd /d "<repo-root>\Tools\APIBuilder"
py -3 main.py -pb -pi -d
echo ===== EXIT CODE: %errorlevel% =====
```

Invoke it by **full Windows path** (`cmd //c "$(cygpath -w "$SP/build.bat")"` from Git Bash) — a
bare filename fails with `is not recognized as an internal or external command` even after `cd`-ing
to its directory, which reads like a missing file rather than a path-resolution problem. Echo the
exit code explicitly: the wrapper's own exit status is not APIBuilder's, and a failed
`main.py` inside a `cmd //c` that still exits 0 is easy to misread as success.

`-pb -pi -d` is the cold-start command. All three flags are one-time-ish:

| Flag | Does | Skips itself when |
| --- | --- | --- |
| `-pb` | configures + builds `z3` from source into `cebuild/z3` | `cebuild/z3` exists |
| `-pi` | `cmake --install`s z3 into `cext/z3` | `cext/z3` exists |
| *(none)* | configures + builds + installs the API into `api/src/py/FixRaidenBoss2/` | never — always runs |
| `-d` | regenerates `core.pyi` (pybind11-stubgen) and `core/xml` (Doxygen) | never |

Because `-pb`/`-pi` no-op once their directories exist, `-pb -pi -d` is safe to leave in a
re-runnable script — it doesn't rebuild z3 every time. Everyday rebuilds are plain `py -3 main.py`
(see [Building](../Building/CLAUDE.md) for the faster `ninja core` shortcut).

### ⚠ `-d` without Doxygen on `PATH` **deletes `core/xml` and does not put it back**

This is the one genuinely destructive failure mode in the whole setup, and it is easy to trigger.
`APIBuilder.buildDocs()` reads:

```python
os.chdir(APICoreFolderPath)
shutil.rmtree(APICoreXMLFolderPath, ignore_errors=True)   # <-- wipes core/xml FIRST
subprocess.run(["doxygen", "Doxyfile"], check=True)       # <-- then needs doxygen to exist
```

The `rmtree` is unconditional and happens *before* the `doxygen` call. So if `doxygen` isn't
resolvable, you get `FileNotFoundError` **after** `core/xml` has already been removed — leaving
**642 tracked files deleted** and nothing to regenerate them. Confirmed hands-on: a `-d` run in a
shell without Doxygen produced exactly that (`git status` showing 642 ` D` entries and no
`core/xml` directory at all).

Two things make this worse than it sounds:

- The compile and install into `api/src/py/FixRaidenBoss2/` have **already succeeded** by this
  point, so the package is fine and importable. The non-zero exit is purely the docs step. Don't
  read it as "the build didn't work" — and equally, don't read the working package as "the run was
  fine".
- `pybind11_stubgen` (the other half of `buildDocs()`) catches and prints its own errors, so
  **Doxygen is the only hard dependency `-d` adds** — and the only one that can destroy tracked
  files.

Recovery is a plain checkout, no rebuild needed:

```bash
git checkout -- "Anime Game Remap (for all users)/api/src/cpp/core/xml"
```

**The most common way to trigger this is a stale `PATH`, not a missing install** — and it is much
stickier than it sounds. Adding Doxygen to the *user* `PATH` never reaches already-running
processes, so a shell (or an agent session's tool host, or an editor's integrated terminal)
started before the install keeps the old environment indefinitely. The install looks perfect from
every angle — the registry value is right, the binary is there, a new terminal finds it — while
`-d` keeps failing and keeps deleting `core/xml` every time it's retried. Observed twice in one
session on the same machine, on a correctly-installed Doxygen.

**So always pre-flight it, in the same shell you're about to build from:**

```bash
doxygen --version
```

If that prints nothing or errors, do **not** run `-d` yet. Fix it with either:

```bash
# no restart needed — works in the current shell
$env:PATH = "$env:LOCALAPPDATA\Programs\doxygen;$env:PATH"
```

...or restart the terminal. To check whether a *fresh* shell would resolve it without opening one
(useful when the failing shell is an agent's tool host you can't restart), read the persisted
values straight out of the environment rather than trusting the inherited `$env:PATH`:

```powershell
$fresh = "$([Environment]::GetEnvironmentVariable('Path','Machine'));$([Environment]::GetEnvironmentVariable('Path','User'))"
$fresh -split ';' | Where-Object { $_ -and (Test-Path (Join-Path $_ 'doxygen.exe')) }
```

Prepending the directory inside the build `.bat` (as the wrapper above does) sidesteps the whole
problem permanently, which is why that wrapper sets `PATH` explicitly instead of trusting the
ambient one. Prefer that over relying on a correctly-configured shell.

If you're not touching docs, just drop the `-d`.

**Run it in the background and tail the log.** Cold-start timings on the reference machine:

| Phase | Targets | Time |
| --- | --- | --- |
| z3 configure + build + install | 888 | ~5 min |
| API (core + pybind11 + Cython) | 593 | ~2 min |
| Doxygen + stubgen (`-d`) | — | ~1 min |

Disk cost afterwards, all at the repo root and all safe to delete:

| Directory | Size | What |
| --- | --- | --- |
| `cbuild/` | **1.9 GB** | the API CMake/Ninja build tree |
| `cebuild/` | 346 MB | the z3 build tree |
| `cext/` | 32 MB | the *installed* z3 (`bin/`, `include/`, `lib/`) |

Budget ~2.3 GB. `cext/z3` is a relocatable `cmake --install` output and can be copied to another
checkout wholesale to skip the 5-minute z3 build; `cbuild/`/`cebuild/` bake in absolute paths and
cannot.

## Step 3 — verify

**Run verification from PowerShell, not the Bash tool.** Importing any freshly-built `.pyd` through
Git Bash fails with `ImportError: DLL load failed while importing core: The parameter is
incorrect`, while the identical command works from native PowerShell. This is an MSYS DLL
search-path quirk, not a broken build — it reproduces on binaries known to be good.

```powershell
$env:PYTHONPATH = "<repo-root>\Anime Game Remap (for all users)\api\src\py"
py -3 -c "import FixRaidenBoss2 as FRB; import FixRaidenBoss2.core as c; print(c.__file__); print(FRB.OrderedMultiMap); from FixRaidenBoss2 import CyDictTools; print('ok')"
```

Then the real check — the unit suite, which exercises the compiled extensions across ~1800 tests
in about 12 seconds:

```bash
cd "Testing/Unit Tester"
py -3 -m pip install -r requirements.txt   # once
py -3 main.py
```

**Do not expect a green bar, and don't treat a red one as a broken setup.** The suite has
substantial pre-existing WIP breakage on `development`; see
[Testing](../Testing/CLAUDE.md)'s "Known-broken/WIP test modules" section. The result from this
cold-start run, for calibration:

```
Ran 1831 tests in 11.921s
FAILED (failures=6, errors=73)
```

That exact count reproduced across two independent builds of the same tree (pybind11 3.1.0 and
3.0.4), which is itself the useful signal: the failures don't move with the toolchain, so they're
repo-side, not setup-side.

What matters is the *shape* of the failures, not the count. These were all in-repo API drift and
mid-port inconsistency — nothing environmental:

| Count | Error | Reading |
| --- | --- | --- |
| 30 | `module 'src.FixRaidenBoss2' has no attribute 'Mod'` / `'IniFile'` / ... | stale import path in old test modules |
| 15 | `__init__(): incompatible constructor arguments ... GIMIParser(iniFile, ..., downloads=...)` invoked with `bufDownloads=` | pure-Python `GIMIObjParserOld` still calling the pre-port keyword on a now-C++ base |
| ~28 | `ModAssets.get() got an unexpected keyword argument 'version'`, `VGRemaps' object has no attribute 'updateRepo'`, missing `IniFile.getResourceName`, ... | ordinary API drift in half-migrated subsystems |

The signature of a genuinely broken *setup*, by contrast, is `ImportError: DLL load failed`,
`ModuleNotFoundError: No module named 'FixRaidenBoss2.core'`, or a mass failure of *every* module
including the C++-backed `test_Cpp*.py` ones. None of those appeared. If you want a stricter
comparison, [Testing](../Testing/CLAUDE.md) records an older snapshot (1637 tests / 0 failures /
37 errors, 2026-08-28) — the counts have grown since from both new modules and new drift, so treat
it as a trend line, not a target.

`Testing/Unit Tester/unitTestResults.txt` is written on every run and is **gitignored** (via
`Testing/Unit Tester/.gitignore`), so it's a local scratch record, not a committed baseline you
can diff against — don't mistake it for one.

## What a build leaves in `git status`

A successful `py -3 main.py -pb -pi -d` from a clean checkout leaves **643 changed paths** (644 if
your pybind11 isn't 3.0.4), and essentially all of it is noise:

| Path | Count | Keep? |
| --- | --- | --- |
| `api/src/cpp/core/xml/*.xml` | 642 | discard, unless you edited a `core/include` doc comment |
| `api/src/py/FixRaidenBoss2/core.pyi` | 0 or 1 | zero on pybind11 3.0.4; otherwise keep only if you changed the binding surface |
| `api/src/py/FixRaidenBoss2/utf8proc_static.lib` | 1 (untracked) | see below |

To get back to clean:

```bash
git checkout -- "Anime Game Remap (for all users)/api/src/cpp/core/xml" \
                "Anime Game Remap (for all users)/api/src/py/FixRaidenBoss2/core.pyi"
git clean -fd  "Anime Game Remap (for all users)/api/src/cpp/core/xml"
```
(the `git clean` matters — Doxygen emits brand-new XML files for brand-new classes, which
`checkout` alone won't remove.)

**You cannot get `core/xml` to come back byte-clean from a rerun, even on the matching Doxygen.**
Measured with 1.17.0 against an unmodified tree: `git status` reports all 642 files modified, but
`git diff` finds real textual changes in only **50 of them (150 lines)** — and those are
`<node id>`/`<label>` blocks inside include-graph sections being emitted in a *different order*,
with identical content. The other ~592 are byte-level (line-ending) differences that `git diff`
normalizes away to nothing. So the churn is a mix of genuine per-run nondeterminism and CRLF
noise, neither of which means anything changed. Discard it; don't try to chase it to zero, and
don't read a 642-file `git status` as evidence your change touched the C++ headers.

**`utf8proc_static.lib` is a genuine small gap in `.gitignore`.** `Anime Game Remap (for all
users)/.gitignore` line 4 ignores `**/py/FixRaidenBoss2/*.dll`, and the root `.gitignore` ignores
`*.pyd`/`*.so` — but nothing covers `*.lib`, so the install step leaves this one file permanently
untracked in the package directory after a build. Worth a one-line `.gitignore` fix if you're
touching that file anyway; until then, expect it in `git status` on every fresh setup and don't
commit it. (Which of `utf8proc_static.lib` / `utf8proc.dll` gets installed varies between
configures of the same tree — both were seen from consecutive runs.)

**A `core.pyi` diff is most often pybind11-patch-version noise, not your change** — see the
pybind11 3.0.4 section above for the measured per-version behaviour and what the cosmetic diff
looks like.

## Corrections to other `AI Agent Help` files

Collected here rather than edited into those files, since they were each written from a different
machine state and their surrounding context is still accurate:

1. **[Building](../Building/CLAUDE.md)'s "Python 3.9 at `py -3` ... the committed `.pyd` is
   `core.cp39-win_amd64.pyd`" is doubly stale.** No `.pyd` is committed at all (as
   [Overview](../Overview/CLAUDE.md) states correctly), so there is nothing to fail to overwrite,
   and the build works fine on 3.13. Its "ask before intentionally changing the pinned dev Python
   version" caveat is still worth honouring as a courtesy, but not because of a tracked binary.
2. **Ninja is not a separate prerequisite** — see the VS component table above.
3. **`-d` does not require PlantUML/mermaid/Java** — see "What you explicitly do not need".
4. **`api/pyproject.toml`'s `pybind11>=2.10` is wrong** — 3.x is required, and the committed
   `core.pyi` corresponds to 3.0.4. This is a real source-file bug, not just a doc inaccuracy;
   `>=3.0` would be the minimum honest bound.
5. **Submodule fetches over HTTPS are not currently broken on this machine** —
   `http.sslBackend=schannel` is configured and `git ls-remote` to GitHub succeeds. Building's
   `robocopy`-from-another-checkout workaround is still the right move for a fresh `git worktree`
   (which never inits submodules), just not for the reason given.

---

# Linux (WSL2 / Ubuntu 24.04) — first port, verified 2026-08-31

Everything above this line was written from Windows and is still accurate there. This section is
the **first time this project has been built on Linux at all** — every prior agent worked on
Windows only. It was done in WSL2 against the *same* `/mnt/c` checkout the Windows build uses, so
the two toolchains' outputs sit side by side.

**Headline: it works, but only after four source fixes.** The C++ itself turned out to be the
portable part — `AGRemapCore`'s 107 translation units and all four Cython modules compiled under
GCC 13.3 / `-std=gnu++23` with **zero** source changes. Every problem was in build glue, vendored
third-party code, or Windows-only assumptions in packaging.

## Environment

| Tool | Version | Note |
| --- | --- | --- |
| Ubuntu | 24.04.1 LTS (WSL2, kernel 5.15) | |
| gcc / g++ | 13.3.0 | handles the C++23 in this codebase fine |
| ninja | 1.11.1 | preinstalled |
| Python | 3.12.3 + `python3-dev` | note: **not** the 3.13 the Windows side uses |
| cmake | 4.4.3 | **pip-installed into a venv**, see below |
| Doxygen | 1.17.0 | tarball into `~/.local/bin`, matching the Windows pin |

Two Ubuntu-specific wrinkles:

- **PEP 668.** Ubuntu 24.04 marks the system Python externally-managed, so `pip install` into it is
  refused. Use a venv (`python3 -m venv ~/agremap-venv`). CMake's `find_package(Python)` prefers an
  active virtualenv, so activating it is enough to steer the build — no `-DPython_EXECUTABLE` needed.
- **`sudo` may want a password**, which an agent can't supply. Everything except one package can be
  installed user-locally without root: `pip install cmake`, and Doxygen from its release tarball.

```bash
python3 -m venv ~/agremap-venv
source ~/agremap-venv/bin/activate
pip install cmake "pybind11==3.0.4" cython "numpy>=1.26.4" "pybind11-stubgen>=2.5.5"
```

Keep that venv **activated** for every build and test command below — `cmake` is installed inside
it, and APIBuilder resolves `cmake`/`doxygen` through `PATH`. See the load-bearing-activation note
under "Build folder naming" for why running `~/agremap-venv/bin/python` directly is not sufficient.

### The venv is optional — a global install also works (verified)

Nothing in the project requires a virtualenv; it is purely a way around PEP 668. A global build was
run end to end with `VIRTUAL_ENV` unset and `/usr/bin/python3`, and produced a working
`IMPORT OK`. Two routes:

```bash
# with root — the cleaner one for cmake
sudo apt-get install -y cmake                 # 3.28.3 on 24.04; project needs >= 3.21
pip install --break-system-packages "pybind11==3.0.4" cython "pybind11-stubgen>=2.5.5"

# without root — everything into ~/.local
python3 -m pip install --user --break-system-packages cmake "pybind11==3.0.4" cython "pybind11-stubgen>=2.5.5"
export PATH="$HOME/.local/bin:$PATH"          # NOT on PATH by default here
```

Three things decide this:

- **`pybind11` must come from pip either way.** Ubuntu's packaged pybind11 is 2.x, which cannot
  compile this codebase at all (the `smart_holder` failure described in the Windows section). This
  is the reason a pure-apt setup is not possible.
- **`numpy` does not need installing.** apt already provides 1.26.4, which exactly satisfies the
  `>=1.26.4` requirement — leave it alone rather than shadowing an apt-managed package.
- **`~/.local/bin` is not on `PATH` by default** in this environment, and pip-installed `cmake`
  lands there. Same `PATH` rule as everywhere else in this file.

### ...but the system Python is where contamination bites, and that is the real argument for the venv

The global build initially failed at 580/592 targets, in the **Cython** step, with:

```
AttributeError: module 'typing' has no attribute '_ClassVar'
```

Cause: the obsolete PyPI **`dataclasses` backport** (version 0.6, for Python 3.6) was installed in
the user site and shadowed the stdlib module; it references `typing._ClassVar`, removed in modern
Python. The venv did not contain it, which is precisely why the venv build had worked.
`pip uninstall dataclasses` fixed it and the global build then completed clean.

Two lessons worth carrying:

- The error names `typing`, not `dataclasses`, and appears during a Cython codegen step — so it
  reads like a Cython or Python-version incompatibility rather than one stray package. If you see
  `typing has no attribute '_ClassVar'` anywhere in this build, check for a `dataclasses` backport
  before anything else.
- The backport was pulled in as a dependency of a completely unrelated project. That is the general
  hazard of the global route: an unrelated package can break this build in a way that looks like a
  bug in this build. If you want the shortest path to a working setup, use the venv; if you want to
  build globally, expect to occasionally debug the shared environment.

Doxygen (only needed for `-d`):

```bash
curl -sSL --fail -o /tmp/doxygen.tar.gz https://github.com/doxygen/doxygen/releases/download/Release_1_17_0/doxygen-1.17.0.linux.bin.tar.gz
```

### The one thing that genuinely needs root

```bash
sudo apt-get install -y libssl-dev
```

`core/CMakeLists.txt` sets `CURL_USE_SCHANNEL` inside `if(WIN32)` and provides **no `else()` branch
selecting a TLS backend**, so on Linux the vendored curl falls through to its OpenSSL default and
configure dies:

```
Could NOT find OpenSSL ... (missing: OPENSSL_CRYPTO_LIBRARY OPENSSL_INCLUDE_DIR)
  extern/curl/CMakeLists.txt:836 (find_package)
```

Ubuntu ships the OpenSSL *runtime* (`libssl.so.3`) but not the headers, so this is a hard stop. It
is the **only** step in the whole Linux setup requiring privileges. `-DCURL_ENABLE_SSL=OFF`
configures and builds fine and is useful for smoke-testing the rest, but it is not a real
configuration — this library downloads over HTTPS.

Once `libssl-dev` is present the configure succeeds and curl reports what you want to see:

```
-- Found OpenSSL: /usr/lib/x86_64-linux-gnu/libcrypto.so (found version "3.0.13")
-- Enabled SSL backends: OpenSSL
-- Protocols: ... http https ...
```

**If you hit the OpenSSL error once, delete `cbuildlin` before retrying** — CMake caches
`OPENSSL_INCLUDE_DIR-NOTFOUND` / `OPENSSL_CRYPTO_LIBRARY-NOTFOUND`, so a reconfigure in the same
build tree fails again for a stale reason even after the package is installed, which reads exactly
like the install not having worked.

## Build folder naming

Per project convention, local Linux build folders take a `lin` suffix (`mac` for macOS, nothing for
Windows), so both OSes can share one checkout:

```bash
source ~/agremap-venv/bin/activate     # REQUIRED -- see below, this is not optional
cd Tools/APIBuilder
python main.py -pb -pi -i -ps lin -pis lin -bs lin
```

giving `cebuildlin` / `cextlin` / `cbuildlin` beside the Windows `cebuild` / `cext` / `cbuild`.
For CI, pass the OS name through the same suffix flags (`-bs ubuntu-latest` gives
`cbuildubuntu-latest`).

### `source .../activate` is load-bearing — running the venv's python by path is not enough

**`~/agremap-venv/bin/python main.py ...` fails**, even though it is the same interpreter:

```
FileNotFoundError: [Errno 2] No such file or directory: 'cmake'
  ... in buildAPI: subprocess.run(["cmake", "-G", "Ninja", ...], check=True)
```

Invoking a venv's interpreter by absolute path does **not** activate the venv — it sets up
`sys.path`, but leaves `VIRTUAL_ENV` unset and never puts `~/agremap-venv/bin` on `PATH`. Verified
directly:

| invocation | `VIRTUAL_ENV` | `shutil.which("cmake")` |
| --- | --- | --- |
| `~/agremap-venv/bin/python -c ...` | `None` | `None` |
| after `source ~/agremap-venv/bin/activate` | `/home/<user>/agremap-venv` | `.../agremap-venv/bin/cmake` |

That matters because when `cmake` comes from `pip install cmake` it lives in the venv's `bin/`, and
**APIBuilder shells out to the bare name `"cmake"`** (likewise `"doxygen"` for `-d`) — so those
resolve through `PATH`, not through whichever Python is running the script. Either activate, or
export the directory yourself:

```bash
export PATH="$HOME/agremap-venv/bin:$PATH"
```

This is the third distinct instance of the same root cause in this file — Ninja after a winget
install, Doxygen for `-d`, and now cmake in the venv. **The general rule: every external tool
APIBuilder invokes must be on `PATH` in the shell that runs it.** Pre-flight it rather than
discovering it mid-build:

```bash
command -v cmake ninja
```

**`-i` is not optional here.** `APIBuilder.cleanInstalls()` deletes *every* `.pyd`/`.so` under
`api/`, and the install directory is shared and unsuffixed — so a default Linux run silently wipes
the Windows binaries (and vice versa). The suffix convention isolates *build* trees, not installs.
Pass `-i`/`--installKeep` whenever the other platform's binaries matter.

(Note the deletion is by exact suffix, so it removes `core.cpython-312-x86_64-linux-gnu.so` but
leaves `libz3.so.4.17` alone — its suffix is `.17`.)

### Never let the Linux install write a symlink into the shared package directory

A Linux build writing symlinks into `api/src/py/FixRaidenBoss2/` **breaks the Windows build**, and
the failure appears on the Windows side with nothing obviously pointing at WSL:

```
OSError: [WinError 1920] The file cannot be accessed by the system:
  '...\api\src\py\FixRaidenBoss2\libcurl.so.4'
  ... in cleanInstalls: if filePath.is_file() and filePath.suffix in targetExtensions
```

A symlink created from WSL on a Windows drive is stored as an `LX_SYMLINK` reparse point that
Windows cannot stat at all — `Get-ChildItem` shows `attrs=Archive, ReparsePoint`, and `os.stat`
raises. `cleanInstalls()` walks the whole `api/` tree with `rglob('*')` and calls `is_file()` on
everything it finds, so one such entry aborts the entire Windows build before a single file is
compiled.

Two independent guards are in place:

- **`py/CMakeLists.txt` installs each dependency as a real file named after its soname**, resolving
  the link with `file(REAL_PATH ...)` and `RENAME`-ing the target, rather than reproducing the
  symlink chain with `FOLLOW_SYMLINK_CHAIN`. The loader only needs *a file* with the soname next to
  the module — this is what auditwheel/delvewheel do — and it installs less (one real file per
  dependency instead of a file plus a link). One gotcha in writing it: CMake rejects `RENAME`
  together with `TYPE SHARED_LIBRARY` (`file INSTALL option RENAME may be used only with FILES or
  PROGRAMS`), so it uses `TYPE FILE` with explicit 755 `FILE_PERMISSIONS`.
- **`cleanInstalls()` tolerates unstattable paths**, skipping anything whose `is_file()` raises
  `OSError` instead of letting it kill the build. Worth keeping even though the first guard should
  prevent the situation, because anything unreadable is by definition not something this function
  needs to delete.

If you are recovering an existing checkout that already has such symlinks, delete them from the
Linux side (`rm -f .../FixRaidenBoss2/libz3.so* libutf8proc.so* libcurl.so*`) and re-run the Linux
install; verify with `Get-ChildItem` that no `ReparsePoint` attribute remains.

Both platforms' artifacts do coexist correctly once installed — Python selects by extension suffix
(`core.cp313-win_amd64.pyd` vs `core.cpython-312-x86_64-linux-gnu.so`).

## The four portability fixes this port required

Listed individually because each is a class of bug likely to recur.

1. **`APIBuilder._setupBuildFolders` nested the suffix instead of concatenating it.**
   `os.path.join(cbuild, "lin")` produced `cbuild/lin`, but `removePrefixedFolder` (the `-b lin/`
   delete path), the `--buildSuffix` help text, and `CommandBuilder`'s own example all assume
   `cbuildlin` — as does the "suffix name cannot contain slashes" restriction. Create and delete
   disagreed with each other. Now concatenates; the empty-suffix (Windows) case is byte-identical
   to before.

2. **Duplicate explicit template instantiation.** `IncIdGenerator<std::uint64_t>` was explicitly
   instantiated in *both* `BaseDFA.tpp` and `BaseTrie.tpp`, and `BaseAhoCorasickDFA.h` pulls in
   both — so any TU including it instantiated twice. MSVC accepts this; GCC rejects it outright
   (`duplicate explicit instantiation ... [-fpermissive]`). Removed from `BaseTrie.tpp`; a TU that
   includes only that file still gets the type by ordinary implicit instantiation. A repo-wide scan
   found no other duplicates.

3. **Vendored Compressonator misses `<cstdint>` under GCC 13.**
   `applications/_plugins/common/{cmp_fileio,utilfuncs}.h` use `uintmax_t`/`uint16_t` without
   including it; GCC 13 no longer provides it transitively. Presents as three distinct errors but is
   one root cause — the `redefinition of 'float HalfToFloat'` is just the same unparsed declaration
   seen twice, and chasing it as a real redefinition is a dead end. Fixed from the *parent* repo
   with `add_compile_options(-include stdint.h)` beside the existing `if(MSVC)` workaround, rather
   than patching the submodule (whose changes this repo can't commit). `stdint.h` rather than
   `<cstdint>` so it stays valid for Compressonator's C sources too.

4. **Runtime `.so` deps were never installed, and no RPATH was set.** `py/CMakeLists.txt` installed
   `libz3`/`utf8proc`/`libcurl` beside the module only under `if (WIN32)`. On Windows the module
   finds them because `FixRaidenBoss2/__init__.py` calls `os.add_dll_directory()`; there is no
   ELF/Mach-O equivalent, and CMake additionally strips the build RPATH on install
   (`Set non-toolchain portion of runtime path to ""`). Result: `ImportError: libz3.so.4.17: cannot
   open shared object file`. Fixed with `INSTALL_RPATH` (`$ORIGIN`, `@loader_path` on Apple) plus an
   install of the deps.

   **Sub-trap worth its own line:** `install(FILES $<TARGET_SONAME_FILE:...>)` looks correct but
   copies the *symlink* without its target, producing dangling links that fail with the exact same
   error as installing nothing at all. A versioned shared library is a chain
   (`libz3.so.4.17` -> `libz3.so.4.17.0.0`); use `file(INSTALL ... FOLLOW_SYMLINK_CHAIN)` so the
   link *and* the real file both land.

   **Second sub-trap: these dependencies do not build as a consistent library type, so the install
   list has to be computed per-target rather than hardcoded.** `utf8proc` flipped STATIC -> SHARED
   between two consecutive reconfigures of the *same* `cbuildlin` tree, with no source or flag
   change in between (`AGRemap: utf8proc is STATIC_LIBRARY; not colocating it` on one run, an
   installed `libutf8proc.so.3` and a matching `NEEDED` entry on the next) — and on Windows the
   install alternates between `utf8proc.dll` and `utf8proc_static.lib` for the same reason. So this
   is not a per-platform or per-tree constant you can pin down once. Naming
   `$<TARGET_SONAME_FILE:utf8proc>` unconditionally makes the **generate** step fail outright
   (`TARGET_SONAME_FILE is allowed only for SHARED libraries`) whenever it lands on the static one —
   note this is a generate-time failure, so it kills the build before a single file compiles, and
   it will look like a regression in whatever you changed last. Loop the candidates, check
   `get_target_property(... TYPE)`, and colocate only `SHARED_LIBRARY` ones; a static dep is
   already linked into `core` and correctly has no `NEEDED` entry at all.

   Also caught here: without this, `libcurl.so.4` silently resolved against the **system**
   `/lib/x86_64-linux-gnu/libcurl.so.4` instead of the vendored build it was linked against — an
   import that "works" while defeating the entire point of vendoring curl.

## Test results — and the `orderedset` trap that distorts them

Figures below are from the **real, SSL-enabled** build (`main.py -pb -pi -i -ps lin -pis lin
-bs lin` with `libssl-dev` installed), verified end to end: `RUNPATH: [$ORIGIN]`, every shared
dependency resolving, `IMPORT OK`. An `-DCURL_ENABLE_SSL=OFF` probe build produced identical test
numbers, so TLS configuration does not affect any of this.

With the fixes above, the Linux suite matches Windows almost exactly:

| | Windows | Linux |
| --- | --- | --- |
| Tests | 1831 | 1831 |
| Errors | 73 | 73 |
| Failures | 6 | **15** |
| Runtime | ~11.9s | ~11.4s |

Identical error count *and* an identical error breakdown — so none of the pre-existing WIP breakage
described in [Testing](../Testing/CLAUDE.md) is platform-dependent.

**Before reading any Linux test run, check that `orderedset` imports.** `GIBuilder.py` fetches it
through the lazy `GlobalPackageManager`, and `orderedset` is a dead PyPI package (Cython-based) that
**cannot be built on Python 3.12** — `pip install orderedset` fails at wheel build. Windows only has
it because it installed successfully there at some earlier point. When it is missing the run does
*not* fail loudly; it silently degrades:

| | `orderedset` missing | resolvable |
| --- | --- | --- |
| Tests collected | 1655 | 1831 |
| Errors | 19 | 73 |
| Runtime | 135.9s | 11.4s |

Mod-type registration aborts, so ~176 parameterised tests are never generated, and the 30 failed
lazy-install attempts each shell out to pip, inflating runtime ~11x. A *smaller* failure count here
means less was tested, not that things are healthier. Note the 11x slowdown is **not** `/mnt/c`
filesystem overhead, which is the obvious wrong guess.

For diagnosis, a venv-local shim (an `orderedset/__init__.py` containing
`from ordered_set import OrderedSet`) restores parity. The real fix belongs in the project: depend
on the maintained `ordered-set`/`ordered_set` rather than the dead `orderedset`.

### The 9 Linux-only failures

All 6 Windows failures also fail on Linux. The 9 extra are deterministic — identical failure sets
across repeated runs, so not `unordered_map` ordering flakiness:

- **8 are hardcoded Windows paths in the tests**, not product bugs: `test_IniResource` (x3),
  `test_IniFixResourceModel` (x3), `test_RemapBlendResource`, `test_RemapTexAddResource` — all
  asserting against literals like `'C:/mods/EiRemap/EiBlend.buf'` or
  `'C:\\mods\\EiRemap\\hello.buf'` while the resolved path is `/mnt/c/...`.
- **1 is not path-related and deserves a real diagnosis:**
  `test_IfTemplateTree.test_nestedAndElifBranches_multiLevelTree`, failing `3 != 1` on node part
  counts. Deterministic per platform but different between platforms, which is the signature of
  iteration-order dependence over an unordered container — but that is a hypothesis, not a
  conclusion. Treat it as the one genuinely open question from this port.

## Other Linux gotchas

- **`.gitignore` doesn't cover versioned sonames.** The install drops `libz3.so.4.17`,
  `libz3.so.4.17.0.0`, `libutf8proc.so.3(.2.3)` and `libcurl.so.4(.8.0)` into the package directory.
  The root `.gitignore` has `*.so`, which does **not** match `*.so.4.17`, and the api-level file
  covers only `*.dll` — so six files sit untracked after every Linux build. Same family as the
  `utf8proc_static.lib` gap noted in the Windows section; a `*.so.*` pattern is needed alongside
  `*.lib`.
- **Quoting through PowerShell -> `wsl` -> `bash` mangles quotes silently.** Inline
  `wsl -e bash -c "...\"...\"..."` produced truncated or empty output repeatedly, and it reads like
  the command legitimately returned nothing rather than like a quoting failure. Write the script to
  a file (with LF endings — `sed -i 's/\r$//'`) and run `wsl -e bash /mnt/c/.../script.sh` instead.
- **Piping a long build through `grep` hides progress**, because the pipe block-buffers: a filtered
  live log can sit empty for many minutes and then reveal that failures were captured all along.
  Redirect full output to a file and grep the file afterwards.
- **Running a single test class prints nothing** on Linux (`python main.py SomeTestClass` produced
  no stdout at all). Re-running the full suite is ~11s once `orderedset` resolves, so diffing full
  runs is the more reliable way to check whether a specific failure is deterministic.
