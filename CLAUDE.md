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
| Overview | [`AI Agent Help/Overview/CLAUDE.md`](AI%20Agent%20Help/Overview/CLAUDE.md) | new to the repo — project purpose, full directory layout, branch/PR norms |
| Setup | [`AI Agent Help/Setup/CLAUDE.md`](AI%20Agent%20Help/Setup/CLAUDE.md) | bootstrapping the API from a fresh clone, **on Windows or Linux** — prerequisites (which VS components, which Python, the exact `pybind11`/Doxygen versions), submodules, the cold-start `-pb -pi -d` build, the Linux/WSL port and its cross-OS traps, and how to tell a broken setup from the suite's pre-existing failures. **Read this before [Building](AI%20Agent%20Help/Building/CLAUDE.md) if `import FixRaidenBoss2` doesn't work yet** |
| Building | [`AI Agent Help/Building/CLAUDE.md`](AI%20Agent%20Help/Building/CLAUDE.md) | compiling the C++ core, pybind11 bindings, or Cython extensions (assumes Setup is done) |
| Testing | [`AI Agent Help/Testing/CLAUDE.md`](AI%20Agent%20Help/Testing/CLAUDE.md) | running the unit or integration test suites |
| Documentation | [`AI Agent Help/Documentation/CLAUDE.md`](AI%20Agent%20Help/Documentation/CLAUDE.md) | writing/building Doxygen or Sphinx docs |
| Architecture | [`AI Agent Help/Architecture/CLAUDE.md`](AI%20Agent%20Help/Architecture/CLAUDE.md) | writing new C++ core code or pybind11 bindings |
| Ini Graph Editing | [`AI Agent Help/IniGraphEditing/CLAUDE.md`](AI%20Agent%20Help/IniGraphEditing/CLAUDE.md) | working on `IniSectionGraph`, `GraphTools`, `CallGraph`, or a `graphEdits/`/`graphGroupEdits/`/`regEdits/` strategy (`RegSurroundedAdd`-style .ini graph edits, `run =` call/cycle handling, dataflow analysis over the graph, or completing a simpler `GraphInherit`-style stub). **`regEdits/` is C++/pybind11 now** — pair this with **Architecture** for anything in that family |
| Texture Editing | [`AI Agent Help/TextureEditing/CLAUDE.md`](AI%20Agent%20Help/TextureEditing/CLAUDE.md) | working on `TextureFile`, `TexEditor`, `TexCreator`, or a `texFilters/`/`pixelTransforms/` strategy (the Compressonator/Pillow dual-engine `.dds` pipeline, the `readPillowImg` buffer-native-vs-`.img` design, or save-format/gamma behavior) |
| Buf Files | [`AI Agent Help/BufFiles/CLAUDE.md`](AI%20Agent%20Help/BufFiles/CLAUDE.md) | working on `BufFile`, `BlendFile`, `PositionFile`, `IbFile`, `VbFile`, the `BufDataType`/`BufElementType` family, `BufTools` or `bufEditors/` — and **mandatory before touching the 3dmigoto dump text format** (`getDumpStr`/`readDumpStr`), where this repo's own notebooks are a reverse-engineering rather than the spec, and the obvious sample folders will validate you in a circle |

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
— most recently — the `iniRemovers/` family: a from-scratch C++ `RemapIniRemover` (a reachability-based
replacement, not a port), its `IniRemoveContext`/`IniRemovalContext` seams, and the full deletion of
the pure-Python `RemapIniRemover`/`BaseIniRemover` — each file says so where relevant, so treat claims
about less-explored subsystems as a starting point to verify, not gospel.

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

**Four repo-mechanics traps that have each cost a full edit-diagnose-repair cycle, none of them
visible from the code:** (1) nearly every tracked text file is **CRLF** (`core.autocrlf=true`), so an
exact-string patch script must normalise to LF before matching and write CRLF back, or every anchor
reports "found 0"; (2) the Bash tool's heredocs eat backslashes (`\ref` arrives as a carriage
return + `ef`), so write patch scripts with the Write tool and run them by path; (3) the dev Python is
**3.13** (`core.cp313-win_amd64.pyd`) and `vcvarsall.bat` lives under `Program Files (x86)\Microsoft
Visual Studio\18\BuildTools` on this machine -- see **Building**'s prerequisites for the exact lines;
(4) a `.bat` launched from the Bash tool as `cmd //c C:\Users\...\build.bat` has its backslashes
stripped, never runs, and still exits 0 -- so the "build" silently leaves the *previous* `.pyd` in
place for your tests. Launch build/test batch files from the **PowerShell** tool with
`cmd /c "<full path>"` instead, and verify by the `.pyd`'s mtime (see **Building**).

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
