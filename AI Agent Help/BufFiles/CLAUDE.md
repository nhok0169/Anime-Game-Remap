# Buf Files

The binary vertex/index data pipeline: `BinaryFile` -> `BufFile` -> `BlendFile`/`PositionFile`/
`IbFile`/`VbFile`, the `BufDataType` family (`BufFloat`/`BufFloat16`/`BufSignedInt`/
`BufUnSignedInt`/`BufUnorm`) that says how to decode one value, `BufElementType` that groups them
into a named field, plus `BufTools` and the `bufEditors/` strategy family. Read this before
touching any of them, and **before writing anything that produces or consumes a 3dmigoto dump**
(`getDumpStr`/`readDumpStr`) -- the text format is not ours to choose, and the obvious references
for it are misleading in a specific way documented below.

See [Architecture](../Architecture/CLAUDE.md) for the generic pybind11 mechanics, and
[Texture Editing](../TextureEditing/CLAUDE.md) for the sibling `.dds` pipeline.

## Where this subsystem currently sits

Nearly all of it is C++ now. As of 2026-09-04, under `api/src/py/FixRaidenBoss2/model/files/`
only **`BufFile.py`** is still pure Python, and only because it adds `toDataFrame`/`fromDataFrame`,
which need pandas and therefore cannot live in the Python-free `AGRemapCore`. Everything else --
`BinaryFile`, `BlendFile`, `PositionFile`, `IbFile`, `VbFile`, the whole `BufDataType` family,
`BaseBufEditor`/`BufEditor` -- is `AGRemapCore` plus a pybind11 binding under its bare name.

One consequence that surprises people: `BlendFile`/`PositionFile`/`IbFile`/`VbFile` derive from
**`CppBufFile`**, not from the pure-Python `BufFile`, so they do **not** have `toDataFrame`/
`fromDataFrame` as methods. That is deliberate and consistent across the family. The capability is
not lost -- `BufTools.toDataFrame(anyBufFile)` works on any of them, because it only needs
`CppBufFile`'s own surface.

## `data` is read-only -- the write path is `src` + `read()`

`BinaryFile.data` is bound `def_property_readonly`, so nothing can assign it from Python, and
`AGRemapCore` deliberately keeps it that way. Every operation that *replaces* a file's bytes
(`encodeAll`, `merge`, `readDumpStr`, and `BufTools.fromDataFrame` on top of them) does:

```cpp
setSrc(newBytes);
read();
```

This is not a workaround, it is the useful path: `BufFile::read()` re-validates, so a byte count
that is not a whole number of lines raises `BadBufData` instead of silently producing a corrupt
file. Two things follow, and both belong in any new method's doc comment:

- **A file originally constructed from a *path* ends up holding raw bytes as its `src`.** The file
  on disk is untouched; use `fix(fixedFile = ...)` to write it out. (`TexEditor.fix` sets
  `texFile.src` the same way -- this is the established shape, not a novelty.)
- Set `elements` **before** the bytes when both change. `read()` validates against
  `bytesPerLine`, which only `setElements` recomputes.

## The dump text format belongs to 3dmigoto -- and the obvious references lie

`getDumpStr`/`readDumpStr` produce and consume the `.txt` files a 3dmigoto *frame analysis*
writes, which Blender then imports. **The format is 3dmigoto's, so 3dmigoto's own output is the
only spec.** This cost a full wasted implementation cycle once, so read the next two paragraphs
before writing a formatter.

`Tools/ModToDumpConverter/` and `Tools/DumpToModConverter/` are Jupyter notebooks that produce and
read these dumps, and they are the natural thing to port from -- but they are a *reverse
engineering* of the format, not the format. Their number formatting is Python's `str()`, which is
**not** what 3dmigoto writes. Port their structure by all means; do not treat their output as
correct.

Those notebooks are also this subsystem's **downstream consumers**, and the maintainer runs them
for real. They are now thin -- `IbFile`/`VbFile`/`merge`/`getDumpStr`/`readDumpStr` replaced the
classes they used to hand-roll -- so if you add something here that a notebook is still doing by
hand, collapsing it is part of the job. See [Overview](../Overview/CLAUDE.md)'s operating norms for
that obligation and for the rule about never writing into the user's real mods.

**Check the provenance of any sample dump before validating against it.** Two folders on this
machine look equally authoritative and are not:

| Folder | What it actually is |
| --- | --- |
| `GI-Model-Importer-Assets/PlayerCharacterData/<Character>/` | **genuine 3dmigoto output** -- the real reference |
| any folder the notebooks were run over (eg. a `Hutao6`) | **the notebook's own output** -- validating against it is circular |

Validating against the second proves only that you reproduced the notebook, which is exactly the
mistake that produced a carefully-built, carefully-tested, wrong implementation.

### What genuine 3dmigoto output actually looks like

Verified by round-tripping real dumps (`real dump -> readDumpStr -> binary -> getDumpStr ->
compare`), which is also the best regression test available for this area:

- **Numbers are C's `"%.9g"`** -- 9 significant digits (`FLT_DECIMAL_DIG`, exactly enough to
  round-trip a float32), trailing zeros dropped, no forced `.0`. So `1`, `-1`, `0.0405528881`,
  `-8.99908148e-09`. Not `1.0`, and not shortest-round-trip-of-a-double.
- **A decoded value is rounded through a 32 bit float before printing**, because 3dmigoto holds it
  as one. A no-op for anything decoded straight out of a float32, but it matters for `BufUnorm`,
  whose decode divides by 255 in double: 128/255 prints `0.501960814` (float) and not
  `0.501960784` (double).
- **The blank line goes *between* lines, not after the last one.** A real dump ends right after
  its final entry.
- The files are **CRLF**. Do *not* put `\r\n` in the returned string -- writing it in Python text
  mode on Windows adds the `\r` back, and emitting it yourself would double up.
- An entry is named by its **element key**, not the raw element name: a second element sharing a
  name is suffixed with its occurrence (`TEXCOORD`, then `TEXCOORD1`), which matches
  `decodeLine`'s own keys. The header's `SemanticIndex` carries the same counter.
- A `.ib` dump's header always claims `format: DXGI_FORMAT_R16_UINT` while the `.ib` binary itself
  stores **32 bit** indices. Write the header 3dmigoto writes, not the one the data implies.

In C++, `std::to_string` (always 6 decimals) and a default `ostringstream` (6 significant digits)
are both wrong for this; `std::to_chars(..., std::chars_format::general, 9)` is `"%.9g"`.

**One known, accepted difference remains**: on an exact tie at the 9th digit, `std::to_chars`
rounds to even and MSVC's `printf` (what 3dmigoto uses) rounds away from zero, so
`0.9853515625` prints `0.985351562` here and `0.985351563` there. Measured at **4 values in
692,496** against real dumps, and both spellings parse back to the same float32. Closing it means
calling `snprintf` per value -- locale-sensitive and much slower on a hot path -- so it was left
alone deliberately. Don't "fix" it without asking.

## Bulk (`decodeAll`/`encodeAll`) vs per-line (`decodeLine`/`encodeLine`)

`decodeLine` builds a fresh keyed map per line. Calling it in a Python loop over a real mod costs
one crossing into C++ **per line**, plus a dict, a list per element and a boxed Python number per
value -- for a 50,000 vertex `Blend.buf` that measured **~200 ms**, of which ~82% was the boundary
crossings and almost none was the Python loop itself.

`decodeAll`/`encodeAll` are the columnar counterparts: they walk every line in C++ and hand back
**one contiguous NumPy array per column**, keyed `(elementKey, indexWithinElement)`. One crossing
for the whole file. That is what `BufTools.toDataFrame`/`fromDataFrame` and `getDumpStr` are built
on, and it is ~20x faster. **Reach for them for anything whole-file; keep `decodeLine` for
genuinely single-line work** (a `BufFile::Filter`, an assertion in a test).

Two traps this area has already sprung:

- **Never flatten a whole frame with `df.to_numpy()`.** A `Blend.buf`'s frame mixes float
  `BLENDWEIGHT` columns with integer `BLENDINDICES` columns, and a whole-frame conversion upcasts
  everything to one dtype -- silently handing every integer element a float to encode. Pull each
  column out on its own (`df.iloc[:, i].to_numpy()`), which keeps its dtype. There is a dedicated
  test pinning this; don't "simplify" it away.
- **`decodeLine` returns a `std::unordered_map`, so its iteration order is arbitrary.** Anything
  that needs the elements in order -- a dump's entry order *and* its byte offsets, a DataFrame's
  columns -- must take that order from `elements`/`decodeAll` (both declaration-ordered), never
  from iterating a decoded line. The pure-Python originals got away with it because a Python dict
  is insertion-ordered; a direct port of that loop is silently wrong.

## What the C++ migration actually bought here, measured

Worth knowing before you weigh a change in this area, because it says where the cost lives. Old =
the published `AnimeGameRemap 4.6.4` (pure Python), new = the current C++-backed API, same mods,
same work, outputs verified equivalent:

| Direction | Old | New | |
| --- | --- | --- | --- |
| Mod -> Dump, 5 mods (1.6--19 MB of binaries, 422 MB of dump text) | 299.5 s | 4.9 s | **61x** |
| ...worst single mod (NilouBreeze, 19 MB) | 240.1 s | 1.9 s | **129x** |
| Dump -> Mod, 5 characters (32 MB of dumps) | 4.2 s | 1.8 s | 2.3x |

Two things to take from that. **The write direction is where the pain was** -- formatting bytes
into text was minutes per mod, and it scaled with vertex count, which is exactly why the speedup
grows with mod size (5x on the smallest mod, 129x on the largest). **The read direction was never
the problem** -- parsing is much cheaper than formatting, so 2.3x is the whole prize there and
there is little left to win. A "make it faster" request aimed at reading dumps is probably not
worth taking; one aimed at anything that formats or decodes per line still might be.

If you change the dump text or the columnar path, the honest regression test is the one that
produced these numbers: run both directions over several real mods and diff the outputs. Binaries
should come out **byte identical**; dump text should differ only in number spelling if you touched
the formatter, and you should say how many values actually changed value rather than eyeballing a
diff (a real run compared 17.4 M values and found 0 value-changing differences).

## Assembling a `.vb` from its parts

A GI character's vertex buffer is split across a `Position.buf`, a `Blend.buf` and a
`Texcoord.buf`, one line each per vertex. `CppBufFile.merge([...])` stitches such a set together
-- line *i* of the result is line *i* of every source concatenated, and `elements` becomes every
source's elements in order:

```python
vbFile = VbFile(b"", [])
vbFile.merge([positionFile, blendFile, texcoordFile])
```

Sources are left usable (their elements are deep-copied in, per `BufElementType`'s
shareable-value contract), and a ragged set truncates to the shortest source rather than reading
past the end of one.

Going the other way, `VbFile.readDumpStr` rebuilds the elements from the dump's **own header**
(`parseDumpHeader` -> `parseFormatName`, mapping `R8G8B8A8_UNORM` to 4 one-byte UNORMs and so on),
so a dump can be read back without being told its layout. `parseFormatName` deliberately returns
nothing at all for a format it only *partly* understands -- encoding against half an element list
would silently corrupt every line.
