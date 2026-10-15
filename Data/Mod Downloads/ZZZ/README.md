# Mod Downloads: Zenless Zone Zero

The download assets of the Zenless Zone Zero characters, one folder per character and one subfolder
per game version, the same arrangement as [`../GI`](../GI). A ZZMI character is drawn the GIMI way --
several **components**, each with its own buffers and index buffer, each index buffer split into
objects by `match_first_index` -- so a folder has the GI multi-component shape (YelanTranquil's):

| file | what it is |
| --- | --- |
| `<Name><Comp>Position.buf` | `POSITION` + `NORMAL` + `TANGENT`, stride 40 (the pre-skin pass's `vb0`) |
| `<Name><Comp>Texcoord.buf` | the pre-skin pass's `vb1`. **Its stride is per component**: Miyabi's `Body` is 24 and her other parts 20, the `Face` 48, `HairShadow` 24 |
| `<Name><Comp>Blend.buf` | four `R32_FLOAT` weights then four `R32_UINT` bone indices, stride 32 (`vb2`) |
| `<Name><Comp><Obj>.ib` | each object's index range, `R32_UINT` (the game's own is `R16_UINT`); objects are lettered `A`, `B`, `C` in first-index order |
| `<Name><Comp><Obj><Role>.dds` | the textures that object's draw binds, by the role ZZMI names them: `Diffuse` (`ps-t3`), `NormalMap` (`ps-t4`), `LightMap` (`ps-t5`), `MaterialMap` (`ps-t6`) on the main character shader; the face and hair-shadow shaders bind fewer, at other slots |
| `<Name>Hash.json` | per component: `ib`, `draw_vb`, `position_vb`, `blend_vb`, `texcoord_vb`, the strides and vertex count, every object's first index and count, and each object's texture hashes by role -- what the `HashData` / `IndexData` / `VertexCountData` rows are read from |

No asset repo was used: both folders were built from frame dumps of the character menu's outfit
preview (unowned agents and outfits preview there, and the dump has no mods loaded) by
`Tools/Misc/Prototypes/zzmiDownloadFolder.py`. 3DMigoto dumps each bound buffer whole, so the buffers
are the pre-skin pass's own bytes, never re-encoded.

## Miyabi / MiyabiBlossom (ZZZ 3.2, 2026-10-15)

| character | components | dump |
| --- | --- | --- |
| `Miyabi` | `Body` (ib `981c1a1e`), `Hair` (`4faabaac`, objects A + B), `Legs` (`d8003df3`), `Face` (`dbd59d30`, A/B/C), `HairShadow` (`acff032e`) | `FrameAnalysis-Miyabi-2026-10-15-093322` |
| `MiyabiBlossom` ("Dignified Blossom") | `Body` (`a913e9a9`), `Dress` (`fbb18630`), `Hair` (`ecaf558f`, A + B), `Face` (`dbd59d30`, A/B/C), `HairShadow` (`244eb75a`) | `FrameAnalysis-MiyabiBlossom-2026-10-15-093639` |

The normal map (`ebac056e`, the same texture on every main-shader object of both outfits) came out of
the default dump as a `.jpg`, so it is taken from a second, textures-only dump
(`FrameAnalysis-MiyabiBlossomDds-2026-10-15-094351`, `dump_tex dds`).

Things the data says that are not guessable:

- **The two outfits share their face**: same buffer hashes, same ib, same textures. Only `Position.buf`
  differs between the folders (962 of 2346 vertices, by at most 2 mm) because the face's pre-skin
  buffer carries the shape-key expression of the frame it was dumped in.
- **The skin reuses Miyabi's hair textures** (`012e84e9` / `a6ea6d83` / `d5462e37`) on a different hair
  mesh; her `Legs` draw with those same textures too.
- Mods name the components inconsistently (`Head` / `Hair`, `ArmsLegs` / `Legs`, the skin as
  `MiyabiSkin`); the names here follow the majority of the Miyabi mods.

How the folders were proved, with no golden to rebuild:

1. `zzmiDownloadFolder.py --verify` rebuilds every buffer and ib a second way, from the dump's `.txt`
   listings rather than its `.buf` files: 23 of 23 identical per character. (3DMigoto lists a float16
   DENORMAL wrongly -- half `0x0001` prints as `1.1479437e-41` -- which the verifier inverts; Miyabi's
   `Legs` texcoord has 36 of them.) The check was shown to fail on a copy with one flipped byte.
2. 11 of the 14 distinct textures are pixel-identical to untouched originals shipped inside the mods
   (`MiyabiBlossom4/Textures/OG`, `Miyabi1`, `Miyabi6`, ...).
3. Every distinct vertex position of the skin's `Body`, `Dress` and `Hair` appears in a real skin
   mod's buffers (MiyabiBlossom4), and 99.8% of Miyabi's `Hair` in Miyabi1's.
