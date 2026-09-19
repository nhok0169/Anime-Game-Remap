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
| `<Name>Texture<hash>.dds` | every texture the asset ships, named by the hash the game binds it under -- which is how a WWMI mod names them too (`Components-0-1 t=<hash>.dds`) and how the prototype's role table keys them |
| `<Name>Metadata.json`, `<Name>TextureUsage.json` | the asset's own manifests: the components, their windows, `vg_offset` / `vg_count` / `vg_map`, the shape-key hashes and checksum; and per component, per `ps-t` slot, which texture hash the dump saw bound and under which shaders |

The buffers are the character's **identity mod** (`Tools/Misc/Prototypes/wwmiIdentityMod.py` over
`WWMI-Assets/PlayerCharacterData/<Name>`), byte for byte: the same files shown in game as
`WWMI/SanhuaIdentity` (clean on Sanhua, 2026-09-19). There is no golden to rebuild here as there is
for the GI folders; the proof of a new folder is that its identity mod renders on its own character.

Not yet wired into a parser: `DownloadTools::urlPath` composes `GI/<char>/<version>/...` and needs
a game folder before a WuWa fixer can fetch these (2026-09-19).
