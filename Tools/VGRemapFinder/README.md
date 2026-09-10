# AG Remap's Vertex Group Remap Finder

[![Static Badge](https://img.shields.io/badge/Python-254F72?style=for-the-badge)](https://www.python.org/downloads/)
[![Static Badge](https://img.shields.io/badge/Jupyter%20Notebook-F37726?style=for-the-badge)](https://jupyter.org/)

<br>

Proposes the **vertex group remap** between two mods --- the table that says which bone index of
one character's skin each bone index of the other's corresponds to --- from the two characters'
3dmigoto dumps. The output is a draft workbook in the format of
[`Data/RemapDrafts/`](../../Data/RemapDrafts/README.md), which is what used to be filled in by hand.

Runs on the API's own `VbFile`/`IbFile` dump readers, so **the API must be built first** (see
[`AI Agent Help/Setup/CLAUDE.md`](../../AI%20Agent%20Help/Setup/CLAUDE.md)). It also needs
`numpy`, and `openpyxl` for the workbook.

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
directions (`from -> to` and `to -> from`) are proposed, one sheet each.

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
| `--fromHashes`, `--toHashes` | For a raw frame analysis: the mod's `POSITION BLEND IB` hashes |
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
  GI/
    GIVGRemapFinder.ipynb    <- the notebook, in the same shape as the other notebook tools
  src/VGRemapFinder/
    DumpMod.py               <- reads a folder's geometry (dumps, or a mod's .buf files) through the API's buf readers
    VertexGroups.py          <- one centre/spread/vertex count/object set per vertex group, and the index runs
    VGMatcher.py             <- the two metrics, the two modes, and the uncertainty scores
    DraftWriter.py           <- reads/writes Data/RemapDrafts-format workbooks
    VGRemapFinder.py         <- ties them together; the summary and the comparison report
    constants/Paths.py       <- absolute path to the API, so the tool runs from anywhere
```
