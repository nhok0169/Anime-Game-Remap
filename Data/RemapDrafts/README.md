# Remap Drafts

Rough work used for helping with finding out Vertex Group Remaps for different mods.

## Format

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
