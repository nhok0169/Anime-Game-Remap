# Mod Downloads: Wuthering Waves

The download assets of the Wuthering Waves characters, one folder per character and one subfolder
per game version, the same arrangement as [`../GI`](../GI) --- but a WWMI character is one mesh with
per-component draw ranges rather than one buffer set per object, so the files differ in shape:

| file | what it is |
| --- | --- |
| `<Name>Index.buf` | the whole mesh's index buffer, `R32_UINT`, stride 12; each component is a range of it (`<Name>Metadata.json`'s `index_offset` / `index_count`, the library's `Indices` / `IndexCounts`) |
| `<Name>Position.buf` | positions, `R32G32B32_FLOAT`, stride 12 |
| `<Name>Blend.buf` | the merged-skeleton blend: four `R8` bone indices then four `R8` weights, 8 bytes a vertex, in the space every component's `vg_map` maps into (see the VGRemaps guide's WuWa section) |
| `<Name>Vector.buf` | normal + tangent, `R8G8B8A8_SNORM` x 2, stride 8 |
| `<Name>Color.buf` | vertex colour, `R8G8B8A8_UNORM`, stride 4 |
| `<Name>Texcoord.buf` | four `R16G16_FLOAT` UV sets, stride 16 |
| `<Name>ShapeKeyOffset.buf`, `...VertexId.buf`, `...VertexOffset.buf` | the sparse shape keys, rebuilt from the dump (`wwmiIdentityMod.py`) |
| `<Name>BlendRemapVertexVG.buf`, `...Forward.buf`, `...Reverse.buf` | only for a character whose merged skeleton passes 256 bones (Chisa: 420), which the 8-bit `Blend.buf` cannot index: every vertex's full bone ids as `R16_UINT`, and per remapped component 512 `uint16` local -> merged / merged -> local -- WWMI's blend remap, exactly as WWMI Tools writes it; `Blend.buf` then holds the ids truncated to 8 bits. A character with eight weights a vertex (Chisa, Augusta) has a 16-byte `Blend.buf` |
| `<Name>Texture<hash>.dds` | every texture the asset ships, named by the hash the game binds it under -- which is how a WWMI mod names them too (`Components-0-1 t=<hash>.dds`) and how the prototype's role table keys them |
| `<Name>Metadata.json`, `<Name>TextureUsage.json` | the asset's own manifests: the components, their windows, `vg_offset` / `vg_count` / `vg_map`, the shape-key hashes and checksum; and per component, per `ps-t` slot, which texture hash the dump saw bound and under which shaders |

The buffers are the character's **identity mod** (`Tools/Misc/Prototypes/wwmiIdentityMod.py` over
`WWMI-Assets/PlayerCharacterData/<Name>`), byte for byte: the same files shown in game as
`WWMI/SanhuaIdentity` (clean on Sanhua, 2026-09-19). There is no golden to rebuild here as there is
for the GI folders; the proof of a new folder is that its identity mod renders on its own character.
`Tools/Misc/Prototypes/wwmiDownloadFolder.py` writes a folder from an asset folder, and `--check`
compares against a shipped one (both Sanhua folders reproduce exactly).

**A character WWMI-Assets does not have** (Chisa, ChisaParfait, 2026-09-19) gets its asset folder
from a frame dump: `Tools/Misc/Prototypes/wwmiExtractDump.py` runs WWMI Tools' own extractor on it.
The geometry that comes out is exact -- run on the Sanhua and SanhuaExorcist dumps it reproduces
WWMI-Assets and both shipped folders' buffers byte for byte, and the Chisa and ChisaParfait identity
mods built from their dumps render correctly in game (Chisa through the blend remap). **The textures are not**: a dump holds
each texture in whatever streaming state it was drawn in (most of Sanhua's and Chisa's came out at
512 x 512 where the full texture is 2048), and 3DMigoto rehashes a texture as its mips load, so the
dump's hashes are the partly-loaded ones. Check the extracted `.dds` sizes before shipping them.

Not yet wired into a parser: `DownloadTools::urlPath` composes `GI/<char>/<version>/...` and needs
a game folder before a WuWa fixer can fetch these (2026-09-19).
