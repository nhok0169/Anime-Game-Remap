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
     Fixed in `PyRemapBlendResource.cpp` (`PyFixFunc`, cast by reference); the same resource's
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

## Where the data lives, and which copy to trust for what

| | what it is | trust it for |
| --- | --- | --- |
| `core/src/data/VGRemapData.cpp` | **the live table**, 58 rows, both directions of every pair (Yelan/YelanTranquil as six component-keyed rows) | what ships. Confirmed against fresh frame dumps and, for the pairs with drafts, against the drafts |
| `Data/RemapDrafts/*.xlsx` | the maintainer's hand-made drafts, one sheet per direction (`README.md` there has the format). **Ground truth for `benchmark.py`** | the intended mapping, with the reasoning in the Comments column. Some early workbooks had only one direction; the missing ones were added as **proposal sheets from the library's rows**, marked in cell `E1` |
| `Data/Mod Downloads/GI/<Name>/<X_Y>/` | a mod-folder copy of each skin's geometry (`Position.buf`, `Blend.buf`, `*.ib`) **at the library's versions** | **the geometry to run the finder over for anything touching the table** --- group counts match the rows exactly |
| `GI-Model-Importer-Assets/PlayerCharacterData/<Name>/` | the asset repo's 3dmigoto dumps, **re-dumped Dec 2024** | hashes (`hash.json`), and geometry for the benchmark; but a newer dump can drift from the table (Xingqiu's has 74 groups, the row 92) |
| a raw `FrameAnalysis-*` folder | thousands of files straight from the game | proving whether a bone *moved* in an update: `--fromHashes` picks the character out |

Three things the drafts will not tell you: the CN skins (Amber, Rosaria, Jean, Mona) have no
drafts because their remaps came from someone else; Kirara, Raiden and Arlecchino have none
either; and two drafts disagree with the library on one value each --- CherryHuTao 60 (draft 59,
library 58, finder 60) and Nilou 67 (draft 15, library 61, finder 15 at a 95% share). Both are
left as shipped; that is the maintainer's call, not yours.

<br>

## The tool, and the four things it is for

`Tools/VGRemapFinder` (README there; `GI/GIVGRemapFinder.ipynb` for the notebook route). It
reads any of the three geometry forms, proposes both directions, writes a drafts-format workbook,
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
   `E1` for one sheet) so `benchmark.py` never scores the tool against its own output.

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
