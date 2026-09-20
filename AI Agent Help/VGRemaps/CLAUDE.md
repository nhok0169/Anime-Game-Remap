# Vertex Group Remaps

The blend-weight table behind every remap: which of the **source** character's vertex groups
(bones) becomes which of the **target**'s. It is what `*RemapBlend.buf` is built from, and it is
the one part of a remap that no `.ini` check can see --- get it wrong and the fix reports success,
the mod loads, and the model deforms in game. Written 2026-09-09 by the first agent to work in
this area, after building `Tools/VGRemapFinder`, fixing issue #213 with it, and sweeping every
gap out of the shipped table. Read this before touching `VGRemapData.cpp`, `Data/RemapDrafts/`,
or a "the model is warped / kinked / exploded" bug.

See [Creating Remaps](../CreatingRemaps/CLAUDE.md) for everything *around* this step (parser,
fixer, hashes, the A/B loop) and [Buf Files](../BufFiles/CLAUDE.md) for the `Blend.buf` format.

<br>

## Where this step sits in the overall process

The maintainer's own recipe for a character is
[issue #84](https://github.com/nhok0169/Anime-Game-Remap/issues/84) (Nilou), and every
character follows it:

1. gather the **hashes and indices** (`hash.json` of the GI-Model-Importer-Assets checkout)
2. find the **vertex group remap** for one direction (source -> target)
3. test it in game and note the quirks
4. write the `.ini` fixing strategy for that direction (the parser/fixer of Creating Remaps)
5. find the vertex group remap for the **reverse** direction
6. test that in game
7. write the reverse direction's `.ini` strategy
8. update the documentation

Steps 2 and 5 are this file. The user-facing manual method --- Blender, weight-paint mode, one
bone at a time --- is `Docs/src/findVertexGroupRemap.rst`; **`Tools/VGRemapFinder` automates it
to a draft** that a human then checks, and its README carries the measured accuracy. The
`Docs/src/createRemap.rst` step 4 still points at the pre-migration `VGRemapData.py`; the live
table is C++ (below).

<br>

## The invariants --- the rules a remap must satisfy

**1. Every source vertex group must map to some target group.** The maintainer confirmed this
rule on 2026-09-09, having learnt it while doing Shenhe by hand; the earliest drafts (Keqing,
Ningguang, Jean's capes) left "I don't think there is a corresponding index" rows blank, and
those blanks reached the library. What an unmapped group does is not "nothing":
`BlendFile::remapIndices` writes it as the **negative bone index `-index-1` with its weight
kept**, so the game reads a garbage bone matrix for that share of every vertex that uses it.
Up to a third of an elbow vertex's weight pointing at bone `-76` was issue #213's "kink at the
elbow". A part the target genuinely lacks still goes *somewhere* --- the bone that moves the skin
it is attached to (a collar to the neck, a belt charm to the hip, a cape to the upper spine or
the arm it drapes over), never left out.

**2. The two directions are two separate rows, and neither is the inverse of the other.**
`Keqing -> KeqingOpulent` and `KeqingOpulent -> Keqing` are found, tested and stored
independently (several source groups may share one target, so no inverse exists). A character
with several targets is several rows keyed `(from, to)` --- not a `MultiModFixer`.

**3. The row is version-keyed.** `{{"1.0", from, "", "<toVer>", to, ""}, VGRemap({...})}` in
`core/src/data/VGRemapData.cpp`, where `toVer` is the version the target skin appeared in
(Ganyu's pair is `4.4`, Keqing's `4.0`). The table floor-matches, so an override registered at a
later version wins over the shipped row for any fix at or after that version.

**4. A source has exactly `max(BLENDINDICES) + 1` groups**, holes included: a bone no vertex
uses still exists and still needs a row. Count it from the geometry, never from the row.

<br>

## A character of SEVERAL components (2026-09-12) --- the hurdle for every newer skin, and for WuWa

Older GI characters are one mesh: one position / blend / index buffer set, one vertex group
index space, one row per direction. **Newer skins are several.** YelanTranquil (5.7) is a
`Body`, a `Bang` and an `Eye`, each with its own buffers and hashes (`hash.json` lists them as
separate entries with their own `position_vb` / `blend_vb` / `ib`), and so **its own vertex
group numbering**: `Body 64` and `Bang 64` are unrelated bones. Wuthering Waves characters are
built this way throughout, so this is the shape to get right, not a Yelan quirk.

What follows from it, layer by layer:

- **A vertex group is `(component, index)`.** A source group of one skin maps to a component of
  the other *and* an index in it. The maintainer's own Yelan draft already says this: its header
  is `Yelan | YelanTranquilBody | YelanTranquilBang | YelanTranquilEye | Uncertainty | Comments`,
  one target column per component, exactly one filled per row. The tool reads and writes that
  layout, and for a multi-component **source** writes one sheet per component with the same
  naming in column A (`YelanTranquilBody to Yelan`, ...).
- **The library already has the columns.** `VGRemapData.cpp`'s row key is
  `{fromVersion, fromChar, fromComp, toVersion, toChar, toComp}`; every older row has `""` in
  both component slots. Yelan is stored as three rows `("Yelan", "") -> ("YelanTranquil",
  "Body" | "Bang" | "Eye")` whose union covers every Yelan group exactly once, and three reverse
  rows keyed by the source component. `getVGRemap(to, fromComp = ..., toComp = ...)` reads one
  row; the single-component habit of asking with `""` finds **nothing** for such a pair (verified),
  so a fixer that reaches these rows has to name the component.
- **The matching works unchanged across components once the candidates are the union**: the
  tool matches every source group against all the target's components' groups at once; a chain
  step is only "the next index" inside one component and a jump otherwise. Yelan -> YelanTranquil
  agreed with the year-old draft on 80 of 113 rows straight off, and every disagreement is
  either both methods agreeing against a quick hand guess, or an absent part (jacket, hood, fur)
  whose anchor is a judgement call either way.
- **The split itself is prototyped, the fixer side is still open.** A remapped mod's single
  `Blend.buf` has to be split per target component and the target's several draw calls fed.
  `Tools/VGRemapFinder`'s `ComponentSplit.py` does it two ways (the README's "Splitting a mod
  across a target of several components"): **negative index** -- every component gets the whole
  mod plus a blend remapped with only its rows, the other components' bones written as the
  `-index-1` sentinel so those vertices collapse -- and **graph cut** -- per component, the
  triangles whose vertices are wholly its (or, in `majority` mode, mostly its), the vertices
  they reference, every `.buf` filtered to those lines and the `.ib` renumbered. Its `Eye`
  output is byte-identical to the maintainer's hand-made one; the difference to watch is the
  `Bang`, which by the remap only receives what is weighted to the bang bones (22 / 318
  vertices) where the hand-made split took the whole hair (3762). The first in-game test
  (front hair missing, a jagged hole on the forehead) pinned the reason: 531 hair vertices are
  weighted head **and** bang, the head maps to `Body:13` only, so the Bang's negative-index blend
  put a sentinel on their head weight. A negative-index component needs every bone its vertices
  carry, so its remap is augmented from the **reverse** sheet (`Bang 0` is Yelan's head), honoured
  only on vertices that also carry a forward bone; and the graph cut grew a `relaxed` mode (a
  triangle kept when two of its three corners are the component's) so the Body side of the seam
  closes. The relaxed run (`YelanCopy4.ini`) brought the bangs back but left a row of 67 gaps along
  the hairline (one corner wholly Body, two live in Bang: nobody's), so the cut grew a `fill`
  mode: the negative-index component draws every triangle whose corners are all live in its
  blend, the cut components take everything else by majority, and the run prints a per-object
  coverage line that must read *drawn by nobody 0, by more than one 0*. `MixedFilter.py --mode
  fill` -> `YelanCopy6.ini`, in-game result pending as of 2026-09-12. Also learnt in game: a
  negative-index component's draw of an object with no live vertex (the Bang drawing Yelan's
  body) does nothing, so such draws are no longer emitted. The `fill` split **matched the
  maintainer's hand-made result of a year earlier**, which closes the geometry side; what was
  left was texture: Yelan's body far paler on YelanTranquil, an opaque lightmap restoring the
  skin, i.e. the same shading-alpha situation as Jean -> JeanSea. `LiftBodyLightMap.py` applies
  `JeanShading::liftLowAlpha`'s edit (alpha at or below 77 gains 77) through the bound
  `TextureFile` and the split binds the result with `--texture Body LightMap <file>`. **Then the
  measurement that should have come first: a GIMI lightmap's alpha is a material band** (0 /
  64-89 / 115-127 / 128 / 165-189 / 255 on this pair), each selecting a shading ramp, and the
  bands differ per skin (Yelan skin 115-127, Tranquil skin 255, Tranquil 128 = her sheer lace,
  dithered). The 128 lift fixed the skin by coincidence and put alpha 0 on the lace band, which
  read as static on the neck; the right edit is one band to one band (`--band 115 127 255`),
  which is also what the Jean 77 lift approximates. And a target's draw slots carry different
  ramp sets (Tranquil's slot B, the dress, has no skin band), so `--objectSlots Head=A Body=A`
  draws the whole mod through the slot that has it (`--variant slotA` -> `YelanCopy7.ini`,
  in-game result pending 2026-09-12). Recipe for the next pair: TexConverter `-a only` on both
  skins' lightmaps, histogram the alpha under skin-coloured diffuse pixels, map band to band.
  Static that survived the band fix led to two more checks worth doing by default: a
  negative-index draw must be **trimmed to its fully-live triangles** (a sentinel corner lands
  near the origin, so the half-live triangles are slivers through the neck and shoulders; the
  generator trims now, `YelanCopy8.ini`), and the **vertex colour** (first four bytes of every
  `Texcoord.buf` line) is a shader parameter the mod may have changed -- this mod's body has
  G = B = 188 against 128 on both game models (`EditVertexColour.py` -> `YelanCopy9.ini`).
  Vertex layout, lightmap RGB and diffuse alpha were measured identical on both skins and are
  not suspects. Both experiments pending in game as of 2026-09-12. The scripts beside the Yelan
  test mod (`Tools/Misc/YelanExperiments/NegIndexFilter.py`, `GraphCutFilter.py`, and
  `MixedFilter.py` for the per-component mix the issue's comment prescribes: Body and Eye cut,
  Bangs negative-index, written as the complete `YelanCopy2.ini`) write `.ini` files that sit
  next to the mod's own -- self-contained, one section group
  per component, the mod's objects handed to the target's `match_first_index` slots in draw
  order, and the texture registers / `ORFix`-vs-`NNFix` per component passed in as an
  `IniLayout` because nothing in the geometry says which register a draw reads. **What no
  fixer does yet**: none of this is reachable from `IniFixer`, and `HashData.cpp`'s key has no
  component column, so YelanTranquil's per-component hashes have nowhere to go until it grows
  one. Both are the fixer work (issue #190's steps 3-9). Yelan and YelanTranquil exist as
  `ModTypeId`s so the rows can be keyed, but have **no `GIBuilder` factory** on purpose:
  registering them would demand hashes, indices and a remove-table row that the fixer work
  will bring.

Two things about the geometry of these skins that cost a run each: the dumps spell the weights
element **`BLENDWEIGHTS`** (plural, older dumps say `BLENDWEIGHT`) and type `BLENDINDICES` as
`UINT` rather than `SINT`; and a `Face` entry in `hash.json` has no buffers at all and must be
skipped. And one thing about the process: the reverse direction of a pair has no draft to score
against, so it was made **inverse-consistent** with the forward one (if forward sends Yelan
`g` to `Body:t`, then `Body:t` goes back to `g` when the chain alignment or the tally proposes
it), and the 37 YelanTranquil groups nothing forward lands on (its own straps and cover-up) are
flagged in their comments for the in-game check.

<br>

## Recipe: a mod onto a skin of several components, end to end (Yelan -> YelanTranquil, 2026-09-12)

The pair is confirmed working in game -- geometry, skin, shading, hair -- and every step below
was found by a single-variable experiment against the previous state, most of them with a wrong
guess first. Read this before the next multi-component skin; the order matters, because a later
symptom is invisible until the earlier one is fixed, and because **the hard failures were NOT in
the remap.** The remap (the blend table) was right from step 1. Everything after it is the part
of a remap the tables never carried before: which draw slot, which shader, which texture channel
means what on each skin. The screenshots of every state are in
`AI Agent Help/CreatingRemaps/Images/Yelan/6_1/`, named by the `YelanCopyN.ini` they came from;
the generator that produced every file is `Tools/Misc/YelanExperiments/MixedFilter.py`
(`--variant final`), on top of `Tools/VGRemapFinder`'s `ComponentSplit.py`. **The whole chain is
also one script over ANY Yelan mod folder: `Tools/Misc/Prototypes/yelanTranquilFix.py`** -- and it
runs through the API's own parser, fixer and `RemapService` rather than re-creating them: a
runtime `ModType` (hash rows for Yelan and three pseudo targets `YelanTranquilBody` / `Bang` /
`Eye`, one vertex-group row each, the shipped GI builders borrowed), a `GIMIParser` on the API's
hash classifier, and one hand-built `GIMIFixer` per component (`GraphGroupRemap` onto the draw
slot, `ResRegCollect` for the texture registers, the index / register / fix-call / hash edits) --
and, since the same afternoon, **the mod's buffers as ONE resource group per `.ini` group**:
`ResGroupCollect` collects the blend, position, texcoord and ib registers through a `BufReplace`
each, and builds a `VGSplitGroupResource`, the API's own grouped resource whose fix splits them
together with `VGComponentSplit` (`core/.../buffers/VGComponentSplit.cpp`, a port of
`ComponentSplit.py`'s negative-index and fill strategies). Nothing of the geometry work is in the
script any more; `ComponentSplit.py` stays as the tool's own prototype of the same algorithm. On
the test mod the 12 buffers are byte-identical to the confirmed hand-run. It holds nothing of the
test mod but the two skins' constants, and is the prototype the fixer transcribes from and the
thing to run over other Yelan mods to find what the china dress over-fitted. Two older versions
sit beside it: `yelanTranquilFixPerBuffer.py` (one `ResRegCollect` + `fixFunc` per buffer, runs on
an API without the split classes) and `yelanTranquilFixStandalone.py` (no API at all). `--loop`
drives parse / fix / resources per `.ini` from the script instead of `RemapService`, for an API
built before the two binding fixes in step 8. **It runs from WSL too** -- the repo path defaults
to `/mnt/e/...` on Linux (or `AG_REMAP_REPO`), a Windows-form mod path is translated, and `--wsl`
from a Windows shell relaunches the same command inside WSL (`AG_REMAP_WSL_DISTRO`, default
`Ubuntu-22.04`; `AG_REMAP_WSL_VENV`, default `~/agremap-venv`); the two routes were checked
byte-identical over a copy of the mod. Point it at a folder that holds ONE mod: the service walks
every subfolder, fixes each Yelan `.ini` it finds, and writes that file's buffers next to the
source files it references -- a test folder with twenty `Fill*/` experiments under it collects
twenty suffixed blends beside the shared `YelanBlend.buf`.

1. **Split the mod per target component** (`ComponentSplit.py`, `--mode fill`): the negative-
   index component (Bang) draws every triangle all of whose corners are live in its blend, the cut
   components (Body, Eye) take everything else by majority. `strict` and `relaxed` cuts leave
   hairline holes (833 and 67 triangles); `fill` leaves none and the run prints a coverage line
   that must read *drawn by nobody 0, by more than one 0*. A negative-index component's remap is
   **augmented from the reverse sheet** (Bang 0 is Yelan's head), honoured only on vertices that
   also carry one of its forward bones, else the whole face is drawn twice. Its `.ib` is trimmed
   to fully-live triangles: a sentinel corner is not invisible, it lands near the origin.
2. **Pick the draw slot by SHADER FAMILY, not by rank.** Tranquil's slot A is the normal-map
   pixel-shader variant; Yelan's body, Tranquil's slot C and her Eye use the no-normal-map
   variant. Drawn through slot A the mod showed "static" on the throat, earrings and shoulders --
   a separate throat strip and a shoulder tattoo the source shader renders invisibly / faintly
   and the other shader renders opaque and bright -- through **eight** texture experiments that
   changed nothing (registers, bands, diffuse alpha, vertex colour, normal-map alpha, second UV,
   trims, cutout). Drawing through slot C removed it in one step (`--objectSlots Head=C Body=C`).
   Read the families off a frame dump: `vs=`/`ps=` hashes per draw, then ORFix's
   `ShaderOverride*` list names them. ORFix does know every one of her draw shaders; the
   `root_vs` in `hash.json` is only GIMI's pre-pass shader and is NOT in that list.
3. **Registers**: on 6.x the main pass is lightmap / normal map / diffuse in `ps-t0..2` (dump
   draws 44/45), and reflection / outline passes differ; bind the mod's textures as the dump's
   texture order for the slot and let `ORFix` (normal-map slots) or `NNFix` (the rest) re-slot
   them -- without them the character goes green (Copy10). `ps-t3` and up are global textures
   identical on both skins; nulling them blackens the body (Copy11).
4. **Per-vertex data the target shader reads and the source's does not.** Tranquil's slot C
   carries a second UV (`TEXCOORD1`) on 3010 of its 4638 vertices, her sheer panels, and none
   elsewhere; Yelan's model carries one on every vertex and the mod copies that. Zero it
   (`EditVertexColour.py --zeroUV1`). The mod's body also carried vertex colour G = B = 188 where
   both game models say 128; normalise it. Together these removed the last skin-tone
   segmentation (upper back paler than the arms).
5. **The lightmap alpha is a material BAND, per skin.** Build the legend from both skins'
   lightmaps' alpha under diffuse-classified pixels (`TexConverter -a only` + a histogram):
   Yelan / the mod: 0 = hair AND every cloth, 64-89 metal, 115-127 skin, 165-189 ornaments
   -- **and 255 = her white FUR** (the shawl, the trims, the jacket lining), which the mod-derived
   legend missed because the china dress has none: it took the IDENTITY mod (below) to read it off
   the skin's own textures at its own vertices. Two consequences: an author who leaves the alpha
   opaque has painted every cloth as fur (the Fontaine mod's grey stockings, which Tranquil then
   shaded as SKIN, beige with a sheen), and Yelan's own shawl would render as skin too. The
   prototype moves 255 to Tranquil's fur band 0 (not to her silk band -- the first guess, before
   the identity mod said what 255 was) and lifts the skin band, on EVERY object's lightmap, the
   head's included (the china-dress head is hair only, so leaving it alone cost nothing there;
   the identity head carries the shawl and the neck). **And the legend is the AUTHOR's, not the
   skin's**: a port keeps its SOURCE character's bands -- the Clorinde port (`Mods/Yelan2`) has
   its hair on 115-127 and its skin on 50-99 -- so an unconditional skin lift put that hair on
   Tranquil's skin ramp, and the skin ramp on dark hair showed as speckles. The lift is now
   conditional on the object's DIFFUSE under the pixel being skin-coloured (warm, R >= G >= B,
   bright enough; `skinColoured` in the prototype), which leaves dark hair on the band it came
   with -- Tranquil's hair band, by the luck that made "head untouched" look right in the first
   place. The fur move stays unconditional. Read a new mod's bands per OBJECT at its own vertices
   (the `diagYelan3`-style tally: diffuse mean per band) before believing any legend for it;
   Tranquil slot A: 0 = white fur, 64-89 silver, 115-128 hair, 165-189 silk and the lace cape,
   255 skin. Then a band table with **diffuse-conditional rows** (hair and cloth share band 0 on
   the source): skin 115-127 -> 255; band 0 where the diffuse is dark (max channel <= 125, the
   hair) -> 121; the rest of band 0 -> 177. A blind lift (the Jean 77 recipe) moved every band at
   once and put the dress on her fur band. `LiftBodyLightMap.py --band / --bandDark`.
6. **The lightmap BLUE channel is the painted hair-highlight mask** (zigzag marks; render the
   channels over the hair rows to see it), and the highlight's COLOUR is the target skin's hair
   constant -- light blue on Tranquil, grey-blue on Yelan -- unreachable from any texture. Band
   (121 / 128 / checkerboard), R and alpha did nothing to it; scaling B did (`--bDark 125 0`
   removes it, 0.5 still reads light blue). A taste knob, not a fix.
7. **Diffuse alpha darkens in this shader and is ignored in Yelan's.** The crown hair (the mod's
   Head object) is drawn from Yelan's head texture at alpha 255, the back hair (Body object)
   from the body texture at alpha 0, so the two halves of the hair shaded differently; set the
   hair pixels of the body diffuse to alpha 255 (`--alphaDark 125 255`).

   **The maintainer then found a simpler texture recipe that fixes all of 5-7 at once (Copy28,
   confirmed in game the same day), and it is what `yelanTranquilFix.py` does now:** draw the
   Body's slot C and the Bang through the NORMAL-MAP layout (`ps-t0` a flat normal map the fix
   CREATES -- `TexCreate` with a `TexCreator(1024, 1024, Colour(127, 127, 255))`, the same
   invention Ganyu's fix makes -- `ps-t1` diffuse, `ps-t2` lightmap, `run = ORFix`), the head
   diffuse at **alpha 1** everywhere (`putalpha(1)`), the body lightmap's skin band 115-127 lifted
   to 255 and nothing else touched -- no hair rule, no highlight mask, no colour match; the Eye
   keeps the head's original diffuse and lightmap under `NNFix`. The band legend above is still
   the map for reading any other pair.
8. **Drive it through the API, and what that cost (2026-09-12).** Everything above is expressible
   with the API's own edits from Python -- `yelanTranquilFix.py` is the proof -- so the fixer-side
   gap is tables and config fields, not machinery. Seven traps, every one of which produced a run
   that looked clean:
   - A drawn object landing on a target SLOT rather than a same-named object: `GraphGroupRemap`
     head -> C, body -> C. The second claimant lands in group 1, which is a second `.ini` file
     (`yelanRemapFix1.ini`, carrying the mod's own sections again plus that group) -- the API's
     merge shape, as Keqing ships it.
   - The target's other slots (Tranquil's A and B, her own body and dress) are hidden without an
     `ib = null` section each: remap `("", "ib")` keeping `handling = skip` and DROP its
     `drawindexed = auto` (the template's `moveDrawIndexed`), then `RegFillMissing("drawindexed",
     "auto")` on each drawn slot. Nothing the mod does not draw itself is redrawn.
   - `GIMISectionClassifier` reverse-looks-up a section's `match_first_index`; a pseudo target with
     an index `0` of its own made Yelan's head fall through to the shared `ib` graph. Filter with
     `hashNonVersionVals` / `indexNonVersionVals = {"name": "Yelan"}`, and register NO index rows
     for a target nothing looks up (`RegNewVals` writes the index from a literal).
   - `GIMIObjPartFilter.filter()` hands out callables that point back at the filter object. Built
     as a local of the fixer factory it is garbage-collected, every window comes back empty, and
     `match_first_index` silently keeps the source's value. Keep the filter alive.
   - The fixer factory runs BEFORE the parser parses, so a split that needs the mod's file names
     cannot read the parser's graphs; read `IniFile.getIfTemplates()` by hash instead.
   - `RemapBlendReplace(fixFunc = ...)` never ran from `RemapService`'s C++ resource loop: pybind
     casts the non-copyable resource by COPY when no Python wrapper exists yet
     (`return_value_policy = copy, but type is non-copyable`, logged per resource, blend count 0).
     Fixed in `PyRemapBlendResource.cpp` (cast by reference -- now the shared `toPyRefFunction` in `py/src/tools/PyRefFunction.h`); the same resource's
     `fix()` from Python always worked, which is why no test saw it.
   - A Python-built `RemapBlendReplace`'s resources are type `resourceRemapBlend`, which
     `RemapStats::get` did not know: *fixed 0 Blend.buf files* over a folder full of them. Aliased
     the same day. Pass `resType = "position"` / `"texcoord"` / `"buf"` for the other buffers so
     each is counted under its own kind.
   - **A per-buffer `ResRegCollect` is the naive shape here, and the maintainer said so:** the ib
     and the vertex buffers depend on each other (issue #190's second comment -- the blend decides
     the vertices, the ib the triangles, and position / texcoord must follow the same vertex set),
     which is exactly what `ResGroupCollect` was built for. The core now has the pieces: a
     `BufReplace` per buffer kind names the resource and builds an `IniFixResource` typed
     `blend` / `position` / `texcoord` / `buf`; a `VGSplitGroupResource` (a
     `RemapIniGroupedResource`, `_fix` = `fixVGSplitGroup`) reads its members by that type, runs
     `VGComponentSplit` for its component and writes every fixed file; the Python-facing
     `VGSplitGroupResource` is a `PyIniGroupedResource` so `IniGroupedResBuilder` can build it
     inside `ResGroupCollect`. `test_VGComponentSplit.py` pins all three. Two things about wiring
     it: **fill the draw call with `RegFillMissingMode.BottomCover`** (added the same day). The
     collect splices the collected register into an `if 1 ... endif` block, which splits the
     section into parts, and `FillMissing` fills the FIRST content part that lacks the register --
     the whole section while it is one part, the wrong end once split, so the draw ran before
     the ib and the textures. `BottomCover` adds a fresh LAST part at each root instead, so the
     collect and the fill can be in any order. And every drawn object's ib is handed to the group
     (`ibPaths`) even when the API's merge put the object in another `.ini` group, since a cut
     component's vertex set is the union over all of them.
   - **The first grouped run had model AND texture errors in game, and the maintainer's hand-made
     pair (`yelanMod/Yelan.ini` + `YelanCopy.ini`, the double-file shape done by hand) was the
     reference that found three omissions.** (1) `override_byte_stride = 40` /
     `override_vertex_count = N` on the remapped draw-hash section: the Bang draws the mod's 17220
     vertices through a buffer sized for Tranquil's own few-thousand-vertex bang, so without the
     raise the model is garbage -- `RegNewVals({...}, addNewKVPs = True)` on `("", "other")`, N
     from the split. (2) The Pillow texture engine writes the head diffuse untagged and
     uncorrected; the Compressonator engine reads the sRGB bit and bakes the 1/2.2 -- except that
     the Python `TextureFile.save` was erasing it (see Creating Remaps' sRGB section; fixed).
     (3) A created texture standing in for an sRGB one is authored pre-corrected (127 -> 55 for
     the flat normal map). The one structural difference from the pair -- it keeps
     `drawindexed = auto` on the remapped ib section and hides Tranquil's slots A / B with
     `ib = null` sections, where the API route drops the ib section's draw and has no hide
     sections -- is confirmed NOT to matter: a skipped draw with no re-issue draws nothing, in
     game, on three mods and the identity (2026-09-12).
   - **The second mod (`Mods/Yelan3`, the Fontaine outfit: head, body, a hidden dress and a cape
     as `extra`) found what the china dress over-fitted, in one pass each.** (1) *The cape hung
     crooked*: it is rigged to Yelan's jacket-flap chains 0-3 / 7-10, which the china dress never
     touches, so the shipped row's entries for them were the tool's per-bone nearest and had never
     been seen in game -- one chain's root on Tranquil's upper ARM (11) and its tail on a hanging
     ornament (125 / 79), the other on the shoulder piece (68). A part the target LACKS wants the
     draft's symmetric anchor for the whole chain (0-4 -> 64, 7-11 -> 68, the sides 5-6 / 12-13 ->
     63), which is what the hand-made sheet said and the tool's "kept the draft's anchor" rows only
     half-kept; `VGRemapData.cpp` now carries the draft's values for 0-13, with the reasoning in a
     comment. The diagnostic that found it in a minute: the per-object vertex-group tally of the new
     mod against the old (`groups used by Yelan3 but not Yelan1`), then the centroid of each such
     group on the mod and of its target on Tranquil's frame dump. (2) *The lower legs did not match
     the torso*: the band note under step 5 -- opaque alpha is Tranquil's skin band.
   - **The whole thing is COMPILED as of 2026-09-13, and confirmed in game the same day** -- `makeGIMIComponentFixer`, the second
     fixer template, with Yelan as its first row; the prototype stays as the thing it was
     transcribed from and A/B'd against (every buffer byte-identical on three mods). Creating
     Remaps' "Yelan is COMPILED now" says how the shape is encoded and the five things the port
     found in the framework; read it before Bennett.
   - **Test the IDENTITY mod first, not whichever download comes to hand (the maintainer's call,
     2026-09-12).** `Tools/Misc/Prototypes/identityMod.py <asset folder> <mod folder>` writes the
     character's own model as a GIMI mod out of `GI-Model-Importer-Assets/PlayerCharacterData/
     <Name>` -- `hash.json` for the hashes and object offsets, the `*-vb0=` dump split into the
     Position / Blend / Texcoord `.buf` files through the API's `VbFile.readDumpStr` (the dump's
     92-byte vertex is the three GIMI buffers laid end to end), every `*-ib=` dump as an R32 `.ib`,
     the `.dds` copied, and the `.ini` in the shape GIMI generates (`run = NNFix` on each object;
     `--noFix` leaves it out). Yelan's (`Mods/Yelan4`) is 16062 vertices, head / body / dress /
     extra, all 113 vertex groups live, every band of the real legend present -- a download
     exercises a subset of that (the china dress no jacket bones and no fur, the Fontaine outfit
     no dress), which is exactly how the two over-fits above stayed hidden. Verified by
     byte-comparing every `.ib` with its dump text and sampling vertices against the `vb0` text;
     the fix runs over it clean (4 objects, every triangle drawn once, the Bang taking the 551
     bang triangles and the Eye the 156 eye triangles).
     **It builds a skin of SEVERAL components too, as of 2026-09-13** -- every `hash.json` entry
     with a `position_vb` is a component and gets its own five buffer sections, its own objects and
     its own `.buf` files, and since a single-component skin's component name is the empty string
     the same loop writes both (Yelan's output is unchanged to the byte). YelanTranquil's is
     `Mods/YelanTranquilIdentity`: Body 25954 vertices in slots A / B / C, Bang 2256, Eye 120,
     built with `--faceRegister ps-t1 --textureFrom Bang=Body:A --textureFrom Eye=Body:A:plain`.
     Three things that shape cost, none of them guessable from `hash.json`:

       * **The texture layout is per OBJECT, and it is the shader family that decides it.** An
         object whose `hash.json` lists a NormalMap is bound in GIMI's three-register convention
         under **ORFix** (`ps-t0` normal map, `ps-t1` diffuse, `ps-t2` light map); one without is
         the two-register **NNFix** layout (`ps-t0` diffuse, `ps-t1` light map). That is not a
         convention someone wrote down -- it is what `ORFix.ini`'s own `CommandListReference` /
         `CommandListReferenceNoNormal` read, and its `CommandListFixLogic` then re-slots them to
         what the shader wants by the `ShaderOverride`'s `filter_index` (`037731.0` -> `LND`:
         light map, normal map, diffuse; `037731.1` and the fall-through `else` -> `LDX`: light
         map, diffuse). Tranquil's dump agrees draw for draw: 44 / 45 (slots B / A, vs
         `2c157719180b096c`, LND) and 46 (slot C, vs `d4c01363144d79d6`, LDX).
       * **A component can have no textures of its own and read another's.** Tranquil's Bang and
         Eye both draw the Body slot A set (`fe0fd573` / `183ca818` / `f5cc10b7` in the dump),
         which is why their `texture_hashes` are empty -- `--textureFrom <comp>=<comp>:<obj>` says
         so. The layout stays the BORROWER's: the Bang is on the normal-map shader and the Eye
         (`95aa6cdb84eb7b99`, no `filter_index`, so LDX) is not, hence the `:plain`. Left
         unpointed, such an object is written with its geometry and **no `run =` line at all**,
         deliberately: ORFix / NNFix re-slot whatever is bound whether or not the section bound
         it, so a fix call over the game's own already-correct slots scrambles them.
       * **The Texcoord stride is per component**, 20 with a second UV set and 12 without --
         Tranquil's Eye carries no `TEXCOORD1` where her Body and Bang do (dump strides 84 vs 92).
         It is measured off the dump and declared per component; only Position (40) and Blend (32)
         are fixed by GIMI's convention, and a layout disagreeing with those is an error.

     And the face: GI 6.x swapped the face diffuse and light map registers, so on a 6.x skin the
     diffuse is bound at **`ps-t1`** (Tranquil's main face draws 42 / 43 / 47 / 48 / 53 / 54 have
     the light map `d4841e1a` at `ps-t0`) and a section overriding `ps-t0` replaces the light map
     with it. `--faceRegister` defaults to `ps-t0`, which is what the older identity mods were
     built with; the maintainer's hand-made `YelanHandMade.ini` says `ps-t1` for Tranquil's
     `e8ad6095`, which is the value to trust. **The acceptance test for an identity mod is a byte
     comparison against the game's own buffers, not a clean run** -- a frame dump holds the real
     `vb0` / `vb1` / `ib` binaries under exactly the hashes `hash.json` names (the game already
     stores position / blend / texcoord as three buffers of those strides), so all 9 `.buf` files
     and all 3 index buffers were checked against `FrameAnalysis-YelanTranquil-2026-09-12-060339`
     rather than against the dump text the writer itself parsed, which would validate in a circle.
   - Still open, from the same conversation: the multiple draws one hand-made section did for a
     single hash + index (the Copy29 shape) would be a new `GraphGroupEdit` that APPENDS one graph
     into another -- the merge's second file is the API's answer today, and WuWa mods will want
     the append (a whole resource graph into a `TextureOverride` graph).

   Both C++ fixes were built and verified on Linux only (`~/cbuildlin-native`, 23 s a rebuild);
   **the Windows `.pyd` needs its own rebuild** before the script so much as imports there (it
   refuses an API without the split classes) -- run it under WSL until then, `--wsl` from a Windows
   shell does exactly that.

What is NOT a remap problem, and was chased as one for a while: the "static" (a shader-family
mismatch), the pale skin (a band), the arm/back tone (per-vertex data). What IS still open for the
library: steps 2-7 are reachable from a hand-built fixer (step 8) but not from the tables -- a
component column in `HashData`, a slot choice by shader family, a per-pair band table, a
`Texcoord.buf` edit and a per-component blend split still need config fields and rows. The pair,
and `yelanTranquilFix.py`, are the worked specification for all of them.

## Bennett -> BennettAdventure: the second multi-component pair, and a shape Yelan did not have (2026-09-14)

`BennettAdventure` is a `Body` (draw slots A and B), a `Bang` and an `Eye` -- structurally
YelanTranquil, so the ids, the rows and the draft all take her shape. Run over the two
`Data/Mod Downloads` folders, `Tools/VGRemapFinder` proposes Bennett's 80 groups against the skin's
106 / 9 / 2, and the union covers each of the 80 exactly once. `VGRemapData.cpp` gained **five**
rows, not six:

| direction | rows |
| --- | --- |
| `Bennett ""` -> `BennettAdventure` | `Body` (78 pairs), `Eye` (2 pairs) |
| `BennettAdventure` -> `Bennett ""` | `Body` (106), `Bang` (9), `Eye` (2) |

**There is no forward `Bang` row, and that is the finding worth carrying forward.** Not one of
Bennett's groups is nearest anything in the skin's Bang: all nine of its groups correspond to
Bennett's three head bones, so the correspondence exists only in the *reverse* direction. Yelan
has a forward Bang row (5 pairs) and still needed `ComponentSplit`'s `augmentFromReverse` to draw
her bangs at all -- so a component whose forward row is *empty* is not a new failure mode, it is
the same one at its limit. Expect the split, not the table, to feed such a component.

The rule the invariants section states -- every source vertex group maps to something -- is about
the **union across a target's component rows**, and that still holds here. Check it that way, not
row by row, or an empty row reads as a gap.

**Neither direction is confirmed in game**, and unlike every pair above Bennett has no hand-made
draft to score against, so the rows have only the geometry behind them. They are commented as such
in `VGRemapData.cpp`; `Data/RemapDrafts/BennettRemapDraft.xlsx` keeps the tool's `About` sheet for
the same reason, which is what stops `benchmark.py` ever scoring the tool against its own output.

<br>

## Where the data lives, and which copy to trust for what

| | what it is | trust it for |
| --- | --- | --- |
| `core/src/data/VGRemapData.cpp` | **the live table**, 58 rows, both directions of every pair (Yelan/YelanTranquil as six component-keyed rows) | what ships. Confirmed against fresh frame dumps and, for the pairs with drafts, against the drafts |
| `Data/RemapDrafts/*.xlsx` | the maintainer's hand-made drafts, one sheet per direction, opening with a `Credits` sheet (`README.md` there has the format, and the credit rule) | the intended mapping, with the reasoning in the Comments column. Some early workbooks had only one direction; the missing ones were added as **proposal sheets from the library's rows**, marked in cell `E1`. **Ground truth for `benchmark.py`** |
| `Data/Mod Downloads/GI/<Name>/<X_Y>/` | a mod-folder copy of each skin's geometry (`Position.buf`, `Blend.buf`, `*.ib`) **at the library's versions** | **the geometry to run the finder over for anything touching the table** --- group counts match the rows exactly |
| `GI-Model-Importer-Assets/PlayerCharacterData/<Name>/` | the asset repo's 3dmigoto dumps, **re-dumped Dec 2024** | hashes (`hash.json`), and geometry for the benchmark; but a newer dump can drift from the table (Xingqiu's has 74 groups, the row 92) |
| a raw `FrameAnalysis-*` folder | thousands of files straight from the game | proving whether a bone *moved* in an update: `--fromHashes` picks the character out |
| `WWMI-Assets/PlayerCharacterData/<Name>/` (Wuthering Waves; `SanhuaSkin1` is the Exorcist skin) | WWMI's own dump: `Metadata.json` plus one `Component N.fmt` / `.vb` / `.ib` triple per component, **one merged skeleton** across them (the per-component `vg_map`) | the only geometry there is for a WuWa character; the finder reads it as a single component in the merged index space, which is the space WWMI mods and the hand-made Sanhua draft use |

Three things the drafts will not tell you: the CN skins (Amber, Rosaria, Jean, Mona) have no
drafts because their remaps came from someone else; Kirara, Raiden and Arlecchino have none
either; and two drafts disagree with the library on one value each --- CherryHuTao 60 (draft 59,
library 58, finder 60) and Nilou 67 (draft 15, library 61, finder 15 at a 95% share). Both are
left as shipped; that is the maintainer's call, not yours.

<br>

## The tool, and the four things it is for

`Tools/VGRemapFinder` (README there; `GI/GIVGRemapFinder.ipynb` for the notebook route). It
reads any of the four geometry forms (three of GI's, and WWMI's for Wuthering Waves --- see the Sanhua
section below), proposes both directions, writes a drafts-format workbook,
and scores itself against a draft (`-c`) or the shipped table (`-C`).

```bash
# 1. a new character: propose both directions, score against the shipped rows if any
py -3 main.py "Data/Mod Downloads/GI/X/4_0" "Data/Mod Downloads/GI/XSkin/4_0" -o "C:/scratch/XRemapDraft.xlsx" -v 4.0 -C

# 2. "did this bone move in the update?": fresh frame dump onto the old dump of the SAME skin
py -3 main.py "<FrameAnalysis-X-...>" "<assets>/PlayerCharacterData/X" --fromHashes <pos> <blend> <ib> --toName XOld --oneWay
#    an unchanged skeleton comes out as one chain 0-N -> 0-N with every distance 0.0000

# 3. one doubtful bone: what drives the same patch of skin on the other skin
py -3 main.py ... --mode vertices        # read the share and runner-up in the comment

# 4. any change to the matching: score it on every draft, never on one character
py -3 benchmark.py "<assets>/PlayerCharacterData"
```

What it is worth: the default (weighted centre + spread, whole-index-order chain alignment)
agrees with the hand-made drafts on **89.6%** of 1993 groups. What it misses is mostly chain
neighbours with the same spread, a chain root whose true counterpart is not its nearest bone, and
"absent part -> bone 0" conventions no geometry can predict. Nearly every miss carries a high
Uncertainty (column C), so that column is the review order. **It writes a draft, not an answer:
the maintainer checks it in game exactly like a hand-made one.**

Three measured facts to keep you from re-deriving them: blend-weighting the summaries was
*neutral on Ganyu alone* and worth 2--5 points over all characters (never tune on one
character); the nearest-vertex mode scores 83% on its own and 72% restricted to the same drawn
object (Head/Body/Dress splits do not correspond between skins), so it is a per-group scalpel, not
a default; and segmenting chains by vertex sharing or proximity scored below one alignment over
the whole index order.

<br>

## Recipe: the vertex group remap for a new character, end to end

1. **Get the geometry at the right version.** `Data/Mod Downloads/GI/<Name>/<X_Y>/` for both
   skins (ask the maintainer to add it if missing --- it is step 3 of `createRemap.rst`). If only
   the asset repo's dump exists, confirm its group count is what the game has now.
2. **Propose both directions** (command 1). Read the summary: `unmatched source groups` must be
   empty by construction; `target groups nothing maps onto` is fine; the `least certain` list is
   what to look at.
3. **Check the high-uncertainty rows** with `--mode vertices` for the ones that are a part the
   other skin lacks, and by geometry (`VertexGroups`' centres/extents, the `objects` column) for
   the rest. Overrule the tally when it lands on hair for a non-hair part.
4. **Hand the workbook to the maintainer for the in-game check.** Write the reasoning for every
   judgement call into the Comments column; that is what the drafts are for.
5. **Transcribe into `VGRemapData.cpp`**: one row per direction, every source group present,
   pairs sorted. Patch it with a script that **re-parses the file afterwards and asserts the row
   count and per-row pair counts** --- a regex patcher that mixed a positional group with named
   groups silently dropped every patched row's closing `})},` once (restored from git). CRLF.
6. **Rebuild, then read the table back through the API** (`modType.getVGRemap(to).remap`) and
   assert no source group in `0..count-1` is missing. `overrideVgRemap.py --dump` prints exactly
   that line for one pair. Then `py -3 main.py VGRemapsTest` in the Unit Tester.
7. **A/B the bytes on a real mod** for the pair: fix a scratch copy with and without the change,
   decode both `RemapBlend.buf` with `BlendFile.decodeAll`, and confirm only the intended
   vertices changed index and no weight changed. `Tools/Misc/Prototypes/overrideVgRemap.py --ab`
   does this for a mod that has **not** been fixed before; a previously-fixed mod needs the
   scratch-copy version because it skips a `RemapBlend.buf` the source already carries.
8. **Add the draft sheets** (both directions) to `Data/RemapDrafts/<Name>RemapDraft.xlsx` if the
   maintainer wants them there; mark anything the tool wrote (`About` sheet for a whole workbook,
   `E1` for one sheet) so `benchmark.py` never scores the tool against its own output. **And
   credit yourself in the workbook's `Credits` sheet** -- every workbook has one, a new one gets
   one, and a Council member who changed any sheet adds `<Council name>: The <nth> member of The
   Council` under `Name`, hyperlinked to the Council README (the drafts' `README.md` has the
   exact layout). Not a member yet? Add nothing until you have joined, then come back. The
   workbooks are edited through `openpyxl`, which round-trips the duplicate-index conditional
   format, the autofilter and the frozen header of every data sheet; snapshot those before the
   save and compare after, rather than trusting it.

<br>

## Recipe: "the model is kinked / warped / exploded in game"

In this order --- each step is minutes, and the first one was the whole answer for #213:

1. **`overrideVgRemap.py --dump`** for the pair (set `SOURCE`/`TARGET`). The line
   `unmapped source groups: [...]` is the diagnosis if it is not `none`. Decode the mod's own
   `Blend.buf` to count how many vertices use the gap. **If every group HAS an entry and one
   part still hangs wrong**, `Tools/Misc/Diagnostics/modTally.py <mod> --against <a mod that
   looked right> --remap From To Comp` lists the groups only the broken mod uses and which
   targets several of them share; then `boneCentroids.py` on the target's frame analysis says
   where those targets are (a cape chain's root on an upper-arm bone, 2026-09-12).
2. **Has a bone moved?** Command 2 above, fresh frame dump vs the old dump of the same skin. If
   the skeleton changed, the whole row is stale and step 3 of the new-character recipe applies.
3. **Does the shipped row agree with the geometry?** Command 1 with `-C`. Rows the finder
   disagrees with at high uncertainty are candidates; rows it agrees with are not the bug.
4. **Fix through the override first** (`EDITS = {group: target}`), prove it with the byte-level
   A/B, *then* transcribe into the table. The override is process-wide and cannot be undone within
   a process, which is why its `--ab` runs the shipped copy first.
5. An exploded mesh with a *correct* table is the other classic: the `.ini` points at an
   **unremapped** `Blend.buf` (Creating Remaps' "The blend").

<br>

## Filling a gap, the policy that was used

For a source group with no row (or a "no counterpart" row) on 2026-09-09, in this order:

1. If a hand-made draft has a value the library dropped, the draft wins (Fischl 0 -> 40).
2. `--mode vertices` is the primary signal --- what drives the nearest skin --- with `--mode
   chains` as the second opinion; where both agree, take it.
3. Overrule both when the winner is hair or a jiggling decoration and the part is not: Keqing's
   thigh charms went to KeqingOpulent's hip (101), not the twin tails the tally picked; Jean's
   collar to JeanSea's neck (59), not her hair.
4. When two sources are the same part renumbered (Jean and JeanCN's cape), derive the second's
   fills through the library's row between them and cross-check against its own tally --- every
   derived value landed on the top or runner-up.

The reasoning for each fill is in the drafts' Comments column, so a future disagreement is a
conversation and not an archaeology dig.

<br>

## WuWa: Sanhua <-> SanhuaExorcist, the first draft outside GI (2026-09-18)

The maintainer had a hand-made `Sanhua -> SanhuaExorcist` sheet from a year before, incomplete, and
no reverse; the ask was to complete the workbook with the finder rather than by hand. Three things
about the WuWa side are worth more than the draft itself.

**The WWMI assets are a different format, and the format tells you the index space.** A folder of
`WWMI-Assets/PlayerCharacterData/<Name>/` holds `Metadata.json` and one `Component N.fmt` / `.vb` /
`.ib` triple per component: the `.fmt` is a 3dmigoto input layout (stride, every element's format
and byte offset, the index buffer's format), the `.vb` one interleaved binary buffer, the `.ib`
binary indices local to the component. Sanhua has seven components with 19 / 4 / 1 / 27 / 72 / 85 / 1
bones, which reads like YelanTranquil's shape and is not: `Metadata.json` gives each component a
`vg_map` from its own bone indices to ONE merged skeleton, and components share bones through it
(bone 1, the head, is in five of Sanhua's seven). A WWMI mod's blend buffer is in that merged space,
so that is the space a remap row must be in --- and the hand-made draft already was: it has 208 rows
for a merged skeleton whose largest index is 207, and its blank rows are *exactly* the 24 merged
indices no component maps to. `DumpMod.readWWMIFolder` applies the maps and concatenates the
components into the single component `""` (objects named `Component N`), needs no API, and refuses a
`.fmt` format it does not know by name. Sanhua onto herself is an exact identity; the forward proposal
agrees with the hand-made sheet on 86% (chains) / 89% (vertices) of scorable rows, the GI band.

**The broken-build check found the tell was a COUNT, not the score (habit 34).** Reading the
component-local bone indices with `vg_offset` added instead of through `vg_map` --- the plausible wrong
reading --- still scored 83% of the rows with a hand-made value (74% once the placeholder rows count
against a reader that gives those bones vertices), because the map is the identity plus an offset for
every bone but the shared ones. What that breakage cannot fake is the group count: 209 and 191 against the draft's 208
and 190 rows. A byte-offset breakage (indices read from the weights' bytes) never reaches the score
at all: the reader's own bounds check refuses it (`uses bone 255 but component 0's vg_map has 19
entries`). So the acceptance for a new reader is the count against the draft's rows plus the score
against its values, and the bounds check is what stands between a wrong offset and a plausible 83%.

**The reverse direction was reviewed from three signals per row, and the hand-made sheet's INVERSE
was the strongest of them.** For each Exorcist bone: the chain alignment, the nearest-vertex tally,
and which Sanhua bones the hand-made forward sheet sends to it. Of 190 rows, 19 are bones no vertex
uses, 129 have all three agreeing, 37 have the hand-made sheet plus one tool mode against the other,
and 5 needed a judgement (the reasons are in the comments): the back ribbon's tip (the chains are
different lengths, so the tip goes to the tip, not the nearest bone), the two stacked hip bones (both
tool modes wanted Sanhua 25 for both; the hand-made sheet pairs them by height), and three Sanhua
bones the hand-made sheet had left blank that are simply the same helper bones on the other skin
(centres 0.2--0.5 apart; the vertex tally sends such small patches to their big neighbour, the chain
alignment does not). The face bones are renumbered between the skins (Sanhua 7 / 11 / 17 are Exorcist
17 / 7 / 18 at identical centres), which the hand-made sheet already had right and the chain alignment
got wrong by insisting on index order. The uncertainty column follows a fixed scheme written into the
sheet's header cell: blank when all three agree, 0.2 for hand + one mode, 0.3 for no hand-made row,
0.4 for a judgement against both modes. **Nothing is checked in game yet**, and there is no WuWa
`ModType`, so no `VGRemapData.cpp` row can take it: the workbook is the deliverable.

**A bone no vertex uses gets a placeholder, and the placeholder must not score.** Every earlier
draft gives every row a value, so the 24 unused Sanhua bones and 19 unused Exorcist bones carry `0`
with the reason in the comment. That dropped `benchmark.py` from 86% to 76% for rows the tool can be
neither right nor wrong about, so `VGRemapFinder.compare` now skips a source group with no vertices
and says how many it skipped.

**The identity mods exist (2026-09-19): `WWMI/SanhuaIdentity` and `WWMI/SanhuaExorcistIdentity`,
built by `Tools/Misc/Prototypes/wwmiIdentityMod.py` -- and building them is what pinned down what a
WWMI mod IS.** Read a real one first (the maintainer's `Mods/Sanhua2`, a Blender export of the
original through WWMI Tools 1.3.4, is the template the script copies), then these four facts:

- **One mesh, several draw slots.** The game draws a character as `Component N` index ranges of ONE
  vertex buffer (`hash = vb0_hash`), and a mod replaces all of them together: one `Meshes/Index.buf`
  (R32_UINT, the components' local indices offset by `vertex_offset`, end to end), one Position /
  Vector / Color / TexCoord / Blend buffer holding every component's vertices in order, and a
  `[TextureOverrideComponentN]` per component matching on `(vb0_hash, index_offset, index_count)`
  that binds the shared buffers and draws its own range. `$object_guid` is the total index count.
- **The merged skeleton is the components' bone lists CONCATENATED, and `vg_map` is a de-duplication
  on top of it.** Each draw brings its component's bones in `vs-cb4`; `SkeletonMerger.hlsl` copies
  them into the merged buffer at that component's `vg_offset`. A bone in several components has
  several slots holding the same matrix, and `vg_map` names the slot WWMI Tools chose per local bone
  (the first component's). So the 24 "holes" in Sanhua's merged space are the duplicate slots, a
  mod may address either, and every real mod, the finder and the drafts use the `vg_map` one. The
  Blend buffer is 8 bytes a vertex: four `R8_UINT` bone indices, then four `R8_UINT` weights.
- **Shape keys are sparse, per key, and rebuildable from the dumps.** The `.fmt` carries them as
  per-vertex `SHAPEKEY<k>` `R16G16B16_FLOAT` deltas on the face-side components only (Sanhua: keys
  19-85 on component 2, 0-13 on 3, 14-18 on 6). `ShapeKeyLoader.hlsl` wants `ShapeKeyOffset.buf`
  (128 uint32s, entry k = where key k starts in the list, every entry past the last key = the total),
  `ShapeKeyVertexId.buf` (the mod vertex ids of every (key, vertex) with a non-zero delta, key by
  key) and `ShapeKeyVertexOffset.buf` (six halfs per entry: the position delta, then three zeros --
  every real mod leaves them zero). Metadata's `checksum` is the sum of the first four offsets and
  `dispatch_y` the entry count in 32s; the rebuilt buffers reproduce both for both characters (3175
  / 963, 2376 / 853) and the maintainer's own export's first offsets (534 / 1053 / 1588). The entry
  count is one above Metadata's `vertex_count` on both, a threshold difference in WWMI Tools that
  the shader does not care about.
- **Textures are overridden by hash, not bound to registers.** `[TextureOverrideTexture<N>]` with
  `this = ResourceTexture<N>` under `$object_detected`; the slot layout of each draw is the game's
  and `TextureUsage.json` records it. The identity ships every `Components-... t=<hash>.dds` of the
  asset folder so a remap's texture edits have something to act on.
- **Four or EIGHT bone weights a vertex, and a merged skeleton that may pass 256 bones**
  (2026-09-19, Chisa). Sanhua's Blend buffer is 8 bytes a vertex; Augusta, Iuno, Chisa and five
  more WWMI-Assets characters carry **eight** influences (16 bytes), and their `.fmt` says
  `R8_UINT` for an element that is 8 bytes wide -- `export_format`'s `stride` is the truth, or the
  distance to the next element. Worse, the 8-bit bone index **cannot name every bone**: Chisa's
  merged skeleton is 420 slots, Iuno's 413, Augusta's 375, and even 4-weight Changli reaches 275.
  WWMI's answer is the **blend remap**, and a mod that needs one carries three more buffers:
  `BlendRemapVertexVG.buf` (every vertex's full ids as `R16_UINT`, as many a vertex as the Blend
  buffer has weights) and `BlendRemapForward` / `Reverse.buf` (512 `uint16`s per remapped
  component: local -> merged and merged -> local). Per component whose vertices carry a non-zero
  weight on a bone >= 256, the sorted distinct bones it uses -- **at most 256** -- become local ids
  0..n-1; at load `BlendRemapper.hlsl` rewrites a private copy of `Blend.buf` through the reverse
  map, and each frame `SkeletonRemapper.hlsl` gathers that component's own skeleton through the
  forward one. `Blend.buf` itself keeps the merged ids **truncated** to 8 bits, which is what a
  component with no remap reads correctly. The merged skeleton resources double (`array = 1536`).
  `wwmiIdentityMod.py` writes all of it (WWMI Tools 1.7.3's `build_blend_remap`; **1.3.3, which is
  what is installed here, hard-codes four ids a vertex and is wrong for an 8-weight character**).
  The check that proves it is to run both shaders in numpy over the written files and require every
  weighted slot to land on the bone the raw `.vb` names -- Chisa, Augusta, Iuno, Galbrena and
  Changli pass, a swapped reverse entry and a stripped `.ini` fail. **Chisa and ChisaParfait's
  identity mods are correct in game (2026-09-19)**, which is the first confirmation of both the
  remap and of an asset folder taken from a frame dump.
- **THE SECOND WUWA DRAFT IS `Data/RemapDrafts/ChisaRemapDraft.xlsx`** (2026-09-20, both
  directions): Chisa 419 groups / 369 with vertices, ChisaParfait 251 / 198, proposed by
  `Tools/VGRemapFinder` from the two dump-extracted asset folders and **not reviewed and not checked
  in game** -- the `About` mark stays on it until it is. Reading an eight-influence character needed
  two lines in `DumpMod`: `R8_UNORM`, and the rule that a `BLENDINDICES` / `BLENDWEIGHT` declared as
  one `R8` channel really spans to the next element (Chisa's eight). `benchmark.py` still scores
  Sanhua at **86.4%**, unchanged, because her `.fmt` uses the four-channel formats and never takes
  that branch. The finder reads its own written draft back at 369/369 with the 50 no-vertex rows
  skipped, which is the check that the workbook is well formed.
- **A character WWMI-Assets does not have** (Chisa, ChisaParfait) gets its asset folder from a
  frame dump: `Tools/Misc/Prototypes/wwmiExtractDump.py` runs **WWMI Tools' own extractor** outside
  Blender (`bpy` stubbed; it skips the `<call>.<n>-[ShaderRegex_...]` sub-call files a mod like
  RabbitFX leaves in a dump, which the addon's name parser rejects). One folder per `vb0` hash, so
  pick the character's by component count and shaders. Run over the Sanhua and SanhuaExorcist dumps
  it reproduces WWMI-Assets' `.vb` / `.ib` / `.fmt` and `Metadata.json` exactly -- but **not the
  textures**: a dump holds each one in whatever streaming state it was drawn in (most of Sanhua's
  and Chisa's at 512 x 512 where the real texture is 2048), and 3DMigoto rehashes a texture as its
  mips load, so **not one** of the 16 hashes the Sanhua dump yields is WWMI-Assets', though 15 are
  pixel-identical to an asset texture. Check the extracted `.dds` sizes before trusting them, and
  note the corollary for the live game: today's Sanhua dump binds `332a6aac` where the shipped
  download folder and `SanhuaHashLineage.json` call `1c0c8b91` current (same pixels, both 2048).
- **A DUMP'S TEXTURE RESOLUTION IS ONE GAME SETTING, AND IT IS NOT "GRAPHICS QUALITY"**
  (2026-09-20). WuWa keeps its real graphics settings in
  `<game>/Client/Saved/LocalStorage/LocalStorage.db` (sqlite), **not** in `GameUserSettings.ini` --
  whose `sg.TextureQuality=3` the game ignores, so reading it says nothing. The setting that decides
  how many mips are resident is `ImageDetail`, the in-game menu's **LOD bias**, and at its default it
  is `0`: every character texture is dumped at 512 x 512 no matter how close the camera is (proved
  by dumping Chisa filling the screen -- same hashes, same sizes). Set LOD bias to **Ultra High**
  (`ImageDetail = 3`) and the same character dumps 12 textures at 2048 instead of 2. The XXMI
  Launcher's own "Max LOD Bias" switch writes that value before launch and **does not work**: the
  launcher set `0 -> 3` at 01:08 and the game rewrote the file at startup two minutes later, so the
  dump still came out at 512 -- change it in the game's own menu, where it sticks. And the hashes
  move with it (`bacb2d38` at 512 is `526b9ed0` at 2048), which is one more reason the fixer places a
  texture by pixel thumbprint rather than by hash.
- **NEVER DUMP A CHARACTER WITH A MOD OF THAT CHARACTER INSTALLED -- THE EXTRACTION DESCRIBES THE
  MODDED PIPELINE AND LOOKS LIKE A DIFFERENT CHARACTER, NOT LIKE AN ERROR** (2026-09-20). Two
  ChisaParfait dumps taken with her own IDENTITY MOD active came back with `cb4_hash` **empty** --
  WWMI had replaced the skeleton constant buffer with its own merged one -- so WWMI Tools read every
  component's bone count off the wrong buffer and reported a merged skeleton of **929 slots** where
  the mod-free dump says 264, past the 512 WWMI can hold. The per-vertex positions and weights were
  byte-identical throughout, and the textures were the MOD's copies under the mod's hashes
  (`2b1da041`, `ae2aab3c`) rather than the game's (`a506a70d`, `d547f3c6`). Nothing failed; only the
  numbers were wrong, and "it reproduces every time" was true and meant nothing. Take the mod out of
  `Mods` (the maintainer's find), re-dump, and the same character extracts clean at the same
  settings. `wwmiExtractDump.py` now prints `RE-DUMP` on an empty `cb4_hash` or a merged skeleton
  over 512 slots, which is the cheap detector for it. A related symptom of the same frames: a
  texture written with no hash in its file name (`t=None.dds`), recoverable from the dump's
  `deduped/` copy (`<hash>-<FORMAT>.dds`) and recovered by the extractor -- though on a modded dump
  what it recovers is the mod's own texture, so the warning above is the one that matters.
  `wwmiDownloadFolder.py --texturesFrom` takes the textures from a second dump of the same
  character, if geometry and textures ever have to come from different frames; `ChisaParfait/3_5`
  no longer needs it.

What proves the build: every vertex buffer is byte-identical to fixed byte ranges of the raw `.vb`
sliced independently of the script's element logic (POSITION 0-12, TANGENT+NORMAL 12-20, COLOR
28-32, the four UV/colour elements 32-48), the Blend indices equal the finder's own `vg_map` reading,
and the `.ini` has the template's section structure with only the numbers, hashes and texture list
changed. **Not yet run in game.** The maintainer's own `RemapBlend.buf` in `Mods/Sanhua1` and
`Sanhua2` was made with the API's `BufFile` given an 8-byte WWMI blend layout (`temp.py` beside each),
which is the shape a WuWa `BlendFile` will need.

**THE FORWARD PROTOTYPE EXISTS (2026-09-19): `Tools/Misc/Prototypes/sanhuaExorcistFix.py` rewrites a
Sanhua WWMI mod for SanhuaExorcist, and it was built from two frame dumps, not from the mod format.**
The maintainer's own year-old R&D (`WWMI/Sanhua1`, `Sanhua2`, issue #188) had the shape right --
retarget the six slots' hash / index range / `vg_offset` / `vg_count`, remap the 8-byte blend, bind
textures by register in the slot sections -- and said its component and register mapping was trial
and error. `Tools/Misc/Diagnostics/wwmiDrawTable.py <FrameAnalysis> <vb0 hash> --metadata ...` reads
the dump's `log.txt` into a per-draw table (component, shaders, every `ps-t` hash), and the `o0`
render targets after each draw name the components; that settled five things:

- **Shader families decide the slot mapping, not bones**: WWMI's merged skeleton makes every bone
  reachable from every draw, so a source component goes through whichever target slot renders it
  with the same slot layout. Head / hair / face / eyes have identical pixel shaders on both skins
  (94d9d5e9 / 69e3d321 / 374a4f8f / 056f9f3c); Sanhua's bodice and skirt (96356f03: `ps-t0` normal,
  `ps-t1` material mask, `ps-t2` diffuse) match Exorcist's torso slot 3 (3093e3c7, same layout);
  Exorcist's slot 4 (bun and trousers, 5cc08ed6) has no mask slot and draws nothing of the mod.
  Sanhua's **bare arms are their own component on a skin family** (7a0ab7c3: normal, diffuse) that
  Exorcist does not have; they go through the torso slot with an INVENTED mask.
- **The material mask legend was measured, not guessed**: on Exorcist's torso mask the texels
  coloured `(255, 77, 0)` are 99% skin-coloured in the diffuse under them and nothing else is; the
  shared face mask uses the same code; Sanhua's own body masks hold `(0,0,0)` / `(203,0,0)` and no
  skin at all. The maintainer's hand-made `Components-4 light map.dds` is a solid `(255, 77, 0)` --
  the same answer found a year earlier by trial. Open: Exorcist's non-skin texels sit at G = 50
  where Sanhua's sit at G = 0.
- **Texture hashes drift two ways and register binding sidesteps both**: the dump's character
  textures are the STREAMED 512-square mips, whose hashes differ from the 2048 asset files (that is
  what the community's `_LOWQ` overrides are for), and the asset hashes themselves changed between
  game versions (the maintainer's mods carry a 2025 set; `wwmi_fix_23`'s `hash_maps.json` maps some
  across). A `[TextureOverrideTexture]` matches the SOURCE's hashes, which never occur while the
  target is drawn, so the prototype leaves them alone (they keep serving the mod on Sanhua) and
  binds `ps-t0..2` / `ps-t5` in each remapped section through a per-component command list --
  gated on the target's main-pass pixel shader through `[ShaderOverride] filter_index` tags
  (`if ps == 3381.94`, the mechanism WWMI's core uses with `cs == 3381.3333`), so the outline and
  shadow passes keep the game's textures. Slots `ps-t3` up are globals identical on both skins.
- **A real mod's draw block is not a list of `drawindexed`**: KanouSakura's Sanhua carries
  `run = CustomShaderTransparency` draws (a blend state and its own `drawindexed`) inside the
  component section. The API's graph follows the `run =`, copies and renames the custom shader
  section with the rest, and the texture command list is added right after the shared-resource
  override -- the EARLIEST spot in its window, not the latest, because a component drawn in two
  ranges (Sanhua1's hair) otherwise had its textures bound between its draws.
- **The identity mod is the first test, and its blend was proved against an independent reading**:
  `RemapBlend.buf` equals the draft applied to the finder's own `vg_map` reading of the assets,
  weights byte-identical, no remapped index on a duplicate slot, and the REVERSE sheet applied by
  mistake gives different bytes (the check can fail).

**THE SPLIT WORKED, AND THE FIRST REAL MOD (the Witch Sanhua merged mod) REMAPPED RIGHT BUT FOR
THE EYES (2026-09-19)**: the eye pass reads the mask at `ps-t1` and the IRIS at `ps-t2` on both skins
-- the dump's textures correlate 1.00 with the asset's mask and iris there -- and at `ps-t0` a
texture matching no asset file, which the game keeps. The plan had the iris at `ps-t0`; the
asset-era `TextureUsage.json` had said `ps-t2` too. Two rules from it: **read a slot's role off the
dump, never off a neighbouring slot's pattern**, and a texture the dump binds that matches nothing
in the asset folder is a global to leave alone. The same check confirmed `ps-t5` (1035197c on
Sanhua, 4478285f on Exorcist).

**FIX THE ORIGINAL MOD'S TEXTURES BEFORE REMAPPING IT (2026-09-19).** A WuWa mod written for an
older game version has texture overrides on hashes the game no longer binds -- they never fire and
the game's own texture shows -- and, as often, `[ResourceTexture]` sections naming files the author
never shipped, which bind a MISSING resource and turn a material mask black. Both are silent, and a
remap inherits both. `Tools/Misc/Diagnostics/wwmiTextureFix.py <mod> --assets <download folder>
--maps <community hash_maps.json> --apply` resolves an old hash through the community maps or by
the mod's file being pixel-identical to exactly one current texture, whatever its size or format
(a repainted texture is left alone on purpose), and copies a missing referenced file in from the
downloads. Every old -> current pair it has measured for Sanhua is in
`Data/Mod Downloads/WuWa/Sanhua/SanhuaHashLineage.json`, to pass as `--maps` beside the community
tables -- it is what carries a REPAINTED texture's old hash forward, which pixels cannot. The Witch Sanhua mod (2026-09-19): five overrides still on 2024-era hashes that the mod's
own files identified as the game's bangs mask, bangs diffuse, hair diffuse, face diffuse and iris;
two material masks (bodice, skirt) referenced and never shipped, supplied from
`Data/Mod Downloads/WuWa/Sanhua/2_5`; four 512-square globals of that era with no current twin,
left as dead overrides. The community fixer beside it knew one of those hashes. The frost mod
(`sanhua-frost-final`, 821963 vertices, three `[KeySwap]` toggles with draws inside `if` branches):
12 hashes through the lineage map, nothing missing -- and then the remap bound NO texture on the
torso slot, because it resolved a texture's role from the `t=<hash>` in its FILE NAME and that mod
names its art `Component3.dds` / `Component3-NM.dds` / `Component3-LM.dds`. **A texture's identity is
the hash its `[TextureOverrideTexture]` section matches, and that section's `this =` names the
resource**; the prototype reads roles that way now and keeps the file-name hash only as the fallback
for a resource no override names. The toggled draws come through the remap verbatim (30 draw lines
in, 30 out, one of them commented in both); a toggled RESOURCE (`if $x / this = A / else / this = B`)
would contribute only its first, which no mod so far does.

**THE CLOAK MOD (2026-09-19) DECLARES ITS TEXTURES IN ANOTHER `.ini` AND HANDS THEM TO A LIBRARY THAT
IS NOT INSTALLED.** `Sanhua3/.../Sanhua Cloak - (longer)` is three LOD folders plus a top-level
`SanhuaCloak.ini` in its own namespace: THAT file holds `[ResourceDiffuse0] filename = Textures/
Component0_Diffuse.dds` and sections on Sanhua's hash that set `Resource\RabbitFX\Diffuse` and run
`CommandList\RabbitFX\SetTextures` -- RabbitFX (gamebanana.com/mods/527815) being the shared WWMI
library that binds textures by register. It is not installed on this machine, so the mod shows the
GAME's textures on Sanhua too; `LOD0/mod.ini`, the file the API fixes, declares no texture at all, and
the remap bound nothing. Three things changed in the prototype for it, all general: **(1)** textures
come from a run-level INDEX of every `.dds` under the folder the run was pointed at (DISABLED-prefixed
folders skipped, as the game does), with a file's role decided by the hash an override in ANY `.ini`
of the tree matches for it, then the hash in its name, then **pixel identity** with one of the game's
own textures in the download folder (colour correlation `>= 0.97` with one, `< 0.90` with every
other -- `Component6_Diffuse.dds` is the ps-t5 RAMP by its pixels, not the iris its name says), and
last the `Component<N>_<Diffuse|LM|NM>` name convention for the repainted ones; a file the fixed
`.ini` has no resource for is declared as the fix's own `[Resource<Role>...RemapFix] filename =
..\Textures\...`. **(2)** the vb6 line and the texture run had landed INSIDE the first
`if $draw_component_x` toggle: `RegSurroundedAdd`'s optional after-register (`drawindexed`) is a MUST
fact, and behind a toggle no draw is certain, so the earliest certain position was inside the toggle
-- the frost mod escaped it only because one of its draws sat outside every toggle. The add is
anchored on the shared-resource override alone now, and the identity mod's output is byte-identical
across the change. **(3)** `--undo` and the index both leave a `disabled/` folder alone in name only:
the API still fixes `disabled/mod.ini` (harmless, the game ignores it). Not done: the mod's
hood / cloak KEYS are gated on `$\SanhuaCloak\object_detected`, which only its own sections on
Sanhua's hash set, so on Exorcist the toggles sit at their persisted defaults; and its LOD1 / LOD2
files are on Sanhua's LOD hashes, which the library does not carry, so at a distance the game draws
Exorcist's own LOD model. **Not seen in game yet.**

**THE FORWARD FIX IS COMPILED (2026-09-19)** -- `makeWWMIFixer` / `makeWWMIParser`, the fourth fixer
template, A/B'd against this prototype on four mods with `Tools/Misc/Diagnostics/abWWMI.py`. What the
port found (a registry empty at table-build time, the group remap renaming before later edits, an undo
that deleted 17 of 19 textures, thumbprints in place of downloads) is in Creating Remaps' "WUWA IS
COMPILED". The prototype stays the oracle; its copies are named `<stem>RemapFix<n>.ini` now, the
API's way.

**Both characters have DOWNLOAD FOLDERS too (2026-09-19): `Data/Mod Downloads/WuWa/<Name>/2_5`** --
the identity mod's nine whole-mesh buffers, every asset texture as `<Name>Texture<hash>.dds`, and the
asset's `Metadata.json` / `TextureUsage.json`; see that folder's README and Creating Remaps'
"The download assets". Not fetched by anything yet (`DownloadTools::urlPath` is GI-only).

**Sanhua and SanhuaExorcist are IN THE LIBRARY as of 2026-09-19, and the prototype runs on it.**
`WWMIBuilder.sanhua()` / `sanhuaExorcist()` (`ModTypeId::Sanhua` / `SanhuaExorcist`, game `WuWa`)
carry the `vb0` / `cb4` / shape-key hashes, every draw slot's `match_first_index` in `Indices` typed
`component0..N`, and four NEW `Indices`-shaped tables for what GI never had -- `IndexCounts`
(`match_index_count`), `VGOffsets`, `VGCounts` and `ShapeKeyChecksums` -- plus `VertexCounts` and
both `VGRemapData` rows from the draft. The prototype reads all of it through `ModType`
(`getIndexCount`, `getVGOffset`, `getVGCount`, `getShapeKeyChecksum`, `getVGRemap`) and remaps the
blend through the API's own `BlendFile` given the 8-byte WWMI layout; the A/B against its
`Metadata.json`-driven predecessor is a byte-identical `.ini` and a blend identical on every
weighted slot (the API leaves weight-zero slots untouched where the numpy version had remapped
them, which is the library's contract and invisible in game). The parse / fix / remove rows are
stubs; Creating Remaps' "Adding a `ModTypeId`" has the WuWa differences.

**AND SINCE LATER THAT DAY IT FIXES IN PLACE, THROUGH THE API, LIKE THE YELAN AND BENNETT
PROTOTYPES.** `sanhuaExorcistFix.py <mod folder>` registers a Python-built `GIMIParser` and
`GIMIFixer` through `CppStrategyOverrides` ahead of the library's stub rows and lets `RemapService`
walk the folder: the fix is appended to the mod's own `.ini` as a `Sanhua Remap` block, the
`SanhuaExorcistRemapBlend.buf` lands beside the mod's `Blend.buf`, a second run undoes the first,
`--undo` removes it (and the two files it wrote) and `--hideOrig` comments the Sanhua sections out.
The mod then renders on BOTH characters, as a GIMI mod fixed onto a different-model skin does. What
the maintainer's question ("why does it need an output folder?") turned up is that the API's GIMI
pipeline handles a WWMI `.ini` almost unchanged -- the classifier sorts the seven slot sections by
`vb0` hash + `match_first_index`, the graph follows `run =` into the shared WWMI command lists and
writes each once, renamed; `RegAssetRemap` moves the hashes and the shape-key checksum through the
new table; `RegNewVals` retargets each slot's four numbers; `ResRegCollect` collects `vb4` out of
the shared override copy into the RemapBlend (with a `fixFunc` supplying the 8-byte layout);
`RegSurroundedAdd` places the texture `run =` -- and needed exactly three things:

1. **A line with no `=` was dropped on parse** (`IniFile.cpp`'s `parseSectionKVPs` skipped it as
   malformed). Every WWMI section opens its state guard with `local $state_id_N`, so the first fix
   left the variable undeclared and the skeleton never merged. Kept as a key with an empty value
   now, and BOTH renderers write an empty value as the key alone -- core's
   `renderIfContentPart` and the pybind `IfContentPart.toStr`, which is the one a Python-built
   fixer renders through (`makeFixerConfig` in `PyGIMIFixer.cpp`). The first fix after the core
   change still wrote `local $state_id_0 = `, which is how the second renderer was found. Pinned by
   `core/tests/IniFile_fix_test.cpp`'s `testKeylessLineRoundTrips` and
   `test_CppIfContentPart.py`'s `test_toStr_emptyValue_rendersTheKeyAlone`.
2. **`GIMIFixer::appendedSections` was not bound.** It is how the compiled component template
   writes its hide section, and it is the only way a fixer emits text that is not a copied graph:
   the per-component texture command lists (an `if ps == <filter>` block, which no register edit can
   add), the section for the slot nothing is drawn through (hash / window / `handling = skip`, and
   its bones still merged), the six `[ShaderOverride]` tags and the invented mask's
   `[Resource...]`. It is a `def_readwrite` now.
3. **A reverse lookup with no version resolves through the NEWEST bucket holding the value, across
   both games.** `match_first_index = 0` is every GI head's index too, filed at 6.1, and that bucket
   holds no Sanhua row, so component 0 classified as nothing while the other six (values unique to
   the 2.5 bucket) classified fine. The prototype hands the classifier `2.5` explicitly when the
   `.ini` carries no `fromVersion`; a compiled WWMI parser will hit the same thing through
   `ctx_.version()`, and the real fix is a game-scoped version line or `ModMappedAssets::getKey`
   preferring buckets that hold the filter's name -- open.

Several source components through one target slot are several SECTIONS on the same draw (Sanhua's
bodice, skirt and arm skin all match Exorcist's slot 3 window), which 3dmigoto runs in turn -- the
same fact a GIMI mod's shared IB section relies on -- so WWMI needs no second `.ini` file.
Verified: the blend is byte-identical to the output-folder version's; an undo restores the mod to
its original bytes bar one trailing blank line; fixing an already-fixed mod gives the same file as
fixing a clean one; the eight core suites and the 2301-test Python suite pass.

**THE FIRST IN-GAME RUN (2026-09-19,
`AI Agent Help/CreatingRemaps/Images/Sanhua/2_5/SanhuaExorcistWavyBody.png`) CAME BACK WITH TWO
REPORTS -- "the body seems all wavy" and "the hair texture seems off" -- AND BOTH WERE ANSWERED
FROM DATA, NOT FROM THE PICTURE.** The head, face, top, skirt and arms render right; what does not
is worth knowing for every WuWa remap after this one:

- **Component 0 is the BANGS, not the head, and its PASSES differ between the skins.** Its asset
  texture `c88cc1fc` ('Components-0-6', shared with the eyes) is an eye-shape mask, and the draw
  table shows component 0 drawn with hair-shader passes on both skins -- `a512f04f` + `f6bc3927` on
  Sanhua, `69e3d321` (the hair shader itself) + `8fbb5532` on Exorcist -- plus an eye-region pass
  (`94d9d5e9`) that binds only globals. The bangs are drawn twice so the eyes show through them. The
  prototype had gated the slot on the eye-region pass and bound the eye mask there, so Sanhua's
  bangs rendered with Exorcist's bang texture. The bangs' diffuse is `ae6e9014` ('Components-0-1'):
  proved by converting the dump's bangs-pass `ps-t0` and the mod's file to images and finding them
  pixel-identical, which is the check to make before assigning any role -- `TextureUsage.json`'s
  shader hashes are from an older game version and no longer match a live dump. `SlotPasses` in the
  script is a LIST of shaders per slot now, one `[ShaderOverride]` tag per distinct shader, and the
  texture command list asks `if ps == a || ps == b`. The eyes bind the iris (`1dcc0f1d`) at `ps-t0`.
- **The wavy parts are the chains Exorcist has no counterpart for, and they are wavy because of
  REST-POSE distance, not a wrong row.** `Tools/Misc/Diagnostics/wwmiBoneTally.py` (the WuWa form
  of `modTally.py --remap`: per source group, its vertices' centroid on the source identity mod,
  its target bone, that bone's centroid on the target identity mod, the distance, and every chain
  with the targets it lands on) says the finger bones match to the millimetre -- Exorcist's hand
  is the same rig, renumbered -- and every skirt chain maps onto an Exorcist skirt chain in order,
  3-5 cm off. The two long back ribbons (Sanhua 69-75 and 62/78/76/77/79-81 onto Exorcist 49-59
  and 52-62) sit 5-14 cm from the bones they land on, worst at the tips (Sanhua's are longer), and
  the front-left belt tassel (169-171 / 157 onto 162 / 164) 8-20 cm. A vertex bound to a bone whose
  rest position is that far off is rotated about the wrong pivot once the chain bends, and those
  ribbons bend ~90 degrees in the idle pose: they curled at her hands. That is Creating Remaps' Yelan
  lesson 5 exactly, so the prototype grew `--anchor ribbons` (each of those chains pinned to its
  root bone's target: the knot at the neck, the top of the belt -- rigid, no physics, no curling)
  and `--anchor all` (every skirt chain too, for the case where the skirt is what waves), plus
  `--vgRemap <json>` for any table at all. `WWMI/SanhuaIdentityOnExorcistAnchored` is the identity
  mod fixed with `--anchor ribbons` (1454 vertices differ from the default, exactly the ones weighted
  to the sixteen anchored groups, weights untouched); the default is in `Mods`. Whichever looks
  right in game is the row for `VGRemapData.cpp`.
- **And then the maintainer's OWN working hand remap settled which of those it is -- read the
  thing that works before theorising about the thing that does not.** `WWMI/Sanhua2/.../mod copy
  2.ini` (2025) renders on Exorcist without waves, and its `RemapBlend.buf`, read back per vertex
  against its `Blend.buf`, applies the SAME table as the library row: identical on 159 of the 162
  groups it uses, the other three its blank rows. Its ribbons are in that mesh (576 + 788 vertices)
  and do not wave. So the table, and the rest-pose distance above, is NOT what waves. What that
  `.ini` does differently: **its shape-key override is off** -- the `shapekey_checksum` line
  commented out, Sanhua's own shape-key hashes kept, which never occur on Exorcist -- where the
  prototype retargeted the shape keys onto Exorcist's buffer, handing Sanhua's 30805 shape-key
  vertices to a dispatch the game sizes for Exorcist's 27267 (`Metadata.json`'s `shapekeys.vertex_count`,
  `dispatch_y` 963 vs 853). Both skins carry the same 86 keys with the face keys identical in size,
  so the KEYS are not the problem; the buffer sizes are the suspect. `--shapeKeys` is opt-in now and
  off by default (the mod's shape-key sections are left on Sanhua's hashes, inert on Exorcist, and
  her face does not animate there -- the same trade the hand remap made). And that `.ini` binds a
  mask at the bangs' `ps-t1` (the 2025 `Components-0` UNORM texture); today's equivalent is the
  shared default mask `1c0c8b91`, whose mean pixel matches what the bangs pass binds in the dump,
  so the bangs bind it too. The `--anchor` variants stay available in case the ribbons still curl
  with the shape keys off.
- **Second report: the bangs are fixed, the waviness is not, and the identity mod itself is clean on
  Sanhua.** So the difference is between the two `.ini` files on the SAME mesh, and the complete
  diff of the hand remap against its own export is four things beyond the retarget: the arm-skin
  component dropped, the bodice sent through slot 4 (whose VERTEX shader differs from slot 3's),
  every `[TextureOverrideTexture]` commented out, and every shape-key section commented out -- the
  two buffer-size overrides included, which "left alone" had kept ACTIVE. Each is a switch now
  (`--plan rnd|rndArms`, `--shapeKeys hide|leave|retarget` with `hide` the default, and
  `--hideTextureOverrides`; the hiding is a post-pass writing the API's `HideOrig` marker, and a
  fix -> undo cycle restores the original to the byte), and four folders of the maintainer's mesh
  bisect it in one session: `WWMI/Sanhua2_A_rnd` is the hand remap's configuration through the
  prototype, `..._B_bodiceSlot3` is A with the bodice back through the torso slot, `..._C_arms` is A
  plus the arm skin, `..._D_shapeKeysLeft` is A with the shape-key sections left active. Whichever
  of B, C, D waves names the cause. One more thing measured on the way and worth keeping: Exorcist's
  torso carries vertex colour R = 0 on every vertex and her coat R = 0..255 (a sway weight, by the
  look of it) where Sanhua carries R = 255 everywhere but the face -- but the hand remap binds that
  same `Color.buf` and does not wave, so it is not the difference between the two.
- **THE ANSWER: A and D clean, B and C wavy -- anything drawn through the TORSO slot waves, the same
  thing through slot 4 does not.** The torso, face and eye draws consume a SIXTH vertex stream,
  `vb6` (stride 24; the buffer `Metadata.json` calls `shapekeys.offsets_hash`, `d709b169` on
  Exorcist), which the game's shape-key compute fills with the live per-vertex offsets INDEXED BY
  VERTEX ID and the vertex shader adds. WWMI's shared override rebinds `vb0`-`vb4` and leaves `vb6`
  alone, so a mod vertex drawn through such a slot is displaced by whatever offset Exorcist's buffer
  holds at the same index. Measured: 67% of Sanhua's arm-skin vertex ids and 11% of her bodice's
  coincide with ids Exorcist animates (her face and torso, 4111 ids), 1-2% of the skirt, none of the
  hair -- the reported pattern exactly, on both meshes. Slot 4's vertex shader reads no `vb6` (the
  dump binds it on nine shader pairs, all face / torso / eye passes), which is why the hand remap's
  bodice-through-slot-4 was clean and why dropping the arm skin "fixed" it. The fix: whenever the
  shape keys are not retargeted, every remapped section binds `vb6` to a buffer of ZERO offsets, 24
  bytes a vertex for the mod's `$mesh_vertex_count` (`<Target>RemapShapeKeyZero.buf`, written by the
  fix, removed by the undo), so the arms and the bodice go back through the torso slot with their
  own textures. `WWMI/Sanhua2_E_zeroOffsets` is that on the maintainer's mesh; the three default
  test mods are rebuilt the same way. **This is a WWMI fact, not a Sanhua one**: any WuWa remap that
  draws a source component through a target slot whose vertex shader reads the offset stream needs
  either the zero stream or WWMI's own shape-key pipeline retargeted and proved to fill it.
  And a WuWa fix is not confined to the mod's own `.ini` (maintainer, 2026-09-19): WWMI loads every
  `.ini` in the folder, so the extra files the GIMI merge writes (`<name>RemapFix1.ini` and on) are
  an option here too, for a fix that needs the same draw matched by more sections than one file
  reads well with, or to keep the remap apart from the mod's text.
- **The zero stream did NOT clear it either (fourth report), and two things reframed the bisect.**
  The maintainer pointed out that in A and D the arms are not drawn at all, so "clean" there says
  only that the skirt through the torso slot is fine; and that every clean variant had exactly ONE
  remapped section matching each Exorcist draw, every wavy one two or three on the torso slot's --
  which is the collision the GIMI merge writes a second `.ini` for, and their own R&D spreads its
  sections over several files. So the prototype now splits by TARGET WINDOW (`--noSplit` to keep
  one file): the first section of a window stays in `mod.ini`, each further one goes into its own
  complete `<stem>RemapFix<n>.ini` (named the API's way since the port) -- the mod's own draw sections commented out so
  only `mod.ini` serves Sanhua, every other remapped slot section cut down to its skeleton merge
  and `handling = skip` -- so every file merges the whole skeleton and draws exactly its own share.
  The extra files are deleted before a fix and on `--undo` (the API's undo does not know them); a
  fix, re-fix, undo cycle restores the export to the byte. `WWMI/Sanhua2_F_split` is the test: arms,
  bodice and skirt all through the torso slot, one per file. If it still waves, what waves is the
  component (arms, bodice), not the multiplicity, and the register bindings are next.

The three test mods are `WWMI/SanhuaIdentityOnExorcist` (a copy of the identity mod, fixed in
place -- also the copy in `Mods`), its `...Anchored` twin, `Sanhua2OnExorcist` and `Sanhua1OnExorcist`
(copies of the maintainer's R&D with only the Sanhua-hash `.ini` enabled; their older-era texture
roles -- now including the bangs' `48616ac9` and the iris' `3cd03f60` -- are inferred from format,
size and `Components-N` tag and marked UNVERIFIED in the script). Three things it does not do, on
purpose, until the next report: any texture edit beyond the invented mask, anything about the arm
normal map's B / A channels (the skin family stores B = 0, A = 255 where the body families vary
both), and the hair / head secondary passes.

Two environment facts that cost time: on this machine `py -3` is 3.13 with no `openpyxl`, and
**3.11 is the one with numpy, scipy and openpyxl** --- run the finder with `py -3.11`; and the Windows
`core.cp313-win_amd64.pyd` is from 2026-09-08, older than the Python package around it, so `import
FixRaidenBoss2` fails on Windows until it is rebuilt. The WWMI reader does not need the API, `-C`
does (and would have nothing to compare against anyway).

**The fix is compiled and the texture side has its own guide now (2026-09-19).** Everything above is
the geometry; for a WuWa mod that renders with the right SHAPE and the wrong LOOK, go to Creating
Remaps' "WuWa triage: what the in-game symptom says", "WuWa: choosing test mods by structural axis"
and "The next WuWa pair" -- the last is the config checklist, including the two texture rules the
compiled path learned after this section was written (a file plays every role its hashes name; a role
the mod has no file for is bound to the source's own texture, downloaded). The maintainer moves the
test mods between `WWMI/Mods` and its parent between turns: `find` by name before trusting a path.

## Mechanics that cost time once

- **Patching `VGRemapData.cpp`**: CRLF; named regex groups end to end; re-parse and assert
  before writing; `git checkout --` the file the moment a parse looks wrong.
- **The rebuild of a data table is not the 8-second build.** On 2026-09-09 changing only
  `VGRemapData.cpp` rebuilt every `py/` object and relinked `core.pyd` for ~30 minutes, and the
  install step **deletes the old `.pyd` before the link finishes** --- so nothing that imports the
  API (tests, the finder, the notebook) may run meanwhile, and a foreground timeout would have
  killed the link. Background it, wait for the notification, verify by the `.pyd`'s mtime.
- **The finder's own workbooks poison its benchmark** if they sit in `Data/RemapDrafts/`: an
  unmarked one scored itself at 100% and moved the total. Everything it writes is marked now
  (`About` sheet / `E1`), and `benchmark.py` skips marked sheets --- keep it that way when adding
  an output path.
- **A frame analysis dumps a buffer once per draw call that used it**, under different names with
  identical bytes; and its `vb0` (position) and `vb1` (blend) text dumps each carry the *whole*
  input layout in their header while holding only their own slot's lines. `DumpMod.fromFrameAnalysis`
  handles both; a header-driven reader does not.
- **`hash.json`'s `ib` can differ from the dump filenames' `ib=`** (KeqingOpulent: `7c6fc8c3` vs
  `44bba21c`); the frame analysis uses the `hash.json` one.
