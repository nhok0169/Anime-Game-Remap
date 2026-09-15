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

Two worked examples live in [`Tools/Misc/Prototypes/`](../../Tools/Misc/README.md) (copies; the
live ones the maintainer runs sit next to the mods in `Importer/GIMI/Mods/` on their machine):

| | what it shows |
| --- | --- |
| `overrideScript.py` | the **config route** --- a `GIMICharParserConfig` and a `GIMICharFixerConfig` handed to `makeGIMICharParser`/`makeGIMICharFixer`, which is the same factory every compiled character uses. GanyuTwilight's whole fix in 38 lines, and its `--ab` proves the output byte-identical to the compiled one |
| `overrideScript2.py` | the **hand-built route** --- a `GIMIParser`/`GIMIFixer` assembled from the individual edits, for a fix the config cannot express |
| `identityMod.py` | **not a fix -- the mod to test a fix on first.** Writes a character's own model as a GIMI mod from its `PlayerCharacterData/<Name>` asset folder (dumps -> the three `.buf` files through `VbFile.readDumpStr`, `.ib` files, textures, a GIMI-shaped `.ini`). Every bone and every material band of the real skin in one mod, where any download covers a subset; see the VGRemaps recipe's step 8 |
| `yelanTranquilFix.py` | the hand-built route **at full size** (2026-09-12; a second Yelan mod, the Fontaine outfit with a cape, found two over-fits of the first in one pass each, and the identity mod then corrected the band legend -- see the VGRemaps recipe's step 8) --- a runtime `ModType` whose hash / index / vertex-group rows are added from Python and whose builders are borrowed from a shipped GI type, three pseudo targets with a fixer each, the textures collected with `ResRegCollect` (edited through `TexReplace`, a flat normal map invented through `TexCreate`) and **the buffers collected as one resource group** with `ResGroupCollect` + `BufReplace` + the core's `VGSplitGroupResource`. Yelan -> YelanTranquil, a skin of three components, which no config field expresses yet; see [Vertex Group Remaps](../VGRemaps/CLAUDE.md)'s recipe, step 8, for the traps it hit. `yelanTranquilFixPerBuffer.py` beside it is the earlier per-buffer `ResRegCollect` + `fixFunc` shape |

**Reach for the config route first.** It is the same code path the shipped characters take, so a
prototype written that way ports to C++ as a straight transcription of the config --- which is
exactly what a `Ini{Parse,Fix}Data/<Name>/` row is. The hand-built route is for a character that
is not the standard GIMI shape (a boss remap, say) or an edit no config field covers.

Six things that will cost you an hour each if you learn them the hard way:

* **Attach a logger or read `RemapService.stats`.** A prototype that raises is caught by the
  per-`.ini` guard and recorded in `stats.ini.skipped` --- and with no logger, printed **nowhere**.
  Every defect found while writing those two scripts was diagnosed through that dict.
* **`FRB.IniNamingTools` is not the naming the compiled fixes use.** It is the pure-Python class,
  and its `getModSuffixedName` has a confirmed bug. Use **`FRB.CppIniNamingTools`**.
* **A prototype is not proven by running.** Diff it against something --- the compiled fix if the
  character has one (`--ab`), the old script if it does not.
* **A `GIMIObjPartFilter` must outlive the callables `filter()` hands out.** They point back at
  it; built as a local of a fixer factory it is collected, every window is empty, and the index
  edit silently does nothing (`match_first_index` keeps the source's value). Keep it on a list.
* **A runtime target that shares a value with the source blinds the reverse lookups.** A pseudo
  target with its own index `0` sent Yelan's head to the `ib` graph; the fix-side filter reverse-
  looks-up the same way. Give the classifier `hashNonVersionVals` / `indexNonVersionVals =
  {"name": <source>}`, and register no index rows for a target nothing looks up.
* **`RemapBlendReplace(fixFunc = ...)` only runs from `RemapService` on an API built after
  2026-09-12** (the non-copyable-cast fix in `PyRemapBlendResource.cpp`); before it, the loop logged
  `return_value_policy = copy, but type is non-copyable` per resource and wrote nothing, while
  `resource.fix()` from Python worked. And give each resource a `resType` the stats know
  (`blend` / `position` / `texcoord` / `buf`); the default `resourceRemapBlend` was counted nowhere
  until the same day.
* **Buffers that depend on each other go through `ResGroupCollect`, and a draw call filled
  afterwards wants `RegFillMissingMode.BottomCover`.** A `ResRegCollect` per buffer is the naive
  shape (issue #190): the blend decides which vertices a component keeps, the ib which triangles,
  and the position / texcoord must follow the same vertex set. `BufReplace(resModObj, kind)` names
  each buffer and builds a member typed by kind, and `IniGroupedResBuilder(VGSplitGroupResource,
  args = [name], kwargs = {component, specs, ibPaths, texcoordLineEdit})` builds the group that
  splits them together. The collect splices the collected register into an `if 1 ... endif`
  block, which splits the section into parts -- and `RegFillMissing`'s default `FillMissing` fills
  the FIRST content part that lacks the register (`IfTemplateNode::getKeyMissingPart`), which put
  `drawindexed` ahead of the ib and the textures. `BottomCover` (2026-09-12) adds a fresh LAST part
  at each root, the same way `TopdownCover` adds a first one, so the draw lands after everything
  the section sets up whatever split it. Read the slot section's order back before believing a run.

<br>

## The Yelan lessons, for ANY new remap (2026-09-12)

Four Yelan mods went through the prototype in one day, and each found something the one before
could not. In the order to apply them:

1. **Build and fix the IDENTITY mod first** (`Tools/Misc/Prototypes/identityMod.py <PlayerCharacterData/Name> <folder>`).
   It is the character's own model as a GIMI mod: every object, every vertex group, every material
   band of the real skin. Any downloaded mod is a subset of it (the china dress used none of the
   jacket bones and painted no fur; the Fontaine outfit hid the dress), so a fix that only ever saw
   downloads carries over-fits that surface one mod at a time. The identity mod is also where a
   skin's TRUE band legend is read from (next point).
2. **Tally a mod per OBJECT at its own vertices before believing anything about it:**
   `Tools/Misc/Diagnostics/modTally.py <mod> [--against <other mod>] [--remap From To Comp]`. It
   prints each object's geometry, its vertex groups (`--against` lists the groups one mod uses and
   the other does not -- the cape's chains showed up there in a minute), and its lightmap bands
   with the diffuse colour under each band. **The band legend is the AUTHOR's, not the skin's**: a
   port keeps its source character's bands (the Clorinde port's hair sits on Yelan's skin band and
   its skin on Yelan's metal band), and an author who leaves alpha opaque has painted every cloth
   as whatever 255 means on the source (fur, on Yelan). Never map a band by number alone.
3. **So a band table is diffuse-conditional.** The prototype's `liftBands`: the source's fur band
   (255) onto the target's (0) unconditionally, the source's skin band (115-127) onto the target's
   (255) only where the diffuse under the pixel is skin-coloured (`skinColoured`: warm, R >= G >= B,
   bright enough). Dark hair painted on the skin band stays put, which on Tranquil is her hair
   band. Apply it to EVERY object's lightmap, the head's included -- "leave the head alone" only
   held because one mod's head was hair alone.
4. **Write every texture with its mip chain** (`TexEditor(mipmaps = True)`, `TexCreator(mipmaps =
   True)`). A texture without one is sampled from its top level at every distance, and on hair
   that reads as scattered off-colour pixels that move with the camera -- "like a lossy
   compression". Every texture the game ships carries 11 levels; ours carried one until this.
5. **A part the target LACKS wants a symmetric, rigid anchor for the whole chain, not the
   finder's per-bone nearest.** The finder had put a cape chain's root on the target's upper arm
   and its tail on a hanging ornament; the cape hung crooked. Read the source groups' centroids
   off the mod (`modTally.py --centroids`) and the target's off its frame analysis
   (`Tools/Misc/Diagnostics/boneCentroids.py <FrameAnalysis> --hashes <pos> <blend> <ib>`), and
   take the hand-made draft's anchor (64 / 68 the shoulder pieces, 63 the chest, for Yelan's
   jacket bones 0-13) -- `VGRemapData.cpp` carries the reasoning in a comment.
6. **A remapped draw section needs `override_byte_stride` / `override_vertex_count`** (the
   target's own buffer is sized for its own vertex count; the mod's 17220 through a bang buffer
   sized for a few thousand is garbage). The count is the component's KEPT vertex count after the
   split -- which means the fix must know the split's result while it writes the `.ini`.
7. **Hiding the target's own parts needs no `ib = null` sections**: the remapped `("", "ib")`
   section keeps `handling = skip` and loses its `drawindexed = auto`, and a skipped draw with no
   re-issue draws nothing. Confirmed in game on three mods and the identity (2026-09-12); the
   maintainer's hand-made pair (`Tools/Misc/YelanExperiments/*HandMade.ini`) hides the slots
   explicitly and both work.
8. **The maintainer's hand-made `.ini` pair is the A/B reference when the generated fix misbehaves
   in game.** Diff section by section, not name by name: three omissions (the vertex-limit
   overrides, the lost sRGB pre-correction, a created texture's gamma) were each a line the pair
   had and the generator did not.
9. **Two objects on one draw slot is the API's merge**: the second claimant lands in
   `<Name>RemapFix1.ini` (then `...2`, `...3` -- the identity's head, body, dress and extra all go
   through Tranquil's slot C, so four files). Each extra file carries the mod's own sections too,
   and each `.ini` group gets its own resource group, so identical buffers are written more than
   once under different suffixes. That is the shape, not a bug.
10. **Run it where the core is built.** When the Windows `.pyd` is behind the C++, the prototypes
    run under WSL (`python yelanTranquilFix.py /mnt/e/...`, or `--wsl` from a Windows shell); a
    prototype that refuses to import names the build lacks is the intended failure.

## Yelan is COMPILED now, through a template every later multi-component skin reuses (2026-09-13)

The prototype above was transcribed on 2026-09-13, and the result is not a Yelan one-off: Yelan is
the first of a shape every GI character from Bennett on has, so the port is a **second template
next to `makeGIMICharFixer`** -- `GIMIComponentFixerConfig` + `makeGIMIComponentFixer(config,
component)` in `data/IniFixData/GIMIComponentFixer.{h,cpp}`, with Yelan's own choices in
`data/IniFixData/Yelan/YelanFixer.cpp` (the slot per component, the strategy, the band legend
as a `liftBands(diffusePath)` filter, the head diffuse at alpha 1) and a plain
`makeGIMICharParser` row in `data/IniParseData/Yelan/YelanParser.cpp`. The acceptance was the
prototype itself: on the identity mod, the china dress and the Fontaine outfit, every buffer the
compiled fixer writes is **byte-identical** to the prototype's, the `.ini` text differs only by
the merge preamble the template adds to its generated files, and the textures differ only in
that the service writes them uncompressed unless `--compressTextures` is passed (the prototype's
own `fixFunc` bypassed that flag). `Tools/Misc/Diagnostics/runCompiled.py` is the driver for that
A/B: fix a scratch copy with the prototype and another with it, diff the folders.

**How the shape is encoded, and why -- read this before adding Bennett:**

- **One fixer per target component, each a row keyed by a component id.** The fix table, the
  hash rows and the naming are all keyed by a mod type NAME, so each component of the skin is a
  `ModTypeId` of its own -- `YelanTranquilBody` / `Bang` / `Eye` -- **as a TARGET ONLY**, like the
  boss ids: no `GIBuilder` factory, never registered, no keywords, no remove row. It was tried
  the other way first, and a registered mod type with no keyword crashed the classifier
  population (`keywords[0]` on an empty list in the population test is the visible half).
  `ModTypeIdTools::getHashRemapTargets(Yelan)` lists the three; `YelanTranquil` itself stays the
  id a mod OF the skin classifies as. The component's hash rows live under its name in
  `HashData.cpp` (5.7), and `IniFixBuilderData` has three `(Yelan, <component>)` rows.
- **The slot indices are in the config, NOT in `IndexData`** (`Component::slotIndex`), and the
  reason is a trap worth knowing on its own: `ModMappedAssets::getKey` -- the REVERSE lookup
  every classifier and part filter makes -- buckets every row holding a value by version, takes
  the newest bucket at or below the version asked, and searches only inside it. A slot filed as
  `"0"` at 5.7 made the 5.7 bucket THE bucket for `"0"`, and every classic character's head
  (index 0, filed at 4.0) reverse-resolved to a slot named `A`. Measured on the identity mod's
  own head. The same rule is why the fixer's own file lookup and the shared parser read the index
  row UNFILTERED by name and check the object name instead: at the latest version the `"0"`
  bucket holds a 5.3 character's row and nothing of a 4.0-era one.
- **The shared parser now filters HASH lookups to the character's own rows** (a hash value is
  unique per character, and every name in `HashData` is a `ModTypeId` name, so this can never
  miss a real row). What it stops: a section carrying another character's hash of the same TYPE
  read as one of this character's objects -- the maintainer's hand-made YelanTranquil `.ini`
  (Tranquil's ib hash, index 0) in a Yelan mod's folder classified as Yelan's head, followed its
  `ib = null`, and failed the file.
- **`ResGroupCollect` had the same inert-from-C++ seam `ResRegCollect` had**: it inherited
  `BaseIniGraphGroupEdit::editFromIni`, which drops the `.ini`, so a compiled fixer collected
  every buffer and built nothing, silently -- `blend: fixed 0` was the only tell. It has its own
  `editFromIni` now. The grouped builder a C++ fixer hands it is `VGSplitGroupResBuilder`
  (`graphGroupEdits/VGSplitGroupResBuilder.{h,cpp}`), which COPIES each member the collect built
  into the group (the collect's capture buffer owns the original, and a group owns its members)
  and stores the group on `IniFile::getGroupedResources`.
- **The fixer reads the mod's buffers while fixing the `.ini`** -- the first ini fix that opens
  a binary file. It runs `VGComponentSplit` once, at construction, over the files it finds by
  hash in `IniFile::getIfTemplates` (the fixer is built before the parser parses), to learn which
  objects its component draws and the kept vertex count the `override_vertex_count` / blend
  `draw` lines need. The grouped resource runs the split again when it writes the files.
- **The identity mod is the download set**: `Data/Mod Downloads/GI/Yelan/4_0` is Yelan's own
  buffers and textures out of the asset folder, so a mod naming no lightmap for an object still
  draws with one. It is not on GitHub until committed and pushed, so a download attempt 404s
  until then -- and a 404 for a HEAD texture is the tell that the parser did not find the head.

What is still open: the Windows `.pyd` and `core.pyi` are behind all of this (built and verified
on Linux only), and the per-object A/B of a compiled character against a prototype has no tool
of its own yet beyond `runCompiled.py` plus `diff`. One thing the Linux suite run found on the
way is NOT open any more: `parseIniReplaceVals` iterated `value.cast<PyReplaceList>().values()`,
a reference into a temporary that dies before the loop body -- MSVC tolerated it, GCC 13
segfaulted on every `ReplaceList` (see Architecture's pybind gotchas).

**The compiled port is confirmed in game (maintainer, 2026-09-13).**

## THE IDENTITY MOD IS THE EASY CASE IN FOUR SEPARATE WAYS (Bennett, 2026-09-15)

Bennett -> BennettAdventure passed on the identity mod and then failed on three real mods in a row,
each for a reason the identity mod structurally cannot have. Read this before concluding a remap
works:

| the identity mod | a real mod |
| --- | --- |
| draws through `drawindexed = auto`, so it has no count to go stale | carries its own `drawindexed = <count>, 0, 0` -- its WHOLE object, which after the split is more than that component's `.ib` holds |
| uses every vertex group and every material band | uses a subset, so a band or a bone that is wrong shows only here |
| 32-bit index buffers | may declare `DXGI_FORMAT_R16_UINT` |
| one variant, hashes from today's dump | several variants behind `$swapvar`, and hashes from whatever version its author dumped |

**The draw COUNT must be measured off the split buffer, never inherited.** `RegFillMissing` does not
fire on a register that is not missing, so a mod's own count survives the split. Measured: an Eye
draw asked for 9879 indices from a buffer holding 828 -- 9051 indices of whatever followed it,
rendered as mangled eyes.

**A mod carries whichever version's hashes its author dumped.** Bennett's moved three times inside
the library's window (draw_vb at 4.1, ib at 4.3, position_vb at 4.4); a 4.0-era mod says
`position_vb 993d1661` where today's model says `6cff51b4`. `data/HashData.cpp` holds every version
for exactly this reason -- a prototype that hardcodes the current one finds no position buffer and
skips the whole file.

**The GIMI mod merger writes a master `.ini` that binds nothing directly.** Its override sections
carry `run = CommandListBennettPosition`, with the real `vb0 =` inside a `$swapvar` branch, and that
master is the file the game loads. Resolve it with `IniSectionGraph` -- it builds the call graph
from a root section and follows every `run =` transitively, cycles included -- rather than writing a
second `run =` walker. One thing it cannot decide for you: a branch offers one resource per variant,
so taking the first remaps the mod from its first variant only.

**A 16-bit index buffer only ERRORS when its byte count does not divide by four.** One that does is
read as half as many wrong indices, silently. Read the `format` the Resource section declares; do
not infer the width from the file size.

<br>

## A TARGET COMPONENT NOTHING IS REMAPPED ONTO STILL DRAWS THE SKIN'S OWN GEOMETRY (2026-09-15)

None of Bennett's vertex groups map to BennettAdventure's `Bang`, because he has no hair bone -- his
hair and his face are both on his head bone. The conclusion drawn from that was to leave her Bang
unfixed so "she keeps her own bangs", and **it was wrong**: his hair IS drawn, as part of his head
object, by the Body component. Her bangs were redundant geometry sitting on top of his, two hair
meshes with two different white textures, which in game reads as *parts of the hair having different
shades of white*.

So: **hide every target component nothing was remapped onto** -- a `TextureOverride` on its ib hash
with `handling = skip` and no `drawindexed`, the same shape the fix already leaves on a component it
does remap. The observation (no groups map there) was right; only the conclusion was not.

**And "nothing was remapped onto it" is a RESULT, not a REQUEST.** The pass doing the hiding read
the prototype's `--components` list, which is what was *asked for*. A component can be asked for and
still come out empty, because the split selects by VERTEX GROUP and a mod is free not to use the
bones a component's row names. A HuoHuo-over-Bennett mod (2026-09-15) weights **nothing** to
Bennett's groups 1 and 2 -- which is the whole of the Eye row -- so its Eye cut was empty, no eye
buffer was written, and BennettAdventure's own eyes drew on top of HuoHuo's: a second pair of eyes
inside the first. Her Bang was hidden correctly in that same run, purely because the Bang is
excluded by the CLI, **which is what made the omission look like a working feature**.

Decide it by reading the OUTPUT: a component is drawn only if some `...RemapFix` section carries a
`drawindexed`. Collect that across **every** `.ini` of the mod, never per file -- GIMI merges them
all, so a hide written into one file suppresses a draw issued from another. And guard the whole
thing: if *no* component drew, the fix did not land, and hiding everything would turn a failed run
into an invisible model.

**And `match_first_index` does not land through the windowed pass on a target object that several
components MERGE onto.** Measured on the reverse direction: `body` (2 members, target index 9879)
came out 0, while the same edit with `--components Body` alone wrote 9879. Write it in the object's
own group pass instead. `tranquilToYelanFix.py` has the same defect and **cannot show it** -- its
only multi-member object is `head`, whose target index IS 0, so a write that never happened is
indistinguishable from one that did. Check that before transcribing either direction into C++.

<br>

<br>

## A REMAPPED SECTION MAY BIND ONLY WHAT THE TARGET'S SLOT BINDS (2026-09-15)

A remapped section inherits its `ps-t` lines from the MOD's section, and those describe the SOURCE's
shader. Bennett's body binds four (diffuse, light map, metal map, shadow ramp). BennettAdventure's
Eye slot binds **two** and her Body slot **three**. Carrying the surplus across is not a harmless
extra: the slot means something different to her shader, and **her own mod leaves it unbound on
purpose** so the GAME's textures serve it. Her eyes rendered as blank white until `ps-t2` and
`ps-t3` were dropped.

**The identity mod is the ground truth for this**, and it is the reason to build one before the
first remap: it is the target's own model as a mod, so its sections are the register layout the
target's shaders actually receive, confirmed in game. Read the layout off it; do not infer it from
the source, from a neighbouring component, or from what "ought" to be bound.

**A register bound TWICE in one section is the same bug wearing a disguise.** The later line
silently discards the earlier. On Bennett's Body the fix emitted

```ini
ps-t2 = ResourceBennettHeadLightMapBennettAdventureBodyLightMapRemapTex
ps-t2 = ResourceBennettHeadMetalMap
```

so the remapped light map -- the band move included -- never reached the GPU at all, and the band
work looked like it was shipping for four rounds. Keep the first binding per register and say what
was dropped.

**Buffer STRIDE is a property of the MOD, not of the character, and both directions of conversion
are real.** Vanilla Bennett's Texcoord is 12 bytes a vertex; a mod whose author carries a second UV
set is 20. Her Body is 20 and her Bang and Eye are 12. So one mod needs WIDENING onto her Body and
another needs NARROWING onto her Eye, and a pass guarded with `if (have >= want): continue` does the
first and skips the second in silence -- every UV read 8 bytes late, which renders as blank white
rather than as anything that looks like a stride bug. Measure the source stride off the mod's own
buffer (`size // vertexCount`); never infer it from the character's vanilla dump.

<br>

## A MERGED MOD'S DISABLED VARIANTS CARRY STALE HASHES (2026-09-15)

GIMI's hash-update tools skip any file named `DISABLED*`. In a merged mod that means only the
MASTER is ever brought current -- the per-variant files rot at whatever game version they were
merged at. Enabling one as-is, which is what picking a variant does, hands the game hashes it no
longer emits.

Measured: a Bennett mod's master carried `position_vb 6cff51b4` (4.4) and `ib cdc66323` (4.3) while
all four of its variants still said `993d1661` and `f51209fc` (4.0). The character's own geometry
then never matched, so **the mod rendered broken while the remap drew perfectly** -- the remapped
sections are keyed on the TARGET's hashes and are unaffected. That split symptom is the signature:
skin fine, original broken.

Reconcile the enabled file against the master, by section name, before fixing. **The master is the
authority, not the version table in `HashData`** -- Bennett's `draw_vb` history says `8b2a1582` was
superseded at 4.1, and this mod's working master kept `8b2a1582`. A table-driven refresh would have
"corrected" a hash that was never wrong. The file the game was demonstrably loading beats the table.

This corrects what the `--variant` note used to claim: picking a variant does **not** simply make a
merged mod "an ordinary single-variant one". It makes it an ordinary mod *with stale hashes*, which
is a different and worse thing, because it looks like it worked.

<br>

## Recipe: a classic-shape mod onto a multi-component skin (Bennett and after)

Every GI character from Bennett on is a skin of several components, so this is the shape the
next remaps take. In order, with what each step needs and where it came from for Yelan:

1. **Hashes and slots of the target, per component.** A frame analysis of the skin in game
   (`FrameAnalysis-<Skin>-<date>`), read with `Tools/VGRemapFinder`'s `DumpMod.fromFrameAnalysis`
   or `Tools/Misc/Diagnostics/boneCentroids.py --hashes`: each component's `position_vb` /
   `blend_vb` / `texcoord_vb` / `ib` / `draw_vb`, and the `match_first_index` of every object
   the game draws through it (its SLOTS -- Tranquil's Body had three, A / B / C). Which slot a
   mod should be drawn through is decided by the slot's shader: read the registers the game binds
   on it (a normal-map layout wants `ORFix`, the plain one `NNFix`). Note the face diffuse hash.
2. **The vertex-group rows, one per (source, component)**, through the finder with the target as
   a multi-component character (its README's "Characters of several components"), into
   `VGRemapData.cpp` as `("Src", "") -> ("Skin", "Comp")` rows plus the reverse rows. Then the
   identity mod of the SOURCE (`Tools/Misc/Prototypes/identityMod.py`) and `modTally.py
   --remap Src Skin Comp` to see every used group has an entry, and `boneCentroids.py` for every
   chain the finder mapped onto a limb (the jacket-flap lesson).
3. **The strategy per component.** Negative index for a component that should draw the whole
   mod trimmed to its own bones (Tranquil's Bang: the hair), graph cut for the rest. The
   prototype's `ComponentSplit.py` / the core's `VGComponentSplit` print the coverage: every
   source triangle drawn by exactly one component, or the strategies are wrong.
4. **The band legend of the target**, read off ITS textures at its vertices (a frame analysis
   dumps them), against the source's read off the source's identity mod. The table goes into
   the character's `liftBands`-style filter; keep the skin lift diffuse-conditional.
5. **Prototype it from Python first** (`Tools/Misc/Prototypes/yelanTranquilFix.py` is the
   template: a runtime `ModType` with pseudo targets, one fixer per component) until four mods
   look right in game -- the identity and three downloads -- then transcribe:
   `IniFixData/<Src>/<Src>Fixer.cpp` building a `GIMIComponentFixerConfig`, three (or however
   many) `ModTypeId`s as targets only, hash rows, `IniFixBuilderData` rows, and `runCompiled.py`
   against the prototype's output until every buffer is byte-identical.
6. **Then do the reverse direction**, which is its own template -- see the next section.

## The reverse direction is COMPILED TOO: a multi-component SOURCE onto a classic target (2026-09-14)

`YelanTranquil -> Yelan` is the **third fixer template**: `GIMIMergeFixerConfig` +
`makeGIMIMergeFixer(config)` in `data/IniFixData/GIMIMergeFixer.{h,cpp}`, with YelanTranquil's own
choices in `data/IniFixData/YelanTranquil/YelanTranquilFixer.cpp` and a
`makeGIMIComponentParser` row in `data/IniParseData/YelanTranquil/YelanTranquilParser.cpp`. One
`.ini` group out, always -- the target draws through ONE set of buffer hashes, so unlike the
split there is nothing to write a second file for.

Under it, in `core/`: `model/buffers/VGComponentMerge` (the inverse of `VGComponentSplit`),
`model/iniresources/VGMergeGroupResource` and `.../graphGroupEdits/VGMergeGroupResBuilder`, each
mirroring its split counterpart and reusing `VGComponentSplit`'s static codecs rather than
restating them. Everything the forward direction learned still applies -- read "Yelan is COMPILED
now" above first; what follows is only what the MERGE direction adds.

**Acceptance, on three real mods** (the identity mod, YelanOutfitRecolor, an NSFW edit): every
`TextureOverride` section of the compiled output equals the prototype's, with each bound resource
reduced to the md5 of the file it names, and on the third -- which the prototype cannot fix at all
-- the merged buffers are the three components laid end to end with the Eye's downloaded ones in
place. The A/B driver is in the session scratchpad rather than the repo; it is thirty lines and the
shape is in "Byte-identical buffers say nothing about the `.ini`" below.

### The remap graph needs the COMPONENT names as SOURCES, not only as targets

The first compiled run wrote **`hash = HashNotFound` into every section** while the buffers were
byte-identical. The cause is one line of the remap graph, and it will hit every later
multi-component skin the same way.

`ModMappedAssets::replace` is reverse-then-forward. It resolves the source hash back to the row
that owns it -- `("YelanTranquilBody", "ib")`, because a multi-component skin's rows are filed
under the COMPONENT names -- and then asks the mod type's own map what *that* name remaps onto.
`GIBuilder::makeRemapMap` built `{name: targets}` with the skin's name only, so the forward half
found no key at all and the whole remap returned nothing.

`ModTypeIdTools::getComponentIds(id)` now names a skin's components, and `makeRemapMap` files each
of them as a source alongside the skin. The forward direction needed no such thing, because there
the component ids are the *targets* and a target is named explicitly. **A remap graph is
directional and a multi-component skin needs BOTH halves spelled out.**

### Byte-identical buffers say nothing about the `.ini`

Worth stating flatly, because it cost most of a session: the first compiled run produced buffers
that were byte-for-byte the prototype's, and an `.ini` file with `HashNotFound` in all ten sections
AND the SOURCE's `match_first_index` in four of them. A mod like that is well-formed, loads
without a warning, and draws nothing.

The `.ini` needs its own comparison, and it cannot be a text diff: the compiled fix suffixes a
graph id onto every resource it generates (`...RemapBlend0_0_0_0` -> `..._B8g.buf`) where a
prototype names them by hand, and it wraps a collected register in `if 1 ... endif`. What works, in
about thirty lines: parse both remap blocks into `section -> [(key, value)]`, drop `if`/`endif`
lines, and **replace every value naming a `Resource` section with the md5 of the file that resource
names**. Two differently-named resources pointing at identical bytes then compare equal, and
everything that actually reaches the game -- the hashes, the indices, the registers, their ORDER
(the last binding of a register wins) and the bytes behind each one -- is compared exactly.

### A per-register download is wrong when the register is not fixed across mods

A download is keyed by `(mod object, REGISTER)` and fires when that ONE register is uncovered. That
works for the classic shape because every mod of a pre-6.x character binds its face diffuse to
`ps-t0`. A 6.x skin's mods split: of the three test mods, two bind `ps-t1` (the 6.x convention) and
one binds `ps-t0` (a port that kept the old one). Whichever register the parser registered, the
other half of the mods looked like they were MISSING a face -- and because the fixer then renames
the face diffuse onto the target's register, the download and the mod's own texture ended up as two
bindings of the same register, the download last. **The modder's face was silently replaced by a
stock one.**

There is no "either register" form of a download need (`GIMIParser::getDownloads` checks one at a
time), so `makeGIMIComponentParser` registers **no** face download at all, with a comment saying
why. Before adding a download, ask whether the register it keys on is the same in every mod of that
character.

### THE FIX LIBRARIES ARE INVOLUTIONS, SO THE RULE IS ONCE PER **PATH** (2026-09-14)

Read this before touching `NNFix`, `ORFix` or `RegDelimitedAdd` anywhere. It is not a
multi-component fact -- it moved both templates, and it had been wrong for every mod of a shape
nobody had tested.

The rule used to be "immediately before every `drawindexed`, and once at the end of a path that
draws nothing". That is right only while **no path draws more than once**, and silently wrong
otherwise, because these command lists do not *set* the registers -- they **re-slot the ones already
bound**:

```ini
[CommandListNNFix]                     [CommandListReferenceNoNormal]    [CommandListLDX]
run = CommandListReferenceNoNormal       ResourceDiffuse  = ref ps-t0      ps-t0 = ref ResourceLightmap
run = CommandListFixLogic                ResourceLightmap = ref ps-t1      ps-t1 = ref ResourceDiffuse
run = CommandListClear                   ResourceNormalMap = null          ps-t2 = ref ResourcePST2
```

It **swaps `ps-t0` and `ps-t1`**, and `CommandListClear` only nulls the `Resource*` refs -- the swap
stays. Call it twice over one set of bindings and you are back where you started, with the LIGHT MAP
sampled as the albedo: **the model renders flat green**.

Three independent sources agree on the placement that avoids it. Mod authors write one `run =` at
the top of a section, after the texture registers and before anything conditional, however many
draws follow. The pure-Python original never inserts one at all -- it renames the modder's own to
`tempNNFix` and back, which preserves wherever they put it. And measured over **33030** real
`TextureOverride` sections, **none** rebind a `ps-t` register after drawing, so the draw is the only
thing that can invalidate the call and no dataflow is needed.

`RegDelimitedAddMode::PerPath` is that rule, and `makeGIMICharFixer` / `makeGIMIMergeFixer` both pass
it now. Its four placement rules are in `RegDelimitedAdd`'s own description; the two non-obvious
ones:

- a section whose draws are **all** inside independent `if` blocks has no position inside any block
  that serves the paths through the others, so the call moves up to the end of the header content;
- and it must then NOT also land inside each block, which is what "once a part takes it, everything
  downstream is covered" is for.

**Why it never showed until now.** The shape that breaks it is `if` blocks that are INDEPENDENT --
`$feet`, `$ears`, `$scarf`, `$top` all on at once -- rather than an exclusive chain. Ganyu's
`$swapvar` branches are exclusive, and every compiled character's own test mod draws once per
section. Over one real mod library, **66 sections across 14 mod folders** draw several times in a
single pass.

**Two counting mistakes to avoid, both made here first.** GIMI spells an exclusive chain `if` /
`else if` / `endif` -- **two words, no `elif`** -- so a tally looking for `elif` reports every chain
as a single branch. And a checker that counts calls per SECTION rather than per PATH then reports an
exclusive chain's two calls as a violation. Only the per-path count means anything.

**The invariant is per path per BINDING GENERATION, not simply per path (2026-09-15).** Re-stated
after this library's own output tripped the coarse version: a call is undone by the next one only
while the registers underneath it have not moved, so **re-binding a `ps-t` resets the count**. The
merged head below emits one `ps-t0` / `ps-t1` / `run = NNFix` / `drawindexed` block PER MEMBER where
the members need different textures -- two calls on one path, both correct, because the second acts
on registers the first never saw. Three units, in order of increasing correctness:

| unit | verdict on an exclusive `if`/`else if` chain | verdict on per-member binding blocks |
| --- | --- | --- |
| per section | **wrong** -- reports it (two calls, one runs) | wrong -- reports it |
| per path | right | **wrong** -- reports it |
| per path per binding generation | right | right |

**Verification, if you touch this again**: fix a whole mod library into scratch copies and run
`Tools/Misc/Diagnostics/fixCallPaths.py` over the output -- 157 mod folders, **7851 remapped
sections across 867 fixed `.ini` files, 0 violations**. A unit test cannot see this one; it needs
real mods with real toggles. `core/tests/RegDelimitedAdd_PerPath_test.cpp` pins the six placement
shapes underneath it. That checker carries four synthetic cases it is proved against first (two
that must fail, two that must pass); the one that must NOT be reported is the per-member shape, and
an earlier draft of it reported three of five real mods until the distinction above was drawn.

### A TARGET OBJECT SEVERAL COMPONENTS MERGE ONTO IS NOT ONE DRAW (2026-09-14)

Yelan's head is Tranquil's `Bang` **followed by** her `Eye` in one merged index buffer. The merge
gets that right and the `.ini` file then throws the second half away, because a mod's own
`drawindexed` lines address ITS OWN buffer -- which is the first member's. They cover the `Bang`
exactly and stop where the `Eye` begins. In game the fringe renders and **the character has no
eyes**, with every buffer byte-perfect and every log line clean.

The fix appends one draw per member after the first -- `drawindexed = <count>, <offset>, 0`, the
offset being the running sum of the preceding members' index counts. Two things have to be true of
it, and **the second is the one that bit, twice**:

**(1) It must happen only when the mod DREW for itself.** A section the fix left with
`drawindexed = auto` already draws the whole merged buffer, members and all, and a second draw of
the later members would be a duplicate. The gate is read off the SOURCE section (`SlotFiles::draws`)
rather than expressed as an edit, because it is a fact about the mod and is known before any edit
runs -- every way of asking the graph instead has to run either before the fill, and so cannot see
it, or after, and so cannot tell `auto` from a real draw.

**(2) It must land where EVERY path reaches it, which is not "as late as possible".** The obvious
edit is a `RegSurroundedAdd` keyed on `drawindexed` with `latest = true`. That puts it at the latest
valid position -- which in a section full of toggles is **inside the last `if` block**. Yelan then
had eyes only while `$pubic == 1`: the same bug one layer down, shipped, and confirmed in game as
still-missing eyes. `RegFillMissingMode::BottomCover` is the placement that is right --
`addBottomContentPart` appends a fresh part at the section's own depth, outside every block. Since
BottomCover will not fire on `drawindexed` once the mod has drawn (its gate is "some root path lacks
the register", and none do), the draw is filled onto a register nothing reads and renamed to
`drawindexed` immediately after. That is the pure-Python original's own trick; it carries a
`tempDrawIndexed` for the same reason.

Two smaller things:

- **The counts are measured, not configured.** One YelanTranquil edit draws 10782 indices out of a
  `Bang` the game draws 7692 from. `Slot::indexCount` is only the fallback for a slot whose `ib` is
  a DOWNLOAD, and so is not on disk while the `.ini` file is being written -- same cause and same
  shape as the vertex count below.
- **It needs no fix call of its own**, now that the rule above issues exactly one per path.

**And a check that reads a `.ini` file must not `.strip()` before it decides.** The first version of
this fix's own checker asserted the appended range existed, was the Eye's, and came last -- all
true, all passing, with the draw sitting inside an `if` block the whole time. Nesting was the entire
question and stripping threw it away. The check that means something is the **depth** of the line,
tracked by counting `if` / `endif`.

### A mod that is MISSING a whole component

This is the shape's own failure mode, and it does not exist for a single-mesh character. The NSFW
edit has no `Eye` sections whatsoever. Two separate things had to be true before it worked, and
each looked like success on its own:

1. **The invented sections must carry a `hash`.** `GIMIParser` hangs a missing object's downloads
   off a `TextureOverride` it invents, and `objIdentityKVPs` is the hook that seeds it -- without
   it the section has no `hash`, which means no draw call matches it AND the merge's own file
   discovery (which walks the sections looking each one's `hash` up) cannot find it either. The
   measured symptom was four `RemapDL` resources fetched, written, referenced by nothing, and paid
   for. `makeGIMIComponentParser` answers it from the same tables its classifiers use, exactly as
   `makeGIMICharParser` does.
2. **`IniFile::fix` runs BEFORE `RemapService::fixResources`,** so nothing that needs a downloaded
   file's BYTES can measure it while the `.ini` is being written -- and the vertex count of every
   component is needed there, for `draw`, for `override_vertex_count`, and for the offsets every
   later component's index buffers are shifted by. A download is by definition the game's own
   buffer, so its length is a static fact: `GIMIMergeFixerConfig::Component::vertexCount` carries
   it, used ONLY when the file is not on disk. Without it the component is dropped and the merge
   comes out short by exactly its vertices -- 203269 where 203389 was right, a number that looks
   entirely plausible on its own.

The `RemapDL` sections being present in the output is NOT evidence the component was used. Check
the arithmetic: the merged buffer's length divided by its stride has to equal the sum of the
components' counts, and the last component's indices have to be shifted by the sum of the ones
before it.

### A `TextureOverride` BINDS REGISTERS ONLY FOR THE DRAW ITS HASH MATCHES (2026-09-14)

**The single most useful GIMI fact for debugging a remap, and it is not written anywhere in the
tool.** A `TextureOverride` section's `ps-t` lines apply to the draw call its `hash` matches and to
nothing else. So a mesh slot whose section binds no `ps-t` at all is not "unbound" or "black" --
**it renders with the GAME's own textures**, because the game's own binding is what the shader
still has. Whether a slot uses the mod's art or the game's art is decided by whether the mod
bothered to write a register line, and mods disagree about it constantly:

| mod | what it binds for the slot that lands on the target's head |
| --- | --- |
| the identity mod | every register, because it is the game's model as a mod |
| a recolour | every register, repainted |
| an NSFW edit | none for the `Eye` -- it has no `Eye` sections at all |
| a hair mod | `ps-t0`/`ps-t1` for the `Bang`, none for the `Eye` |
| a costume port | `ps-t0` only, on the 5.x convention, for a slot whose sibling binds `ps-t1` |

Two consequences, each of which was shipped as a bug first:

- **A textureless slot must be given the GAME's textures, not a sibling slot's.** The first fix
  "borrowed" the mod's own body texture for a textureless eye, which is right for exactly one mod
  and wrong for the four others -- their eye UVs address the game's eye atlas, not the mod's body
  one. `Slot::textureDonor` in `GIMIComponentParserConfig` names where a textureless slot's
  textures come from, and the answer is a download of the game's, not a peer.
- **A GATE on "the mod is missing the whole component" is an over-fit.** It was built, it made the
  NSFW mod correct, and the maintainer's next two downloads broke on it anyway: a mod can HAVE the
  component, draw it, and still bind nothing for it. The rule is per SLOT and has nothing to do
  with whether the component exists.

**How to find this out in two minutes, for any character**: do not reason about which textures a
slot "should" use -- read the mod's own `.ini` and tabulate it. Parse every `TextureOverride`, key
it by the slot its `hash` + `match_first_index` identify, and print which `ps-t` registers it
declares and whether it draws. One such table over five mods explained every failure that had been
reported, contradicted three of this session's diagnoses, and predicted a fourth bug nobody had
sent in yet. **The mod is the ground truth; this repo's model of it is not.**

### WHEN THE SYMPTOM IS ON A TEXTURE, CROP THE UV ISLAND AND LOOK AT IT (2026-09-14)

Eyes that drew but looked wrong produced three confident, mechanical, mutually exclusive
diagnoses -- the texture binding, then the material band remap, then the geometry -- each refuted
by a measurement within minutes of being written down. The maintainer's own hypothesis ("a scaling
factor, the texture is extra long") was also not a global transform, and that was measurable too:
the mod's own meshes span the same UV range as the game's.

What ended it was reading the mesh's `Texcoord.buf`, taking the min/max of the UV island for that
slot, cropping exactly that rectangle out of the `.dds`, and **looking at the image**. The iris is a
circle in PIXELS on an atlas that is twice as wide as it is tall, so the crop is a 2:1 rectangle
holding a circle -- which is correct, and which every numeric check had said nothing about.

Generalise it: `Images/` in this folder exists because a screenshot decides things arithmetic
cannot. **A texture bug is a picture; get a picture.** Read the Texture Editing guide's first
section for how to view a `.dds` at all (the Read tool cannot open one), and prefer a crop of the
island in question to a look at the whole atlas -- a 2048x1024 atlas tells you nothing and a
64x64 crop tells you everything.

### CHOOSING TEST MODS: VARY THE STRUCTURE, NOT THE CHARACTER (2026-09-14)

Four mods of the same skin can exercise one code path between them. What distinguishes a useful
set is the **structural axes of the `.ini` file**, and for a merge there are four that each broke
something:

| axis | the two sides | what only one side reaches |
| --- | --- | --- |
| has the component / does not | `Eye` sections present vs absent | the invented section, `objIdentityKVPs`, the configured vertex count |
| binds textures / does not | any `ps-t` for the slot vs none | the game-texture donor above |
| draws for itself / leaves `auto` | a `drawindexed` in the section vs none | the appended per-member draw, and its gate |
| repaints the atlas / keeps the game's | moved UV islands vs original | whether the download's coordinates agree with the mod's art |

Five mods covering those (the identity mod, a recolour, an NSFW edit with no `Eye`, a hair mod,
a costume port) is a better suite than twenty of one shape -- and the identity mod is the one to
ask for first, because it fixes every axis to "the game's own answer" and so gives every other mod
a baseline to differ from. **When the maintainer reports a regression, the first question is which
axis the new mod sits on the other side of**; twice this session the answer named the bug before
any code was read.

### THE BAND LEGEND IS A TABLE NOW, NOT A CLOSURE PER DIRECTION (2026-09-15)

`MaterialBandRemapFilter` (`model/strategies/texEditors/texFilters/`) is the light map band move for
ANY character pair: a list of `{source band (or range), target band, optional diffuse gate,
negate}`, handed to `MaterialBandRemapFilter::lightMapEdit(bands)`, which is exactly the
`std::function<TexEditor::Filter(const std::string&)>` that both fixer templates' `lightMapEdit`
field wants. So a character's legend is a table beside its config:

```cpp
const std::vector<MaterialBandRemapFilter::Band> Bands = {
    {255, 121, &MaterialBandRemapFilter::skinColoured},              // skin
    {0, 255, &MaterialBandRemapFilter::whiteFurColoured},            // white fur
    {115, 128, 0, &MaterialBandRemapFilter::skinColoured, true},     // hair: where NOT skin
};
config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(Bands);
```

It replaced two hand-written closures that were ~90% identical -- same diffuse nearest-neighbour
sampling, same pixel loop, `skinColoured` duplicated verbatim -- and **copies three and four were
already written**, stubbed in `Tools/Misc/Prototypes/bennettAdventureFix.py` and
`adventureToBennettFix.py` waiting for Bennett's two legends to be measured. Every GI character from
Bennett on is the multi-component shape and each needs a legend both ways, so this is the N=4 case,
not speculative generality.

**Three semantics it pins, each learned the expensive way:**

- **The moves are SIMULTANEOUS** -- every decision read from the ORIGINAL alpha. A real legend is a
  *permutation* of the bands, and applied in sequence a permutation chases itself: `0 -> 255`, then
  `255 -> 121`, and band 0 has landed on 121 through a band it was never meant to occupy. The
  first band whose range contains a pixel decides, and nothing downstream sees the value it wrote.
- **A diffuse that cannot be read makes every gate PASS.** The legend is the better of the two
  guesses when there is nothing to check against. See the drift warning below for why this is not
  as harmless as it sounds.
- **The gate exists because a mod that is a PORT carries a THIRD character's legend.** A move off a
  band that cannot be mistaken for anything else needs no gate; a move off band `0` -- the DEFAULT a
  lazy or ported mod leaves everything on -- always does.

`core/tests/MaterialBandRemapFilter_test.cpp` pins all of it, and its permutation case was **proved
to fail** against a deliberately sequential build before being trusted (habit 34). Acceptance: the
five test mods re-fixed and compared file by file against the previous build -- 183 files, all
byte-identical.

### A DOWNLOADED TEXTURE THAT DOES NOT LAND CHANGES THE BAND OUTPUT (2026-09-15, OPEN)

Found while A/B-ing the above, and it is **not** caused by it: **fixing one mod twice, with the same
build, produces two different light maps.** Measured on two of the five test mods, alternating run
to run:

```
run 1   YelanTranquilBodyADiffuseRemapDL.dds on disk: yes
run 2   YelanTranquilBodyADiffuseRemapDL.dds on disk: NO
        33312 of 1048576 pixels differ, and every one is a gate that would have REJECTED:
          band   0 -> 255 (fur)   29113 extra pixels moved
          band 255 -> 121 (skin)   4122 extra
          band 115-128 -> 0 (hair)   59 extra
```

The mechanism is exactly the second semantic above. The band gates read the object's diffuse; for a
component the mod does not supply, that diffuse is a DOWNLOAD; when the download does not land, the
gates all pass and the whole legend is applied unconditionally. In game that is a body shaded partly
as fur where it should be cloth -- and it depends on nothing but whether the fetch happened.

Two things follow. **(1)** Any A/B over a fix that edits textures must check whether the downloaded
inputs are present on both sides, or it will report a diff that is not the change under test -- the
clean comparison here was between two runs that both had the download. **(2)** The gate's
"unreadable diffuse passes" rule is right for a mod that genuinely has no such texture and wrong for
a download that merely failed, and the code cannot currently tell those apart. The download side is
where this wants fixing, not the filter.

### `drawindexed = auto` IS ONE DRAW WITH ONE BINDING (2026-09-14)

Stated separately because it is the trap under three of the above. `auto` draws the WHOLE buffer,
which after a merge is every component -- with whatever single set of registers the section binds.
So it is the right output only when the members agree on their textures **and** the mod drew
nothing of its own. Stack it on top of a mod's own draws and the mod renders both variants of
something it meant to toggle between; use it where members need different textures and the later
members get the earlier one's art.

The fix's own shape follows from that: fill `auto` only when the section draws nothing, and where
the members differ, emit one explicit `ps-t*` + `drawindexed <count>, <offset>, 0` block per member
instead. An `.ini` file in that state is correct and also unreadable at a glance -- check it with a
script, not by eye.

<br>

## What the reverse direction needed, for the next skin

Kept from when this section said the work was open, because it is still the checklist:

- **Exists.** `ModTypeId::YelanTranquil` with the `yelantranquil` keyword and a `GIBuilder`
  factory; the reverse vertex-group rows `("YelanTranquil", "Body"/"Bang"/"Eye") -> ("Yelan",
  "")` in `VGRemapData.cpp` (finder-made, half-checked); Yelan's hash / index / vertex-count
  rows and her download folder; the component hashes under `YelanTranquilBody` / `Bang` /
  `Eye`; the frame analysis of the skin on the maintainer's machine; the band legend of both
  skins (Creating Remaps' "The Yelan lessons", point 2).
- **Does not exist, and is the work.** (1) A PARSER for a multi-component mod: three sets of
  buffers, three ib hashes, objects keyed `(component, object)` -- `GIMICharParser` assumes one
  set, and the classifier's `IndexKey` already carries a component column for exactly this.
  Register the skin's hashes under `YelanTranquil` (with a component column, or three
  `HashData` name variants) so a Tranquil mod classifies as her. (2) The inverse of the split:
  MERGING three components' blends / positions / texcoords / ibs into Yelan's single set -- a
  grouped resource that is the mirror of `VGSplitGroupResource` (concatenate the vertex buffers,
  offset each component's ibs by the running vertex count, remap each component's bones through
  its own reverse row into Yelan's numbering), collected the same way through `ResGroupCollect`
  + `BufReplace`. Sentinel `-1` bones from the negative-index side must not appear on this side:
  a Tranquil mod's vertices are already skinned per component. (3) The register layout the
  other way: Tranquil's Body slot C and Bang carry a normal map on `ps-t0` that Yelan has no
  slot for -- drop it and shift down, exactly GanyuTwilight -> Ganyu (`objRegRemovals` +
  `objRegRemaps`), and re-issue `NNFix`. (4) The band table inverted (Tranquil 255 skin ->
  Yelan 115-127, Tranquil 0 fur -> Yelan 255, Tranquil 115-128 hair -> Yelan 0), still
  diffuse-conditional. (5) An identity mod of the skin to test on first -- `identityMod.py`
  refuses a multi-component `hash.json` today; extending it is the first task, and the
  YelanTranquil frame analysis plus its `hash.json` (copied to
  `Tools/Misc/YelanExperiments/YelanTranquil_hash.json`) are the inputs.
- **Reuse, do not re-derive**: the vertex-limit overrides, the skip-only ib section, the
  mipmapped texture writes, `BottomCover` for the draw call, the merge naming -- all in
  `GIMIComponentFixer.cpp`, and most of it wants lifting into a shared base with the classic
  template once a second config exists.

## Adding a `ModTypeId`: every place it enters (2026-09-13)

Missed one and the build is fine, the tests are fine, and the type quietly does not exist. In
order, for a type that is BUILT (a character a `.ini` can classify as):

1. `constants/ModTypeId.h` -- the enumerator, with a doc comment;
2. `constants/ModTypeId.cpp` -- `getEnum` (int -> id), `getName`, `getHashRemapTargets` (the
   fix-to graph; `getIndexRemapTargets` follows it), `getKeywords` (what a section name
   classifies by -- lowercase);
3. `py/src/constants/PyModTypeId.cpp` -- the `.value(...)`;
4. `constants/GIBuilder.{h,cpp}` -- a factory with the aliases, and the entry in `all()`; and
   `py/src/constants/PyGIBuilder.cpp` -- its `.def_static`;
5. the three builder tables -- a parse row (`IniParseBuilderData.{h,cpp}`), fix rows per target
   (`IniFixBuilderData.{h,cpp}`), a remove stub (`IniRemoveBuilderData.{h,cpp}`) -- and the
   counts in `core/tests/BuilderData_test.cpp`;
6. `data/HashData.cpp`, `data/IndexData.cpp`, `data/VertexCountData.cpp` rows (read the
   reverse-lookup note in "Yelan is COMPILED now" before filing an index row at a new version);
7. the oracles in `core/tests/ModTypeRemaps_test.cpp` and `core/tests/IniClassifierPopulation_test.cpp`
   (both list every built type; a row with NO keyword crashes the second test, and the
   population does not hold a keyword-less type either);
8. `core/CMakeLists.txt` for the new `.cpp` files.

**Four suites hardcode a count that steps 4-7 move, and nothing builds them**, so budget a pass over
all four rather than only the one you remember (all measured 2026-09-13):
`BuilderData_test.cpp` (rows per builder table), `VertexCounts_test.cpp` (**44** rows -- one per
step 6's `VertexCountData.cpp` entry), `VGRemaps_test.cpp` (**58** rows, one per *direction* per
*component*) and `ModTypeRemaps_test.cpp` (a **45**-row oracle). Step 4 also moves the GI mod type
count itself, which several doc comments restate --- see the next note. Yelan moved all of these and
nobody noticed for a day.

**The C++ tables are AHEAD of the pure-Python ones now, deliberately -- do not "resync" them.**
`data/VertexCountData.cpp` has 44 rows against `VertexCountData.py`'s 43; `data/VGRemapData.cpp` has
58 against `vgRemapDataBuilder.build()`'s 52; `GIBuilder::all()` builds 45 mod types against
`ModTypes.getAll()`'s 43. The gap is Yelan / YelanTranquil, which were only ever compiled into C++.
So when a comment or docstring says "43", read *which side it is talking about* before touching it:
three of the four "all 43"s in `core/src/constants/GIBuilder.cpp` describe the *pure-Python*
`GIBuilder` and are still correct. A number "corrected" by regenerating from the Python side
silently deletes a character.

A type that is a TARGET ONLY (a boss, a skin's component) takes steps 1-3 and 6 only: no
factory, no `all()` entry, no keyword, no remove row, no oracle row.

<br>

## Start here: adding a character, in order

Forty-four characters are done, in five *shapes*. **Work out which one you have first, because
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
8. **Update the hardcoded counts in `core/tests/`** -- `BuilderData_test.cpp` (rows per builder
   table), `VertexCounts_test.cpp`, `VGRemaps_test.cpp` and `ModTypeRemaps_test.cpp`. Nothing builds
   any of them, so adding a character silently breaks all four; see the count/divergence notes under
   "Adding a `ModTypeId`" above before you edit a number.

**Before writing anything, find out what already exists for this character.** Four places, and
each can save an afternoon:

| where | what you get |
| --- | --- |
| `Testing/Integration Tester/.../APIDocsTests/expected_*/` | **a full golden `.ini` for some characters** -- Raiden, Amber, AmberCN, Jean and Keqing all have one. That is a free specification: exact section names, hashes, indices, which sections get which edit. Read it before guessing |
| `api/src/py/FixRaidenBoss2/data/Ini{Parse,Fix}BuilderData.py.txt` | the character's pre-migration row -- reference only, but it says what the fix *used* to do |
| `data/HashData.cpp`, `IndexData.cpp`, `VGRemapData.cpp` | whether the asset data is already in (it usually is, across several game versions) |
| `Data/RemapDrafts/<Name>RemapDraft.xlsx` | the maintainer's hand-made vertex group remap, with the reasoning per row in its Comments column. Some early workbooks have one direction only; the CN skins, Kirara, Raiden and Arlecchino have none. Every workbook opens with a `Credits` sheet: edit one and, once you have joined The Council, credit yourself there (the drafts' `README.md` has the row format). See [Vertex Group Remaps](../VGRemaps/CLAUDE.md) |
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

Forty-four characters are done (`data/IniParseData/<Name>/` + `data/IniFixData/<Name>/`, one
directory each -- counted 2026-09-13), and FORTY-THREE of them go through a template rather than
being copied -- everything from the plainest CN skin (Amber, Mona, Rosaria) through the three-way
merge (ShenheFrostFlower) to the ones that edit textures conditionally and shift a `Position.buf`
(AyakaSpringbloom, CherryHuTao, XianglingCheer). Only Raiden, whose remap keeps the source
geometry and hides the originals, is written by hand:

- `data/IniParseData/GIMICharParser.h` — `makeGIMICharParser(GIMICharParserConfig)`
- `data/IniFixData/GIMICharFixer.h` — `makeGIMICharFixer(GIMICharFixerConfig)`

All 43 use `makeGIMICharParser`; 42 of them use `makeGIMICharFixer`, and the forty-third (Yelan)
uses its multi-component sibling `makeGIMIComponentFixer` on the fix side while still taking the
classic parser -- see "Yelan is COMPILED now" above. So "one template" is one *parser* template and
two *fixer* templates, and a new multi-component skin picks the fixer, not the whole shape.

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

### Proving a NEW download folder, without a golden to compare against (2026-09-14)

The conversion from a character's asset dump to its `Data/Mod Downloads` folder is
`Tools/Misc/Prototypes/identityMod.py`'s, minus the `.ini`: the vb0 dump's vertex splits into
`Position.buf` (40 bytes) + `Blend.buf` (32) + `Texcoord.buf` (measured, 12 or 20), and each
object's ib is written as R32_UINT whatever the game's own format was. A skin of several components
gets one buffer set per `hash.json` entry with a `position_vb`, named `<Prefix><Component><Part>`;
a classic character's component name is the empty string, so one loop writes both shapes.

**A new character has no golden, so prove the pipeline on the characters that do.** Rebuilding six
shipped folders from their sources reproduced them byte-identically -- Diluc, Amber, Klee (10 files
each), Ganyu (13), Yelan (16) and, for the multi-component path, YelanTranquil (23). That is the
check that makes the seventh folder trustworthy, and it costs one `--check` run per character.

**Then verify the new folder a second way, not the same way twice.** Re-running the same reader
proves nothing. Parse the dump text by hand -- no `VbFile`/`IbFile` -- rebuild the bytes and compare:
that caught nothing on Bennett (all 18 binaries matched, including the R16 -> R32 index conversion)
but it is the only check that could have. The `.dds` files are straight copies, so md5 them against
the asset folder.

Two things measured on Bennett that are not guessable:

- **The Texcoord stride is per COMPONENT, and differs inside one skin.** BennettAdventure's `Body`
  is 20 and its `Bang` is 12; YelanTranquil's Bang is 20. Carrying a neighbour's number over is
  wrong.
- **A component with an empty `texture_hashes` list borrows another's**, and gets no texture files
  of its own. BennettAdventure's Bang and Eye both do, exactly as YelanTranquil's do.

And the version folder is the one thing here that the data cannot tell you: base characters are all
filed under `4_0` (verified -- `4_0` holds the CURRENT dump even for characters whose geometry
hashes moved later, as Bennett's did at 4.1 / 4.3 / 4.4), while a skin takes its own release
version. **Ask which one.** The dump files' dates are suggestive and not decisive.

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

**SIXTEEN of the forty-four fixers carry a `texEdits` now** -- Arlecchino, Ayaka, AyakaSpringbloom,
CherryHuTao, DilucFlamme, Ganyu, HuTao, Jean, JeanCN, Keqing, KeqingOpulent, Kirara, Klee,
KleeBlossomingStarlight, Ningguang, Xiangling -- so the config route below is the one to reach for;
the hand-built collector after it is for a fix the config cannot express. SEVEN of them also
`texAdds` a texture the mod does not have at all (Ayaka, Ganyu, HuTao, Kaeya, KiraraBoots, Lisa,
Xiangling), and two carry a `positionEdit` (Xiangling, XianglingCheer).

*(Those three lists are worth re-deriving rather than trusting: this paragraph previously said
thirteen and named JeanSea and NingguangOrchid, neither of which has ever carried a `texEdits`.
`grep -rln "config.texEdits" core/src/data/IniFixData/` settles it in one line.)*

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

**And a save driven from PYTHON used to throw the correction away (fixed 2026-09-12).** The
pure-Python `TextureFile.save` re-applied `info["gamma"]` unconditionally, and that key is set by
nobody -- so `self.gamma = None` erased what the C++ `open` had just read off the DX10 header,
and a `BC7_UNORM_SRGB` diffuse edited through a Python `TexEditor` came back out untagged AND
uncorrected: measured 161.9 -> 161.9 mean on Yelan's head diffuse where the correction gives 117.4.
Every compiled character goes through the C++ `TexEditorReplace`, which never touches that
method, so no A/B saw it; the Yelan prototype did, as a visibly wrong head in game. It now only
overrides when the metadata names a gamma. Two corollaries from the same afternoon: **the Pillow
engine cannot do this at all** (it reads no DX10 header, so it neither knows the source is sRGB
nor corrects -- fine for a lightmap, wrong for a diffuse; use the Compressonator engine for
anything that was sRGB), and **a texture the fix CREATES to stand in for an sRGB one has to be
authored pre-corrected**, because `TexCreator` writes it untagged: Tranquil's flat normal map is
127/127/255 under an sRGB header, so the created one holds `round(255 * (127 / 255) ** 2.2) = 55`.

**And a written texture carries no mip chain unless asked (2026-09-12).** Every texture the game
ships has its full chain; ours shipped one level, which on hair reads in game as scattered
off-colour pixels. `TextureFile.save(mipmaps = True)` / `TexEditor(mipmaps = True)` /
`TexCreator(mipmaps = True)` write the chain; the default is unchanged. See
[Texture Editing](../TextureEditing/CLAUDE.md)'s mip section.

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
  `.ini` file naming a texture that was never created. From Python it does write: `TexReplace(
  resModObj, editor, fixFunc = f)` where `f` runs a Pillow-engine `TexEditor` over
  `resource.srcPath` into `resource.fixedPath` is how `yelanTranquilFix.py` edits its textures.
  The file is named per SOURCE texture (`YelanHeadDiffuseRemapTex.dds`, no target in it), so one
  edit of one texture shared by several targets is written once and referenced by all.
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

1. **Read the "unmapped source groups" line first.** `Tools/Misc/Prototypes/overrideVgRemap.py
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
- **`ResGroupCollect` had exactly that seam until 2026-09-13** -- the first grouped fix driven
  from C++ (Yelan's) collected every buffer and built nothing, and the only tell was `blend:
  fixed 0` in the stats. It has its own `editFromIni` now, and a C++ caller hands it a
  `VGSplitGroupResBuilder` (the core-side `GroupedResBuilder`; the binding's is `PyGroupedResBuilder`).
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

### A REGISTER SHIFT IS UNCONDITIONAL --- `RegValChecks` reads a NAME, not a texture (2026-09-12)

A row that shifts registers down a slot removes one at the top and renames the rest. It is
tempting to guard each rule on what is bound, so that a mod "already in the target's layout" is
not shifted twice:

```cpp
// DON'T. Every check here is reading a string the modder chose.
config.objRegRemovals = {{"head", {{"ps-t0", &RegValChecks::isNormalMap}, "ps-t3"}}};
config.objRegRemaps   = {{"head", {{"ps-t1", {{"ps-t0", &RegValChecks::isDiffuse}}, true},
                                   {"ps-t2", {{"ps-t1", &RegValChecks::isLightMap}}, true}}}};
```

**`RegValChecks` tests the RESOURCE NAME** --- `isDiffuse` is `containsIgnoreCase(val,
"diffuse")` and nothing more. A name is a label the modder picks, and a mod ported forward
keeps its old labels long after the content behind them has moved. LisaStudent1 binds `ps-t0`
to a section called `ResourceLisaStudentHeadDiffuse` that holds her **normal map**. Guarding on
that name does not detect "already shifted"; it detects what somebody typed.

This cost a full cycle and a wrong bug report. The guarded row left `ps-t0` in place on every
LisaStudent mod, the output was read as "the fix is deleting the diffuse", and a second guard
was added to "fix" it --- when the unguarded shift had been right all along and the resource
names were the only thing saying otherwise.

**The register POSITION is the contract.** A character's row says what slot the source binds
each map to; shift on that, unconditionally, the way the pure-Python rows always did. Reach for
a `RegValChecks` guard only where the *value* genuinely decides something the position cannot ---
a texture edit that should only run on one of two things a slot may hold --- and never to infer
what a texture IS.

<br>

### Three things the merge and the split can do that most characters never need (2026-09-12)

**1. A merge can put one source object in BOTH generated `.ini` files.** The pure-Python rows
spell it by listing the source twice --- `{"head": ["head", "head"]}` --- and here it is
`{"head", {"head", "head"}}`: the first target claims the main group, the second collides into
the next, which is exactly `objSplits`' documented first-claim rule doing the work. Diluc and
Fischl both need it; Lisa and Keqing do not, and their head appears only in the first file. The
symptom of getting it wrong is the second overlapping `.ini` drawing a body with no head --- and
**no section-name diff can see it**, because the head section exists either way, just in one
file instead of two. Count the sections PER FILE.

**2. A split can name a target object that neither character's parser lists.** Kaeya draws
three objects and has FOUR indices: `IndexData` gives him an `extra` at 47727 that no Kaeya
`.ini` file declares. It exists only as the far half of KaeyaSailwind's dress split
(`{"dress", {"dress", "extra"}}`). So when a split's target list looks longer than the target's
`drawnObjs`, check `IndexData` before assuming it is a typo --- the index table is the authority
on what a model draws, not the parser config.

**3. `objRegRemovals` takes keys that are not registers.** KaeyaSailwind -> Kaeya strips
`ResourceRefHeadDiffuse`, `ResourceRefHeadLightMap` and `$CharacterIB` --- 3dmigoto's
reflection-support keys, which a mod writes beside its bindings and which name the SOURCE's
slots. Carried across unchanged they point the reflection pass at the wrong textures. They go
through the same `objRegRemovals` as a `ps-t0`; the field removes by KEY and does not care that
the key looks like a register.

One asymmetry worth knowing because it looks like a bug and is not: that row strips those keys
from head, body and dress but **not** from `extra`, and the pure-Python table defines a
`ReflectionExtraRemove` it then never uses. Transcribed as-is rather than corrected.

<br>

### A SPLIT's second copy is a separate target, and the register edits are keyed by target

`objRegRemovals` and `objRegRemaps` are keyed by TARGET object. A split makes a second copy of
one source graph under a different target name --- LisaStudent's `body` becomes Lisa's `body`
**and** `dress` --- and that copy arrives with the same registers bound the same way. Name only
the first and the shift applies to half the model:

```cpp
config.objSplits      = {{"head", {"head"}}, {"body", {"body", "dress"}}};
config.objRegRemaps   = {{"head", ...}, {"body", ...}, {"dress", ...}};   // all THREE
```

The pure-Python rows do not need the third entry because `preRegEditOldObj` makes the edit run
before the split, so both copies inherit it --- which is the same flag that makes Klee's texture
edit land on her dress. Transcribing such a row into this template means writing the copy out
explicitly. Nothing catches it: the section NAMES are all present, the references all resolve,
and only reading the two copies' bodies side by side shows one shifted and one not.

<br>

### Widening what CLASSIFIES runs fixer rows that have never run (2026-09-12)

The bug above had been in the tree since the morning and no A/B could see it, because the only
mod that would have exposed it was classifying as the wrong character and never reached that row
at all. Teaching the classifier to read hashes fixed the classification, and the very first run
afterwards produced the broken head.

So: **a classification change is a fixer change.** Every mod that starts classifying differently
is now being run through a row that no test, no A/B and no in-game session has ever exercised on
it. Re-A/B each one individually, read the section bodies rather than the section names, and
expect to find something --- these are by definition the inputs the row was never written
against. The old script is no help here either: it classifies by name, so on exactly these mods
there is no reference output to compare to.

<br>

### KNOW WHICH CHECK YOUR SUMMARY LINE IS (2026-09-13)

`--ab`'s run prints two comparisons that are easy to conflate, and one of them is the line that
looks like a score:

```
--- remapped section names, old vs new ---     <- a diff of the section NAME lists
--- generated binaries, old vs new ---
  4 identical, 0 differ, 8 only-old, 10 only-new   <- cmp_binaries.py: the .dds and .buf FILES
```

That `N identical, N differ` line is about the **files a fix writes**, not about sections. It is
the stronger check of the two --- a texture that comes out with different pixels shows up here
and nowhere else --- but reporting it as "the sections match" claims something it never tested,
and hides that the section diff is printed separately just above. Read both, and name the one
you mean.

<br>

### THE OLD SCRIPT'S `--version` IS THE VERSION FIXED **TO**, AND IT DRIVES FOUR THINGS (2026-09-13)

`FixRaidenBoss6.py`'s `--version` becomes `IniFile.version`, and one value then selects **all**
of: the parser row, the fixer row, the version-keyed hashes, and the indices. This checkout
splits that in two, because they are genuinely independent -- the ordinary case is a mod written
for an old game version being fixed with the newest fix, which one option cannot express:

| knob | option | selects |
| --- | --- | --- |
| `RemapService::toVersion` | `--version` / `-v` | the FIXER row. `IniFixBuilderData::repo` is keyed `{fromVersion, fromMod, toVersion, toMod}` and every shipped row is keyed from `1.0`, so this half alone picks it. **This is the pure-Python API's `version`** |
| `RemapService::fromVersion` | `--fromVersion` / `-fv` | the PARSER row, the remover, and the hashes/indices the mod is read with |

So **reproducing the old script at version X means passing BOTH**: `--version X --fromVersion X`.
Passing one alone compares a fix built for X against hashes looked up for the latest version, and
the resulting divergence is a bug in neither script. `scratchpad/ab_ver.sh` does this for you.

Two notes for anyone reading an older comment in this tree. Until 2026-09-13 `RemapService` had no
`toVersion` at all and `createIni` handed `IniFile` a hardcoded `std::nullopt`, so `--version`
could select a parser row and *never* a fixer one -- every run got the newest fix whatever was
asked for. The first repair, one option setting both halves, was no better: it made every A/B at a
historical version uninterpretable, since a divergence could be coming from either selection.

<br>

### TRANSCRIBING A ROW: THE FOUR WAYS IT GOES WRONG AND STILL BUILDS (2026-09-13)

All 59 historical rows are implemented now -- every `4_0` through `5_7` entry, and the two
`giDefault`s. They produced four distinct classes of mistake between them, and **not one of them
was a compile error or a crash**. Each built, ran, reported success, and wrote output that was
wrong in a way only the A/B could see. In the order you are likely to hit them:

**1. The FIXER CLASS decides how to read the object map, not the map's shape.** The identical
literal `{"body": ["body", "dress"]}` means opposite things:

| fixer class | reading | means |
| --- | --- | --- |
| `GIMIObjSplitFixer` | SOURCE -> targets | this source's body becomes the target's body and dress |
| `GIMIObjMergeFixer` | TARGET <- sources | the target's body is fed by this source's body and dress |

`lisa4_0` and `lisaStudent4_0` have near-identical maps and opposite classes. Reading Lisa's
merge as a split inverted her whole remap and emitted a dress apparatus the old script never
writes -- 35 only-new sections, and it ran perfectly.

**2. An object OMITTED from a merge map goes into EVERY generated file.** Naming it explicitly
once is what restricts it to the first. `jeanSea6_1` names its head; `jeanSea4_0` does not mention
it at all. Since `objSplits` here is all-or-nothing, "in both files" has to be spelled out as two
entries: `{"head", {"head", "head"}}`.

**3. An EXPLICIT repeat is lost the same way, in the inversion.** `ayakaSpringbloom5_6`'s
`{"head": ["body", "body"], ...}` feeds target head from source body TWICE, so inverting to the
source-keyed form gives source body THREE targets. `shenheFrostFlower5_7` takes it further --
`{"head": ["head", "head", "head"], "body": ["head", ...]}` gives its head FOUR.

(2) and (3) are the same rule reached from opposite directions, and both produce **exactly one
missing section in the second generated file**. That is invisible to `cmp_binaries` -- 0 differ
throughout -- and invisible to the section-NAME check, because the section still exists globally,
just in the wrong file. Only the per-file body comparison sees it.

**4. The FIXER row says what is EDITED; the PARSER row says what EXISTS.** This one bit twice on
the same character:

- `ayaka4_0`'s `RegTexEdit` names only REGISTERS (`{"BrightLightMap": ["ps-t1"], ...}`). Which
  object owns each edit is in her parse row -- head/ps-t0, body/ps-t1, dress/ps-t0. Reading the
  fixer row alone put all three on the head: three edits of one texture, body and dress untouched.
- `drawnObjs` is the parse row's object set, `[{"head", "body", "dress"}]` -- **not** the objects
  the fixer row happens to name in its removals, which for all four of her historical rows is just
  head and body. Getting it wrong dropped her dress remap entirely.

The first of those failed to compile only by luck, because the helper names happened to differ.
With matching names it would have built clean. **A character's fix is the PAIR of rows; reading
one of them is not transcription.**

<br>

### A ROW CAN GET SIMPLER WITH THE VERSION, SO DO NOT INTERPOLATE (2026-09-13)

The tempting shortcut across 59 rows is to derive one from its neighbour. It works exactly where
the pure-Python rows say it does and nowhere else:

- **`5_0` really is `4_0` with one thing changed** -- `TexFx` renamed its transparency
  sub-commands at 5.0, `T.0`/`T.1` becoming the Natlan `TN.0`/`TN.1`. That rename is the entire
  difference between the `...4_0` and `...5_0` value-rename constants.
- **`5_7` really is "the draw call moved"** for eight characters -- `IbRemapData` +
  `IbDrawIndexedRename` + `IbTempToDrawIndexed` + a postModel `drawindexed` removal, which is one
  `moveDrawIndexed` here. Those eight were GENERATED from their `4_0` rows rather than retyped,
  because the bodies are register lists and retyping one is how a register goes missing.

And then it stops. `lisa5_7` is three removals and a merge -- the register shift, the invented
normal map and the `TexFx` re-issue that `lisa5_4` carries are **gone** by 5.7. `nilouBreeze5_7` is
three removals, full stop. `ayaka5_7` drops the `ps-t0 -> ps-t1` duplication that `ayaka5_6` has,
and then **`ayaka6_1` puts it back**. A row is not a monotonic accretion of its predecessors, so
reading either neighbour and interpolating gives a wrong row that builds.

<br>

### THE OLD SCRIPT CRASHES ON ONE ROW, AND THE A/B READS THAT AS OUR BUG (2026-09-13)

`FixRaidenBoss6.py` raises on `kiraraBoots5_7`:

```
TypeError: _regValIsOrFixWrapper() missing 1 required positional argument: 'part'
```

It fixes **0 of 1 mods** and writes no remapped sections at all. The A/B then reports every
section our row produces as `only new`, which reads exactly like over-production. `kirara5_7` uses
the same guarded-remap machinery and runs fine on both sides, so there is nothing structural to
infer from it either.

**A one-sided A/B is not a pass and not a fail -- it is no measurement.** Read the old side's log
before interpreting a lopsided only-new count; `fixed 0 mods and skipped 1` is right there in the
summary. That row is marked transcription-reviewed-only in the code, which is the honest status
and the one the next reader needs.

<br>

### `defaultFactory()` IS SOMETIMES THE ANSWER, NOT A STUB (2026-09-13)

Both tables' `giDefault` returned `IniFixBuilder::defaultFactory()` / 
`IniParseBuilder::defaultFactory()` and sat on the to-do list for months because that is what every
genuine stub looked like too. Their pure-Python bodies are `(GIMIFixer, [], {})` and
`(GIMIParser, [], {})`, and those factories build exactly that -- a real `GIMIFixer`/`GIMIParser`,
not a bare base. **They were the implementation.** `giDefault` is what Raiden falls back to at 4.0
and ArlecchinoBoss at 4.6.

The lesson generalises past this one function: a line that is indistinguishable from a placeholder
will be read as one indefinitely. Both now carry a comment saying why they are correct.

<br>

### A HISTORICAL ROW HAS TO SWITCH OFF WHAT THE TEMPLATE DOES BY DEFAULT (2026-09-13)

`makeGIMICharFixer`'s defaults are **6.1-era**. Three of them are things the pure-Python 4.0 rows
do not do, and none of them shows up in a section-name diff or a binary comparison:

| default | what a pre-6.x row sets | why |
| --- | --- | --- |
| re-issues `NNFix` for every drawn object | `objFixCalls` with an **empty list per target** | the whole NNFix/ORFix layer postdates these rows |
| `swapFaceRegs = true` | `false` | the swap corrects something GI 6.x did to the shader; at 4.0 the diffuse still belongs on `ps-t0` |
| `removeSrcFixCalls = true` | `false` | the strip exists because the fix re-issues those calls; a row that re-issues nothing just **deletes the modder's line** |

The third is the subtle one, and it is a **framework-versus-row** mistake worth recognising
generally. In the pure-Python original, dropping the mod's own `ORFix`/`NNFix` calls is *per-row
configuration*: every 6.1 row carries its own `RegRemove(*cls.ORFixCompleteRemoval)` and no 4.0
row does. This template folded it into the framework as an unconditional step -- correct for all
forty-four current characters, and silently wrong for any row that does not re-issue. It showed
as `run = CommandList\global\ORFix\NNFix` present in the old script's `--version 4.0` output
for RosariaCN's head and absent from ours.

**When the template does unconditionally what the source table did per row, the fix is a flag,
not a smarter rule.** The tempting repair here was to derive the strip set from `objFixCalls` --
"strip exactly what you re-issue", which is more principled, matches the rule already used for
`TexFx`, and is what the surrounding comment says. It is also not equivalent, and the difference
is invisible unless you measure it: see the next section.

<br>

### MEASURING A CHANGE TO SHARED FIXER CODE: THE A/B CANNOT DO IT (2026-09-13)

`GIMICharFixer` is shared by all forty-four characters, so any edit to it needs a regression
check -- and **the A/B against the old script is the wrong instrument**. Several divergences from
that script are deliberate and permanent (the face register swap, the reworked placement of the
re-issued draw call and libraries), so `0 of 5 shared .ini files identical in shape` is the
*expected* reading both before and after your change. It cannot tell you which differences you
just caused.

Run **this checkout against itself**, across a spread of mods, with and without the edit:
`scratchpad/batch9/newside.sh <tag>` fixes a fixed list of twelve, concatenates every `.ini` it
writes into one blob per mod, and a plain `diff` of the two tags then attributes every changed
line to the edit and nothing else. Budget two rebuilds (`git checkout --` the one file, build,
capture, restore, build, capture); the link alone is about five minutes here.

That check is what caught the "principled" version of the fix above. Twelve mods, and it moved
two of them:

```
SAME    Arlecchino, AyakaSpringbloom, Ganyu, Jean, JeanSea, Kaeya,
        KaeyaSailwind, Keqing, KiraraBoots, Xiangling
CHANGED CherryHuTao:    0 removed, 2 added   (+2 run = ...ORFix\ORFix, where the old script writes 4 and we now wrote 6)
CHANGED GanyuTwilight:  1 removed, 2 added   (+1 ORFix, and TexFx\TN.0 MOVED as a knock-on)
```

Ten of twelve unchanged is exactly what a wrong-but-plausible change looks like here. The flag
version leaves all twelve byte-identical.

<br>

### A COUNT OF `NNFix` IN THE OUTPUT IS NOT EVIDENCE ABOUT THE FIXER (2026-09-13)

**The fix copies a section's body verbatim into the remapped section.** So a mod whose own `.ini`
already calls `NNFix` has those lines in the output at every version, from every fixer row,
including a row that does nothing at all.

This cost most of a day. Checking the historical `4_0` rows, `RosariaCN` appeared to prove the old
script *ignored* `--version`: 10 `run =` lines at `--version 4.0` and 10 at its default, where
`rosariaCN4_0` is `(GIMIObjRegEditFixer, [], {})` and should have emitted none. The conclusion
written down was "the A/B oracle is unreliable" and the next 59 rows were held back on it.

The oracle was fine. `RosariaCN1`'s **unfixed baseline** already contained 8 `NNFix` and 8 `ORFix`
calls. Old-script-against-itself, 4.0 vs default:

| | Amber | Barbara | Diluc | MonaCN | RosariaCN |
| --- | --- | --- | --- | --- | --- |
| `NNFix` @4.0 / @default | 0 / 8 | 0 / 24 | 0 / 3 | 0 / 2 | **8 / 8** |
| in the unfixed baseline | 0 | 0 | 0 | 0 | **8** |

The flag worked on `RosariaCN` too: its real 4.0-vs-default difference was two hashes and one
`run` line **moved** -- which a count cannot see in either direction.

Three rules out of it:

- **Diff, never count.** `ab_ver.sh` prints the baseline's own `NNFix`/`ORFix` totals next to the
  run's for exactly this reason, and the whole-text diff is the line to read.
- **Ask the source directly when you can.** Whether the old script honours a flag is answerable
  from its own tables in seconds -- `scratchpad/batch9/probe_oldver2.py` imports
  `FixRaidenBoss6.py` and prints, per mod type, which row each version resolves to. That is a fact
  about the selection; a run's output is that fact plus everything else the fix did.
- **Compare the old script to ITSELF before comparing it to ours.** `old_ver_ab.sh` runs it at two
  versions with no new-side involved, which separates "the flag did nothing" from "the flag worked
  and the two rows agree" -- indistinguishable in a three-way comparison.

<br>

### `skipped` in the download summary means two different things (2026-09-13)

The run's last line reads like a tally of work done:

```
downloaded 1 files, copied 1 files from existing downloads and skipped 3 downloads
```

**`skipped` counts both "already had it / did not need it" and "could not fetch it".** A github
DNS outage produces `skipped 1` and so does a redundant request that was correctly elided; the
same Arlecchino mod printed `copied 4, skipped 0` on one run and `copied 1, skipped 3` on the
next, both self-consistent and both with zero dangling references. So a clean-looking `skipped N`
is **not** evidence the downloads succeeded. Check `check_dangling.py` for whether the files a
fix references actually exist, and grep the log for `Attempt .* failed` if you need to know
whether the network was the reason --- this is the counter-that-cannot-be-wrong trap from the
root `CLAUDE.md`, still live in this one line.

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
