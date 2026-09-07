# Creating Remaps

How to add or fix the remap for one character: the `IniParser` that finds the mod's parts, the
`IniFixer` that rewrites them, and the loop that proves the result actually works in game.

Read [Architecture](../Architecture/CLAUDE.md) first if you have never touched
`model/strategies/` — this file assumes the strategy-context seam and the
`GIMIParser`/`GIMIFixer` split, and does not re-explain them.

<br>

## Start here: adding a character, in order

Two characters are done and they are deliberately the two different *shapes*. **Work out which one
you have first, because four separate decisions follow from it** (see "Two shapes of remap" below):

| | Source and target share geometry | Target is a different model |
| --- | --- | --- |
| Worked example | **Raiden -> RaidenBoss** (`raiden6_1`) | **Amber -> AmberCN** (`amber4_0`/`amber6_1`) |
| Hashes | kept | replaced |
| Originals | **hidden** | left alone |

Then, in this order -- each step is verifiable before the next, and skipping ahead is how sessions
get lost:

1. **Ask which game version** the parser and fixer rows are for. They are version-keyed and are not
   necessarily the same version (Amber is `amber4_0` + `amber6_1`).
2. **Get the hashes and indices in** (`data/HashData.cpp`, `data/IndexData.cpp`) from the
   GI-Model-Importer-Assets checkout -- see "Before writing anything" below. Nothing downstream can
   be right until these are.
3. **Write the parser**: one mod object per thing the fix has something to say about. Verify by
   running the CLI and reading which sections got classified, before writing any fixer.
4. **Write the fixer**, using the table above to decide hiding/hashes/indices.
5. **Wire downloads** if the character needs them -- one line each via `tools/DownloadTools.h`.
6. **A/B against the old script**, then **ask for an in-game screenshot**. Both, always.
7. **Update the counts in `core/tests/BuilderData_test.cpp`** -- it hardcodes the number of rows in
   each builder table, nothing builds it, and adding a character silently breaks it.

**Ask rather than guess about these three**, every time. They are not derivable and a wrong guess is
silent: the game version(s), the `CommandList` path of any external library the fix re-issues (eg.
`NNFix` lives under `CommandList\global\ORFix\`, *not* an `NNFix` folder), and which `ps-tN` a
given texture hangs off.

**And know what "done" means here.** A remap is not done when the run is clean, nor when the tests
pass. It is done when the A/B diff against the old script is explained line by line -- every
remaining difference either intended or a bug you have named -- and the maintainer has confirmed it
in game.

<br>

## Most characters are two short files, not two long ones

Seven characters are done. Six of them (Amber, AmberCN, Mona, MonaCN, Rosaria, RosariaCN) share the
**standard GIMI shape** and are written against a template rather than copied:

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

`moveDrawIndexed` is the nasty one. Amber and Mona ship *identically shaped*
`[TextureOverride<Char>IB]` sections — `handling = skip` plus `drawindexed = auto` — and Amber's fix
moves that draw call onto the drawn objects while Mona's leaves it exactly where it is. You cannot
read it off the `.ini` file. Read the character's pure-Python `IniFixBuilderData` row (the `Ib*`
entries are the tell) **and** confirm against a mod the old script has already fixed.

Raiden is the exception and stays hand-written: a boss remap is the other shape entirely (see "Two
shapes of remap"), not a variation on this one.

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
| One character's parser | `core/{include/AGRemapCore,src}/data/IniParseData/<Name>Parser.*` |
| One character's fixer | `core/{include/AGRemapCore,src}/data/IniFixData/<Name>Fixer.*` |

Every generator in those two tables is still a stub returning `defaultFactory()` **except Raiden's
6.1 pair**, which is the worked example to copy. A real generator is roughly an order of magnitude
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
`core/src/data/IniParseData/<Name>Parser.cpp` -- but the boilerplate lives in
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

### A trailing `NNFix` after the last `drawindexed` is CORRECT -- do not "fix" it

`RegDelimitedAdd` adds its `KVP` before every delimiter **and once at the end of every path-terminal
part**, including when the delimiter is that part's last `KVP` and the stretch after it is empty. So
a section that ends in `drawindexed` renders as:

```ini
run = CommandList\global\ORFix\NNFix
drawindexed = auto
run = CommandList\global\ORFix\NNFix
```

The old pure-Python script emits only the first, so an A/B **will** flag this as a divergence. It is
not one. The behaviour is specified, it is pinned by nine tests in
`Testing/Unit Tester/UnitTester/Tests/test_RegDelimitedAdd.py` -- read
`test_edit_severalDelimitersInOnePart_beforeEachAndOnceAfterTheLast` and
`test_edit_branchEndingRightAfterItsDraw_trailingInsertionInsideTheBranch` before touching it -- and
**the maintainer confirmed in game (2026-09-06) that the extra call is harmless.** An `NNFix` after
the final draw of a path is a no-op for that draw.

Raiden never shows it because her sections carry no `drawindexed` at all: the draw lives inside the
`CommandList` her `TextureOverride` runs into, so the root part is one draw-free segment and gets
exactly one. Amber shows it because `RegFillMissing` appends the `drawindexed` to the very end.

The header's "never twice in a row" line means *two insertions with nothing between them*, not an
insertion following a delimiter. Reading it the other way costs a build cycle and nine red tests.

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

### Editing a texture, if you ever do need to

Nothing needs this today -- the face fix above used to and no longer does -- but the machinery is
built, tested and the pure-Python `_hutaoEditHeadDiffuse` row does exactly this, so the next
character that wants it should not have to rediscover the traps.

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
warped, vertices stretched into spikes (`Images/Jean/JeanSeaWarped.jpg`), which is what wrong
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
```

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

### Getting a genuinely unfixed baseline — two traps

1. **A `RemapBKUP*.txt` backup is not necessarily pristine.** The ones in the test mod were
   themselves taken from an already-fixed `.ini`. Restore from them and the old script correctly
   no-ops, which looks like the old script doing nothing.
2. So instead: copy the folder, run `FixRaidenBoss6.py -s <copy> -u`, delete leftover `RemapBKUP*`,
   and **verify** `grep -c "RemapBlend\|RemapFix"` returns **0** before branching into `old/`/`new/`.

The `EOFError: EOF when reading a line` ending every run is `logger.waitExit()` with no stdin.
Harmless. A run left sitting at `== Press ENTER to exit ==` **holds the `core.*.pyd` open**, which
makes the next build's install step fail — see [Building](../Building/CLAUDE.md).

### What to check

```bash
# The load-bearing one. An unremapped .buf is byte-identical to its source,
# produces a correct-looking .ini, and reports success.
cmp -s "$d/<Folder>/<Char>Blend.buf" "$d/<Folder>/<Char><Target>RemapBlend.buf" \
  && echo "NOT remapped" || echo "remapped"
```

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
