# GameView

Lets an agent **look at and drive the game itself**: screenshots, keyboard and mouse, 3DMigoto
reloads, frame dumps, and which mod folders are loaded. An in-game check of a remap no longer
needs the maintainer at the keyboard.

Windows only (it drives a Windows game). Python 3.8+ with Pillow (`pip install -r requirements.txt`).
No pywin32: the Win32 layer is plain `ctypes` (`GameView/win32.py`).

**Agents: read [AI Agent Help/GameView](../../AI%20Agent%20Help/GameView/CLAUDE.md) first.** It has
the in-game verification loop, the safety rules (the Shop spends real currency), and the
recipes for getting a character on screen. This file is the command reference.

## One-time and per-session setup

```bash
py -3 main.py setup          # discovers the importers from the XXMI Launcher's config
py -3 main.py status         # hotkeys, hunting mode, loaded mods, what is running, the helper
py -3 main.py helper start   # ONE UAC prompt for the user; needed whenever the game runs elevated
```

**Why a helper.** Genshin runs as administrator. Windows (UIPI) silently drops keyboard and mouse
input a non-elevated process sends to an elevated window: `SendInput` still reports success, and
nothing happens. Screen capture of it is refused too. So when the game is elevated and the caller
is not, every interactive command is forwarded to `helper` -- a small localhost server the USER
starts elevated, once per session -- which re-runs the same command line in a fresh child and hands
back the output. It installs nothing, keeps no state beyond `~/.claude/gameview/helper.json`, and
ends with `helper stop` or at logoff. Commands that do not touch the game (`status`, `mods`,
`log`, `crop`, `pair`) always run locally.

`helper start` pops a UAC prompt that the user must approve. If nobody is there to click it, the
command fails after 60 s. The alternative is `py -3 main.py helper serve` in an admin terminal.

## Where things go

| what | where |
| --- | --- |
| scratch screenshots | `~/.claude/gameview/shots/` (`<time>_<name>.png` full size + `.view.png` at most 1568 px) |
| screenshots worth keeping for future agents | `--keep Char/Version/Name` -> `AI Agent Help/CreatingRemaps/Images/Char/Version/Name.jpg` |
| config (edit when discovery guesses wrong) | `~/.claude/gameview/config.json` |
| frame dumps | the importer folder, as 3DMigoto writes them, renamed `FrameAnalysis-<label>-<timestamp>` |
| mod moves journal | `~/.claude/gameview/mods-journal-<IMPORTER>.json` |

Override the scratch folder with `AGREMAP_GAMEVIEW_HOME`.

**Every capture has its bottom 4% cut off before it is saved** (`uidStrip` in the config,
`GameView/game.py`'s `hideUid`). Both games print the player's account id there, and no
screenshot may carry it. Only the bottom is cut, so `view` click coordinates are unaffected;
verified on a probe window, where 800x500 captured as 800x480 and a `view` click at (400, 250)
landed at (400, 250).

## Commands

Everything that touches the game takes `-i GIMI|WWMI` (default: whichever game is running), and
`--stay` to leave the game in front afterwards. By default the previous foreground window gets
focus back.

**Coordinates** (`click`, `move`, `drag`, `scroll --at`, `--crop`) are in `--space view` by
default: pixels of the last `.view.png` taken of the game window, which is the picture the agent
is looking at. The tool refuses if the window has been resized since then. Other spaces are
`client` (full-resolution window pixels), `frac` (0..1 of the window) and `screen`.

| command | what it does |
| --- | --- |
| `screenshot [--name N] [--crop X Y W H] [--keep ...] [--original] [--screen] [--method auto/blt/print]` | capture the game's client area; `--original` holds show_original (F9) = every mod off; `--screen` = the whole desktop (launcher dialogs); `print` = no focus change |
| `compare [--crop ...] [--name N]` | the same frame with mods and with F9 held, labelled, stacked. **Same pose, camera and light**: the cheapest honest A/B the game gives |
| `crop X Y W H [--from IMG]` | zoom into the last full-resolution shot without re-capturing |
| `pair A.png B.png [--labels a b] [--crop fx fy fw fh]` | any two existing images in one labelled picture (base vs remap from separate runs) |
| `key c` / `key esc esc` / `key ctrl+f10` / `key VK_OEM_4` | chords; names, Windows `VK_` names, or `0x..` codes. `--hold ms`, `--gap ms` |
| `hold w 1.5` | hold a key (walk) |
| `type "text"` | unicode text into a focused text box |
| `click X Y [--button right] [--double]`, `move X Y` | mouse |
| `drag X1 Y1 X2 Y2 [--duration s]` | press, glide, release: turn a model on the character screen |
| `look DX DY` | relative mouse motion: turns the overworld camera |
| `scroll N [--at X Y]` | wheel notches (+ = zoom in) |
| `wait S` | pause (inside `do`) |
| `do "step; step; ..."` or `do --file steps.txt` | several steps, one focus, one helper round trip |
| `reload [--mod GLOB] [--wait S]` | F10, **wait until 3DMigoto logs `> d3dx.ini reloaded`** (every section and warning comes before it), then list the log's warnings **grouped by the section they belong to**. A duplicate hash is listed under **every** section 3DMigoto names for it, so a conflict between two mods shows under both. `--mod` keeps only `Mods\<GLOB>`. It never prints a bare zero: a clean result says how many sections (by kind) it parsed, `NOTHING WAS CHECKED` means none of the mod's sections were in the text, and a `NOTE` names the mod's `.ini` files that log under a `namespace` (which `--mod` cannot attribute) |
| `log [--problems] [--mod GLOB] [--tail N]` | the importer's `d3d11_log.txt`, read from the end (WWMI's has reached 91 GB) |
| `dump [--label L] [--options "dump_ib txt"]` | F8, re-pressed if nothing reacts; waits for the log's `Frame analysis saved to`; renames the folder. `--options` swaps `analyse_options` for this dump only and restores `d3dx.ini` byte-for-byte |
| `toggle` | toggle hunting (numpad 0). WWMI ships `hunting = 2`, where F8/F9 are dead until toggled; `dump` does this itself |
| `mods IMP list/load/park/only/restore [names/globs] [--from DIR]` | move mod folders into and out of `Mods/`. Loading is from the importer folder (the maintainer's parking convention) or `--from` any folder, e.g. every mod of one character. Parking returns each mod to the folder it was loaded FROM. `only` keeps library folders (`protectedMods` in config). Every move is a same-drive rename, journaled, and `restore` undoes them all |
| `mods IMP scan [--from DIR]` | one row per mod folder: `.ini` count, hash count, already fixed?, and a character hint from its `TextureOverride` names (a guess, not a classification) |
| `launch GIMI` / `launch WWMI` | GIMI: its own `3DMigoto Loader.exe`, then `GenshinImpact.exe` (the maintainer's way: XXMI's injection is blocked by Genshin). WWMI: `XXMI Launcher.exe --nogui --xxmi WWMI`. Waits for the window |
| `close [--force]` | ask the game to close; `--force` terminates it |
| `windows` | list top-level windows (for `-w REGEX`, which drives any window: used to test the tool itself) |
| `clean [--days 3]` | delete old scratch screenshots |
| `helper start/stop/status/serve` | see above |

Every hotkey is read from the importer's own `d3dx.ini` `[Hunting]` section (`reload_config`,
`analyse_frame`, `show_original`, `toggle_hunting`), so a rebinding there is followed.

## How it was verified (2026-09-23, Genshin 3440x1440 borderless, old-loader GIMI)

- **Input:** a Tk probe window logged every key (including `shift+a`, `ctrl+f10`, numpad 0), both
  buttons at the requested client pixels, the wheel, and a drag's press and release points.
  Against the game: an outfit-screen model turned, the UI hid, Esc and menus navigated, the
  overworld camera turned 90 degrees with `look`.
- **Capture:** pixel-exact on the probe. Against the game: the full 3440x1440 client area through the
  helper. **Without the helper, capture of the elevated game fails with access denied**, and input
  would be silently dropped.
- **`compare`:** F9 held showed the unmodded model, released showed the mod.
- **`reload`, fixed the same day:** the first version waited for the log to go quiet for 1.5 s. 3DMigoto
  logs the `[Resource...]` sections, then loads every Resource file **without logging anything**, then
  logs the `[TextureOverride...]` sections. `reload --mod` printed "no warnings" for five Charlotte mods
  whose Duplicate-hash AND `Unrecognised entry` warnings were in the log; that both kinds were missing
  says the text stopped before the TextureOverride sections, i.e. in that gap (inferred from the log,
  not timed live). It now waits for `> d3dx.ini reloaded`, which came after every section and warning in all
  15 reloads of the GIMI log. The reload's real last line cannot be used: with `[Logging] unbuffered=0`
  its last few KB stay in 3DMigoto's buffer until the next reload. A replay of the log, checked against
  each Duplicate block's own list of sections, matched for all 11 Charlotte reloads.
  `tests/` (`py -3 -m unittest discover -s tests`) pins the parser on real log lines and the wait on
  a fake log with the gap.
- **`reload`:** waited out a multi-second reload. `--mod Bennett*` listed 18 real warnings, and
  `--mod Citlali*` listed none. (The Bennett ones include `Unrecognised entry: override_byte_stride` on the fix's
  `VertexLimitRaise` sections, because the old-loader `d3d11.dll` predates that key.)
- **`dump`:** a full dump (8023 files, 4.1 GB) ended on the log line, and a `--options "dump_ib txt"` dump
  (136 files) left `d3dx.ini`'s md5 unchanged. The first version pressed F8 while the options
  reload was still running, and that press was lost. That is why the reload is now awaited and F8 is
  retried.
- **`mods`:** a park/restore round trip left the folder where it started and the journal empty.
  `only --from` over a scratch importer and a character folder moved each mod back to its own
  folder on the next swap, and `restore` returned all of them.
- **WWMI (same day, after a reboot):** `launch WWMI` brought the window up in 35 s. `key c`, the
  Resonator Outfits button and `compare` all worked. Two things found:
  - **Dump:** a full WWMI dump with XXMI's call and debug logging on froze the frame long enough for
    Unreal's watchdog to kill the game ("Hang detected on GameThread"), leaving a partial folder.
    The same logging put every log line under `FrameAnalysisContext(...)`, which the first
    "dump started" pattern mistook for a dump in progress, so it never re-pressed F8. The pattern now
    requires 3DMigoto's `Frame analysis` with the space.
  - **Helper after a reboot:** a ping holding the previous session's token got "bad token" from the
    new helper and was taken for an answer. A ping now needs a pid in the reply.
- **Not yet run live:** `close`, a completed WWMI dump.
