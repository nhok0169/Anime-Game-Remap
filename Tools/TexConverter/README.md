# AG Remap's Texture Converter

Converts `.dds` textures into an ordinary image format (`.png` by default), so they can actually be
viewed --- most image viewers, browsers and AI coding agents can't open a BCn-compressed `.dds` at
all.

Runs on the API's own `TextureFile.saveAs`, so it decodes exactly the same way the remapper itself
does (via [Compressonator](https://github.com/GPUOpen-Tools/compressonator)) --- **the API must be
built first** (see [`AI Agent Help/Setup/CLAUDE.md`](../../AI%20Agent%20Help/Setup/CLAUDE.md)).

<br>

## How To Run
On [CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py <src> <dest>
```

`src` is a `.dds` file or a folder of them; `dest` is the output file or the folder to put the
output in. `dest` is **required** --- this tool never writes next to the source, to avoid littering
mod/data folders with untracked images.

<br>

### Examples

One texture, to a named file:

```bash
python3 main.py "SomeMod/AmberBodyDiffuse.dds" "C:/scratch/amber.png"
```

One texture, into a folder (the name is kept, the extension is swapped):

```bash
python3 main.py "SomeMod/AmberBodyDiffuse.dds" "C:/scratch"
```

Every texture in a mod, mirroring the mod's own subfolder structure:

```bash
python3 main.py "SomeMod" "C:/scratch/textures"
```

<br>

### Options

| Option | Description |
| --- | --- |
| `-f`, `--format` | The image format to convert to (default: `png`). Also accepts `bmp`, `jpg`, and `dds` (re-encode) |
| `-a`, `--alpha` | How to treat the alpha channel: `drop` (default), `keep`, or `only` --- see below |
| `-s`, `--silent` | Don't print each conversion |

Exits `0` when at least one texture was converted, `1` otherwise.

<br>

### Why `--alpha` defaults to `drop`

These textures routinely use the alpha channel as a **mask** rather than as transparency. Amber's
face diffuse, for example, keeps the entire face in RGB while its alpha holds a *blush mask* --- two
soft ellipses over the cheeks --- so alpha is near zero nearly everywhere. An ordinary image viewer
honours that as opacity and shows you a blank image, which looks exactly like an empty or broken
texture.

Since the whole point of this tool is *seeing* the texture, it forces every pixel opaque by default:

| `--alpha` | What you get |
| --- | --- |
| `drop` (default) | RGB exactly as decoded, alpha forced to 255 --- always shows the artwork |
| `keep` | All 4 channels, byte-faithful to the source |
| `only` | The alpha channel on its own, as a greyscale image |

```bash
python3 main.py "SomeMod/FaceDiffuse.dds" "C:/scratch/face_mask.png" --alpha only
```
