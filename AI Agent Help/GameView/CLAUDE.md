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
result in the README and Sphinx. Stop early for only five things: the helper's UAC click (below),
a game login, anything that spends or sends, a decision the guides say is the maintainer's, and the
new download folders once step 1 of the remap pipeline has built them (commit them and stop, so the
maintainer can merge them into GitHub's `master` -- see Creating Remaps' pipeline).
Everything else, including a failed test, is yours to diagnose and retry. The final check is one
message, with the evidence (kept screenshots, `pair`s of base vs remap per mod, the warnings that
remain and why). Read the tool's README for the command reference. This file is how to use it well.

## AN ARROW KEY THIS TOOL SENT ARRIVED AS A NUMPAD KEY (fixed 2026-09-27)

`key left`, `key ctrl+left`, `key alt+down` -- every arrow, modified or not -- did **nothing**, in
total silence, while `f7`, `ctrl+f7`, `esc` and `numpad4` all worked. That pattern reads as a broken
MODIFIER and is nothing of the kind.

`sendKeyScan` asks Windows for a key's scan code with `MapVirtualKeyW(vk, MAPVK_VK_TO_VSC_EX)`,
which is documented to put `0xE0` in the high byte for an extended key, and sets
`KEYEVENTF_EXTENDEDKEY` when it sees one. Measured on this machine:

```
VK_LEFT      0x025  MAPVK_VK_TO_VSC 0x004b   MAPVK_VK_TO_VSC_EX 0x004b   extended? False
VK_NUMPAD4   0x064  MAPVK_VK_TO_VSC 0x004b   MAPVK_VK_TO_VSC_EX 0x004b   extended? False
```

It does not report it. **Bare scan `0x4B` is numpad 4** -- the navigation cluster and the numpad
share scan codes and the extended flag is the only thing telling them apart -- so every arrow was
delivered as a numpad press, which is a perfectly valid thing to press and so provokes no error
anywhere. `win32.EXTENDED_VKS` now forces the flag from a table instead of asking the OS.

This matters more than it sounds: **most mod toggles are arrows**. One Chisa mod binds all four, plus
`alt` and `ctrl` variants of each, and none of them could be driven at all.

**The lesson is how it was found, not the table.** Three reasonable-looking readings came first and
all were wrong -- "the modifier is broken", "the hotkey's `condition = $object_detected` is false",
"the mod's toggle is broken by the fix". What settled it was **rebinding the mod's own hotkey** to
one candidate at a time (`VK_F7`, `ctrl VK_F7`, `VK_LEFT`, `ctrl VK_LEFT`, `VK_NUMPAD4`) against a
single unmistakable effect -- blonde gyaru hair against black OG hair, a 67% pixel change -- so each
answer was a yes or a no rather than a judgement. `keyProbe.py` / `chordTry.py` in that session's
scratchpad are the shape of it.

**And before any of that: prove the keypress landed at all.** A diff of two screenshots said 19% of
the head had changed, which read as "the toggle worked"; painting the changed pixels showed every one
was leaves and canopy. When a toggle switches a texture, **paint the two branch files different flat
colours** -- then "did it switch" is a colour, not a statistic.

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
   **The helper runs under whichever Python started it, and screenshots need Pillow in THAT one**
   (2026-09-29). A helper started under the laptop's 3.12, which has no Pillow, drove keys fine and
   failed every capture. Ask for it to be restarted under the Python that has Pillow (`py -3`, 3.9
   there; habit 77 says to check with `py -0`), rather than installing packages into another one.
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
- **`only X --from F` does not replace a mod named X that is already in `Mods`** (2026-09-29): the
  loaded one stays, and you look at the wrong build without knowing it. Park the loaded one first
  (`only` with another name, or an empty selection), then load from `F`. Check which copy is loaded
  from `reload --mod`'s path, not from its name.
- **`restore` returns to the state when the journal STARTED**, which can include a test copy you
  loaded before an earlier restore. List `Mods` after restoring and park anything of yours that is
  still there.

### Proving a shared-code change in game: the old build against the new (2026-09-29)

When a fix changes shared code, the other characters it touches need an in-game look too (Overview
habit 83). Fix one copy of the mod with the OLD build and one with the NEW build. Stage both on the
importer's drive, e.g. `GIMI\compareFace\old\<mod>` and `...\new\<mod>`, because a cross-drive
folder is refused. Then load each in turn with `only <mod> --from <that folder>`, parking in between,
at the same camera. The outfit shop preview is the best rig for a face or a colour: the base card and
the skin card show the base mod and the remap one click apart. **Also, the character screen keeps
the model's rotation across a reload**, so a drag-to-angle done once holds for both builds. Crop the
part at full resolution from several shots into one strip per build (the idle moves the face), and
compare strips, not single frames. Delete the staged copies, or list them for the maintainer, when
you finish.

Write coordinates in any script you keep with `--space frac`. The window has been 1920x1037,
3440x1382 and 3840 wide in different sessions, and a script in `view` pixels breaks silently when
that changes.

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
  **If the orange overlay shows warnings the command did not, suspect the tool first** and grep
  `d3d11_log.txt` after the last `Reloading d3dx.ini`: a branch without the 2026-09-23 fix said
  "no warnings" for `CharlotteHurlock2` over 47 real ones.
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
- **TAKE THE DUMP FROM THE CHARACTER MENU, NOT THE OVERWORLD (the maintainer's rule, 2026-09-30;
  BOTH GAMES).** The character / Resonator screen draws the character and almost nothing else, and
  a frame dump holds *everything on screen*. The overworld does not just cost disk: it is the
  reason for two failures this repo has already paid for.
  - **It can kill the game.** ChisaParfait standing in an overworld field at WuWa 3.7 dumped
    **36857 files, 9.8 GB, 558 draw calls**, and the game was gone by the next command.
  - **An unrelated object aborts the extraction.** `wwmiExtractDump.py` runs WWMI Tools' own
    extractor, which walks *every* vb0 object in the dump and raises on the first one whose
    skeleton buffer is shorter than its highest blend index -- an NPC, a prop, a creature. The
    message names neither the object nor its hash (`skeleton of Component_0 has only 43 bones,
    while there are 83 VGs declared`), so a dump whose own character is perfectly readable looks
    exactly like a dump that is unusable.
  - And it is the same reason a register read off an overworld dump may belong to a passer-by
    rather than to the character -- Creating Remaps' "A REGISTER A DRAW INHERITED MAY BELONG TO A
    DIFFERENT CHARACTER ENTIRELY", which cost an in-game round on a yellow kimono.
  The character menu also shows the model close and lit, which is what streams its textures in at
  full size -- the other half of the LOD-bias trap in [Vertex Group Remaps](../VGRemaps/CLAUDE.md).
- **A texture that looks unchanged may be a cache, not a failed fix.** 3DMigoto can keep serving
  a texture it already loaded (Creating Remaps' "A SCREENSHOT IS EVIDENCE ABOUT THE GAME'S STATE").
  Check the written `.dds` first. If the file is right and the game is not, `close --force` then
  `launch` settles it.

## Observing a remap in game: see it yourself, before the maintainer does (the maintainer's rule, 2026-09-27)

**Be diligent and look at the details.** Most of what the maintainer reported on Neuvillette -> NeuvilletteMelusent
was visible to the agent in its own screenshots and went unnoticed: Neuvillette2's inner dress drawing flat light
blue instead of its texture, NeuvilletteMelusent1 wearing the default outfit's colours ("all the textures are
wrong") while the agent checked only its eyes, a flap clipping the leg that a single idle frame happened to hide,
and two whole rounds of screenshots of NILOU because the preview had closed back to the shop grid. Each cost the
maintainer a message and a round trip. Before you report, look for yourself:

* **Prove the page before every capture.** Read the outfit's name (a crop of the name area; compare the bright
  text pixels with a known shot) -- the preview closes back to the shop grid now and then, and a blind card
  click opens someone else's outfit. And the window can flip between 1920 and 3840 wide mid-session: take the
  scale from a fresh screenshot, never from an old one.
* **Every angle.** Drag LEFT / RIGHT to turn the character about the vertical axis (front, both sides, back),
  and drag UP / DOWN to tilt the view up and down -- the underside of a skirt or coat, the inside of a cape, the
  top of the head. A part's inside and underside are where lining, backface and clipping faults live.
* **Every distance.** Scroll a LOT: zoomed right in on the face, the hems, a flap against the leg; zoomed right
  out for the silhouette. A texture fault can be invisible at the default framing.
* **Every toggle the mod has.** Read the mod's `[Key...]` sections and cycle each one (outfit variants, merged
  master `$swapvar`s, accessories, a help menu) -- and in the outfit PREVIEW, since on the shop grid a key does
  nothing (Creating Remaps' "test mod toggles").
  **Press a mod's key with `key <k> --vk` (2026-09-27).** A mod's `key = vk_down` is 3DMigoto polling a VIRTUAL
  key, and the arrows' default path (by scan code, extended) never reached it: a whole round of toggle pairs on
  Yaoyao came back identical state after state, which reads as "the toggle works on both" and proved nothing. The
  check that catches it is a toggle whose states LOOK different -- see one change before believing any pair.
* **Over time, not one frame.** The idle animation moves the limbs: a timed series (several shots a couple of
  seconds apart) shows a clip or a fold one frame can hide.
* **Against the mod on its OWN character**, part by part -- colours, every garment, the face, the eyes.
  "Looks plausible" is not a check: a remap drawing the target's default outfit looks plausible.
* **In the overworld, for a character the account has unlocked.** Walk with `hold w 2` (WASD moves; never
  press Enter there, it opens chat), which shows cloth swinging and legs stepping as the preview never does.
  Change the time of day from the game menu's CLOCK: on Genshin it is on the menu's LEFT side bar, on WuWa on its
  BOTTOM bar -- daylight and night light a surface differently, and a lighting fault may show only in one.

Say in your report what you looked at and what you did not. A fault the maintainer has to find in a screenshot you
already took is a fault you missed.

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
| the overworld | `esc` until the minimap is back. `look DX DY` turns the camera (the cursor is captured there, so a mouse MOVE turns it; `drag` does not -- `look 500 0` is about a quarter turn), `1`-`4` switch party members, `hold w 1.5` walks |
| hide the UI | outfit preview: the eye icon bottom left. The character screen has no hide button. Crop instead |

The character screen's lighting is the same every time, which is why it beats the overworld for
before/after pairs. Record the exact steps you used (`do --file` keeps them) so the next shot
repeats the camera.

### The Genshin outfit shop, driven (Charlotte, Hu Tao; 2026-09-23/24)

The shop preview is the fastest before/after rig there is: the base card and the skin card sit side
by side (top right, view `1285,118` and `1412,118` at 3440x1382), a mod of the base shows on the first
and a remap onto the skin on the second, or the other way round. What the Charlotte pair taught about
driving it:

- **A reload (F10) sometimes drops the preview back to the shop GRID** (and sometimes does not): take a
  screenshot after every `reload` and reopen the outfit card before you click a variant card, or the
  click lands on another character's card.
- **The left / right arrows (view `88,328` / `1478,328`) step through the grid in order**, one outfit
  per click. That is the way from one character's preview to another's without leaving the shop; the
  "Character Outfits" header top left is NOT a back button. Read the title under the price after each
  step rather than counting clicks.
- **`Esc` in a preview hides the UI; a SECOND `Esc` leaves the shop for the overworld and opens the
  Paimon menu, which prints the account UID top LEFT** -- outside the 4% the tool crops. One screenshot
  of it was taken and deleted on 2026-09-24. Never press `Esc` twice in the shop, and if you must, do
  not screenshot until the Paimon menu is closed. Back into the shop from there: the Paimon menu's
  **Shop** tile (view `162,245`), then **Character Outfits** (view `130,152`), then `scroll -25 --at
  900 400` for the outfits far down the grid.
- **Both cards can play the base outfit's idle** (Charlotte's newspaper): the pose does not say which
  card is shown, and a shot right after a card click can catch the switch. Wait 12 s after a card
  click, and when two shots of "different cards" look alike, check the title before believing either.
- **A merged mod's variants cycle on its own key** (`h` / `n` for `namespace_merge.py` masters; the
  number keys for CharlotteHurlock4) and the master PERSISTS the variable: press it until you are back
  where you started. A parked master's persisted value is dropped from `d3dx_user.ini` at the next
  save (the reload says so in a NOTICE) -- restoring the folder brings the mod back, not the variant.

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
- **And `reload` is what usually undid it (2026-09-25).** F10 re-reads `d3dx.ini`, where `hunting`
  is `2`, so a reload silently puts hunting back to soft-off. The natural order --- fix the mod,
  `reload`, `compare` --- therefore produces two IDENTICAL halves every time, which reads exactly
  like a mod that does not draw. Toggle **after** the reload, not before.
- **`reload --mod`'s "no warnings" is an empty check here.** It reads `d3d11_log.txt`, and WWMI's
  call logging is off --- which it must stay (110 GB in 40 minutes). `status` shows `log: 0.0 MB`
  when that is the case, and then the line means "nothing was read", not "nothing was wrong"
  (Overview's habit 66). On WWMI the picture is the evidence; the log is not available.
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
| keys work, every screenshot fails in the helper | the helper runs under a Python without Pillow. Ask for it to be restarted under the one that has it |
| the fix changed and the game shows the old result | a same-named mod was already in `Mods`, and `only --from` kept it. Park it, then load |
| a mod's toggle key "works" and nothing changes | the key went by scan code; 3DMigoto reads a mod's `vk_...` by virtual key. `key <k> --vk` |
| the shop UI is gone, a green `VS:0/0 PS:0/0 ... IB:n/m skip` line is at the top | hunting is ON and its selected IB is the shop UI's own (`35a2ed91`), which hunting hides. GIMI's `d3dx.ini` ships `hunting = 1`, so **every `reload` turns it back on a few seconds after it returns**, and the IB selection survives. `toggle`, but only when the overlay is really there -- `toggle` flips blind, and a scripted "toggle when the page check fails" turned it ON. The overlay is ~2600 pixels of `g > 200, g - r > 40` in the band 12-42 px below the top, 640-1280 across a 1920-wide shot |

## What the first session found with it (2026-09-23)

The first `reload --mod "Bennett*"` against the maintainer's live GIMI folder listed `Unrecognised
entry: override_byte_stride` and `override_vertex_count` in every `...VertexLimitRaise...RemapFix`
section the fix writes. The old-loader `d3d11.dll` GIMI is launched with does not know those keys, so
on that install the vertex-limit raise is silently not applied. Nothing in the fix's own output
could have shown it; only the log could. It is recorded here as found and **not yet investigated**.
