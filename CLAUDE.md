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
| Tools | [`AI Agent Help/Tools/CLAUDE.md`](AI%20Agent%20Help/Tools/CLAUDE.md) | touching anything under `Tools/` — the builders, the `CIPipeline`, the script, or the shared `AGRemapUtils` library. **Nothing tests this layer and it rots silently: run the tool before you change it.** One session found three tools that could not run at all, each broken by the API's package moving during the C++ migration. Also covers the `##### Script` keyword sections and the substring trap in them, and where an option goes now that the script no longer contains the API |
| Vertex Group Remaps | [`AI Agent Help/VGRemaps/CLAUDE.md`](AI%20Agent%20Help/VGRemaps/CLAUDE.md) | touching `data/VGRemapData.cpp`, `Data/RemapDrafts/`, `Tools/VGRemapFinder`, or a **"the model is warped / kinked in game"** bug -- where the blend-weight table sits in the maintainer's 8-step remap process, the rule that **every source vertex group must map somewhere** (an unmapped one becomes a *negative* bone index, not nothing), which geometry copy matches the library's versions, and the two recipes: a new character's remap end to end, and diagnosing a deformed model in minutes |

**A COUNTER THAT CAN ONLY EVER BE ZERO READS EXACTLY LIKE A ZERO THAT MEANS SOMETHING
(2026-09-10).** Two of this repo's own summary lines were saying nothing, for weeks, and both
looked like ordinary results. The run reported *copied 0 files from existing downloads* on every
mod --- which reads as "this one had no repeats" and actually meant the download cache had been
unreachable since the strategy builders were de-flyweighted, so one XingqiuBamboo texture was
fetched from github **36 times** in a run. And *fixed 1 Blend.buf files* was a set keyed by path,
so it could not distinguish one file from one file remapped twice --- which a merge does by
construction. **When a number looks right, check that it is capable of being wrong**; both of
these were found by counting the log lines rather than reading the summary. See
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "Verifying".

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
`core/src/data/Ini{Parse,Fix}BuilderData.cpp`, with **one folder per character** under
`core/{include/AGRemapCore,src}/data/Ini{Parse,Fix}Data/<Name>/` (2026-09-08). A file shared by more
than one character -- `GIMICharFixer`, `GIMICharParser`, `DarkDiffuse`, `JeanShading` -- stays at the
top level of those directories instead.

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

**FORTY-TWO characters are real now, in five DIFFERENT shapes, and which one you have decides
almost everything else.** `raiden6_1` remaps onto a boss that **shares the source's geometry**
(hashes kept, originals hidden). Amber/AmberCN, Mona/MonaCN, Rosaria/RosariaCN and
Ningguang/NingguangOrchid remap onto a **different model** -- a CN skin or another outfit (hashes
replaced, originals left alone, indices forward-looked-up). Jean/JeanCN/JeanSea add the third: **two
targets, and one of them draws objects the source does not have**, so Jean's `body` graph is *split*
into JeanSea's `body` + `dress`. **Ganyu/GanyuTwilight** are the fourth, and the pair that stresses
the fix libraries: a `$swapvar`-branching `CommandList`, `moveDrawIndexed`, `ORFix`, the only
characters so far that re-issue **`TexFx`** -- whose placement rule is genuinely different from
`NNFix`/`ORFix`'s -- and, in the Ganyu direction, the only fix that has to **invent** a texture
rather than drop or edit one.

**Keqing/KeqingOpulent and Shenhe/ShenheFrostFlower add the fifth: the MERGE**, where the source
draws objects the target has nowhere to put. Two sources landing on one target collide, so the
fix writes **more than one `.ini` file** and the game overlaps them -- two for Keqing, and three
for ShenheFrostFlower, whose head, body and extra all come through Shenhe's single `body` draw
call. It is the same `objSplits` field as the split, read the other way round.

**The lantern-rite batch (Xiangling/XianglingCheer, HuTao/CherryHuTao, Xingqiu/XingqiuBamboo)
adds no sixth shape but stresses one thing none of the others did: EDITING TEXTURES, and a
merge that has to edit them DIFFERENTLY per source.** `GIMICharFixerConfig` grew three
source-keyed fields for it -- `srcObjRegRemovals`, `srcObjRegRemaps`, and `TexEdit::srcObj` --
because everything else in that config is keyed by the TARGET, which is right for a split (one
source per target) and wrong for a merge (several). It also brought the first `positionEdit`:
XianglingCheer's model sits at a different height, so her `Position.buf` is shifted as it is
copied. All forty-two are verified against the old pure-Python script, and every pair through
Lisa/LisaStudent is **confirmed in game** -- the lantern-rite batch on 2026-09-10, Ayaka/Nilou
on 2026-09-11, and Klee, Barbara and both Lisa directions on 2026-09-12. Diluc, Fischl and
Kaeya landed the same day and are A/B-verified but not yet in game.

**Ayaka/AyakaSpringbloom, Nilou/NilouBreeze and Kirara/KiraraBoots (2026-09-11) added no new
shape either, and every one of them needed the TEMPLATE extended rather than a row transcribed.**
Four config fields came out of it -- register-value predicates (`RegRef`, `RegValChecks`),
per-object download registers, a texture edit that COPIES instead of moving (`TexEdit::toReg`),
and value-gated texture edits -- plus two real bugs in shared code: a section the parser INVENTS
for a missing object carried no `hash` or `match_first_index`, and two texture edits of one
source wrote to one FILE. AyakaSpringbloom -> Ayaka is the second merge, and the one that swaps
head and body.

**A MOD FOLDER NAMED IN A NON-LATIN SCRIPT WORKS END TO END NOW (2026-09-11), AND DID NOT
BEFORE.** A Korean-named AyakaSpringbloom mod had all 7 of its `.ini` files skipped, and then --
once those were fixed -- reported `editted 18 *.dds files and skipped 0` having written **none** of
them. Two separate causes, both worth knowing because the rule they break is already written down:
**(1)** ten `std::ofstream(str)` / `std::filesystem::path(str)` sites that the "all ~96 conversion
sites" sweep of 2026-09-07 missed, two of them in `py/` (see **Architecture**'s path section, which
now carries a re-runnable grep -- check it rather than trusting a past sweep); **(2)**
Compressonator's narrow-`char` C API, which that same section used to call out of reach. It is not:
when the path is not pure ASCII the library never sees it, and `std::filesystem` stages the bytes
through an ASCII scratch file. `TextureFile::save` also stopped DISCARDING its write result, which
is what hid the whole thing. See **Texture Editing**.

**And the console rendering it wrong is a DIFFERENT thing from the data being wrong** -- single-
encoded mojibake (`Ayaka∞òä...`) is correct UTF-8 drawn in CP437 and cosmetic; double-encoded
(`Ayaka├¼ΓÇó...`) is a real active-code-page round trip and a bug. The CLI now sets the console code
page to UTF-8 for the duration of a run and restores it afterwards.

**Lisa/LisaStudent, Klee/KleeBlossomingStarlight and Barbara/BarbaraSummertime (2026-09-12) needed
no template change at all -- and three of the six were WRONG anyway, in a way that passed a clean
A/B.** A config written from the remap's shape alone (`drawnObjs` + `objSplits`) runs, logs success,
generates every section name the old script does, and performs none of the character's texture work:
the tell was that the old script edited two `.dds` files for Klee and ours edited zero. The lesson is
the transcription step, not the shapes: **account for every entry in the character's pure-Python
`IniFixBuilderData.py.txt` row**, match it by function NAME rather than by what sits nearest it in
the file, and do not carry over a field because a neighbouring character has one. Two corollaries
worth knowing before the next batch: `preRegEditOldObj` decides whether a SPLIT's second half
inherits a texture edit (Klee sets it, Jean does not, and they need opposite configs because of it),
and **`--ab`'s section check compares section NAMES, not their contents** -- compare the bodies too.
See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "The SHAPE tells you
`objSplits` and nothing else".

**THE .INI CLASSIFIER READS HASHES NOW, AND UNTIL 2026-09-12 IT NEVER HAD.** A mod whose author
named every section after the BASE character while building on the SKIN's model --- `lisa` for a
LisaStudent model, `xianglingpifu` (皮肤, "skin") for XianglingCheer --- classified as the base
character, and the fix then ran the wrong direction and wrote `HashNotFound` into every hash it
could not reverse-look-up. `IniClassifier` had the machinery all along: `addGIModType` takes a
hash set, `incModTypeCountByHash` weighs a hash at **2** against a section name's **1**, and
`readLine` already skips a `hash =` inside a `Remap`-named section so an already-fixed mod's
target hashes cannot vote. The GI population simply passed `{}`. It now passes the five hash
types that actually IDENTIFY a character --- `ib`, `draw_vb`, `position_vb`, `blend_vb`,
`texcoord_vb`, unique across all 312 of their rows --- and no texture hashes, which are shared
assets (`b0e08915` is filed under **forty** names). Measured over 150 real mod `.ini` files: 141
classify identically, 9 change, and all 9 are corrections (4 of them files that classified as
*nothing* and were being skipped). See **Overview**'s "A live feature with an empty input", and
**Creating Remaps**' "Widening what CLASSIFIES runs fixer rows that have never run" --- which is
what this immediately did.

**Diluc/DilucFlamme, Fischl/FischlHighness and Kaeya/KaeyaSailwind (2026-09-12) brought three
things no earlier character has, all of them in the MERGE and SPLIT machinery.** A merge whose
**head is listed twice** -- ``{"head", {"head", "head"}}`` -- so it appears in BOTH generated
``.ini`` files rather than only the first; Lisa's and Keqing's merges do not, and without it the
second file draws a body with no head. A split into **four** targets, where KaeyaSailwind's
dress becomes Kaeya's ``dress`` AND his ``extra`` -- an index no Kaeya ``.ini`` declares and
that his own parser does not list, because it exists only as the far half of that split. And
**register removals that are not registers**: ``ResourceRef<Obj>Diffuse``,
``ResourceRef<Obj>LightMap`` and ``$CharacterIB``, the 3dmigoto reflection-support keys, which
name the SOURCE's slots and so aim the reflection pass at the wrong textures if carried across.
See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "Three things the merge and
the split can do".

**Placement of the re-issued draw call and of the three external libraries was substantially
reworked on 2026-09-08, and the old script is NOT the reference for it** -- matching its topology
reproduced a real bug that silently disabled a mod's transparency. Read
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "The three external libraries" and
"Where `drawindexed` goes decides whether the mod's own effects work" before touching any of it.

**A NEW REMAP NO LONGER COSTS A REBUILD PER IDEA (2026-09-09).** `CppStrategyOverrides` registers
a parser or fixer at runtime, ahead of the compiled-in row, so a remap is now **prototyped from
Python until it works, then transcribed into the C++ tables and rebuilt once**. For a character
of the standard GIMI shape the prototype is a `GIMICharFixerConfig` handed to
`makeGIMICharFixer` --- the same factory the compiled characters use, so the transcription is
nearly mechanical. Two worked examples live in `Tools/Misc/Prototypes/` (copies of the maintainer's
`Importer/GIMI/Mods/` scripts): `overrideScript.py` (the config route, `--ab` proves it
byte-identical to the compiled fix) and `overrideScript2.py` (hand-built from the individual
edits, for a fix the config cannot express).
See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s opening section, and
**attach a logger or read `RemapService.stats` before believing a prototype did nothing** --- a
fix that raises is recorded in `stats.ini.skipped` and printed nowhere else.

**A character with several targets is several rows in `IniFixBuilderData`, NOT a `MultiModFixer`** --
that table is keyed by `(from mod, to mod)`, which is what made the pure-Python indirection
unnecessary. **Read [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md) and copy whichever
shape matches your character** --- it opens with the order of operations, with where to find a free
specification for the character (several have full Integration Tester goldens), and records the
silent ways a remap can be wrong while every log line still says it worked.
**Everything below about the fix being stubbed still holds for every OTHER character.**

All forty-two characters also carry the **face diffuse register swap** (white shiny cheek spots), which
has no pure-Python equivalent. **The obvious diagnosis is the wrong one and was built and thrown
away once already:** the spots are not an opaque blush mask needing a transparent alpha, they are GI
6.x having swapped which register the shader reads the face diffuse and the face lightmap out of, so
a section still binding its diffuse to `ps-t0` hands it to the lightmap slot. The fix is a two-way
`RegRemap` (`ps-t0` <-> `ps-t1`) over the face graph --- one of the things NNFix does under the
hood. See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "The face diffuse".

**THE FIX IS LIVE FOR FORTY-TWO CHARACTERS (verified end-to-end 2026-09-12; thirty-six of them
in game). Earlier revisions of this
file said every `IniFixer`/`IniParser` was stubbed and that `IniFile::getResources()` comes back
empty --- that is NO LONGER TRUE, and believing it will cost you the best verification tool the repo
has.** Real fixers and parsers exist for **Amber, AmberCN, Ayaka, AyakaSpringbloom, Barbara,
BarbaraSummertime, CherryHuTao, Diluc, DilucFlamme, Fischl, FischlHighness,
Ganyu, GanyuTwilight, HuTao, Jean, JeanCN, JeanSea, Kaeya, KaeyaSailwind, Keqing, KeqingOpulent,
Kirara, KiraraBoots, Klee, KleeBlossomingStarlight, Lisa, LisaStudent,
Mona, MonaCN, Nilou, NilouBreeze, Ningguang, NingguangOrchid, Raiden, Rosaria, RosariaCN, Shenhe,
ShenheFrostFlower, Xiangling, XianglingCheer, Xingqiu, XingqiuBamboo**
(`core/src/data/Ini{Fix,Parse}Data/`), a real run generates remapped sections,
and `fixResources` really does correct `Blend.buf` files and really does write textures. Confirmed by
running the CLI over the in-repo Jean fixture and watching two `.dds` files appear.

Two consequences, both the opposite of what this file used to say:
- **"The fix produces correct output" IS a usable acceptance criterion now** --- for these forty-two.
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

**VERTEX GROUP REMAPS HAVE A TOOL AND A RULE NOW (2026-09-09).** `Tools/VGRemapFinder` proposes
the blend-weight table for a pair of skins from their geometry (dumps, a mod's `.buf` files, or a
raw frame analysis) at 89.6% agreement with the hand-made drafts, scores itself against them
(`benchmark.py`) and against the shipped table (`-C`), and found issue #213 in one run: the
shipped `KeqingOpulent -> Keqing` row had **no entry** for two elbow helper bones, and
`BlendFile::remapIndices` writes an unmapped source group as bone `-index-1` with its weight
kept -- a kink, logged nowhere. Every such gap in `VGRemapData.cpp` was then filled (six
directions), and the rule is now explicit: **every source vertex group maps to something**. When
a model deforms in game, `Tools/Misc/Prototypes/overrideVgRemap.py --dump`'s `unmapped source
groups` line is the first thing to read. See [Vertex Group Remaps](AI%20Agent%20Help/VGRemaps/CLAUDE.md).

**A SKIN CAN BE SEVERAL COMPONENTS, EACH WITH ITS OWN VERTEX GROUP NUMBERING (2026-09-12).**
YelanTranquil is a `Body`, a `Bang` and an `Eye` with separate buffers, and WuWa characters are
built that way throughout. A vertex group is `(component, index)` from here on: the finder
matches across all of a target's components, the drafts carry one column per target component
(the maintainer's Yelan convention), and `VGRemapData.cpp` holds one row per (source component,
target component) in the component slots every older row leaves as `""`. Yelan/YelanTranquil are
`ModTypeId`s with remap rows and **no `GIBuilder` factory**; a component column for `HashData`
is the open fixer-side work. **The per-component split of a mod's buffers is core now**
(`model/buffers/VGComponentSplit`, negative index + fill cut, a port of the tool's
`ComponentSplit.py`) **and it runs as a RESOURCE GROUP**: `VGSplitGroupResource` fixes a mod's
blend / position / texcoord / ib members together, `BufReplace` names and builds each member, and
`ResGroupCollect` collects them -- the ib and the vertex buffers depend on each other (issue #190),
so a `ResRegCollect` per buffer is the naive shape. **The whole Yelan -> YelanTranquil fix runs
through the API from Python on that** (`Tools/Misc/Prototypes/yelanTranquilFix.py`: a runtime
`ModType`, one hand-built fixer per component). Doing so found two bindings that had always passed
their tests: a `RemapBlendResource.fixFunc` the C++ service loop could not call (non-copyable
cast) and a `resourceRemapBlend` type the stats never counted. Everything here is **built on
Linux only -- the Windows `.pyd` needs a rebuild**, and `FixRaidenBoss2/__init__.py` guards the
new names with a `try` until it has one. See
[Vertex Group Remaps](AI%20Agent%20Help/VGRemaps/CLAUDE.md)'s "A character of SEVERAL components"
and its recipe's step 8.

**THAT FIX IS CONFIRMED IN GAME ON FOUR YELAN MODS (2026-09-12), AND EACH ONE FOUND SOMETHING THE
ONE BEFORE HAD HIDDEN.** The china dress uses no jacket bones and paints no fur; the Fontaine
outfit hangs a cape on the jacket chains (crooked, until they anchored symmetrically) and paints
its stockings on the alpha the target shades as skin; the Clorinde port keeps CLORINDE's band
legend (hair on the skin band), and its hair speckled because no written texture carried a mip
chain. The maintainer's answer to "which mod next" was **the identity mod**: the character's own
model as a mod (`Tools/Misc/Prototypes/identityMod.py`), every bone and every band of the real
skin in one folder -- and it read Yelan's true legend (255 = fur, not cloth) off her own textures.
Read [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s **"The Yelan lessons, for
ANY new remap"** before starting a remap, and its **"Porting the Yelan prototype into C++"**
checklist before transcribing one; the diagnostics that found each of these are
`Tools/Misc/Diagnostics/modTally.py` and `boneCentroids.py`. Two things that changed under the
API for it: `TextureFile::save` / `TexEditor` / `TexCreator` take a `mipmaps` flag (default off,
so no compiled character's output moved), and `RegFillMissingMode::BottomCover` fills a section's
LAST part. **Every script the guides mention that lives outside the repo is copied under
`Tools/Misc/`** (its README says what each is).

**AND THE PORT LANDED (2026-09-13): YELAN IS COMPILED, THROUGH A SECOND FIXER TEMPLATE.**
`makeGIMIComponentFixer(config, component)` (`data/IniFixData/GIMIComponentFixer.{h,cpp}`) is to a
skin of several components what `makeGIMICharFixer` is to the classic shape -- one fixer per target
component, the target's components as target-only `ModTypeId`s (`YelanTranquilBody` / `Bang` /
`Eye`, like the boss ids: no factory, never registered), the mod's buffers split once at
construction and collected as one resource group per `.ini` group. Every GI character from Bennett
on is this shape, so the next one is a config in its own `IniFixData/<Name>/` folder plus rows, not
new machinery. The A/B against the prototype is byte-identical on every buffer over three mods and
the identity. Four framework findings came out of it, each written up in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)' "Yelan is COMPILED now": the
reverse lookup's version-bucket rule (why the slot indices are in the config and not in
`IndexData`), the shared parser now filtering hash lookups by its own name, `ResGroupCollect`'s
inert-from-C++ seam (fixed), and a registered mod type with no keyword crashing the classifier
population. Still Linux-built only. The Linux suite run also caught a real bug in an untouched
binding -- `parseIniReplaceVals` walked a reference into a temporary, silent on MSVC, a segfault
under GCC 13 -- fixed; see Architecture's pybind gotchas. **Confirmed in game on 2026-09-13.**
The REVERSE direction, `YelanTranquil -> Yelan`, is open and handed to another agent: Creating
Remaps' "The reverse direction is OPEN" says what exists (the ids, the reverse vertex-group
rows, the hashes) and what does not (a multi-component parser, the merge that is the split's
inverse, an identity mod of the skin) -- start there, not from the prototype. And its "Adding a
`ModTypeId`: every place it enters" is the checklist for the eight places a new type has to be
named before it exists.

**THE SCRIPT NO LONGER CONTAINS THE API (2026-09-10), AND NEITHER DID THREE OTHER TOOLS STILL
WORK.** `script build/`'s `AGRemap.py` used to be the whole pure-Python API flattened into one file
by the `ScriptBuilder`. That is impossible now --- a single `.py` cannot carry a compiled extension
module --- so the script *references* the API instead (a path in a `dev` build, a pypi download in a
`prod` one) and went from **31732 lines to 490**. Its source is its own tool at `Tools/Script`, and
the `ScriptBuilder` topologically compiles *that*. The same session found `ScriptBuilder`,
`APIMirrorBuilder` and the script build's own output path all broken by the API's package having
moved to `src/py/` during the C++ migration, none of which anything reported.
**Read [Tools](AI%20Agent%20Help/Tools/CLAUDE.md) before touching anything under `Tools/`, and run
the tool before you change it.**

**Seven repo-mechanics traps that have each cost a full edit-diagnose-repair cycle, none of them
visible from the code:** (1) nearly every tracked text file is **CRLF** (`core.autocrlf=true`), so an
exact-string patch script must normalise to LF before matching and write CRLF back, or every anchor
reports "found 0"; (2) the Bash tool's heredocs eat backslashes (`\ref` arrives as a carriage
return + `ef`), so write patch scripts with the Write tool and run them by path -- **and `sed -i`
mangles the same things in two more ways**: it rewrites a CRLF file as **LF** (silent whole-file
line-ending churn in your diff) and it eats the doubled backslash in this codebase's RST plurals
(`:cpp:enum:`X`\\s` arrives as `X`s`, which is broken RST). Prefer a Python patch script for any
file with CRLF or doc comments; if you do use `sed -i`, normalise the file back to CRLF afterwards
and check with `git diff --stat` against `git diff --stat --ignore-cr-at-eol` (the two must agree).
**And a swallowed `\r` keeps costing after it is committed**: git's CRLF normalisation refuses to
touch a file containing a LONE carriage return, so that file's working copy is compared raw and
**every diff of it is a whole-file rewrite** -- 419 changed lines where 14 were real, which reads
exactly like the line-ending churn of trap (1) and is not. If `--ignore-cr-at-eol` shrinks a file's
diff to almost nothing, grep it for a carriage return that is not followed by a newline: `TexEdit.h`
carried one inside `\ref resSubType` (a broken Doxygen reference) from an older heredoc until
2026-09-11; (3) the dev Python and the VS install root have both MOVED since much of this
documentation was written, and that pair has now flipped **four** times -- as of 2026-09-09 `py -0p` lists **3.9.3**
(so `py -3` is 3.9 and the built module is `core.cp39-win_amd64.pyd`) and `vcvarsall.bat` lives
under `Program Files\Microsoft Visual Studio\18\Community`, with no
`Program Files (x86)\...\18\BuildTools` existing at all -- the exact reverse of what this line said
two days earlier, which was itself the reverse of the day before. **Read the version off
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
list (`subprocess.run(["git", "checkout", "--", *paths])`) instead of the shell;
(7) **`open(f, "wb").write(open(f, "rb").read().replace(...))` DELETES THE FILE.** Python evaluates
the call's owner before its arguments, so the `"wb"` open truncates `f` to zero bytes and the read
that was supposed to supply the new contents then returns `b""`. It exits 0 and prints whatever you
told it to print. Three headers were emptied this way in one line on 2026-09-11 --- in a *cleanup*
script, normalising line endings, run after the real work was finished and verified. **Read into a
variable first, then open for writing**, and note that an emptied file is not obviously wrong in
`git status` (it reports "modified") or in `git diff --stat` (it reports deletions, which a big
refactor also does): what gives it away is a file whose stat line has **insertions of zero**.
Recovery, if it happens: the working tree is the only copy, so restore the file from the last commit
that had it and replay the session's edits --- every patch script's exact text is in the session
transcript under `~/.claude/projects/<slug>/<session>.jsonl`, and a full build is what proves the
reconstruction complete.

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
**(2)** the committed default is sccache **off** (so a machine without sccache, and the
Linux/wheel paths, still work). This line used to say this machine's `cbuild` had it **on**;
**as of 2026-09-12 it does not** --- `AGREMAP_SCCACHE:BOOL=OFF` with `AGREMAP_PCH:BOOL=ON`, which
is the other side of the mutual exclusion in (1). Read the four options out of
`cbuild/CMakeCache.txt` rather than trusting any of this, including this sentence; the
configuration drifts and the performance advice is worthless against the wrong one. **(3)** `AGREMAP_UNITY_BUILD` is wired up,
measured, and deliberately **off**: on 24 threads it tripled the cost of the common single-file
edit. Don't "fix" it by turning it on. **(4)** any source given per-file `COMPILE_OPTIONS` (today
just `VGRemapData.cpp`, at `/Od`) **must** also carry `SKIP_PRECOMPILE_HEADERS` and
`SKIP_UNITY_BUILD_INCLUSION` --- the flags silently stop applying otherwise, and that one file went
from 14s to 144s the first time this was missed. If a build feels slow, check what `cbuild` was
configured with before optimising anything.

**MOD FIXING ITSELF WAS FIRST RUN ON LINUX ON 2026-09-11, and that is a different claim from the
port building.** The same mod fixed on Windows and under WSL now produces **148 of 148 files
byte-identical** -- `.ini` files, textures, blends, downloads, backups. Getting there needed two
fixes, and the first alone looked like success: a `.ini` says `filename = .\Sub\file.buf`, which
on POSIX is one nonexistent filename rather than a path, so every mod pointing into a subfolder
failed; fixing the READ then had Linux WRITE `./Sub/...` back into a file a Windows game reads.
See **Architecture**'s "A path INSIDE a `.ini` is a Windows path, on every OS", and
**Overview**'s habit 28 for why the acceptance test is a byte comparison against Windows rather
than a clean run.

**And the setup notes were written from ONE Linux box.** A second environment (Ubuntu 22.04)
found six things they do not cover -- the GCC floor is 13 and it is *Z3* that sets it, `pybind11`
is missing from `Tools/APIBuilder/requirements.txt`, `wsl -u root` makes the "needs root" step
unattended, a changed compiler needs a fresh build tree, a Linux build silently modifies three
TRACKED `.so` binaries, and numpy's ABI is not its Python version. All six are at the top of
[Setup](AI%20Agent%20Help/Setup/CLAUDE.md)'s Linux section.

**This repo is cross-platform as of 2026-08-31, and that is newer than most of the documentation
around it.** The API has been built, imported and tested on Linux (WSL2 / Ubuntu 24.04 and 22.04,
GCC 13) as well as Windows. The C++ core and Cython layer turned out to be fully portable — every bug that
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

**AND IF YOU BUILD THEM FROM A LIST YOU TYPED, THE LIST IS THE COVERAGE (2026-09-12).** A
hand-maintained runner covering the suites a session happened to care about ran **24 of 46**
test files for a whole day. In the gap: four assertions in `IniNamingTools_test`, broken that
morning by this session's own `.ini`-paths-are-Windows-paths commit, and three Z3 suites that
had never compiled under it at all because they include a private header from `core/src` that
the compile line did not have on its include path. **Glob `core/tests/*_test.cpp` instead of
listing them**, and put `/I <core>/src` on the line -- with both, 45 of 46 build and pass. See
[Testing](AI%20Agent%20Help/Testing/CLAUDE.md).

**The flip side of that, and the single easiest way to leave a mess behind: `core/tests/*.cpp` are
built by nothing.** Change a core class's shape — make it a template, add a parameter to a `virtual`,
rename a method — and every test file mentioning it stops compiling, with no build, no CI and no
Python test failing to tell you. One sat broken for several sessions this way. Before you call a
`core/` interface change done, `grep -rl <the changed name> core/tests/` and rebuild every hit; see
**Testing**'s note for the full story.
