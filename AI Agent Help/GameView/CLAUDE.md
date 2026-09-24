# GameView: checking a remap in the game yourself

Until 2026-09-23 every in-game check of a remap went through the maintainer: an agent asked for a
screenshot, a frame dump, or "does it still kink?", and waited. The maintainer's words: *"Im
getting tired, my wish is that when I tell them the remap and some of my mod folder locations,
asset folder locations, etc..., they can automatically go through entire remap pipeline without
me."* [`Tools/GameView`](../../Tools/GameView/README.md) is the part of that which touches the
game. It takes screenshots, sends keyboard and mouse input, reloads 3DMigoto, takes frame dumps and
chooses which mods are loaded.

**Use it instead of asking. The maintainer's stated rule (2026-09-23): "the only time I should
really intervene is at the end when they finished the entire remap, and want my final check."**
Given a remap, the mod folders and the asset folders, the whole pipeline is yours: the downloads,
the fix, testing **every mod of the character in that folder** in game, and documenting the
result in the README and Sphinx. Stop early for only four things: the helper's UAC click (below),
a game login, anything that spends or sends, and a decision the guides say is the maintainer's.
Everything else, including a failed test, is yours to diagnose and retry. The final check is one
message, with the evidence (kept screenshots, `pair`s of base vs remap per mod, the warnings that
remain and why). Read the tool's README for the command reference. This file is how to use it well.

## Before the first command

1. **Is the game running?** `py -3 main.py status` (from `Tools/GameView`). If not, `launch GIMI`
   / `launch WWMI` starts it the maintainer's way. GIMI uses its own `3DMigoto Loader.exe`, because
   XXMI's injection is blocked by Genshin. The game then sits on its door / login screen: screenshot
   it and click through. A login that asks for a password or a code is the maintainer's to do. Stop
   and ask.
2. **Is the helper running?** `status` says. Genshin runs as administrator, and Windows silently
   drops input a non-elevated process sends to it: `SendInput` reports success and nothing moves.
   Every interactive command is forwarded to the elevated helper, which **the user has to start**:
   `helper start` pops one UAC prompt. The helper lives until logoff or reboot, so that is one click
   per WINDOWS session, not per agent: check `helper status` first and ask only if it is not
   running, at the START of the work. If the prompt goes unanswered, the command fails after 60 s. Ask the maintainer to click it, or to run `helper serve` from an admin terminal. Do not
   look for a way around the prompt. A registered highest-privilege task was proposed for that and
   refused as unrequested persistence.
3. **Is hunting on?** `status` shows `hunting` per importer. `show_original` (F9, which `compare`
   uses) and `analyse_frame` (F8, the dump) only work while it is **1**. WWMI ships `2`
   (soft-off); `dump` toggles it for itself, and `toggle` does it by hand.

## The loop, per mod under test

```bash
py -3 main.py mods GIMI only CitlaliWhisper5          # park everything else; libraries stay
# ... run the fix on the mod folder (FixRaidenBoss7.py / the prototype) ...
py -3 main.py reload --mod "CitlaliWhisper5"          # F10, awaited; warnings from THIS mod only
py -3 main.py do "key esc; wait 2; ..."               # get the character on screen (recipes below)
py -3 main.py compare --crop 0.3 0.05 0.4 0.9 --space frac --name CitlaliFix
py -3 main.py crop 600 150 300 300 --name face          # look closer, no re-capture
py -3 main.py dump --label CitlaliGothRemap            # only when the picture cannot answer it
py -3 main.py mods GIMI restore                        # ALWAYS, before you finish
```

### Every mod of a character, from a folder the maintainer names

The maintainer expects a remap tested on **all** the character's mods they point you at, not one.
`--from <folder>` loads from that folder, and parking returns each mod to the folder it came from:

```bash
py -3 main.py mods GIMI scan --from "<their folder>"   # every mod folder: .ini count, already fixed?, character hint
py -3 main.py mods GIMI only "<mod 1>" --from "<their folder>"
#   fix it, reload --mod, look (the loop above) ... then the next one:
py -3 main.py mods GIMI only "<mod 2>" --from "<their folder>"   # mod 1 goes back to <their folder>
py -3 main.py mods GIMI restore                                  # at the end: everything where it was
```

- `scan`'s hint is the commonest first word of a mod's `TextureOverride` sections, and it is only
  a guess, because authors name sections after the base character while building on a skin. The fix
  run's own classification decides which mods are the character's.
- Moves are renames only. A mod folder on another drive than the importer is refused, because a
  cross-drive move is a copy plus a delete of the maintainer's original.
- The fix edits the mod IN PLACE (backups and `-u` undo are the API's), the maintainer's own way.
  Keep a list of what you fixed so the final report can say, and so an undo is one command per mod
  if they reject it.
- Pick the ORDER by structural axis (Creating Remaps' "choosing test mods by structural axis"): the
  identity mod first, then one mod per shape the fix has to handle.

- **`reload --mod` must come back empty, or with warnings you can explain.** It reads the lines
  3DMigoto logged during that reload and attributes each to the section it was parsing, which is
  what the orange overlay text cannot tell you. A new `Unrecognised entry` or `entry outside of
  section` under the mod you just fixed is a bug in the fix, found without a screenshot.
  "Empty" means the line that says **how many sections it parsed**, with a non-zero
  `TextureOverride` count. `NOTHING WAS CHECKED` is not empty, and neither is a `NOTE` about
  `namespace` .ini files, whose sections `--mod` cannot attribute. A `Duplicate TextureOverride
  hash` is listed under every section 3DMigoto names for it. For a fix that writes a second
  `...RemapFix1.ini` next to the original, expect it under both files: that is a real double
  match to explain, not noise. (Until 2026-09-23 the wait could stop while 3DMigoto was loading
  Resource files, and five Charlotte mods came back "empty" with these warnings in the log.)
- **`compare` is the default picture.** It is the same frame with mods and with every mod off
  (F9 held): same pose, camera and light. For a REMAP, "mods off" is the target's own skin, which is the
  baseline for "did the mod replace everything it should". What it does **not** give you is the
  source mod on its source character. Take that as its own screenshot, using the same camera
  recipe with the source character, and put the two together with `pair`.
- **Crop before you conclude.** A 1568 px view of a 3440 px ultrawide frame loses the detail most
  symptoms live in: a seam, a kink at the elbow, a speckled texture. `crop` re-reads the
  full-resolution shot.
- **Dump only when the picture cannot answer.** A GIMI dump is ~4 GB and 90 s. `--options "dump_ib
  txt"` (or any `analyse_options`) makes a small one for a narrow question, and the tool restores
  `d3dx.ini` byte-for-byte. Label every dump the maintainer's way (`FrameAnalysis-<What>-<time>`),
  and **never dump a character with a mod of that character installed** when the dump is for asset
  extraction: see [Vertex Group Remaps](../VGRemaps/CLAUDE.md). `mods ... only` with an empty
  selection is how you get there, and `mods ... restore` is how you come back.
- **A texture that looks unchanged may be a cache, not a failed fix.** 3DMigoto can keep serving
  a texture it already loaded (Creating Remaps' "A SCREENSHOT IS EVIDENCE ABOUT THE GAME'S STATE").
  Check the written `.dds` first. If the file is right and the game is not, `close --force` then
  `launch` settles it.

## Keeping a screenshot for the next agent

Scratch shots live in `~/.claude/gameview/shots/` and nobody else sees them. When a picture is
evidence the next agent needs (the broken state behind a guide's paragraph, a before/after pair),
add `--keep Char/Version/Name` to `screenshot` / `compare` / `crop` / `pair`. It lands as a
<=1920 px `.jpg` in [`CreatingRemaps/Images`](../CreatingRemaps/Images), in the folder's existing
convention (`Citlali/6_7/CitlaliHeadFix.jpg`), and should be linked from the paragraph it
supports. Do not keep routine shots: the folder is committed.

## Getting a character on screen (Genshin, verified 2026-09-23)

| want | how |
| --- | --- |
| a controlled studio view of a party member | overworld -> `key c` opens the character screen (verified). From the Paimon menu (`esc`) click the **Character** tile instead: `c` does nothing there. Every owned character is in the avatar row across the top, and clicking one switches. `drag` across the model turns it; `scroll` zooms |
| the wardrobe / a different outfit of that character | on the character screen, `key r` (verified). **Switch** equips the selected outfit. A remap ONTO a skin is only visible while the character wears that skin, so switching is allowed; note what was equipped and switch back before you finish |
| a skin the account does NOT own | Shop -> Character Outfits -> an outfit opens a live preview of it. **View only; see the rules below.** The eye icon bottom left hides the UI |
| the overworld | `esc` until the minimap is back. `look DX DY` turns the camera, `1`-`4` switch party members, `hold w 1.5` walks |
| hide the UI | outfit preview: the eye icon bottom left. The character screen has no hide button. Crop instead |

The character screen's lighting is the same every time, which is why it beats the overworld for
before/after pairs. Record the exact steps you used (`do --file` keeps them) so the next shot
repeats the camera.

## Getting a character on screen (WuWa, the maintainer's notes + verified 2026-09-23)

WuWa does not keep a character's base look and skins together in one shop, so **first find out
whether the account owns the character**:

| want | how |
| --- | --- |
| does the account own this character? | `esc` (game menu) -> **Gallery** -> **Crossing Stars**, and check whether the character is unlocked |
| an OWNED character, base or skin | overworld -> `key c` (character menu, verified; the first frame after it is a black fade, so `wait 5`). The roster is down the right. The shirt button at the bottom right opens **Resonator Outfits** (verified): the outfit cards are on the left, **Wear Outfit** equips one, and the eye icon hides the UI. Same switch-back rule as Genshin |
| an UNOWNED character's base look | Gallery -> Crossing Stars -> the character |
| an UNOWNED character's skin | `esc` -> **Shop** -> **Outfit Store**. **View only**: the rules below apply in full |

Two WuWa specifics about the tool:

- **Hunting ships soft-off (`hunting = 2`) and there is no on-screen overlay to read its state.**
  `toggle` flips it without knowing which way it went. `compare` and `dump` need it ON; `dump`
  toggles it for itself when no dump starts. When a `compare` comes back identical on a mod you know
  draws, toggle and try again before concluding anything.
- **A frame dump can kill WuWa.** Unreal's own watchdog ends the game with "Hang detected on
  GameThread" when a frame takes too long, and 3DMigoto freezes the frame for the whole dump. With
  XXMI's WWMI call and debug logging on (every call is written to `d3d11_log.txt`, which grew 110 GB
  in 40 minutes on 2026-09-23), the first full dump attempt was killed partway. Keep WWMI's
  logging OFF in XXMI's settings, which is the maintainer's switch to flip, not yours. Narrow the
  dump with `--options` when the question allows; that also avoids the reload wait, which never sees
  the log go quiet while call logging floods it.

## Rules for driving the game

This is the maintainer's real account, and the tool presses real keys.

- **Never click anything that spends or sends.** That covers Purchase / Buy / Confirm in the Shop,
  Wish, Battle Pass or Crystal Top-Up, and the chat box. **`Enter` opens chat** in Genshin, so do not
  press it in the overworld; `type` only into a box you have seen on screen. Treat co-op, friend
  and mail screens the same way. If the only way on is a paid or a sending step, stop and ask.
- **No screenshot may show the player's account id** (the maintainer, 2026-09-23). Both games print
  it bottom right: Genshin `UID: ...`, WuWa `User ID: ...`. The tool cuts the bottom 4% off every
  capture before anything is saved (`uidStrip` in the config), so every `crop`, `compare`, `pair`
  and `--keep` is already clean. Do not set `uidStrip` to 0. Do not `pair` or `--keep` an image the
  tool did not capture, such as one of the maintainer's own screenshots or a Windows Snipping Tool
  file, without cropping its bottom yourself first. If a kept image ever shows an id, crop and
  re-save it before committing.
- **Look before every click.** Take a screenshot after each navigation step and click only
  coordinates read off the newest view. `view` coordinates are refused if the window was resized,
  but not if the screen merely changed underneath them.
- **Leave it as you found it.** Run `mods <IMP> restore`, switch any outfit you changed back, return
  to the overworld, and do not change game settings without asking. The exception is WuWa's LOD-bias setting that
  [Vertex Group Remaps](../VGRemaps/CLAUDE.md) says a dump needs; ask first even for that. Do not
  delete frame dumps. They are the maintainer's evidence as much as yours, and several are
  referenced by name in these guides.
- **The tool takes the foreground.** Each interactive command focuses the game and then hands focus
  back. While a `do` sequence runs, the maintainer's own keyboard and mouse fight it. Batch steps
  into one `do` so each window of it is short.

## How `d3d11_log.txt` behaves (read before changing `reload` or `log`)

Each fact below was measured on the GIMI log on 2026-09-23, and each one broke a detector that assumed otherwise.

- **The log is buffered.** `d3dx.ini` ships `[Logging] unbuffered=0`, so the last few KB of anything
  3DMigoto logs stay in its memory until more is logged. A reload's real last line (`> successfully
  reloaded shaders from ShaderFixes`) was still not on disk 90 s after that reload. It lands only when
  the NEXT reload starts. **Never wait for a final line.** Wait for one that has plenty of text
  after it. `reload` waits for `> d3dx.ini reloaded`, which came after every section header and warning in
  all 15 reloads of that log.
- **"The log went quiet" is not "the reload ended".** 3DMigoto logs the `[Resource...]` sections,
  then loads every Resource file without logging anything, then logs the `[TextureOverride...]`
  sections. A 1.5 s quiet window fell in that gap, and `reload --mod` printed "no warnings" for five
  Charlotte mods that had Duplicate-hash and `Unrecognised entry` warnings in the log.
- **A warning is not always about the section above it.** `Possible Mod Conflict: Duplicate
  TextureOverride hash=...` is followed by the name of every section carrying that hash, in any mod,
  in the same `[Kind\Mods\...]` format as a section being parsed, and then `If this is intentional...`.
  Those names are part of the warning, not new sections being parsed.
- **`--mod` goes by path, and a `namespace =` .ini has no `Mods\` path.** Its sections are logged as
  `[Resource\global\ORFix\...]`. The only sign that it belongs to a mod folder is the `Processing
  "...\Mods\<mod>\x.ini"` line. `reload` and `log` name such files instead of reporting them clean.
- **The whole history is in the file.** Every reload since the game started is there, each opening
  with `Reloading d3dx.ini` and a `D3D11 DLL starting init - ... <time>` line. When a report says the
  tool got a reload wrong, **replay that reload's slice of the log through the parser** before you
  change the parser. The Charlotte report pointed at the parser, and the replay showed the parser
  was fine: the text `reload` had read was incomplete. `py -3 -m unittest discover -s tests` (from
  `Tools/GameView`) runs the parser tests on real lines and the wait tests on a fake log with the
  silent gap.
- **Another session may be driving the game.** Before you press anything, look at the newest
  `D3D11 DLL starting init` time and the log's size. If a reload happened minutes ago and you did not
  do it, someone else is testing, and an F10 from you lands in the middle of their check. A log-only
  question is answered with `log --problems --mod`, which reads without pressing anything.

## When it does not work

| symptom | cause |
| --- | --- |
| a command "worked" and nothing happened in game | the input did not reach an elevated game. Check that `status` says `helper: running`; the helper route is automatic when it is |
| `could not bring ... to the foreground` | a UAC prompt or another elevated window is on top, or the game is minimised. `screenshot --screen` shows the desktop |
| `no frame dump started` | hunting off (`status`), a menu of the OS on top, or F8 pressed during a reload. The tool waits the reload out and re-presses twice, so the first two are the usual cause |
| `reload` warns the log never said `Reloading d3dx.ini` | the key did not arrive (focus), or the importer logs nothing |
| a black screenshot | exclusive fullscreen, HDR, or a loading screen. Try `--method print`, or set the game to borderless |
| clicks land in the wrong place | the view is from before a resolution change, or the click was in `client` space with `view` numbers. Take a fresh screenshot |
| the game vanished (WuWa) | read the newest `Client/Saved/Crashes/*/CrashContext.runtime-xml` under the game folder. Its `Client.log` is encrypted, but this file's `ErrorMessage` and call-stack module names are plain text. "Hang detected on GameThread" means a frame dump ran too long; `0xc0000417` with `d3d11` on top is 3DMigoto itself |
| every command takes ~3 s | two Python start-ups (client + helper child) and a focus hand-off. Put steps in one `do` |

## What the first session found with it (2026-09-23)

The first `reload --mod "Bennett*"` against the maintainer's live GIMI folder listed `Unrecognised
entry: override_byte_stride` and `override_vertex_count` in every `...VertexLimitRaise...RemapFix`
section the fix writes. The old-loader `d3d11.dll` GIMI is launched with does not know those keys, so
on that install the vertex-limit raise is silently not applied. Nothing in the fix's own output
could have shown it; only the log could. It is recorded here as found and **not yet investigated**.
