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
| CI | [`AI Agent Help/CI/CLAUDE.md`](AI%20Agent%20Help/CI/CLAUDE.md) | touching anything under `.github/workflows`, or a CI run, badge or pull request check behaves oddly -- the map of the eleven workflows, **why renaming a job strands branch protection** (checks are matched by the job-name CHAIN), why the testers need the API's own dependencies installed, which cache works (z3) and which cannot (`cbuild`: checkout resets mtimes), cibuildwheel's copied-not-mounted container, what a "No status" badge means, and how to see the remote when `git fetch` is blocked here. **Run `Tools/Misc/Diagnostics/checkWorkflowWiring.py` before and after any workflow change** |
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

**HOW FAST IS THIS LIBRARY AGAINST THE OLD PURE-PYTHON SCRIPT? MEASURED, AND EVERY ROW IS A WIN NOW
(2026-09-20).** Over 22 of the maintainer's own mods, with `--download Disabled` passed to **both**
(their defaults differ, and without it one side is timed doing network work the other refuses):
**1.62x** faster fixing each mod by its own invocation (99.1s vs 61.3s), **1.57x** fixing the whole
folder in one (85.8s vs 54.5s), **2.52x** on `Ayaka6` (244.2s vs 96.8s), and slightly ahead on
startup too (0.80s vs 0.74s).

**Both rows this library used to LOSE were fixed costs with nothing to do with remapping a mod, and
both went the same day.** Startup was **1.59s**, which made the per-mod row a dead heat (105.4s vs
107.9s): an eager `numpy` import worth 240ms of every run, plus `BaseAhoCorasickDFA::add` rebuilding
the whole automaton per keyword (0.40s to file the library's own 49 mod types). **The DLL sizes this
file used to blame are not it** -- `libz3` (16 MB) loads in 11ms, `core.pyd` in 28ms. And texture
editing was **~2x SLOWER** (20.3s vs 41.7s on the six mods that edit textures; **1.32x faster** now,
24.4s vs 18.5s): a texture edit costs **7.3x** less than it did that morning (a 4096x4096 `BC7`
round trip 2.84s -> 0.35s, beating Pillow's 0.62s) after three byte-identical changes -- a gamma
lookup table, **decoding `BC7` per block across threads** instead of through
`CMP_ConvertMipTexture`, and one linear pass for the gamma instead of 33 million
`getPixel`/`setPixel` calls. The decode alone was **85%** of a texture edit and is 11.8x faster;
`BC1` is deliberately NOT claimed, because the one `BC1` texture in the corpus decoded differently
the two ways (1456 of 67M bytes, off by one) -- found only by decoding all 171 corpus textures both
ways, which is how a format gets qualified here.

See [Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s "Startup: where the 1.59s went" and habit 62
(a stage whose boundary you did not check is a stage you invented --- make the parts sum to the
whole), [Architecture](AI%20Agent%20Help/Architecture/CLAUDE.md)'s "`add` on an Aho-Corasick
automaton is a FULL REBUILD", and
[Texture Editing](AI%20Agent%20Help/TextureEditing/CLAUDE.md)'s "AND THEN THE DECODE WAS ALL OF IT".
The advantage also grows with the
`.ini`: `Ayaka6` (194 `.ini`, ~200 toggles) is **244.2s -> 96.8s**, producing the same 193
`RemapBlend.buf` contents. `--compressTextures` is ours alone and costs 10-14x
(`CherryHutao1` 19.6s -> 278.8s for a 4x smaller file); at default settings **both** write
uncompressed 32-bit `.dds`. Full table, method and caveats in
[Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s "The old pure-Python script vs the C++ API,
measured", and the trap that nearly made the numbers meaningless is its habit 60: **the two sides'
summary counters do not mean the same thing**, so compare hashed artifacts, never the reported
counts.

**Whatever your task is, read [Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s "Working a
feature or bug request here: the habits that pay" first.** It is sixty-three short habits, none of
them about the domain, all of them about how *this* codebase fails --- and the failure mode it opens with
is the one that has cost the most time by far: **code that runs, logs success, and does nothing.**
"The run was clean" is never evidence here. It also covers the two test trees (grep both, or you
will conclude there is no coverage when there is), when a divergence from the old script is *not*
a bug, and how to prove a refactor changed nothing.

**A request for a NEW class may describe one that already exists (habit 53, 2026-09-18).**
"Build a `GraphCompose` edit" turned out to be `GraphInherit` with one pluggable piece added, and
the maintainer, shown that, chose to extend it. Before the first header of any new
`regEdits`/`graphEdits`/`graphGroupEdits` class (or any strategy), grep the family for the
primitive it would call. If something already does the core of it, show it and ask: extend,
subclass or rename.

**A PROTOTYPE IS BUILT FROM THE LIBRARY, NOT BESIDE IT (2026-09-22).** The maintainer's standing
complaint, about every agent: prototypes under `Tools/Misc/Prototypes/` reinvent what the library
already does. `chisaParfaitFix.py` hand-rolled its whole texture half -- re-parsing the `.ini`,
expanding `$swapvar` toggles by string, writing `.dds` files and resource sections as text -- while
only its blend went through `ResRegCollect`, and two shipped bugs (lost toggles, a mod's own `ps-tN`
lines overriding the fix) were ones the section graph would have carried for free. "Otherwise I might
as well make you write the prototype without any library." **Before writing any helper, grep
`core.pyi` for the primitive; when the request names library classes, use them; where the library
truly lacks something, say so in a comment and keep the custom part minimal.** Creating Remaps'
"A prototype is built FROM the library" has the need -> class table.

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

**FORTY-EIGHT characters are real now (Citlali, 2026-09-21; count them with
`ls -d "Anime Game Remap (for all users)/api/src/cpp/core/src/data/IniFixData/*/"` rather than
trusting this number -- the written one has been wrong before), in SIX different shapes, and which
one you have decides almost everything else.** Five of them are below; the sixth is the
multi-component skin -- Yelan/YelanTranquil and Bennett/BennettAdventure -- which has its own
sections further down and its own two templates. `raiden6_1` remaps onto a boss that **shares the source's geometry**
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
copied. All of the classic-shape characters are verified against the old pure-Python script, and
**every GI character is now confirmed in game** -- the lantern-rite batch on 2026-09-10, Ayaka/Nilou
on 2026-09-11, Klee/Barbara/Lisa on 2026-09-12, Diluc, Fischl, Kaeya and Arlecchino on 2026-09-13,
Yelan on 2026-09-13 and Bennett on 2026-09-15.

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
`texcoord_vb`, unique across all 357 of their rows --- and no texture hashes, which are shared
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

**ARLECCHINO IS NOT THE RAIDEN SHAPE, DESPITE REMAPPING ONTO A BOSS (2026-09-13).** The boss in
the name is the whole trap: Raiden's remap KEEPS the source's hashes and hides the originals,
and hers REPLACES them (ib ``e811d2a1`` becomes ``480f1267``, every vertex buffer likewise) while
the indices are IDENTICAL on both sides, so the forward index lookup is a no-op that still has to
happen. That is the Mona/MonaCN shape, so she goes through the template and Raiden stays
hand-written. **Read the hash and index tables before picking a shape off the character's name.**
She also has no 6.1 row -- her 5.7 one serves 6.1, as NilouBreeze's does -- and her pure-Python
parse row registers NO downloads, the only 5.x row in that table setting neither ``bufDownloads``
nor ``objFileDownloads``. The assets were on disk all along; only six object textures were
missing from ``Data/Mod Downloads/GI/Arlecchino/5_4`` and they came out of
GI-Model-Importer-Assets.

**THERE ARE TWO GAME VERSIONS NOW, AND THE OLD SCRIPT'S `--version` IS THE ONE BEING FIXED *TO*
(2026-09-13).** `FixRaidenBoss6.py` has a single `version`, and it drives four selections at once:
the parser row, the fixer row, the version-keyed hashes and the indices. This checkout splits it,
because those are independent -- the ordinary case is a mod written for an old version fixed with
the newest fix, which one option cannot express. `--version` / `-v` is `RemapService::toVersion`
and picks the **fixer** (the pure-Python meaning, kept); `--fromVersion` / `-fv` is
`RemapService::fromVersion` and picks the **parser** and the hashes/indices. So reproducing the old
script at version X means passing **both**. Two corollaries for the historical rows:
`makeGIMICharFixer`'s defaults are 6.1-era, so a pre-6.x row must switch off `swapFaceRegs` (a GI
6.x shader correction), `removeSrcFixCalls` (in pure-Python that strip is a `RegRemove` every 6.1
row carries and no 4.0 row does -- left on, it deletes the modder's own `NNFix` call and puts
nothing back) and the default `NNFix` re-issue; and **a count of `NNFix` lines in the output is not
evidence about the fixer**, because the fix copies section bodies and many mods already call it --
that mistake made the verification oracle look broken for a day when it was working. See
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md).

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

Every character on the classic `makeGIMICharFixer` template also carries the **face diffuse
register swap** (white shiny cheek spots), which has no pure-Python equivalent. The two
multi-component templates do it differently and you cannot copy the field across: the merge names
the target's face register outright (`GIMIMergeFixerConfig::faceReg`) and the component template has
no equivalent at all, because a skin of several components binds its face in its own slot. **The obvious diagnosis is the wrong one and was built and thrown
away once already:** the spots are not an opaque blush mask needing a transparent alpha, they are GI
6.x having swapped which register the shader reads the face diffuse and the face lightmap out of, so
a section still binding its diffuse to `ps-t0` hands it to the lightmap slot. The fix is a two-way
`RegRemap` (`ps-t0` <-> `ps-t1`) over the face graph --- one of the things NNFix does under the
hood. See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "The face diffuse".

**THE FIX IS LIVE FOR FORTY-EIGHT CHARACTERS (verified end-to-end, and every one of them in
game -- Citlali through her prototype, which the compiled fix is A/B-identical to). Earlier revisions of this
file said every `IniFixer`/`IniParser` was stubbed and that `IniFile::getResources()` comes back
empty --- that is NO LONGER TRUE, and believing it will cost you the best verification tool the repo
has.** Real fixers and parsers exist for **Amber, AmberCN, Arlecchino, Ayaka, AyakaSpringbloom,
Barbara, BarbaraSummertime, Bennett, BennettAdventure, CherryHuTao, Citlali, Diluc, DilucFlamme, Fischl,
FischlHighness, Ganyu, GanyuTwilight, HuTao, Jean, JeanCN, JeanSea, Kaeya, KaeyaSailwind, Keqing,
KeqingOpulent, Kirara, KiraraBoots, Klee, KleeBlossomingStarlight, Lisa, LisaStudent,
Mona, MonaCN, Nilou, NilouBreeze, Ningguang, NingguangOrchid, Raiden, Rosaria, RosariaCN, Shenhe,
ShenheFrostFlower, Xiangling, XianglingCheer, Xingqiu, XingqiuBamboo, Yelan, YelanTranquil**
(`core/src/data/Ini{Fix,Parse}Data/`), a real run generates remapped sections,
and `fixResources` really does correct `Blend.buf` files and really does write textures. Confirmed by
running the CLI over the in-repo Jean fixture and watching two `.dds` files appear.

Two consequences, both the opposite of what this file used to say:
- **"The fix produces correct output" IS a usable acceptance criterion now** --- for these forty-eight.
  Prefer it over any unit test when the change could possibly affect a fix.
- **Characters outside that list still have no fixer**, so a run over one of *those* still writes
  only the credit header. That is the stub, not a bug. Check
  `ls "Anime Game Remap (for all users)/api/src/cpp/core/src/data/IniFixData/"` before concluding
  anything --- the list grows, and this paragraph will go stale the same way the last one did.

**THE UNIT TESTER IS GREEN ON BOTH OPERATING SYSTEMS NOW, AND "THE BASELINE HAS 7 ERRORS" IS DEAD
ADVICE (2026-09-17).** Windows: **2288 tests, 0 failures, 0 errors**. Linux: the same **2288**, and
**0 failures too since 2026-09-18** --- the 11 it used to report were called "test-side assumptions",
and TWO of them were product bugs: the pure-Python `IniNamingTools.getFixedFile` wrote `./x` into a
`.ini` path on Linux where the core writes `.\x`, and `IfTemplateNode.children` listed branches in
reverse on Linux (an `unordered_map`). **A failure that happens on only one OS is a finding, not a
baseline** --- see [Testing](AI%20Agent%20Help/Testing/CLAUDE.md)'s "The first CI run". The seven
`baseIniFileTest.py` classes that had run **zero** tests since the pure-Python `IniFile` was deleted
run again, so **any error is yours**, and "a change only those classes cover is unverified" no longer
holds. Two things to know before writing a test that needs an `.ini` file: it is a REAL temporary file
with a runtime `ModType` passed through `overrideModTypes` and the strategy injected via
`CppStrategyOverrides` (assigning `_iniParser` is gone with the pure-Python class), and a
Python-built parser must not be reused across `IniFile.clear()`. See
[Testing](AI%20Agent%20Help/Testing/CLAUDE.md)'s "The `.ini` fixture classes run on the C++ `IniFile`",
which also lists the behaviours whose expectations moved on purpose.

**The Integration Tester works again and its goldens are CURRENT C++ output (2026-09-17)** --- regenerated
on Linux after its fixtures gained real hashes (the C++ parsers match by hash, so the old hash-less
fixtures pinned a fix that did nothing). The pre-migration goldens are in git history. See
[Testing](AI%20Agent%20Help/Testing/CLAUDE.md)'s "Integration Tester" before regenerating them --- it has
the five-step loop and its three tools (`Tools/Misc/Linux/integrationTest.sh`,
`Tools/Misc/Diagnostics/goldenChanges.py`, `Tools/Misc/Docs/genApiExamples.py`). **`Docs/src/apiExamples.rst`
is generated from those goldens**, so an API change that moves an example is a test change plus a
regeneration, never a hand edit of the page (see [Documentation](AI%20Agent%20Help/Documentation/CLAUDE.md)).

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
**Every draft workbook opens with a `Credits` sheet (2026-09-14)**, and a Council member who edits
any sheet of one adds their own row there -- `<Council name>: The <nth> member of The Council`,
linked to the Council README -- *after* joining, never before. `Data/RemapDrafts/README.md` has the
layout; the Council ritual in Overview has it as its last step.

**THE FIRST WUWA REMAP DRAFT EXISTS (2026-09-18): Sanhua <-> SanhuaExorcist, both directions, in
`Data/RemapDrafts/SanhuaRemapDraft.xlsx`.** `Tools/VGRemapFinder` reads WWMI-Assets' format now
(`Metadata.json` + `Component N.fmt/.vb/.ib`, no API needed), and the thing to know before touching
any WuWa geometry is that a WWMI character's several components are **one merged skeleton**: each
component's `vg_map` in `Metadata.json` maps its own bone indices into it, that is the space a WWMI
mod's blend buffer uses, and it is the space the maintainer's hand-made draft was already in (208 rows,
blanks exactly at the 24 merged indices no component reaches). The reverse sheet was reviewed from
three signals per row and is **not checked in game**. **Both characters are REGISTERED in the
library (2026-09-19)**: `WWMIBuilder` (`ModTypeId::Sanhua` / `SanhuaExorcist`, game `WuWa`, no
section keywords -- a WWMI `.ini` classifies by `vb0` hash under `$\WWMIv1`), rows in `HashData`,
`IndexData` (typed `componentN`), `VertexCountData` and `VGRemapData` (both directions), and **four
new `Indices`-shaped asset tables** for what GI never carried -- `IndexCounts`, `VGOffsets`,
`VGCounts`, `ShapeKeyChecksums` -- as `ModType` members with `getIndexCount` and friends, remappable
by the same reverse-then-forward lookup as an index. **Sanhua's parse row and the Sanhua ->
SanhuaExorcist fix row are REAL since 2026-09-19** (`makeWWMIParser` / `makeWWMIFixer`, the
FOURTH fixer template -- see the paragraph after this one); the reverse direction and
SanhuaExorcist's own parser are still stubs. **Download
folders for both exist** (`Data/Mod Downloads/WuWa/<Name>/2_5`: the identity mod's whole-mesh buffers,
every asset texture by hash, the asset's manifests; nothing fetches them until `DownloadTools::urlPath`
takes a game folder). **Registering
it found a latent classifier bug**: a WuWa `.ini` left the shared `IniClassifier`'s DFA parked on its
WuWa state and every GI file classified after it came back as nothing (fixed: each entry point now
resets to `start`). See Creating Remaps' "Adding a `ModTypeId`" for the WuWa differences. **Both IDENTITY mods exist too (2026-09-19)**, built by
`Tools/Misc/Prototypes/wwmiIdentityMod.py` into `WWMI/SanhuaIdentity` and `WWMI/SanhuaExorcistIdentity`
--- and a WWMI mod is ONE mesh with per-component draw ranges, a Blend buffer of 8 bytes a vertex, and
SPARSE shape keys rebuilt from the dumps, none of which a GIMI-shaped reader or writer can handle.
**And the forward remap is PROTOTYPED (`Tools/Misc/Prototypes/sanhuaExorcistFix.py`, 2026-09-19), from
two frame dumps read with `Tools/Misc/Diagnostics/wwmiDrawTable.py`**: shader families decide which
target slot draws which source component (bones do not, under a merged skeleton), the material mask's
skin code is `(255, 77, 0)` by measurement, and textures are bound by register on the target's draws
because WuWa texture hashes drift with both streaming and game version. **It fixes IN PLACE through
the API like the Yelan / Bennett prototypes** (a Python-built parser and fixer on `CppStrategyOverrides`,
`RemapService` over the folder, the fix block in the mod's own `.ini`, undo and backups the API's) --
and the GIMI pipeline took a WWMI `.ini` with three changes: a section line with no `=` (3dmigoto's
`local $var`) used to be DROPPED on parse and now round-trips through both renderers,
`GIMIFixer.appendedSections` is bound, and a versionless reverse lookup resolves `0` through GI's
6.1 bucket rather than WuWa's 2.5 (the classifier is handed `2.5`; the game-scoped fix is open).
**Seen in game once (2026-09-19): head, face, top, skirt and arms right; the bangs wrong (component 0
is the BANGS, drawn on hair-shader passes that differ per skin -- fixed by gating on a LIST of passes)
and the back ribbons and belt tassel curled (chains Exorcist has no counterpart for, landing 5-20 cm
from where their vertices sit -- `--anchor ribbons` pins them to their root, the Yelan lesson 5;
`Tools/Misc/Diagnostics/wwmiBoneTally.py` is what measured it). Then the maintainer's OWN working
hand remap turned out to apply the SAME table without waving: what it has off is the SHAPE-KEY
override, so `--shapeKeys` is opt-in now (Sanhua's 30805 shape-key vertices on a buffer the game
sizes for Exorcist's 27267 is the suspect) and the bangs bind the shared default mask at `ps-t1`.
Then a four-way bisect on that mesh found the real thing: the torso, face and eye draws read a SIXTH
vertex stream (`vb6`) of the game's live shape-key offsets BY VERTEX ID, which WWMI's override never
rebinds, so a mod vertex through those slots takes whatever offset the target's buffer holds at the
same index -- 67% of the arm skin, 11% of the bodice, 1% of the skirt. Every remapped section now
binds `vb6` to a zero-offset buffer the fix writes -- which did NOT clear it; the live lead is the
maintainer's: every clean variant had ONE remapped section per Exorcist draw, so the fix now splits
the extra sections of a draw window into their own `.ini` files, the GIMI merge's shape.** See
[Vertex Group Remaps](AI%20Agent%20Help/VGRemaps/CLAUDE.md)'s "WuWa: Sanhua <-> SanhuaExorcist" ---
including why the broken-build check for a new reader is a group COUNT and not a score, the four facts
of a WWMI mod's anatomy, and that `py -3.11`, not `py -3`, is the Python with `openpyxl` here.

**AND THE WUWA FIX IS COMPILED (2026-09-19): `makeWWMIFixer` / `makeWWMIParser` ARE THE FOURTH
TEMPLATE, for a MULTI-COMPONENT character onto a MULTI-COMPONENT skin -- which every WuWa pair is.**
`data/IniFixData/WWMIFixer.{h,cpp}` and `data/IniParseData/WWMIParser.{h,cpp}`, with Sanhua's own
choices in `IniFixData/Sanhua/SanhuaFixer.cpp` (a `WWMIFixerConfig`: the target's passes per slot,
the plan of source component -> target slot + registers, the texture roles by hash, the name
convention, the invented skin mask) and her texture THUMBPRINTS in `Sanhua/SanhuaThumbprints.cpp`,
generated by `Tools/Misc/Diagnostics/wwmiTextureThumbs.py`. ONE row for the pair, not one per
target component: a WWMI character's components are draw slots of one mesh, not mod types. The
prototype (`Tools/Misc/Prototypes/sanhuaExorcistFix.py`) is the oracle and stays working:
`Tools/Misc/Diagnostics/abWWMI.py <mod>` fixes two scratch copies and diffs them, and on four mods
(the identity, a succubus, the frost mod, the RabbitFX cloak with LOD folders) every remapped
section is identical and every `RemapBlend.buf` byte-identical. The copies are now the prototype's
shape too (`GIMIFixer::appendedSectionsInCopies` / `copyHiddenSectionNames`): the first in-game run
of the compiled cloak mod drew every body part with the FIRST claimant's textures while its copies
referenced the texture lists across files, and the prototype's self-contained copies drew it right.
Four things
the port found, all in shared code, are in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "WUWA IS COMPILED": a builder-table
factory that asks the mod type REGISTRY at table-build time gets nothing (it is empty then -- the
parser had no slot objects and classified every slot section as nothing while the hash-only ones
worked); the group remap (`GraphGroupRemap`) both creates the extra copies AND renames the called
lists before any later edit runs, so an anchor has to be matched under both names; **an undo deletes
every file a resource section inside the fix block names** -- 17 of the cloak mod's 19 textures went,
and `IniKeywords::RemapRef` now marks a section that only REFERENCES one of the mod's files; and
pixel identity needs no download: a 16 x 16 grayscale thumbprint of each game texture in the config
makes the same decision as the full-size correlation on every one of 19 files. **A fifth, from the
first mod that reuses one atlas for two components: a texture FILE plays EVERY role its hashes name**
-- the index took the first matching hash and stopped, the bodice got no texture list, and the
Exorcist's own textures drew on it as an all-red body (prototype and compiled alike, so the A/B was
blind to it; the prototype's per-slot table was not). **And a sixth, which is what the red actually
was: a role the mod ships NO file for is bound to the SOURCE's own game texture, downloaded** -- the
mod's UVs are the source's, and the Exorcist's mask at those UVs shaded cloth as skin. That is the
first WuWa download (`WWMIFixerConfig::fallbackTextures`; `DownloadTools::urlPath` takes a game
folder now). **Not seen in game through the compiled path yet.** Open: WuWa BUFFER downloads (only textures are fetched so far), the
reverse direction, retargeting the shape keys, the skin's LOD hashes.

**A WUWA REMAP TO DEBUG OR A NEW WUWA PAIR: three sections of [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)
carry everything the in-game rounds taught (2026-09-19).** "WuWa triage" maps every report's WORDS
so far to its cause (a hue over body and clothes is the MASK; one part in another's texture is a
role with no file; everything in one part's textures is the copies; wavy is `vb6` and the
one-section-per-draw rule); "choosing test mods by structural axis" is the five Sanhua mods and the
one thing each exercises, with the warning that the maintainer moves them between `WWMI/Mods` and
its parent between turns; "The next WuWa pair" is the config checklist in the order to fill it
(`slotPasses` from a dump, the plan by shader family, roles current AND older, thumbprints, the
measured skin code, `fallbackTextures` with the download folder, labels) and the three loop
mechanics that differ from GI: the launcher lives in `WWMI/Mods`, the prototype's copy beside it is
LF and must be re-synced after every change, and the A/B is per mod before AND after a rebuild.
**The prototype's per-slot table is the instrument** -- `GAME (mod has none)` on a register is the
first line to read on any texture report; the compiled fixer prints nothing by request.

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
**AND SO IS THE REVERSE (2026-09-14): `YelanTranquil -> Yelan` IS THE THIRD FIXER TEMPLATE.**
`makeGIMIMergeFixer` is to a multi-component SOURCE on a one-mesh target what
`makeGIMIComponentFixer` is to the other direction, with `makeGIMIComponentParser` beside it and
`VGComponentMerge` / `VGMergeGroupResource` / `VGMergeGroupResBuilder` mirroring the split trio
in core. Always ONE `.ini` group out. Four findings are written up in Creating Remaps' "The
reverse direction is COMPILED TOO", and the first is the one to read before any of it:
**the remap graph is directional, and a multi-component skin has to be named as a remap SOURCE
under each COMPONENT name** -- `ModMappedAssets::replace` is reverse-then-forward, so without
those edges every hash in the output is `HashNotFound` while the buffers stay byte-perfect.
The other three: **byte-identical buffers say nothing about the `.ini`** (compare it separately,
resolving each resource to the md5 of the file it names), a per-register **download fires
spuriously when the register is not fixed across mods** and then overwrites the modder's own
texture, and a mod **missing a whole component** needs both halves -- `objIdentityKVPs` so the
invented section carries a `hash`, and a configured vertex count because `IniFile::fix` runs
before `fixResources` fetches the download. Creating Remaps' "Adding a `ModTypeId`: every place
it enters" is still the checklist for the eight places a new type has to be named before it
exists.

**A PART'S CONDITION WAS WRONG BEHIND A `run =`, FOR AS LONG AS ANYTHING HAS ASKED (2026-09-16).**
`IniSectionGraph::iterByQuery` reports the predicate each part sits under, and its per-depth counter
was a `std::vector` **indexed by depth** but **grown per node** -- which drift the moment a `run =`
is followed, running the index past the end where `operator[]` is undefined behaviour. An
`if`/`else if` chain behind a call gave every branch after the first the FIRST branch's predicate as
well as its own: `$swapvar == 0 AND $swapvar != 0 AND $swapvar == 1`, unsatisfiable. **An `.ini`
renders from its PARTS, so the output was well formed the whole time** and only a caller asking what
a part's CONDITION was could see it -- 772 files over 18 characters are byte-identical across the
fix, because nothing else was asking. Pinned by `core/tests/IniSectionGraph_RunQuery_test.cpp`, whose
first version was written without the `run =` and without the nested `if` a collect leaves behind,
passed against the broken build, and would have shipped meaning nothing. See
[Ini Graph Editing](AI%20Agent%20Help/IniGraphEditing/CLAUDE.md).

**AND FIXING A MERGED MASTER IS ITS OWN SHAPE (2026-09-16).** Its `TextureOverride` carries nothing
but `hash`, `match_first_index` and `run =`, so a read that looks only at the matched section finds
no registers and no draws -- which left the mod's NORMAL map bound where the target's shader reads
the diffuse (every surface pale and flat) and a merged object's second member never drawn (no lower
body). And a master is twelve mods behind one `.ini`, so an appended draw's count and offset are per
BRANCH: `RegBranchAdd` puts the block inside the branch with the numbers that branch's own index
buffers give, where `RegBottomAdd` can only carry one set for all of them. See
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "FIXING A MERGED MASTER".

**AN UNDO IS ONLY AS COMPLETE AS WHAT A FIX WROTE INSIDE ITS OWN BLOCK (2026-09-16).** A section
appended after a fix's closing line survived every undo and hid BennettAdventure's bangs with no mod
installed; a fixer that gave up wrote the mod's own sections again under their source names, which
an undo then used to delete the AUTHOR's sections of the same name. Both multi-component templates
now write nothing when they give up (`GraphGroupRemove`) and render their extra sections inside the
block -- and the fix went into the writers, not the shared remover. Before debugging a merged mod
that works only on its first variant, **diff the `[KeySwap]` of every `.ini` in the folder**: a
merge writes a second `RemapFix1.ini`, and two files on different `$swapvar` values draw one
variant's head over another's buffers. When a remap is done, it is documented in three places. All of
this is in [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md) ("Undo is only as complete",
"Triage", "Closing out a remap"), and [Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s habits 39-43
carry the operating side: WSL through script files, restoring the maintainer's live mods, the whole
surface a new graph edit ships with, building the docs on Linux, and fixing the writer rather than
the shared reader.

**AND AN UNDO USED TO RECOGNISE A FIX BY THE SUBSTRING `Remap`, WHICH A MOD'S OWN SECTION CAN HOLD
(2026-09-20).** Outside the fix's boilerplate, any section whose name merely CONTAINED `Remap` was
taken for a previous fix's leftover, removed, and the file it named deleted. WWMI's blend remap --
what every Wuthering Waves character past 256 bones carries -- declares
`ResourceBlendRemapVertexVGBuffer` and two more, so an undo deleted three `.buf` files of Chisa's
identity mod **on a mod that had never been fixed**, every fix undoing first. `RemapIniRemover` now
asks for `<modName>Remap` (`IniNamingTools::getRemapName`'s shape) using a new
`IniRemoveContext::modTypeNames()`, defaulted to empty so a hand-built remover keeps the old rule;
inside the boilerplate nothing changed. Two general lessons in it: **a keyword test over names a
third party also writes is a landmine** (the `##### Script` substring trap again, in the remover),
and when the two remover suites went red, the fixtures -- which named their leftovers
`<object>Remap<element>` while their own mod types are `TestMod` and `Amber` -- were what had to
move, not the rule. See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "An undo
recognises a fix by `<modName>Remap`".

**THE SECOND WUWA PAIR IS REGISTERED AND PROTOTYPED: CHISA <-> CHISAPARFAIT (2026-09-20); THE
FORWARD DIRECTION HAS BEEN SEEN IN GAME ONCE AND WAS WRONG, FOR A REASON WORTH KNOWING BEFORE ANY
SOURCE PAST 256 BONES.** A mod of such a character carries three lines per component --
`Resource{BlendBuffer,MergedSkeleton,ExtraMergedSkeleton}Override = ref ...Component<N>` -- that
point the draw at WWMI's blend remap of the SOURCE, and copied into a remapped section they are an
inverse of the whole fix rather than a stale binding: the two shaders are exact inverses of each
other, so the pair feeds the draw the source's OWN merged index against the target's skeleton. The
components with a blend remap collapse into a drape while the ones without render correctly, which
reads in game as "a secondary jello body" under an intact head. The remap data was innocent and
looked guilty for hours; what settled it was skinning the mod's mesh under three skeletons rebuilt
from the frame dumps' `vs-cb4` and comparing the pictures. Both are in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "A SOURCE PAST 256 BONES CARRIES
THREE LINES THAT UNDO THE REMAP", with the per-section check that fails against the broken build.
**The second round left the geometry right and the whole body under a translucent red**, which was
the MATERIAL MASK: two skins of one character can pack it differently, and Chisa marks bare skin
with `R = 255` where the Parfait skin marks it with `R = 0`, so her own mask tells the skin's shader
that 97% of her clothes is flesh. The fix repacks the mask into the target's layout rather than
binding the mod's, and the way to learn a legend is to ask the DIFFUSE under each region how
flesh-coloured it is -- see "TWO SKINS OF ONE CHARACTER CAN PACK THEIR MATERIAL MASK DIFFERENTLY".
**And a THIRD round found what the red actually was: a register the TARGET's shader reads and the
source's does not.** The skin's clothing pass takes an `R8_UNORM` map at `ps-t2` that Chisa's has no
input for, so hers stayed bound and its codes landed at the mod's UVs -- and it reads as a near-black
texture while being a small-integer code per pixel (median 4, max 82), which is why `4` where `0`
belongs is a different material rather than a darker one. The fix binds a flat neutral there. It was
found by BISECTING with a flat colour per register over four in-game rounds, and the two strongest
hypotheses on the way were both wrong; the method is in "A REGISTER THE TARGET'S SHADER READS AND THE
SOURCE'S DOES NOT". **THEN FOUR ROUNDS WENT ON ONE PART -- her hair RIBBON, right in colour and
wrong in surface -- and the first two of them chased a difference that was not there, because the
MEASUREMENT was wrong.** Comparing a part between two screenshots means selecting its pixels, and a
statistic over the wrong region reads exactly like one over the right region: "every reddish pixel
in the left 45%" swallowed the character-portrait card in the corner, and "the largest connected red
component" took the ear, the neck, and on one shot the WEAPON BLADE. Both tables said the base had
three times the remap's highlights, which is the opposite of the truth. Selecting by SATURATION
inside a centroid window, and reporting the part as a ratio to the hair and skin of the SAME shot,
says the base is simply DARKER. See [Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s "A screenshot
statistic is only as good as its mask" and `Tools/Misc/Diagnostics/screenshotPart.py`, whose
`--preview` paints the pixels it used. Under a correct mask the ribbon was three real defects, each
its own section in Creating Remaps: **a pass is a register layout and only the draw that SETS it
says what it is** (of the four draws of that slot one sets the whole set and three inherit, so a
carried-forward slot table reads as four layouts -- and that pass turns out to be the clothing
layout with `ps-t0` and `ps-t2` exchanged, so the config had the diffuse on the detail slot and the
normal map on the MATCAP slot; `Tools/Misc/Diagnostics/wwmiPassLayout.py`); **the mask's GREEN
channel is how shiny a surface is**, not a material id, so the flat cloth code invented for a role
the source lacks says "the mattest cloth on the model"; and **a shader family is a COLOUR GRADE** --
her ribbon is painted by a hair shader and the skin has nowhere to draw it but a cloth one, which
renders the same diffuse at x1.15 where hers renders it at (0.89, 0.69, 0.66), and the only place to
put that back is the texture (`ColourGrades`, the GI side's `DarkDiffuse` idea, written with the
sRGB bit or it undoes itself). **The geometry fix, the mask repack, the neutral binding and all
three ribbon fixes are confirmed in game; the reverse direction is unseen.** What is left on the
ribbon is a measured ceiling rather than a defect: split into a gain and an additive ambient from
two texture points, the cloth shader's ambient floor is sRGB 44-52 per channel and the base's ribbon
renders AT it, so the last 11 of green and 15 of blue are not reachable by any texture edit --
brightness now matches exactly (part/hair 1.56 against 1.56) and saturation is 0.585 against 0.659.
Past that lies routing the component through one of the target's HAIR slots, which merges it into a
hair draw. WWMI-Assets has neither of them, so everything comes from frame dumps
(`Tools/Misc/Prototypes/wwmiExtractDump.py`, which runs WWMI Tools' own extractor outside Blender):
the download folders `Data/Mod Downloads/WuWa/Chisa/2_8` and `ChisaParfait/3_5`, both `ModTypeId`s
with their hash / index / count / vg / shape-key rows, a `VGRemapData` row each way out of
`Data/RemapDrafts/ChisaRemapDraft.xlsx` (the finder's proposal, unreviewed), and
`Tools/Misc/Prototypes/chisaParfaitFix.py`. Four things that pair taught, all in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "The SECOND WuWa pair": pair the
slots by GEOMETRY when the two are skins of one character (IoU 1.00 on three of them settles in
minutes what shader names argue about), a role sits at a DIFFERENT register on the two skins (the
diffuse moves from `ps-t2` to `ps-t3`), a character past 256 bones keeps her ids in
`BlendRemapVertexVG.buf` rather than `Blend.buf` so remapping the latter is a no-op, and such a mod
binds `vb4` twice. **A dump's textures also come out at 512 x 512 unless the game's LOD bias is
Ultra High** -- one setting in its own sqlite settings store, not in `GameUserSettings.ini` -- and a
dump taken with a mod of that character installed describes the MODDED pipeline (see
[Vertex Group Remaps](AI%20Agent%20Help/VGRemaps/CLAUDE.md)).

**AND THE "TWO SKINS PACK THEIR MASK INVERSELY" FINDING ABOVE IS WRONG, WHICH IS WHAT A RED STAIN
ON A KIMONO TURNED OUT TO BE (2026-09-20).** Chisa and ChisaParfait **both** mark bare skin with
`R = 255`. The inverse reading came from a percentage-of-flesh-like-pixels test over the two atlases
(88% against 52%), which her pale cream atlas defeats -- "flesh-like" fires on her clothes as
readily as on her skin -- so the repack was mapping Chisa's skin to the target's cloth code and her
kimono to the target's SKIN code, shading the garment with subsurface scattering. **Rendering the
two masks' R channels beside their diffuses settles it in one glance** (Chisa's is black with one
white patch, exactly where her diffuse is flesh; the skin's is white over a bare torso), and it
outlived four fixes aimed at registers because it was never in a register. Two corollaries: a repack
that rewrites ONE channel leaves the others in the source's packing (her codes carry `B ~126, A 0`
where his carry `B 0, A 255`), and G is kept from the MOD because green is how shiny a surface is.
**And a FLAT mask is not a neutral one** -- Chisa's hair mask is a flat `(255, 0, 126, 0)`, i.e. "all
of this is skin", which shaded the crown of the hair red; a flat texture has no UV dependence, so the
"bind the source's, the UVs are the source's" rule does not apply and the register is better left to
the game. See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md).

**AND A FIFTH ROUND, ON A KIMONO MOD, FOUND TWO MORE -- ONE OF THEM A FIX OF MINE THAT MADE THINGS
WORSE (2026-09-20).** **(1) A REGISTER A DRAW INHERITED MAY BELONG TO A DIFFERENT CHARACTER
ENTIRELY.** `wwmiPassLayout.py` marks an inherited register `~`, and that was read as "what Chisa
binds there" -- but a frame dump holds everything on screen, and the draw that had last written her
upper body's `ps-t5` appears **byte-identically in the other skin's dump too**: an NPC standing in
both scenes. Its ramp was transcribed into the fix as hers and **the kimono came back yellow**, a
fresh worse symptom in place of the red one being fixed. `--against <the other dump>` now marks such
a register `!`, and the rule is that **only a `sets` line is evidence about a character**. The same
question, asked of every candidate, also separates the three shared globals the plan was about to
"fix" (both characters' own draws set them) from the one genuinely per-character pair: the 512 x 25
subsurface lookup, Chisa's `06790f7e` against the skin's redder `6a9ec87e`, which is what put a red
cast on her decollete. **(2) DO NOT PICK A RIGID ANCHOR OFF A BONE'S `vs-cb4` TRANSLATION COLUMN** --
that is a skinning matrix, not a pose, and reading it named a bone that put her fox mask and hairpins
90 units away at her hip. Nor score candidates by whether the part keeps its axis-aligned extents: an
AABB is not rotation invariant, and that test rejected 267 of 272 bones including the right one. Skin
the part with each candidate, keep the ones whose **RMS radius from the centroid** is unchanged, and
take the one the surrounding geometry already moves with --
`Tools/Misc/Diagnostics/wwmiAnchorSearch.py`. Also: `AnchorChains`' key is a **source** bone and the
chain takes whatever IT maps to, so writing the target id there is a silent no-op-shaped error, and
the part's OTHER bones have to be anchored too (27% of that prop's weight sat on a bone outside the
chain, which left it torn between two places). Both are in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md).

**AND AN UNDO USED TO RECOGNISE A FIX BY THE SUBSTRING `Remap`, WHICH A MOD'S OWN SECTION CAN HOLD
(2026-09-20).** Outside the fix's boilerplate, any section whose name merely CONTAINED `Remap` was
taken for a previous fix's leftover, removed, and the file it named deleted. WWMI's blend remap --
what every Wuthering Waves character past 256 bones carries -- declares
`ResourceBlendRemapVertexVGBuffer` and two more, so an undo deleted three `.buf` files of Chisa's
identity mod **on a mod that had never been fixed**, every fix undoing first. `RemapIniRemover` now
asks for `<modName>Remap` (`IniNamingTools::getRemapName`'s shape) using a new
`IniRemoveContext::modTypeNames()`, defaulted to empty so a hand-built remover keeps the old rule;
inside the boilerplate nothing changed. Two general lessons in it: **a keyword test over names a
third party also writes is a landmine** (the `##### Script` substring trap again, in the remover),
and when the two remover suites went red, the fixtures -- which named their leftovers
`<object>Remap<element>` while their own mod types are `TestMod` and `Amber` -- were what had to
move, not the rule. See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "An undo
recognises a fix by `<modName>Remap`".

**THE SECOND WUWA PAIR IS REGISTERED AND PROTOTYPED: CHISA <-> CHISAPARFAIT (2026-09-20); THE
FORWARD DIRECTION HAS BEEN SEEN IN GAME ONCE AND WAS WRONG, FOR A REASON WORTH KNOWING BEFORE ANY
SOURCE PAST 256 BONES.** A mod of such a character carries three lines per component --
`Resource{BlendBuffer,MergedSkeleton,ExtraMergedSkeleton}Override = ref ...Component<N>` -- that
point the draw at WWMI's blend remap of the SOURCE, and copied into a remapped section they are an
inverse of the whole fix rather than a stale binding: the two shaders are exact inverses of each
other, so the pair feeds the draw the source's OWN merged index against the target's skeleton. The
components with a blend remap collapse into a drape while the ones without render correctly, which
reads in game as "a secondary jello body" under an intact head. The remap data was innocent and
looked guilty for hours; what settled it was skinning the mod's mesh under three skeletons rebuilt
from the frame dumps' `vs-cb4` and comparing the pictures. Both are in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "A SOURCE PAST 256 BONES CARRIES
THREE LINES THAT UNDO THE REMAP", with the per-section check that fails against the broken build.
**The second round left the geometry right and the whole body under a translucent red**, which was
the MATERIAL MASK: two skins of one character can pack it differently, and Chisa marks bare skin
with `R = 255` where the Parfait skin marks it with `R = 0`, so her own mask tells the skin's shader
that 97% of her clothes is flesh. The fix repacks the mask into the target's layout rather than
binding the mod's, and the way to learn a legend is to ask the DIFFUSE under each region how
flesh-coloured it is -- see "TWO SKINS OF ONE CHARACTER CAN PACK THEIR MATERIAL MASK DIFFERENTLY".
**And a THIRD round found what the red actually was: a register the TARGET's shader reads and the
source's does not.** The skin's clothing pass takes an `R8_UNORM` map at `ps-t2` that Chisa's has no
input for, so hers stayed bound and its codes landed at the mod's UVs -- and it reads as a near-black
texture while being a small-integer code per pixel (median 4, max 82), which is why `4` where `0`
belongs is a different material rather than a darker one. The fix binds a flat neutral there. It was
found by BISECTING with a flat colour per register over four in-game rounds, and the two strongest
hypotheses on the way were both wrong; the method is in "A REGISTER THE TARGET'S SHADER READS AND THE
SOURCE'S DOES NOT". **THEN FOUR ROUNDS WENT ON ONE PART -- her hair RIBBON, right in colour and
wrong in surface -- and the first two of them chased a difference that was not there, because the
MEASUREMENT was wrong.** Comparing a part between two screenshots means selecting its pixels, and a
statistic over the wrong region reads exactly like one over the right region: "every reddish pixel
in the left 45%" swallowed the character-portrait card in the corner, and "the largest connected red
component" took the ear, the neck, and on one shot the WEAPON BLADE. Both tables said the base had
three times the remap's highlights, which is the opposite of the truth. Selecting by SATURATION
inside a centroid window, and reporting the part as a ratio to the hair and skin of the SAME shot,
says the base is simply DARKER. See [Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s "A screenshot
statistic is only as good as its mask" and `Tools/Misc/Diagnostics/screenshotPart.py`, whose
`--preview` paints the pixels it used. Under a correct mask the ribbon was three real defects, each
its own section in Creating Remaps: **a pass is a register layout and only the draw that SETS it
says what it is** (of the four draws of that slot one sets the whole set and three inherit, so a
carried-forward slot table reads as four layouts -- and that pass turns out to be the clothing
layout with `ps-t0` and `ps-t2` exchanged, so the config had the diffuse on the detail slot and the
normal map on the MATCAP slot; `Tools/Misc/Diagnostics/wwmiPassLayout.py`); **the mask's GREEN
channel is how shiny a surface is**, not a material id, so the flat cloth code invented for a role
the source lacks says "the mattest cloth on the model"; and **a shader family is a COLOUR GRADE** --
her ribbon is painted by a hair shader and the skin has nowhere to draw it but a cloth one, which
renders the same diffuse at x1.15 where hers renders it at (0.89, 0.69, 0.66), and the only place to
put that back is the texture (`ColourGrades`, the GI side's `DarkDiffuse` idea, written with the
sRGB bit or it undoes itself). **The geometry fix, the mask repack, the neutral binding and all
three ribbon fixes are confirmed in game; the reverse direction is unseen.** What is left on the
ribbon is a measured ceiling rather than a defect: split into a gain and an additive ambient from
two texture points, the cloth shader's ambient floor is sRGB 44-52 per channel and the base's ribbon
renders AT it, so the last 11 of green and 15 of blue are not reachable by any texture edit --
brightness now matches exactly (part/hair 1.56 against 1.56) and saturation is 0.585 against 0.659.
Past that lies routing the component through one of the target's HAIR slots, which merges it into a
hair draw. WWMI-Assets has neither of them, so everything comes from frame dumps
(`Tools/Misc/Prototypes/wwmiExtractDump.py`, which runs WWMI Tools' own extractor outside Blender):
the download folders `Data/Mod Downloads/WuWa/Chisa/2_8` and `ChisaParfait/3_5`, both `ModTypeId`s
with their hash / index / count / vg / shape-key rows, a `VGRemapData` row each way out of
`Data/RemapDrafts/ChisaRemapDraft.xlsx` (the finder's proposal, unreviewed), and
`Tools/Misc/Prototypes/chisaParfaitFix.py`. Four things that pair taught, all in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "The SECOND WuWa pair": pair the
slots by GEOMETRY when the two are skins of one character (IoU 1.00 on three of them settles in
minutes what shader names argue about), a role sits at a DIFFERENT register on the two skins (the
diffuse moves from `ps-t2` to `ps-t3`), a character past 256 bones keeps her ids in
`BlendRemapVertexVG.buf` rather than `Blend.buf` so remapping the latter is a no-op, and such a mod
binds `vb4` twice. **A dump's textures also come out at 512 x 512 unless the game's LOD bias is
Ultra High** -- one setting in its own sqlite settings store, not in `GameUserSettings.ini` -- and a
dump taken with a mod of that character installed describes the MODDED pipeline (see
[Vertex Group Remaps](AI%20Agent%20Help/VGRemaps/CLAUDE.md)).

**AND THE "TWO SKINS PACK THEIR MASK INVERSELY" FINDING ABOVE IS WRONG, WHICH IS WHAT A RED STAIN
ON A KIMONO TURNED OUT TO BE (2026-09-20).** Chisa and ChisaParfait **both** mark bare skin with
`R = 255`. The inverse reading came from a percentage-of-flesh-like-pixels test over the two atlases
(88% against 52%), which her pale cream atlas defeats -- "flesh-like" fires on her clothes as
readily as on her skin -- so the repack was mapping Chisa's skin to the target's cloth code and her
kimono to the target's SKIN code, shading the garment with subsurface scattering. **Rendering the
two masks' R channels beside their diffuses settles it in one glance** (Chisa's is black with one
white patch, exactly where her diffuse is flesh; the skin's is white over a bare torso), and it
outlived four fixes aimed at registers because it was never in a register. Two corollaries: a repack
that rewrites ONE channel leaves the others in the source's packing (her codes carry `B ~126, A 0`
where his carry `B 0, A 255`), and G is kept from the MOD because green is how shiny a surface is.
**And a FLAT mask is not a neutral one** -- Chisa's hair mask is a flat `(255, 0, 126, 0)`, i.e. "all
of this is skin", which shaded the crown of the hair red; a flat texture has no UV dependence, so the
"bind the source's, the UVs are the source's" rule does not apply and the register is better left to
the game. See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md).

**AND A FIFTH ROUND, ON A KIMONO MOD, FOUND TWO MORE -- ONE OF THEM A FIX OF MINE THAT MADE THINGS
WORSE (2026-09-20).** **(1) A REGISTER A DRAW INHERITED MAY BELONG TO A DIFFERENT CHARACTER
ENTIRELY.** `wwmiPassLayout.py` marks an inherited register `~`, and that was read as "what Chisa
binds there" -- but a frame dump holds everything on screen, and the draw that had last written her
upper body's `ps-t5` appears **byte-identically in the other skin's dump too**: an NPC standing in
both scenes. Its ramp was transcribed into the fix as hers and **the kimono came back yellow**, a
fresh worse symptom in place of the red one being fixed. `--against <the other dump>` now marks such
a register `!`, and the rule is that **only a `sets` line is evidence about a character**. The same
question, asked of every candidate, also separates the three shared globals the plan was about to
"fix" (both characters' own draws set them) from the one genuinely per-character pair: the 512 x 25
subsurface lookup, Chisa's `06790f7e` against the skin's redder `6a9ec87e`, which is what put a red
cast on her decollete. **(2) DO NOT PICK A RIGID ANCHOR OFF A BONE'S `vs-cb4` TRANSLATION COLUMN** --
that is a skinning matrix, not a pose, and reading it named a bone that put her fox mask and hairpins
90 units away at her hip. Nor score candidates by whether the part keeps its axis-aligned extents: an
AABB is not rotation invariant, and that test rejected 267 of 272 bones including the right one. Skin
the part with each candidate, keep the ones whose **RMS radius from the centroid** is unchanged, and
take the one the surrounding geometry already moves with --
`Tools/Misc/Diagnostics/wwmiAnchorSearch.py`. Also: `AnchorChains`' key is a **source** bone and the
chain takes whatever IT maps to, so writing the target id there is a silent no-op-shaped error, and
the part's OTHER bones have to be anchored too (27% of that prop's weight sat on a bone outside the
chain, which left it torn between two places). Both are in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md).

**THREE THINGS TO READ BEFORE ANY TASK, DEPENDING ON WHICH KIND YOU HAVE (2026-09-14, a third added
2026-09-20).** They are the lenses the maintainer keeps having to re-teach, and each now has its own
writing:

- **A feature or a bug** --- [Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s habits **34-37**
  and **55-56** (a symptom that survives a verified fix is a second bug, so re-read the report's
  words; and snapshot every test input's output with the current build BEFORE rebuilding, because
  the prototype-vs-compiled A/B is blind to a change that lands on both sides), and 34 above all: **run a new check against the BROKEN build before you trust it on the fixed
  one.** Three checks written in one session each passed a fix that was wrong, and the first of
  them shipped --- one had `.strip()`ed away the nesting that was the whole question, one quietly
  skipped two of the five mods it claimed to cover, and one counted per section where the question
  was per path. Keeping the previous output directory and requiring the new check to FAIL against
  it costs one command. The other three cover reading the artifact instead of reasoning about it,
  a checkout several agents and the maintainer are all moving files in, and two build-tool failures
  that look like your bug.
- **A remap to create or debug** --- [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s
  four new sections, starting with **"A `TextureOverride` binds registers only for the draw its hash
  matches"**: a mesh slot whose section binds no `ps-t` renders with the **GAME's** textures, so
  whether a slot uses the mod's art or the game's is decided by whether that mod's author bothered
  to write a register line --- and five mods of one skin will disagree about it. The way to learn
  that is to parse the mod's own sections into a per-slot table, never to reason about which
  textures a slot ought to use; one such table explained every reported failure at once and refuted
  three careful diagnoses. The other three: crop the UV island and LOOK at it when the symptom is on
  a texture, pick test mods by STRUCTURAL axis rather than by character, and what `drawindexed =
  auto` actually does.
- **"Make this faster"** --- [Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s **"MAKE THIS FASTER:
  the recipe, and what it has cost to skip a step"**. Four speed-ups landed in one day and **every
  one was a fixed cost nobody had measured, in a place nobody had guessed** -- an eager `numpy`
  import, an automaton rebuilt per keyword, a library decode that was 85% of a texture edit, a
  registry rebuilt on each of the two calls a run makes. The recipe is six steps, and the two that
  are skipped hardest are **print each CALL rather than the total** (a one-time 0.28s init summed
  with a 0.004s one reads as a 0.15s per-file cost, which is a different bug with a different fix)
  and **add a switch so the old path stays reachable** (`AGREMAP_TEXCACHE=0`, `AGREMAP_BC7_DECODE=0`)
  so the A/B happens inside ONE binary. It also carries the env-gated instrumentation pattern and
  its one hard rule: **restore from your own backup, never `git checkout`** -- every file worth
  instrumenting here is already modified by the work in progress.

**AND THE FIRST IN-GAME RUN OF IT FOUND TWO THINGS THAT ARE NOT ABOUT YELAN AT ALL (2026-09-14).**
**(1) `NNFix` AND `ORFix` ARE INVOLUTIONS, so the rule is once per PATH, not once per DRAW.** They
re-slot the bound `ps-t` registers rather than setting them -- `NNFix` reads the diffuse out of
`ps-t0` and the light map out of `ps-t1`, and `CommandListLDX` writes them back the other way round
-- so a second call over one set of bindings undoes the first and the model renders **flat green**.
The old "immediately before every `drawindexed`" rule holds only while no path draws twice, and
**66 sections across 14 mod folders** of one real library draw several times in a single pass
(independent `if` toggles, not an exclusive chain -- and GIMI writes a chain as `else if`, so a
tally looking for `elif` will tell you there are none). `RegDelimitedAddMode::PerPath` is the
corrected rule and ALL THREE templates pass it (the component one only since 2026-09-21 -- it was
missed, and a Citlali mod drawing its body as nine toggled ranges got 9 `ORFix` calls on one path); the acceptance test is a whole mod library fixed into
scratch copies -- **157 folders, 7851 remapped sections, 867 `.ini` files, 0 violations**
(`Tools/Misc/Diagnostics/fixCallPaths.py`). The precise invariant is per path per **binding
generation**: re-binding a `ps-t` resets the count, because a call is undone by the next only while
the registers underneath it have not moved -- so the per-member `ps-t` / `NNFix` / `drawindexed`
blocks the merge emits are two calls on one path and both correct. **(2) A TARGET OBJECT SEVERAL COMPONENTS MERGE ONTO IS NOT ONE DRAW CALL**: the
merged index buffer is member after member, and a mod's own `drawindexed` lines address its own
buffer, so they cover the FIRST member and stop -- Yelan rendered with her fringe and no eyes while
every buffer was byte-perfect. The appended draw has to land at the section's OWN depth
(`RegFillMissingMode::BottomCover`), not "as late as possible" -- a `RegSurroundedAdd` with
`latest = true` puts it inside the last `if` block, which shipped once and gave her eyes only while
one toggle was on. A checker that `.strip()`s a `.ini` line before judging it cannot see that at
all; track the `if`/`endif` depth. Both are written up in Creating Remaps' "The fix libraries are
involutions" and "A target object several components merge onto is not one draw".

**THE LIGHT MAP BAND LEGEND IS A SHARED FILTER NOW (2026-09-15), AND IT SURFACED AN OPEN BUG.**
`MaterialBandRemapFilter` (`model/strategies/texEditors/texFilters/`) takes a character's legend as
a TABLE of `{source band or range -> target band, optional diffuse gate}` and hands both fixer
templates the `lightMapEdit` they already wanted, replacing two ~90%-identical hand-written closures
whose third and fourth copies were **already stubbed** in Bennett's two prototypes. The moves are
applied SIMULTANEOUSLY, every decision from the ORIGINAL alpha, because a legend is a *permutation*
and in sequence a permutation chases itself. Refactor acceptance: 183 files over five mods, all
byte-identical to the previous build; `core/tests/MaterialBandRemapFilter_test.cpp` was proved to
fail against a deliberately sequential build first. **The open bug it found is not in the filter**:
the band gates read a DOWNLOADED diffuse for any component the mod lacks, so when the download does
not land every gate passes and 33312 pixels move that should not -- **the same mod fixed twice gives
two different light maps**, alternating run to run. See Creating Remaps' "A downloaded texture that
does not land changes the band output".

**BENNETT AND BENNETTADVENTURE ARE COMPILED IN BOTH DIRECTIONS (2026-09-15).** The data landed
first, on 2026-09-14, as a deliberate half-step, and all of it is still what the fix reads from.
Both download folders are in (`GI/Bennett/4_0`, 10 files; `GI/BennettAdventure/5_7`,
20 files), and so are the five `ModTypeId`s -- `Bennett`, `BennettAdventure`, and the three
target-only component ids `BennettAdventure{Body,Bang,Eye}`, the YelanTranquil arrangement -- plus
`HashData` (Bennett at **4.0 / 4.1 / 4.3 / 4.4**, following his `hash.json`'s history in the assets
repo, and the skin's three components at 5.7), `IndexData`, `VertexCountData`, five `VGRemapData`
rows and `Data/RemapDrafts/BennettRemapDraft.xlsx`. The counts the suites hardcode moved with it:
GI mod types **45 -> 47**, `VertexCountData` **44 -> 45**, `VGRemapData` **58 -> 63**, the remove
table **45 -> 47** -- and with the fix, the parse table **57 -> 59** and the fix table
**125 -> 128**. Read [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "Recipe: a
classic-shape mod onto a multi-component skin (Bennett and after)" before adding the fix, and its
new "Proving a NEW download folder" before adding another character's assets --- the pipeline is
proved by rebuilding six *shipped* folders byte-identically, because a new one has no golden.
**The vertex group rows are PROPOSED, not confirmed in game**, and Bennett has no hand-made draft
to score against.

**THE PROTOTYPES CAME FIRST AND ARE STILL THE ORACLE (2026-09-15).**
`Tools/Misc/Prototypes/bennettAdventureFix.py` (Bennett -> BennettAdventure) and
`adventureToBennettFix.py` (the reverse), both confirmed in game on real mods, are what the compiled
pair was A/B'd against -- five reverse-direction buffers byte-identical, and a forward file set
matching on everything but a preamble, one extra download and 2431 bytes of one light map. Keep them
working: they are the only thing that can tell a transcription error from a template gap.

**THE PORT NEEDED SIX TEMPLATE CHANGES, AND NONE OF THEM IS ABOUT BENNETT.** Each was invisible on
YelanTranquil because her shapes happen to line up, and each defaults to the previous behaviour so
no compiled character's output moved: a line edit may change a buffer's **stride** (and the copied
resource section still DECLARES the old one, which is the half that renders blank white); a merged
master is **several mods behind one `.ini`**, so the merge follows `run =` and then runs once per
`$swapvar` branch; `ib = null` is a **hidden object**, not a missing component, so it must not
trigger a download; a component taken from downloads brings the GAME's UVs and so needs the game's
**atlas with it**; a band legend is per **object**, not per character; and the section that hides an
unremapped component has an **owner** -- the last configured component's fixer, because each fixer's
output replaces the `.ini` rather than adding to it. All six, with what each cost to find, are in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "BENNETT IS COMPILED NOW, IN BOTH
DIRECTIONS".

**AND THE MERGE PAIRS BRANCHES BY SATISFIABILITY (2026-09-16), WHICH FIXED A BUG THE "SAFE" CASE
ALREADY HAD.** Which branch of one component's `CommandList` belongs with which branch of another's
is decided by asking `Z3` whether the two conditions can hold at the same time, not by index. The
missing piece was an identity: `ResGroupCollect` already computes the query each group co-occurs
under and called `build()` with nothing, so a builder could only count its own calls.
`GroupedResBuilder::beginGroup` hands the query over -- a defaulted no-op, so the split builder and
the `Python` `IniGroupedResBuilder` are untouched -- and `VGMergeGroupResBuilder` takes a resolver
instead of a list of configs.

Pairing by position was not only fragile in theory. `ib = null` used to be DROPPED from a slot's
branch list, so a slot nulled in three of twelve branches had nine entries describing twelve states,
and **four of that mod's twelve merged index buffers were wrong** -- on an `if / else if` chain, the
shape that was supposed to be the safe one. One of the four hands "Nude" the index buffer of "Nude
*Gloveless*" at **exactly the same byte count**, so nothing short of a checksum could see it. The
acceptance test is to reverse a `CommandList`'s chain in the MOD -- a different `.ini` saying the
same thing -- and require the output not to move: 0 of 321 files under satisfiability, 11 under
position. See [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "BENNETT IS COMPILED
NOW".

What the real mods taught, none of which the identity mod can show, is in
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s **"THE IDENTITY MOD IS THE EASY CASE
IN FOUR SEPARATE WAYS"**: a mod that draws for itself carries a stale draw COUNT after the split, a
mod carries whichever VERSION's hashes its author dumped, a merged mod binds nothing directly and
hides its buffers behind `run =` command lists, and an index buffer may be 16-bit -- which only
errors when its byte count does not divide by four. And **a target component nothing is remapped onto
still draws the SKIN's own geometry**: BennettAdventure's bangs sat on top of Bennett's hair as two
different whites, and the reasoning that had left them drawing was simply wrong.

**The method that found it is worth more than the finding** -- see
[Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s "WHEN YOU CANNOT TELL WHAT A DRAW IS USING,
REPLACE THE TEXTURE WITH SOMETHING UNMISTAKABLE". Three measured hypotheses missed; one flat purple
texture settled it in a single run. It is a tool now:
`Tools/Misc/Diagnostics/purpleSlot.py <mod> --component Eye`, reversible with `--off`.

**A SECOND ROUND ON TWO MORE REAL MODS (2026-09-15) FOUND FOUR MORE, AND NOT ONE OF THEM WAS ABOUT
BENNETT.** Blank white eyes were a remapped section binding `ps-t2` and `ps-t3` that the target's
Eye slot does not bind -- her own mod leaves them to the GAME -- and the same trim found the Body
binding `ps-t2` TWICE, so the remapped light map, band move and all, had been silently discarded by
the metal map for four rounds. A second pair of eyes on a HuoHuo-over-Bennett mod was her own Eye
component still drawing, because the hide pass asked what was REQUESTED (`--components`) rather than
what the output actually DRAWS, and that mod weights nothing to the vertex groups the Eye row names.
A mod that looked broken as Bennett while the skin drew perfectly was a merged mod's DISABLED
variants carrying 4.0 hashes, because GIMI's hash-update tools skip `DISABLED*`. And a Texcoord pass
that had **never changed a byte** was credited with a fix throughout.

All four are in [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md) -- "A REMAPPED SECTION
MAY BIND ONLY WHAT THE TARGET'S SLOT BINDS", "A MERGED MOD'S DISABLED VARIANTS CARRY STALE HASHES",
and the request-versus-result correction to "A TARGET COMPONENT NOTHING IS REMAPPED ONTO" -- plus
[Overview](AI%20Agent%20Help/Overview/CLAUDE.md)'s "A PASS THAT NEVER FIRES LOOKS EXACTLY LIKE A
PASS THAT WORKS". **The identity mod is the ground truth for a target's register layout**, which is
the strongest argument yet for building one first.

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

**THE PUBLISHED DOCS COMPILE NOTHING, SO TWO TRACKED ARTIFACTS *ARE* THE SITE (2026-09-17).** Read the
Docs installs `Docs/requirements.txt` and runs Sphinx --- no submodules, no CMake, no Doxygen and no pip
install of the API, since that would mean building z3 (~45 minutes) against a build time limit.
`coreAPI.rst` renders from the committed `core/xml` and `api.rst` from the committed `core.pyi`, through
`Docs/src/extensions/compiledStubs.py`, which stands in for every extension module that is *not* built
beside its stub and steps aside for one that is. So **a pybind11 class you add without regenerating
`core.pyi` renders on your machine and is absent from the published site**, and a stale artifact publishes
a signature the library no longer has --- `git status` on those two is part of finishing a docs change.
Its config was fatal to any branch carrying the C++ API --- it pip-installed the API *and* ran Doxygen from
a working directory where the Doxyfile's relative `INPUT` means nothing --- which the published `latest` did
not notice only because the default branch (`nhok0169`, now `master`) was still the pure-Python library
until 2026-09-18. Test the
published path with `AGREMAP_DOCS_STUBS=force` in front of the usual Sphinx command; see
[Documentation](AI%20Agent%20Help/Documentation/CLAUDE.md).

**THE FOLDER WALK VISITED EVERY BATCH OF FOLDERS BACKWARDS UNTIL 2026-09-20.** `RemapService`'s
walk pushed folders at the back of its queue and took them off the back as well --- the shape an
iterative depth-first walk falls into --- and every batch it queues is already in the order it
should be reported in, so a `Mods` folder holding `A`, `B`, `C` was walked `C`, `B`, `A`, and a
subtree came out after the sibling that follows it. **No file output ever depended on it**: the same
fixture fixed with both builds gave 132 byte-identical files. `summaryLog.txt` did, through its
"skipped due to warnings" list, so the Integration Tester failed 7 of 24 and its **22 log goldens
were regenerated on Linux**. See [Architecture](AI%20Agent%20Help/Architecture/CLAUDE.md)'s "The
folder walk reported every batch BACKWARDS" and [Testing](AI%20Agent%20Help/Testing/CLAUDE.md).

**CITLALI -> CITLALIWHISPEROFSTARS IS COMPILED (2026-09-21), AND THE PORT FOUND TWO BUGS IN THE
COMPONENT TEMPLATE THAT BENNETT AND YELAN SHARE.** A mod that draws an object as several TOGGLED
RANGES (`drawindexed = <count>, <start>, 0`, nine for one real Citlali mod) was carried through the
split with its own numbers -- which overrun the split buffer and, after a removal in the middle
(her eyes), draw the wrong triangles -- and got one `ORFix` per range on one path, which undoes
itself. The template now remaps each range through the split (`VGComponentBuffers::keptTriangleIds`,
bound, and a file-local `DrawRangeRemap`) and passes `RegDelimitedAddMode::PerPath` like the other
two. Bennett's prototype had the first bug differently: it replaced every count with the full split
size, drawing a toggled object once per range with nothing left to hide. Two opt-in config fields came
with it -- `sourceLayout` (Citlali's own sections are already the normal-map layout, so no register
shift or invented normal map) and `faceSwapOnlyFromDiffuseReg` -- and `GIMICharParserConfig::faceDownload`.
Bennett, Yelan and Klee mods are byte-identical across all of it. See
[Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "CITLALI IS COMPILED".

**THE CHARACTER LIST LIVES IN FOUR PLACES OUTSIDE THE LIBRARY, AND ON 2026-09-20 THREE OF THEM WERE
WRONG.** The mod-type table is in `api/README.md`, `apiMirror/README.md` and
`Docs/src/commandOpts.rst` (all three now with a **Game** column, `GI` / `WuWa`), the per-remap
table is `Docs/src/remapGrading.rst`, and the Python `ModTypes` enum is a fourth list of the same
thing. Generated from `GIBuilder`/`WWMIBuilder` and diffed, the tables turned out to carry a
misspelled character (`BarabaraSummertime` --- a name no `--types` argument could match) and a
missing alias, and the enum was **six characters behind** (Bennett, BennettAdventure, Yelan,
YelanTranquil, Sanhua, SanhuaExorcist), which is what the CLI's `--help` was printing. All four are
in step now, `--help` prints the docs link instead of a list that grows with every remap, and
`core/xml` --- which had not been regenerated since Bennett, so every WuWa class was missing from the
published core API --- was regenerated with the pinned Doxygen. **Generate these lists, never retype
them**: see [Documentation](AI%20Agent%20Help/Documentation/CLAUDE.md)'s "A CHARACTER IS FOUR DOC
TABLES" and [Creating Remaps](AI%20Agent%20Help/CreatingRemaps/CLAUDE.md)'s "Closing out a remap".

**Seven repo-mechanics traps that have each cost a full edit-diagnose-repair cycle, none of them
visible from the code:** (1) nearly every tracked text file is **CRLF** (`core.autocrlf=true`), so an
exact-string patch script must normalise to LF before matching and write CRLF back, or every anchor
reports "found 0". **Two corollaries that each cost a cycle on 2026-09-20.** *Assert the anchor
count*: `str.replace` silently does nothing when it matches nothing, so a script that ends
`print("patched")` will tell you it worked having changed not one character --- `assert
t.count(old) == 1` before every replacement, and print which anchor failed. And *the read is half
the line-ending bug*: `open(p, encoding="utf-8").read()` uses universal newlines and hands back LF
regardless, so reading that way and writing with `newline=""` **converts the whole file to LF**
without touching a single line you meant to change. Read binary (or with `newline=""`), normalise
explicitly, write back explicitly -- then re-check `file` or `git diff --shortstat` against
`--ignore-cr-at-eol`; (2) the Bash tool's heredocs eat backslashes (`\ref` arrives as a carriage
return + `ef`), so write patch scripts with the Write tool and run them by path -- **and `sed -i`
mangles the same things in two more ways**: it rewrites a CRLF file as **LF** (silent whole-file
line-ending churn in your diff) and it eats the doubled backslash in this codebase's RST plurals
(`:cpp:enum:`X`\\s` arrives as `X`s`, which is broken RST). **The same eating happens to a `py -3 -c` one-liner run
from the Bash tool** --- `.replace('/', '\\')` arrives as `.replace('/', '\')` and Python reports an
unterminated string literal, which reads like your own quoting and is the tool's; a one-liner that
needs a backslash goes in a script file too (or writes it as `chr(92)`). Prefer a Python patch script for any
file with CRLF or doc comments; if you do use `sed -i`, normalise the file back to CRLF afterwards
and check with `git diff --stat` against `git diff --stat --ignore-cr-at-eol` (the two must agree).
**And a swallowed `\r` keeps costing after it is committed**: git's CRLF normalisation refuses to
touch a file containing a LONE carriage return, so that file's working copy is compared raw and
**every diff of it is a whole-file rewrite** -- 419 changed lines where 14 were real, which reads
exactly like the line-ending churn of trap (1) and is not. If `--ignore-cr-at-eol` shrinks a file's
diff to almost nothing, grep it for a carriage return that is not followed by a newline: `TexEdit.h`
carried one inside `\ref resSubType` (a broken Doxygen reference) from an older heredoc until
2026-09-11; (3) the dev Python and the VS install root have both MOVED since much of this
documentation was written, and that pair has now flipped **five** times -- as of 2026-09-20 `py -3`
is **3.13** (the built module is `core.cp313-win_amd64.pyd`) and `vcvarsall.bat` lives under
`Program Files (x86)\Microsoft Visual Studio\18\BuildTools`, with no
`Program Files\Microsoft Visual Studio` existing at all -- the exact reverse of what this line said
on 2026-09-09, which was itself the reverse of the day before. A script carrying the older reading
fails with **`COMPILE_EXIT=9009`** (`cl` is not on the PATH because `vcvarsall.bat` was never
found), which names nothing. **Read the version off
`cbuild/CMakeCache.txt` and locate `vcvarsall.bat` with a `find` rather than trusting any number or
path written down anywhere, this line included**. **And the pair is per-MACHINE, not merely
per-date (2026-09-20): on the 6-core laptop, measured the same day as the reading above, `py -3` is
3.9.3 (module `core.cp39-win_amd64.pyd`) and the only `vcvarsall.bat` is Community under
`Program Files` -- the exact reverse of this sentence, on the other computer.** See
**Building**'s prerequisites;
(4) a `.bat` launched from the Bash tool as `cmd //c C:\Users\...\build.bat` has its backslashes
stripped, never runs, and still exits 0 -- so the "build" silently leaves the *previous* `.pyd` in
place for your tests. Launch build/test batch files from the **PowerShell** tool with
`cmd /c "<full path>"` instead, and verify by the `.pyd`'s mtime (see **Building**); (5) checking
out a release ref from before 2026-09-18 **deletes `api/src/cpp` out from under you** (the release
branch, `nhok0169` then and `master` now, predated the C++ core until `development` was merged into it
that day) and strands `development`'s submodules under `api/extern/` as part of ~11k untracked files,
so move your working directory to the repo root before switching and **never `git add -A` there** --
see **Overview**'s operating norms, which also say why released packages NEED a branch named
`nhok0169` (GitHub's rename redirect does not cover `raw/` downloads, so they 404 without one); (6) **every repo path contains both spaces and parentheses**
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

**BUILD TIMES ARE A PROPERTY OF THE MACHINE, AND THE MAINTAINER HAS TWO VERY DIFFERENT ONES
(2026-09-17).** The figures in the next paragraph are from a 24-thread, 31 GB Xeon. On the
maintainer's other computer -- a 6-core, 16 GB laptop whose repo lives on an **external USB disk**,
often with the game open holding ~7 GB -- the same "tuned" build took 10-30 minutes and agents kept
trying to fix it in code. What was actually wrong, in order of size: the build tree was on the USB
disk (a one-file rebuild 171s there, 20s on the internal SSD), then RAM, then compile work. On
that laptop `cbuild` is now a junction to the SSD, and **`Tools/APIBuilder --buildLocation` (or
`AGREMAP_BUILD_LOCATION`) is how any other checkout or worktree gets the same**. Three code-side
changes landed with it and are now conventions: `bindings.cpp` **declares** each `initCppXxx`
instead of including its header; MSVC builds with `/Zc:inline`; and core class templates used at
`<std::string, std::string>` are **explicitly instantiated** (`extern template` in the header, an
`XxxInstantiation.cpp` beside the source -- follow it for a new template). Measured on the laptop,
none of these helped: fewer `ninja -j` jobs (even with the game open), `/O1`, or sccache as the
default. **Before optimising a build, say which machine you are on and where `cbuild` physically
is** (`Get-Item <repo>\cbuild | Select LinkType,Target`). Building's "Build speed on a small
machine" and the sections after it have the numbers.

**On the Xeon, the build is no longer the ten-minute wall this file's older advice was written
around (2026-09-08).** A one-line change to a
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
