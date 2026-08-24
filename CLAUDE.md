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
| Building | [`AI Agent Help/Building/CLAUDE.md`](AI%20Agent%20Help/Building/CLAUDE.md) | compiling the C++ core, pybind11 bindings, or Cython extensions |
| Testing | [`AI Agent Help/Testing/CLAUDE.md`](AI%20Agent%20Help/Testing/CLAUDE.md) | running the unit or integration test suites |
| Documentation | [`AI Agent Help/Documentation/CLAUDE.md`](AI%20Agent%20Help/Documentation/CLAUDE.md) | writing/building Doxygen or Sphinx docs |
| Architecture | [`AI Agent Help/Architecture/CLAUDE.md`](AI%20Agent%20Help/Architecture/CLAUDE.md) | writing new C++ core code or pybind11 bindings |
| Ini Graph Editing | [`AI Agent Help/IniGraphEditing/CLAUDE.md`](AI%20Agent%20Help/IniGraphEditing/CLAUDE.md) | working on `IniSectionGraph`, `GraphTools`, `CallGraph`, or a `graphEdits/`/`graphGroupEdits/`/`regEdits/` strategy (`RegSurroundedAdd`-style .ini graph edits, `run =` call/cycle handling, dataflow analysis over the graph, or completing a simpler `RegAdd`/`GraphInherit`-style stub) |
| Texture Editing | [`AI Agent Help/TextureEditing/CLAUDE.md`](AI%20Agent%20Help/TextureEditing/CLAUDE.md) | working on `TextureFile`, `TexEditor`, `TexCreator`, or a `texFilters/`/`pixelTransforms/` strategy (the Compressonator/Pillow dual-engine `.dds` pipeline, the `readPillowImg` buffer-native-vs-`.img` design, or save-format/gamma behavior) |

If you're unsure which applies, start with **Overview** — it's the map the rest assume you have.
These files were authored from hands-on, verified work in three subsystems: the C++ core / pybind11
`OrderedMultiMap`/`IfContentPart` layer, the Python-side `.ini` graph model and its
dataflow-analysis-based graph edits (see **Ini Graph Editing**), and — separately, later — the
Compressonator-backed C++ port of the texture-editing pipeline (see **Texture Editing**) — each
file says so where relevant, so treat claims about less-explored subsystems as a starting point to
verify, not gospel.
