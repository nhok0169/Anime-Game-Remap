# Handoff: prove the WWMIFixer refactor byte-identical on real WuWa mods

For an agent on the computer that holds the maintainer's Wuthering Waves mods. Branch `add-miyabi`.

## What is being proved

`WWMIFixer.cpp` is being moved onto library edits ("a fixer is a list of edits", repo `CLAUDE.md`). The
refactor must not move a single output byte. Commit **`e0ca4f97`** is the first WWMI slice; its parent
**`e359867a`** is the baseline (everything before it in the WWMI fixer is untouched). Locally it was proved
on identity mods and synthetic variants only -- 28/28 SAME -- because that machine has no real WuWa mods.
Your sweep over the real mods is the actual acceptance test.

## Steps

1. **Build the baseline.** `git switch --detach e359867a` in a worktree of your own (habit 123: never stash,
   reset or clean the shared checkout), build the API as `AI Agent Help/Building/CLAUDE.md` says
   (`py -3 Tools/APIBuilder/main.py -i`, from a VS developer prompt on Windows), check the build log has no
   errors and that the `.pyd`'s mtime moved, then copy `Anime Game Remap (for all users)/api/src/py/FixRaidenBoss2`
   to `<scratch>/pkgA/FixRaidenBoss2`.
2. **Build the refactor** the same way at `e0ca4f97` (or the branch tip, if later WWMI slices landed -- then
   name the commit you built in your report) and copy it to `<scratch>/pkgB/FixRaidenBoss2`.
3. **List the corpus**: one mod folder per line in `wwmi.txt` -- every WWMI mod folder you have, of every
   WuWa character (all eight registered characters are remap sources or targets, and a mod of an
   UNREGISTERED character must also come out SAME: it should be untouched). Include the mods in `WWMI/Mods`
   and the ones parked in its parent. Put a folder over ~4000 files on a line of its own and run it last.
4. **Sweep**:
   ```
   set PYTHONIOENCODING=utf-8
   py -3 Tools/Misc/Diagnostics/regressBuilds.py wwmi.txt <scratch>/sweepWwmi --a <scratch>/pkgA --b <scratch>/pkgB --jobs 3
   ```
   `PYTHONIOENCODING` matters: a non-ASCII folder name otherwise crashes the console print mid-run.
   It copies each folder twice, never touching the originals.
5. **Then once more with downloads on** for every folder that touches a fallback or game-texture download
   (`--download normal`, habit 122) -- the local proof ran with downloads disabled only.
6. **Optional, cheap: the local corpus**, to confirm your two builds are the same two builds:
   `py -3 Tools/Misc/Diagnostics/wwmiIdentityFromDownload.py "Data/Mod Downloads/WuWa/<Name>/<ver>" <Name> <out>/<Name>Identity`
   for each character, and `wwmiSynth.py <identity> "Data/Mod Downloads/WuWa/<Name>/<ver>/<Name>TextureUsage.json" <out>/<Name>Bound bound`
   (and `toggle`). All must be SAME.

## Reading the result

Every line must be `SAME`. A `DIFF` keeps both copies in `<scratch>/sweepWwmi/A|B/<folder>`: diff the first
differing files it names and report the folder, the file and the first differing lines (or bytes for a
`.buf`). Do **not** fix anything -- hand the DIFFs back; an `ERR` likewise, with its traceback. `MISSING`
means the folder moved (the maintainer moves mods between `WWMI/Mods` and its parent).

## Where a DIFF is most likely (the paths no local input exercised)

- RabbitFX pinning on a real RabbitFX mod; the mod's own `ps-t` lines being re-keyed.
- `RegAnchorAdd` matches keys **case-insensitively** where the old mirror-twin / bypass-restore code matched
  exactly -- a mod writing `Ps-t0` or `DrawIndexed` in odd case is the suspect.
- `meshHashesIn` now reads through `IniScan` (a section header must close with `]`);
  `presentResetsObjectDetected` follows the parsed file rather than the raw text.
- Malformed index / texcoord / position buffers now raise a `BufFile` error instead of being read partially.
- Legacy pre-merged-skeleton mods (`liftLegacyBlend`), sibling and same-draw `.ini` files
  (`readSiblingRecolours`, `readSameDrawInis`, `editFileTag`), namespace-merged mods and merged masters,
  `offPasses`, texcoord straddle fold, copies of one draw across files, older and live hash generations,
  mod-manager-packaged mods (GUID `.assets`), LOD folders, 16-bit index buffers, toggled `ps-t` beyond `ps-t0`.

## Rules

No git writes in the shared checkout; no game, no GameView; never run the CLI over a real mod folder in place
(the sweep copies); no background Python left running while building (it locks `core.pyd`).

## Report

The two commits built, the corpus size, the SAME / DIFF / ERR / MISSING counts for each run, and every DIFF
or ERR in full.
