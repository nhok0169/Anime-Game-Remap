# CLAUDE.md

**Anime Game Remap** (formerly `FixRaidenBoss2`) — a library/CLI that remaps mods installed on
one character onto another character's skin, for GIMI-style mods. This repo is the monorepo for
the script/CLI/API distributions, its docs site, and its test suites.

Detailed operating instructions — how to build, test, document, and extend this project — live
under [`AI Agent Help/`](AI%20Agent%20Help/README.md), split by topic. **Read the file(s) below
relevant to your task before guessing at commands or conventions**; don't rediscover the
build/test/doc pipelines from scratch when they're already written down.

| Topic | File | Read it when you're... |
| --- | --- | --- |
| Overview | [`AI Agent Help/Overview/CLAUDE.md`](AI%20Agent%20Help/Overview/CLAUDE.md) | new to the repo — project purpose, full directory layout, branch/PR norms. **Also opens with "Working a feature or bug request here" — read that before starting any task, whatever the subsystem** |
| Setup | [`AI Agent Help/Setup/CLAUDE.md`](AI%20Agent%20Help/Setup/CLAUDE.md) | bootstrapping the API from a fresh clone, **on Windows or Linux** — prerequisites (which VS components, which Python, the exact `pybind11`/Doxygen versions), submodules, the cold-start `-pb -pi -d` build, the Linux/WSL port and its cross-OS traps, and how to tell a broken setup from the suite's pre-existing failures. **Read this before [Building](AI%20Agent%20Help/Building/CLAUDE.md) if `import FixRaidenBoss2` doesn't work yet** |
| Building | [`AI Agent Help/Building/CLAUDE.md`](AI%20Agent%20Help/Building/CLAUDE.md) | compiling the C++ core, pybind11 bindings, or Cython extensions (assumes Setup is done) -- **or wondering why a rebuild is taking so long**, which has its own "Build speed" section |
| Testing | [`AI Agent Help/Testing/CLAUDE.md`](AI%20Agent%20Help/Testing/CLAUDE.md) | running the unit or integration test suites |
| Documentation | [`AI Agent Help/Documentation/CLAUDE.md`](AI%20Agent%20Help/Documentation/CLAUDE.md) | writing/building Doxygen or Sphinx docs |
| Architecture | [`AI Agent Help/Architecture/CLAUDE.md`](AI%20Agent%20Help/Architecture/CLAUDE.md) | writing new C++ core code or pybind11 bindings |
| Creating Remaps | [`AI Agent Help/CreatingRemaps/CLAUDE.md`](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md) | adding or fixing the remap for **one character** — the `IniParser`/`IniFixer` pair, where the hash/index data comes from, and the A/B-against-the-old-script loop that is the only thing that actually proves a remap works. **Read this before touching `data/IniParseData/` or `data/IniFixData/`** |
| Ini Graph Editing | [`AI Agent Help/IniGraphEditing/CLAUDE.md`](AI%20Agent%20Help/IniGraphEditing/CLAUDE.md) | working on `IniSectionGraph`, `GraphTools`, `CallGraph`, or a `graphEdits/`/`graphGroupEdits/`/`regEdits/` strategy (`RegSurroundedAdd`-style .ini graph edits, `run =` call/cycle handling, dataflow analysis over the graph, or completing a simpler `GraphInherit`-style stub). **`regEdits/` is C++/pybind11 now** — pair this with **Architecture** for anything in that family |
| Texture Editing | [`AI Agent Help/TextureEditing/CLAUDE.md`](AI%20Agent%20Help/TextureEditing/CLAUDE.md) | working on `TextureFile`, `TexEditor`, `TexCreator`, or a `texFilters/`/`pixelTransforms/` strategy (the Compressonator/Pillow dual-engine `.dds` pipeline, the `readPillowImg` buffer-native-vs-`.img` design, or save-format/gamma behavior) — **also read its first section if you just want to *look at* a `.dds`**, which the Read tool cannot open directly |
| Buf Files | [`AI Agent Help/BufFiles/CLAUDE.md`](AI%20Agent%20Help/BufFiles/CLAUDE.md) | working on `BufFile`, `BlendFile`, `PositionFile`, `IbFile`, `VbFile`, the `BufDataType`/`BufElementType` family, `BufTools` or `bufEditors/` — and **mandatory before touching the 3dmigoto dump text format** (`getDumpStr`/`readDumpStr`), where this repo's own notebooks are a reverse-engineering rather than the spec, and the obvious sample folders will validate you in a circle |

**Whatever your task is, read [Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s "Working a
feature or bug request here: the habits that pay" first.** It is nine short habits, none of them
about the domain, all of them about how *this* codebase fails --- and the failure mode it opens with
is the one that has cost the most time by far: **code that runs, logs success, and does nothing.**
"The run was clean" is never evidence here. It also covers the two test trees (grep both, or you
will conclude there is no coverage when there is), when a divergence from the old script is *not*
a bug, and how to prove a refactor changed nothing.

If you're unsure which applies, start with **Overview** — it's the map the rest assume you have.
These files were authored from hands-on, verified work in five subsystems: the C++ core / pybind11
`OrderedMultiMap`/`IfContentPart` layer, the Python-side `.ini` graph model and its
dataflow-analysis-based graph edits (see **Ini Graph Editing**), — separately, later — the
Compressonator-backed C++ port of the texture-editing pipeline (see **Texture Editing**), and,
later still, the full pure-Python-to-C++ replacement of the `iniFixers/regEdits/` family (see
**Architecture** for the porting patterns it produced), then the `ModType` strategy
/ asset layer: its three `Ini*Builder`s and `Ini*BuilderData` tables, its four asset attributes
(`hashes`/`indices`/`vertexCounts`/`vgRemaps`), and the `ModAssets`/`ModDictAssets`/
`ModMappedAssets` lookup family underneath them (see **Architecture**'s last three sections), and
then the `iniRemovers/` family: a from-scratch C++ `RemapIniRemover` (a reachability-based
replacement, not a port), its `IniRemoveContext`/`IniRemovalContext` seams, and the full deletion of
the pure-Python `RemapIniRemover`/`BaseIniRemover`, and — most recently — the whole `RemapService`
layer: the model/UI split into `AGRemapCore::RemapService` + `AGRemapCore::RemapServiceCLI`, the
rewiring of `main.py` onto it, and the deletion of `remapService.py` and `model/Mod.py` — each file
says so where relevant, so treat claims about less-explored subsystems as a starting point to
verify, not gospel.

**`data/IniParseBuilderData.py.txt` and `data/IniFixBuilderData.py.txt` are REFERENCE ONLY --- the
live tables are C++.** They are the pre-migration pure-Python tables, kept so an agent can read what
a character's fix used to do. Editing them changes nothing, and they cannot be wired back up:
`CppIniParseBuilderArgs` is bound opaque ("there is no way to build one from Python yet"), so the
version-keyed tables are unreachable from `ModData.IniParseBuilderArgs`. The real ones are
`core/src/data/Ini{Parse,Fix}BuilderData.cpp`, with one file per character under
`core/{include/AGRemapCore,src}/data/Ini{Parse,Fix}Data/`.

**A "port this pure-Python class to C++" request is a well-trodden path here, not a one-off.**
Several have landed already, and the accumulated conventions are load-bearing — read
**Architecture**'s "Two different outcomes for porting a class to C++/pybind11" (plus the sections
after it on templating for pybind reach, still-pure-Python collaborator types, and how a binding
holds a Python-supplied argument) *before* writing the first header, and **Testing**'s note on
reading the class's existing `test_Xxx.py` as a behavioural contract before designing the binding.

**If your task touches `model/strategies/` at all — a parser, fixer, remover or resource edit —
read Architecture's "The strategy context seam" section first.** It is the one architectural pattern
you cannot work around: a C++ strategy never holds an `AGRemapCore::IniFile*`, it holds a
pure-virtual context interface with **two** implementations (a `Py*` one reached from Python, and an
`IniFileXxx*` one that wraps the C++ `IniFile`). Adding a method to a seam is half-done until both
sides implement it, and only one of those halves is a compile error.

**`IniFile` is the C++ class now — the pure-Python `model/files/IniFile.py` was deleted on
2026-09-03**, so any older note describing a "still-pure-Python `IniFile`" (including inside doc
comments) is stale. `AGRemapCore::IniFile::parse()`/`fix()` are **live** for a plain C++ caller too;
the old "they are inert" warning is likewise obsolete, though its one surviving half still holds:
core deliberately has no section renderer, so don't add an `IfTemplate::toStr` — the renderer is
handed in as a callback (`AGRemapCore::renderIfTemplate`). Architecture's "`IniFile` is the C++
class now" section covers what changed in its constructor and which ~33 methods moved out to the
strategies rather than disappearing.

**The MVC view is C++ now too (2026-09-03): `AGRemapCore::BaseLogger` (abstract, owns all formatting)
and `Logger`, bound as `BaseLogger`/`Logger`; `view/Logger.py` is deleted.** A task that needs to
send output somewhere new (a GUI, a backend server talking to a frontend) subclasses `BaseLogger` and
implements `write`/`read` -- from Python or C++, both are reached through the trampoline. Read
**Architecture**'s "The view is C++ now" before touching it; in particular the Python-facing `Logger`
is deliberately *not* a binding of the core `Logger`, and `Model.print` forwards kwargs by name, so
`py::arg` names must match the old Python parameter names exactly.

**Text handling in the C++ core is grapheme-aware, and the maintainer wants it kept that way
(2026-09-03).** Every file-local `toLowerAscii`/`stripAscii`/`std::isspace`-loop helper that had
accumulated in `model/` was deleted in favour of `AGRemapCore::StringTools` (`strip`/`lstrip`/
`rstrip`/`isSpace`/`toLower`/`firstGraphemes`/`lastGraphemes`/`startsWith`/`endsWith`/
`equalsIgnoreCase`/`endsWithIgnoreCase`/`countGrapheme`, all utf8proc-backed, all per grapheme). If a
new feature needs whitespace, case, or a character index, use those or `GraphemeRange`; byte-wise
`find`/`substr` against ASCII *delimiters* (`[`, `=`, `;`, `\n`) are fine. Indices this library hands
out are grapheme indices, and a byte cursor and a grapheme cursor must be separate variables --- see
**Architecture**'s "Text handling in core is grapheme-aware" section for the full rule set, what was
deliberately left byte-wise, and the hand-built test that covers it.

**Nine characters are real now, in three DIFFERENT shapes, and which one you have decides almost
everything else.** `raiden6_1` remaps onto a boss that **shares the source's geometry** (hashes
kept, originals hidden). Amber/AmberCN, Mona/MonaCN and Rosaria/RosariaCN remap onto a **CN skin** --
a genuinely different model (hashes replaced, originals left alone, indices forward-looked-up).
Jean/JeanCN add the third: **two targets, and one of them draws objects the source does not have**,
so Jean's `body` graph is *split* into JeanSea's `body` + `dress`. All are verified against the old
pure-Python script and in game.

**A character with several targets is several rows in `IniFixBuilderData`, NOT a `MultiModFixer`** --
that table is keyed by `(from mod, to mod)`, which is what made the pure-Python indirection
unnecessary. **Read [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md) and copy whichever
shape matches your character** --- it opens with the order of operations, with where to find a free
specification for the character (several have full Integration Tester goldens), and records the
silent ways a remap can be wrong while every log line still says it worked.
**Everything below about the fix being stubbed still holds for every OTHER character.**

All nine characters also carry the **face diffuse register swap** (white shiny cheek spots), which
has no pure-Python equivalent. **The obvious diagnosis is the wrong one and was built and thrown
away once already:** the spots are not an opaque blush mask needing a transparent alpha, they are GI
6.x having swapped which register the shader reads the face diffuse and the face lightmap out of, so
a section still binding its diffuse to `ps-t0` hands it to the lightmap slot. The fix is a two-way
`RegRemap` (`ps-t0` <-> `ps-t1`) over the face graph --- one of the things NNFix does under the
hood. See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "The face diffuse".

**THE FIX IS LIVE FOR NINE CHARACTERS (verified end-to-end 2026-09-07). Earlier revisions of this
file said every `IniFixer`/`IniParser` was stubbed and that `IniFile::getResources()` comes back
empty --- that is NO LONGER TRUE, and believing it will cost you the best verification tool the repo
has.** Real fixers and parsers exist for **Amber, AmberCN, Jean, JeanCN, Mona, MonaCN, Raiden,
Rosaria, RosariaCN** (`core/src/data/Ini{Fix,Parse}Data/`), a real run generates remapped sections,
and `fixResources` really does correct `Blend.buf` files and really does write textures. Confirmed by
running the CLI over the in-repo Jean fixture and watching two `.dds` files appear.

Two consequences, both the opposite of what this file used to say:
- **"The fix produces correct output" IS a usable acceptance criterion now** --- for these nine.
  Prefer it over any unit test when the change could possibly affect a fix.
- **Characters outside that list still have no fixer**, so a run over one of *those* still writes
  only the credit header. That is the stub, not a bug. Check
  `ls "Anime Game Remap (for all users)/api/src/cpp/core/src/data/IniFixData/"` before concluding
  anything --- the list grows, and this paragraph will go stale the same way the last one did.

Still true: **do not "repair" the Integration Tester's golden trees to match current output.** Those
goldens are pre-migration and the naming has legitimately moved on (the Jean texture golden reads
`JeanSeaBodyRemapTex...`, the C++ fixer writes `...ShadeLightMap...`). Regenerating them is its own
task.

**But DO still run the real entry point over a real mod before calling a change done.** The suites
cannot see the class of bug that matters most here. Confirmed the expensive way (2026-09-05): a
default run **emptied every `.ini` file it touched** --- 31 lines of someone's mod replaced by 9
lines of boilerplate, and with `--deleteBackup` no backup either --- while 10 C++ standalone suites
and 1913 Python tests stayed green. Undo-only passed and fix-only passed; only the two *in sequence*,
which is what every real run does, was broken. See [Testing](AI%20Agent%20Help/Testing/CLAUDE.md)'s
"A green suite does not mean the product works" for the two-minute smoke check, and its **"Real mod
data: what is in the repo and which path each fixture exercises"** for the inventory --- which
fixture to point the CLI at depends on what you changed, and picking the wrong one is why a texture
change can look untested when it is two minutes from being proven. Short version: the Raiden
fixture exercises `.ini` rewriting only, and **`inputs/multiFix/select/Jean` is the one that writes
textures**.

**`remapService.py` and `model/Mod.py` are DELETED (2026-09-05); `main.py` drives
`RemapServiceCLI`.** The live entry path is now `main.py` (argparse) -> the Python `RemapServiceCLI`
(`remapServiceCLI.py`, which subclasses the bound `CppRemapServiceCLI` purely to own the things that
name command-line options: `addTips` and the `ConflictingOptions` check) -> `AGRemapCore::
RemapServiceCLI` (log file, tips hook, the "Types of Mods To Fix" banner, and every string ->
model conversion) -> `AGRemapCore::RemapService` (the model: folder walk, per-`.ini` handling, stats,
summary). Argparse stays out of core on purpose. See [Architecture](AI%20Agent%20Help/Architecture/CLAUDE.md)'s
"The `RemapService` / `RemapServiceCLI` split".

**Seven repo-mechanics traps that have each cost a full edit-diagnose-repair cycle, none of them
visible from the code:** (1) nearly every tracked text file is **CRLF** (`core.autocrlf=true`), so an
exact-string patch script must normalise to LF before matching and write CRLF back, or every anchor
reports "found 0"; (2) the Bash tool's heredocs eat backslashes (`\ref` arrives as a carriage
return + `ef`), so write patch scripts with the Write tool and run them by path -- **and `sed -i`
mangles the same things in two more ways**: it rewrites a CRLF file as **LF** (silent whole-file
line-ending churn in your diff) and it eats the doubled backslash in this codebase's RST plurals
(`:cpp:enum:`X`\\s` arrives as `X`s`, which is broken RST). Prefer a Python patch script for any
file with CRLF or doc comments; if you do use `sed -i`, normalise the file back to CRLF afterwards
and check with `git diff --stat` against `git diff --stat --ignore-cr-at-eol` (the two must agree);
(3) the dev Python and the VS install root have both MOVED since much of this documentation was
written, and that pair has now flipped **three** times -- as of 2026-09-07 `py -0p` lists **3.13**
as default (so `py -3` is 3.13, `core.cp313-win_amd64.pyd`, and `cbuild/CMakeCache.txt` reads
`v3.13.1`) and `vcvarsall.bat` lives under
`Program Files (x86)\Microsoft Visual Studio\18\BuildTools`, with no `Program Files` VS 18 existing
at all -- the exact reverse of what this line said one day earlier. **Read the version off
`cbuild/CMakeCache.txt` and locate `vcvarsall.bat` with a `find` rather than trusting any number or
path written down anywhere, this line included** -- see **Building**'s prerequisites;
(4) a `.bat` launched from the Bash tool as `cmd //c C:\Users\...\build.bat` has its backslashes
stripped, never runs, and still exits 0 -- so the "build" silently leaves the *previous* `.pyd` in
place for your tests. Launch build/test batch files from the **PowerShell** tool with
`cmd /c "<full path>"` instead, and verify by the `.pyd`'s mtime (see **Building**); (5) checking
out `nhok0169` **deletes `api/src/cpp` out from under you** (that branch predates the C++ core) and
strands `development`'s submodules under `api/extern/` as part of ~11k untracked files, so move your
working directory to the repo root before switching and **never `git add -A` there** -- see
**Overview**'s operating norms; (6) **every repo path contains both spaces and parentheses**
(`Anime Game Remap (for all users)`), so an unquoted shell variable holding a path silently
shatters into pieces -- `for f in $(git diff --name-only ...); do git checkout -- $f; done` reports
`error: pathspec 'Anime' did not match any file(s)` and **changes nothing while looking like it
ran**. Quote every expansion (`"$f"`), or do path-list work in a Python script with a real argument
list (`subprocess.run(["git", "checkout", "--", *paths])`) instead of the shell.

**The build is no longer the ten-minute wall this file's older advice was written around
(2026-09-08), and the tuning is already done --- do not re-derive it.** A one-line change to a
`core/src/*.cpp` rebuilds in about **8 seconds**, a change to a widely-included `core/include`
header in about **2 minutes**, and a tree that has to build every object from scratch comes back
from the compiler cache in about **30 seconds**. Getting there was a measured exercise, and the
result is five CMake options documented in [Building](AI%20Agent%20Help/Building/CLAUDE.md)'s
**"Build speed"** section: `AGREMAP_ENABLE_LTO` (off for `python_dev`, on for wheels --- pybind11's
default `/GL`+`-LTCG` was the entire 57s floor for *any* change), `AGREMAP_PCH` / `AGREMAP_PCH_LIB`,
`AGREMAP_SCCACHE`, and `AGREMAP_UNITY_BUILD`. Four things to know before you touch any of it:
**(1)** `AGREMAP_SCCACHE` and the two PCH options are **mutually exclusive** --- sccache refuses to
cache a compilation that uses a precompiled header and says so only in `sccache --show-stats`, so
running both is the worst configuration available; turning sccache on disables them for you.
**(2)** this machine's `cbuild` is configured with **sccache on**, while the committed default is
off (so a machine without sccache, and the Linux/wheel paths, still work) --- if a build here
behaves unlike the committed defaults, that is why. **(3)** `AGREMAP_UNITY_BUILD` is wired up,
measured, and deliberately **off**: on 24 threads it tripled the cost of the common single-file
edit. Don't "fix" it by turning it on. **(4)** any source given per-file `COMPILE_OPTIONS` (today
just `VGRemapData.cpp`, at `/Od`) **must** also carry `SKIP_PRECOMPILE_HEADERS` and
`SKIP_UNITY_BUILD_INCLUSION` --- the flags silently stop applying otherwise, and that one file went
from 14s to 144s the first time this was missed. If a build feels slow, check what `cbuild` was
configured with before optimising anything.

**This repo is cross-platform as of 2026-08-31, and that is newer than most of the documentation
around it.** The API has been built, imported and tested on Linux (WSL2 / Ubuntu 24.04, GCC 13)
as well as Windows. The C++ core and Cython layer turned out to be fully portable — every bug that
port surfaced was in CMake glue, vendored third-party code, or `APIBuilder`, and several were
`if(WIN32)` blocks with no `else()` that fail only on the other OS, sometimes only at runtime. If
your task touches the build system at all, read **Setup**'s Linux section and **Overview**'s
operating norms first: they cover which tool versions are pinned to committed artifacts
(`core.pyi` ↔ pybind11 3.0.4, `core/xml` ↔ Doxygen 1.17.0), the two `APIBuilder` functions that
delete things *before* checking their preconditions, and why a checkout shared between the two
OSes needs `-i` on both sides.

**Not every port has a Python test to read, though — and increasingly it won't.** Work landing in
`AGRemapCore` with no pybind11 binding is unreachable from `Testing/Unit Tester`, so a green Python
suite proves *no regression*, not *new code covered*. **Testing**'s "C++-only work is invisible to
the Python suite" section covers what to write instead, and **Building**'s standalone-test sections
cover how to compile it — including the static-lib link line you'll need the moment a test touches
`IniFile::parse`/`fix`.

**The flip side of that, and the single easiest way to leave a mess behind: `core/tests/*.cpp` are
built by nothing.** Change a core class's shape — make it a template, add a parameter to a `virtual`,
rename a method — and every test file mentioning it stops compiling, with no build, no CI and no
Python test failing to tell you. One sat broken for several sessions this way. Before you call a
`core/` interface change done, `grep -rl <the changed name> core/tests/` and rebuild every hit; see
**Testing**'s note for the full story.
