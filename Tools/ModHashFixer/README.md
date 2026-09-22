# Mod Hash Fixer

**Your WuWa mod's textures look broken? Its hashes have probably gone stale, and nothing is corrupt.**

WWMI binds a mod's textures by hash --- `[TextureOverrideTexture<N>] hash = <h>`. Wuthering Waves
**rehashes** a texture between game versions, so once the hash the author exported no longer matches
anything the game emits, the override never fires and the surface draws with the **game's own art**
instead of the mod's. The mod is addressing textures that no longer exist under those names.

This tool resolves each stale hash **backwards** to the role it played (`upperDiffuse`, `hairNormal`,
...) and then **forwards** to the hash that role has on the current version, using the hash history
AGRemap already carries for every character it knows (`core/src/data/HashData.cpp`). It needs no
frame dump. It is the Wuthering Waves counterpart of what ORFix does for Genshin every version.

It **reports first and writes nothing**, keeps one backup per `.ini`, and can undo itself.

<br>

## Using it

```bash
python3 main.py "<mod folder>"                 # report only -- writes nothing
python3 main.py "<mod folder>" --apply         # write it, keeping a backup per .ini
python3 main.py "<mod folder>" --undo          # put the backups back
```

| option | |
| --- | --- |
| `-c`, `--character` | the character the mod is FOR. Detected from the hashes themselves when omitted, which is what you want: a mod folder is named after whatever its author or the downloader called it |
| `-v`, `--version` | the game version to bring the hashes to (default: the newest the library knows) |
| `-a`, `--apply` | write the change. Without it the run only reports |
| `-u`, `--undo` | restore the `.ini` files this tool backed up |
| `-m`, `--maps` | your own old -> new hash tables (json, any nesting). These **win** over the library |
| `-g`, `--geometry` | also retarget `vb0` / `cb4` / the shape-key pair. **Only for a mod that does not draw AT ALL** |
| `--no-by-file` | switch off the second route below, so only the recorded history is used |

There is a notebook too, at [`WuWa/WuWaModHashFixer.ipynb`](WuWa/WuWaModHashFixer.ipynb).

<br>

## The second route: the mod's own texture FILE

**The body and the legs are the two roles the hash history is most likely to miss**, and that is not
bad luck. The history was largely built by correlating mod files against the game's own textures --
which only identifies a file a mod ships UNCHANGED. A mod that *repaints* the torso never matched, so
a run could report every accessory fixed while the body stayed broken. That is what this route is for.

When a hash is older than the recorded history, the FILE that section names is typed instead:

* **the component**, from WWMI's own export name `Components-<N> t=<hash>.dds`, which says which
  component the mod binds it for --- and the file must be named for *that section's* hash, so a
  section pointing at another texture's file is refused;
* **the kind** --- mask, normal map or diffuse --- from the pixels, but only as a claim about the
  component as a whole: a file is the diffuse when it is the **only** one of that component's
  textures that reads as one, and only where all three of that component's roles are known. A
  component whose textures do not separate that cleanly abstains.

Anything resolved this way is listed separately in the report, because it is an **inference where the
history is a record** --- so it is the first thing to look at if something still renders wrong.
`--no-by-file` turns it off.

**Scored before it was trusted.** Every hash the *history* explains is a labelled example, so the
route was run on those same files and compared: over 76 mod folders it agreed 121 times, abstained
214 times and disagreed **0** times. Its first two versions disagreed 24 times and 1 time, and both
were rejected --- typing a file on its own confused a hair normal with a hair mask and a lower
diffuse with a lower normal, because the thresholds had been calibrated on the game's textures rather
than on repaints of them.

<br>

## A character this does not cover

The history only reaches the characters AGRemap has registered --- but **a character it does not
carry is not out of reach**. If you already know which old hash became which new one, hand over the
table and it is used in preference to the library:

```bash
python3 main.py "<mod folder>" --maps hash_maps.json --apply
```

`--maps` takes json of any nesting and picks up every `"oldhash": "newhash"` pair at any depth,
because the community fixers publish these with no agreed layout. From Python (and in the notebook)
the same thing is `ModHashFixer(mod, extraHashes = {"37250244": "f2646d21"})`, and
`ModHashFixer.loadHashMaps(*paths)` builds that dict from the json files.

Anything resolved that way is reported as **`custom (yours)`**, so a run never hides which answers
came from the library and which from you. It is also the way to fix the one case the library cannot:
a texture whose older hash was never recorded shows up as *unrecognised*, and one line in your own
table resolves it.

If you want the character covered for everyone rather than in your own table, the lasting place is a
row per `(version, character, role)` in the library's `HashData` --- that is what lets every later
version be resolved through it. `Tools/Misc/Diagnostics/chisaHashHistory.py` shows the shape.

<br>

## A resource that names a file the mod does not ship

**This is worse than a stale hash, and it looks like nothing at all.** A stale hash means the
override never fires. A *dangling filename* means it fires and binds **nothing**, so the surface
draws with the game's own art --- while every hash in the file reads correctly and every check that
follows a hash passes.

Chisa16's lower body was exactly that. `ResourceTexture15` named `Components-4 t=0c153c12.dds`,
which the mod does not ship, while `Components-4 t=ffa1f581.dds` --- named for that very section's
hash --- sat beside it referenced by nothing. So the game's own mask shaded the mod's legs, and
because the game's mask is laid out for Chisa's own outfit, her **stockings were shaded as bare
skin**.

The repair is offered only where the evidence is exact: the section's own `hash` names a file the
mod ships, under WWMI's `Components-<N> t=<hash>.dds` **exactly**, and there is only one such file.
Two things that rule out:

* a file with anything after the hash --- `Components-4 t=21f813ba off.dds` is how a modder
  **disables** a texture, and a real Chisa mod has one sitting beside a live one. Repairing to it
  would switch back on something its author switched off;
* a file of the same name elsewhere in the tree (an LOD subfolder), where the path is the author's
  business and renaming the file to itself repairs nothing.

It runs before the hash work, because the file route above types the file a section *names* --- so a
section naming nothing can be typed by nothing, and the two fixes only compose in that order.

<br>

## Reading the report

* **to update** --- resolved to one of this character's roles, and has moved since.
* **already current** --- resolved, and already right. A mod where everything is current does not have
  this problem, and whatever is wrong with it is something else.
* **geometry left alone** --- `vb0` / `cb4` / the shape-key pair, skipped by default. A mod whose
  `vb0` is stale does not draw **at all**, which is a different symptom from broken textures, and
  rewriting those on a mod that *does* draw breaks what works.
* **typed from the file** --- older than the recorded history, and resolved by the second route
  above. An inference rather than a record: check these first if something still looks wrong.
* **unrecognised** --- not this character's, or older than the recorded history AND not typeable from
  its file. Left alone, never guessed. **This is the honest limit of the tool's coverage**: if a
  texture you care about is here, add a line to your own `--maps` table.

<br>

## What it will not do

* **It will not invent a hash.** A hash it cannot resolve by the history, by your own table, or by
  the file route's own standard of evidence is reported and left alone. Extending coverage properly
  means adding rows to `HashData.cpp` --- `Tools/Misc/Diagnostics/chisaHashHistory.py` is how Chisa's
  older generations were derived, from the mods themselves and on hash-level evidence only.
* **It will not touch a previous fix's sections.** Anything whose section name carries `Remap` holds
  the *target's* hashes and is correct.
* **It will not invent a file.** A resource naming a `.dds` the mod does not ship is repaired only
  where the evidence is exact --- see below --- and otherwise reported and left alone.
* **It changes nothing about geometry, weights or shaders.** If the mod's shape is wrong, or a part
  is missing, the hashes are not your problem.

<br>

## Why a remap can look *better* than the mod it came from

Worth knowing, because it looks like a contradiction: AGRemap's own WuWa fixes bind textures **by
register** on the target's draws rather than by hash, so a remap shows what the author painted while
the original --- with its stale hashes --- shows the character's own textures. If a base screenshot
disagrees with a remap that way, suspect this before suspecting the remap. See Creating Remaps'
*"A MOD WITH EVERY HASH STALE"*.
