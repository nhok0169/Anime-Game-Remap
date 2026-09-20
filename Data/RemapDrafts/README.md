# Remap Drafts

Rough work used for helping with finding out Vertex Group Remaps for different mods used [here](https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/data/VGRemapData.py)

<br>

## Format

> [!TIP]
> After reading the format requirements below, it might be better to look at a few of the Excel files in this folder
> to be more familiar with the the format of the files

<br>

- Each Excel file represents the remap for a single character
- Each sheet within an Excel file represents the data for a single remap in one direction, for a particular game version<br>*eg. Keqing --> KeqingOpulent (version 4.8)*

<br>

- A sheet contains the following columns:

### Columns
 
| Column Letter | Column Name | Description |
| --- | --- | --- |
| A | [Name of mod to be remapped] | The Vertex Group Indices of the mod to be remapped |
| B | [Name of the remapped mod] | The corresponding Vertex Group Indices of the remapped mod that matches the index of the mod to be remapped |
| C | Uncertainty | A floating point number from 0 - 1 where: <br> <br>  0 means you are confident that the index at column B matches the index at column A <br>AND<br>   1 means that the index at column B is basically randomly guessed to match with the index at column A <br> <br> By default, assume a blank value in this column to represent 0. <br> <br> Usually, we will check indices with a higher uncertainty number first if something goes wrong. |
| D | Comments | Any comments worth noting about a particular index. This column is pretty useful for debugging purposes. |

<br>

- For column B, add a conditional formatting yellow highlight for duplicate indices, which indicates that many different indices in the mod to be remapped map onto the same index of the remapped mod.

<br>

### The `Credits` sheet

- **Every workbook opens with a `Credits` sheet** (the first sheet). Its layout, copied from the hand-made workbooks: `A1` = `Authors` (bold, 20 pt), `A3` = `Name` (bold), then one author per row from `A4` down, each cell a hyperlink to that author (the maintainer's row is `AlbertGold#2696`, linking to their Discord profile). A workbook without one gets one added, with the maintainer's row first.
- **A member of [The Council of CLAUDE Agents](https://github.com/nhok0169/Anime-Game-Remap/blob/development/AI%20Agent%20Help/README.md) who edits a workbook adds a row of their own** under `Name`, in the form `<Council name>: The <nth> member of The Council`, the whole cell hyperlinked to that Council page (for example `🦴🧭 The Vertex Group Cartographer: The 32nd member of The Council`). `n` is the value the Council's counter took when the member joined -- the number in their "Join The Council" commit -- not the row's position in the member list, since one badge can carry a count of 2.
- **An agent that has not joined The Council yet adds nothing**: join first (the ritual is in `AI Agent Help/Overview/CLAUDE.md`), then come back to every workbook edited and add the row. Editing a workbook means changing any sheet in it -- filling gaps, adding a direction, adding a proposal sheet from `Tools/VGRemapFinder`.
- The `Credits` sheet is not a remap sheet, and `Tools/VGRemapFinder`'s readers (`benchmark.py`, `DraftWriter.readDraft`, `ComponentSplit`) skip it because its first row is not a `from | to` header pair (`A1` = `Authors`, `B1` empty), not because of its name. Keep it that way: a character name in its first row would make it read as a remap sheet.

<br>

## Rules learnt the hard way

- **Every index in column A must have a value in column B.** A source vertex group with no row is written into the remapped `Blend.buf` as a *negative* bone index (`-index-1`) with its weight kept, which the game renders as a kink or a warp ([issue #213](https://github.com/nhok0169/Anime-Game-Remap/issues/213)). A part the other skin does not have still maps to the bone that moves the skin it is attached to (a collar to the neck, a belt charm to the hip, a cape to the upper spine), never left blank. The early drafts that left such rows blank were filled on 2026-09-09, with the reasoning in column D.
- **Each direction is its own sheet**, found independently. Some early workbooks only had one; the other direction was added from the library's shipped table.
- **A sheet or workbook written by `Tools/VGRemapFinder` is marked** -- an `About` sheet for a whole workbook, or the text `Proposed by Tools/VGRemapFinder` in cell `E1` of one sheet -- so the tool's benchmark does not score it as a hand-made draft. Remove the mark once the proposal has been checked in game and is a draft in its own right.
- **A skin of several components (YelanTranquil: Body, Bang, Eye) gets one column per component**, headed by the skin's name with the component appended (`YelanTranquilBody | YelanTranquilBang | YelanTranquilEye`), exactly one of which is filled per row, since each component has its own vertex group numbering. When such a skin is the one being remapped *from*, each component gets its own sheet, headed the same way in column A. `Tools/VGRemapFinder` reads and writes this layout, and the library stores it as one row per (source component, target component).
- **A bone no vertex uses still gets a row**, with `0` as a placeholder and the reason in its comment (Sanhua's 24, SanhuaExorcist's 19: merged indices no component's `vg_map` reaches). Nothing can map through such a bone, so the value is a convention that keeps the row complete for transcription; `Tools/VGRemapFinder`'s comparison does not score those rows.
- **`SanhuaRemapDraft.xlsx` is a Wuthering Waves pair** (`SanhuaExorcist` is WWMI-Assets' `SanhuaSkin1`), and its indices are WWMI's **merged skeleton** -- the space `Metadata.json`'s per-component `vg_map` maps into and the one a WWMI mod's blend buffer is written in -- not any one component's. The forward sheet is hand-made (its three blank rows with vertices were filled from the geometry, saying so); the `(tool)` forward sheet and the reverse sheet are the finder's, marked in their header cell, the reverse one reviewed row by row and not yet checked in game.
- **`ChisaRemapDraft.xlsx` is the second Wuthering Waves pair** (2026-09-20), both directions, in the
  same merged-skeleton space. Unlike Sanhua's it is **entirely the finder's proposal** -- no hand-made
  sheet exists to check it against and it has not been checked in game -- so the `About` mark stays on
  the workbook until it has. Chisa is 419 vertex groups (369 with vertices) and ChisaParfait 251 (198);
  the placeholder rows are 50 and 53. Her geometry comes from the frame dumps that
  `Data/Mod Downloads/WuWa/Chisa/2_8` and `ChisaParfait/3_5` were built from, since WWMI-Assets has
  neither character, and reading it needed the finder to learn the **eight-influence** blend layout
  (`R8_UNORM`, and an element whose real width is the distance to the next one).
- Not recorded here: the CN skins (Amber, Rosaria, Jean, Mona), whose remaps came from someone else, and Kirara, Raiden and Arlecchino.
