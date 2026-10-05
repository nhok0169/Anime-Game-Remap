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
| `<Name>/<Name>HashLineage.json` (beside the version folders) | the character's OLDER texture hashes -> the hash the game binds today, each pair measured by pixels (Sanhua's from mods, SanhuaExorcist's out of WWMI-Assets' git history). What `wwmiTextureFix.py --maps` and the remap prototypes' role tables read to place a REPAINTED texture of a mod exported for an older game version, which no pixel match can |
| `<Name>Metadata.json`, `<Name>TextureUsage.json` | the asset's own manifests: the components, their windows, `vg_offset` / `vg_count` / `vg_map`, the shape-key hashes and checksum; and per component, per `ps-t` slot, which texture hash the dump saw bound and under which shaders |

The buffers are the character's **identity mod** (`Tools/Misc/Prototypes/wwmiIdentityMod.py` over
`WWMI-Assets/PlayerCharacterData/<Name>`), byte for byte: the same files shown in game as
`WWMI/SanhuaIdentity` (clean on Sanhua, 2026-09-19). There is no golden to rebuild here as there is
for the GI folders; the proof of a new folder is that its identity mod renders on its own character.
`Tools/Misc/Prototypes/wwmiDownloadFolder.py` writes a folder from an asset folder, and `--check`
compares against a shipped one (both Sanhua folders reproduce exactly).

`Chisa/2_8` and `ChisaParfait/3_5` (2026-09-20) are the first folders built this way, and Chisa is
the first character here whose merged skeleton (420 bones) needs the blend remap buffers.

**WUTHERING WAVES 3.7 MOVED ONE OF THE FOUR, AND THE OTHER THREE ARE UNTOUCHED (2026-09-30).** The
game updated its model system at 3.7 and every mod stopped matching, which reads like a rehash of
everything. Re-dumped and rebuilt, it is not:

| character | at 3.7 |
| --- | --- |
| `Chisa` | **unchanged**. All 12 buffers, `Metadata.json` and every texture byte-identical to `2_8`; `vb0` still `afa1587c` |
| `Sanhua` | **unchanged**. All 9 buffers and `Metadata.json` byte-identical to `2_5`; `vb0` still `33e4890f` |
| `SanhuaExorcist` | **unchanged**. All 9 buffers and `Metadata.json` byte-identical to `2_5`; `vb0` still `b101dcf3` |
| `ChisaParfait` | **moved**: `vb0` `e611d493` -> `95ecef77`, and `Position` / `Texcoord` / `Color` / `ShapeKeyVertexOffset` carry small edits (0.2 - 1.4% of their bytes, every count and offset the same). `Index`, `Blend`, `Vector`, `ShapeKeyOffset` and `ShapeKeyVertexId` are byte-identical, all 27 of her textures are byte-identical under the SAME hashes, and `cb4`, `shapekey_offsets` (`57bb099f`) and `shapekey_scale` (`9c738856`) are unchanged too -- so exactly one hash row moves. `ChisaParfait/3_7` is that folder, with six more textures the 3.5 dump never caught (`327bfd4a`, `50bb611a`, `90adf0cf`, `bb73967a`, `c7c8e963`, `eac20ee2` -- the face, eye and prop passes' `ps-t0`-`ps-t4`) |

Three shipped folders reproduced byte for byte from fresh 3.7 dumps is a stronger proof of the
pipeline than the usual one-character `--check`, and it is what makes `3_7` trustworthy. `3_7` was
then verified a SECOND way, against the dump's own bytes rather than the extractor that built it
(Creating Remaps' "Proving a NEW download folder"): 3DMigoto writes each bound buffer whole, and
`Position`, `Vector`, `Color`, `Texcoord` and `Index` are byte-identical to the slots the 27 draws
of `vb0=95ecef77` bound (`vb0`, `vb1=d3e7581e`, `vb3=ee32541e`, `vb2=c21e0513`, `ib=65023833`).
`Blend.buf` is deliberately not one of them -- the extractor rewrites each component's local bone
ids into the merged skeleton, so 47.7% of its bytes differ from the raw `vb4` -- and the three shape
key buffers are rebuilt sparse. No `HashLineage.json` is written beside `3_7`: every texture hash it
would record is the one `3_5`'s already does, and `HashData.cpp`'s 3.5 rows still cite that file.

**A character WWMI-Assets does not have** (Chisa, ChisaParfait, 2026-09-19) gets its asset folder
from a frame dump: `Tools/Misc/Prototypes/wwmiExtractDump.py` runs WWMI Tools' own extractor on it.
The geometry that comes out is exact -- run on the Sanhua and SanhuaExorcist dumps it reproduces
WWMI-Assets and both shipped folders' buffers byte for byte, and the Chisa and ChisaParfait identity
mods built from their dumps render correctly in game (Chisa through the blend remap). **The textures are not**: a dump holds
each texture in whatever streaming state it was drawn in, and 3DMigoto rehashes a texture as its mips
load, so the dump's hashes are the partly-loaded ones. **Dump with the game's LOD bias at its
MAXIMUM** (`ImageDetail` in `Client/Saved/LocalStorage/LocalStorage.db`, set from the in-game menu --
the launcher's own switch is overwritten by the game at startup): at the default `0` every character
texture is 512 x 512 however close the camera is. **The top of that scale is not a fixed number**:
it was `3` ("Ultra High") on 2026-09-20 and is `2` ("High") at 3.7, where the menu's right arrow
greys out -- so read the value the menu will actually accept rather than looking for a named option,
and confirm by the extracted sizes (at `2`, ChisaParfait's set comes out 19 x 2048, 3 x 1024,
12 x 512, matching every size `3_5` shipped). **And take the dump from the CHARACTER MENU, not the
overworld** -- see the Game View guide: an overworld dump of one character at 3.7 was 36857 files and
9.8 GB, Unreal's watchdog killed the game, and WWMI Tools' extractor aborted on an unrelated NPC
whose skeleton buffer was short, naming neither the object nor its hash. Check the extracted `.dds` sizes before shipping
them, and read the extractor's warnings -- a dump whose `cb4_hash` comes back empty has bone counts
that are not the character's. **Take every mod of that character out of `Mods` before dumping**: with
her own identity mod installed, ChisaParfait extracted as a 929-slot skeleton (against 264) with the
MOD's textures under the mod's hashes, every time, and nothing reported an error. See the VGRemaps
guide.

**`Lynae/3_7` and `LynaePeppermint/3_7` (2026-10-04)** come from two character-menu dumps at 3.7 with
LOD bias `2` (High, the menu's maximum), no Lynae mod installed: `FrameAnalysis-Lynae-2026-10-04-232242`
(original outfit) and `FrameAnalysis-LynaePeppermint-2026-10-04-232459` (the Peppermint outfit's
preview in Resonator Outfits, which works for an outfit the account does not own).

| character | `vb0` | components | merged skeleton | blend remap | textures |
| --- | --- | --- | --- | --- | --- |
| `Lynae` | `7e400733` (`ib` `27e35645`) | 8, 75828 vertices | 401 slots, highest bone 366 | components 4, 5, 6 | 26: 17 x 2048, 3 x 1024, 6 x 512 |
| `LynaePeppermint` | `ebbfa346` (`ib` `b41c509e`) | 8, 71603 vertices | 315 slots, highest bone 310 | components 4, 6 | 27: 13 x 2048, 7 x 1024, 7 x 512 |

Both are eight-weight characters (16-byte `Blend.buf`), and both shape-key checksums match their
`Metadata.json` (2020 and 4892). **Lynae's extraction needed `wwmiExtractDump.py --only`**: the addon
aborted with `components CB4 hash mismatch for object 7e400733`, because her component 6 (indices
301470+) is drawn twice in a pass that binds the SCENE's `vs-cb4` (`4785ce09`) as well as four times
with her own (`f02baf77`), and the addon keeps one draw per component. `--only` drops a kept object's
draws that bind a minority `cb4` (calls `000045`, `000046` here) and every other object's draws. It was
proved by re-extracting `ChisaParfait/3_7` from its own 3.7 dump with `--only 95ecef77`: `--check`
reports 44 identical, 0 differ. Both new folders were then verified against the dumps' own bytes by
`wwmiCheckDownload.py`: `Position`, `Vector`, `Color`, `Texcoord` and `Index` are byte-identical to
the slots their draws bound (Lynae `vb1=086b1dc5`, `vb2=5b48efca`, `vb3=483b3ebd`; LynaePeppermint
`vb1=41d28b52`, `vb2=25beb1d7`, `vb3=cfc48cd0`), 0 unexplained.

Not yet wired into a parser: `DownloadTools::urlPath` composes `GI/<char>/<version>/...` and needs
a game folder before a WuWa fixer can fetch these (2026-09-19).
