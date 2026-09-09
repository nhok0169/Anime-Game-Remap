# Texture Editing

The `.dds` texture read/edit/write pipeline: `TextureFile`, `TexEditor`, `TexCreator`, and the
`texFilters/`/`pixelTransforms/` strategy families under
`api/src/py/FixRaidenBoss2/model/files/` and `.../model/strategies/texEditors/`. Read this before
touching any of those, or before answering a "should engine/readPillowImg move/change shape"-style
design question about them — the shape of this subsystem encodes real, deliberate tradeoffs, not
just whatever was easiest to port. See [Architecture](../Architecture/CLAUDE.md) for the generic
pybind11 mechanics referenced below (the wrapper-class `__dict__` note, the dispatch-method and
native-fast-path-classification patterns) and [Building](../Building/CLAUDE.md) for the
`extern/Compressonator` submodule.

## `compress = False`: the Pillow-speed escape hatch

`TexEditor(filters, compress = False)` -- and `TextureFile.save(compress = False)` under it -- writes
the edited texture as a **plain 32-bit uncompressed `.dds`** instead of re-encoding it to BCn.
`GIMICharFixerConfig::TexEdit` carries the same flag, so a character's fix can opt in.

**BCn encoding is essentially the whole cost of a texture edit.** Measured on Jean's 4096x2048
`BC7_UNORM` body lightmap:

| | decode | write | result |
| --- | --- | --- | --- |
| `compress = True` (default) | ~1.5s | **~15-22s** | BC7_UNORM, 8.00 MB |
| `compress = False` | ~1.2s | **0.09s** | uncompressed 32bpp, 32.00 MB |

Over 250x on the write step, for a file four times larger.

**This is exactly what the pure-Python implementation always did.** Its Pillow engine's
`img.save(src, 'DDS')` has no BCn encoder at all -- it dumped raw 32bpp. Comparing the old script's
texture-editing speed against this one is therefore not a like-for-like comparison, and the old
output was never in the source's format:

```
source        4096x2048  DX10/BC7_UNORM      10.67 MB   (13 mips)
old (Pillow)  4096x2048  uncompressed 32bpp  32.00 MB   (0 mips)
new, default  4096x2048  DX10/BC7_UNORM       8.00 MB   (1 mip)
```

Default stays `True`: matching the source asset's format is the right default, and the game loads a
BC7 texture more cheaply than a 32MB uncompressed one. Reach for `False` when iterating on a fix and
the encode wait is what is slowing you down.

**A cheaper win first, though:** the fix re-encodes byte-identical source textures repeatedly. Three
of the four `JeanBodyLightMap.dds` in one Jean mod are the same file, so the same 4096x2048 image is
BC7-encoded three times -- ~45s of pure duplicate work. A content-hash cache on the encode would
remove that without giving up the format.

## Looking at a `.dds` (you can't `Read` one directly)

The Read tool renders `.png`/`.jpg` but **not** `.dds` — a BCn-compressed texture is opaque to it,
and to most ordinary image viewers. Convert it first:

```bash
py -3 "E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Tools\TexConverter\main.py" "<some.dds>" "<scratchdir>"
```

then `Read` the `.png` it prints. `Tools/TexConverter/` takes either a single `.dds` or a whole
folder (converting every `.dds` under it, mirroring the subfolder structure), and **requires** an
explicit output path — deliberately, so nobody litters `Data/Mod Downloads/` or a user's mod folder
with untracked `.png`s. Point it at your scratchpad directory. See its
[README](../../Tools/TexConverter/README.md) for the flags.

**Alpha in these textures is often a mask, not transparency — so the converter forces it opaque by
default.** Confirmed the hard way on `Data/Mod Downloads/GI/Amber/4_0/AmberFaceDiffuse.dds`: its
RGB holds the whole face (eyes, lashes, blush, skin shading), but its alpha is ~0 almost everywhere
(mean 3.1, *zero* fully-opaque pixels) because that channel carries a **blush mask** — two soft
ellipses over the cheeks — rather than opacity. Any ordinary viewer honours it as opacity and
renders the texture essentially blank, which reads as "this texture is empty" when it very much
isn't. So `--alpha` defaults to `drop`:

| `--alpha` | What you get |
| --- | --- |
| `drop` (default) | RGB as decoded, every pixel forced opaque — always shows the artwork |
| `keep` | all 4 channels byte-faithful (verified identical to the source decode) |
| `only` | the alpha channel by itself, as greyscale — how the blush mask above was found |

`TextureFile.saveAs` itself is not involved in any of this: it always writes all four channels
faithfully, and the alpha handling lives entirely in the tool, since it's a *viewing* concern.
If you call `saveAs` directly and get a blank-looking image, that's this same trap — force alpha
opaque yourself (`px = bytearray(tf.getPixels()); px[3::4] = b"\xff" * (len(px)//4)`) before saving.

Two more things that will bite you here:
- **Run it from the PowerShell tool, not the Bash tool.** It imports the built `core.pyd`, and
  Git Bash fails that import with `DLL load failed ... The parameter is incorrect` — a documented
  environment quirk, not a broken build (see [Building](../Building/CLAUDE.md)).
- **The API has to be built already.** The tool imports `FixRaidenBoss2`; it reports this clearly
  rather than failing cryptically, but if you see that error go read [Setup](../Setup/CLAUDE.md).

Underneath, this is `TextureFile.saveAs(dest)` — the format follows `dest`'s extension, so the
same call writes `.png`/`.bmp`/`.jpg` (uncompressed, straight out of the RGBA8 buffer, via
`Compressonator`'s own built-in stb writers) or `.dds` (re-encoded, same as `save()`). It opens the
file on demand, so the whole thing is a one-liner if you'd rather skip the tool:

```python
FRB.TextureFile("some.dds").saveAs("out.png")
```

`saveAs` deliberately does **not** apply `info["gamma"]`, unlike `save()`: that gamma is a
pre-correction for the `.dds`/BCn sRGB round trip, and `save()` applies it *destructively, in
place* to the pixel buffer. Neither is wanted when the point is to see the texture's real decoded
pixels. It also leaves `src` pointing at the original file — it's an export, not a "save as and
move on".

## The vendored Compressonator reads and writes ordinary image formats too

Worth knowing before you reach for Pillow (or a new dependency) to handle a non-`.dds` format:
**`CMP_LoadTexture`/`CMP_SaveTexture` already handle `.png`/`.bmp`/`.jpg` themselves.** Only `DDS`
is registered as a real image plugin (`CMP_RegisterHostPlugins` in
`extern/Compressonator/cmp_framework/compute_base.cpp` registers exactly one `"IMAGE"` entry), so
every other extension falls through to Compressonator's own bundled stb writers/readers —
`stbi_write_png`/`_bmp`/`_jpg` in `CMP_SaveTexture` (~line 1085) and `stb_load` in
`CMP_LoadTexture`, the latter producing a `CMP_FORMAT_RGBA_8888` mipset directly. Both work off the
uncompressed RGBA8 buffer, so no conversion step is involved either way.

That's why `saveAs` needed no new dependency and no Pillow: a `.png` export is just the ordinary
save path with the BC7 compression step skipped. It also means a `TextureFile` can `open()` a
`.png`, which is occasionally handy for round-trip checks in a test.

## Mipmapped `.dds` files (most real ones) used to fail to open at all

Fixed 2026-09-06, and worth knowing because the symptom was *silence*, not an error.

`CMP_ConvertMipTexture` returns `CMP_ERR_INVALID_SOURCE_TEXTURE` (2) when handed a mip **chain**,
and `CMP_OK` on the identical texture with a single level. `TextureFile::open` fed it whatever
`CMP_LoadTexture` produced, so for any texture shipping mipmaps -- which is most textures a real
mod contains, since the game's own are built that way -- `open()` set `hasImage_ = false` and
cleared the pixel buffer. Every caller downstream reads that as "nothing to do":

- `TexEditor::fix` returns early on `!hasImage()`, so a texture edit logged
  `Editting texture for <name>.dds` and then wrote **no file at all**
- `Tools/TexConverter` reported `unreadable texture`
- nothing anywhere raised

The fix is one line in `open()`, before the convert:

```cpp
mipSetIn.m_nMipLevels = 1;   // this class only ever reads level 0
```

It is safe because `CMP_CMIPS::FreeMipSet` frees by `m_nMaxMipLevels`, which is untouched, so all
the levels are still released. `save()` writes a single level back, so nothing round-trips a chain.

**Two lessons if you hit something like this again.** First, the discriminator was not obvious from
the file -- the working and failing textures had the *same* DXGI format (99, `BC7_UNORM_SRGB`) and
differed only in `dwMipMapCount`. Rewriting a failing file's header to say `mipMapCount = 1` made it
load, which is a cheap way to test that kind of hypothesis without building anything. Second, the
thing that actually settled it was a ~50-line standalone `.cpp` calling `CMP_LoadTexture` and
`CMP_ConvertMipTexture` directly and printing both status codes; reading Compressonator's source was
leading nowhere. Measure the call, don't infer it.

<br>

## Uncompressed `.dds` files used to fail to open too -- and came out too bright once they did

The same failure shape as the mipmap bug above, found the same way, a batch later (2026-09-09):
Keqing's head and dress diffuses are **`DXGI_FORMAT 29` (`R8G8B8A8_UNORM_SRGB`) in a DX10
header** -- uncompressed, which is what a mod author's export looks like, as opposed to the BCn
a game texture ships as. `open()` reported no image for every one of them, her fix logged
`Editting texture for ...` four times, the summary said **"editted 8 *.dds files and skipped
0"**, and not one file was written. The `.ini` then pointed `ps-t0` at a texture that did not
exist.

The standalone probe (again: measure the call, do not infer it) split the two halves apart:

```
  CMP_LoadTexture = 0     format=0  1024x1024  mips=11/11
  compressed=0  dwDataSize=4194304  raw level0 readable      <- the pixels ARE there
  CMP_ConvertMipTexture = 4                                  <- CMP_ERR_UNSUPPORTED_SOURCE_FORMAT
```

Compressonator's DDS plugin maps a DX10 header's DXGI format through a table that covers the BCn
formats and stops, so **every uncompressed DXGI format loads with its pixels intact and comes
back `CMP_FORMAT_Unknown`** -- which the convert then refuses. `open()` now reads the format out
of the file itself (DDS magic, the `DX10` fourCC, the DXGI value at offset 128) and names the
four 32-bpp unorm formats a mod actually ships: 28/29 -> `RGBA_8888`, 87/91 -> `BGRA_8888`.
Anything else is left `Unknown`, so it fails exactly as it did rather than being silently
misread. **It has to happen before `format_` is taken** -- `save()` uses `format_` as the format
to write back out, and `Unknown` is not one.

**Then the second half, which is the one worth remembering.** With the format named, the
textures decoded -- and came out `229` where the old script writes `201`, on every RGB byte,
alpha matching exactly. Across every distinct source value in the texture,
`201 == round(255 * (229/255) ** 2.2)`, with zero disagreements: the old script is applying
`GammaFilter(1 / 2.2)`, the same pre-correction `DarkDiffuse` declares by hand for Ganyu.

It matters in game rather than on paper. **`save()` writes the edited texture back untagged** --
plain 32-bit unorm, no DX10 header -- so the shader samples the new file *without* the
sRGB-to-linear transform the source was written to be read through, and the remapped character
renders visibly brighter than the mod does on its own model. `open()` now sets the texture's
gamma metadata when the header says sRGB, as **metadata rather than a pixel pass**, for the
reason `DarkDiffuse` gives: it belongs immediately before the encode, not before the fix's own
filters, so a filter matching on colour still sees the values the texture actually holds.

Three cases, and the third is why this cannot live in each fix:

| source format | what happens |
| --- | --- |
| `R8G8B8A8_UNORM_SRGB` (Keqing's diffuses) | the new branch: format named **and** gamma set |
| `BC7_UNORM_SRGB` (Ganyu's diffuse) | never reaches the branch -- Compressonator maps BCn itself and hands the values back **raw**; `DarkDiffuse` declares the same gamma by hand |
| `BC7_UNORM` (KeqingOpulent's lightmap) | not sRGB, no correction |

All three A/B byte-identical against the old script. If you touch this, re-run Ganyu as well as
whatever you are working on -- she is the one that proves the BCn path was left alone.

<br>
## Two engines, on purpose

`TextureFile` can read/write a `.dds` through either of two backends, selected per-instance via
the `engine: TexEngine` constructor flag (`constants/TexEngine.py`):

- **`TexEngine.Compressonator`** (the default) — AMD's Compressonator library
  (`api/extern/Compressonator`, linked into `AGRemapCore`), cross-platform, and the one that
  correctly round-trips BC7/BCn compressed formats (Pillow's DDS support has real correctness gaps
  here — see the gamma note below). This is where all *new* development should default to.
- **`TexEngine.Pillow`** — the original pure-Python path, kept for compatibility, not being
  extended further. **Never assume Compressonator is the only live path** — a feature request that
  only updates the Compressonator branch of `TextureFile.open`/`save` and leaves the Pillow branch
  behind is an incomplete port, not a acceptable simplification, unless the user explicitly says
  otherwise.

Don't conflate this with `readPillowImg` below — `engine` picks *which library actually
reads/writes the file on disk*; `readPillowImg` (Compressonator-engine only) picks whether a
`PIL.Image` mirror of the result also gets built in Python.

## Ported vs. still-pure-Python

Everything under `texFilters/`/`pixelTransforms/` is now a thin pure-Python subclass of a
pybind11-bound `Cpp`-prefixed core class (the "Wrapper" outcome — see
[Architecture](../Architecture/CLAUDE.md)'s "Two different outcomes" section): `Colour`,
`ColourRange`, `TextureFile`, `BaseTexEditor`, `BaseTexFilter`, `GammaFilter`, `TexEditor`,
`TexCreator`, `BasePixelTransform`, `CorrectGamma`, `ColourReplace`, `HighlightShadow`,
`InvertAlpha`, `TempControl`, `TintTransform`, `Transparency`, `ColourReplaceFilter`,
`TransparencyAdjustFilter`, `InvertAlphaFilter`, `HueAdjust`, `PixelFilter`.

**`TexMetadataFilter` is deliberately still pure Python, unported, on top of `BaseTexFilter`** —
not a gap. It only ever touches `texFile.info` (a plain Python `dict`, mirroring `PIL.Image.info`,
that lives directly on `TextureFile` regardless of engine — see `TextureFile.py`), never pixel
data or `.img` at all, so there's no Compressonator-side buffer work for a C++ port to actually
speed up. Don't "finish the port" by moving this one over without a concrete reason.

Two pre-existing quirks were carried over faithfully from the original Pillow implementations
during the port, not silently fixed: `InvertAlpha`'s `0 - alpha` (vs. `InvertAlphaFilter`'s
correct `255 - alpha`), and `HueAdjust`'s byte/degree-scale mismatch in its hue math. If you're
asked to "fix" either, that's a deliberate behavior change, not a bug fix on the port — flag it as
such.

## `readPillowImg`: buffer-native by default, `.img` is opt-in

`TextureFile` holds pixel data two ways: the native Compressonator RGBA8 buffer (always present
when `hasImage()`), and an optional real `PIL.Image` mirror at `.img`. Building/mirroring that
`.img` costs a real per-pixel round trip through Pillow — the entire reason `PixelFilter`'s own
docstring used to warn "this filter is slow" before the C++ port. Since the ported filters
(`ColourReplaceFilter`, `TransparencyAdjustFilter`, `InvertAlphaFilter`, `HueAdjust`,
`PixelFilter`) now operate directly on the native buffer, `.img` doesn't need to exist at all for
them to work — so `TextureFile`/`BaseTexEditor` (and its `TexEditor`/`TexCreator` subclasses) all
carry a `readPillowImg: bool` flag, **default `False`**, ignored when `engine` is `Pillow` (which
always needs `.img` — there's no separate buffer for it to skip). With it off:

- `TextureFile.open()`/`.save()` never touch `.img` at all — it's left exactly as it was.
- The shared filter-binding sync helpers (`PyTexFilterCommon.cpp`'s `syncTextureFileFromImg`/
  `syncTextureFileToImg`) skip the Pillow pull/push entirely when `.img` is `None` — this is the
  *one* choke point that makes every ported filter buffer-native for free; don't duplicate this
  logic per-filter.
- `TextureFile.hasImage`/`.width`/`.height` **fall back to the native Compressonator buffer state**
  when `.img` is unset, rather than reporting `False`/`0`/`0` — see `TextureFile.py`'s property
  overrides. This is why the post-fix success check was `not tex.hasImage`, not
  `tex.img is None` (it lived in `Mod.py`, **deleted 2026-09-05** — the equivalent check now belongs
  with whatever calls `RemapTexResource::fix`, reached from `RemapService::fixResources`) — the latter would incorrectly read as failure on the (now-default) fast path.
  **If you add a new "did this texture edit actually produce something" check anywhere, use
  `hasImage`/`width`/`height`, never a raw `.img is None` test** — the whole point of this flag is
  that `.img` being `None` no longer means failure.
- `TextureFile.read()` still builds `.img` on demand regardless of the flag — it's the one
  intentional escape hatch for a caller (or a plain-Python custom callable in a filter list) that
  actually needs pixel-level Pillow access.

**Set `readPillowImg=True` on the *editor* (`TexEditor`/`TexCreator`), not on the generic caller,
whenever the editor's own filter list contains anything that touches `.img` directly** — a plain
Python callable passed into `filters=[...]` (not a ported `Cpp`-backed filter), or a not-yet-ported
filter. The editor is the only thing that actually knows what its filter chain needs; the generic
caller constructing the `TextureFile` (`Mod.py` used to be the example; it was deleted 2026-09-05,
and `AGRemapCore::RemapService` is the equivalent caller now) has no way to know this and shouldn't
be made to guess per-instance. This is why `TexEditor.fix()`/`TexCreator.fix()` **unconditionally
overwrite** `texFile.engine`/`texFile.readPillowImg` with their own values for the duration of the
call, even though `TextureFile` also carries its own copies of both flags — the override exists so
one `TextureFile` can safely be reused across editors with different needs (see
`IniParseBuilderData.py`'s Jean/JeanCN `TexEditor(filters=[cls._jeanEditBodyLightMap5_5],
readPillowImg=True)` for the one real production call site that needs this — every other
`TexEditor` there uses only native filters and leaves it `False`). **Don't "simplify" this by
removing `engine`/`readPillowImg` from `BaseTexEditor` and having editors just inherit whatever the
passed-in `texFile` already has** — that pushes the "does my filter chain need `.img`" decision
onto callers that structurally can't make it correctly.

## Save format

`TextureFile.save()` re-encodes to whatever `CMP_FORMAT` the file was originally opened with
(remembered in `format_`); a `TextureFile` that was never successfully opened (e.g. `TexCreator`
writing a brand-new file) has no original format to preserve and defaults to `CMP_FORMAT_BC7`
(`TextureFile::DefaultFormat`) — the common format for GIMI diffuse/lightmap textures. If a future
request needs a different default or a way to override it explicitly, that's new surface area, not
something already exposed to Python today.

Gamma correction (`TextureFile.info["gamma"]`) is applied via `GammaFilter`, which runs directly
on the native buffer — it was never placed in anyone's real `filters` list (grep confirms
`TextureFile.save()` is its only caller), so it doesn't participate in the `.img` sync dance above
at all.

## Testing

Test files: `test_TextureFile.py`, `test_TexEditor.py`, `test_TexCreator.py`,
`test_BaseTexEditor.py`, plus one `test_CppXxx.py` per ported `Cpp`-prefixed class, under
`Testing/Unit Tester/UnitTester/Tests/` (registered per [Testing](../Testing/CLAUDE.md)'s
two-place `Tests/__init__.py` rule). `readPillowImg`/`engine` coverage already exists for all four
non-`Cpp` classes above — extend those sections rather than starting a new file for a related flag.
A real sample `.dds` for hands-on/manual verification lives at
`Data/Mod Downloads/GI/Amber/4_0/AmberBodyDiffuse.dds`.
