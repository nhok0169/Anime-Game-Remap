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

## Where the data lives, and which copy to trust for what

| | what it is | trust it for |
| --- | --- | --- |
| `core/src/data/VGRemapData.cpp` | **the live table**, 52 rows, both directions of every pair | what ships. Confirmed against fresh frame dumps and, for the pairs with drafts, against the drafts |
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
   vertices changed index and no weight changed. `Importer/GIMI/Mods/overrideVgRemap.py --ab`
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
   `Blend.buf` to count how many vertices use the gap.
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
