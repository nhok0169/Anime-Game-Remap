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
remove that without giving up the format. **That cache exists now** -- see the next section.

## At the CLI's DEFAULT settings, NONE of the above is what costs (2026-09-20)

Everything above is measured with `compress = True`. **The CLI leaves textures uncompressed
unless `--compressTextures` is passed**, so an ordinary run encodes nothing, and the sentence
"BCn encoding is essentially the whole cost of a texture edit" does not describe it at all. Taken
at face value it sends you to optimise a code path a default run never enters.

Measured on a 4096x4096 `BC7` source, writing uncompressed, per texture:

| phase | cost |
| --- | --- |
| decode (`CMP_ConvertMipTexture`) | ~2.5s |
| **`GammaFilter`** | **~4.2s** |
| the actual write | ~0.5s (of which disk I/O is ~0.05s) |
| Pillow's entire round trip, for scale | ~0.66s |

**The gamma pass was the single most expensive thing in a default texture edit, and it looked
like part of the write.** `save()` runs `GammaFilter` for every sRGB source (`open` sets the gamma
from the DX10 header's sRGB bit), and it called `std::pow` three times per pixel -- 67 million
calls for one 4096x4096 texture. `CorrectGamma::correctGamma` is a **static pure function of an
8-bit channel value**, so it has exactly 256 possible answers; it is a 256-entry lookup table now,
filled by calling that same function so the output is identical by construction rather than by
approximation. `save` on that texture went **4.285s -> 0.384s**.

Two more changes landed with it, both also byte-identical:

- **`TexCache`** (`model/files/TexCache.h`), owned by :cpp:class:`RemapService` and handed to each
  texture resource exactly where a download is handed its `DownloadCache`. Two halves, both keyed
  on **content**: the decode (a pure function of the source file's bytes) and the write (keyed on
  the finished pixel buffer, so every filter still runs and only the re-encode of an image already
  written is replaced -- with a file copy). Keyed on content because
  `TexEditor::Filter` is a `std::function` and cannot be introspected for purity, and at least one
  real filter (`MaterialBandRemapFilter`) reads a *second* file.
- **`TextureFile::writeUncompressedDds`**, which skips Compressonator for the uncompressed case.
  The uncompressed write turned out to be a fixed 128-byte legacy header plus the pixels swizzled
  `RGBA` -> `BGRA` and nothing else, so it is reproduced exactly. 64MB now writes in **74ms**.

`TexCache` is bound as ``TexCache`` and reachable from Python
(``texFile.setCache(cache)``), for a prototype that drives :class:`TextureFile` directly instead of
through the service. ``setCache`` carries ``py::keep_alive<1, 2>`` because the texture holds a
BORROWED pointer -- without it a cache Python has dropped is collected while the texture still
reads through it. ``AGREMAP_TEXCACHE=0`` turns both halves off, for measuring the cache against
itself in one binary.

**Regenerating the two committed doc artifacts for this needed `doxygenSplice.py`, not `main.py -d`.**
A whole-directory Doxygen run reported 1477 changed `core/xml` files, of which only ~15 were this
work -- the rest were include-graph node REORDERING, a Doxygen artifact, plus a large number that
`git status` listed purely because the rewrite left the index stale (`git diff` showed no content
change at all, and `git update-index --refresh` settled them). Splicing the 14 compounds this work
touched gave a 15-file diff with CRLF preserved. `core.pyi` has no such problem and came out clean
at +124/-1. Note that **`core.pyi` does not parse as Python either way** -- a pre-existing
`ModMappedAssets::replace` signature emits a non-default argument after a defaulted one, at HEAD as
well as after a regeneration, so do not read that as something your change broke.

**Three traps this work walked into, all worth knowing before touching this pipeline again:**

1. **`saveAs` passes `compress = "does dest end in .dds"`.** So every *other* format -- the
   `.png`/`.jpg` "let me actually look at this texture" path -- arrives at `writeTo` with
   `compress = false`. The first version of the bypass took that as its cue and wrote a **DDS into
   a `.png` file**. The mod corpus cannot see this (it only ever drives `save()` to a `.dds`), and
   a byte-identity check over 7 mods passed with the bug present; `test_CppTextureFile`'s three
   `saveAs` cases caught it. **Run the unit suite as well as the A/B.**
2. **A phase split can lump two things together and hide the big one.** "Write phase = round trip
   minus decode" attributed 4.2s of gamma work to the write, which is why the bypass was built
   first and bought almost nothing. Split until each number names one operation.
3. **This machine cannot measure a whole-mod change.** The spread on one mod is 96-158s for
   identical work. Cross-build timings are worthless here; `AGREMAP_TEXCACHE=0` exists so the cache
   can be measured against itself in ONE binary, and the per-texture numbers above come from
   timing `open`/`save` directly rather than from a run.


## AND THEN THE DECODE WAS ALL OF IT: `TextureFile` DECODES `BC7` ITSELF NOW (2026-09-20)

The table above was measured before the gamma lookup landed. With the gamma pass down to ~0.3s, the
remaining shape of a 4096x4096 texture edit was stark:

| phase | cost |
| --- | --- |
| `CMP_LoadTexture` (read the file, parse the header) | 0.010s |
| **`CMP_ConvertMipTexture` (the BC7 decode)** | **2.45s** |
| copy the decoded level out | 0.020s |
| `GammaFilter` | 0.30s |
| `writeUncompressedDds` (swizzle + 64MB) | 0.05s |

**2.45s for 1.05 million 4x4 blocks is 2.3us a block**, which is orders of magnitude more than
unpacking one should cost. Compressonator's own **`cmp_core`** exposes `DecompressBlockBC7` for a
single block, and blocks are independent, so `TextureFile` decodes BC7 itself now -- one block at a
time, across `std::thread::hardware_concurrency()` threads by horizontal bands. On that texture the
decode is **2.45s -> 0.21s** and the whole round trip **2.84s -> 0.35s**, which is faster than
Pillow's 0.62s. Over 14 textures from 2048x2048 to 5120x3072, the round trip totals **22.07s ->
3.03s (7.3x)** with everything else held constant.

**`GammaFilter` was rewritten in the same pass, and it is the smaller half of the same lesson.** It
had the lookup table already and still cost 0.30s, because it walked the image through
`texFile.getPixel(x, y)` / `setPixel(x, y, ...)`: 33 million cross-translation-unit calls for one
texture, plus 50 million `Colour::boundColourChannel` calls, with **LTO off for `python_dev` by
design** so none of it inlines. One linear pass over the buffer took it to ~0.07s. The clamp went
with it and loses nothing -- values out of an RGBA8 buffer are already 0-255 -- and the table is
stored as `std::uint8_t` rather than `int` **because `setPixel` cast rather than clamped**, so an
entry outside 0-255 had always been truncated. Keeping that cast is what makes the rewrite
identical rather than nearly identical.

### Why only BC7, and how that was decided

`cmp_core` has `DecompressBlockBC1`/`BC2`/`BC3` too, and the fast path claims **none of them**.
Decoding all 171 distinct textures in the corpora both ways and comparing:

| format | byte-identical | differing |
| --- | --- | --- |
| BC7 (`CMP_FORMAT_BC7`, DXGI 98 and 99) | **149** | 0 |
| BC1 (`CMP_FORMAT_BC1`, DXT1) | 0 | **1** |

The one BC1 texture came back differing on **1456 of 67108864 bytes -- off by one**, on interpolated
colours: the framework and the block decoder round BC1's 2/3-1/3 blend differently. Nothing warns
about that, it is invisible in any screenshot, and it would have gone into every DXT1 texture the
fix writes. **It was found only by decoding every texture both ways**, which is the whole argument
for running the sweep before claiming a format rather than after. A format is qualified by running
that sweep, not by reading the API.

Everything that is not BC7 falls through to `CMP_ConvertMipTexture` exactly as before, and so does
a texture whose dimensions are not multiples of 4 (BCn stores whole blocks, so those have partial
blocks at the edges, and no test here can cover them) or whose data is shorter than its own
dimensions claim. **This can only ever be faster, never a new way to be wrong** -- the rule
`writeUncompressedDds` already follows.

### Two things to know before touching it

- **`DecompressBlockBC7` with `options = NULL` is not thread-safe on its first call.** It runs
  `init_BC7ramps()`, which guards on a plain non-atomic `static` and fills a global table: two
  threads arriving together both see it unset and both write -- a data race that happens to produce
  the right answer only because they write identical values. One block is decoded under
  `std::call_once` before any thread starts, so the initialisation happens once and every later call
  only reads. Passing a `CreateOptionsBC7` object would also avoid it, and is deliberately *not*
  done: that hands the decoder a `new`-ed struct initialised differently from the `{0}` one the NULL
  path builds, and the bytes were only ever verified for the NULL path.
- **`AGREMAP_BC7_DECODE=0`** forces the framework decode, so the two can be A/B'd inside one binary
  (the `AGREMAP_TEXCACHE=0` pattern; 332 files byte-identical across the 21-mod corpus). The value
  is **trimmed** before comparing, and that is not fussiness: `cmd`'s `set VAR=0 && prog` hands the
  child `"0 "`, trailing space included. Written without the trim, this option's own test set the
  variable that way, compared the fast path against **itself**, and passed against a build with a
  deliberately corrupted pixel in it. `core/tests/TextureFile_Bc7Decode_test.cpp` sets it as
  `set "VAR=0"` now and detects a single wrong byte in 67MB.


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

## A written texture has NO mip chain unless you ask for one (`mipmaps = True`, 2026-09-12)

`TextureFile::open` decodes mip 0 only (next section), and `save` wrote mip 0 only -- so every
texture the fix has ever written shipped as a single level, where every texture the game ships
carries its full chain (a 1024x1024 mod `.dds` has 11). A texture without a chain is sampled from
its top level at every distance, and on a fine-grained texture that reads in game as **scattered
off-colour pixels that move with the camera** -- "almost like data is lost from a lossy
compression", on the hair of a Yelan mod, which is how it was found. `TextureFile::save(compress,
mipmaps)` now takes a second flag: `true` has Compressonator box-filter the chain down to 1x1 from
the edited pixels before the BCn encode (or the uncompressed write), and the DDS plugin writes every
level with the right `dwMipMapCount`. `TexEditor` and `TexCreator` carry the same flag (C++ and
Python), and `TextureFile.save(img, compress, mipmaps)` threads it. **The default is still
`false`**, so no compiled character's output changed; the Yelan prototype turns it on for every
texture it writes. A chain costs a third more file size and a little encode time.
`test_TextureFile.test_save_mipmaps_writesTheFullChain` pins the header count both ways.

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

**THE CORRECTION IS KEYED ON THE sRGB BIT, WHATEVER THE COMPRESSION -- and the first version of
this section said otherwise, which shipped a bug (corrected 2026-09-10).** It claimed BCn
"never reaches the branch" because Compressonator maps BCn itself, so the correction was gated
on *uncompressed* DX10 textures only. Then Xiangling's and HuTao's head diffuses came out
visibly pale in game (`../CreatingRemaps/Images/Xiangling/6_1/XianglingCheerPaleHair.jpg`): they are
`BC7_UNORM_SRGB`, and they were not being corrected.

**Compressonator has no sRGB BCn format at all.** Only ETC2 has sRGB entries in its table, so
one `CMP_FORMAT_BC7` covers DXGI 98 *and* 99 and the sRGB bit is gone the moment the file
loads -- exactly the same loss as the uncompressed case, just invisible because the format is
still named correctly. `dx10Format()` (renamed from `dx10UncompressedFormat`) therefore reads
the DX10 header for **every** texture and returns `{format, srgb}`; the format is only *used*
when Compressonator came back `Unknown`, but `gamma_` is set whenever the sRGB bit is on:
`BC1/2/3/7_UNORM_SRGB` (72/75/78/99) and `R8G8B8A8`/`B8G8R8A8_UNORM_SRGB` (29/91) alike.

| source format | what happens |
| --- | --- |
| `R8G8B8A8_UNORM_SRGB` (Keqing's diffuses) | format named **and** gamma set |
| `BC7_UNORM_SRGB` (Xiangling's, HuTao's head diffuses) | format left to Compressonator, **gamma set** |
| `BC7_UNORM` (KeqingOpulent's lightmap) | not sRGB, no correction |

**WHY THE WRONG VERSION LOOKED VERIFIED, which is the part to carry away.** Ganyu's edited
textures A/B'd byte-identical under the gated version, and that was read as "the BCn path is
fine". It was not evidence of anything: Ganyu's `DarkDiffuse` **declares `setGamma(1/2.2)` by
hand**, so she gets the correction from her own fix and would look identical either way. **An
A/B that passes because of a per-character override says nothing about the general path.** Pick
a character that does *not* declare it -- Xiangling and HuTao are the ones that do not.

Five textures were re-A/B'd byte-identical after the correction, Ganyu's among them. If you
touch this, run a character from each row of that table.

**`--compressTextures` is not related to any of this, despite sounding like it.** Measured:
`save(compress=true)` writes DXGI 98 and `save(compress=false)` writes a legacy header, and
**both are linear**. The flag changes the file's size, not its colour space. (And it only
*permits* compression -- `RemapService::_applyCompressTextures` forces it **off** when the flag
is absent, so a `TexEdit` asking for it does not get it unless the run asks too.)

Flagged and not done: writing the sRGB DX10 header ourselves instead of baking a 2.2 power into
8-bit values, which currently crushes the low end (52 -> 8). It would want its own in-game check.

<br>
## Compressonator cannot be handed a non-ASCII path, so it is never given one

`CMP_LoadTexture` and `CMP_SaveTexture` take a narrow `const char*`, which Windows decodes in the
**active code page**. Every path in this codebase is UTF-8, so a mod folder named in Korean reaches
the library as bytes naming nothing. `FileService::strToPath` cannot help here the way it does
everywhere else -- there is no wide overload to hand a path to.

**So the path never reaches Compressonator when it is not pure ASCII.** `TextureFile` reads and
writes a scratch file with an ASCII name under `temp_directory_path()`, and `std::filesystem` --
which IS unicode-safe -- moves the bytes the rest of the way: `rename` where it can, copy-and-remove
across volumes. An ASCII path takes the direct call exactly as before, so the common case is
unchanged. Two details are load-bearing:

* **the scratch file keeps the real extension**, because Compressonator picks both its reader and
  its writer off it -- a `.dds` written through a scratch file named `.tmp` comes out as something
  else entirely;
* **it is bounded by the temp folder being ASCII too.** A Windows profile named in a non-Latin
  script leaves nowhere to stage, and the code falls back to the direct call, which still fails. It
  fails *honestly* now (see below) rather than silently, but it fails. If this is ever reported from
  a Korean or Japanese install, that is the reason.

## `save()` used to discard `writeTo()`'s bool, and that is how the above stayed invisible

**The run reported `editted 18 *.dds files and skipped 0` having written NONE of them.** Not a
rounding error or a partial failure -- zero files on disk, full marks in the summary. This is the
failure mode **Overview** opens with, in its purest form.

`TextureFile::save` called `writeTo(src_, compress);` and dropped the result. Both of its callers
(`TexCreator`, `TexEditor`) treat `save` as `void`, so a failed write had nowhere to go. It now
throws, which the layer above already knows how to handle: the `.ini` is recorded as skipped, with
a message naming the file.

**A texture that cannot be written is a fix that did not happen**, and anything in this pipeline
that returns a `bool` nobody reads is the same bug waiting. `grep` for bare-statement calls to
functions returning `bool` if you touch this file.

### How this was diagnosed, which took one probe rather than a code read

```python
tf = FRB.TextureFile(ascii_src); tf.open()
tf.saveAs(ascii_dest)   # -> True,  file written
tf.saveAs(korean_dest)  # -> False, nothing written
```

Six lines, no rebuild, and it separates "the path is wrong" from "the encode failed" from "the
caller never asked" in one shot. Reach for that before reading Compressonator's source -- see
**Overview**'s habit about measuring a third-party failure rather than inferring it.

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
