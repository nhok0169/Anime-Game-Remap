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
