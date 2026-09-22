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

There is a notebook too, at [`WuWa/WuWaModHashFixer.ipynb`](WuWa/WuWaModHashFixer.ipynb).

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

## Reading the report

* **to update** --- resolved to one of this character's roles, and has moved since.
* **already current** --- resolved, and already right. A mod where everything is current does not have
  this problem, and whatever is wrong with it is something else.
* **geometry left alone** --- `vb0` / `cb4` / the shape-key pair, skipped by default. A mod whose
  `vb0` is stale does not draw **at all**, which is a different symptom from broken textures, and
  rewriting those on a mod that *does* draw breaks what works.
* **unrecognised** --- not this character's, or older than the recorded history. Left alone, never
  guessed. **This is the honest limit of the tool's coverage**: if a texture you care about is here,
  its older hash is not in the library yet.

<br>

## What it will not do

* **It will not invent a hash.** A role whose older generations were never recorded cannot be
  resolved, and the run says so rather than leaving you to infer it from a texture that did not
  change. Extending coverage means adding rows to `HashData.cpp` --- `Tools/Misc/Diagnostics/
  chisaHashHistory.py` is how Chisa's older generations were derived, from the mods themselves and
  on hash-level evidence only.
* **It will not touch a previous fix's sections.** Anything whose section name carries `Remap` holds
  the *target's* hashes and is correct.
* **It will not repair a missing file.** A mod naming a `.dds` that is not on disk is a different
  fault, and shows up as a dangling `filename =` rather than a stale `hash =`.
* **It changes nothing about geometry, weights or shaders.** If the mod's shape is wrong, or a part
  is missing, the hashes are not your problem.

<br>

## Why a remap can look *better* than the mod it came from

Worth knowing, because it looks like a contradiction: AGRemap's own WuWa fixes bind textures **by
register** on the target's draws rather than by hash, so a remap shows what the author painted while
the original --- with its stale hashes --- shows the character's own textures. If a base screenshot
disagrees with a remap that way, suspect this before suspecting the remap. See Creating Remaps'
*"A MOD WITH EVERY HASH STALE"*.
