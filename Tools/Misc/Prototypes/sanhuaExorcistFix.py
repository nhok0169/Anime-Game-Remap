#
# ===== sanhuaExorcistFix =====
#
# PROTOTYPE of the first Wuthering Waves remap: a WWMI mod made for Sanhua, fixed IN PLACE so it
# also renders on her Exorcist skin (WWMI-Assets' SanhuaSkin1) -- the same shape as
# yelanTranquilFix.py and bennettAdventureFix.py: a parser and a fixer built from Python, registered
# through CppStrategyOverrides ahead of the library's stub rows, and driven by the API's own
# RemapService over the mod folder. The fix lives in the mod's own .ini file, appended as a
# `; --------------- Sanhua Remap ---------------` block after the mod's sections; the remapped
# blend is written beside the mod's own; a second run undoes the first before fixing again; --undo
# removes it. Read AI Agent Help/VGRemaps/CLAUDE.md's "WuWa: Sanhua <-> SanhuaExorcist" first: what
# a WWMI mod is (one mesh, per-component draw ranges, ONE merged skeleton, sparse shape keys,
# textures overridden by hash), and why every number below was read off the two frame dumps of
# 2026-09-19 rather than reasoned about.
#
#   python sanhuaExorcistFix.py <mod folder> [--keepBackups] [--hideOrig] [--undo] [--verbose]
#                              [--anchor ribbons|all] [--vgRemap <json: {source group: target group}>]
#                              [--plan default|rnd] [--shapeKeys hide|leave|retarget] [--hideTextureOverrides] [--noSplit]
#
# ---- Everything it knows about the two characters comes from the LIBRARY (2026-09-19) ----
#
# WWMIBuilder.sanhua() / sanhuaExorcist() are the source of every number below: the vertex-buffer,
# bone-data and shape-key hashes (Hashes, types vb0 / cb4 / shapekey_offsets / shapekey_scale), every
# draw slot's match_first_index (Indices, type componentN), match_index_count (IndexCounts), vg_offset
# (VGOffsets) and vg_count (VGCounts), the shape-key checksum (ShapeKeyChecksums), the mesh vertex
# count (VertexCounts) and the vertex group remap (VGRemaps, through ModType.getVGRemap). The blend is
# remapped by the API's own BlendFile given the 8-byte WWMI layout (four R8 indices, four R8 weights).
# So this script holds nothing of the characters but the SHADER-side facts the tables do not carry
# yet: the slot plan, the register roles, the main-pass pixel shaders and the mask legend. Those are
# what the WWMI fixer template will grow config fields for.
#
# ---- What the API does with a WWMI .ini, and what this script adds to it (2026-09-19) ----
#
# The GIMIParser sorts the mod's sections by the library's hashes exactly as it sorts a GIMI mod's:
# the seven [TextureOverrideComponentN] by the vb0 hash plus their match_first_index (Indices rows
# typed componentN), [TextureOverrideMarkBoneDataCB] by the cb4 hash, the shape-key overrides by
# theirs. Each object's graph follows its `run =` calls, so the shared CommandListMergeSkeleton /
# TriggerResourceOverrides / OverrideSharedResources / CleanupSharedResources come along and are
# written once, renamed. The GIMIFixer then applies, per object: RegAssetRemap on `hash` (vb0 -> vb0,
# cb4 -> cb4, shape keys -> shape keys, all through the Hashes remap map) and on the shape-key
# checksum (ShapeKeyChecksums, the reverse-then-forward lookup the table was added for); RegNewVals
# for the target slot's match_first_index / match_index_count / vg_offset / vg_count (the slot plan
# is not the identity, so these are read from the target's tables rather than remapped by type);
# RegSurroundedAdd of a `run =` to a per-component texture command list; ResRegCollect of the blend
# register into a RemapBlend resource; GraphRename with the Remap keyword. What no graph edit can
# express is text the fixer appends inside the fix block (GIMIFixer.appendedSections): the texture
# command lists (an `if ps == <filter>` block), the section for the target slot nothing is drawn
# through, the [ShaderOverride] filter tags and the invented mask's resource.
#
# Two things the API needed for this, both in core now: a section line with no `=` (3dmigoto's
# `local $var`, which opens every WWMI section's state guard) used to be dropped on parse, and a
# reverse index lookup with no version resolves through the NEWEST bucket holding the value -- for
# `0` that is GI's 6.1, not WuWa's 2.5, so the classifier is handed WuWa's version explicitly.
#
# ---- What a WuWa remap has to do, and what this one does (2026-09-19) ----
#
#  1. RETARGET THE DRAW SLOTS. A [TextureOverrideComponentN] matches the game's draw of one component
#     by (vb0 hash, match_first_index, match_index_count) and then skips it and draws the mod's own
#     index range instead. On the target those three numbers are the TARGET's (hash b101dcf3 and her
#     six index ranges), and each slot's merge-skeleton step carries the TARGET's vg_offset /
#     vg_count, so the merged skeleton is Exorcist's bones. Every target slot gets a section even
#     when nothing of the mod is drawn through it: the section still has to skip the skin's own
#     geometry (or Exorcist's bun and trousers render under the mod) and still has to merge that
#     component's bones (the remap points at them).
#  2. REMAP THE BLEND. Meshes/Blend.buf is 8 bytes a vertex (four R8 bone indices, four R8 weights)
#     in Sanhua's merged skeleton; the RemapBlend sends each index through the library's VGRemaps
#     row (Data/RemapDrafts/SanhuaRemapDraft.xlsx, 'Sanhua -> SanhuaExorcist'). Weights untouched.
#  3. WHICH SOURCE COMPONENT GOES THROUGH WHICH TARGET SLOT is decided by the SHADER FAMILY, and the
#     bones do not constrain it (any draw can use any merged bone). The main-pass pixel shaders per
#     component, off the dumps:
#         Sanhua   0 bangs a512f04f + f6bc3927 (t0 diffuse ae6e9014; drawn twice, so the eyes show
#                    through) + an eye-region pass 94d9d5e9 that binds only globals
#                  1 hair 69e3d321  2 face 374a4f8f  6 eyes 056f9f3c (t1 eye mask, t2 iris; t0 a global)
#                  3 arm skin 7a0ab7c3 (t0 normal, t1 diffuse)
#                  4 bodice+hat+ribbons+boots 96356f03 (t0 normal, t1 material mask, t2 diffuse)
#                  5 skirt 96356f03
#         Exorcist 0 bangs 69e3d321 + 8fbb5532 (the hair shader itself, plus the see-through pass)
#                    + the same eye-region pass 94d9d5e9
#                  1 hair 69e3d321  2 face 374a4f8f  5 eyes 056f9f3c   (identical)
#                  3 torso+arms+ribbons 3093e3c7 (t0 normal, t1 material mask, t2 diffuse)
#                  4 hair bun + trousers 5cc08ed6 (t0 normal, t1 a dynamic global, t2 diffuse)
#     So bangs/hair/face/eyes map one to one -- and the bangs are the one slot whose PASSES differ
#     between the skins: the first in-game run (2026-09-19) gated the bangs on the eye-region pass and
#     bound the eye mask there, so Sanhua's bangs rendered with Exorcist's bang texture ("the hair
#     texture seems off"). A slot's bindings are gated on EVERY pass that binds character textures on
#     the target (SlotPasses); the bodice and the skirt go through the torso slot
#     (same slot layout, a mask slot to carry their masks); the bare arms have NO skin family on
#     Exorcist and go through the torso slot too, with a mask that says "skin" everywhere (below);
#     and the bun+trousers slot draws nothing of the mod. Several source components through one
#     target slot are several SECTIONS matching the same draw -- 3dmigoto runs every
#     [TextureOverride] whose hash and index window match, which is what a GIMI mod's shared IB
#     section relies on -- each binding its own textures and drawing its own range. WWMI does not
#     need the second .ini file the GIMI merge writes.
#  4. TEXTURES ARE BOUND BY REGISTER ON THE TARGET'S DRAWS, NOT BY HASH. A mod's [TextureOverrideTexture]
#     sections match the SOURCE's texture hashes, which never occur while Exorcist is drawn (and the
#     hashes drift with texture streaming and game versions anyway), so they are left alone -- they
#     still serve the mod on Sanhua -- and each remapped section binds ps-t0..2 / ps-t5 itself
#     through a command list. The bindings are gated on the target's MAIN-pass pixel shader through
#     [ShaderOverride] filter_index tags (the mechanism WWMI's own core uses for its compute
#     shaders): the outline / shadow / other passes of the same draw range keep the game's
#     textures, as they would under hash overrides. Slots ps-t3 and up are globals identical on both
#     skins and are never bound.
#  5. THE MATERIAL MASK LEGEND (ps-t1 of the body families) was measured against the diffuse under it:
#     on Exorcist's torso the texels coloured (255, 77, 0) are 99% skin-coloured in the diffuse and
#     nothing else is (the shared face mask uses the same code); Sanhua's own body masks hold only
#     (0, 0, 0) and (203, 0, 0), no skin, because her skin is its own component. The maintainer's
#     hand-made 'Components-4 light map.dds' from 2025 is a solid (255, 77, 0) -- the same finding.
#     So the arm skin is drawn with an invented solid (255, 77, 0) mask; the bodice and skirt keep
#     their own masks. OPEN: Exorcist's non-skin texels carry G = 50 where Sanhua's carry G = 0
#     ((0,50,0) / (203,50,0) vs (0,0,0) / (203,0,0)); if her cloth shades wrong in game, +50 on G
#     of the two masks is the first edit to try.
#  6. SHAPE KEYS: the mod keeps its own three buffers (the face keys are numbered the same on both
#     skins -- same face mesh), and the override hashes / checksum become the TARGET's, since the
#     ShapeKeyOverrider matches the game's live constant buffer by checksum.
#
#  7. THE CHAINS EXORCIST HAS NO COUNTERPART FOR are the open question (first in-game run, 2026-09-19:
#     "the body seems all wavy", curls of black cloth at both hands). A per-bone tally of the blend
#     (blendTally in the session scratchpad; the finger bones match Exorcist's to the millimetre and the
#     skirt chains map chain to chain in order) puts the mismatch on Sanhua's two long back ribbons
#     (69-75 and 62/78/76/77/79-81, landing on Exorcist's 49-59 / 52-62 with rest positions 5-14 cm
#     apart, worst at the tips) and the front-left belt tassel (169-171 / 157 on 162 / 164, 8-20 cm).
#     A vertex bound to a chain bone whose rest position is off by that much is rotated about the
#     wrong pivot once the chain bends, and those ribbons bend ~90 degrees in the idle pose: that is
#     what crumples. `--anchor ribbons` pins each of those chains to its ROOT bone's target (the knot
#     at the neck, the belt's top) -- rigid, no physics, no curling -- which is the Yelan lesson 5
#     (Creating Remaps' "The Yelan lessons", point 5); `--anchor all` does the same to every skirt
#     chain, for the case where the skirt itself is what waves. `--vgRemap <json>` takes any table
#     at all, e.g. a sheet of Data/RemapDrafts/SanhuaRemapDraft.xlsx exported with openpyxl under
#     py -3.11. Whichever variant looks right in game is what goes into VGRemapData.cpp.
#
#  8. THE SHAPE KEYS ARE OFF BY DEFAULT (--shapeKeys turns them on), BECAUSE THE MAINTAINER'S OWN
#     WORKING HAND REMAP HAS THEM OFF. Their Sanhua2 (WWMI/Sanhua2/.../mod copy 2.ini, 2025) applies
#     the SAME vertex group table as the library row -- read back off its RemapBlend.buf: identical on
#     159 of the 162 groups it uses, the other three its blank rows -- and its ribbons do not wave. What
#     it does differently is leave the shape-key sections on Sanhua's own hashes with the checksum line
#     commented out, so the ShapeKeyOverrider never engages on Exorcist. Retargeting them (point 6)
#     hands Sanhua's 30805 shape-key vertices to a buffer the game dispatches for Exorcist's 27267, and
#     that is the prime suspect for "the body seems all wavy". Off, the mod's shape-key sections are
#     left untouched (they still serve it on Sanhua) and the face does not animate on Exorcist; on,
#     they are copied and retargeted as before. Their bangs also bind a mask at ps-t1 (the 2025
#     'Components-0' UNORM texture); today's equivalent is the shared default mask 1c0c8b91, whose
#     mean pixel matches what the bangs pass binds at ps-t1 in the dump.
#
#  9. BISECTING AGAINST THAT HAND REMAP (second in-game report, 2026-09-19: still wavy with the shape
#     keys left alone and the ribbons anchored; the identity mod itself is clean on Sanhua). The FULL
#     diff of `mod copy 2.ini` against its export is four things beyond the retarget: the arm-skin
#     component dropped, the bodice through slot 4 (its vertex shader differs from slot 3's), every
#     [TextureOverrideTexture] commented out, every shape-key section commented out -- the two
#     buffer-size overrides ([TextureOverrideShapeKeyOffsets] / [TextureOverrideShapeKeyScale])
#     included, which "left alone" kept ACTIVE. Each is a switch now: --plan rnd (bodice -> slot 4,
#     no arm skin), --shapeKeys hide (the default: the mod's own shape-key sections commented out with
#     the API's HideOrig marker, so --undo restores them) and --hideTextureOverrides. The four
#     variants to test are in the VGRemaps guide. The vertex COLOUR is also a suspect worth knowing
#     about: Exorcist's torso carries R = 0 on every vertex and her coat R = 0..255 (a sway weight,
#     by the look of it) where Sanhua carries R = 255 everywhere but the face -- but the hand remap
#     binds the same Color.buf and does not wave, so it is not the difference between the two.
#
# 10. THE ANSWER (third in-game report, 2026-09-19): A and D clean, B and C wavy -- so anything drawn
#     through the TORSO slot waves and the same thing through slot 4 does not. The torso, face and
#     eye draws consume a SIXTH vertex stream, vb6 (stride 24; the ShapeKeyOffsets buffer, Metadata's
#     `shapekeys.offsets_hash`, d709b169 on Exorcist), which the game's shape-key compute fills with
#     the live per-vertex offsets, indexed by VERTEX ID; the vertex shader adds them. WWMI's shared
#     override rebinds vb0-vb4 and leaves vb6 alone, so a mod vertex drawn through such a slot gets
#     whatever offset Exorcist's buffer holds at the SAME index: 67% of Sanhua's arm-skin vertices and
#     11% of her bodice coincide with ids Exorcist animates (her face and torso), 1-2% of the skirt,
#     none of the hair -- exactly the reported pattern. Slot 4's vertex shader reads no vb6, which is
#     why the hand remap's bodice-through-slot-4 was clean, and why dropping the arm skin "fixed" it.
#     So when the shape keys are not retargeted, every remapped section binds vb6 to a buffer of ZERO
#     offsets, 24 bytes a vertex for the mod's vertex count (ShapeKeyZero); with --shapeKeys retarget
#     WWMI's own pipeline is expected to fill the resized buffer with the mod's offsets instead
#     (unverified: the first in-game run had it retargeted and still waved).
#
# 11. ONE SECTION PER DRAW PER FILE (the maintainer's tip after the zero stream did not help either,
#     2026-09-19; the shape of the GIMI merge, and of their own R&D, which spreads its sections over
#     several .ini files). Every variant that was clean had exactly ONE remapped section matching each
#     Exorcist draw; every one that waved had two or three on the torso slot's draw. So the fix
#     splits by TARGET WINDOW: the first section of a window stays in mod.ini, each further one moves
#     into its own <stem>RemapFix<n>.ini (named as the API names a merge's copies, so either undo
#     removes them) -- a complete copy of the fixed file, the mod's own
#     draw sections commented out (only mod.ini serves Sanhua), every OTHER remapped slot section cut
#     down to its skeleton merge and `handling = skip` -- so every file merges the whole skeleton and
#     draws exactly its own share. --noSplit keeps everything in one file. The extra files are deleted
#     before a fix and on --undo. Whether this or the component itself is what waves is what
#     `WWMI/Sanhua2_F_split` decides.
#
# 12. THE EYES READ THE IRIS AT ps-t2 (fifth in-game report, 2026-09-19: everything right but the eyes).
#     The eye pass (056f9f3c) binds the eye mask at ps-t1 and the iris at ps-t2 on both skins -- the
#     dump's textures correlate 1.00 with the asset's c88cc1fc and 1dcc0f1d there -- and at ps-t0 a
#     texture that matches NO asset file (an eye highlight the game keeps). The plan had put the iris
#     at ps-t0, which the asset-era TextureUsage.json had also said was t2; the dump is the reference.
#     Checked at the same time: ps-t5 really is 1035197c on Sanhua and 4478285f on Exorcist (1.00 each).
#
# 13. A TEXTURE'S ROLE COMES FROM THE HASH ITS OVERRIDE MATCHES, NOT FROM ITS FILE NAME (the frost mod,
#     2026-09-19). The exporter names files `Components-N t=<hash>.dds`, which is what the first two
#     mods carried and what the role lookup keyed on -- and the frost mod names its skin, bodice and
#     skirt art `Component3.dds` / `Component3-NM.dds` / `Component3-LM.dds`, so every torso texture came
#     back roleless and slot 3 would have drawn with the GAME's. What identifies a texture is the hash
#     in its [TextureOverrideTexture] section (current once wwmiTextureFix has run) and that section's
#     `this =` names the resource; the file-name hash is only the fallback for a resource no override
#     names. A toggled override (`if $x / this = A / else / this = B`) contributes its first resource.
#
# 14. A MOD'S TEXTURES MAY BE DECLARED IN ANOTHER .INI ENTIRELY, BOUND BY A LIBRARY THAT IS NOT INSTALLED,
#     AND NAMED BY COMPONENT (the cloak mod, 2026-09-19). Its LOD0/mod.ini declares no texture at all: a
#     top-level SanhuaCloak.ini (its own namespace) holds `[ResourceDiffuse0] filename = Textures/
#     Component0_Diffuse.dds` and sections on Sanhua's hash that hand them to RabbitFX, a shared library
#     mod that binds them by register -- and RabbitFX is not installed here, so the mod renders with the
#     GAME's textures even on Sanhua. So the textures are found by a run-level INDEX of every .dds under
#     the folder the run was pointed at (skipping DISABLED-prefixed folders like the game does), and a
#     file's role is decided in this order: the hash an override in ANY .ini of the tree matches for it
#     (point 13), the hash in its file name, PIXEL IDENTITY with one of the game's own textures in the
#     download folder (colour correlation >= 0.97 with one asset and < 0.90 with every other; the
#     cloak's `Component6_Diffuse.dds` is the ps-t5 ramp by its pixels, not the iris its name says), and
#     last the `Component<N>_<Diffuse|LM|NM>` name convention (the RabbitFX / WWMI-Tools export names) for
#     the repainted ones a pixel match cannot place. A file the fixed .ini has no resource for is declared
#     as the fix's own `[Resource<Role>SanhuaExorcistRemapFix] filename = ..\Textures\...`. The same
#     mod also showed the vb6 line and the texture run landing INSIDE the first `if $draw_component_x`
#     toggle: RegSurroundedAdd's optional after-register (drawindexed) is a MUST fact, and behind a
#     toggle no draw is certain, so the earliest certain spot was inside the toggle -- the frost mod
#     escaped it only because one of its draws sat outside every toggle. The add is anchored on the
#     shared-resource override alone now. Its LOD1 / LOD2 .ini files are on Sanhua's LOD hashes, which
#     the library does not know, so at a distance the game draws Exorcist's own LOD model.
#
# Not done, deliberately, until the identity mod has been seen in game: any texture edit beyond the
# invented mask, the arm normal map's B / A channels (Sanhua's skin family stores B = 0, A = 255
# where the body families vary both), and the hair / head extra passes.
#

import argparse
import json
import os
import re
import shutil
import struct
import sys
from typing import Dict, List, Optional, Tuple

OnWindows = (sys.platform == "win32")


def winToPosix(path: str) -> str:
    if (OnWindows or (len(path) < 2) or (path[1] != ":") or (not path[0].isalpha())):
        return path
    return "/mnt/" + path[0].lower() + path[2:].replace("\\", "/")


Repo = os.environ.get("AG_REMAP_REPO") or winToPosix(r"C:\Users\AlexX\Documents\Games\Mods\Repos\Anime-Game-Remap")
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path on this OS")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

SourceName, TargetName = "Sanhua", "SanhuaExorcist"
WuWaVersion = "2.5"               # the version the library files both characters under (see the header on why the classifier needs it)
SlotPrefix = "component"          # the 'type' the library files a WWMI draw slot under: component0, component1, ...
ShapeKeyType = "shapekeys"        # the 'type' of the ShapeKeyChecksums row

SkinMaskColour = (255, 77, 0, 255)          # Exorcist's material-mask code for skin (measured; the maintainer's hand-made mask agrees)
SkinMask = "SkinMask"                       # the invented mask's resource / file stem, suffixed with the RemapTex keyword
ShapeKeyZero = "ShapeKeyZero"               # the zero shape-key offset stream bound at vb6 (see the header, point 10)
ShapeKeyStride = 24                         # bytes a vertex in that stream, off the frame dump's vb6 layout

# The target's pixel shaders that bind CHARACTER textures, per draw slot, off
#   FrameAnalysis-SanhuaExorcist-2026-09-19-005636 (wwmiDrawTable.py): a slot's texture command list
#   binds on every one of them. Slot 0 (the bangs) has two -- the hair shader and the see-through pass
#   over the eyes -- and its third pass (94d9d5e9, the eye region) binds only globals, so it is not here.
SlotPasses = {0: ["69e3d3219c979981", "8fbb55320274f090"], 1: ["69e3d3219c979981"], 2: ["374a4f8fc9a5ea6a"],
              3: ["3093e3c72c791456"], 4: ["5cc08ed68341dd38"], 5: ["056f9f3c356ff96e"]}
FilterBase = 3381.91   # one filter_index per distinct shader, 3381.91 up, beside WWMI's own 3381.3333 / 3381.4444 / 3381.7777
PassFilters = {ps: f"{FilterBase + 0.01 * i:.4f}".rstrip("0") for i, ps in enumerate(dict.fromkeys(ps for passes in SlotPasses.values() for ps in passes))}

# Sanhua's textures by the hash WWMI Tools filed them under (WWMI-Assets/PlayerCharacterData/Sanhua,
#   TextureUsage.json), with the role each plays in its component's MAIN pass (read off the 2026-09-19
#   dump's slot table: the same slots at the streamed hashes 7441ca3d / 61e07d7b / 9c8839ed ...)
Roles = {
    # c88cc1fc ('Components-0-6') is the EYE MASK the bangs pass and the eyes share (green with the two
    #   eye shapes); ae6e9014 ('Components-0-1') is the bangs' own diffuse, pixel-identical to what the
    #   bangs pass binds at ps-t0 today; 1dcc0f1d ('Components-6') is the iris; 1035197c (every
    #   component) is bound at ps-t5 on the head, hair and eye passes -- a ramp of some kind, UNVERIFIED
    "c88cc1fc": "eyeMask", "ae6e9014": "bangsDiffuse", "1dcc0f1d": "irisDiffuse", "1035197c": "t5Ramp",
    "1c0c8b91": "bangsMask",      # 'Components-0-1-2-3-4-5': the shared default mask every outline pass and the bangs pass bind at ps-t1
    "68ca7071": "hairDiffuse", "cef6494f": "hairNormal",
    "46177147": "faceMask", "881c236d": "faceDiffuse",
    "e39835c7": "skinNormal", "fde0f298": "skinDiffuse",
    "efb25eb3": "bodiceNormal", "89ba19a1": "bodiceMask", "abda232b": "bodiceDiffuse",
    "f3b217ab": "skirtNormal", "f0713dc7": "skirtMask", "c689a8ee": "skirtDiffuse",
    # the same textures under the hashes of the 2024-2025 game versions the maintainer's mods were
    #   exported from (Sanhua1 = KanouSakura's mod, Sanhua2 = the maintainer's simplified export):
    #   2584190a -> cef6494f and 28708ab8 -> 0bd3b5ab are in wwmi_fix_23's hash_maps.json; the rest are
    #   read off the file's format and 'Components-N' tag (UNORM 2048 = normal, DXT1 = mask, sRGB = diffuse)
    #   and are UNVERIFIED in game
    "2584190a": "hairNormal", "98b9635b": "hairDiffuse", "aa70ef15": "faceDiffuse",
    "03d9850b": "skinNormal", "4b6d52b9": "skinDiffuse",
    "0521977a": "bodiceNormal", "ebeeda8c": "bodiceDiffuse", "5efe7892": "bodiceMask",
    "16695017": "skirtNormal", "2c0c2728": "skirtDiffuse", "11b9cadd": "skirtMask",
    "1bdd0987": "t5Ramp",        # measured: the 2025 mods' file correlates 1.00 with 1035197c. 0bd3b5ab / 28708ab8 were listed here
                                 #   as the ramp too and are NOT (0.00): a 2048 UNORM shared texture with no current twin -- and
                                 #   two hashes on one role made the script bind whichever came first (the Witch mod's eyes, 2026-09-19)
    "48616ac9": "bangsDiffuse", "3cd03f60": "irisDiffuse",     # the 2025 'Components-0' sRGB 2048 and the 1024 sRGB the eyes share
    "345368c9": "bangsMask",                                    # the 2025 'Components-0' UNORM 2048, which the maintainer's hand remap binds at ps-t1
    # the LIVE hashes of the game's own textures at LOD bias Ultra High (the 2026-09-20 max-LOD
    #   frame dump, each measured 1.00 against the asset file): what a mod exported from a dump today
    #   carries, where the asset repo's hashes are what WWMI Tools' own exports carry
    "b0828323": "bangsDiffuse", "332a6aac": "bangsMask", "07f0a2ea": "hairDiffuse", "466478e4": "hairNormal",
    "bf16c0c7": "faceMask", "d2765954": "faceDiffuse", "7aee2f16": "skinNormal", "fa792b59": "skinDiffuse",
    "091a8707": "t5Ramp", "8141e933": "bodiceNormal", "02746d45": "bodiceMask", "fcecfdcb": "bodiceDiffuse",
    "01ef7c05": "skirtNormal", "0b5be4da": "skirtMask", "1cd3181e": "skirtDiffuse", "5764478b": "irisDiffuse",
    "d0524bfb": "eyeMask",
}

# source component -> (target slot, {ps register: role, or the invented mask})
Plan = {
    0: (0, {"ps-t0": "bangsDiffuse", "ps-t1": "bangsMask", "ps-t5": "t5Ramp"}),
    1: (1, {"ps-t0": "hairDiffuse", "ps-t1": "hairNormal", "ps-t5": "t5Ramp"}),
    2: (2, {"ps-t0": "faceMask", "ps-t1": "faceDiffuse"}),
    6: (5, {"ps-t1": "eyeMask", "ps-t2": "irisDiffuse", "ps-t5": "t5Ramp"}),   # ps-t0 is a global the game keeps (matches no asset texture)
    4: (3, {"ps-t0": "bodiceNormal", "ps-t1": "bodiceMask", "ps-t2": "bodiceDiffuse"}),
    5: (3, {"ps-t0": "skirtNormal", "ps-t1": "skirtMask", "ps-t2": "skirtDiffuse"}),
    3: (3, {"ps-t0": "skinNormal", "ps-t1": SkinMask, "ps-t2": "skinDiffuse"}),
}
# The maintainer's hand remap's plan (WWMI/Sanhua2/.../mod copy 2.ini): the bodice through slot 4 and
#   the arm skin not drawn at all. A source component absent from the plan has its graph REMOVED, so
#   nothing of it is written.
Plans = {
    "default": Plan,
    "rnd": {i: Plan[i] for i in (0, 1, 2, 6, 5)} | {4: (4, Plan[4][1])},
    "rndArms": {i: Plan[i] for i in (0, 1, 2, 6, 5, 3)} | {4: (4, Plan[4][1])},   # rnd, plus the arm skin through the torso slot
}
Labels = {0: "bangs", 1: "hair", 2: "face", 3: "arm skin", 4: "bodice, hat, ribbons, boots", 5: "skirt", 6: "eyes"}
TargetLabels = {0: "bangs", 1: "hair", 2: "face", 3: "torso, arms, ribbons", 4: "hair bun, trousers", 5: "eyes"}

# The chains Exorcist has no counterpart for, as {root source group: [the chain's other groups]}: with
#   --anchor every listed group takes its ROOT's target, so the chain hangs rigidly from the bone the
#   root maps to instead of crumpling on bones whose rest positions are centimetres off. Groups from
#   the hand-made sheet's comments and the identity mod's per-bone centroids (see the header, point 7).
AnchorChains = {
    "ribbons": {
        69: [70, 71, 72, 73, 74, 75],             # the left back ribbon, knot at the neck downwards
        62: [78, 76, 77, 79, 80, 81],             # the right back ribbon
        168: [169, 170, 171, 157],                # the dangling belt at the left front of the skirt
    },
    "skirt": {
        156: [153, 152, 154, 155, 184],           # back skirt, centre, top to hem
        173: [174, 177, 179, 181, 183],           # left back skirt
        175: [176, 195, 196, 197, 201],           # right back skirt
        162: [164, 178, 180, 182, 185],           # left side
        194: [193, 200, 198, 199, 202],           # right side
        148: [161, 163],                          # front middle
        158: [159, 160],                          # the charm at the front right waist
        86: [191, 190, 189],                      # left dangling thing from the back skirt
        104: [207, 206, 205],                     # right dangling thing from the back skirt
    },
}


def anchoredRemap(remap: Dict[int, int], mode: str) -> Dict[int, int]:
    """'remap' with every chain of the given anchor mode ('ribbons', or 'all' for ribbons + skirt) pinned
    to its root's target"""
    out = dict(remap)
    for name, chains in AnchorChains.items():
        if (mode == "all" or mode == name):
            for root, members in chains.items():
                for g in members:
                    out[g] = remap[root]
    return out

# The WWMI command lists every component section runs, by the value of its `run =` line. The texture
#   command list is added right after the shared-resource override and before the first draw.
OverrideSharedResources = "CommandListOverrideSharedResources"

_alive: List[object] = []     # Python-built edits, classifiers and resources the C++ side holds only by reference


# ---------------------------------------------------------------------------------------------
# the library

def characterFromLibrary(modType):
    """Everything the fix needs to know about one WWMI character, read out of its ModType's tables
    (the same shape WWMI-Assets' Metadata.json has). A missing row is an error: a zero here would
    look like a real value."""
    name = modType.name

    def hashOf(kind):
        value = modType.hashes.get([name, kind], None, False)
        if (value is None):
            raise SystemExit(f"the library has no '{kind}' hash for {name}")
        return value

    components = []
    while (True):
        slot = f"{SlotPrefix}{len(components)}"
        first = modType.indices.get([name, "", slot], None, False)
        if (first is None):
            break
        count, vgOffset, vgCount = modType.getIndexCount(slot), modType.getVGOffset(slot), modType.getVGCount(slot)
        if (count is None or vgOffset is None or vgCount is None):
            raise SystemExit(f"the library has {name}'s {slot} match_first_index but not all of its match_index_count / vg_offset / vg_count")
        components.append({"index_offset": int(first), "index_count": int(count), "vg_offset": int(vgOffset), "vg_count": int(vgCount)})
    if (not components):
        raise SystemExit(f"the library has no draw slots (Indices rows typed {SlotPrefix}N) for {name}")

    checksum = modType.getShapeKeyChecksum(ShapeKeyType)
    vertexCount = modType.getVertexCount()
    if (checksum is None or vertexCount is None):
        raise SystemExit(f"the library has no shape-key checksum or vertex count for {name}")

    return {"name": name, "vb0_hash": hashOf("vb0"), "cb4_hash": hashOf("cb4"), "vertex_count": int(vertexCount),
            "index_count": components[-1]["index_offset"] + components[-1]["index_count"], "components": components,
            "shapekeys": {"offsets_hash": hashOf("shapekey_offsets"), "scale_hash": hashOf("shapekey_scale"), "checksum": int(checksum)}}


def wwmiBlendElements():
    """The 8-byte WWMI blend line: four R8 bone indices then four R8 weights (Metadata.json's
    export_format 'Blend'; the API's default BlendFile layout is GIMI's 32-byte one)"""
    uint8 = FRB.BufUnSignedInt(name = "UnsignedInt8", size = 1)
    return [FRB.BufElementType(FRB.BufElementNames.BlendIndices.value, "R8G8B8A8_UINT", [uint8] * 4),
            FRB.BufElementType(FRB.BufElementNames.BlendWeight.value, "R8G8B8A8_UINT", [uint8] * 4)]


def writeSolidDds(path, colour, size = 16):
    """An uncompressed R8G8B8A8_UNORM DDS of one colour"""
    w = h = size
    header = bytearray(b"DDS ")
    flags = 0x1 | 0x2 | 0x4 | 0x1000 | 0x8      # caps, height, width, pixelformat, pitch
    header += struct.pack("<IIIIIII", 124, flags, h, w, w * 4, 0, 1)
    header += b"\0" * 44                          # reserved
    header += struct.pack("<II4sIIIII", 32, 0x4, b"DX10", 0, 0, 0, 0, 0)   # pixel format: fourcc DX10
    header += struct.pack("<IIIII", 0x1000, 0, 0, 0, 0)                   # caps
    header += struct.pack("<IIIII", 28, 3, 0, 1, 0)                        # DX10: R8G8B8A8_UNORM, texture2d, 1 array
    body = bytes(colour) * (w * h)
    with open(path, "wb") as f:
        f.write(bytes(header) + body)


# ---------------------------------------------------------------------------------------------
# the parser

SlotObjs = [("", f"{SlotPrefix}{i}") for i in range(7)]
BoneDataObjs = {"cb4": ("", "boneData")}
ShapeKeyObjs = {"shapekey_offsets": ("", "shapekeyOffsets"), "shapekey_scale": ("", "shapekeyScale")}


def hashOnlyObjs(shapeKeys: bool) -> Dict[str, Tuple[str, str]]:
    """The objects identified by a hash alone: the bone-data override always, the shape-key overrides
    only when the run retargets them (see the header, point 8)"""
    return {**BoneDataObjs, **(ShapeKeyObjs if shapeKeys else {})}


def makeParser(sourceType, shapeKeys: bool = False):
    """
    A GIMIParser whose sections are sorted by the API's hash / index classifier -- the seven draw
    slots by the vb0 hash plus their match_first_index, the bone-data and shape-key overrides by a
    hash of their own. The texture overrides carry hashes the library does not know and stay
    unclassified: they keep serving the mod on Sanhua and never fire while Exorcist is drawn.
    """
    def factory(iniFile, modTypeId):
        # The version is passed explicitly. A reverse lookup with no version resolves through the
        # NEWEST bucket holding the value, and `0` (component 0's index) is every GI head's index
        # too, filed at 6.1: that bucket holds no Sanhua row, so component 0 classified as nothing.
        version = iniFile.fromVersion if (iniFile.fromVersion is not None) else WuWaVersion
        hashOnly = hashOnlyObjs(shapeKeys)
        classifier = FRB.GIMISectionClassifier(dict(hashOnly), sourceType.hashes, {"vb0": {obj: obj for obj in SlotObjs}}, sourceType.indices, version)
        classifier.hashNonVersionVals = {"name": SourceName}
        classifier.indexNonVersionVals = {"name": SourceName}
        parser = FRB.GIMIParser(iniFile, modObjs = SlotObjs + list(hashOnly.values()), objTargetFuncs = [classifier], modTypeId = modTypeId)
        parser.trackKeys = True
        parser.keysToTrack = {"hash", "match_first_index", "match_index_count"}
        _alive.append(classifier)
        return parser
    return factory


# ---------------------------------------------------------------------------------------------
# the mod's own files, read off the .ini the API parsed

SectionPattern = re.compile(r"^\[(?P<name>[^\]]+)\]\s*$")


def iniSections(text: str) -> Dict[str, List[str]]:
    """{section name: [lines]} of an .ini's text, comments included"""
    sections: Dict[str, List[str]] = {}
    current = None
    for line in text.splitlines():
        match = SectionPattern.match(line)
        if (match):
            current = match.group("name")
            sections[current] = []
        elif (current is not None):
            sections[current].append(line)
    return sections


def keyValue(line):
    stripped = line.strip()
    if (not stripped or stripped.startswith(";") or "=" not in stripped):
        return None, None
    key, _, value = stripped.partition("=")
    return key.strip(), value.strip()


# ---------------------------------------------------------------------------------------------
# the mod's textures, by role (header point 14)

ServiceRoot: Optional[str] = None       # the folder the run was pointed at: a mod's textures may be declared in any .ini of it
AssetsFolder = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", SourceName, WuWaVersion.replace(".", "_"))

# A planned role the mod has NO file for is bound to the SOURCE's own game texture (the mod's UVs are the
# source's, so the target's texture -- what an unbound register samples on the target's draw -- is wrong
# by construction: the red-camellia mod ships no bodice or skirt mask, and the Exorcist's mask at its UVs
# put skin codes over cloth, a reddish hue over the whole body, 2026-09-19). Role -> the source's hash;
# the API downloads the same file from the repo, the prototype copies it out of AssetsFolder. Roles whose
# hash BOTH skins bind (eyeMask c88cc1fc, faceMask 46177147) need none: the target's texture IS the source's.
FallbackTextures: Dict[str, str] = {
    "bangsDiffuse": "ae6e9014", "bangsMask": "1c0c8b91", "t5Ramp": "1035197c",
    "hairDiffuse": "68ca7071", "hairNormal": "cef6494f", "faceDiffuse": "881c236d",
    "skinNormal": "e39835c7", "skinDiffuse": "fde0f298",
    "bodiceNormal": "efb25eb3", "bodiceMask": "89ba19a1", "bodiceDiffuse": "abda232b",
    "skirtNormal": "f3b217ab", "skirtMask": "f0713dc7", "skirtDiffuse": "c689a8ee",
    "irisDiffuse": "1dcc0f1d",
}
IdentityMin, IdentityGap = 0.97, 0.90   # a file IS a game texture when its colour correlates >= IdentityMin with one asset and < IdentityGap with every other
ComponentFilePattern = re.compile(r"component[\s_-]*(\d+)[\s_-]+([a-z]+)\.dds$", re.IGNORECASE)
TypeOfSuffix = {"diffuse": "diffuse", "albedo": "diffuse", "base": "diffuse", "color": "diffuse", "colour": "diffuse", "d": "diffuse",
                "lm": "mask", "lightmap": "mask", "mask": "mask", "m": "mask", "nm": "normal", "normal": "normal", "normalmap": "normal", "n": "normal"}
TypeRoles = {0: {"diffuse": "bangsDiffuse", "mask": "bangsMask"},
             1: {"diffuse": "hairDiffuse", "mask": "hairNormal", "normal": "hairNormal"},     # cef6494f reads as a mask by its pixels; the label is historical
             2: {"diffuse": "faceDiffuse", "mask": "faceMask"},
             3: {"diffuse": "skinDiffuse", "normal": "skinNormal"},
             4: {"diffuse": "bodiceDiffuse", "mask": "bodiceMask", "normal": "bodiceNormal"},
             5: {"diffuse": "skirtDiffuse", "mask": "skirtMask", "normal": "skirtNormal"},
             6: {"diffuse": "irisDiffuse", "mask": "eyeMask"}}


class TextureIndex():
    """Every .dds under the run's root with the role it plays, and every .ini's resource sections -> files"""

    def __init__(self, root: str):
        self.root = root
        self.resourcesByIni: Dict[str, Dict[str, str]] = {}      # .ini abs path -> resource section -> file abs path
        self.roleOf: Dict[str, List[Tuple[str, str]]] = {}       # file abs path -> [(role, how it was decided)]: EVERY role its hashes name
        self.unresolved: List[str] = []
        self.real: Dict[str, str] = {}                           # matching key (case-folded abs path) -> the file's real spelling
        hashesOfFile: Dict[str, List[str]] = {}
        ddsFiles: List[str] = []
        remapFix = FRB.IniKeywords.RemapFix.value.lower()
        for folder, dirs, names in os.walk(root):
            dirs[:] = sorted(d for d in dirs if (not d.upper().startswith("DISABLED")))
            for name in sorted(names):
                path = os.path.normcase(os.path.abspath(os.path.join(folder, name)))
                low = name.lower()
                if (low.endswith(".dds") and FRB.IniKeywords.RemapTex.value.lower() not in low):
                    ddsFiles.append(path)
                    self.real[path] = os.path.abspath(os.path.join(folder, name))
                elif (low.endswith(".ini") and not name.upper().startswith("DISABLED") and remapFix not in low):
                    with open(path, "r", encoding = "utf-8", errors = "replace") as f:
                        sections = iniSections(f.read())
                    resources: Dict[str, str] = {}
                    for sec, lines in sections.items():
                        if (sec.startswith("Resource") and remapFix not in sec.lower()):
                            fileName = next((v for k, v in map(keyValue, lines) if k == "filename"), "")
                            if (fileName.lower().endswith(".dds")):
                                resources[sec] = os.path.normcase(os.path.abspath(os.path.join(folder, fileName.replace("\\", "/"))))
                    self.resourcesByIni[path] = resources
                    for sec, lines in sections.items():
                        if (sec.startswith("TextureOverrideTexture")):
                            kvps = [keyValue(line) for line in lines]
                            h = next((v.lower() for k, v in kvps if k == "hash"), None)
                            res = next((v for k, v in kvps if k == "this"), None)
                            if (h and res in resources):
                                hashesOfFile.setdefault(resources[res], []).append(h)
        counts = {"hash": 0, "pixels": 0, "name": 0}
        pending: List[str] = []
        for f in ddsFiles:
            match = re.search(r"t=([0-9a-fA-F]{8})\.dds$", f)
            hashes = hashesOfFile.get(f, []) + ([match.group(1).lower()] if (match) else [])
            # a mod declares one file under two hashes when one atlas serves two components (Upper_D.dds as
            # both the arm skin's and the bodice's diffuse); taking the first role only left the second
            # component unbound, drawing with the TARGET's textures (2026-09-19)
            roles: List[Tuple[str, str]] = []
            for h in hashes:
                if (h in Roles and Roles[h] not in [r for r, _ in roles]):
                    roles.append((Roles[h], f"hash {h}"))
            if (roles):
                self.roleOf[f] = roles; counts["hash"] += 1
            else:
                pending.append(f)
        for f, h, score in self._identify(pending):
            self.roleOf[f] = [(Roles[h], f"the game's own {h} by its pixels ({score:.2f})")]; counts["pixels"] += 1
        for f in pending:
            if (f in self.roleOf):
                continue
            match = ComponentFilePattern.search(os.path.basename(f))
            role = TypeRoles.get(int(match.group(1)), {}).get(TypeOfSuffix.get(match.group(2).lower(), "")) if (match) else None
            if (role):
                self.roleOf[f] = [(role, "its name")]; counts["name"] += 1
            else:
                self.unresolved.append(f)
        print(f"  textures under {os.path.basename(root)}: {len(ddsFiles)} files -- {counts['hash']} placed by hash, {counts['pixels']} by pixel identity with a game texture, "
              f"{counts['name']} by their Component<N>_<Type> name, {len(self.unresolved)} with no role")

    def _identify(self, files: List[str]):
        """(file, asset hash, colour correlation) for every file that is one of the game's own textures under another name"""
        if (not files or not os.path.isdir(AssetsFolder)):
            return
        sys.path.insert(0, os.path.join(Repo, "Tools", "Misc", "Diagnostics"))
        import wwmiTextureFix as texFix
        cache: Dict[str, object] = {}
        assets = {h: texFix.decode(p, cache) for h, p in texFix.currentTextures(AssetsFolder).items() if h in Roles}
        for f in files:
            x = texFix.decode(f, cache)
            if (x is None):
                continue
            scores = sorted(((texFix.corr(x[..., :3], y[..., :3]), h) for h, y in assets.items() if y is not None), reverse = True)
            if (scores and scores[0][0] >= IdentityMin and (len(scores) == 1 or scores[1][0] < IdentityGap)):
                yield f, scores[0][1], scores[0][0]


_textureIndex: Optional[TextureIndex] = None


def textureIndex() -> TextureIndex:
    global _textureIndex
    if (_textureIndex is None or _textureIndex.root != ServiceRoot):
        _textureIndex = TextureIndex(ServiceRoot)
    return _textureIndex


class ModFiles():
    """
    One mod's textures by role and its draws per source component, read off the .ini file the API
    parsed and the run's texture index (header points 13 and 14): a role is bound through the .ini's
    own resource section when it has one for that file, else through a resource the fix declares
    """
    def __init__(self, ini, parser, source):
        self.sections = iniSections(ini.fileTxt)
        self.iniFolder = ini.folder
        self.present: List[int] = []         # source components the mod has a section for
        self.draws: Dict[int, int] = {}      # source component -> its number of drawindexed / custom-shader draws
        targets = parser._sectionTargets
        for i in range(len(source["components"])):
            names = targets.get(("", f"{SlotPrefix}{i}")) or []
            if (not names):
                continue
            self.present.append(i)
            self.draws[i] = sum(1 for name in names for line in self.sections.get(name, [])
                                if (keyValue(line)[0] == "drawindexed" or (keyValue(line)[0] == "run" and keyValue(line)[1].startswith("CustomShader"))))

        # the textures: the run's index decides each file's role; this .ini binds the nearest file of each
        # role -- through its own resource section when it has one for that file, else through a resource
        # the fix declares (`declare`), with a path relative to this .ini's folder
        index = textureIndex()
        iniPath = os.path.normcase(os.path.abspath(ini.file))
        iniFolder = os.path.dirname(iniPath)
        resourceOfFile: Dict[str, str] = {}
        for res, f in index.resourcesByIni.get(iniPath, {}).items():
            resourceOfFile.setdefault(f, res)
        real = lambda f: index.real.get(f, f)       # noqa: E731 -- the file's real spelling, for what gets written
        self.textureFolder = next((os.path.dirname(os.path.relpath(real(f), iniFolder)).replace("\\", "/") for f in resourceOfFile), "") or "Textures"
        self.resourceOfRole: Dict[str, str] = {}
        self.declared: Dict[str, str] = {}          # role -> the file's path relative to this .ini, for a file no resource of this .ini names
        self.unknownTextures: List[str] = [os.path.relpath(f, index.root) for f in index.unresolved]
        byRole: Dict[str, List[Tuple[str, str]]] = {}      # role -> [(file, how)], every role of every file
        for f, roles in index.roleOf.items():
            for role, how in roles:
                byRole.setdefault(role, []).append((f, how))

        def rank(f: str):
            rel = os.path.relpath(f, iniFolder).replace("\\", "/")
            return (0 if (f in resourceOfFile) else 1, rel.count("../"), len(rel))
        for role, cands in byRole.items():
            cands.sort(key = lambda c: rank(c[0]))
            best = cands[0][0]
            if (len(cands) > 1 and rank(cands[1][0])[:2] == rank(best)[:2]):
                # two shipped textures on one role, equally close: the first is bound and only a measurement
                # (wwmiTextureFix's correlation) can say which is right -- say so loudly
                print(f"    WARNING: {os.path.relpath(cands[1][0], index.root)} also has the role {role} ({cands[1][1]}), "
                      f"already taken by {os.path.relpath(best, index.root)} ({cands[0][1]}); the first one is bound")
            if (best in resourceOfFile):
                self.resourceOfRole[role] = resourceOfFile[best]
            else:
                self.declared[role] = os.path.relpath(real(best), iniFolder).replace("/", "\\")

    def fallbacks(self, roles: List[str]) -> List[str]:
        """The resource sections for the planned 'roles' this .ini has no file for: the source's own game
        texture, copied out of AssetsFolder into the texture folder as <Source><Role>RemapDL.dds (the name the
        API downloads it under, so the two outputs match); binds them"""
        out: List[str] = []
        for role in roles:
            if (role in self.resourceOfRole or role == SkinMask or role not in FallbackTextures):
                continue
            src = os.path.join(AssetsFolder, f"{SourceName}Texture{FallbackTextures[role]}.dds")
            if (not os.path.isfile(src)):
                print(f"    WARNING: no {os.path.basename(src)} under {AssetsFolder} for the {role} the mod lacks")
                continue
            fileName = f"{SourceName}{role[0].upper()}{role[1:]}{FRB.IniKeywords.RemapDL.value}.dds"
            rel = f"{self.textureFolder}/{fileName}"
            os.makedirs(os.path.join(self.iniFolder, self.textureFolder), exist_ok = True)
            shutil.copyfile(src, os.path.join(self.iniFolder, self.textureFolder, fileName))
            name = f"Resource{SourceName}{role[0].upper()}{role[1:]}{FRB.IniKeywords.RemapDL.value}"
            self.resourceOfRole[role] = name
            out.append("\n".join([f"[{name}]", f"filename = {rel}", ""]))
        return out

    def declare(self, fixName) -> List[str]:
        """The resource sections this .ini needs for the files none of its own resources name; binds them"""
        out: List[str] = []
        for role, rel in self.declared.items():
            # RemapRef, not RemapFix: the section sits inside the fix block and names one of the MOD's
            # files, and the API's undo deletes every file a RemapFix section in the block names --
            # it took 17 of the cloak mod's 19 textures before the keyword existed (2026-09-19)
            name = f"Resource{role[0].upper()}{role[1:]}{TargetName}{FRB.IniKeywords.RemapRef.value}"
            self.resourceOfRole[role] = name
            out.append("\n".join([f"[{name}]", f"filename = {rel}", ""]))
        return out


# ---------------------------------------------------------------------------------------------
# the fixer

class WWMIBlendReplace(FRB.RemapBlendReplace):
    """RemapBlendReplace whose bytes this script supplies: the API's BlendFile over the 8-byte WWMI
    layout, through the library's VGRemaps row. The built resource is kept referenced from Python."""

    def buildResModel(self, *args, **kwargs):
        resource = super().buildResModel(*args, **kwargs)
        if (resource is not None):
            _alive.append(resource)
        return resource


def remapWWMIBlend(vgRemap, forced: bool):
    """A RemapBlendResource fixFunc: Blend.buf -> the resource's fixed path, indices through 'vgRemap'
    (the library's row the resource carries, unless 'forced' says the script's table wins)"""
    def fix(resource) -> bool:
        remap = vgRemap if (forced or getattr(resource, "vgRemap", None) is None) else resource.vgRemap
        FRB.BlendFile(resource.srcPath, wwmiBlendElements()).remap(remap, fixedBlendFile = resource.fixedPath)
        return True
    return fix


def effectiveRemap(sourceType, target, remapOverride: Optional[Dict[int, int]], anchor: Optional[str]):
    """The vertex group remap this run writes the blend with: the library's row, or --vgRemap's table,
    with --anchor's chains pinned; checked to cover every group the library knows and to point inside
    the target's merged skeleton"""
    library = sourceType.getVGRemap(TargetName)
    if (library is None):
        raise SystemExit(f"the library has no vertex group remap {SourceName} -> {TargetName}")
    base = {int(k): int(v) for k, v in dict(library.remap).items()}
    remap = dict(remapOverride) if (remapOverride is not None) else base
    missing = sorted(set(base) - set(remap))
    if (missing):
        raise SystemExit(f"--vgRemap has no row for source groups the library's row maps: {missing}")
    if (anchor):
        remap = anchoredRemap(remap, anchor)
    targetSlots = int(target["components"][-1]["vg_offset"]) + int(target["components"][-1]["vg_count"])
    if (max(remap.values()) >= targetSlots):
        raise SystemExit(f"the remap points outside Exorcist's {targetSlots} merged slots")
    changed = sum(1 for g in base if remap.get(g) != base[g])
    forced = (remapOverride is not None) or bool(anchor)
    print(f"  vertex group remap: {'the library row' if not forced else ('--vgRemap' if remapOverride is not None else 'the library row')}"
          + (f", --anchor {anchor}" if anchor else "") + f" ({len(remap)} rows, {changed} differ from the library row)")
    return FRB.VGRemap(remap), forced


def makeFixer(sourceType, targetType, remapOverride: Optional[Dict[int, int]] = None, anchor: Optional[str] = None, shapeKeys: bool = False,
              planName: str = "default"):
    plan = Plans[planName]
    source, target = characterFromLibrary(sourceType), characterFromLibrary(targetType)
    vgRemap, forcedRemap = effectiveRemap(sourceType, target, remapOverride, anchor)
    naming = FRB.CppIniNamingTools

    def factory(parser, toModName: str, modTypeId: int):
        ini = parser._iniFile
        modType = FRB.ModTypeIdTools.getModType(modTypeId)
        files = ModFiles(ini, parser, source)
        if (not files.present):
            raise ValueError(f"no [TextureOverrideComponent*] section on {SourceName}'s hash {source['vb0_hash']}")
        print(f"  {os.path.relpath(ini.file, ini.folder) if ini.folder else ini.file}: {SourceName} components {files.present} -> {toModName}")

        def fixName(name: str) -> str:
            return naming.getRemapFixName(name, toModName)

        maskResource = fixName(f"Resource{SkinMask}")      # a 3dmigoto resource section's name starts with Resource

        appended: List[str] = files.declare(fixName)     # the textures this .ini has no resource of its own for
        appended += files.fallbacks([role for i in files.present if (i in plan) for role in plan[i][1].values()])
        # ---- the zero shape-key offset stream, unless WWMI's own pipeline is retargeted to fill vb6 ----
        zeroResource = None
        if (not shapeKeys):
            constants = {k: v for k, v in map(keyValue, files.sections.get("Constants", [])) if k}
            vertexCount = int(constants.get("global $mesh_vertex_count", 0))
            if (vertexCount <= 0):
                raise ValueError("no `global $mesh_vertex_count` in [Constants], so the zero shape-key stream cannot be sized")
            blendFile = next((v for k, v in map(keyValue, files.sections.get("ResourceBlendBuffer", [])) if k == "filename"), "Meshes/Blend.buf")
            zeroFile = os.path.join(os.path.dirname(blendFile.replace("\\", "/")), f"{toModName}{FRB.IniKeywords.Remap.value}{ShapeKeyZero}.buf").replace("\\", "/")
            os.makedirs(os.path.dirname(os.path.join(ini.folder, zeroFile)) or ini.folder, exist_ok = True)
            with open(os.path.join(ini.folder, zeroFile), "wb") as f:
                f.write(bytes(vertexCount * ShapeKeyStride))
            zeroResource = fixName(f"Resource{ShapeKeyZero}")
            appended.append("\n".join([f"[{zeroResource}]", "type = Buffer", "format = DXGI_FORMAT_R32G32B32_FLOAT", f"stride = {ShapeKeyStride}", f"filename = {zeroFile}", ""]))
            print(f"    vb6 (the game's shape-key offsets, applied by vertex id): every remapped draw binds {vertexCount} zero offsets instead")

        # ---- the edits shared by every object: the target's hashes, the target's checksum, the names ----
        hashRemap = FRB.RegAssetRemap({"hash": (modType.hashes, FRB.IniKeywords.HashNotFound.value),
                                       "$\\WWMIv1\\shapekey_checksum": (modType.shapeKeyChecksums, "ChecksumNotFound")},
                                      toModName, SourceName, ini.fromVersion, ini.toVersion)
        rename = FRB.GraphRename(fixName)
        perObj: Dict[Tuple[str, str], List[object]] = {obj: [hashRemap, rename] for obj in hashOnlyObjs(shapeKeys).values()}
        print(f"  shape keys: {'retargeted to ' + toModName if shapeKeys else 'not retargeted (the mod' + chr(39) + 's own sections are hidden or left alone per --shapeKeys)'}")

        # ---- the draw slots: retargeted, and handed their texture command list ----
        dropped = [i for i in files.present if (i not in plan)]
        if (dropped):
            print(f"    not drawn under --plan {planName}: components {dropped} ({', '.join(Labels[i] for i in dropped)})")
        for i in files.present:
            if (i not in plan):
                perObj[("", f"{SlotPrefix}{i}")] = []
                continue
            slot, regs = plan[i]
            c = target["components"][slot]
            edits: List[object] = []
            bindings = [f"    {reg} = {maskResource if (role == SkinMask) else files.resourceOfRole[role]}"
                        for reg, role in regs.items() if (role == SkinMask or role in files.resourceOfRole)]
            additions: List[Tuple[str, str]] = [("vb6", zeroResource)] if (zeroResource) else []
            if (bindings):
                cmdList = fixName(f"CommandList{SourceName}{SlotPrefix.capitalize()}{i}Textures")
                condition = " || ".join(f"ps == {PassFilters[ps]}" for ps in SlotPasses[slot])
                appended.append("\n".join([f"[{cmdList}]", f"if {condition}"] + bindings + ["endif", ""]))
                additions.append(("run", cmdList))
            if (additions):
                # right after the shared-resource override (the mod's buffers are bound there): the EARLIEST spot
                # after it, so a component drawn in several ranges has its textures before the FIRST draw. No
                # after-register: `drawindexed` as one is a MUST fact, and behind a `$draw_x` toggle no draw is
                # certain, which parked the additions INSIDE the first toggle (header point 14)
                edits.append(FRB.RegSurroundedAdd(additions,
                                                  beforeRegs = {"run": lambda v: v == OverrideSharedResources},
                                                  latest = False))
            edits.append(FRB.RegNewVals({"match_first_index": str(c["index_offset"]), "match_index_count": str(c["index_count"]),
                                         "$\\WWMIv1\\vg_offset": str(c["vg_offset"]), "$\\WWMIv1\\vg_count": str(c["vg_count"])}))
            edits += [hashRemap, rename]
            perObj[("", f"{SlotPrefix}{i}")] = edits

        graphEdits: List[object] = [FRB.GraphGroupEdit([perObj])]
        if (dropped):
            graphEdits.append(FRB.GraphRemove([(0, "", f"{SlotPrefix}{i}") for i in dropped]))

        # ---- the blend: the register bound in the shared override, collected into a RemapBlend ----
        graphEdits.append(FRB.ResRegCollect({(0, "", f"{SlotPrefix}{i}"): "vb4" for i in files.present if (i in plan)},
                                            {"blend": WWMIBlendReplace((0, "", "blend"), resType = "blend", fixFunc = remapWWMIBlend(vgRemap, forcedRemap))}))

        # ---- the target slots nothing is drawn through: skipped, and their bones still merged ----
        drawnSlots = {plan[i][0] for i in files.present if (i in plan)}
        for slot, c in enumerate(target["components"]):
            if (slot in drawnSlots):
                continue
            name = f"TextureOverride{toModName}{SlotPrefix.capitalize()}{slot}{FRB.IniKeywords.Remap.value}Hide"
            appended.append("\n".join([
                f"; nothing of the mod is drawn through {toModName}'s {TargetLabels.get(slot, slot)} slot: the skin's own geometry is skipped and its bones still merged",
                f"[{name}]", f"hash = {target['vb0_hash']}", f"match_first_index = {c['index_offset']}", f"match_index_count = {c['index_count']}",
                "$object_detected = 1", "if $mod_enabled", f"    local $state_id_{slot}", f"    if $state_id_{slot} != $state_id", f"        $state_id_{slot} = $state_id",
                f"        $\\WWMIv1\\vg_offset = {c['vg_offset']}", f"        $\\WWMIv1\\vg_count = {c['vg_count']}", f"        run = {fixName('CommandListMergeSkeleton')}", "    endif",
                "    if ResourceMergedSkeleton !== null", "        handling = skip", "    endif", "endif", ""]))

        # ---- the shader tags the texture command lists ask about, and the invented mask ----
        for i, (ps, filterIndex) in enumerate(PassFilters.items()):
            appended.append("\n".join([f"[{fixName(f'ShaderOverridePass{i}')}]", f"hash = {ps}", f"filter_index = {filterIndex}", ""]))
        if (any(role == SkinMask for i in files.present if (i in plan) for role in plan[i][1].values())):
            maskFile = os.path.join(files.textureFolder, f"{SkinMask}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
            os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
            writeSolidDds(os.path.join(ini.folder, maskFile), SkinMaskColour)
            appended.append("\n".join([f"[{maskResource}]", f"filename = {maskFile}", ""]))

        _alive.extend(graphEdits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = graphEdits, modsToFix = [toModName])
        fixer.appendedSections = "\n".join(appended)
        _alive.append(fixer)

        # ---- report ----
        for i in files.present:
            if (i not in plan):
                continue
            slot, regs = plan[i]
            bound = ", ".join(f"{reg}={maskResource if (role == SkinMask) else files.resourceOfRole.get(role, 'GAME (mod has none)')}" for reg, role in regs.items())
            print(f"    slot {slot} {TargetLabels[slot]:<22} <- {i} {Labels[i]:<28} {files.draws[i]} draws  {bound}")
        for slot in range(len(target["components"])):
            if (slot not in drawnSlots):
                print(f"    slot {slot} {TargetLabels[slot]:<22} <- (nothing; the skin's own geometry skipped)")
        if (files.unknownTextures):
            print(f"    textures with no role (left to the mod's own hash overrides): {len(files.unknownTextures)}")
            for u in files.unknownTextures:
                print(f"      {u}")
        return fixer
    return factory


# ---------------------------------------------------------------------------------------------
# run

ShapeKeySections = ["TextureOverrideShapeKeyOffsets", "TextureOverrideShapeKeyScale", "CommandListSetupShapeKeys", "CommandListLoadShapeKeys",
                    "TextureOverrideShapeKeyLoaderCallback", "CommandListMultiplyShapeKeys", "TextureOverrideShapeKeyMultiplierCallback"]
HideMarker = FRB.IniKeywords.HideOriginalComment.value


def hideOriginalSections(iniPath: str, matches) -> int:
    """Comments out, with the API's HideOrig marker (so an undo restores them), every section of the mod's
    OWN text -- everything before the fix block -- whose name 'matches' accepts. Returns the count."""
    with open(iniPath, "r", encoding = "utf-8", newline = "") as f:
        text = f.read()
    ending = "\r\n" if ("\r\n" in text) else "\n"
    lines = text.split(ending)
    header = next((k for k, line in enumerate(lines) if line.startswith("; ---") and "Remap" in line), len(lines))
    hidden, inside = 0, False
    for k in range(header):
        line = lines[k]
        match = SectionPattern.match(line)
        if (match):
            inside = matches(match.group("name")) and not line.startswith(HideMarker)
            hidden += int(inside)
        if (inside and line.strip() and not line.startswith(HideMarker)):
            lines[k] = HideMarker + line
        if (not line.strip()):
            inside = False
    with open(iniPath, "w", encoding = "utf-8", newline = "") as f:
        f.write(ending.join(lines))
    return hidden


SlotSectionPattern = re.compile(rf"^TextureOverride{SlotPrefix}\d+{TargetName}{FRB.IniKeywords.RemapFix.value}$", re.IGNORECASE)
DrawKeys = {"drawindexed", "vb6"}


def isDrawLine(line: str) -> bool:
    """A line of a remapped slot section that DRAWS or serves a draw: the draw itself, the zero stream,
    the texture command list, a custom-shader draw"""
    key, value = keyValue(line)
    return key in DrawKeys or (key == "run" and (value.startswith(f"CommandList{SourceName}") or value.startswith("CustomShader")))


def splitSlotFiles(iniPath: str) -> List[str]:
    """One remapped section per target window per file (see the header, point 11). Returns the extra
    files written."""
    with open(iniPath, "r", encoding = "utf-8", newline = "") as f:
        text = f.read()
    ending = "\r\n" if ("\r\n" in text) else "\n"
    lines = text.split(ending)
    header = next((k for k, line in enumerate(lines) if line.startswith("; ---") and "Remap" in line), None)
    if (header is None):
        return []
    # the fix block's sections: [(name, first line, last line exclusive)]
    sections = []
    k = header
    while (k < len(lines)):
        match = SectionPattern.match(lines[k])
        if (match):
            end = k + 1
            while (end < len(lines) and not SectionPattern.match(lines[end]) and not lines[end].startswith("; ----")):
                end += 1
            sections.append((match.group("name"), k, end))
            k = end
        else:
            k += 1
    windows: Dict[str, List[Tuple[str, int, int]]] = {}
    for name, start, end in sections:
        if (SlotSectionPattern.match(name)):
            first = next((v for key, v in map(keyValue, lines[start:end]) if key == "match_first_index"), None)
            windows.setdefault(first, []).append((name, start, end))
    extras = [(name, start, end) for group in windows.values() for name, start, end in group[1:]]
    if (not extras):
        return []

    def skipOnly(block: List[str]) -> List[str]:
        return [line for line in block if not isDrawLine(line)]

    stem, ext = os.path.splitext(iniPath)
    written = []
    for n, (own, ownStart, ownEnd) in enumerate(extras, 1):
        out = []
        # the mod's own text, its draw sections commented out: only mod.ini serves the mod on Sanhua
        inside = False
        for line in lines[:header]:
            match = SectionPattern.match(line)
            if (match):
                inside = match.group("name").startswith("TextureOverride") and not line.startswith(HideMarker)
            if (not line.strip()):
                inside = False
            out.append((HideMarker + line) if (inside and line.strip() and not line.startswith(HideMarker)) else line)
        # the fix block: this file's own section as is, every other slot section skip-only, the rest verbatim
        k = header
        while (k < len(lines)):
            sec = next(((name, s, e) for name, s, e in sections if s == k), None)
            if (sec is None):
                out.append(lines[k]); k += 1; continue
            name, s, e = sec
            block = lines[s:e]
            if (SlotSectionPattern.match(name) and name != own):
                block = skipOnly(block)
            out.extend(block)
            k = e
        # named as the API names a merge's copies, so the API's own undo removes them too
        extra = f"{stem}{FRB.IniKeywords.RemapFix.value}{n}{ext}"
        with open(extra, "w", encoding = "utf-8", newline = "") as f:
            f.write(ending.join(out))
        written.append(extra)
    # mod.ini keeps the first section of every window and loses the moved ones
    drop = {start for _, start, _ in extras}
    kept = []
    k = 0
    while (k < len(lines)):
        sec = next(((name, s, e) for name, s, e in sections if s == k and s in drop), None)
        if (sec is None):
            kept.append(lines[k]); k += 1
        else:
            k = sec[2]
    with open(iniPath, "w", encoding = "utf-8", newline = "") as f:
        f.write(ending.join(kept))
    return written


def removeSplitFiles(folder: str) -> int:
    """Deletes the extra .ini files a previous split wrote (the API's undo does not know them)"""
    removed = 0
    for root, _, names in os.walk(folder):
        for name in names:
            if (re.search(rf"(?:{TargetName})?{FRB.IniKeywords.RemapFix.value}\d+\.ini$", name, re.IGNORECASE)):
                os.remove(os.path.join(root, name)); removed += 1
    return removed


def runService(folder: str, args) -> None:
    """The whole run through RemapService: folder walk, undo of a previous fix, backups, resources, summary"""
    removed = removeSplitFiles(folder)
    if (removed):
        print(f"  removed {removed} extra .ini file(s) of a previous split")
    service = FRB.RemapService(path = folder, keepBackups = args.keepBackups, hideOrig = args.hideOrig, undoOnly = args.undo,
                               logger = FRB.Logger() if args.verbose else None)
    service.fix()
    stats = service.stats
    if (not args.undo):
        for path in stats.ini.fixed:
            if (args.shapeKeys == "hide"):
                n = hideOriginalSections(path, lambda name: name in ShapeKeySections)
                print(f"  {os.path.relpath(path, folder)}: {n} shape-key sections of the mod's own commented out (the hand remap's configuration; --shapeKeys leave keeps them)")
            if (args.hideTextureOverrides):
                n = hideOriginalSections(path, lambda name: name.startswith("TextureOverrideTexture"))
                print(f"  {os.path.relpath(path, folder)}: {n} [TextureOverrideTexture] sections commented out")
            if (not args.noSplit):
                extras = splitSlotFiles(path)
                if (extras):
                    print(f"  {os.path.relpath(path, folder)}: {len(extras)} further section(s) on an already-taken draw moved into their own file(s): "
                          + ", ".join(os.path.basename(e) for e in extras))
    print(f"\n.ini fixed: {len(stats.ini.fixed)}, skipped: {len(stats.ini.skipped)}")
    for path in sorted(stats.ini.fixed):
        print(f"  {os.path.relpath(path, folder)}")
    for path, error in stats.ini.skipped.items():
        print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
    for label in ("blend", "position", "texcoord", "buf", "texEdit", "texAdd"):
        s = getattr(stats, label)
        if (not s.fixed and not s.skipped):
            continue
        print(f"{label}: fixed {len(s.fixed)}, skipped {len(s.skipped)}")
        for path in sorted(s.fixed):
            print(f"  {os.path.relpath(path, folder)}")
        for path, error in s.skipped.items():
            print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")


def main():
    parser = argparse.ArgumentParser(description = f"{SourceName} -> {TargetName}, in place, through the API's parser, fixer and RemapService")
    parser.add_argument("mod", help = f"the mod folder (every {SourceName} .ini under it is fixed in place; DISABLED ones are skipped)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--hideOrig", action = "store_true", help = f"comment out the mod's own {SourceName} sections, so the mod renders on {TargetName} only")
    parser.add_argument("--undo", action = "store_true", help = "remove a previous fix instead of fixing")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--anchor", choices = ["ribbons", "all"], default = None,
                        help = "pin the chains Exorcist has no counterpart for to their root bone: the two back ribbons and the belt tassel, or ('all') every skirt chain too -- see the header, point 7")
    parser.add_argument("--shapeKeys", choices = ["hide", "leave", "retarget"], default = "hide",
                        help = f"the mod's shape-key sections: 'hide' (default) comments them out like the maintainer's working hand remap, 'leave' keeps them on {SourceName}'s own hashes, 'retarget' copies them onto {TargetName}'s buffer -- see the header, points 8 and 9")
    parser.add_argument("--plan", choices = list(Plans), default = "default",
                        help = "which source component goes through which target slot: 'default' (everything of the body through the torso slot), or 'rnd' (the hand remap's: bodice through slot 4, arm skin not drawn), or 'rndArms' (rnd plus the arm skin through the torso slot)")
    parser.add_argument("--hideTextureOverrides", action = "store_true",
                        help = "comment the mod's own [TextureOverrideTexture] sections out too, as the hand remap does")
    parser.add_argument("--noSplit", action = "store_true",
                        help = "keep every remapped section in mod.ini (default: one section per target draw per file, the rest in <stem>SanhuaExorcistRemapFix<n>.ini -- see the header, point 11)")
    parser.add_argument("--vgRemap", default = None, metavar = "JSON",
                        help = "a {source group: target group} table to write the blend with instead of the library's VGRemaps row (e.g. a draft sheet exported to json)")
    args = parser.parse_args()
    remapOverride = None
    if (args.vgRemap):
        with open(winToPosix(args.vgRemap), "r", encoding = "utf-8") as f:
            remapOverride = {int(k): int(v) for k, v in json.load(f).items()}
    folder = os.path.abspath(winToPosix(args.mod))
    if (not os.path.isdir(folder)):
        raise SystemExit(f"not a folder: {folder}")
    global ServiceRoot
    ServiceRoot = folder

    sourceType, targetType = FRB.WWMIBuilder.sanhua(), FRB.WWMIBuilder.sanhuaExorcist()
    FRB.CppStrategyOverrides.clear()
    retarget = (args.shapeKeys == "retarget")
    FRB.CppStrategyOverrides.setParser(SourceName, makeParser(sourceType, retarget))
    FRB.CppStrategyOverrides.setFixer(SourceName, TargetName, makeFixer(sourceType, targetType, remapOverride, args.anchor, retarget, args.plan))
    try:
        runService(folder, args)
    finally:
        FRB.CppStrategyOverrides.clear()


if (__name__ == "__main__"):
    main()
