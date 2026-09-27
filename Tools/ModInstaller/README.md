# ModInstaller

Extracts a folder of downloaded mod archives into a mods folder, one numbered folder per archive:
`<Name>1`, `<Name>2`, ... It is meant for "I downloaded a pile of Charlotte mods, put them in my
mods folder", so an agent can do the unpacking without asking.

```bash
py -3 main.py <archive folder> <mods folder> <character name> [--dryRun]
py -3 main.py "C:/Users/me/Downloads/Charlotte" "E:/Computer/Games/.../GIMI/Mods" Charlotte --dryRun
```

Standard library only; no `requirements.txt` to install. Run it from anywhere.

## What it does

| Case | Behaviour |
| --- | --- |
| `.zip` | Python's `zipfile`. A name without the zip UTF-8 flag is retried as UTF-8, then `--zipEncoding` (default `gbk`; `shift_jis` or `cp949` for Japanese / Korean authors) |
| `.rar` (WinRAR) | WinRAR's `UnRAR.exe` / `Rar.exe` if installed, otherwise Windows' `tar` (bsdtar reads RAR4 and RAR5) |
| multi-volume `x.part1.rar`, `x.part2.rar`, ... | installed once, from `part1`; the later parts are not counted as mods |
| `.7z`, `.tar*` | Windows' built-in `C:\Windows\System32\tar.exe` (bsdtar). A Git-for-Windows GNU `tar` cannot read `.7z`, so the system one is preferred |
| wrapper folders | an archive holding only `ModName/` (or `a/b/ModName/`) lands as `<Name><i>/<contents of ModName>`. An archive with several things at its top level is kept as-is |
| junk | `__MACOSX`, `.DS_Store`, `Thumbs.db`, `desktop.ini` are dropped |
| numbering | continues after the highest `<Name><i>` or `DISABLED<Name><i>` in the mods folder, **its parent** (where parked mods live; `--noParentCheck` to skip), and any `--check DIR`. An index is never reused |
| re-running | each installed folder gets a `.modInstall.json` holding the archive name and sha256. An archive whose sha256 is already installed is skipped (`--reinstall` to force), so running it again over a growing Downloads folder installs only the new ones |
| failures | a password-protected, corrupt or empty archive is reported `FAILED` and leaves nothing behind. The exit code is 1 if anything failed |
| not a mod | a folder with no `.ini` is installed but flagged `WARNING` (a texture pack, or a mod inside a nested archive, which is listed) |

Archives are extracted into a temporary `.modInstall-*` folder inside the mods folder and then
renamed into place, so a failed extraction never leaves a half-written `<Name><i>`.

## Verified (2026-09-23)

Against a scratch fixture: a zip with a wrapper folder and `__MACOSX`, a WinRAR `.rar` with two
wrapper levels, a 4-volume `.rar`, a `.7z` with loose files, a zip with GBK names and no UTF-8
flag, a zip with no `.ini`, a corrupt zip, a password-protected `.rar` (fails, does not prompt),
with `Charlotte2` in the mods folder and `DISABLEDCharlotte5` in its parent. Numbering started at
6, payloads were byte-identical to the sources, no temporary folder was left behind, and a second
run skipped all six installed archives.
