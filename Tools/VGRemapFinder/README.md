# AG Remap's Vertex Group Remap Finder

[![Static Badge](https://img.shields.io/badge/Python-254F72?style=for-the-badge)](https://www.python.org/downloads/)
[![Static Badge](https://img.shields.io/badge/Jupyter%20Notebook-F37726?style=for-the-badge)](https://jupyter.org/)

<br>

Proposes the **vertex group remap** between two mods --- the table that says which bone index of
one character's skin each bone index of the other's corresponds to --- from the two characters'
3dmigoto dumps. The output is a draft workbook in the format of
[`Data/RemapDrafts/`](../../Data/RemapDrafts/README.md), which is what used to be filled in by hand.

Runs on the API's own `VbFile`/`IbFile` dump readers, so **the API must be built first** (see
[`AI Agent Help/Setup/CLAUDE.md`](../../AI%20Agent%20Help/Setup/CLAUDE.md)). Its own
dependencies (`numpy`, `openpyxl` for the workbook, `pandas` for the notebook's tables, `scipy` for
a fast nearest-vertex search) are in [`requirements.txt`](requirements.txt):

```bash
pip install -r requirements.txt
```

<br>

## How To Run

Either through the notebook, like the other notebook tools in this folder:

1. Go to the corresponding notebook:

| Game | Notebook Location |
| ---- | ----------------- |
| GI | [GIVGRemapFinder.ipynb](GI/GIVGRemapFinder.ipynb)

2. Follow the instructions on the selected notebook and run the codeblocks

<br>

Or from the command line. On
[CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py <fromFolder> <toFolder> [-o <draft.xlsx>] [-c <existingDraft.xlsx>]
```

`fromFolder` / `toFolder` each hold one character's geometry, in either of two forms, told apart
by what the folder holds:

| Form | What the folder holds | The mod's default name |
| --- | --- | --- |
| **3dmigoto dumps**, in the layout of [GI-Model-Importer-Assets](https://github.com/SilentNightSound/GI-Model-Importer-Assets)' `PlayerCharacterData` | `*-vb0=<hash>.txt` and `*-ib=<hash>.txt` | the folder's name |
| **A mod's raw files** | `*Position.buf`, `*Blend.buf` and the `*.ib` files (`Texcoord.buf` and textures are not needed; a `*RemapBlend.buf` next to the mod's own blend is ignored) | what precedes `Position.buf` (`GanyuPosition.buf` -> `Ganyu`) |
| **A raw 3dmigoto frame analysis** (`FrameAnalysis-<date>` straight out of the game, thousands of files) | every buffer of every draw call, named `000123-vb0=<hash>-...txt`; the character's files are picked out by its **position, blend and ib hashes** (`--fromHashes` / `--toHashes`, the `position_vb` / `blend_vb` / `ib` of a `hash.json`). Give a frame analysis without hashes to be shown which it holds | the folder's name with `FrameAnalysis-` and the date stripped |

Subfolders are not searched, so point at the folder that holds the files. All three forms decode
to the same numbers (checked on Ganyu, where the mod's `.buf` files and the dumps give identical
positions, blend data and object membership, and on Keqing, where a fresh frame analysis remapped
onto the old dumps is an exact identity). A frame analysis dumps the same buffer once per draw
call that used it, so only the first copy of each is read; its index buffers are told apart by
their `first index` and named `Head`, `Body`, `Dress`, `Extra` in that order. By default **both**
directions (`from -> to` and `to -> from`) are proposed.

<br>

### Characters of several components

Newer skins are not one mesh. YelanTranquil (5.7) is a **Body**, a **Bang** and an **Eye**, each
with its own position / blend / index buffers --- and so **its own vertex group index space**:
`Body 64` and `Bang 64` are different bones. A vertex group is therefore a `(component, index)`
pair throughout the tool, and a single-component character is the one component named `""`,
which is also what the library's remap rows use for it.

How each form declares its components:

| Form | Components come from |
| --- | --- |
| dumps | the folder's `hash.json`: every entry with a `position_vb` (a `Face` entry with no buffers is skipped), its `*-vb0=<position hash>.txt` and `*-ib=<ib hash>.txt` files, objects named by its `object_classifications` in draw order |
| a mod's raw files | one `*Position.buf` / `*Blend.buf` pair per component, named by what follows their shared prefix (`YelanTranquilBodyPosition.buf`, `YelanTranquilBangPosition.buf` -> `Body`, `Bang`); an `*.ib` belongs to the component whose prefix it starts with |
| a frame analysis | `--fromHashes <path to hash.json>` instead of three hashes |

What the matcher does with them: every source group is matched against the target groups of
**all** components at once, so the answer for a source group is a component *and* an index. In
`chains` mode a step to the next index only counts as "next" inside one component; a step into
another component is a jump. In `vertices` mode the target's components are one skin.

How the draft records it, matching the maintainer's own convention for Yelan:

- a target of several components gets **one column per component**, headed by the target's name
  with the component appended (`YelanTranquilBody | YelanTranquilBang | YelanTranquilEye`), exactly
  one of which is filled per row;
- a source of several components gets **one sheet per component**, headed the same way in column A
  (`YelanTranquilBody` in one sheet, `YelanTranquilBang` in the next).

And the library stores it the same way: one `VGRemapData.cpp` row per **(source component, target
component)** pair --- `("Yelan", "") -> ("YelanTranquil", "Body")`, `-> "Bang"`, `-> "Eye"`, and
the three reverse rows keyed by the source component. The union of the rows to one target must
cover every source group exactly once; `getVGRemap(to, fromComp = ..., toComp = ...)` reads one.
`-C` scores a proposal against them per component.

The dumps of these skins also spell the weights element `BLENDWEIGHTS` (plural) and type the
indices unsigned; both spellings are accepted.

### Splitting a mod across a target of several components

Once the remap exists, a mod made for the single-component skin still has **one** `Blend.buf`
and one set of draw calls, while the target draws each component from its own buffers. The
remap alone is not enough: the mod has to be *split*, one buffer set per target component, each
skinned in that component's bones. `src/VGRemapFinder/ComponentSplit.py` does that two ways,
both from the maintainer's issue #190 notes:

| Strategy | What every component gets | Where it is weak |
| --- | --- | --- |
| **negative index** (`negativeIndexSplit`) | the **whole** mod (its own position / texcoord / `.ib` files, untouched) plus a `RemapBlend.buf` remapped with only that component's rows; every bone of another component becomes the `-index-1` sentinel, so the game's skinning collapses the vertices that are not this component's | a triangle straddling two components stretches between a kept vertex and a collapsed one; each component draws every triangle |
| **graph cut** (`graphCutSplit`, `strict`) | the triangles whose vertices' bones are **all** in the component's rows, the vertices they reference, every `.buf` filtered to those lines (an `.ib` index is a line number in every `.buf`), the `.ib` files renumbered, the blend remapped -- no sentinel can occur | a triangle with vertices in two components is kept by neither: a one-triangle hole along each seam |
| **graph cut** (`relaxed`) | the same membership, but a triangle is kept when at least **two** of its three vertices are the component's; the third is dragged in, its foreign weight dropped and the rest renormalised | the seam closes from the side that owns two corners; a triangle with one corner in each of three components is still dropped |
| **graph cut** (`fill`) | for a mix with negative-index components: those draw every triangle all of whose corners are live in their blend (no sentinel), and the cut components share out **all the rest** by majority, so every triangle is drawn exactly once | the seam's shape is decided by which corners the negative-index blend keeps live |
| **graph cut** (`majority`) | the same, but a vertex belongs to the component holding **most** of its weight and a triangle to the component most of its vertices belong to; foreign weights are dropped and the rest renormalised, a vertex dragged into a component it has no bone in is skinned to its triangle neighbours' bone | every triangle drawn exactly once and the seams closed, at the price of a few hundred vertices skinned slightly differently from the mod |

Both write one folder per component and a self-contained `.ini` beside the mod's own (own
resource names, the target's hashes and `match_first_index` slots from its `hash.json`, the
mod's objects handed to the slots in draw order, `override_vertex_count` on the draw buffer,
`draw = N,0` on the blend). What the `.ini` cannot know is passed in: which texture registers
each component's draw sets and which external fix (`ORFix` / `NNFix`) it runs -- an `IniLayout`
per component. The worked example is the pair of scripts beside the Yelan test mod in
`Importer/GIMI/Mods/Yelan1/.../yelanMod/` (`NegIndexFilter.py`, `GraphCutFilter.py`, and
`MixedFilter.py`, which follows issue #190's table -- Body and Eye graph-cut, Bangs negative-index --
and prepends the mod's own sections with `--base` so its `YelanCopy2.ini` is a complete file), each
a dozen lines of Yelan-specific layout on top of `makeArgParser` + `runSplit`, whose strategy may
be one word or a dict naming one per component. The remap comes
from a drafts-format sheet with one column per target component (`--draft` / `--sheet`) or
from the API's shared table (`--library`, on a build carrying the rows), and `verifySplit`
checks every output (row counts, index ranges, no negatives and unit weights in a graph cut,
no triangle drawn twice). Two things the first in-game test taught: a negative-index component's
remap must also carry the source groups its **own** bones correspond to (YelanTranquil's `Bang 0`
is Yelan's head, which the forward rows send to `Body:13` only -- so every hair vertex weighted
head + bang got a sentinel on the head and the front hair vanished), which `augmentFromReverse`
reads off the reverse sheets and the split honours only on vertices that also carry one of the
component's forward bones (otherwise the whole face would be drawn twice); and two mod objects
landing in one draw slot must be drawn from **one** section, `ib` / `drawindexed` pairs in
sequence, because two sections on the same hash and `match_first_index` collide. The relaxed
cut alone still left 67 head triangles undrawn on Yelan (one corner wholly the Body's, two live in
the Bang -- a row of gaps along the hairline); `fill` is the mode that closes them, and the run
prints a per-object *coverage* line (drawn by nobody / by more than one) that must read 0 / 0.
A negative-index component's draw of an object with no fully-live triangle is not emitted at
all (the Bang's draw of Yelan's body did nothing in game, as expected: every vertex collapsed).
`--texture OBJECT KIND FILE` binds an edited texture in place of the mod's own -- Yelan's body
came out far paler on YelanTranquil, and the same lightmap lift the Jean -> JeanSea fix applies
(alpha at or below 77 gains 77, `LiftBodyLightMap.py` beside the scripts, through the API's
`TextureFile`) is the first thing to try; a texture edit is the part of a remap the split cannot
derive from geometry. **But measure before lifting: the lightmap alpha is a MATERIAL BAND, not a
brightness.** On Yelan / YelanTranquil it sits in discrete bands (0, 64-89, 115-127, 128, 165-189,
255), each selecting a shading ramp; Yelan's skin is 115-127 and Tranquil's is 255, so a lift of
128 fixed the skin by accident and, by moving alpha 0 onto 128 -- Tranquil's sheer-lace band, drawn
with dithered transparency -- put static on the neck and earrings. `LiftBodyLightMap.py --band 115
127 255` moves one band only. And the target's draw SLOTS carry different ramp sets: Tranquil's
slot A has the skin band, slot B (the dress) has none, so the mod's body drawn in slot B shaded
its arms and legs differently from the neck; `--objectSlots Head=A Body=A` pins both to slot A --
**and the slot to pin to is the one whose pixel-shader family matches the source's** (Tranquil's
slot C, the no-normal-map variant like Yelan's own; slot A's normal-map variant rendered a hidden
throat strip and a faint tattoo as bright "static" that no texture edit could touch). The full
worked chain -- split, slot by shader family, second-UV and vertex-colour normalisation, the band
legend with diffuse-conditional rows, the highlight mask in lightmap B, diffuse alpha -- is
`AI Agent Help/VGRemaps/CLAUDE.md`'s "Recipe: a mod onto a skin of several components".
A negative-index component now draws **only the triangles all of whose corners are live** (its
`.ib` filtered, vertex buffers whole; `--keepAllTriangles` for the original whole-mod draw): a
sentinel corner is not invisible, it lands near the origin, so a half-live triangle is a sliver
from the bangs down through the neck and shoulders. `--texcoordFile` draws with an edited
`Texcoord.buf` -- the vertex colour lives in its first four bytes, and this mod's body carries
G = B = 188 where both game models say 128 (`EditVertexColour.py` beside the scripts).

Measured on the Yelan mod (17220 vertices) against the maintainer's hand-made per-component
buffers: the `Eye` split is **byte-identical** to the hand-made one (120 vertices, same
weights, same bones); the strict `Body` cut has 16504 vertices to the hand-made 16522, 16502 of
them the same rows; the `Bang` is where the strategies differ from the hand work -- 22 vertices
strict, 318 majority, where the hand-made `hair/` folder took 3762 (the whole hair, 3188 of them
*also* in its body folder). The remap sends most of the hair to the head bone, which is the
`Body` component's, so the `Bang` component only gets what is weighted to the five bang bones.

<br>

### Examples

Propose Ganyu <-> GanyuTwilight and write the draft:

```bash
python3 main.py "PlayerCharacterData/Ganyu" "PlayerCharacterData/GanyuTwilight" -o "C:/scratch/GanyuRemapDraft.xlsx" -v 4.4
```

Score the proposal against the hand-made draft instead (the sheet is found by its header row, so
the names must match):

```bash
python3 main.py "PlayerCharacterData/Ganyu" "PlayerCharacterData/GanyuTwilight" -c "Data/RemapDrafts/GanyuRemapDraft.xlsx"
```

From a mod's raw `.buf` files instead of dumps (either side may be either form):

```bash
python3 main.py "Data/Mod Downloads/GI/Ganyu/4_0" "PlayerCharacterData/GanyuTwilight" -o "C:/scratch/GanyuRemapDraft.xlsx"
```

Straight from two raw frame analyses, scored against the remap the library ships for the pair
(this is how the KeqingOpulent -> Keqing elbow bug of issue #213 was pinned down: the proposal
agreed with every shipped row and showed the two source groups the shipped table had no row for):

```bash
python3 main.py "GIMI/FrameAnalysis-KeqingOpulent 2026-09-09-213520" "GIMI/FrameAnalysis-Keqing-2026-09-09-213224" --fromHashes 0d7e3cc5 6f010b58 7c6fc8c3 --toHashes 3aaf3e94 0bf8e621 cbf1894b -C
```

Score a change to the matching against **every** draft whose characters have dumps (see below):

```bash
python3 benchmark.py "PlayerCharacterData"
```

<br>

### Options

| Option | Description |
| --- | --- |
| `-o`, `--output` | The `.xlsx` to write the proposal into. An existing workbook keeps its other sheets; sheets for the same directions are replaced. The workbook carries an `About` sheet marking it as this tool's proposal --- `benchmark.py` skips such workbooks, so remove that sheet once the proposal has been checked and is a draft in its own right |
| `-c`, `--compare` | An existing draft to score against; prints the agreement and every disagreement |
| `-C`, `--compareLibrary` | Also score against the remap the AG Remap library ships for the pair (the mod names must be the library's own, eg. `Keqing` / `KeqingOpulent`). A source group the proposal maps but the shipped remap has no row for shows up as an entry the comparison cannot score --- check the `unmapped source groups` the library's table has before trusting it, since an unmapped group is written as a **negative bone index**, not dropped |
| `--fromHashes`, `--toHashes` | For a raw frame analysis: the mod's `POSITION BLEND IB` hashes, or the path of its `hash.json` (the only way to give a character of several components) |
| `--fromName`, `--toName` | The mod names (default: see the table above). They head columns A/B and name the sheets |
| `-v`, `--version` | The game version, for the sheet titles (`V4.4 - Ganyu to GanyuTwilight`) |
| `-m`, `--metric` | `gaussian` (default: centre and spread) or `center` (centres only) |
| `--mode` | `chains` (default: align runs of consecutive indices as a whole), `nearest` (each group independently), or `vertices` (nearest-vertex tally, see below) |
| `--stayCost`, `--skipCost`, `--jumpCost` | The chain alignment's step costs, in bone spacings (defaults `1`, `0.25`, `1`) |
| `-u`, `--unweighted` | Count every vertex equally when summarising a group, instead of by blend weight |
| `--oneWay` | Only propose `from -> to` |
| `-s`, `--silent` | Don't print progress or the summary |

Exits `0` on success, `1` if a folder or the API could not be read.

<br>

## How it works

1. **Read the geometry.** Every vertex has a position (the `POSITION` element of a dump's
   `*-vb0=<hash>.txt`, or the `*Position.buf`) and up to four `(BLENDINDICES, BLENDWEIGHT)`
   pairs (the same dump, or the `*Blend.buf`); a vertex belongs to every vertex group it carries
   with a non-zero weight. The index buffers (`*-ib=<hash>.txt` or `*.ib`) only say which drawn
   object (`Head`, `Body`, `Dress`, ...) each vertex is part of, which goes into the comments.
2. **Summarise each vertex group** by its **centre** (the mean of its vertices' positions) and
   its **spread** (the covariance of those positions --- how far, and in which directions, the
   group reaches). Both are weighted by blend weight, so a vertex a bone barely influences barely
   counts; `--unweighted` counts every vertex equally instead.
3. **Measure how far apart two groups are.** The default `gaussian` metric fits a Gaussian to each
   group and takes the 2-Wasserstein distance between them: the centre distance plus a spread term
   in the same units, with nothing to tune. `--metric center` compares the centres only.
4. **Match whole chains at once.** The game numbers a skeleton's bones in order, so a chain of
   bones --- hair, a ribbon, the rows of a skirt --- is a run of consecutive indices in *both*
   mods. The default `chains` mode aligns each run of consecutive source indices onto the target
   indices as a whole, by the cheapest path where stepping to the next target index is free,
   skipping one costs `--skipCost`, and staying on the same target (several sources onto one) or
   jumping anywhere else costs `--stayCost` / `--jumpCost`, all in units of the target's typical
   bone spacing. `--mode nearest` maps every group independently onto its closest one instead.

The dump stores a position as `(x, z, y)`; the axis order does not matter here, since a distance
between two points is the same whichever way the axes are labelled, as long as both mods agree.

**Uncertainty** (column C) is 0 for a confident match and 1 for a coin toss. In `chains` mode it
comes from the alignment: how much more the whole chain's best alignment would cost if that one
group were forced onto its runner-up (`exp(-margin / spacing)`). In `nearest` mode it is the ratio
of the best distance to the runner-up's. **Comments** (column D) carry the vertex counts, the
objects each group is drawn in, the runner-up with its distance, and the chain the match is part
of, so a wrong guess can be checked without re-running anything.

<br>

## How good it is, and how to tell if a change helps

`benchmark.py` scores the finder against every sheet in `Data/RemapDrafts/` whose two characters
both have dump folders --- 20 remap directions over 11 characters, 1993 vertex groups, as of
2026-09-09 --- and takes the same `--metric` / `--mode` / cost / `--unweighted` options as
`main.py`, so any change can be judged on all of them rather than on the one character it was
tuned on. (The dumps are cached under `.benchmarkCache/` after the first run.) Agreement with the
hand-made drafts, which are what `VGRemapData.cpp` ships:

| | `--mode nearest` | `--mode chains` |
| --- | --- | --- |
| `--metric center`, `--unweighted` | 81.9% | 85.5% |
| `--metric gaussian`, `--unweighted` | 82.9% | 87.1% |
| `--metric center` | 87.1% | 89.1% |
| `--metric gaussian` (**default**) | 87.8% | **89.6%** |

Three things the numbers say, all found by measuring rather than by reasoning:

- **Chain alignment is the single biggest win**, and its best form is the simplest: one alignment
  over the *whole* index order, breaking only at empty groups. Segmenting the sequence into
  chains first by spatial proximity or by shared vertices was tried and scored worse every time.
  Cheaper backward steps did not help either.
- **The spread on its own does nothing for nearest matching** on a single character (the misses
  are neighbouring bones along a chain, and neighbours have the same spread) but is worth a point
  or two once combined with the chain alignment.
- **Blend-weight weighting looked useless on Ganyu alone** --- it moved one group either way ---
  and turned out to be worth 2--5 points over the whole benchmark. Never tune on one character.

What remains is mostly two kinds of miss. A hand-made draft sometimes sends a bone the other skin
simply does not have to bone `0` (Klee's `67-70 -> 0`), which no geometry can predict; those
come out with a large distance and a high uncertainty. And where two skins' proportions differ,
the true counterpart of a chain's root is not its nearest bone (Ganyu's hair `16 -> 4`, where `5`
is much closer). Nearly every miss carries a high uncertainty, so the workbook's column C is the
list to check first: above `0.5` it flags about a fifth of the rows and catches over half the
wrong ones.

**For one doubtful bone, `--mode vertices` is a sharper tool than any group summary.** It takes
the vertices that bone drives, finds the *nearest vertex* on the other skin to each, and tallies
which bones drive those with what weight (source weight times target weight). Two skins of one
character share most of their skin geometry, so this answers "what moves this patch on the other
skin" directly; the comment reports the winning bone's share and the mean distance to the nearest
skin, which says whether the patch exists on the target at all. It is what settled
KeqingOpulent's elbow helpers (75 / 99): the chain alignment had them tied between Keqing's 45
and 46 (two upper-arm-side helpers), while the tally showed that skin is driven 69% by Keqing's
upper arm and 28% by her forearm, with the helpers at 1% --- so they went to the upper arm.

**But it is a scalpel, not a default.** Over the whole benchmark it scores 83.2%, well below the
chain alignment, because a strand of hair that hangs next to a body part wins the tally for that
part (Keqing's thigh charms landed on KeqingOpulent's twin tails; Jean's collar on JeanSea's hair).
Restricting the search to the same drawn object was tried and was worse still (71.6%): the
Head/Body/Dress split is arbitrary and does not correspond between two skins. So use it for the
groups the other modes cannot place --- a part the target skin does not have at all --- read the
share and the runner-up, and overrule it when the winner is hair or a jiggling decoration and the
part is not: a collar goes to the neck, a belt charm to the hip, whatever the tally says.

That is how the library's own gaps were filled on 2026-09-09 (every source group with no row, in
six remap directions: Fischl 0, Jean's and JeanCN's cape and collar onto JeanSea, Keqing's thigh
decorations onto KeqingOpulent, KeqingOpulent's elbow helpers onto Keqing, Ningguang's feather and
front-dress tips onto Orchid). The draft rows carry the reasoning in their comments.

<br>

## Layout

```
VGRemapFinder/
  main.py                    <- the CLI
  benchmark.py               <- scores the finder against every draft with dumps available
  requirements.txt           <- the finder's own Python dependencies (the API is separate)
  GI/
    GIVGRemapFinder.ipynb    <- the notebook, in the same shape as the other notebook tools
  src/VGRemapFinder/
    DumpMod.py               <- reads a folder's geometry (dumps, or a mod's .buf files) through the API's buf readers
    VertexGroups.py          <- one centre/spread/vertex count/object set per vertex group, and the index runs
    VGMatcher.py             <- the two metrics, the two modes, and the uncertainty scores
    DraftWriter.py           <- reads/writes Data/RemapDrafts-format workbooks
    VGRemapFinder.py         <- ties them together; the summary and the comparison report
    ComponentSplit.py        <- splits a mod's buffers across a target of several components (negative index / graph cut) and writes the .ini
    TextureBands.py          <- the texture side of a remap: a material-band table with diffuse-conditioned rows, channel edits, UV coverage
    constants/Paths.py       <- absolute path to the API, so the tool runs from anywhere
```
