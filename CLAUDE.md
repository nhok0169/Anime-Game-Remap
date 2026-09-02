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
pure-virtual context interface with **two** implementations (a `Py*` one that forwards to the
still-pure-Python `IniFile`, and an `IniFileXxx*` one that wraps the C++ one). Adding a method to a
seam is half-done until both sides implement it, and only one of those halves is a compile error.
That section also records why `AGRemapCore::IniFile::fix()`/`parse()` are still inert for a
plain-C++ caller (their builders' default factories return do-nothing bases, and core has no section
renderer by deliberate design) — don't rediscover that as a bug, and don't "fix" it by adding an
`IfTemplate::toStr`.

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
