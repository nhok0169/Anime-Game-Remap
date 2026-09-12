# Creating Remaps

How to add or fix the remap for one character: the `IniParser` that finds the mod's parts, the
`IniFixer` that rewrites them, and the loop that proves the result actually works in game.

Read [Architecture](../Architecture/CLAUDE.md) first if you have never touched
`model/strategies/` — this file assumes the strategy-context seam and the
`GIMIParser`/`GIMIFixer` split, and does not re-explain them.

<br>

## The loop changed: prototype in Python, then port (2026-09-09)

**A new remap no longer costs a rebuild per idea.** `CppStrategyOverrides` registers a parser or
fixer at runtime, taking precedence over the compiled-in row for that mod, so the loop is now:

1. **prototype from Python** until the fix is right --- edit, run, look at the output, repeat,
   with no build in between
2. **copy the working shape into the C++ tables** (`core/src/data/Ini{Parse,Fix}Data/<Name>/`)
3. **rebuild once**, and A/B the compiled result against the prototype

Step 1 is where a remap is actually figured out, and it used to be the expensive part: every
guess about which register a texture hangs off, or where a fix call belongs, cost a compile.

Two worked examples live next to the mods they fix, in `Importer/GIMI/Mods/`:

| | what it shows |
| --- | --- |
| `overrideScript.py` | the **config route** --- a `GIMICharParserConfig` and a `GIMICharFixerConfig` handed to `makeGIMICharParser`/`makeGIMICharFixer`, which is the same factory every compiled character uses. GanyuTwilight's whole fix in 38 lines, and its `--ab` proves the output byte-identical to the compiled one |
| `overrideScript2.py` | the **hand-built route** --- a `GIMIParser`/`GIMIFixer` assembled from the individual edits, for a fix the config cannot express |

**Reach for the config route first.** It is the same code path the shipped characters take, so a
prototype written that way ports to C++ as a straight transcription of the config --- which is
exactly what a `Ini{Parse,Fix}Data/<Name>/` row is. The hand-built route is for a character that
is not the standard GIMI shape (a boss remap, say) or an edit no config field covers.

Three things that will cost you an hour each if you learn them the hard way:

* **Attach a logger or read `RemapService.stats`.** A prototype that raises is caught by the
  per-`.ini` guard and recorded in `stats.ini.skipped` --- and with no logger, printed **nowhere**.
  Every defect found while writing those two scripts was diagnosed through that dict.
* **`FRB.IniNamingTools` is not the naming the compiled fixes use.** It is the pure-Python class,
  and its `getModSuffixedName` has a confirmed bug. Use **`FRB.CppIniNamingTools`**.
* **A prototype is not proven by running.** Diff it against something --- the compiled fix if the
  character has one (`--ab`), the old script if it does not.

<br>

## Start here: adding a character, in order

Thirty characters are done, in five *shapes*. **Work out which one you have first, because
several decisions follow from it** (see "Two shapes of remap" below, "A character with TWO
targets" for the third, and "The merge" for the one that writes more than one .ini file):

| | Share geometry | Different model | Target draws MORE objects | Target draws FEWER objects | Stresses the fix libraries |
| --- | --- | --- | --- | --- | --- |
| Worked example | **Raiden -> RaidenBoss** (`raiden6_1`) | **Amber -> AmberCN**, Mona, Rosaria, Ningguang | **Jean -> JeanSea** (`jean6_1ToJeanSea`) | **Keqing -> KeqingOpulent** (`keqing6_1`) | **GanyuTwilight -> Ganyu** |
| Hashes | kept | replaced | replaced | replaced | replaced |
| Originals | **hidden** | left alone | left alone | left alone | left alone |
| Objects | one-to-one | one-to-one | **split** via `objSplits` | **merge** via `objSplits` | one-to-one |
| `.ini` files out | one | one | one | **more than one** | one |

The split and the merge are the same field read in opposite directions, and every pair here has
both halves: Keqing merges onto KeqingOpulent and KeqingOpulent splits back, Shenhe splits onto
ShenheFrostFlower and ShenheFrostFlower merges back. **ShenheFrostFlower -> Shenhe is the widest
one** -- three of the skin's objects through Shenhe's single `body` draw call, so three `.ini`
files.

**A sixth column would be "edits textures", and it cuts across the others rather than being a
shape of its own.** The lantern-rite pairs are where to look for it: HuTao -> CherryHuTao is a
two-into-four split that also invents a normal map and makes a diffuse transparent;
CherryHuTao -> HuTao is a four-into-two merge that edits three textures and needs two of them
keyed by SOURCE rather than by target; Xiangling -> XianglingCheer darkens a diffuse and shifts
the whole model with a `positionEdit`. See "Editing a texture" below -- it is no longer the
hypothetical section it used to be.

The last column is the one to read if your character re-issues anything: the Ganyu pair has a
`$swapvar`-branching `CommandList`, a moved `drawindexed`, `ORFix`, the only re-issued
`TexFx` so far --- whose placement rule is genuinely different from `NNFix`/`ORFix`'s --- and, in
the Ganyu direction, the only fix that **invents** a texture instead of dropping or editing one.

A character may need **more than one fixer** -- one row per target in `IniFixBuilderData`, not a
`MultiModFixer`. Jean is the worked example: she remaps onto JeanCN (ordinary) and JeanSea (split).

Then, in this order -- each step is verifiable before the next, and skipping ahead is how sessions
get lost:

1. **Ask which game version** the parser and fixer rows are for. They are version-keyed and are not
   necessarily the same version (Amber is `amber4_0` + `amber6_1`).
2. **Get the hashes and indices in** (`data/HashData.cpp`, `data/IndexData.cpp`) from the
   GI-Model-Importer-Assets checkout -- see "Before writing anything" below. Nothing downstream can
   be right until these are.
3. **Find the vertex group remap, both directions**, and get it checked in game before any
   `.ini` work -- it is what `RemapBlend.buf` is built from, and no `.ini` check can see it.
   `Tools/VGRemapFinder` proposes it from the geometry in `Data/Mod Downloads/GI/`; every source
   group must map to something, and each direction is its own row in `VGRemapData.cpp`. The
   whole step, the tool, and the "model is kinked in game" recipe are in
   [Vertex Group Remaps](../VGRemaps/CLAUDE.md).
4. **Write the parser**: one mod object per thing the fix has something to say about. Verify by
   running the CLI and reading which sections got classified, before writing any fixer.
5. **Write the fixer**, using the table above to decide hiding/hashes/indices.
6. **Wire downloads** if the character needs them -- one line each via `tools/DownloadTools.h`.
7. **A/B against the old script**, then **ask for an in-game screenshot**. Both, always.
8. **Update the counts in `core/tests/BuilderData_test.cpp`** -- it hardcodes the number of rows in
   each builder table, nothing builds it, and adding a character silently breaks it.

**Before writing anything, find out what already exists for this character.** Four places, and
each can save an afternoon:

| where | what you get |
| --- | --- |
| `Testing/Integration Tester/.../APIDocsTests/expected_*/` | **a full golden `.ini` for some characters** -- Raiden, Amber, AmberCN, Jean and Keqing all have one. That is a free specification: exact section names, hashes, indices, which sections get which edit. Read it before guessing |
| `api/src/py/FixRaidenBoss2/data/Ini{Parse,Fix}BuilderData.py.txt` | the character's pre-migration row -- reference only, but it says what the fix *used* to do |
| `data/HashData.cpp`, `IndexData.cpp`, `VGRemapData.cpp` | whether the asset data is already in (it usually is, across several game versions) |
| `Data/RemapDrafts/<Name>RemapDraft.xlsx` | the maintainer's hand-made vertex group remap, with the reasoning per row in its Comments column. Some early workbooks have one direction only; the CN skins, Kirara, Raiden and Arlecchino have none. See [Vertex Group Remaps](../VGRemaps/CLAUDE.md) |
| `Importer/GIMI/Mods/` **and its parent** | real mods to test with. The maintainer swaps folders in and out of `Mods/`, so check the parent directory too |

**Ask rather than guess about these three**, every time. They are not derivable and a wrong guess is
silent: the game version(s), the `CommandList` path of any external library the fix re-issues (eg.
`NNFix` lives under `CommandList\global\ORFix\`, *not* an `NNFix` folder), and which `ps-tN` a
given texture hangs off.

**A PART THAT ALREADY CALLED `ORFix` NEEDS NO 6.1 ROW, and this is the maintainer's rule rather
than anything readable from the tables (2026-09-10).** GI 6.1 swapped the registers many
characters read their diffuse and lightmap out of -- the same swap behind the white cheek spots
below. `NNFix` was written to answer it, but its authors also **baked the swap into `ORFix`**, so
a mod part that was already re-issuing `ORFix` before 6.1 gets the correction for free. Xiangling
is the worked example: her `4_0` row is registered at `toVersion` 4.0 with **no 6.1 row at all**,
because every part of her fix already calls `ORFix`. Do not add one "for completeness" -- ask.

**And know what "done" means here.** A remap is not done when the run is clean, nor when the tests
pass. It is done when the A/B diff against the old script is explained line by line -- every
remaining difference either intended or a bug you have named -- and the maintainer has confirmed it
in game.

**A SCREENSHOT IS EVIDENCE ABOUT THE GAME'S STATE, AND THE GAME'S STATE INCLUDES ITS CACHES
(2026-09-10).** 3dmigoto serves a texture it has already loaded, so a fix that writes a new `.dds`
can be looked at in game and judged on the *previous* run's output. HuTao's face diffuse got an
alpha edit on the strength of one such look, and the edit came back out a day later once it was
clear what had been seen. When a screenshot disagrees with the file on disk, the file on disk is
the one to check first -- open the written `.dds` (see [Texture Editing](../TextureEditing/CLAUDE.md))
rather than reasoning from the picture.

<br>

## Most characters are two short files, not two long ones

Thirty-six characters are done, and THIRTY-FIVE of them go through the same template rather than
being copied -- everything from the plainest CN skin (Amber, Mona, Rosaria) through the three-way
merge (ShenheFrostFlower) to the ones that edit textures conditionally and shift a `Position.buf`
(AyakaSpringbloom, CherryHuTao, XianglingCheer). Only Raiden, whose remap keeps the source
geometry and hides the originals, is written by hand:

- `data/IniParseData/GIMICharParser.h` — `makeGIMICharParser(GIMICharParserConfig)`
- `data/IniFixData/GIMICharFixer.h` — `makeGIMICharFixer(GIMICharFixerConfig)`

A whole character is then this much:

```cpp
GIMICharParserConfig config{};
config.modTypeId = ModTypeId::Mona;
config.downloadCharFolder = "Mona";
config.downloadVersionFolder = "4_0";
config.downloadPrefix = "Mona";
config.drawnObjs = {"head", "body"};
config.texcoordStride = 12;
return makeGIMICharParser(std::move(config));
```

**Check these four per character; none is guessable, and three have already differed:**

| | Amber / AmberCN | Mona / MonaCN | Rosaria / RosariaCN |
| --- | --- | --- | --- |
| `drawnObjs` | head, body | head, body | head, body, **dress, extra** |
| `texcoordStride` | 12 | 12 | **20** |
| `moveDrawIndexed` | **true** | false | false |
| download prefix | = folder | = folder | = folder (**not always**) |
| `objFixCalls` | default (`NNFix`) | default | default |

A fifth followed with GanyuTwilight: **`objFixCalls`**, which names the external libraries an object
re-issues. It defaults to `NNFix` alone; GanyuTwilight's head is
`{NNFixPath, TexFxTransparency0}`. Only characters whose row names a `TexFx` sub-command exercise
the `TexFx` path at all -- see "The three external libraries" below.

`moveDrawIndexed` is the nasty one. Amber and Mona ship *identically shaped*
`[TextureOverride<Char>IB]` sections — `handling = skip` plus `drawindexed = auto` — and Amber's fix
moves that draw call onto the drawn objects while Mona's leaves it exactly where it is. You cannot
read it off the `.ini` file. Read the character's pure-Python `IniFixBuilderData` row (the `Ib*`
entries are the tell) **and** confirm against a mod the old script has already fixed.

Raiden is the exception and stays hand-written: a boss remap is the other shape entirely (see "Two
shapes of remap"), not a variation on this one.

<br>

### The SHAPE tells you `objSplits` and nothing else -- transcribe the old row (2026-09-12)

The temptation, once you can recognise a split from a merge, is to write the config from the shape:
`drawnObjs`, `objSplits`, the download folder, done. **A config with only those is not wrong, it is
EMPTY** -- it compiles, runs, remaps the blend, writes an `.ini` whose every section name matches the
old script's, and silently does none of the character's actual work. Three of the six characters in
the Lisa/Klee/Barbara batch were written that way first, and the A/B reported 0 differ on all of
them, because the A/B compares section NAMES (see "What the A/B's section check does not compare").
What gave it away was counting side effects rather than reading the summary: the old script edited
two `.dds` files for Klee and ours edited **zero**.

**So the step is not optional and it is not a skim: open the character's pure-Python
`IniFixBuilderData.py.txt` row and account for every entry in it.** The mapping is mechanical:

| Old row entry | Config field |
| --- | --- |
| `RegRemove` | `objRegRemovals` / `srcObjRegRemovals` |
| `RegRemap` | `objRegRemaps` / `srcObjRegRemaps` |
| `RegTexEdit` | `texEdits` |
| `RegNewVals` | `objNewRegVals` |
| `Ib*` entries | `moveDrawIndexed = true` |
| `preRegEditOldObj = True` | the edit applies PRE-split, so BOTH halves inherit it |

Three ways that reading goes wrong, all of them found the expensive way in one batch:

**`preRegEditOldObj` decides whether a split's second half inherits an edit, and the two splits
in the repo disagree.** `klee4_0` sets it, so the texture edit written against `body` lands on the
dress copy too and the config needs a SECOND `texEdits` entry with `srcObj = "body"` to say so.
Jean, the other split, does not set it. Copying Jean's arrangement into Klee produced a dress with
no lightmap edit -- visible in game, invisible in the diff.

**"The nearest preceding mention" is not attribution.** The `.py.txt` tables interleave helper
functions with the rows that use them, so a `TexEditor` defined a few lines above a character's row
very often belongs to a different character. A block of texture edits was attributed to
BarbaraSummertime on exactly that reasoning and turned out to be `cherryHutao5_3`'s. **Match on the
function NAME** (`klee4_0`, `lisaStudent4_0`), never on proximity.

**A field a neighbouring character carries is not a field yours carries.** Jean's `objNewRegVals`
nulls her `ib` register; that was copied into Klee and LisaStudent because their configs looked
like hers. Neither pure-Python row has it. If you cannot point at the entry in the old row, it does
not go in the config.

**When you change the template, prove it by byte-identical output.** Porting Amber onto it produced
an `.ini` byte-for-byte identical to her hand-written version, which is the only reason the port was
trustworthy. The suites do not cover this layer well enough to catch a behaviour change.

<br>

## The one thing to internalise: **almost every failure here is silent**

A remap can be wrong in five different ways and *every observable signal still says it worked* —
the section names look right, the `.buf` file is on disk, the summary prints a success line, no
exception is raised. That is not bad luck; it is the shape of the domain. The `.ini` file is data,
the game is the only interpreter, and this software has no way to ask it anything.

Concretely, all five of these happened while building Raiden's 6.1 remap, and none of them raised:

- the fix generated **no remapped sections at all** and hid the originals on top of it, so the mod
  rendered nothing
- the `Blend.buf` was written as an **unremapped copy**, which explodes the mesh in game
  (`Images/Raiden/6_1/RaidenBlendBroken.jpg`) while looking perfect on disk
- the resource was built correctly and **counted nowhere**, so the summary said `0 Blend.buf files`
- `--undo` computed a perfect removal and **threw it away**, while printing
  `Removed fix from up to 3 .ini files`
- the fix reflowed **every line of every `.ini` file** from CRLF to LF

So: **never conclude a remap works because a run was clean.** The two things that actually decide it
are the A/B diff against the old script (below) and a screenshot from the game.

<br>

## Where the data lives — **not** in the `.py.txt` files

`data/IniParseBuilderData.py.txt` and `data/IniFixBuilderData.py.txt` are **reference only**. They
are the pre-migration pure-Python tables, kept so you can read what a character's fix used to do.
They are not loaded, and porting them by editing Python will not work: `CppIniParseBuilderArgs` is
bound opaque (*"there is no way to build one from Python yet"* — only the fixed-factory flavour is
constructible), so the version-keyed tables cannot be fed from `ModData.IniParseBuilderArgs` at all.

The live tables are C++:

| | |
| --- | --- |
| Parse table | `api/src/cpp/core/src/data/IniParseBuilderData.cpp` |
| Fix table | `api/src/cpp/core/src/data/IniFixBuilderData.cpp` |
| One character's parser | `core/{include/AGRemapCore,src}/data/IniParseData/<Name>/<Name>Parser.*` |
| One character's fixer | `core/{include/AGRemapCore,src}/data/IniFixData/<Name>/<Name>Fixer.*` |

Most generators in those two tables are still stubs returning `defaultFactory()`, but **nine
characters are real as of 2026-09-07** -- Amber, AmberCN, Jean, JeanCN, Mona, MonaCN, Raiden,
Rosaria, RosariaCN. `ls core/src/data/IniFixData/` is the current answer; this sentence will go

### Each character owns a folder (2026-09-08)

`IniFixData/` and `IniParseData/` are one directory per character, on both the `src` and the
`include` side:

```
core/src/data/IniFixData/
    GIMICharFixer.cpp      <- shared by 13 characters, stays at the top level
    DarkDiffuse.cpp        <- shared by Ningguang AND NingguangOrchid
    JeanShading.cpp        <- shared by Jean AND JeanCN
    Jean/JeanFixer.cpp
    JeanCN/JeanCNFixer.cpp
    GanyuTwilight/GanyuTwilightFixer.cpp
    ...
```

**The rule: a character's own files live in its folder; a file used by MORE THAN ONE character stays
at the top level of `IniFixData/`/`IniParseData/`.** That is what keeps `GIMICharFixer`,
`GIMICharParser`, `DarkDiffuse` and `JeanShading` where they are -- each is shared. A helper used by
exactly one character belongs inside that character's folder.

So the include is `AGRemapCore/data/IniFixData/<Name>/<Name>Fixer.h`, and a new character means a
new folder in four places (`src`/`include` x `IniFixData`/`IniParseData`) plus its `.cpp` paths in
`core/CMakeLists.txt`, which lists sources explicitly rather than globbing.

The character folders are keyed by **mod type**, not by family: `Jean`, `JeanCN` and `JeanSea` are
three folders, because they are three characters. `Ganyu` has a parser folder and no fixer folder --
its remap is not built yet.
stale. Copy whichever existing one matches your character's *shape* (see the two shapes below), and
note **Jean/JeanCN are the only pair carrying a `texEdits` row**, so they are the ones to read if
your character needs a texture rewritten. A real generator is roughly an order of magnitude
larger than the stub it replaces, which is why each character gets its own file rather than living
in the table's own translation unit. Keep the `Ini*BuilderFuncs::<name><version>` declaration in the
table header (the table refers to it by that name) and put only the *definition* in the new file;
expose it as `<Name>Parser::vX_Y()` / `<Name>Fixer::vX_Y()` for discoverability. **A new `.cpp` needs
a `core/CMakeLists.txt` entry** — the source list is explicit, not a glob.

Adding a row breaks `core/tests/BuilderData_test.cpp`'s hardcoded row/version counts. Nothing builds
`core/tests/*.cpp`, so it fails silently — see [Testing](../Testing/CLAUDE.md).

<br>

## Before writing anything: get the hashes and indices

A remap cannot work without them, and they are the one input this repo does not derive.

They come from the **GI-Model-Importer-Assets** checkout (ask the user for its path;
`PlayerCharacterData/<Character>/hash.json` is the file). Populate
`core/src/data/HashData.cpp` and `core/src/data/IndexData.cpp`.

**`hash.json` is a moving snapshot, not a per-version record.** Its `draw_vb`/`ib` are whatever the
*latest* game version uses, while `HashData.cpp` is keyed by version. Do not file everything under
the character's base version and hope. **Use that repo's own git history** to see what each bump
actually changed:

```bash
git log --oneline -- "PlayerCharacterData/<Character>/hash.json"
git diff <older> <newer> -- "PlayerCharacterData/<Character>/hash.json"
```

For Raiden this showed the 4.1 commit changed *only* `draw_vb` and the 4.3 commit *only* `ib`, with
`texture_hashes` and `object_indexes` untouched across the entire history — so those belong at the
4.0 baseline, and only two rows belong at the later versions. Guessing would have mis-filed 20 rows.

`object_indexes` maps positionally onto `object_classifications`. Cross-check against the
`<Character><Obj>-ib=*.txt` dumps' `first index:` headers, which are independent of `hash.json`.

<br>

## The download assets (`Data/Mod Downloads`)

Step 3 of `Docs/src/createRemap.rst` -- the textures and binaries a fix can *download* for the
character it is remapping onto. The layout is `Data/Mod Downloads/GI/<CharFolder>/<X_Y>/`: one
folder per download character, one subfolder per game version. The sources are the same
**GI-Model-Importer-Assets** checkout the hashes come from (`PlayerCharacterData/<AssetFolder>/`).

**Three separate name spaces collide here and none of them can be composed from another**, so
copying one asset in means resolving all three rather than string-formatting the character's name:

- **Derive the destination file prefix from the files already in that version subfolder**, never
  from the folder name. `AyakaSpringbloom/4_0` uses `AyakaSpringBloom` (capital B) while
  `AyakaSpringbloom/5_4` uses `AyakaSpringbloom`; the `Raiden/` download folder holds
  `RaidenShogun`-prefixed files (its `4_0` is byte-identical to `RaidenShogun/4_0`). Anchor on
  `*Position.buf` / `*Blend.buf` / `*Head.ib`, and fall back to `*HeadDiffuse.dds` -- `Nilou/5_4`
  is a `.dds`-only folder with no buf/ib to anchor on.
- **Glob the asset-side file, don't compose its name either.**
  `GaMing/GamingFaceHeadDiffuse.dds` and `RosariaCN/RosariaFaceHeadDiffuse.dds` are each named for
  something other than their own folder.
- **Asset folder names differ from download folder names**: `Ayaka` -> `KamisatoAyaka`,
  `Raiden` -> `RaidenShogun`. Every other character matched 1:1 as of 2026-09-06.

Two more things that look like mistakes and are not:

- **CN variants legitimately share the base character's texture.** Amber/AmberCN and
  Rosaria/RosariaCN faces are md5-identical *in the asset repo itself*, so writing the same bytes
  under both prefixes is correct -- don't "deduplicate" it.
- **Nine skins have no face texture of their own**: five with no asset folder at all (CherryHuTao,
  JeanSea, KiraraBoots, NilouBreeze, XianglingCheer) and four whose asset folder ships no
  `*FaceHeadDiffuse.dds` (GanyuTwilight, KleeBlossomingStarlight, ShenheFrostFlower,
  XingqiuBamboo). The maintainer's ruling (2026-09-06) is to fall back to the base character's
  texture, renamed to the skin's own prefix. Where a character has several version subfolders the
  **newest** is the target -- the asset repo only ever holds the current game version.

**Putting a file here does not wire it up.** Downloads are registered per character in
`core/src/data/IniParseData/<Name>/<Name>Parser.cpp` -- but the boilerplate lives in
**`tools/DownloadTools.h`** as of 2026-09-06, so a new character writes only its own choices:

```cpp
add({"", "head"}, "ps-t0", "Diffuse", "HeadDiffuse", ".dds");
add({"", "blend"}, IniKeywords::Vb1, IniKeywords::Blend, "Blend", ".buf",
     DownloadTools::bufResourceKVPs(32), DownloadTools::blendRefKVPs(vertexCount));
```

`DownloadTools` owns the GitHub base URL, the `DownloadConfig`, the `RemapDL` naming
(`fixedFileName`), the URL layout (`urlPath`), the common resource `KVPs`, and `vertexCountOf`;
`DownloadStore` owns the `unique_ptr`s, because `GIMIParser::downloads` holds **borrowed** pointers.
Two things it deliberately will not do for you: `urlPath` takes the character folder and the file
prefix **separately** (`Raiden/` holds `RaidenShogun`-prefixed files), and it does not guess which
`ps-tN` anything hangs off.

Do **not** alias it as `Downloads` inside a parser class -- `GIMIParser` already inherits that name
for the borrowed-pointer map, and the inherited one wins. Spell out `DownloadTools::`.

The legacy pure-Python table is `api/src/py/FixRaidenBoss2/data/FileDownloadData.py`, keyed modType -> part
(`"head"`/`"body"`/`"dress"`) -> texture register (`ps-t0`, `ps-t1`, ...). As of 2026-09-06 neither
carries a `"face"` row anywhere, even though `AmberParser.cpp` already treats `tex_face_diffuse` as
a hash swap -- so the `<Prefix>FaceDiffuse.dds` files committed on both branches are present but
referenced by nothing. Which `ps-tN` a given character's face sits on is not derivable from the
folder contents; ask rather than guess. (Coverage is complete as of 2026-09-06: all **44** GI
character folders now ship a `*FaceDiffuse.dds`. They were added deliberately -- the repo used to
omit face textures to save space, and the fix now needs them -- so wiring them up is pending work,
not a mistake to undo.)

**The pure-Python `FileDownloadData.py` has no face rows and never will.** The face textures did not
exist in the downloads back then and fixing faces was not necessary, so its absence there is not a
gap to mirror -- it is simply older than the problem. Add the row to the character's C++ parser:

```cpp
add({"", "face"}, "ps-t0", "FaceDiffuse", "FaceDiffuse", ".dds");   // GI/<Char>/<X_Y>/<Prefix>FaceDiffuse.dds
```

Amber has one. **Raiden deliberately does not** -- the maintainer's call, and her parser has no
downloads at all.

**Before you eyeball a face diffuse, read
[Texture Editing](../TextureEditing/CLAUDE.md)'s first section.** These files put a *blush mask*
in their alpha channel rather than transparency (alpha averages ~3/255 on
`Amber/4_0/AmberFaceDiffuse.dds`, with zero fully-opaque pixels), so a straight `.dds` -> `.png`
conversion renders the face **blank** and looks exactly like an empty or broken texture. Use
`Tools/TexConverter` (its `--alpha` defaults to `drop` for precisely this reason) rather than
concluding the asset is bad.

**Downloads were inert on the C++ path until 2026-09-06, and the symptom was silence.** A
parser recorded them onto `IniFile::getFileDownloads()`, but `RemapService::fixResources` walked
only `getResources()` -- so a download was written into the `.ini` file as a
`[Resource<Mod><Name>RemapDL]` section and the file was never fetched. `_fixResource` had always
known how to run one; nothing ever handed it one. `fixResources` now screens both lists, and
**downloads go first**, because a resource can be built *from* a downloaded file --- in the other
order the edit finds no file and silently writes nothing. If you add a download and see its
`RemapDL` section but no file on disk, check that ordering first.

<br>

**`Data/Mod Downloads` is identical on `development` and `nhok0169`**, so a data-only change here
has to be committed on both -- read [Overview](../Overview/CLAUDE.md)'s norm on switching branches
before you do the second one, because that checkout has teeth.

<br>

## The parser

One `GIMIParser` per (character, version). Its whole job is to say **which sections belong to which
mod object**, using one `GIMISectionClassifier` that works two ways at once:

- **`hashKeyOnlyToModObj`** — for objects a hash names outright. `blend` is the usual case: its
  `blend_vb` hash identifies it and nothing else.
- **`indexKeyToModObj`** — for objects that *share* a hash and need a `match_first_index` to tell
  them apart. `head`/`body`/`dress` all draw from one `ib`, so they go here, keyed by the last two
  index columns of their own `Indices` row — `(component, object)`, which *is* the mod object.

**Set `disjointModObjs = false`.** 3dmigoto reads only the first `hash`/`match_first_index` pair in
a section and applies it to every branch, ignoring its own `if`/`else` grammar — so a section that
spells out a different pair per branch does not actually cycle mod objects the way its author
intended. Attributing such a section to every object it names is what a reader expects, and costs
nothing in practice because nobody writes that structure (anyone who tries hits the same 3dmigoto
bug and stops).

A parser subclass has to **own its `IniFileParseContext` and its classifier** — `GIMIParser` holds a
bare `Context*` and stores obj-target funcs as `std::function`s, and a `Factory` hands back only the
parser. Declare the context before the constructor body so `setCtx` can take its address.

**On `("", "other")`** — a catch-all mod object for sections carrying one of the character's hashes
that no other mod object claims (Position/Texcoord/IB/VertexLimitRaise and friends). Add it *only
when those sections genuinely need their hash or index remapped.* Raiden does not: the 6.1 change
was the NNFix handling, the fix historically only touched the blend, and an `("", "other")` there
emitted sections byte-identical to the originals — pure `.ini` bloat. Other characters do need it.
Ask rather than assume.

<br>

## The fixer

One `GIMIFixer` subclass per (character, version), owning **everything it hands over by pointer**:
its `IniFileFixContext`, every edit, and every group edit. Declaration order is load-bearing twice
over — the context before the constructor body, and every edit before whatever points at it.

A fix is built out of these pieces:

| Piece | What it does |
| --- | --- |
| `GraphRename` | **Turns the copy into the remap.** Without it nothing is generated at all |
| `RegAssetRemap` | Rewrites `hash`/`match_first_index` onto the target mod's equivalents |
| `ResRegCollect` + `VGRemapBlendReplace` | Collects `vb1` and builds the remapped `Blend.buf` |
| `RegRemove` / `RegDelimitedAdd` | The per-character register surgery (eg. NNFix placement) |
| `hiddenModObjs` | Comments out originals the fix *replaces* rather than adds beside |

### Two shapes of remap, and almost everything follows from which one you have

Raiden and Amber are the two worked examples, and they differ in one fact that then decides four
separate design questions. **Ask it first:**

> Does the remap keep the source's `hash` and `match_first_index`, or replace them?

- **Keeps them** (Raiden -> RaidenBoss). The target shares the source's geometry and only the blend
  weights differ, so the original and the remapped section trigger on *the same draw*.
- **Replaces them** (Amber -> AmberCN, and every CN-skin character). The target is a genuinely
  different model with its own hashes.

| | Keeps hash/index | Replaces hash/index |
| --- | --- | --- |
| `hiddenModObjs` | **Required.** Both sections fire on the same draw; hiding the original is what stops the double draw | **Must not.** They can never both fire, and hiding the originals breaks the mod on the character it was built for |
| `RegAssetRemap` | Usually just the blend's `hash` | The `hash` of every object -- but see below, **never** the index |
| `match_first_index` | Left alone | Rewritten per mod object, with a **forward** lookup |
| `("", "other")` | Skip it -- an empty remap is wasted `.ini` space | Usually needed; those sections have hashes too |

### `RegAssetRemap` is right for a hash and wrong for an index

`ModMappedAssets::replace` is *reverse-then-forward*: look the old value up to find which row owns
it, then forward that row's key onto the target.

- For a **hash** the reverse step is the point. The value is what tells you which *kind* of hash it
  is (`ib` / `blend_vb` / `position_vb`), which a mod object cannot tell you, and hashes are unique
  so the lookup is unambiguous.
- For an **index** it is both ambiguous and unnecessary. `0` is *every* character's head index, so
  the reverse lookup fails and writes the literal `IndexNotFound` into a numeric field. And an
  index's meaning **is** its mod object, which the fixer already knows.

So do the index with a forward lookup and `RegNewVals`, per mod object:

```cpp
std::optional<std::string> target = indices->get({toModName_, modObj.first, modObj.second}, toVersion, false);
if (!target.has_value()) { continue; }   // no row on the target is a real answer -- leave it alone
```

`RegNewVals`' `addNewKVPs = false` default is what stops a section with no `match_first_index` of
its own from sprouting one.

### One `GraphRename` per resource kind

`IniNamingTools` has `getRemapFixName`, `getRemapBlendName`, `getRemapPositionName`,
`getRemapTexcoordName` and `getRemapIbName`. Using the generic `getRemapFixName` everywhere compiles,
runs, reports success and produces `…AmberIBAmberCNRemapFix` where every other tool expects
`…AmberAmberCNRemapIB`. `("", "other")` is the one object that genuinely wants the generic name.

### `("", "other")` is many hash types to ONE mod object

It is not a catch-all in the classifier -- there is no fallback. It is several entries in
`hashKeyOnlyToModObj` pointing at the same `ModObj`:

```cpp
{DrawHashKey,        otherObj},   // draw_vb          -- VertexLimitRaise
{FaceDiffuseHashKey, otherObj},   // tex_face_diffuse -- the face's diffuse override
```

**Deduplicate when building `modObjs` from that list**, or the parser is handed the same graph twice.
These sections are a hash swap with no geometry, no index and no resource behind them, which is why
one shared graph is enough.

### The trailing fix call after the last `drawindexed` -- harmless for `NNFix`, NOT for `ORFix`

**This section reversed on 2026-09-08. An earlier revision said the trailing call was correct and
must not be "fixed"; that was true of `NNFix` and false of `ORFix`, and the rule has changed.**

`RegDelimitedAdd` adds its `KVP` before every delimiter **and once at the end of every path-terminal
part** -- including when the delimiter is that part's last `KVP` and the stretch after it is empty.
A section ending in `drawindexed` therefore renders as:

```ini
run = CommandList\global\ORFix\NNFix
drawindexed = auto
run = CommandList\global\ORFix\NNFix
```

That extra call really is a no-op for `NNFix` (confirmed in game 2026-09-06). **`ORFix` is not the
same**: it swaps the diffuse and lightmap registers on *every* call, so a surplus call leaves them
swapped, and the model renders green and yellow. GanyuTwilight re-issues `ORFix`, and that is how
this surfaced.

So `RegDelimitedAdd` now takes **`pathEndOnlyWhenUndelimited`**, and `GIMICharFixer` passes `true`
for the mandatory libraries:

- **`false` (the default)** -- every delimiter-free segment holds the addition, the last one
  included. This is what the ~14 tests in `test_RegDelimitedAdd.py` pin, and it is what the bound
  Python class still does for every external caller. Unchanged.
- **`true`** -- before every delimiter, and at the end **only for a path that never delimits at
  all**. That last clause is load-bearing: Mona and Rosaria have no `drawindexed` anywhere, so the
  end-of-path addition is the *only* thing that places `NNFix` for them. "Never add at a path end"
  would silently drop it.

The condition has to consult the callee, not just the part: a `TextureOverride` that draws nothing
itself has still drawn by the time control returns from the `CommandList` it ran.

`RaidenFixer` builds its own `RegDelimitedAdd` and was deliberately left on the default -- it has no
A/B fixture to prove a change against, and it documents the same rule in a comment. Worth revisiting
if you touch it.

**Do not read a line-count difference here as a behaviour difference.** Old writes the call in the
two `$Tight` branches; we write it in the six `ib` branches. A path executes exactly one of each, so
`6 vs 2` in the `.ini` is `1 vs 1` at runtime. Trace the execution order along one path before
concluding anything -- see "Count per path, not per line" below.

### The three external libraries are NOT one rule: two are mandatory, one is opt-in

`NNFix`, `ORFix` and `TexFx` all live under `CommandList\global\...` / `CommandList\TexFx\`, and it
is tempting to place all three the same way. That is wrong, and it shipped wrong.

| | Mandatory? | Placed by | Keyed on |
| --- | --- | --- | --- |
| `NNFix`, `ORFix` | **yes** -- re-issued for every drawn object | `RegDelimitedAdd` | the draw call |
| `TexFx` | **no** -- the modder opts in | `RegSurroundedAdd` | its own registers |

`TexFx` owns two dedicated registers, `ps-t69` and `ps-t70` (`IniKeywords::PsT69` / `PsT70`). A
modder opts into the library **by binding them**, so a `TexFx` sub-command belongs only after one of
them is bound, and nowhere at all when neither is. It does not have to be the last thing on the
path. That is exactly an any-of "must come before" group:

```cpp
RegSurroundedAdd<>(
    RegSurroundedAdd<>::Additions{{IniKeywords::Run, path}},
    RegSurroundedAdd<>::RegMap{},   // beforeRegs
    RegSurroundedAdd<>::RegMap{},   // afterRegs
    false,                          // latest
    RegSurroundedAdd<>::RegMap{{IniKeywords::PsT69, {}}, {IniKeywords::PsT70, {}}});  // optBeforeRegs
```

Placing `TexFx` like `NNFix` put six `TN.0` calls on GanyuTwilight's head where one belongs.

**And never delete a mod's own `TexFx` call.** `removeFixCalls_` strips the source's fix-library
calls so the fixer can re-issue its own, which is right for the two mandatory ones. Matching `TexFx`
by folder also deleted `run = CommandList\TexFx\Transparency.0` -- a feature of somebody's mod, that
this fixer never re-issues and therefore cannot duplicate. The predicate strips ORFix/NNFix plus
**only the `TexFx` sub-commands this character's `objFixCalls` names**, and leaves the rest alone.

### Where `drawindexed` goes decides whether the mod's own effects work

The single most expensive lesson of the GanyuTwilight session. A mod's `CommandList` often sets
things up *after* its geometry bindings:

```ini
[CommandList<Mod>Dress]
if $top == 0 ...
    ib = ResourceDressIB.0
    ps-t0 = ...
endif
if $DressTransparency == 1
    ps-t69 = ResourceDressTransparency
    run = CommandList\TexFx\Transparency.0
endif
```

Unfixed, nothing in this section draws -- the draw lives in the shared `[CommandList<Mod>IB]`, which
runs once this section has **returned**. So `ps-t69` and `TexFx` are in place before any geometry is
rendered.

Re-issuing the draw **inside the `ib` branches** puts it ahead of that block, and the dress is drawn
before its transparency exists. The mod looks right and the transparency silently does nothing.

So the draw is re-issued with **`RegFillMissing`**, which places it after everything the section
sets up -- see [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)'s "one placement per graph" section
for the bubble-up that makes that work.

**The old script derives the draw from `ib` and has this bug.** Its `IbRemapData` /
`IbDrawIndexedRename` chain was simulating `RegFillMissing` with the filter tools it had -- it had no
`RegFillMissing` and no complex graph filters. Reproducing its topology (which an A/B rewards) also
reproduces the bug. **The old script is a reference, not a specification**; see
[Overview](../Overview/CLAUDE.md)'s habit on agreement not being correctness.

### Count per path, not per line

Every count in this domain is a count along one **execution path**, and the `.ini` is not laid out
per path. Two arrangements that differ wildly by `grep -c` can be identical at runtime:

```
old:  TO[ORFix] -> CL[draw, ORFix($Tight)] -> TO[draw]      = ORFix, draw, ORFix, draw
new:  TO[     ] -> CL[ORFix(ib), draw]     -> TO[ORFix, draw] = ORFix, draw, ORFix, draw
```

`grep -c ORFix` says 16 vs 20; the runtime says one call before each draw, both ways. Before
reporting a divergence, write out the order in which 3dmigoto executes one path -- `TextureOverride`
KVPs in order, stepping into each `run =` where it appears and continuing after it returns.

### The face diffuse: white cheek spots are a REGISTER SWAP, not a bad texture

A problem that is **not** in the pure-Python original: character faces show white shiny spots on
the cheeks.

**Read this before you touch it, because the obvious diagnosis is wrong and was built, shipped and
thrown away once already (2026-09-06, replaced 2026-09-07).** The blush mask really does live in
the face diffuse's alpha channel and really is opaque, so "erase the mask -- set alpha to 1" is a
fix that *sounds* right, produces a plausible edited `.dds`, and is not what is wrong. What is
wrong is that **GI 6.x swapped which register the shader reads the face diffuse and the face
lightmap out of.** A section still binding its diffuse to `ps-t0` is handing it to the slot the
shader now treats as the lightmap, and the mask in its alpha channel is what comes back as the
spots.

So the fix is a **two-way `RegRemap` over the face graph** -- `ps-t0` -> `ps-t1` and `ps-t1` ->
`ps-t0` -- and nothing else. It is one of the things the external `NNFix` library does under the
hood ("an overglorified RegEdit", in the maintainer's words), which is also why a mod that calls
NNFix for itself was never showing the bug.

Two pieces:

1. **Parser** -- give the face a mod object of its own, keyed by the `tex_face_diffuse` hash:
   `hashKeyOnlyToModObj = {..., {FaceDiffuseHashKey, faceObj}}`. It draws nothing; the object exists
   only so the fix can reach the registers its graph binds.
2. **Fixer** -- a `RegRemap` in that mod object's edit list, alongside its `GraphRename`:

   ```cpp
   using RemapTo = RemapList<std::string, std::string>;
   faceRegSwap_ = std::make_unique<RegRemap<>>(
       std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>>{
           {"ps-t0", RegRemap<>::KeyRemapValue(RemapTo{"ps-t1"})},
           {"ps-t1", RegRemap<>::KeyRemapValue(RemapTo{"ps-t0"})}});

   iniEdits.edits[FaceObj] = {renameAdapter_.get(), faceSwapAdapter_.get()};
   ```

Four things worth knowing:

- **Both directions must be in ONE `RegRemap`.** `IfContentPart::remapKeys` rebuilds the part in a
  single pass, consulting the rules once per *original* key, so one edit gives a true swap. Two
  edits in sequence collapse both registers onto one.
- **A mod binding only `ps-t0` is the common case, and needs nothing extra.** Every CN mod checked
  has a three-line face section. It ends up binding only `ps-t1`, which is right -- the game
  supplies the slot the mod says nothing about.
- **It reaches every part of the graph.** That matters for Raiden, whose face is a `TextureOverride`
  running a `CommandList` that binds a different texture per `$swapvar` branch: each branch is a
  part of its own and each one gets the swap.
- **Which `ps-tN` is per character.** Read it off the mod's `.ini` file; it is not derivable, though
  every character so far uses `ps-t0`/`ps-t1`. `GIMICharFixerConfig` carries both.

**Hide the original face section.** Every remap here keeps the source's face hash (RaidenBoss has no
`tex_face_diffuse` row because it does not need one; each CN pair shares one outright -- Amber
`1d064079`, Mona `8e116301`, Rosaria `2abd61ee`), so the original and the remap fire on the same
draw. Left visible the original binds the diffuse to `ps-t0` while the remap binds it to `ps-t1`,
and the face gets a diffuse in **both** slots -- worse than the bug being fixed.

**The face diffuse downloads stay** even though nothing edits the texture any more: a mod missing
its face diffuse entirely still wants one, and the download's KVP is added straight into the part
during *parsing* (`GIMIParser::addDownloads`), so the fixer's swap moves it to `ps-t1` along with
everything else. The ordering works out only because of that -- a download applied after the fix
would land on the wrong register.

### Editing a texture

**THIRTEEN of the thirty fixers carry a `texEdits` now** -- Ayaka, AyakaSpringbloom, CherryHuTao, Ganyu, HuTao, Jean,
JeanCN, JeanSea, Keqing, KeqingOpulent, Ningguang, NingguangOrchid, Xiangling -- so the config route
below is the one to reach for; the hand-built collector after it is for a fix the config cannot
express. Three of them also `texAdds` a texture the mod does not have at all (Ganyu, HuTao,
Xiangling), and two carry a `positionEdit` (Xiangling, XianglingCheer).

```cpp
// {target object, register, edit name, the edit, [compress], [source object]}
config.texEdits = {{"head", "ps-t0", "TransparentHeadDiffuse", &makeHeadTransparent}};
```

Four things about that line, each of which was a bug first:

- **THE REGISTER IS THE ONE THE COLLECTORS SEE, NOT THE ONE THE TEXTURE ENDS UP ON.** Texture
  edits run before the register edits, so a diffuse that this fix later duplicates into `ps-t2`
  or swaps with `ps-t1` is still named `ps-t0` here. Naming the final register collects nothing
  and writes no file.
- **THE EDIT NAME IS PART OF THE RESOURCE GRAPH'S IDENTITY.** Two edits on one object used to
  share a graph named after the object alone, so only the first survived and the `.ini` came out
  with `ps-t1 = Resource...OpaqueBodyLightMap...RemapTex` **referenced and never defined**. In
  game CherryHuTao's body drew with no lightmap, which reads as washed out -- and was reported as
  the sRGB bug below, which it was not. See "the A/B diff cannot see a missing file".
- **`srcObj` (the sixth field) is for a MERGE.** Everything else in this config is keyed by the
  TARGET object, which is exactly right when one source becomes one target and wrong when several
  sources land on one. CherryHuTao -> HuTao edits her body diffuse, her body lightmap and her
  *dress* diffuse, and all three targets are HuTao's `body`; without `srcObj` all three edits fire
  over all three sources. `srcObjRegRemovals` and `srcObjRegRemaps` are the same idea for
  registers.
- **The edit runs on the SOURCE's texture and writes a new file**; it never modifies the mod's own.

#### `obj` is the TARGET, and on a merge that is almost certainly not what you mean

**The single most expensive mistake of the 2026-09-11 batch, and it looks right in every check.**

A `TexEdit`'s `obj` names the object the edited copy ENDS UP on. The edit *names*, though, come
from the pure-Python **parser** row, where they are declared against the object the texture
BELONGS to -- before any merge has happened. On a one-to-one remap or a split those are the same
object and nothing goes wrong. On a merge they can be different objects entirely:

```
AyakaSpringbloom -> Ayaka   objSplits = {{"head", {"body"}}, {"body", {"head","head","body"}}, ...}
```

-- the skin's head is drawn through Ayaka's BODY, and its body through her HEAD. So
`{"head", "ps-t2", "HeadShadeLightMap", ...}` reads perfectly and collects the wrong texture:
the head's shade lightmap edit lands on the BODY's shadow map. In game her neck rendered pale
and flat (`Images/AyakaSpringBloom/6_1/AyakaNeckPale.jpg`), because that lightmap is what shades
it.

**So on a merge, name both: `obj` for where the copy goes, `srcObj` for whose texture it is** --
and expect more entries than the pure-Python row has, because one source landing on two targets
needs one entry per target:

```cpp
// the skin's HEAD -> Ayaka's body
{"body", "ps-t2", "HeadShadeLightMap", &editHeadShadeLightMap, true, "head", "", &RegValChecks::isShadow},
// the skin's BODY -> Ayaka's head AND her body
{"head", "ps-t1", "BodyTransparentDiffuse", &makeTransparent, true, "body", "", &RegValChecks::isLightMap},
{"body", "ps-t1", "BodyTransparentDiffuse", &makeTransparent, true, "body", "", &RegValChecks::isLightMap},
```

**Nothing catches this.** The `.ini` file is internally consistent either way -- every register
resolves, every resource section is defined, every file exists -- so `check_sections`,
`check_dangling` and the section diff all pass. What catches it is comparing the edited TEXTURES
against the old script's, which needs the two harness repairs described under "Verifying".

#### sRGB: the pale-hair bug, and why it is not about compression

**A texture whose DX10 header says sRGB must be pre-corrected on the way out, whatever its
compression (2026-09-10).** `TextureFile::open` reads the DXGI format for every texture and sets
`gamma_ = 1/2.2` whenever the sRGB bit is set -- `BC1/2/3/7_UNORM_SRGB` and
`R8G8B8A8/B8G8R8A8_UNORM_SRGB` alike.

The reason is a Compressonator limitation, not a choice: **it has no sRGB BCn format**. Only ETC2
has sRGB entries in its table, so one `CMP_FORMAT_BC7` covers DXGI 98 *and* 99 and the sRGB bit is
lost the moment the file is loaded. Writing the value back untouched therefore brightens it.

This was got wrong once in the obvious way: the first version gated the correction on
*uncompressed* DX10 textures, on the inference that BCn was already handled because Ganyu's
edited textures A/B'd identical. They did -- because Ganyu's `DarkDiffuse` **declares
`setGamma(1/2.2)` by hand**. Xiangling's and HuTao's head diffuses are `BC7_UNORM_SRGB` with no
such declaration, and came out visibly pale in game
(`Images/Xiangling/6_1/XianglingCheerPaleHair.jpg`). **An A/B that passes because of a per-character
override is not evidence about the general path.**

**`--compressTextures` is not the answer to this, and it does not do what its name suggests.**
Measured: `save(compress=true)` writes DXGI 98 and `save(compress=false)` writes a legacy header,
and **both are linear** -- the flag changes the file's size, not its colour space. (It also only
*permits* compression: `_applyCompressTextures` forces it **off** when the flag is absent, so a
character asking for it in its `TexEdit` does not get it unless the run does too.)

Flagged and not done: writing the sRGB header ourselves instead of baking a 2.2 power into 8-bit
values, which currently crushes the low end (52 -> 8). It needs its own in-game check.

#### Where a hash row's authority comes from, in order

Three face-diffuse rows were added in one batch, on three different strengths of evidence, and the
difference is worth keeping straight because this table's standing policy is to FOLLOW the assets
repo rather than reason a value into existence:

1. **An asset dump says so.** `AyakaSpringbloom`'s `146097c4` is in
   `GI-Model-Importer-Assets/PlayerCharacterData/AyakaSpringbloom/hash.json`. Strongest, and the
   only kind that needs no argument. Note the repo carries only ~10 skins -- `NilouBreeze`,
   `KiraraBoots`, `JeanSea`, `XianglingCheer` and `CherryHuTao` are not among them.
2. **A real mod DECLARES it.** `KiraraBoots` has no asset folder, but a shipped mod binds
   `[TextureOverrideKiraraBootsFaceHeadNormalMap] / hash = 6eb20522` itself. That is an author
   stating the hash, not us inferring it -- a source, not a pattern.
3. **The base/skin pattern.** Seven of the eight pairs that carry both rows share a face diffuse.
   **This is a hint and not a source**: `Keqing`/`KeqingOpulent` do not (`d8c9c399` vs `c2b17f84`).

Only `NilouBreeze` rests on (3), and it was allowed only after measuring that her value does not
reach the output in the direction that can be tested -- see the probe in her row's comment, and
**Overview**'s habit 25. **A hash that is wrong fails exactly as silently as a hash that is
missing**, so when you cannot get to (1) or (2), find out what the value actually affects before
guessing it.

#### A missing row is not a missing feature: it silently disables the section built for it

Worth knowing what the absence costs, because nothing reports it. When a mod lacks an object, the
parser INVENTS a `TextureOverride` to hang the download off, and seeds it with the SOURCE's hash so
the ordinary `RegAssetRemap` can rewrite it to the target's. With no source row there is nothing to
seed and nothing to replace, so the section goes out carrying a register and **no hash** --

```
[TextureOverrideNilouBreezeFaceNilouRemapFix]
ps-t1 = ResourceNilouBreezeFaceDiffuseRemapDL
```

-- which matches no draw call. The file is downloaded, written, referenced, and never sampled.
`check_sections` and `check_dangling` both pass, because every reference resolves. **Grep the
generated `.ini` for a `RemapFix` section with no `hash =` line** after adding any character whose
fix can download.

#### Asking what a register is bound TO (2026-09-11)

`RegRef` and `TexEdit::check` both take a `RegValCheck` -- a test over the register's VALUE --
and `RegValChecks` supplies the five ready-made ones (`isDiffuse`, `isLightMap`, `isNormalMap`,
`isMetalMap`, `isShadow`).

**What they are actually for is not what it looks like.** They are not disambiguating a mod that
binds different things to one slot; they are a guard for a mod that has ALREADY been fixed by
hand. GI swapped which registers the shader reads the diffuse and lightmap out of, and a mod
author can answer that either by re-issuing NNFix/ORFix (better -- the library carries the
correction forward) or by swapping the registers themselves, which is the same thing these
`regEdits` do. A mod that took the second route arrives with its registers already where the fix
was going to move them, and shifting again undoes the author's work. Hence
`RegRemapRule::keepIfNoneMatch`: an occurrence no rule matched has already been moved, so leaving
it alone is the ANSWER rather than a fallback.

**It is a heuristic on purpose and cannot be made exact.** Nothing in a `.dds` says whether it is
a lightmap or a diffuse, so the test is a substring of the resource NAME, mirroring the
pure-Python `_isLightMap` family. Reading the texture's CONTENT instead was considered and
rejected: it would misjudge any mod that recolours a character deliberately -- paint someone in
green goo and their diffuse starts looking like a lightmap. A name is weaker evidence than
pixels, but it is the author's own statement of intent. Someone naming a lightmap "diffuse" is
breaking their own fix, and no heuristic survives an author working against it.

#### One texture FILE per edit, not one per target object

On a merge the same source texture is edited once per target it lands on -- AyakaSpringbloom's body
reaches Ayaka's head AND her body -- and those are two collections of two different registers, so
each ran its own edit and asked for its own file name. That produced 7 `.dds` holding 3 distinct
textures (32 MB for 16 MB of content), and, worse:

```ini
[ResourceAyakaSpringBloomBodyLightMapAyakaBodyAltTransparentDiffuseRemapTex]
filename = AyakaHeadRemapTexHfW_B MQC_B.dds

[ResourceAyakaSpringBloomBodyLightMapAyakaBodyAltTransparentDiffuseRemapTex]
filename = AyakaBodyRemapTexHfW_C MQC_C.dds
```

**The same section defined twice, naming two different files.** The section name is built from the
source resource plus the edit, so it was already identical while the file names were not -- and it
only worked because the two files happened to hold the same bytes. No `.ini`-level check sees this:
both sections exist and both files exist.

Two changes fix it. `GIMICharFixer` names the file after the **source** object when the edit has
one (`texEdit.srcObj.empty() ? texEdit.obj : texEdit.srcObj`), which is also what the pure-Python
original does; and `HashTools::getStableShortHashStr` is used for the name's two hashes.

**That second one matters more than it looks.** `getShortDeterministicHashStr` hands out a FRESH
token on every call -- ask it three times for the same string and you get `HfW`, `HfW_B`, `HfW_C` --
because its job is naming a series of distinct things that might collide. It is the wrong function
for "what is the name for THIS texture", which has one answer. The stable variant memoises by input
while still disambiguating genuinely different strings.

**What is left, deliberately.** `AyakaSpringBloom3` still writes 6 `.dds` for 3 distinct textures in
one folder: two DIFFERENT edit names over one source that happen to produce identical bytes. The old
script writes 6 there too, so we match it file for file; collapsing that would need content
addressing and would DIVERGE. Check the old side before assuming a remaining duplicate is your gap.

And count duplication **per folder**. Across folders it is not duplication at all -- each mod
subfolder is self-contained and needs its own copy, which is why one mod legitimately writes the
same 3 textures 6 times. (This is the third time that distinction has cost this repo time; see
**Overview**'s habit 1c.)

#### A texture edit that COPIES rather than moves: `TexEdit::toReg`

A plain texture edit is a MOVE -- `ResRegCollect` rewrites the register it collected from, so the
original file ends up referenced by nothing. Naming a `toReg` makes it a copy instead: the edited
result is bound to that register and the source register keeps the original.

**`toReg` is a PRE-edit register, exactly like `reg`, and for the face that inverts what you
write.** The face's edit chain ends in the diffuse <-> lightmap swap, and texture edits run
before the register edits -- so a copy that should finish on `ps-t0` is written as `ps-t1`:

```cpp
// Kirara: read the diffuse from ps-t0, put an alpha-1 copy on ps-t1...
// ...which the face swap then lands on ps-t0, with the original on ps-t1.
config.texEdits = {{"face", "ps-t0", "OpaqueFaceDiffuse", &makeFaceOpaque, true, "", "ps-t1"}};
```

#### Where a downloaded texture hangs off: `objDownloadRegs`

`makeGIMICharParser` puts a downloaded diffuse on `ps-t0` and a lightmap on `ps-t1`, which is the
modern layout and right for most characters. **A 4.0-era character reads its diffuse from
`ps-t1` and its lightmap from `ps-t2`, leaving `ps-t0` for a normal map** -- and it varies per
OBJECT as well as per version. Kirara at 4.0 has three different layouts at once (head and body
carry a normal map and sit a slot higher, the dress sits a slot higher without one), and by 5.7
her body and dress have moved down while her head has not. KiraraBoots is the exact mirror. Put
a download on the wrong slot and the shader samples a lightmap as a diffuse; nothing reports it.

`faceDownloadVersionFolder` / `faceDownloadPrefix` are the same idea for the face, and exist
because THREE characters file their face diffuse away from the rest of their assets
(AyakaSpringbloom, Nilou, LisaStudent all keep it under `5_4`), and one of them spells it
differently too -- `AyakaSpringBloomBodyDiffuse.dds` but `AyakaSpringbloomFaceDiffuse.dds`. A raw
GitHub URL is case-sensitive, so that is a 404 rather than a near miss.

#### Shifting the model: `positionEdit`

XianglingCheer stands at a different height from Xiangling, so the fix adds an offset to every
vertex as the `Position.buf` is copied:

```cpp
config.positionEdit = [OffsetY, OffsetZ](const BufLineData& line, long long, double, long long) {...};
```

This is one `BufFile::Filter`, and it is rare -- **46 of the 47 pairs in the pure-Python
`PositionEditorData` are `None`**. It brought a whole subsystem with it
(`RemapPositionResource`, `resEdits/PositionEdit.h`), and one trap worth carrying into any other
new resource kind: see [Architecture](../Architecture/CLAUDE.md)'s note on
`RemapService::_fixResource`, which dispatches on the concrete type and therefore **cannot see a
resource kind nobody added a branch for**. The `.ini` named the file, the summary counted it, and
nothing was ever written.

#### Hand-built, for what the config cannot express

A `ResRegCollect` over the graph, collecting the register, with a **`TexEditorReplace`**
(`resEdits/TexEditorEdit.h`):

```cpp
faceReplace_ = std::make_unique<TexEditorReplace<>>(
    faceResGraph, TexEditor({[](TextureFile& tf) { TexEditor::setTransparency(tf, 1); }}),
    makeResEditConfig(), "resourceRemapTexEdit", std::string("FaceDiffuse"));

faceCollect_.srcRegs  = {{faceGraph, "ps-t0"}};
faceCollect_.resEdits = {{"face", faceReplace_.get()}};
```

- **`TexReplace` alone writes nothing from C++.** Exactly like `RemapBlendReplace`, it names
  everything correctly and does not override `buildResModel`, because the editor that does the work
  reaches it from Python. `TexEditorReplace` exists for that; using the base gives you a correct
  `.ini` file naming a texture that was never created.
- **`resModObj` must differ from the source graph** (`("", "face")` -> `("", "faceRemapTex")`), or
  the resource overwrites the graph it was collected from.
- **The collecting graph still needs its own `GraphRename`**, like everything else -- see below.
- `TexEditor::setTransparency(texFile, alpha)` mirrors the pure-Python helper of the same name
  (`putalpha`) and **overwrites** alpha rather than adjusting it -- which is what separates it from
  the `Transparency` pixel transform, and the right operation when alpha is a mask rather than
  opacity. Expect the written `.dds` to come back with alpha in 0..4 rather than exactly 1: that is
  BC7 re-encoding a uniform value, not a bug.

### A character with TWO targets, and a target that draws different objects

Jean is the first of both, and the two things go together. She remaps onto **JeanCN** (an ordinary CN
skin) and onto **JeanSea** (Sea Breeze Dandelion), and JeanSea draws a `dress` -- her cape -- that
Jean has no geometry for at all.

**Two targets means two rows, not a `MultiModFixer`.** The pure-Python original wrapped both fixers
in one because its table was keyed only by the source; `IniFixBuilderData` is keyed by
`(fromVersion, fromMod, toVersion, toMod)`, so they are simply two rows pointing at two factories.
(`MultiModFixer` still exists, for the different job of one `.ini` classified as several *mod types*.)

**A different object set means a split**, and `GIMICharFixerConfig::objSplits` expresses it:

```cpp
config.objSplits    = {{"head", {"head"}}, {"body", {"body", "dress"}}};
config.objNewRegVals = {{"dress", {{IniKeywords::Ib, "null"}}}};
```

Jean's `body` graph is emitted twice, once carrying JeanSea's body index and once her dress index,
both under JeanSea's `ib` hash. Underneath it is `GraphGroupRemap`, which needed no changes: distinct
targets share the `.ini` group, and a **colliding** target lands in an extra group -- which is how the
opposite direction (JeanSea's body+dress *merging* onto Jean's body) produces `<name>RemapFix1.ini`.

Three things to know:

- **The split runs FIRST in `graphGroupEdits`.** After it, every later edit is keyed by the *target's*
  object names, so the index rewrite and asset remap need not know a split happened.
- **`GIMIObjPartFilter` is the exception** -- it reads the *source's* own hash and index, so the dress
  copy has to ask about `body`. Ask about a "dress" the source never had and the window comes back
  empty and the edit silently does nothing.
- **`Testing/Integration Tester/.../expected_fullFix_someFix/multiFix/select/Jean/` is a full golden
  for this character.** Read it before guessing: it pins the indices, the `ib = null`, and the fact
  that the `ShadeLightMap` texture edit lands on the body and *not* the dress.

<br>

### The merge, and why "per group" has to be asked twice

The reverse of the split: several of the source's objects becoming one of the target's, which is
`objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}}`. Two targets collide, so
`GraphGroupRemap` puts the loser in an additional `IniGraphGroup` and the fixer writes it as
`<name>RemapFix1.ini` -- the importer overlaps the two files and both halves draw. Set
`copyPreamble = IniComments::GIMIObjMergerPreamble` so the generated file says why it exists.

**Everything addressed by group has to be built per group, and there are three such things.** Each
was a separate bug on the way in:

1. **`GraphGroupEdit`'s edits vector is indexed by group**, and a group past its end gets *nothing*.
   One `IniEdits` means the second file comes out an unrenamed verbatim copy.
2. **The index window differs per group.** `GIMIObjPartFilter` reads the *source's* hash and index,
   and group 1's `body` was copied from `dress` -- ask about `body` and the window comes back empty
   and the rewrite silently does nothing.
3. **A `ResRegCollect` is addressed by `GraphId`, whose `iniIndex` is the GROUP.** A blend collector
   built for group 0 never sees group 1, so the second file keeps `vb1 = Resource<Mod>Blend` -- the
   original -- and never gets a `[Resource<Mod><Target>RemapBlend]` section. **Every text-level check
   passes**: the names match, the reference resolves (the original blend is right there), and the
   binaries are byte-identical because the missing file is one nothing referenced.

Also: the graphs the merge does **not** touch -- blend, position, texcoord, ib, VertexLimitRaise,
face -- must be duplicated into every group, or the second file names buffers it does not contain.
Ask for that by listing the same target twice, and give those targets an **identity rename function**
-- `copyGraph` renames as it copies, and these already have a rename of their own, so letting both
run yields `...JeanRemapBlendJeanRemapFix`.

Read the old script's own `JeanSeaRemapFix1.ini` before building one of these. It settles every one
of the questions above in about a minute.

**Transcribing a merge from the pure-Python row is a TRANSPOSE, and the order inside the lists is
the part that carries meaning.** The old table is written `{target: [sources]}` and the config is
`{source: [targets]}`, so Keqing's `{"head": ["dress", "head"], "body": ["body"]}` becomes
`{{"dress", {"head"}}, {"head", {"head"}}, {"body", {"body"}}}`. Reading the old list left to
right gives the **claim order**: first claimant keeps the mod's own `.ini` file, so `dress` first
is what puts the dress in the file a reader opens and the head in `RemapFix1.ini`. Get it
backwards and the fix still works -- it just hides the main object in a generated file.

**A merge can be wider than two, and the same source/target pair may repeat.** ShenheFrostFlower
-> Shenhe puts the skin's `head`, `body` and `extra` all through Shenhe's one `body` draw call,
which is **three** `.ini` files, and her `head` and `dress` have to appear in all three:

```cpp
config.objSplits = {{"head", {"head", "body", "head", "head"}},   // head: groups 0,1,2 + body group 0
                    {"body", {"body"}},                            // body group 1
                    {"extra", {"body"}},                           // body group 2
                    {"dress", {"dress", "dress", "dress"}}};      // dress: groups 0,1,2
```

Repeating a pair is the same mechanism `makeGIMICharFixer` uses internally to put the blend,
position, texcoord and face in every group -- there is nothing special about doing it for a drawn
object. An object that must not draw in the copies gets
`objNewRegVals = {{"head", {{IniKeywords::Ib, "null"}}}}` -- which is what stops Shenhe's head
index range drawing the skin's head mesh a second time, on top of the copy the body call already
put there.

**`texEdits` name the TARGET object, and a merge gives one target several sources -- so a
per-source edit needs `srcObj` (2026-09-10).** Keqing was the easy case: her pure-Python row has
two edits (`OpaqueDressDiffuse`, `OpaqueHeadDiffuse`) whose bodies are byte-identical, so one
entry firing once over each source is the same answer. CherryHuTao is not: she edits her body
diffuse, her body lightmap and her **dress** diffuse, all three landing on HuTao's `body`, and
the three edits genuinely differ.

So `TexEdit` carries an optional sixth field naming the SOURCE object, and there are two more
source-keyed fields beside it:

```cpp
config.srcObjRegRemovals = {{"head", {"ps-t0"}}, {"dress", {"ps-t0"}}};
config.srcObjRegRemaps   = {{"head", shift}, {"dress", shift}};
config.texEdits = {{"body", "ps-t0", "TransparentBodyDiffuse", &invertAlpha, true, "body"},
                   {"body", "ps-t1", "OpaqueBodyLightMap", &flattenEmission, true, "body"},
                   {"body", "ps-t1", "TransparentyDressDiffuse", &invertAlpha, true, "dress"}};
```

**The target-keyed field is the one to reach for by default** -- it is right for every one-to-one
remap and every split. Reach for the source-keyed one only when a merge makes the answer differ
per source, and remember that the two are not alternatives: a fix can use both, and
`groupHasSrc(group, targetObj, srcObj)` in `GIMICharFixer.cpp` is what decides whether a given
group carries a given source at all.

<br>

### A fix must never mutate the parse of the file it is reading

The rule, and the bug that produced it (2026-09-07). `ResRegCollect` built its resource graph with
`copySections = resEdits.size() > 1`, while `createGraph` is handed `ctx.sectionIfTemplates()` -- the
**`IniFile`'s own parsed sections**. With one resource edit the graph pointed straight at them, and
`ResReplace::buildResModels`' `part->setValByInd(ind, newVal)` rewrote the mod's real `filename =`
line.

With one fixer per `.ini` file nobody noticed. With two, the JeanCN fixer left `[ResourceJeanBlend]`
saying `filename = JeanJeanCNRemapBlend.buf`, and the JeanSea fixer took *that* as its source and
produced a `Blend.buf` remapped **twice**.

**Every text-level check passed the whole time** -- correct section names, correct `vb1`, zero dangling
references -- because the `.ini` is rendered from the raw source text plus the fixer's own graph
copies. Only the resource *model* carried the wrong path. The visible symptom was in game: JeanSea
warped, vertices stretched into spikes (`Images/Jean/6_1/JeanSeaWarped.jpg`), which is what wrong
vertex-group weights look like.

**So diff the generated `Blend.buf` byte for byte against the old script's, per sub-mod.** A section
diff cannot see this, and neither can `check_dangling.py`:

```bash
cmp old/<sub>/<Mod><Target>RemapBlend.buf new/<sub>/<Mod><Target>RemapBlend.buf
```

`copySections` is now unconditionally true. If you add a resource edit of any kind, keep it that way.

<br>

### `GraphRename` is not optional

`GIMIFixer` edits a deep **copy** of what the parser found; renaming that copy is what makes it the
remapped mod, and `IniSectionGraph::rename` rewrites every `run =` pointing at a renamed section so
the copy's internal wiring follows. Forget it and the fix emits **zero** remapped sections while
still hiding the originals — strictly worse than doing nothing.

Use `IniNamingTools::getRemapFixName` for drawn objects and `getRemapBlendName` for blend; they are
different conventions and mixing them produces names the remover will not recognise later. Rename
goes **first** in the edit list and **unfiltered** (an empty `PartFilter` = the whole part), since
it renames whole sections.

### `keyFilters` are load-bearing, not polish

Because the parser runs with `disjointModObjs = false`, one section is routinely classified into
several mod objects, and only the stretch governed by a given object's own `hash`/`match_first_index`
pair belongs to it. Without a filter every edit runs over the *whole* section for *each* of its
objects — so "fixing" `body` rewrites `head`'s registers. Wrong output, not merely broad.

`AGRemapCore::GIMIObjPartFilter` is that logic, generalized — the fix-side counterpart of
`GIMISectionClassifier`:

```cpp
GIMIObjPartFilter<> objFilter(ctx.modTypeHashes(), ctx.modTypeIndices(), {"ib"}, ctx.version());

iniEdits.edits[modObj]       = {renameAdapter, removeAdapter, addAdapter};
iniEdits.keyFilters[modObj]  = {PartFilter{}, objFilter.filter(modObj), objFilter.filter(modObj)};
iniEdits.keysToTrack[modObj] = objFilter.keysToTrack();
iniEdits.trackKeys[modObj]   = true;
```

`keyFilters` is a list **parallel to `edits`** — one entry per edit, same order.

**`keysToTrack` is about what the *filters* read, never what the edits act on.** The natural mistake
is `{run, drawindexed}` (the keys the edits touch); the answer is `{hash, match_first_index}`. Get it
wrong and the filter sees an empty colouring, reads it as "this object owns nothing", and skips every
edit while everything still reports success. **Call `objFilter.keysToTrack()` and never restate it.**

### The blend

Use **`AGRemapCore::VGRemapBlendReplace`**, never the plain `RemapBlendReplace`. The base inherits
`ResReplace::buildResModel`, which builds a plain `IniFixResource` — a straight *copy* of the
`Blend.buf` with the vertex-group weights untouched. Only the pybind11 layer overrides it. That
inherited behaviour is the exploded-mesh bug, and a remapped `.ini` pointing at an unremapped `.buf`
is worse than emitting nothing, because everything observable says it worked.

Three more things that each silently produce nothing:

- **`resModObj` must be a distinct mod object** from the source graph — it is where the resource's
  *newly created* graph goes. Pointing it at the source makes the resource overwrite what it was
  collected from. Use `("", "<obj>RemapBlend")`.
- **A blend `partPredicate` must be a whole-part accept/reject**, not the sub-part windowing
  `GIMISectionClassifier` does. The qualifying `hash` is on the `TextureOverride` root while the
  `vb1` is down in the `CommandList` it runs — different sections, so there is no span within one
  part, and windowing by the hash's own order index selects nothing.
- **`resType` must be `"blend"`**, not the base's `"resourceRemapBlend"`. It becomes
  `IniResource::type`, which `RemapStats::get` looks up, and that only knows the short kind names.

### When the blend IS remapped and the model still kinks: a source group with no row (2026-09-09)

Issue #213: a KeqingOpulent mod remapped onto Keqing loaded fine, every check above passed, and
in game the elbows had a kink (`Images/KeqingOpulent/6_1/KeqingCrookedArms.jpg`). The `RemapBlend.buf` was being written, the `.ini` referenced it,
the bytes were remapped -- and the vertex group remap was *correct on every row it had*. What it
did not have was a row at all for KeqingOpulent's groups 75 and 99, two 57-vertex elbow helper
bones. Neither the C++ table nor the pure-Python one before it ever had them.

**An unmapped source group is not dropped.** `BlendFile::remapIndices` writes it as the negative
bone index `-index-1` and keeps its weight, so the game reads a garbage bone matrix for that
share of the vertex. Up to a third of an elbow vertex's weight pointing at bone `-76` is exactly
a kink, and nothing in the fix logs it. So when a remap looks wrong in game:

1. **Read the "unmapped source groups" line first.** `Importer/GIMI/Mods/overrideVgRemap.py
   --dump` prints it for the pair it is set to (the compiled remap has 1 row per source group
   it maps, and a source has one past its largest `BLENDINDICES` value). A gap there is the whole
   diagnosis; a run over the mod's own `Blend.buf` shows how many vertices use the gap.
2. **Let `Tools/VGRemapFinder` propose the pair from fresh geometry and score it against the
   shipped table** (`-C`). It reads the game's raw frame analysis directly
   (`--fromHashes POSITION BLEND IB`, the `hash.json` values; run without hashes to be shown the
   inventory), so "did this bone move in the update" is one run: remap the fresh dump onto the
   old asset-repo dump of the *same* character, and an unchanged skeleton comes out as an exact
   identity (it did, for both Keqing skins). The proposal agreeing with all 101 shipped rows and
   then mapping 75/99 anyway is what said "nothing moved, two rows are missing".
3. **For the one bone in doubt, tally nearest vertices rather than trusting a tied centre.** The
   finder had 75 tied between Keqing's 45 and 46; the vertices it drives sit within 0.006 of
   Keqing's skin, which is driven 69% by her upper arm (40) and 28% by her forearm (22), with the
   two helpers at 1% -- so it went to the upper arm (`{75: 40, 99: 66}`). See the tool's README.
4. **Prove it at the byte level before calling it fixed.** Fix a scratch copy with and without the
   override, decode both `RemapBlend.buf` with `BlendFile.decodeAll`, and confirm the only
   difference is `-76 -> 40` / `-100 -> 66` on exactly the vertices that used those groups
   (266 per sub-mod here), with every weight unchanged. `overrideVgRemap.py --ab` does the copy
   and diff, but only on a mod that has *not* been fixed before -- it skips a `RemapBlend.buf`
   the source already carries, and this one had been.

**Every such gap in the table was then filled (2026-09-09).** The sweep found six directions
with a source group and no row -- Fischl -> FischlHighness (0; the draft had it, the port dropped
it), Jean -> JeanSea and JeanCN -> JeanSea (22 each: the cape and collar JeanSea has no bones for),
Keqing -> KeqingOpulent (96-100: thigh decorations), KeqingOpulent -> Keqing (75, 99) and
Ningguang -> NingguangOrchid (28, 49-51: a feather and the front-dress tips) -- and the maintainer
confirmed the rule: **every source group must map somewhere**, and the early hand-made drafts
left "no counterpart" rows blank before that was known. Three things about how they were filled:

- The geometry that matches the library's versions is `Data/Mod Downloads/GI/<Name>/<ver>`, read
  as a mod folder -- the asset repo's dumps are newer re-dumps and Xingqiu's no longer matches the
  table (74 groups vs the row's 92; that pair needs a fresh dump before anyone judges it).
- For a part the target lacks, the finder's `--mode vertices` (what drives the nearest skin) is
  the primary signal and the chain alignment the second opinion, but both were **overruled by
  hand where the winner was hair or a jiggling bone and the part is not**: a collar goes to the
  neck, a belt charm to the hip, a cape to the upper spine or the arm it drapes over.
- The drafts were filled in the same pass, each filled cell's comment carrying the reasoning, and
  the two directions the drafts never had (KeqingOpulent -> Keqing, Orchid -> Ningguang) were
  written as sheets **from the library's shipped rows**, marked in `E1` as proposals so the
  finder's benchmark does not score the tool against itself.

The sweep also turned up two rows where a hand-made draft and the library disagree and neither is
a gap -- CherryHuTao 60 -> HuTao (draft 59, library 58; the finder says 60) and Nilou 67 ->
NilouBreeze (draft 15, library 61; the finder says 15 at a 95% share). Both left as shipped; the
maintainer decides those.

### Registers: `RegAssetRemap` vs `RegNewVals`

Both write register values; they answer different questions.

- **`RegAssetRemap`** maps whatever value is *actually there* onto the target mod's equivalent
  (reverse-lookup then forward), the way the pure-Python `_getHashReplacement`/`_getIndexReplacement`
  pair did. It takes the asset table **per register**, so in principle hashes and indices are two
  entries rather than two classes. An unmapped value gets `HashNotFound`/`IndexNotFound` rather than
  being left in place — a silently-unmapped hash is indistinguishable from one correctly mapped to
  itself.
- **`RegNewVals`** writes a value that does not depend on the old one. Its values may be a callable
  taking the `ModType`, which is the right tool when the new value is derived from the target mod
  but not from what was there.

**In practice `RegAssetRemap` should only ever be given the `hash`.** Handing it the
`match_first_index` as a second entry is the obvious thing to do and it is wrong — see
"`RegAssetRemap` is right for a hash and wrong for an index" above for why the reverse lookup cannot
work on an index, and what to use instead. Which registers need remapping at all is per-character:
Raiden needs only the blend's `hash`, a CN-skin character needs every object's `hash` **and** a
forward-looked-up index.

<br>

## Seams that work from Python and do nothing from C++

Whole halves of the fixer layer were only ever reached through pybind11. Each fails silently. If a
fix builds nothing, check these before anything else:

- **`ResRegCollect` never builds from C++ unless it is given a context.** It collects *and* builds,
  and the build half is gated on `ctx != nullptr && ctx->hasIni()`. `edit()` passes `nullptr`, and
  it inherited `BaseIniGraphGroupEdit::editFromIni`, which **deliberately discards its `ini`**.
  Fixed in core now (`ResRegCollect::editFromIni` builds an `IniFileResEditContext`), but the
  pattern recurs.
- **`GIMIFixer::applyGraphGroupEdits` hands every group edit a `nullptr` ini.** A fixer that owns
  its own `IniFileFixContext` must override it and pass `ctx_.getIniFile()`.
- **`GraphGroupEdit` had no core-side `PartEdit` adapters** — only `PyPartEdit`. Use
  `GraphPartEdit`/`RegPartEdit` from `graphGroupEdits/GraphGroupPartEdits.h`.
- **`RemapBlendReplace` copies a `Blend.buf` without remapping it**, and **`TexReplace` writes no
  texture at all** — both name everything correctly and leave `buildResModel` to the binding layer.
  Use `VGRemapBlendReplace` and `TexEditorReplace`. Two instances of one pattern: if you find a
  third, it will look exactly like these.
- **Downloads were recorded and never fetched** until `RemapService::fixResources` was taught to
  walk `getFileDownloads()` as well as `getResources()`. Not a seam you have to fix any more, but
  the same shape: a model built by one half and read by nobody.

General rule: when a `graphGroupEdits/` or `resEdits/` class has a `Py*` counterpart that overrides
something, **check whether core overrides it too**. If only the binding does, the plain-C++ path is
almost certainly inert.

<br>

## Verifying — the only thing that actually decides it

The Python suite cannot see `AGRemapCore` work with no binding, and the standalone C++ tests only
assert on a strategy's *shape*. Neither would have caught any of the five silent failures above.

Two launchers live in the user's mods folder (ask for the path — Raiden's was
`E:\Computer\Games\Wuthering Waves Mods\Importer\GIMI\Mods`):

- **`FixRaidenBoss6.py`** — the old pure-Python script. Still works. **This is the reference.**
- **`FixRaidenBoss7.py`** — a thin launcher onto the development checkout's live `main.py`.

Both take `-s <abs path>`. **Always run them against scratch copies**, never the user's real mods.

### The A/B diff cannot see a missing file — check the references too

A section diff compares `.ini` **text**. An `.ini` that names a resource the run never produced --
or produced and then deleted -- diffs perfectly and is broken in game. Add this to every
verification:

```bash
# every "filename = ..." in a fixed .ini must exist on disk
python check_dangling.py <fixed mod folder>

# ...and every "<register> = Resource..." must name a section the folder DEFINES
python check_sections.py <fixed mod folder>
```

**The second check exists because the first one cannot see its own blind spot (2026-09-10).**
`check_dangling` follows a `filename =` to the disk; a register naming a resource section
*nobody declares* has no filename to follow, so it passes with nothing to say. CherryHuTao
shipped exactly that -- `ps-t1 = ResourceHuTaoCherryBodyLightMap...RemapTex`, referenced and
never defined, so the body drew with no lightmap and looked flat. It was reported in game as
the same paleness as the sRGB bug and was a completely different defect. Pool the defined names
**per folder, not per file**: a merge splits its resources across `<name>.ini` and
`<name>RemapFix1.ini` and the importer loads both together.

**And run it on a tree that was NOT undone first.** This is the trap that cost a full in-game
debugging round: undoing to get a pristine baseline is exactly what hides the bug, because the bug
only appears when a folder still carries an old fix.

The real case: a mod folder with three `.ini` files, two of them `DISABLED_` copies still holding
stale remap sections. A folder is handled one `.ini` at a time, **removal then fix**, so the second
file's *removal* deleted the `Blend.buf` the first file's *fix* had just written, and the fix then
skipped rebuilding it as "already done on an earlier .ini". The run logged
`Fixing blend for ...RemapBlend.buf...`, summarised `fixed 1 Blend.buf files`, and left no such file
on disk. In game the body collapsed and left a floating head
(`Images/Amber/6_1/AmberFloatingHead.jpg`).

Guarded twice now: the removal **skips any path already in `resourceStats->fixed`** (never delete
what this run just produced -- the real fix), and `fixResources` treats "already fixed" as true only
when the file is **still there** (a safety net). But the general lesson is the check: **the summary
counting a file as fixed is not evidence that it exists.**

And do not test that guard with the summary's `Removed N old ...` line: `addRemoved` is recorded
even when the file was never on disk, so a phantom removal makes the line appear either way.

**Run the dangling check on the OLD tree too -- sometimes the old side is the wrong one
(2026-09-10).** ShenheFrostFlower -> Shenhe produced fifteen files the old script did not, which
reads like a divergence to explain away until you check: the old output carries **33 dangling
references**, writing `RemapDL` sections and downloading nothing, while the new one fetches all
of them and has zero. The old script is the reference for what the fix should *say*, not a
ceiling on what it should *do*.

**And the pure-Python row is a reference for the FIX, not for the downloads.** KeqingOpulent's
`keqingOpulent4_0` parse row wires no `bufDownloads` or `objFileDownloads` at all, though her
assets have been sitting in `Data/Mod Downloads/GI/KeqingOpulent/4_0/` the whole time. Wiring
them -- which every compiled character does -- changes the **section naming**: the position and
texcoord are then classified as the buf objects they are and come out
`...KeqingRemapPosition` rather than the generic `...PositionKeqingRemapFix`. In the A/B that is
four renamed sections out of nowhere, and it is an improvement rather than a regression -- but
only if you know why it happened.

### Getting a genuinely unfixed baseline — four traps

1. **A `RemapBKUP*.txt` backup is not necessarily pristine.** The ones in the test mod were
   themselves taken from an already-fixed `.ini`. Restore from them and the old script correctly
   no-ops, which looks like the old script doing nothing.
2. **`--undo` does not delete the `.ini` files a previous fix WROTE.** It strips the fix out of
   the mod's own `.ini` and leaves every generated `<name>RemapFix<N>.ini` sitting there, still
   naming textures from that older run. Left in place they fool an A/B in **both** directions:
   the old script looks clean because it regenerates those textures under the same names it used
   last time, while the new one shows dangling references to files that were never its to write.
   `find <baseline> -iname "*RemapFix[0-9]*.ini" -delete` after the undo.
3. **`--undo` does not delete the generated BINARIES either** -- every `RemapTex`, `RemapBlend`,
   `RemapDL` and `RemapPosition` file a previous run wrote is still sitting there. The maintainer
   runs the fix on the folders in `Mods/` directly, so a copy taken from there arrives carrying
   the last run's output, which is then copied into **both** sides and compared against itself.
   The failure is genuinely confusing: files written by an earlier run of the NEW script turn up
   on the OLD side, looking exactly like the reference you are diffing against. Cost an afternoon
   on 2026-09-11. `find <baseline> \( -iname "*RemapTex*" -o -iname "*RemapBlend*" -o -iname
   "*RemapDL*" -o -iname "*RemapPosition*" \) -delete` after the undo.
4. So instead: copy the folder, run `FixRaidenBoss6.py -s <copy> -u`, delete leftover `RemapBKUP*`,
   the generated `RemapFix<N>.ini` files **and every generated binary**, and **verify**
   `grep -c "RemapBlend\|RemapFix"` returns **0** before branching into `old/`/`new/`.

The `EOFError: EOF when reading a line` ending every run is `logger.waitExit()` with no stdin.
Harmless. A run left sitting at `== Press ENTER to exit ==` **holds the `core.*.pyd` open**, which
makes the next build's install step fail — see [Building](../Building/CLAUDE.md).

### The edited TEXTURES need their own comparison, by CONTENT

**A texture edit's output file is named with two hashes, so a name-based diff cannot pair the
two sides at all** -- and one written as an `endswith` test will not even see the files:

```
AyakaBodyRemapTexMzY IMY.dds      <- old script
AyakaBodyRemapTexIV8 LEY.dds      <- ours, same content, different hash function
```

`cmp_binaries.py` used to match names ENDING in `remaptex.dds`; an edited texture ends in its
hash, so every one of them was skipped silently. Several "0 differ" runs were reported over a
live texture bug because of it. It now matches the marker anywhere in the name -- but even so,
**names cannot be paired across the two scripts**, so compare by content instead: read every
`*RemapTex*.dds` on each side, reduce each to a signature (alpha range plus mean R and G is
enough to tell these edits apart), and check that every signature the OLD script produced is
also produced by the new one.

That is what finally distinguished "the same edits, renamed" from "the wrong texture edited" on
AyakaSpringbloom: three of three old signatures reproduced, versus two of five while the bug was
live. Extra copies on the new side are usually harmless -- our file name keys on the TARGET
object where the old script keys on the SOURCE, so one edit can land under two names with
identical bytes.

**The two hashes are not redundant.** The pure-Python name is
`<target><Obj>RemapTex<hash of the source file> <hash of the edit name>.dds`: the first
separates edits of different sources, the second separates DIFFERENT EDITS OF THE SAME SOURCE.
Ours had neither, so CherryHuTao-style double edits wrote to one path and the second landed on
top of the first -- a lightmap given both a colour replacement and an alpha of 1, which the
pixels show plainly (`18a100ff -> 015d0001`) and no `.ini` check can.

### What the A/B's section check does not compare: anything INSIDE a section (2026-09-12)

`--ab`'s section check compares the **set of remapped section names**. That is the right check for
"did we generate the same sections" and it is blind to everything that lives inside one: which
registers a section binds, which resource each points at, whether `drawindexed` is there at all.
An empty config generates every section the old script does and fills none of them in, so it comes
back **0 differ** -- which is how three bare configs passed a full A/B sweep.

**Compare the section BODIES as well.** Names cannot be paired across the two scripts (they hash
generated filenames differently), so compare each section's key ORDER plus the values of the keys
whose value is not a generated filename:

```python
# for every key = value line in a generated section, keep 'key = value' unless the value
# matches Remap(Tex|Blend|DL|Position|Fix) -- for those keep the bare key.
# Then diff the resulting per-section lists between old/ and new/.
```

A full version is `cmp_ini.py` in the session scratchpad; it is ~80 lines and worth rewriting
rather than copying a stale path. Run it on every A/B pair before believing a "0 differ".

<br>

### A before/after snapshot proves nothing if you diff the wrong two snapshots (2026-09-12)

The way to show a config field is load-bearing is to snapshot the output with it off, turn it on,
and diff. Barbara's `moveDrawIndexed` was checked that way and came back **9 of 9 files
byte-identical** -- so it was reported as a flag that did nothing, in a fix that had just been
called done. It was not: the two snapshot directories being compared (`c6_` and `f6_`) were both
taken AFTER the flag was already on. Against the genuine pre-flag snapshot (`b6_`), 9 of 9 files
differ, by exactly the four lines the flag is specified to move.

An A/B run leaves a directory per generation, and a wrong pair of them is indistinguishable from a
true no-op: both print zero. **Name the snapshot after the change, not after the run** (`mdi_off`,
not `c6`), and before trusting a zero, confirm the two inputs actually straddle the change --
`grep` the thing you expect to have moved out of one side and into the other. A no-op result is a
claim about two directories, and it is only as good as knowing which two.

<br>

### What an A/B structurally CANNOT see: the data both scripts share

The A/B compares this library against `FixRaidenBoss6.py`. Where the two **share a defect**, it
reports agreement, which reads as a pass. That is not hypothetical --- it hid two dead hash rows
for the whole life of the C++ table (see Overview's habit 16): `"29cf09   14"` and
`"b0e089    15"`, spaces and all, matching nothing in either script.

**So for anything that comes out of a DATA table, add a check on the data's own shape**, separate
from any comparison:

```bash
# every hash must be 8 lowercase hex digits; the only intended exceptions are
# ShenheFrostFlower's two '000050-ps-t3' shadow ramps
py -3 -c "import io,re; t=io.open('src/data/HashData.cpp',encoding='utf-8').read(); \
  rows=re.findall(r'\{\{\"([^\"]*)\",\s*\"([^\"]*)\",\s*\"([^\"]*)\"\},\s*\"([^\"]*)\"\}',t); \
  print([r for r in rows if not re.fullmatch(r'[0-9a-f]{8}', r[3])])"
```

**And note what the repair did NOT prove.** No mod in `Importer/GIMI/Mods/` binds either hash, so
re-running the A/Bs afterwards showed no change at all --- Nilou and Ganyu both came back 0 differ,
0 undefined, 0 dangling, exactly as before. The rows are correct now and currently unexercised;
they take effect for a mod that binds a dress lightmap by hash. Saying "verified by A/B" of a
latent repair like this one would be false, and the distinction is worth keeping: a green A/B after
a data fix means *nothing regressed*, not *the fix works*.

### What to check

```bash
# 1. Was it remapped at all? An unremapped .buf is byte-identical to its source,
#    produces a correct-looking .ini, and reports success.
cmp -s "$d/<Folder>/<Char>Blend.buf" "$d/<Folder>/<Char><Target>RemapBlend.buf" \
  && echo "NOT remapped" || echo "remapped"

# 2. Was it remapped CORRECTLY? Check 1 cannot tell you -- a file remapped twice
#    passes it. Diff against the old script's output instead, for every sub-mod
#    and every target.
cmp "$old/<Folder>/<Char><Target>RemapBlend.buf" "$new/<Folder>/<Char><Target>RemapBlend.buf"
```

**Check 2 is not optional on a character with more than one target.** It is the only thing that
caught Jean -> JeanSea being remapped through JeanCN first: the `.ini` was perfect, references all
resolved, and the model came out warped in game. If you write that comparison yourself, read
[Overview](../Overview/CLAUDE.md)'s habit 10 first -- a mistyped filename makes `cmp` report a
mismatch that is really your harness.

Then: the set of generated sections vs the old script's, each section's content diffed, and
**repeat-run stability** — run the fix 3-4 times and confirm `merged.ini` comes out byte-identical
every time, then `-u` and confirm it lands back on the pristine file. Repeat runs are where
re-fixing your own output, unbounded trailing newlines, and line-ending reflow all show up, and none
of them appear on a single run.

Finally, **ask the user to check in game and send a screenshot.** That is the only test for whether
the vertex groups are right.

### Reading a screenshot: shape vs colour

Two independent things can be wrong, and the screenshot says which:

- **Shape wrong** -- spikes, stretched geometry, the model torn across the screen. That is the
  **vertex** side: the blend weights (`Blend.buf` / the VG remap), or a `match_first_index` putting
  one object's vertex range onto another. `Images/Raiden/6_1/RaidenBlendBroken.jpg` is this failure
  and `RaidenWorking.jpg` is the same mod fixed.
- **Shape right, colours wrong** -- the silhouette is perfect and the character is flat green,
  yellow or pink. That is the **texture** side: a register that should hold a diffuse is being
  handed a `LightMap.dds` or similar. Either the texture-to-register mapping is wrong, or the
  texture itself needs modifying.

`Images/Amber/6_1/AmberLightMapInDiffuse.jpg` is the second kind -- **and it is worth knowing why,
because it is a fixer bug wearing a texture bug's clothes.** It is what Amber looks like on game
version 6.x when **`NNFix` was never applied**. ORFix/NNFix do some genuinely useful texture
copying, but much of that external library is an overglorified `regEdits/`, and from 6.x on a mod
that does not re-issue `NNFix` loses the copy that puts the right texture into the right register.

So on 6.x, "all green" is the expected symptom of the `RegDelimitedAdd` step silently not firing.
**Check `grep -c NNFix` on the output before concluding anything about textures.** The Amber mod
used for testing is genuinely broken in the texture department, and the old pure-Python library
still remapped it back to its correct colours -- so anything less than that is this library's
regression, not the mod's fault.

<br>

## Things this repo's writers must not do to a user's mod

Learned by doing them:

- **Do not change line endings.** `IniFile::readFromDisk` normalizes CRLF to LF; both writers
  (`IniFile::write` and `IniFileFixContext::writeFixedFile`) restore the file's own ending via
  `IniFile::lineEnding()`. There are exactly two writers — fixing one and not the other looks fixed
  and is not.
- **Do not let output grow across runs.** The fix appends a `\n\n` separator every run and the
  removal does not take the previous one back out, so `srcTxt` is `rstrip`'d first.
- **Do not leave a mod without its `.ini` file when something throws.** A fix *moves the file
  aside before it writes* (`disableIni` renames `X.ini` to `RemapBKUP<name>.txt` and does not copy
  it back), so every exception between those two points used to destroy the user's mod.
  `IniFile::fix` now carries a scope guard that restores from `fileTxt_` on unwind. **Restore from
  memory, never from the backup** -- a run with `keepBackups` off deletes the backup, and the
  rename may have overwritten an older one the modder was keeping.
- **Do not re-classify this software's own output.** `GIMISectionClassifier::classify` refuses
  sections whose name carries `Remap` (as `classifyByTextureOverrideName` always has). A leftover
  remapped section keeps a perfectly valid hash and classifies just as convincingly as the original,
  which is how a second run comes to remap an already-remapped name into
  `…RaidenBossRemapRaidenBossRemapBlend`.
