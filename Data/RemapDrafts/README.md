# Remap Drafts

Rough work used for helping with finding out Vertex Group Remaps for different mods used [here](https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/data/VGRemapData.py)

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

## Rules learnt the hard way

- **Every index in column A must have a value in column B.** A source vertex group with no row is written into the remapped `Blend.buf` as a *negative* bone index (`-index-1`) with its weight kept, which the game renders as a kink or a warp ([issue #213](https://github.com/nhok0169/Anime-Game-Remap/issues/213)). A part the other skin does not have still maps to the bone that moves the skin it is attached to (a collar to the neck, a belt charm to the hip, a cape to the upper spine), never left blank. The early drafts that left such rows blank were filled on 2026-09-09, with the reasoning in column D.
- **Each direction is its own sheet**, found independently. Some early workbooks only had one; the other direction was added from the library's shipped table.
- **A sheet or workbook written by `Tools/VGRemapFinder` is marked** -- an `About` sheet for a whole workbook, or the text `Proposed by Tools/VGRemapFinder` in cell `E1` of one sheet -- so the tool's benchmark does not score it as a hand-made draft. Remove the mark once the proposal has been checked in game and is a draft in its own right.
- Not recorded here: the CN skins (Amber, Rosaria, Jean, Mona), whose remaps came from someone else, and Kirara, Raiden and Arlecchino.
