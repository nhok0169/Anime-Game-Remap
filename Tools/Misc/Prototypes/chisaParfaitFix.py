#
# ===== chisaParfaitFix =====
#
# PROTOTYPE of the SECOND Wuthering Waves remap: a WWMI mod made for Chisa, fixed IN PLACE so it also
# renders on her Parfait skin. It is sanhuaExorcistFix.py with one pair's worth of facts changed, so
# READ THAT SCRIPT'S HEADER FIRST -- what a WWMI mod is, what the API does with one, how the fix
# block / undo / split-per-draw work, and why textures are bound by register rather than by hash. All
# of that machinery is shared and unchanged. This header records only what is different about Chisa.
#
#   python chisaParfaitFix.py <mod folder> [--keepBackups] [--hideOrig] [--undo] [--verbose]
#                             [--vgRemap <json: {source group: target group}>] [--shapeKeys hide|leave|retarget]
#                             [--hideTextureOverrides] [--noSplit]
#
# ---- What makes this pair different from Sanhua <-> SanhuaExorcist (2026-09-20) ----
#
#  1. THE SLOT PLAN IS ONE TO ONE, not a merge. Measured from the two characters' own geometry rather
#     than guessed: with both meshes in the same rest pose, component N of Chisa and slot N of the
#     skin overlap (bounding-box IoU / centroid distance, componentGeometry.py in the session
#     scratchpad) -- 0, 2 and 6 are the SAME mesh (IoU 1.00: the skin keeps her head, face and eyes),
#     1 is 0.72, 3 is 0.61 and 4 is 0.35. So no source component shares a target slot with another,
#     and there is no invented mask: Sanhua needed one only because her bare arms had no skin family
#     on the Exorcist and went through the torso slot.
#  2. THE SKIN HAS TWO SLOTS NOTHING MAPS ONTO -- slot 5 (5624 vertices, which Chisa's own slot 5 is
#     a 2087-vertex version of) and slot 7 (140 vertices, on her right hip). Both still get a section,
#     as every target slot does, so the skin's own geometry is skipped rather than drawn under the mod.
#  3. THE TWO BIG CLOTHING SLOTS READ THEIR DIFFUSE FROM A DIFFERENT REGISTER on the skin than on
#     Chisa: her upper- and lower-body passes bind normal / mask / diffuse at ps-t0 / t1 / t2, and the
#     skin's bind them at ps-t0 / t1 / **t3** (ps-t2 there is a small texture the dump's size filter
#     drops -- not the diffuse). Read off the two frame dumps' slot tables with wwmiDrawTable.py and
#     cross-checked by FORMAT: BC7_UNORM is the normal map, DXT1 / DXT5 the material mask, BC7_SRGB
#     the diffuse. Binding the diffuse at the source's register would have left it unread.
#  4. THREE ROLES NEED NO FALLBACK because both skins bind the SAME texture hash for them (the front
#     hair's mask / diffuse / normal, d3b9ba76 / f2646d21 / 9ccd7ea7, and the eyes' 226b31fc and
#     8224e584). A mod that repaints those is served by its own [TextureOverrideTexture] on the
#     target as well, since the hash it matches is the skin's too.
#  5. THE VERSIONS DIFFER PER CHARACTER: Chisa is filed at 2.8 and the Parfait skin at 3.5 (the
#     maintainer's choice, being the versions each was introduced), where Sanhua and her skin share
#     2.5. The library lookups take the latest row per character, so only the classifier's default
#     version and the download folder need the source's.
#  6. NO ANCHOR CHAINS ARE CONFIGURED. Sanhua's two back ribbons needed pinning because the Exorcist
#     has no counterpart bone for them; nothing here has been shown to need it yet, and the hook is
#     left in place (AnchorChains) for the first in-game round to fill.
#
# ---- What is NOT established yet (2026-09-20) ----
#
#  * THE VERTEX GROUP REMAP IS A PROPOSAL. Data/RemapDrafts/ChisaRemapDraft.xlsx is entirely
#    Tools/VGRemapFinder's output -- no hand-made sheet, no in-game round -- so a warped limb is at
#    least as likely to be a wrong row there as a bug here; the draft's Uncertainty column is the
#    review order (76 rows at 0.9 or above in the forward direction).
#  * SLOT 5's BINDINGS ARE A GUESS. The skin's slot 5 binds no texture of its own in the dump (only
#    globals), so the two textures Chisa's slot 5 uses are bound on the one pass the two share
#    (3df800c3); if that part renders wrong, its registers are the first thing to re-read.
#  * THE SKIN'S MATERIAL MASK LEGEND IS UNMEASURED. Nothing here invents a mask, so it does not
#    matter yet -- but it will the moment a source component has to borrow another slot's family.
#  * NOT SEEN IN GAME AT ALL. The first test is the identity mod (WWMI/ChisaIdentity), which is the
#    only input whose every slot, bone and texture is the real character's.
#

import argparse
import json
import os
import re
import shutil
import struct
import sys
from typing import Dict, List, Optional, Tuple

import numpy as np       # the blend remap and the material-mask repack are both array work

OnWindows = (sys.platform == "win32")


def winToPosix(path: str) -> str:
    if (OnWindows or (len(path) < 2) or (path[1] != ":") or (not path[0].isalpha())):
        return path
    return "/mnt/" + path[0].lower() + path[2:].replace("\\", "/")


MainRepo = winToPosix(r"C:\Users\AlexX\Documents\Games\Mods\Repos\Anime-Game-Remap")


def apiSrcOf(repo: str) -> str:
    return os.path.join(repo, "Anime Game Remap (for all users)", "api", "src", "py")


def knowsThePair(repo: str) -> bool:
    """Whether the API built in 'repo' has Chisa registered -- asked of the BUILT module rather than
    of the source, since that is what this script imports. A build that predates the pair raises
    `WWMIBuilder has no attribute 'chisa'` halfway through a run otherwise, which reads like a
    missing binding rather than the wrong checkout."""
    package = os.path.join(apiSrcOf(repo), "FixRaidenBoss2")
    try:
        for fileName in os.listdir(package):
            if (fileName.startswith("core.") and fileName.endswith(".pyd")):
                with open(os.path.join(package, fileName), "rb") as f:
                    return b"chisaParfait" in f.read()
    except OSError:
        return False
    return False


# AG_REMAP_REPO wins; otherwise the main checkout, and then any worktree of it -- the pair may only
#   exist on a branch, and the copy of this script that runs beside the mods has no repo of its own
#   to infer one from.
_candidates = [os.environ["AG_REMAP_REPO"]] if (os.environ.get("AG_REMAP_REPO")) else []
if (not _candidates):
    _candidates.append(MainRepo)
    worktrees = os.path.join(MainRepo, ".claude", "worktrees")
    if (os.path.isdir(worktrees)):
        _candidates += [os.path.join(worktrees, name) for name in sorted(os.listdir(worktrees))]

Repo = next((candidate for candidate in _candidates if knowsThePair(candidate)), _candidates[0])
APISrc = apiSrcOf(Repo)
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path on this OS")
if (not knowsThePair(Repo)):
    raise SystemExit(f"the API built at {Repo} does not know {'Chisa'}: build a checkout that has her registered, "
                     f"or point AG_REMAP_REPO at one (tried: {', '.join(_candidates)})")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

SourceName, TargetName = "Chisa", "ChisaParfait"
SourceVersion, TargetVersion = "2.8", "3.5"   # the versions the library files them under -- unlike Sanhua's pair, they differ
SlotPrefix = "component"          # the 'type' the library files a WWMI draw slot under: component0, component1, ...
ShapeKeyType = "shapekeys"        # the 'type' of the ShapeKeyChecksums row

SkinMaskColour = (255, 77, 0, 255)          # kept from the Sanhua config and UNMEASURED for this skin: nothing invents a mask here (header, point 1)
SkinMask = "SkinMask"                       # the invented mask's resource / file stem, if a plan ever needs one

# THE TWO SKINS PACK THEIR MATERIAL MASK DIFFERENTLY, AND BINDING THE MOD'S OWN IS WRONG (header,
#   point 10). Chisa's clothing shader takes a BC1 mask of flat material codes and marks bare skin
#   with R = 255; the skin's takes a BC3 one and marks bare skin with R = 0, cloth with R = 255 --
#   the INVERSE. Asked of the diffuse under each region rather than of any name (legend.py in the
#   session scratchpad): Chisa's R >= 128 is 99.2% flesh-coloured on the upper body and 79.7% on
#   the lower (her bare legs, 41% of that atlas), and her R < 128 is 0.4%; the skin's R < 128 is
#   the more flesh-like of her two by 88% to 52%. So her own mask tells the skin's shader that 97%
#   of her jacket, blouse and skirt is bare flesh, and the whole body renders under a translucent
#   subsurface red -- the Sanhua "cloth shaded as skin" symptom, from the opposite direction.
# --probe: the registers a planned slot does NOT bind, each given an unmistakable flat colour, so
#   ONE in-game round says which one paints a surface (Overview's "WHEN YOU CANNOT TELL WHAT A DRAW
#   IS USING, REPLACE THE TEXTURE WITH SOMETHING UNMISTAKABLE", by register rather than by slot).
#   The two skins' clothing passes are different pixel shaders -- the skin's takes an extra R8 map at
#   ps-t2 and a different set of auxiliary maps after it, one of which (742c5c7b) is bright RED --
#   and which of them a surface reads cannot be told from the dump. A probe build also identifies
#   ITSELF: if the model is not gaudy, the game is not running the build you just made.
# THE SKIN'S CLOTHING PASS TAKES A MATERIAL-DETAIL MAP AT ps-t2 THAT CHISA'S SHADER HAS NO INPUT
#   FOR, AND IT IS WHAT PAINTED THE RED (bisected in game, 2026-09-20). Her pass binds normal / mask
#   / diffuse at ps-t0 / t1 / t2; the skin's inserts this R8_UNORM map at ps-t2 and shifts the rest
#   down one, which is the same shift that moves the diffuse to ps-t3. The mod ships nothing for it,
#   so the skin's own stayed bound -- and it is authored for HER atlas, so at the mod's UVs its codes
#   land on the wrong surfaces.
#
#   It reads like a near-black texture and is not one: measured over her two, the values are 2, 4 and
#   82 out of 255 with a median of 4, which is a small-integer CODE per pixel rather than a
#   brightness. That is why 4-where-0-belongs is a different material rather than a slightly darker
#   one, and why flattening the register -- to `0`, the neutral code, which is what the flat green of
#   the bisect supplied in `.r` -- is the fix rather than a workaround.
#
#   Bisected rather than reasoned out: the full probe was clean, ps-t4 alone (the bright red 742c5c7b,
#   the obvious suspect) was not, {ps-t2, ps-t5, ps-t6} was, and ps-t2 alone was. Four rounds.
# --paint: every slot's DIFFUSE replaced by a flat colour, one per source component, on every pass
#   the fix covers. It answers the question no amount of reading the tables can -- WHICH slot draws
#   the part I am looking at, and does the fix control it at all. A part that takes its slot's
#   colour is drawn by us and the problem is which texture we chose; a part that keeps its original
#   look is NOT drawn through any section we write, and no texture table will ever fix it.
PaintColours = {0: (255, 0, 0, 255), 1: (0, 255, 0, 255), 2: (0, 0, 255, 255), 3: (255, 255, 0, 255),
                4: (255, 0, 255, 255), 5: (0, 255, 255, 255), 6: (255, 128, 0, 255)}
PaintNames = {0: "red", 1: "green", 2: "blue", 3: "yellow", 4: "magenta", 5: "cyan", 6: "orange"}

# --passPaint <slot>: one flat colour per PASS of that slot, on every register. A slot is drawn
#   several times per frame by different shaders, and only the pass that actually paints a surface
#   matters -- mirroring the source's bindings on the WRONG pass looks exactly like mirroring them
#   badly. Whichever colour the surface takes names the pass, and then the source's own bindings for
#   THAT pass are the ones to copy.
#   Every pass the skin draws the slot on is painted (TargetSlotPasses) -- until 2026-09-22 only
#   SlotPasses' were, so a surface drawn by `21176cf6` or by a pass the fix rebinds nothing on could
#   never take a colour -- and no pass is red, since the symptom this tool was reached for last was.
PassPaintColours = [(0, 255, 0, 255), (0, 0, 255, 255), (255, 255, 0, 255), (255, 0, 255, 255), (0, 255, 255, 255)]
PassPaintNames = ["green", "blue", "yellow", "magenta", "cyan"]

# ps-t4 ON THE UPPER BODY IS THE SKIN'S OWN BLUSH/TINT OVERLAY, AND IT IS BRIGHT RED (2026-09-20).
#   742c5c7b means (249.5, 11.8, 3.8) at alpha 15.9 -- a strong red laid on weakly, which is the
#   literal description of the reported stain. The target's upper pass reads it; Chisa's upper pass
#   has no counterpart (hers reads five registers, the skin's eleven), so it is the skin's own map
#   sampled at the mod's UVs: the same bug class as ps-t2, and the same answer.
#
#   It was NOT tested for this symptom, and the reason is worth recording: an older bisect against a
#   DIFFERENT red -- the one ps-t2 turned out to own -- had cleared it, and the note saying so
#   ("ps-t4 alone, the bright red 742c5c7b, the obvious suspect, was not") was read four builds later
#   as though it applied here too. A register cleared for one symptom is not cleared for the next.
# AND ps-t10 IS WHAT THE KIMONO'S RED STAIN WAS (2026-09-20), found by the maintainer nulling it in
#   the generated command list -- the same one-line test that found the hair's ps-t5. It is the
#   512 x 25 SUBSURFACE ramp. The skin's own there (6a9ec87e) was red, and swapping in Chisa's
#   (06790f7e) left it red, because the fault is not WHICH ramp: it is that the target's upper-body
#   shader applies subsurface scattering to that region at all, and warm subsurface light on a
#   decollete reads as a stain. Chisa's upper pass has no such input.
#   Bound to a flat zero rather than left out: an unset register keeps the TARGET's own ramp, which
#   is the red one. A zero texture samples exactly as `null` does.
NeutralBindings = {3: {"ps-t2": (0, 0, 0, 255), "ps-t4": (0, 0, 0, 0), "ps-t10": (0, 0, 0, 0)},
                   4: {"ps-t2": (0, 0, 0, 255)}}
NeutralName = "DetailZero"      # the resource / file stem of the flat map bound there
FlatName = "Flat"
# THE TWO SKINS ARE **NOT** INVERTED, AND BELIEVING THEY WERE IS WHAT THE RED STAIN WAS
#   (2026-09-20). This file used to say Chisa marks bare skin with R = 255 and the Parfait skin
#   with R = 0 -- "the INVERSE" -- on the strength of a percentage-of-flesh-like-pixels test over
#   the two atlases. Both skins mark bare skin with **R = 255**. The test was defeated by her atlas
#   being pale cream throughout, so "flesh-like" fired on her clothes as readily as on her skin and
#   the two figures (88% against 52%) were noise dressed as a measurement.
#
#   LOOKING at the two masks settles it in one glance, which is why Creating Remaps says to crop the
#   island and look: Chisa's R channel is black with ONE white patch, and that patch is exactly where
#   her diffuse shows flesh; the skin's R channel is white almost everywhere, and her diffuse there
#   is a bare torso. The only black patches in hers are two small garment pieces.
#
#   So the repack was mapping Chisa's skin to the target's CLOTH code and her kimono to the target's
#   SKIN code -- the garment shaded with subsurface scattering, which is the translucent red over the
#   clothes this guide's own triage table already describes, arrived at from the opposite direction.
TargetMaskSkin = (255, 0, 126, 0)               # measured: 84.9% of her upper atlas, under a bare torso
TargetMaskCloth = (0, 0, 126, 0)                # measured: her two small garment pieces, 5.5%
# THE MASK'S GREEN CHANNEL IS HOW SHINY THE SURFACE IS, AND A FLAT CLOTH CODE SAYS "NOT AT ALL"
#   (2026-09-20). The skin's mask is not a small set of material ids -- R and G both vary
#   continuously -- and plotting G over each atlas says what it is: zero across flat fabric and
#   high along every lace edge, ribbon trim and pleat highlight. Her body atlases are 3.2% and 6.1%
#   above G 64; her slot 5, the frilled and beribboned one, is 29.4%.
#
#   So a ribbon handed TargetMaskCloth is told it is the mattest cloth on the model, which is the
#   last of the three reasons it rendered flat (the first two were the normal map on the matcap
#   slot and the diffuse on the detail slot). These are the medians of the skin's OWN shiny
#   accessory trim -- the pixels of her slot 5 mask with G > 64, 1.2M of them -- rather than a
#   number chosen to look right: R 222, G 90, B 126.
TargetMaskAccessory = (222, 90, 126, 0)               # the stem of a flat map an ExtraPassRegs entry asks for by COLOUR

# ps-t9 AND ps-t10 WERE UNREACHABLE UNTIL 2026-09-20, AND THE UPPER BODY'S PASS HAS BOTH. The list
#   stopped at ps-t8 because the pass that was being bisected then had eight registers; the skin's
#   upper-body pass (3311e8a5) sets ELEVEN. So two of its inputs had never been probed, while the
#   bisect they were left out of was read as having cleared the set.
ProbeColours = [("ps-t2", (0, 255, 0, 255), "green"), ("ps-t3", (255, 255, 255, 255), "white"),
                ("ps-t4", (0, 0, 255, 255), "blue"), ("ps-t5", (255, 255, 0, 255), "yellow"),
                ("ps-t6", (0, 255, 255, 255), "cyan"), ("ps-t7", (255, 0, 255, 255), "magenta"),
                ("ps-t8", (255, 128, 0, 255), "orange"), ("ps-t9", (255, 255, 255, 255), "white"),
                ("ps-t10", (128, 0, 255, 255), "violet")]
ProbeSlots = {3, 4}                             # the source components a --probe run paints; the rest are left readable

# A --probe run takes an optional SPEC -- `--probe 3:ps-t4,4:ps-t5` -- naming the registers to
#   flatten per source component, which is how the culprit is bisected once a full probe has shown
#   that SOME register of the set paints a surface. The two clothing slots bind DIFFERENT textures
#   at the same register (slot 3's ps-t4 is the red 742c5c7b, which slot 4 does not bind at all), so
#   the answer is per slot and the bisect tests one suspect on each in the same round.
def parseProbeSpec(spec: Optional[str]) -> Optional[Dict[int, List[str]]]:
    if (not spec):
        return None
    out: Dict[int, List[str]] = {}
    for entry in spec.split(","):
        slot, _, reg = entry.strip().partition(":")
        if (not reg):
            raise SystemExit(f"--probe takes `<component>:<register>` entries, not {entry!r}")
        out.setdefault(int(slot), []).append(reg.strip())
    return out

MaskTranslations = {"upperMask", "lowerMask"}   # the roles whose file is repacked before it is bound

# THE TWO SKINS' SHEEN TEXTURES ARE DIFFERENT KINDS OF DATA, SO THE SHEEN IS TRANSLATED TOO (2026-09-21).
#   Chisa's bb73967a (her upper pass's ps-t3) is a matcap whose four channels are four GRAYSCALE sheen
#   profiles: her shader 42721e1d samples it by view-space normal and blends R -> G -> B -> A by the
#   normal map's alpha x 3, ending in ONE scalar. It only looks pink and purple viewed as RGB. The
#   skin's own ps-t8 (4bee4070) is a HOLOGRAPHIC FOIL: rainbow RGB with a matcap ring in alpha -- her
#   outfit's pearl. Bound raw, Chisa's packed channels were read as foil COLOUR: a white shirt mod came
#   out pink-lavender pearly, and the Hanabi kimono carried a slight tint the maintainer had put down
#   to the original. Translated: alpha (the intensity) is Chisa's first profile -- the one a low
#   normal-map alpha selects -- and the RGB is that same profile as neutral gray, so no rainbow.
SheenTranslations = {"bodySheen"}

# A SHADER FAMILY IS A COLOUR GRADE, AND A TEXTURE IS THE ONLY PLACE TO PUT IT BACK (2026-09-20).
#   Chisa's ribbon is painted by a HAIR shader (3df800c3, the only pass her component 5 is drawn on)
#   and the skin has nowhere to draw it but a CLOTH shader, so the two treat the same diffuse
#   differently, and no binding can change that. Measured, with both sides read against the SAME
#   texture -- her accessory diffuse sampled at her component 5's own vertices, median (148, 65, 68)
#   -- and each screenshot's ribbon normalised against the hair and skin in the same shot:
#
#       her own shader renders it   (132, 45, 45)   = the texture x (0.89, 0.69, 0.66)
#       the remap's renders it      (171, 73, 78)   = the texture x (1.16, 1.12, 1.15)
#
#   So the cloth shader is a near-flat brightness gain -- no grade, no additive lift -- where hers
#   darkens and deepens. That is the whole of the "less metallic, dark red" report, and it had not
#   moved across four builds while the three register fixes before it moved contrast.
#
#   The gain below is the ratio of those two, so the graded texture through the cloth shader lands
#   on (132, 45, 45) exactly. The check that it is not merely a curve fit: the graded texture's
#   SATURATION comes out 0.657 against the base render's 0.659, and saturation was not fitted.
#
#   This is a compensation, not a fix -- the structural answer is to route the ribbon through one of
#   the target's HAIR slots so it is drawn by the shader it was authored for, which merges it into a
#   hair draw. Precedent for grading instead: the GI side has a shared DarkDiffuse for Ningguang.
ColourGrades = {"accessoryDiffuse": (0.772, 0.616, 0.577)}
TargetMaskSkinR = 255                           # what she marks bare skin with, in R -- the SAME as Chisa
# CHISA'S R IS FIVE BANDS, AND ONLY THE TOP ONE IS SKIN (2026-09-21). Read off her upper-body pixel
#   shader 42721e1d0c282918 (a hunting-mode dump): the mask's R is compared against 0.05 / 0.3 /
#   0.5 / 0.9, R >= 0.9 selects the skin tint, and the 0.5-0.9 band joins skin only in switching the
#   SPECULAR highlight off (`r3.w = 1 - max(band, skin)` multiplies the highlight term). So that band
#   is matte cloth, not skin. The threshold used to be 128, which put it with skin: the shirt mod
#   paints 60% of its shirt R = 183-186, and the repack handed all of it the target's SKIN code,
#   which her shader draws with a pinkish pearly sheen.
#
#   BUT THE SAME AUTHOR PAINTS THE BARE CHEST 183 TOO (92% of the skin-coloured texels), so the
#   band alone cannot say which is which, and moving the threshold to 0.9 alone shades her chest as
#   cloth. The diffuse under the texel can: warm skin on the chest, neutral white on the shirt.
#   So a code in the matte band is skin only where the diffuse is flesh-coloured
#   (SourceMaskFleshBand), and R >= 0.9 is skin unconditionally, as her shader has it.
SourceMaskSkinAbove = 230                       # Chisa's R at or above this (0.9) is bare skin, always
SourceMaskFleshBand = 128                       # and at or above this, where the diffuse under it is flesh-coloured
ShapeKeyZero = "ShapeKeyZero"               # the zero shape-key offset stream bound at vb6 (sanhuaExorcistFix.py's header, point 10)
ShapeKeyStride = 24                         # bytes a vertex in that stream, off the frame dump's vb6 layout

# The target's pixel shaders that bind CHARACTER textures, per draw slot, off
#   FrameAnalysis-ChisaParfait-2026-09-20-020434 (wwmiDrawTable.py). The outline / shadow passes
#   (21176cf6, 32414b55, 259b766b) bind only globals and are left to the game, as are slot 0's
#   94d9d5e9 and slot 6's 92ca4bd9. Slot 7 is absent: nothing is drawn through it.
SlotPasses = {0: ["c0ad88a930c4d853", "71f60c461ae3f166"], 1: ["71f60c461ae3f166"], 2: ["2060326dcea397fb"],
              3: ["3311e8a58d8c5d20"], 4: ["a99f09b6f36e94af"],
              # slot 5 draws on THREE art passes, not one: 87825a9a and ced9a47f bind her own
              #   accessory diffuse at ps-t0 just as 3df800c3 does, and a pass the config does not
              #   name renders the mod's geometry with the GAME's textures. Chisa's accessory is her
              #   hair ribbon and its tails (component 5 spans z 72..144), so the two missing passes
              #   drew the ribbon in ChisaParfait's pink instead of Chisa's red -- reported in game
              #   2026-09-20. passCoverage.py lists every pass per slot against this table.
              # only the pass CHISA also draws her accessory on keeps the plan's map, which mirrors her
              #   bindings exactly. The skin's other two accessory passes are shaders Chisa never
              #   draws at all, so there is nothing of hers to mirror and they get their own map
              #   below, from what the probe measured rather than from what she binds.
              5: ["3df800c350681ec9"], 6: ["da00ec8f7c73d5e3"]}
# A slot's OTHER passes bind the same art at DIFFERENT registers, so they need their own map.
#   `21176cf6` and `32414b55` take each slot's diffuse at ps-t0 where the main pass takes it at
#   ps-t1 (hair) or ps-t3 (clothing), and both take a per-character pair at ps-t1 / ps-t5. Generated
#   rather than judged (genPassRegs.py in the session scratchpad): for every pass the TARGET draws a
#   slot on, whatever the SOURCE binds on that same pass for that component, wherever the two
#   differ and the source's texture has a role. A pass left out renders the mod's geometry with the
#   GAME's textures -- run `Tools/Misc/Diagnostics/wwmiPassCoverage.py` after changing either table.
ExtraPassRegs = {
    0: {"21176cf68a65ab7a": {"ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"},
        "32414b557630d98d": {"ps-t0": "hairDiffuse", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"}},
    1: {"21176cf68a65ab7a": {"ps-t0": "hairDiffuse", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"},
        "32414b557630d98d": {"ps-t0": "hairDiffuse", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"}},
    2: {"259b766b59f72419": {"ps-t0": "upperDiffuse", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"}},
    3: {"21176cf68a65ab7a": {"ps-t0": "upperDiffuse", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"}},
    4: {"21176cf68a65ab7a": {"ps-t0": "lowerDiffuse", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"}},
    # THE RIBBON'S PASS IS A CLOTHING SHADER WITH THE NORMAL AND THE DETAIL MAP SWAPPED (2026-09-20).
    #   87825a9a is the pass that paints it (a colour per PASS came back green, which was its), and
    #   it is the ONLY draw of the target's slot 5 that sets the whole register set itself -- the
    #   other three set ps-t0 alone and inherit the rest, which is what made a carried-forward slot
    #   table read as four different layouts. What each register HOLDS was then read off the pixels
    #   of the textures the game binds there, against the two body clothing passes:
    #
    #        pass                    ps-t0            ps-t1      ps-t2            ps-t3     ps-t5
    #        a99f09b6 (lower body)   NORMAL RG,B=0    mask       detail ~black    diffuse   matcap
    #        3311e8a5 (upper body)   NORMAL RG,B=0    mask       detail ~black    diffuse   matcap
    #        87825a9a (slot 5)       detail ~black    mask       NORMAL RG,B=0    diffuse   matcap
    #
    #   So this pass is the clothing layout with ps-t0 and ps-t2 EXCHANGED, and the earlier map had
    #   the diffuse on the detail slot, the front hair's diffuse on the mask slot, and the ribbon's
    #   own normal on the MATCAP slot -- which is why the ribbon came out flat and un-metallic while
    #   its colour was right (reported in game, 2026-09-20). Only ps-t3 was ever correct.
    #
    #   Chisa has no accessory mask and no accessory detail map -- her ribbon is drawn on a HAIR
    #   shader (3df800c3) that binds one texture, her diffuse at ps-t0, and inherits the front
    #   hair's maps for the rest -- so those two registers take a flat created map rather than the
    #   skin's own sampled at the mod's UVs, which is the material-mask bug of the round before.
    #   The mask is TargetMaskAccessory, not TargetMaskCloth: see its comment for why a ribbon told
    #   it is ordinary cloth cannot have a highlight.
    5: {"87825a9a29529f9b": {"ps-t0": (0, 0, 0, 255), "ps-t1": TargetMaskAccessory,
                             "ps-t2": "accessoryNormal", "ps-t3": "accessoryDiffuse"},
        "ced9a47fb6ad4d16": {"ps-t0": (0, 0, 0, 255), "ps-t1": TargetMaskAccessory,
                             "ps-t2": "accessoryNormal", "ps-t3": "accessoryDiffuse"}},
}

# A CHARACTER IS NOT ONLY HER vb0 MESH. Chisa and ChisaParfait both draw a SECOND mesh, vb0
#   b00403dc -- the same hash on both, so the same geometry, 2 components and 65973 indices -- and
#   the game textures it PER CHARACTER: on Chisa's draws `ps-t0 = cbab5910`, her hair diffuse, and
#   on the skin's her own `2c990f51`. It is her hair ribbon. Nothing the fix writes touches it: the
#   remapped sections match the vb0 hash of the MAIN mesh, and a mod's `[TextureOverrideTexture]`
#   overrides by the hash the GAME binds, which on the skin is never Chisa's. So the ribbon kept the
#   skin's pink pattern through three rounds of texture work, and the --paint diagnostic is what
#   showed it: no slot's colour reached it, because no section of ours draws it.
#
#   The geometry is shared, so there is nothing to remap -- only the textures to rebind, on the
#   passes where the two characters differ. Generated the same way as ExtraPassRegs.
#
#   CAVEAT: the section matches the hash with no index window, so it applies wherever that mesh is
#   drawn. It is assumed to be this character's own accessory, shared between her skins; if another
#   character turns out to draw it too, this repaints theirs as well.
SharedMeshes = {
    "b00403dc": {
        "ca134b7ad59cdf8c": {"ps-t0": "hairDiffuse", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"},
        "a7bdec26cf254853": {"ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"},
        "21a483170781cfeb": {"ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"},
    },
}

FilterBase = 3381.71   # 3dmigoto keys [ShaderOverride] by shader hash GLOBALLY, so these sit clear of the Sanhua fix's 3381.91 and the reverse one's 3381.81
FilterStep = 0.001     # ~25 vertex shaders are tagged now, and a step of 0.01 would run them past 3381.81

# THE FIX'S OWN PASS FILTERS WERE SWITCHING RABBITFX OFF (2026-09-21). RabbitFX patches a pixel shader
#   through its [ShaderRegexMain] and marks it `filter_index = 1718.1`; its `SetTextures` then acts
#   only `if ps == 1718.1`. The fix tagged the very same pixel shaders with 3381.7x so its texture
#   lists could tell the passes apart, and a shader carries ONE filter_index -- so in a dump taken with
#   the remap installed, every remapped `SetTextures` read `if ps == 1718.1: false`, on all seven of
#   the skin's passes RabbitFX patches (upper, lower, hair, bangs, face, both side-panel passes). And
#   because a [ShaderOverride] is keyed by shader hash globally, it did that for ANY mod drawing with
#   those shaders, not only this one.
#
#   RabbitFX patches PIXEL shaders only, so the fix tags the VERTEX shader instead and asks `vs == ...`.
#
# AND "THE PIXEL SHADERS RABBITFX LEAVES ALONE" WAS A LIST NOBODY COULD READ (2026-09-22). The table
#   above used to move only the passes a dump showed RabbitFX patching, and keep `ps ==` for the rest
#   -- `21176cf6` and `32414b55` (every slot's second and third pass), `259b766b`, `3df800c3`,
#   `da00ec8f` and the shared mesh's three. But a dump shows a patched shader only through
#   `[ShaderRegex_RabbitFX_Main]`'s `dump = desc` lines, and RabbitFX has SIX more regexes (Eye,
#   Transparent, SingleOutput, Outline, TexturelessOutline, ...) whose dump lines are commented out,
#   every one of which marks its shader `1718.1` and four of which carry the same FX-map discard. So
#   a pass could be RabbitFX's with no trace in any dump. One was: Chisa6's backless sweater cuts its
#   see-through panels out with an FX map, and on the skin those panels rendered RED -- the mod paints
#   them with the skin mask code (R = 255) over a near-black diffuse, which is harmless while every
#   pass discards them and is dark skin lit red when one does not. The fix's `ps` tag on `21176cf6`,
#   which draws the upper body too, had switched that pass's discard off.
#   So NO pixel shader is tagged any more: every pass is gated through the vertex shaders it is drawn
#   with. A pass can have SEVERAL -- the face pass runs on `683e019f` in some dumps and `c277738c` in
#   others, and `32414b55` on a different one per component -- so the gate is an OR over all of them.
#   No component draws two differently-bound passes on one vertex shader: `d83a5477` carries the bangs
#   and hair passes (component 0 binds one list for both, component 1 is only drawn on the hair one),
#   and `6594231b` the two side-panel passes, whose lists are identical.
#   Read off every FrameAnalysis-ChisaParfait* dump's draw table (wwmiDrawTable.py), mesh e611d493
#   and the shared mesh b00403dc.
PassVertexShaders: Dict[str, List[str]] = {
    "c0ad88a930c4d853": ["d83a54772fc666f9"], "71f60c461ae3f166": ["d83a54772fc666f9"],   # bangs, hair
    "2060326dcea397fb": ["683e019f389b2624", "c277738ca4039045"],                         # face
    "3311e8a58d8c5d20": ["d24888b5b268a084"],                                             # upper body
    "a99f09b6f36e94af": ["22195a190e37d3cf"],                                             # lower body
    "3df800c350681ec9": ["bbabe18b97a63509"],                                             # accessory
    "87825a9a29529f9b": ["6594231b96dfca5f"], "ced9a47fb6ad4d16": ["6594231b96dfca5f"],   # the side panels
    "da00ec8f7c73d5e3": ["72f45530b1e1f75a", "a6e9eb6303b1b631"],                         # slot 6
    "259b766b59f72419": ["fd12d3374ac7a7dd", "1479e3f5a626af60"],                         # the face's second pass
    "21176cf68a65ab7a": ["0ccd030bff8b515c", "5d60ebdc89fe3833"],                         # every slot's second pass
    "32414b557630d98d": ["ba4eee7b53cf726e", "60b893ec7f585976", "f906b8aa4c220a6f",      # every slot's third pass,
                         "3edca9a0f68c8b15", "59585b690c6e1f01", "4cf784b1b2c7ca1c"],     #   one vs per component
    "ca134b7ad59cdf8c": ["0b22e4a80375c4d0", "a5cd08444f0fca2e"],                         # the shared mesh b00403dc
    "a7bdec26cf254853": ["ee6166816ce9f788"],
    "21a483170781cfeb": ["da98d2d08d937357"],
    "94d9d5e981938d52": ["5102d7edd774359e"],                                             # passes the fix binds nothing
    "320a753b019eff67": ["ac592389c85c3e38", "aef4fc536fbff1e7"],                         #   on, gated only so that
    "92ca4bd985fe6887": ["676fdbd61b302294", "89577c176b52b351"],                         #   --passPaint can reach them
}

# EVERY pass the skin draws each slot on, from the same draw tables -- a superset of SlotPasses and
#   ExtraPassRegs, which name only the passes the fix REBINDS. A pass left off both still draws the
#   mod's geometry, with the GAME's textures, and --passPaint is how one of those is caught painting a
#   surface: it paints every pass here.
TargetSlotPasses: Dict[int, List[str]] = {
    0: ["c0ad88a930c4d853", "71f60c461ae3f166", "94d9d5e981938d52", "21176cf68a65ab7a", "32414b557630d98d"],
    1: ["71f60c461ae3f166", "21176cf68a65ab7a", "32414b557630d98d"],
    2: ["2060326dcea397fb", "320a753b019eff67", "259b766b59f72419", "32414b557630d98d"],
    3: ["3311e8a58d8c5d20", "21176cf68a65ab7a", "32414b557630d98d"],
    4: ["a99f09b6f36e94af", "21176cf68a65ab7a", "32414b557630d98d"],
    5: ["3df800c350681ec9", "87825a9a29529f9b", "ced9a47fb6ad4d16", "32414b557630d98d"],
    6: ["da00ec8f7c73d5e3", "92ca4bd985fe6887"],
}
_AllPasses = list(dict.fromkeys([ps for passes in SlotPasses.values() for ps in passes]
                                + [ps for byPass in ExtraPassRegs.values() for ps in byPass]
                                + [ps for byPass in SharedMeshes.values() for ps in byPass]
                                + [ps for passes in TargetSlotPasses.values() for ps in passes]))
_untagged = [ps for ps in _AllPasses if (ps not in PassVertexShaders)]
assert (not _untagged), f"passes with no vertex shader to gate them by (read one off a dump's draw table): {_untagged}"
ShaderFilters = {vs: f"{FilterBase + FilterStep * i:.4f}".rstrip("0")          # the vertex shader tagged -> its filter
                 for i, vs in enumerate(dict.fromkeys(vs for ps in _AllPasses for vs in PassVertexShaders[ps]))}


def passTest(ps: str) -> str:
    """The .ini condition that is true on a draw of pixel shader 'ps' -- through the vertex shaders it
    is drawn with, never the pixel shader itself (see PassVertexShaders)"""
    return " || ".join(f"vs == {ShaderFilters[vs]}" for vs in PassVertexShaders[ps])

# Chisa's textures by the hash the game binds them under, with the role each plays in its component's
#   MAIN pass -- read off FrameAnalysis-Chisa-2026-09-20-013225's slot table, and the kind of each
#   confirmed by its DXGI format (BC7_UNORM = normal, DXT1 / DXT5 = material mask, BC7_SRGB = diffuse).
#   These are the hashes of a dump taken at LOD bias Ultra High; a mod exported at the default bias
#   carries the half-size variants instead, which is what the pixel-identity path in TextureIndex is
#   for (no hash lineage exists for Chisa yet -- the first mod that needs one is how it gets written).
Roles = {
    # slot 0, the front hair: the skin binds the SAME three, so a mod repainting them is served by its
    #   own [TextureOverrideTexture] on the target too
    "d3b9ba76": "frontHairMask", "f2646d21": "frontHairDiffuse", "9ccd7ea7": "frontHairNormal",
    "a842d51f": "hairMask", "cbab5910": "hairDiffuse", "e921181d": "hairNormal",
    # THE HAIR SHADING RAMP, AND THE TWO SKINS PUT OPPOSITE COLOURS IN IT (2026-09-20). Chisa's
    #   `ps-t2` on her hair pass is a 256 x 256 ramp averaging (127, 149, 252) -- blue -- and
    #   ChisaParfait's is a 512 x 512 one averaging (206, 177, 2) -- orange. Binding the mod's other
    #   hair textures and leaving this register alone left the TARGET's orange ramp on the mod's
    #   hair, which is what "the hair has orange stains" was: a warm tint on whatever catches the
    #   light. The FRONT hair slot needs no such entry, because both skins bind b0ee686b there.
    "232c2dbc": "hairRamp",
    # AND THE CLOTHING PASS HAS ONE TOO, WHICH IS WHAT SHADES BARE SKIN (2026-09-20). The bust
    #   report turned out to be the DECOLLETAGE -- the chest above the kimono, which the mask marks
    #   as skin -- and a flat colour at the target's upper-body `ps-t5` painted exactly that region.
    #   IT IS A 512 x 25 SUBSURFACE LOOKUP, AND THE SKIN'S IS THE REDDER OF THE TWO: Chisa's
    #   06790f7e means (143.1, 106.7, 141.2), a cool purple-pink, and the skin's 6a9ec87e means
    #   (155.5, 121.8, 121.6). Her decollete rendering through the skin's ramp is the reported stain.
    # THE FIRST ATTEMPT AT THIS BOUND 4848ae14 AT ps-t5 AND TURNED THE WHOLE KIMONO YELLOW, because
    #   4848ae14 IS NOT CHISA'S. It was read off her upper-body draw's INHERITED ps-t5 -- and
    #   whoBinds.py, asked which draws SET it, answers "another draw" in BOTH dumps: it belongs to a
    #   third character standing in both scenes (start 0, count 47037, ps 0136ff2c, byte-identical
    #   across the two frames). A register a draw inherits holds whatever the last draw left there,
    #   and that draw does not have to be the same character. Only `sets` lines are evidence. The
    #   three registers around it -- 742c5c7b, 7a9915c5, 30bf03f4 -- are set by BOTH characters' own
    #   draws, so they are shared globals and the plan must leave them alone.
    "06790f7e": "skinRamp",
    # AND TWO MORE THE PLAN NEVER NAMED, FOUND THE SAME WAY (2026-09-20). Asking which draws SET
    #   each register of the target's hair pass (71f60c46) against Chisa's own (b455d737) leaves two
    #   per-character textures the fix was not binding, both of which the skin's shader reads:
    #   a 512 x 4 gradient -- hers 2b16c5ac (127.8, 135.5, 135.9), the skin's 57aa5a71 (140.7,
    #   141.3, 133.2), the warmer of the two -- and, at ps-t7, the SAME subsurface ramp the body
    #   reads. That is why the hair kept a warm cast at the ENDS after the ps-t2 ramp was corrected:
    #   the tips are where the hair catches the light, so a warm term shows there first.
    "2b16c5ac": "hairTipRamp",
    # The 512 x 512 sheen ramp, likewise per character: hers bb73967a (205.2, 160.1, 152.0) against
    #   the skin's 4bee4070 on the upper body and 74a761f5 on the lower. Deliberately left alone on
    #   the previous round so that one thing changed at a time; the round proved the subsurface ramp
    #   alone does not account for the reported red, so it goes in now.
    "bb73967a": "bodySheen",
    "6ae8dd10": "faceMask", "d030af95": "faceDiffuse",
    "526b9ed0": "upperNormal", "90196068": "upperMask", "165f3a1b": "upperDiffuse",
    "2b6f8bcb": "lowerNormal", "3f0e6f21": "lowerMask", "f642139e": "lowerDiffuse",
    "019c268e": "accessoryDiffuse", "40528957": "accessoryNormal", "4eaa9816": "accessorySheen",
    "226b31fc": "irisDiffuse",
    # 742c5c7b (1024 sRGB) and 8224e584 (2048 sRGB) are bound by BOTH characters -- a shared detail
    #   texture and an eye one -- so they are left to the game rather than given a role to rebind
}

# source component -> (target slot, {ps register: role}). One to one; the diffuse register of the two
#   clothing slots is ps-t3 on the skin where Chisa's own pass reads it at ps-t2 (header, point 3).
Plan = {
    0: (0, {"ps-t0": "frontHairMask", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"}),
    # ps-t7 IS DELIBERATELY NOT BOUND, AND TRYING IT COST A ROUND (2026-09-20). The subsurface ramp
    #   was mirrored onto the hair pass here at the same time as ps-t4, on the strength of both being
    #   512 x 25 -- two changes in one round, against this file's own advice. The result says which
    #   did what: the ENDS stopped being orange (ps-t4, a root-to-tip gradient, doing its job) and
    #   the TOP of the hair gained a red stain. Chisa's ramp is (143.1, 106.7, 141.2), magenta where
    #   the skin's is warm, and on a hair shader that lands where the light scatters -- the crown.
    #   Whatever the skin's ps-t7 is for, it is not what her ps-t4 clothing register holds.
    # REVERTED TO THE LAST STATE THE MAINTAINER CALLED ACCEPTABLE (2026-09-20), at their request:
    #   black on top, orange only at the ENDS. Everything tried past that point made the crown RED,
    #   which is worse, and three rounds failed to find why -- so the hair sits here until there is
    #   a measurement rather than another hypothesis.
    #
    #   What was tried and undone, so it is not tried again blind:
    #     ps-t4 = hairTipRamp (2b16c5ac)  -- FIXED the orange ends. Removed anyway: it is the only
    #        hair change still standing when the crown first went red, and the two 512 x 4 ramps are
    #        near-identical in their means and their ends, so it is not obviously the cause either.
    #        This is the one to reintroduce FIRST, on its own, when the crown is understood.
    #     ps-t7 = skinRamp (06790f7e)     -- removed; the crown stayed red without it.
    #     ps-t0 unbound (Chisa's mask is a flat (255, 0, 126, 0), and R = 255 is what both skins
    #        mean by bare skin) -- restored; the crown stayed red without it too.
    # ps-t5 IS THE ONE THAT WAS PAINTING THE HAIR, AND THE MAINTAINER FOUND IT IN ONE TEST by
    #   setting it to `null` in the generated command list (2026-09-20). Three rounds of mine had
    #   been spent on the ramps and the mask around it. The role is correctly assigned -- the mod's
    #   file has Chisa's own channel structure -- but the two shaders PACK that map differently,
    #   which the means give away at a glance:
    #
    #       the target's own d547f3c6   R   0.0  G  86.6  B   9.3  A 255
    #       Chisa's own      e921181d   R   8.6  G  52.7  B  41.1  A   0
    #       the mod's file bound here   R   2.8  G 117.1  B 114.0  A   0
    #
    #   B and A are structurally different between the two skins, exactly as they are in the
    #   material mask -- so the mod's map hands the target's shader a large B where it wants ~0 and
    #   A 0 where it wants 255, and the result is a warm cast over the hair. Unbound, the slot takes
    #   the GAME's own map, which is the right packing; its UVs are hers rather than the mod's, and
    #   on this map that is evidently the lesser error.
    #   NOT done for slot 0's frontHairNormal: same register, same shape of risk, untested. One at
    #   a time.
    1: (1, {"ps-t0": "hairMask", "ps-t1": "hairDiffuse", "ps-t2": "hairRamp"}),
    2: (2, {"ps-t0": "faceMask", "ps-t1": "faceDiffuse"}),
    # ps-t10 / ps-t6 is the subsurface ramp, and the two body halves read it at different registers
    #   because the two passes bind different numbers of auxiliary maps before it. Measured off the
    #   draws that SET them: the skin's upper pass (3311e8a5) binds 6a9ec87e at ps-t10 and its lower
    #   pass (a99f09b6) binds the same texture at ps-t6.
    3: (3, {"ps-t0": "upperNormal", "ps-t1": "upperMask", "ps-t3": "upperDiffuse",
            "ps-t8": "bodySheen"}),     # ps-t10: zeroed in NeutralBindings, see there
    4: (4, {"ps-t0": "lowerNormal", "ps-t1": "lowerMask", "ps-t3": "lowerDiffuse",
            "ps-t5": "bodySheen", "ps-t6": "skinRamp"}),
    # MEASURED, not guessed (2026-09-20): on the pass both skins draw this slot with (3df800c3)
    #   Chisa binds 019c268e / f2646d21 / 9ccd7ea7 at ps-t0 / t1 / t5 -- her accessory diffuse and
    #   then the FRONT HAIR pair, which is the per-character pair every other slot carries too. The
    #   guess had ps-t5 as the accessory normal (40528957), which she binds on a pass the skin never
    #   draws this slot on, and had no ps-t1 at all -- and ps-t1 is where the ribbon's colour comes
    #   from, so it kept the skin's 2f911db8 through four rounds. --paint over EVERY register is what
    #   found it: painting only the role called "diffuse" left the ribbon untouched and looked like
    #   the slot was not ours at all.
    #   ps-t3 is the one the RIBBON reads, found by giving each of the slot's remaining registers its
    #   own probe colour: it came back white, which was ps-t3's. Chisa binds a 256 x 128 grey stripe
    #   ramp there (4eaa9816, an anisotropic sheen) and the skin binds her pink frilly art, so the
    #   ribbon wore the skin's pattern. The mod ships no such file -- it is the game's -- so it comes
    #   through FallbackTextures out of the download folder.
    5: (5, {"ps-t0": "accessoryDiffuse", "ps-t1": "frontHairDiffuse", "ps-t3": "accessorySheen",
            "ps-t5": "frontHairNormal"}),
    6: (6, {"ps-t1": "irisDiffuse"}),
}
Plans = {"default": Plan}
Labels = {0: "front hair", 1: "hair", 2: "face", 3: "upper body", 4: "lower body", 5: "left-side accessory", 6: "eyes"}
TargetLabels = {0: "front hair", 1: "hair", 2: "face", 3: "upper body", 4: "lower body",
                5: "the skin's own (nothing maps onto it)", 6: "eyes", 7: "the skin's own, right hip (nothing maps onto it)"}

# THE SAILOR COLLAR HAS NO COUNTERPART ON THE SKIN, so its bones cannot all be matched well and the
#   finder scatters them: of the bones that carry it, the targets land in the skin's HAIR window
#   (31, 32, 37, 44, 56, 69, 70), her upper body (72, 87, 119, 120) and even her lower body (167).
#   Bones 1.9 to 7.6 apart on Chisa end up 17 to 27 apart on the skin, which tears the collar off the
#   shoulder -- reported in game as jagged and floating (2026-09-20).
#
#   Every one of those rows is already AT its nearest target by centroid, so there is nothing to
#   correct row by row: the tear is structural, and the answer is the Yelan lesson -- pin the chain
#   to one bone so the part stays rigid and attached instead of being pulled between several.
#
#   The chain is the bones whose weight lies MOSTLY on collar vertices, which are selected by
#   sampling Chisa's own atlas at each vertex's UV (the collar is its white, low-saturation region)
#   rather than by guessing at bone numbers -- collarAnchor.py in the session scratchpad. A bone that
#   merely reaches the collar is left alone: 152 carries 34% of its weight but only 12% of ITS OWN
#   weight is collar, and pinning that would rigidify the whole jacket.
#
#   Measured before being believed: vertices torn by more than 3 units go 372 -> 202 over the collar
#   and 2016 -> 1829 over the whole component, so it does not buy the collar at the shoulder's cost.
AnchorChains: Dict[str, Dict[int, List[int]]] = {
    "collar": {233: [192, 201, 202, 204, 223, 224, 228, 229, 230, 231, 232, 234, 240, 241]},
    # THE FOX MASK AND THE HAIRPINS, which the Hanabi mod hangs off Chisa's ACCESSORY bones
    #   (409-418) although their geometry is drawn by component 3. They are a rigid prop: all 1137
    #   of their vertices sit in a box 4.5 x 6.4 x 14 beside her head. The finder maps those ten
    #   bones one at a time, and the targets it picks are scattered over the whole model --
    #   measured off the skin's own skeleton, they span 59.5 x 92.6 x 145.7, from z 45.9 at the
    #   thigh to z 143.3 at the head. A rigid prop pulled between points that far apart is smeared
    #   through the body, which is why it reads as "the mask is gone" rather than as anything
    #   misplaced -- and why a flat per-component paint could not show it either, since the smear
    #   is the same colour as the torso it is buried in.
    #
    #   The key is a SOURCE bone and the chain takes whatever IT maps to. Source 3 is the bone the
    #   HEAD itself leans 83% of its weight on, and 27% of the prop's weight was already on it; the
    #   other 73% went to bones the finder matched one at a time, which tore the prop from her head
    #   down to her waist. Riding the head bone puts it back at (-9.5, -7.9, 137.0) -- three units
    #   from the (-10, -6, 139.9) its author modelled it at. This is the Yelan lesson: a part the
    #   target has no counterpart for wants ONE rigid anchor for the whole chain, not the finder's
    #   per-bone nearest. DON'T pick that anchor by reading a bone's position out of the
    #   translation column of its `vs-cb4` matrix -- that is a skinning matrix, not a pose, and it
    #   named a bone that put the prop 90 units away. Skin the prop with each candidate and look at
    #   where the cloud lands.
    "props": {3: [409, 410, 411, 412, 413, 414, 415, 416, 417, 418]},
}


def anchoredRemap(remap: Dict[int, int], mode: str) -> Dict[int, int]:
    """'remap' with every chain of the given anchor mode pinned to its root's target"""
    result = dict(remap)
    for name, chains in AnchorChains.items():
        if (mode != "all" and mode != name):
            continue
        for root, chain in chains.items():
            for group in chain:
                result[group] = remap[root]
    return result


# The WWMI command lists every component section runs, by the value of its `run =` line. The texture
#   command list is added right after the shared-resource override and before the first draw.
OverrideSharedResources = "CommandListOverrideSharedResources"

# A character past 256 merged bones keeps her true 16-bit bone ids in BlendRemapVertexVG.buf, and the
#   mod's own component section points the draw at WWMI's blend remap of THAT by setting these three.
#   They must not survive into a remapped section. BlendRemapper writes vb4 = reverse[trueId] and
#   SkeletonRemapper gathers skeleton[j] = merged[forward[j]], and the two maps are exact inverses --
#   so the pair feeds the draw `merged[trueId]`, the SOURCE's own index, against the TARGET's merged
#   skeleton. Every bone past the target's count reads a slot no draw ever writes (zero matrices), and
#   the components that have a blend remap collapse towards the origin while the ones that do not
#   render correctly: a body smeared into a drape under an intact head (2026-09-20, ChisaIdentity).
#   Removed, the section takes the `=== null` branch, which is the fix's own remapped blend and the
#   target's plain merged skeleton.
#   The predicate takes only the `ref` form, because an object's graph follows its `run =` lines and
#   the shared cleanup list sets the same three to `null` -- which is worth keeping as a safety net,
#   and is what the unfiltered edit removed as well. (`RegRemove` takes {register: predicate or None};
#   its constructor accepts a bare list or set too and the fixer then reads it as a mapping --
#   "dictionary update sequence element #0 has length 27", the length of the first NAME, and the
#   whole .ini is skipped with that as its only explanation.)
BlendRemapOverrideRegs = {reg: (lambda _ind, val: val.strip().lower().startswith("ref "))
                          for reg in ("ResourceBlendBufferOverride", "ResourceMergedSkeletonOverride", "ResourceExtraMergedSkeletonOverride")}

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
    """A flat texture of one colour, through the library's TexCreator (uncompressed, so the code is exact)"""
    FRB.TexCreator(size, size, FRB.Colour(*colour), compress = False).fix(FRB.TextureFile(path), path)


def fleshColoured(rgb: np.ndarray) -> np.ndarray:
    """Per pixel: warm, bright and ordered R > G >= B the way skin is -- a white or grey shirt is not"""
    r, g, b = (rgb[..., i].astype(np.int32) for i in range(3))
    return (r >= 150) & (r - g >= 15) & (g >= b)


# THE TEXTURE EDITS ARE THE LIBRARY'S, AND ONLY THE PIXEL ARITHMETIC IS THIS FILE'S (2026-09-22). Each
#   edit below is a TexEditor FILTER -- a function of the TextureFile the library opened -- handed to a
#   TexReplace through its fixFunc (texEdit()), the way yelanTranquilFix.py edits its light maps. The
#   library opens the file, runs the filter, writes the result with its own save (sRGB round trip
#   included), names the file, declares its resource section and rewrites every reference to it; the
#   undo removes it. This file used to do all of that by hand -- its own DDS writer, its own resource
#   sections, its own names -- and the maintainer's verdict on that is in Creating Remaps' "A prototype
#   is built FROM the library". What the library has no answer for is what these three edits COMPUTE.

def pixelsOf(texFile) -> np.ndarray:
    return np.array(texFile.img.convert("RGBA"))


def setPixels(texFile, px: np.ndarray) -> None:
    from PIL import Image
    texFile.img = Image.fromarray(np.ascontiguousarray(px, dtype = np.uint8), "RGBA")


def asData(texFile) -> None:
    """Write 'texFile' back WITHOUT the library's sRGB pre-correction: its bytes are data, not colour.

    The library's save pre-corrects a texture whose source was sRGB-tagged, so the linear file it
    writes renders as the original did -- right for a diffuse (the grade below keeps it), and wrong for
    a mask or a sheen profile, whose bytes are CODES the shader compares against thresholds: a mask
    code 183 would come out 124 and cross from Chisa's matte band into the one below it. Chisa's masks
    and her sheen ramp ARE sRGB-tagged (BC1 / BC7 _SRGB), and the values confirmed in game
    (2026-09-21) are the raw ones, so these two edits turn the correction off (2026-09-22)."""
    texFile.gamma = None


def sheenFilter(texFile) -> None:
    """Chisa's packed four-profile matcap as the target's foil: RGBA all her first profile (SheenTranslations)"""
    asData(texFile)
    profile = pixelsOf(texFile)[..., 0]
    setPixels(texFile, np.stack([profile, profile, profile, profile], axis = -1))


def maskFilter(diffusePath: Optional[str], label: str):
    """Repack a material mask from the SOURCE's layout into the TARGET's (MaskTranslations above).

    Every non-skin texel keeps the SOURCE's own R and G -- R is the shared band legend, G how shiny a
    surface is -- and takes only the target's packing in B and A (B ~126, A 0 on her side; B 0, A 255
    on Chisa's). Skin is R >= 0.9, or the matte band over a flesh-coloured diffuse, and takes the
    target's whole skin code. Prints the share written as non-skin when it runs."""
    def edit(texFile) -> None:
        asData(texFile)
        px = pixelsOf(texFile)
        out = px.copy()
        out[..., 2] = TargetMaskCloth[2]
        out[..., 3] = TargetMaskCloth[3]
        skin = px[..., 0] >= SourceMaskSkinAbove
        band = (px[..., 0] >= SourceMaskFleshBand) & ~skin
        if (band.any()):
            flesh = None
            try:
                if (diffusePath and os.path.isfile(diffusePath)):
                    diffuse = FRB.TextureFile(diffusePath, readPillowImg = True)
                    diffuse.open()
                    d = np.array(diffuse.img.convert("RGB"))
                    ys = np.arange(px.shape[0]) * d.shape[0] // px.shape[0]
                    xs = np.arange(px.shape[1]) * d.shape[1] // px.shape[1]
                    flesh = fleshColoured(d[ys][:, xs, :3])
            except Exception:
                flesh = None
            # with no diffuse to ask, the band keeps the old answer (skin) rather than guessing cloth
            skin |= band & (flesh if (flesh is not None) else True)
        out[skin] = np.array(TargetMaskSkin, dtype = np.uint8)
        out[..., 1] = px[..., 1]
        setPixels(texFile, out)
        print(f"    {label}: repacked into {TargetName}'s layout, {100.0 * float((~skin).mean()):.1f}% of it non-skin")
    return edit


def gradeFilter(gain, label: str):
    """A per-channel gain on the RGB (ColourGrades above), alpha untouched; prints the medians it moved"""
    def edit(texFile) -> None:
        px = pixelsOf(texFile)
        out = px.copy()
        for c in range(3):
            out[..., c] = np.clip(px[..., c].astype(np.float64) * gain[c], 0, 255).astype(np.uint8)
        setPixels(texFile, out)
        was = tuple(int(v) for v in np.median(px.reshape(-1, 4)[:, :3], axis = 0))
        now = tuple(int(v) for v in np.median(out.reshape(-1, 4)[:, :3], axis = 0))
        print(f"    {label}: graded by {gain} for {TargetName}'s shader, median {was} -> {now}")
    return edit


def texEdit(resModObj, subType: str, filterFunc) -> "FRB.TexReplace":
    """A TexReplace whose file is written by a TexEditor running 'filterFunc' -- uncompressed, because a
    mask is codes and BCn would move them"""
    def fix(resource) -> bool:
        editor = FRB.TexEditor([filterFunc], readPillowImg = True, compress = False)
        editor.fix(FRB.TextureFile(resource.srcPath, readPillowImg = True), resource.fixedPath)
        return True
    edit = FRB.TexReplace(resModObj, FRB.TexEditor([], compress = False), fixFunc = fix, resSubType = subType)
    _alive.extend([fix, edit])
    return edit


# ---------------------------------------------------------------------------------------------
# the parser

SlotObjs = [("", f"{SlotPrefix}{i}") for i in range(7)]
BoneDataObjs = {"cb4": ("", "boneData")}
ShapeKeyObjs = {"shapekey_offsets": ("", "shapekeyOffsets"), "shapekey_scale": ("", "shapekeyScale")}


def hashOnlyObjs(shapeKeys: bool) -> Dict[str, Tuple[str, str]]:
    """The objects identified by a hash alone: the bone-data override always, the shape-key overrides
    only when the run retargets them (see the header, point 8)"""
    return {**BoneDataObjs, **(ShapeKeyObjs if shapeKeys else {})}


def makeParser(sourceType, shapeKeys: bool = False, plannedRoles: Optional[List[str]] = None):
    """
    A GIMIParser whose sections are sorted by the API's hash / index classifier -- the seven draw
    slots by the vb0 hash plus their match_first_index, the bone-data and shape-key overrides by a
    hash of their own. The texture overrides carry hashes the library does not know and stay
    unclassified: they keep serving the mod on Chisa and never fire while the Parfait skin is drawn.
    """
    def factory(iniFile, modTypeId):
        # The version is passed explicitly. A reverse lookup with no version resolves through the
        # NEWEST bucket holding the value, and `0` (component 0's index) is every GI head's index
        # too, filed at 6.1: that bucket holds no Chisa row, so component 0 classified as nothing.
        version = iniFile.fromVersion if (iniFile.fromVersion is not None) else SourceVersion
        hashOnly = hashOnlyObjs(shapeKeys)
        classifier = FRB.GIMISectionClassifier(dict(hashOnly), sourceType.hashes, {"vb0": {obj: obj for obj in SlotObjs}}, sourceType.indices, version)
        classifier.hashNonVersionVals = {"name": SourceName}
        classifier.indexNonVersionVals = {"name": SourceName}

        # the textures: every [TextureOverrideTexture] that binds a planned role is classified as that
        #   role's mod object (TextureRoles), and a role with no such section gets a download instead
        text = iniFile.fileTxt if (getattr(iniFile, "fileTxt", None)) else open(iniFile.file, encoding = "utf-8", errors = "replace").read()
        roles = TextureRoles(iniFile.file, text, plannedRoles or [])
        _rolesByIni[os.path.normcase(os.path.abspath(iniFile.file))] = roles

        def texTargets(p, sectionName, ifTemplate, single, part = None, colouring = None):
            return [roleObj(r) for r in roles.rolesOfSection.get(sectionName, [])]
        downloads = {}
        for role, src in roles.downloads.items():
            name = f"{role[0].upper()}{role[1:]}{FRB.IniKeywords.RemapDL.value}"
            data = FRB.DownloadData(name, FRB.FileDownload(roles.downloadUrls[role], f"{SourceName}{name}.dds"))
            downloads[roleObj(role)] = {"this": data}
            _alive.append(data)
        texObjs = [roleObj(r) for r in sorted(roles.graphRoles)]
        # disjointModObjs off: one atlas may play two roles, so one override section may be two objects
        parser = FRB.GIMIParser(iniFile, modObjs = SlotObjs + list(hashOnly.values()) + texObjs, objTargetFuncs = [classifier, texTargets],
                                downloads = downloads, modTypeId = modTypeId, disjointModObjs = False)
        parser.trackKeys = True
        parser.keysToTrack = {"hash", "match_first_index", "match_index_count"}
        _alive.extend([classifier, texTargets])
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
AssetsFolder = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", SourceName, SourceVersion.replace(".", "_"))

# A planned role the mod has NO file for is bound to the SOURCE's own game texture (the mod's UVs are the
# source's, so the target's texture -- what an unbound register samples on the target's draw -- is wrong
# by construction: the red-camellia mod ships no bodice or skirt mask, and the Exorcist's mask at its UVs
# put skin codes over cloth, a reddish hue over the whole body, 2026-09-19). Role -> the source's hash;
# the API downloads the same file from the repo, the prototype copies it out of AssetsFolder. Roles whose
# hash BOTH skins bind (eyeMask c88cc1fc, faceMask 46177147) need none: the target's texture IS the source's.
# SlotBoneStandIns: per source component, {target bone: a stand-in} for a bone the skeleton its
#   vertices are skinned with does not hold. EMPTY, and the one entry it had was WRONG (2026-09-21).
#
#   It held {3: {3: 8}}, on this argument: the skin's vs-cb4 is rebuilt per draw slot, bone 3 is her
#   head on the face slot and something near the root on the upper-body slot, and her head is at no
#   index of the upper-body slot -- so the fox mask, pins and bells (component 3, all on bone 3) were
#   skinned to the wrong bone. All of that is true OF THE VANILLA GAME'S OWN DRAWS, and none of it
#   applies to the remap: CommandListOverrideSharedResources replaces vs-cb4 with
#   ResourceMergedSkeleton whenever no blend override is active (always, on the skin), and the merged
#   skeleton is one index space for every slot, where 3 IS her head. The per-slot matrices were read
#   off a skeleton the remapped draws never use.
#
#   In game the stand-in made no difference to the accessories and made the NECK wobble "like jello,
#   disconnected from the head": merged bone 8 is not her neck, it is whatever the merger put there,
#   and 9595 head-attached vertices -- the collar and choker among them -- rode it. Measure a remap
#   against the skeleton the REMAPPED draw is handed; for a WWMI remap that is the merged one.
SlotBoneStandIns: Dict[int, Dict[int, int]] = {}

# WHAT A COMPONENT IS TEXTURED WITH IS SAID BY THE COMPONENT'S OWN SECTION, NOT GUESSED (2026-09-21).
#   Three signals decide a role's file, in this order: what the component's own section BINDS (below),
#   then a [TextureOverrideTexture] whose hash names the role, then the file's shape; and a role none
#   of them fills falls back to the source's vanilla texture.
#
#   The first one is the mod's own statement, and it is what the Hanabi kimono mod actually uses: its
#   [TextureOverrideTexture] hashes appear NOWHERE in the current game -- not in any of the eight
#   Chisa dumps, and not in the max-LOD one (FrameAnalysis-Chisa-Max-LOD-2026-09-21-080231), which
#   binds exactly the hashes the 512 px ones do. They are dead on this version; what textures the mod
#   on Chisa is each component's own references:
#
#       component 1   Resource\RabbitFX\Diffuse = ref ResourceTexture5
#       component 3   ps-t0 = ResourceNormal, ps-t1 = ResourceSub, ps-t2 = ResourceBase
#       component 5   Resource\RabbitFX\Diffuse = ref ResourceTexture15      <- the body atlas: the sakura
#       0, 2, 4, 6    nothing at all, so on Chisa they draw with her VANILLA textures
#
#   That corrects a guess this file made the day before: "component 4 has no file, so borrow the upper
#   body's atlas". Component 4 binds nothing, so on Chisa it is vanilla, and the vanilla fallback was
#   right. The side panels were component 5 all along (the --paint cyan), and the maintainer found the
#   fix by binding ResourceBase at its ps-t3.
#
#   RabbitFX's Normalmap is deliberately NOT taken: RabbitFX binds its maps at ps-t60..t65 for its own
#   patched shaders, so a RabbitFX normal is not in the game's normal packing -- the hair's
#   ResourceTexture2 is one, and binding it at the game's ps-t5 was the orange-hair bug.
#
# The SOURCE's register layout per component, off Chisa's own max-LOD draws (wwmiPassLayout.py):
SourceRegisterRoles: Dict[int, Dict[str, str]] = {
    0: {"ps-t0": "frontHairMask", "ps-t1": "frontHairDiffuse", "ps-t5": "frontHairNormal"},
    1: {"ps-t0": "hairMask", "ps-t1": "hairDiffuse", "ps-t2": "hairRamp", "ps-t5": "hairNormal"},
    2: {"ps-t0": "faceMask", "ps-t1": "faceDiffuse"},
    3: {"ps-t0": "upperNormal", "ps-t1": "upperMask", "ps-t2": "upperDiffuse"},
    4: {"ps-t0": "lowerNormal", "ps-t1": "lowerMask", "ps-t2": "lowerDiffuse"},
    5: {"ps-t0": "accessoryDiffuse"},
    6: {"ps-t1": "irisDiffuse"},
}
# ...and the diffuse role a `Resource\RabbitFX\Diffuse = ref <resource>` in each component states
RabbitFXDiffuseRoles: Dict[int, str] = {0: "frontHairDiffuse", 1: "hairDiffuse", 2: "faceDiffuse", 3: "upperDiffuse",
                                        4: "lowerDiffuse", 5: "accessoryDiffuse", 6: "irisDiffuse"}
# ...and its NORMAL MAP and LIGHT MAP too (2026-09-21). A backless-sweater mod names all three through
#   RabbitFX on its lower body; reading only the diffuse left the real normal (4666f52a) roleless while
#   the shape rule handed the normal slot a shared red detail texture (see SharedGameTextures) -- the
#   dress rendered red. RabbitFX's "lightmap" is what this file calls the mask.
RabbitFXRoles: Dict[str, Dict[int, str]] = {
    "resource/rabbitfx/diffuse": RabbitFXDiffuseRoles,
    "resource/rabbitfx/normalmap": {0: "frontHairNormal", 1: "hairNormal", 3: "upperNormal", 4: "lowerNormal", 5: "accessoryNormal"},
    "resource/rabbitfx/lightmap": {0: "frontHairMask", 1: "hairMask", 2: "faceMask", 3: "upperMask", 4: "lowerMask"},
}
# Game textures BOTH skins bind themselves -- a shared detail map (742c5c7b, a strong red laid on at
#   alpha ~16) and an eye one (8224e584). A mod that ships a copy under an older hash (21f813ba) must
#   not have it placed by shape: it is not a role, it is the game's, and the game still binds it.
SharedGameTextures = {"742c5c7b", "8224e584"}

FallbackTextures: Dict[str, str] = {
    # CHISA's hashes -- this table arrived as a copy of Sanhua's and sat unmeasured until 2026-09-20,
    #   where every role name matched hers and every hash did not, so a role the mod lacked would have
    #   fetched a file that does not exist in this folder. Only `accessorySheen` is actually reached
    #   today: every other role here is one a mod of Chisa ships for itself.
    "frontHairMask": "d3b9ba76", "frontHairDiffuse": "f2646d21", "frontHairNormal": "9ccd7ea7",
    "hairMask": "a842d51f", "hairDiffuse": "cbab5910", "hairNormal": "e921181d", "hairRamp": "232c2dbc",
    "skinRamp": "06790f7e", "hairTipRamp": "2b16c5ac", "bodySheen": "bb73967a",
    "faceMask": "6ae8dd10", "faceDiffuse": "d030af95",
    "upperNormal": "526b9ed0", "upperMask": "90196068", "upperDiffuse": "165f3a1b",
    "lowerNormal": "2b6f8bcb", "lowerMask": "3f0e6f21", "lowerDiffuse": "f642139e",
    "accessoryDiffuse": "019c268e", "accessoryNormal": "40528957", "accessorySheen": "4eaa9816",
    "irisDiffuse": "226b31fc",
}
IdentityMin, IdentityGap = 0.97, 0.90   # a file IS a game texture when its colour correlates >= IdentityMin with one asset and < IdentityGap with every other
RepaintMin, RepaintGap = 0.60, 0.30     # a file is a REPAINT of a game texture when its LUMINANCE correlates >= RepaintMin with one and < RepaintGap with every other

# THE COMPONENT IN A MOD'S FILE NAME IS THE STRONGEST ROLE SIGNAL IT HAS, AND IT IS FREE
#   (2026-09-20). WWMI names every texture it exports `Components-<N> t=<hash>.dds`, and N is the
#   component the mod's own .ini binds it for -- so a file can only play a role of THAT component,
#   whatever it looks like. Without the constraint, a bunny-suit mod's `Components-3` upper-body
#   pair correlated 0.98 and 1.00 with Chisa's ACCESSORY diffuse and normal (its art is a repaint
#   of that atlas) and took those roles, while the real accessory files, matching at 1.00, were
#   refused as duplicates and the upper body got nothing at all.
RoleComponent = {"frontHairMask": 0, "frontHairDiffuse": 0, "frontHairNormal": 0,
                 "hairMask": 1, "hairDiffuse": 1, "hairNormal": 1, "hairRamp": 1,
                 "skinRamp": 3, "hairTipRamp": 1, "bodySheen": 3,
                 "faceMask": 2, "faceDiffuse": 2,
                 "upperNormal": 3, "upperMask": 3, "upperDiffuse": 3,
                 "lowerNormal": 4, "lowerMask": 4, "lowerDiffuse": 4,
                 "accessoryDiffuse": 5, "accessoryNormal": 5,
                 "irisDiffuse": 6}
# `Components-1-2 t=...` serves BOTH components, and skipping such a name cost a round in game:
#   a Hanabi mod's own hair diffuse is called `Components-1-2 t=23b680fe.dds`, went unplaced, and
#   the hair then drew with Chisa's downloaded atlas at the mod's UVs -- warm patches over black hair.
ModComponentPattern = re.compile(r"components?[\s_-]*(\d+(?:-\d+)*)[\s_-]+t=", re.IGNORECASE)


def kindOfShape(shape) -> Tuple[str, str]:
    """A texture's KIND from its pixels: "Mask", "Normal" or "Diffuse", with why.

    CHECKED AGAINST AN ORACLE BEFORE IT WAS USED. The seventeen textures in `Roles` have known
    roles, so they are the test set for any rule that guesses one, and the obvious rules fail it:
    "R and G centred on 127" is true of the three CLOTHING normals and false of the two hair ones
    (Chisa's front hair normal averages 27, 50, 101), and "saturated with a flat alpha" is true of
    every mask AND of both hair normals. What separates those two is how many distinct colours the
    texture holds -- a mask is a handful of codes (1 to 13 here) and a normal map is a continuous
    field (64 to 293). This rule gets 17 of 17; the first one written got 15."""
    mean, saturation, colours = shape
    if (colours <= 40 and saturation >= 0.9):
        return "Mask", f"{colours} distinct colours at full saturation, so codes rather than shading"
    if (abs(mean[0] - 127) < 8 and abs(mean[1] - 127) < 8):
        return "Normal", "R and G both centred on 127"
    if (saturation >= 0.25 and colours >= 40):
        return "Normal", f"saturated and continuous ({colours} colours)"
    return "Diffuse", "neither a set of codes nor a normal map"


def componentsOfModFile(name: str) -> List[int]:
    """Every component a WWMI-exported texture's name claims (usually one)"""
    match = ModComponentPattern.search(os.path.basename(name))
    return [int(n) for n in match.group(1).split("-")] if (match) else []


def componentOfModFile(name: str) -> Optional[int]:
    """The single component it claims, or None when it names none or several"""
    components = componentsOfModFile(name)
    return components[0] if (len(components) == 1) else None
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
                            # A GROUP MAY OFFER SEVERAL FILES FOR ONE HASH, AND THEY ARE ONE ROLE.
                            #   A `$part_0` toggle mod writes `if $part_0 == 0 / this = ResourceTextureN
                            #   / elif $part_0 == 1 / this = ResourceTextureN_2`, the first being the
                            #   character's own art and the second the mod's. Taking only the first left
                            #   the mod's own copy roleless.
                            for res in (v for k, v in kvps if k == "this"):
                                if (h and res in resources):
                                    hashesOfFile.setdefault(resources[res], []).append(h)
        # A FILE NO RESOURCE SECTION NAMES IS AN ALTERNATIVE THE AUTHOR SHIPPED, NOT THE MOD'S ART
        #   (2026-09-20). A Hanabi mod carries `1Color Variation  Remove trans/{Black,Red,White}/
        #   Components-3 t=4c7e5ddf.dds` beside the `Textures/` copy the player actually installed:
        #   six alternative colourways of one upper-body diffuse, for the player to copy over the
        #   live one. They walk in before `Textures/` and sort before it, so the fix bound a
        #   colourway the player had NOT chosen -- in game, a kimono with the wrong bust panel.
        #   The mod's own `.ini` says which is live: it declares a resource for it.
        self.referenced = {f for resources in self.resourcesByIni.values() for f in resources.values()}
        counts = {"hash": 0, "pixels": 0, "name": 0}
        pending: List[str] = []
        for f in sorted(ddsFiles, key = lambda p: (p not in self.referenced, p)):
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
        self.alternativesOf: Dict[str, List[str]] = {}
        for f, hashes in hashesOfFile.items():
            for other, theirs in hashesOfFile.items():
                if (other != f and set(hashes) & set(theirs)):
                    self.alternativesOf.setdefault(f, []).append(other)
        pending.sort(key = lambda p: (p not in self.referenced, p))
        self.shared: Dict[str, str] = {}      # file -> the shared game texture it is a copy of (SharedGameTextures)
        for f, h, score in self._identify(pending):
            if (h in SharedGameTextures):
                self.shared[f] = h
                continue
            components = componentsOfModFile(f)
            if (components and RoleComponent.get(Roles[h]) not in components):
                continue                            # its own name says it belongs to another component
            self.roleOf[f] = [(Roles[h], f"the game's own {h} by its pixels ({score:.2f})")]; counts["pixels"] += 1
        counts["shape"] = 0
        for f in self._byShape([p for p in pending if (p not in self.roleOf and p not in self.shared)]):
            self.roleOf[f[0]] = [(f[1], f[2])]; counts["shape"] += 1
        for f in pending:
            if (f in self.roleOf or f in self.shared):
                continue
            match = ComponentFilePattern.search(os.path.basename(f))
            role = TypeRoles.get(int(match.group(1)), {}).get(TypeOfSuffix.get(match.group(2).lower(), "")) if (match) else None
            if (role):
                self.roleOf[f] = [(role, "its name")]; counts["name"] += 1
            else:
                self.unresolved.append(f)
        counts["luma"] = 0
        for f, role, why in self._byLuminance([p for p in pending if (p not in self.roleOf and p not in self.shared)]):
            self.roleOf[f] = [(role, why)]; counts["luma"] += 1
        counts["copy"], counts["draw"] = self._byCopyAndDraw(ddsFiles)
        self.unresolved = [f for f in self.unresolved if (f not in self.roleOf)]
        print(f"  textures under {os.path.basename(root)}: {len(ddsFiles)} files -- {counts['hash']} placed by hash, {counts['pixels']} by pixel identity with a game texture, "
              f"{counts['shape']} by their component and shape, {counts['name']} by their Component<N>_<Type> name, {counts['luma']} as a recolour of one, "
              f"{counts['copy']} as byte-identical copies, {counts['draw']} by the dumped draw they came from, {len(self.unresolved)} with no role")

    def _byLuminance(self, files: List[str]):
        """(file, role, why) for a RECOLOURED game texture: its shading, not its colour, is the game's.

        THE SAME MOD'S BANGS WERE PINK, AND PINK IS NOT BLACK (2026-09-21). The shirt mod repaints
        Chisa's black bangs pink and ships them as `Components-0-1-2-3-4-5 t=15cddd7c.dds`: a hash
        no dump has, a name listing six components, and an RGB correlation with her own bangs far
        below `IdentityMin`. It went roleless, so the bangs drew with her game diffuse -- black
        bangs over the author's pink hair. The strands are the game's strands, though: grayscale
        at 128 x 128, the file correlates 0.74 with her bangs diffuse and at most -0.01 with any
        other diffuse she has, and her long-hair repaint 0.81 with her hair. A repaint that
        CHANGES the art (the shirt over her upper-body atlas: 0.47 against 0.27) clears neither
        threshold and is left to the rules that follow.

        Only roles no file holds yet, and only diffuses -- a normal map or a mask is not shading."""
        if (not files):
            return
        taken = {role for roles in self.roleOf.values() for role, _ in roles}
        game: Dict[str, np.ndarray] = {}
        for role, h in FallbackTextures.items():
            asset = os.path.join(AssetsFolder, f"{SourceName}Texture{h}.dds")
            if (role.endswith("Diffuse") and role not in taken and os.path.isfile(asset)):
                g = self._luminanceOf(asset)
                if (g is not None):
                    game[role] = g
        if (not game):
            return
        for f in files:
            x = self._luminanceOf(f)
            if (x is None):
                continue
            components = componentsOfModFile(f)
            scores = sorted(((float((x * y).sum()), r) for r, y in game.items()
                             if (not components or RoleComponent.get(r) in components)), reverse = True)
            if (not scores or scores[0][0] < RepaintMin or (len(scores) > 1 and scores[1][0] >= RepaintGap)):
                continue
            role = scores[0][1]
            runnerUp = f" against {scores[1][0]:.2f} for her {scores[1][1]}" if (len(scores) > 1) else ""
            yield f, role, f"a recolour of {SourceName}'s own {role}: luminance {scores[0][0]:.2f}{runnerUp}"
            del game[role]

    @staticmethod
    def _luminanceOf(path: str, size: int = 128) -> Optional[np.ndarray]:
        """A texture's grayscale box-averaged to size x size, zero-mean and unit-norm, or None"""
        try:
            tex = FRB.TextureFile(path)
            tex.open()
            px = np.frombuffer(tex.getPixels(), dtype = np.uint8).reshape(tex.height, tex.width, 4)[..., :3].astype(np.float64)
        except Exception:
            return None
        if (px.shape[0] % size or px.shape[1] % size):
            return None
        g = (px @ np.array([0.299, 0.587, 0.114])).reshape(size, px.shape[0] // size, size, px.shape[1] // size).mean(axis = (1, 3))
        g -= g.mean()
        norm = np.linalg.norm(g)
        return (g / norm) if (norm) else None

    def _byCopyAndDraw(self, ddsFiles: List[str]) -> Tuple[int, int]:
        """(copies placed, dump-named files placed): the two things a file's NEIGHBOURS say about it.

        A MOD WHOSE HASHES ARE ALL STALE STILL SAYS WHICH REGISTER EACH OF SOME FILES CAME FROM
        (2026-09-21). A shirt mod for Chisa carries 25 texture hashes, none of them in any dump of
        today's game, and ships its upper-body diffuse twice: as `Components-0-1-2-3-4 t=662f126a.dds`,
        whose name lists five components and so decides nothing, and as
        `000134-ps-t2=2970cef1-vs=...-ps=dce450ef....dds` -- a FRAME ANALYSIS file name, which states
        the draw it was dumped from and the register it was bound at. The same draw's `ps-t0` file is
        byte-identical to `Components-3 t=eca8e09b.dds`, which the shape rule had already placed as
        the upper-body normal. So that draw is component 3's, and its `ps-t2` is what component 3
        binds at `ps-t2`: the upper diffuse. Left roleless, the shirt drew with the GAME's upper
        diffuse under the mod's UVs -- a dark, lustrous mottle over a shirt the author painted white.

        Two rules, each only ever filling a file nothing else placed:
            a byte-identical copy of a placed file plays its roles;
            files dumped from ONE draw (same draw number and pixel shader) belong to ONE component,
                decided by any member already placed at the register it names in SourceRegisterRoles,
                and each other member takes that component's role at ITS register."""
        import hashlib
        byContent: Dict[str, List[str]] = {}
        for f in ddsFiles:
            try:
                with open(f, "rb") as fh:
                    byContent.setdefault(hashlib.md5(fh.read()).hexdigest(), []).append(f)
            except OSError:
                pass

        def shareCopies() -> int:
            placed = 0
            for group in byContent.values():
                known = next((g for g in group if (g in self.roleOf)), None)
                if (known is None):
                    continue
                for g in group:
                    if (g not in self.roleOf):
                        self.roleOf[g] = [(role, f"byte-identical to {os.path.basename(self.real.get(known, known))}") for role, _ in self.roleOf[known]]
                        placed += 1
            return placed

        copies = shareCopies()
        draws: Dict[Tuple[str, str], List[Tuple[str, str]]] = {}
        for f in ddsFiles:
            match = DumpFilePattern.match(os.path.basename(f))
            if (match):
                draws.setdefault((match.group(1), match.group(4).lower()), []).append((f, f"ps-t{int(match.group(2))}"))
        byDraw = 0
        for (draw, ps), members in draws.items():
            components = set()
            for f, reg in members:
                for role, _ in self.roleOf.get(f, []):
                    components |= {c for c, regs in SourceRegisterRoles.items() if (regs.get(reg) == role)}
            if (len(components) != 1):
                continue                    # nothing placed in this draw, or its members disagree
            component = components.pop()
            for f, reg in members:
                role = SourceRegisterRoles[component].get(reg)
                if (role and f not in self.roleOf):
                    self.roleOf[f] = [(role, f"dumped at {reg} of draw {draw} (ps {ps[:8]}), component {component}'s")]
                    byDraw += 1
        return copies + shareCopies(), byDraw

    def _byShape(self, files: List[str]):
        """(file, role, why) for a REPAINTED texture: its component says which roles it may play, and
        its own pixels say which of them it is.

        This is the case pixel identity cannot reach -- a mod that repaints a component's art matches
        none of the game's textures -- and the component in the file name is what makes it safe: the
        choice is between that component's two or three roles, not between all seventeen.

            a NORMAL map has both R and G centred on 127, whatever its blue;
            a MASK is fully saturated with a flat alpha (it is codes, not shading);
            anything else is the DIFFUSE.

        Checked against the files pixel identity DOES place: on the bunny mod every one of the eight
        it recognised gets the same role from this rule, which is the only reason to trust it on the
        eight it cannot."""
        taken = {role for roles in self.roleOf.values() for role, _ in roles}
        # A FILE THAT NAMES ONE COMPONENT SPEAKS FOR IT; ONE THAT NAMES FOUR IS A LEFTOVER CLAIM.
        #   `Components-0-2-4-6 t=21f813ba.dds` sorts before `Components-4 t=9b396b0d.dds` and took
        #   the lower body's NORMAL off it, which is the role the single-component file was named
        #   for. Singles go first and the shared files take only what is left.
        for f in sorted(files, key = lambda p: (p not in self.referenced, len(componentsOfModFile(p)) != 1, p)):
            components = componentsOfModFile(f)
            # a file an earlier group already spoke for is not judged again on its own pixels
            if (not components or f in self.roleOf):
                continue
            free = [r for r, c in RoleComponent.items() if (c in components and r not in taken)]
            shape = self._shapeOf(f)
            if (not free or shape is None):
                continue
            wanted, why = kindOfShape(shape)
            candidates = [r for r in free if (r.endswith(wanted))]
            if (not candidates):
                continue
            named = "-".join(str(c) for c in components)
            if (len(candidates) == 1):
                role, why = candidates[0], f"component {named} in its name, and {why}"
            else:
                # A NAME LISTING SEVERAL COMPONENTS LEAVES A CHOICE, AND BRIGHTNESS CANNOT MAKE IT --
                #   a repaint changes that. CHROMA can: the differences between the channels survive a
                #   recolour far better than their level, and the source's own texture for each
                #   candidate is right there to compare against. Only a CLEAR winner is taken; an
                #   ambiguous file is better left to the download than guessed onto the wrong part.
                ranked = sorted((d, r) for r, d in ((r, self._chromaDistance(shape[0], r)) for r in candidates)
                                if (d is not None))
                if (len(ranked) < 2 or ranked[0][0] > 30 or ranked[0][0] > 0.5 * ranked[1][0]):
                    continue
                role = ranked[0][1]
                why = (f"components {named} in its name, {why}, and its colour balance is {ranked[0][0]:.0f} from "
                       f"{SourceName}'s own {role} against {ranked[1][0]:.0f} from her {ranked[1][1]}")
            taken.add(role)
            yield f, role, why
            # a `$part_0` toggle mod offers two files for one hash -- the character's own art and
            #   its own repaint. They are one role, and the shape rule would judge the repaint on
            #   its own pixels and get a different answer (the bunny mod's recoloured hair normal
            #   reads as a mask). The one bound is whichever comes first, which is the mod's
            #   `$part_0 == 0` default.
            for other in self.alternativesOf.get(f, []):
                if (other not in self.roleOf and set(componentsOfModFile(other)) & set(components)):
                    yield other, role, f"the same [TextureOverrideTexture] as {os.path.basename(self.real.get(f, f))}"

    _chromaCache: Dict[str, object] = {}

    @classmethod
    def _chromaDistance(cls, mean, role: str):
        """How far a texture's colour BALANCE is from the source's own texture for 'role', or None.

        Levels move when a mod recolours; the differences between the channels mostly do not, so this
        is what tells a hair diffuse from a face one without assuming either is unpainted."""
        if (role not in cls._chromaCache):
            asset = os.path.join(AssetsFolder, f"{SourceName}Texture{FallbackTextures.get(role, '')}.dds")
            shape = cls._shapeOf(asset) if (FallbackTextures.get(role) and os.path.isfile(asset)) else None
            cls._chromaCache[role] = shape[0] if (shape is not None) else None
        theirs = cls._chromaCache[role]
        if (theirs is None):
            return None
        return abs((mean[0] - mean[1]) - (theirs[0] - theirs[1])) + abs((mean[1] - mean[2]) - (theirs[1] - theirs[2]))

    @staticmethod
    def _shapeOf(path: str):
        """(mean RGB, median saturation, distinct 5-bit colours) of a subsample, or None"""
        try:
            tex = FRB.TextureFile(path)
            tex.open()
            px = np.frombuffer(tex.getPixels(), dtype = np.uint8).reshape(tex.height, tex.width, 4).astype(np.float64)[::8, ::8]
        except Exception:
            return None
        high, low = px[..., :3].max(axis = 2), px[..., :3].min(axis = 2)
        return (px[..., :3].mean(axis = (0, 1)), float(np.median((high - low) / np.maximum(high, 1))),
                len(np.unique((px[..., :3] // 32).reshape(-1, 3), axis = 0)))

    def _identify(self, files: List[str]):
        """(file, asset hash, colour correlation) for every file that is one of the game's own textures under another name"""
        if (not files or not os.path.isdir(AssetsFolder)):
            return
        sys.path.insert(0, os.path.join(Repo, "Tools", "Misc", "Diagnostics"))
        import wwmiTextureFix as texFix
        cache: Dict[str, object] = {}
        assets = {h: texFix.decode(p, cache) for h, p in texFix.currentTextures(AssetsFolder).items() if (h in Roles or h in SharedGameTextures)}
        for f in files:
            x = texFix.decode(f, cache)
            if (x is None):
                continue
            scores = sorted(((texFix.corr(x[..., :3], y[..., :3]), h) for h, y in assets.items() if y is not None), reverse = True)
            if (scores and scores[0][0] >= IdentityMin and (len(scores) == 1 or scores[1][0] < IdentityGap)):
                yield f, scores[0][1], scores[0][0]


# a frame analysis texture's file name: <draw>-ps-t<N>=<hash>[(<hash>)]-vs=<vs>-ps=<ps>.dds
DumpFilePattern = re.compile(r"^(\d{6})-ps-t(\d+)=([0-9a-fA-F]{8})(?:\([0-9a-fA-F]{8}\))?-vs=[0-9a-fA-F]+-ps=([0-9a-fA-F]+)\.dds$", re.IGNORECASE)

_textureIndex: Optional[TextureIndex] = None


def textureIndex() -> TextureIndex:
    global _textureIndex
    if (_textureIndex is None or _textureIndex.root != ServiceRoot):
        _textureIndex = TextureIndex(ServiceRoot)
    return _textureIndex


def plannedRolesOf(plan) -> List[str]:
    """Every role the plan, the slots' other passes and the shared meshes bind"""
    roles = [role for _slot, regs in plan.values() for role in regs.values()]
    roles += [role for byPass in ExtraPassRegs.values() for regs in byPass.values() for role in regs.values()]
    roles += [role for byPass in SharedMeshes.values() for regs in byPass.values() for role in regs.values()]
    return [r for r in dict.fromkeys(roles) if (isinstance(r, str) and r != SkinMask)]


def roleObj(role: str) -> Tuple[str, str]:
    """The parser's mod object for a texture role -- the graph of the mod's [TextureOverrideTexture] that
    binds it, or of the section the parser invents for its download"""
    return ("", f"tex{role[0].upper()}{role[1:]}")


def fileUrl(path: str) -> str:
    """A local file as a URL the library's FileDownload accepts (spaces must be escaped)"""
    from urllib.parse import quote
    return "file:///" + quote(os.path.abspath(path).replace(os.sep, "/"), safe = "/:")


# WHERE A FALLBACK TEXTURE IS FETCHED FROM: `master`, exactly as the compiled fixer's DownloadTools does,
#   because that is what a user's run reads. --localDownloads reads the same paths out of this checkout
#   instead -- for an asset added here and not merged yet (on 2026-09-22 four of Chisa's 2_8 textures:
#   the two ramps, the hair-tip gradient and the accessory sheen). A mod's OWN file that no resource
#   names is always copied locally: it is the mod's, not the download folder's.
DownloadRoot = "https://github.com/nhok0169/Anime-Game-Remap/raw/master/Data/Mod%20Downloads"
LocalDownloads = False


def assetUrl(assetPath: str) -> str:
    """The URL a download-folder asset is fetched from (DownloadRoot, or this checkout under --localDownloads)"""
    if (LocalDownloads):
        return fileUrl(assetPath)
    from urllib.parse import quote
    rel = os.path.relpath(assetPath, os.path.join(Repo, "Data", "Mod Downloads")).replace(os.sep, "/")
    return f"{DownloadRoot}/{quote(rel)}"


class TextureRoles():
    """
    Which texture each role is, for one .ini -- decided ONCE, when the parser is built, because the parser
    is what turns the answer into graphs:

    * a role the mod binds through a [TextureOverrideTexture] -> that section is classified as the role's
      mod object (roleObj), so its whole graph -- `$swapvar` toggles and all -- is what the fixer copies;
    * a role the mod has a file for and NO resource section, or no file at all while the plan binds it ->
      a parser DOWNLOAD on the role's object (the mod's own file, or the source's game texture out of the
      download folder), so it becomes a graph of the same shape;
    * a role declared only in a component's own section with no override behind it -> bound by its
      resource name (the one case the graph cannot carry into a pass list).

    Which FILE plays a role is still this file's own placement (TextureIndex: hash, pixels, shape,
    luminance, dumped draws) -- the library has nothing that decides what an unknown .dds is.
    """
    def __init__(self, iniPath: str, text: str, plannedRoles: List[str]):
        index = textureIndex()
        key = os.path.normcase(os.path.abspath(iniPath))
        iniFolder = os.path.dirname(key)
        self.sections = iniSections(text)
        resources = index.resourcesByIni.get(key, {})
        resourceOfFile: Dict[str, str] = {}
        for res, f in resources.items():
            resourceOfFile.setdefault(f, res)
        self.fileOfResource: Dict[str, str] = {res.lower(): f for res, f in resources.items()}
        real = lambda f: index.real.get(f, f)       # noqa: E731 -- the file's real spelling
        self.resourceOfRole: Dict[str, str] = {}
        self.fileOfRole: Dict[str, str] = {}
        self.borrowed: set = set()                  # roles whose file is ANOTHER role's too (a shared atlas): never graded
        unreferenced: Dict[str, str] = {}           # role -> a file of the mod no resource section names
        byRole: Dict[str, List[Tuple[str, str]]] = {}
        for f, fileRoles in index.roleOf.items():
            for role, how in fileRoles:
                byRole.setdefault(role, []).append((f, how))

        def rank(f: str):
            rel = os.path.relpath(f, iniFolder).replace("\\", "/")
            return (0 if (f in resourceOfFile) else 1, rel.count("../"), len(rel))
        for role, cands in byRole.items():
            cands.sort(key = lambda c: rank(c[0]))
            best = cands[0][0]
            if (len(cands) > 1 and rank(cands[1][0])[:2] == rank(best)[:2]):
                print(f"    WARNING: {os.path.relpath(cands[1][0], index.root)} also has the role {role} ({cands[1][1]}), "
                      f"already taken by {os.path.relpath(best, index.root)} ({cands[0][1]}); the first one is bound")
            self.fileOfRole[role] = real(best)
            if (best in resourceOfFile):
                self.resourceOfRole[role] = resourceOfFile[best]
            else:
                unreferenced[role] = real(best)

        # a role a component's OWN section names (SourceRegisterRoles / RabbitFXRoles) wins over placement
        for name, lines in self.sections.items():
            match = re.fullmatch(r"TextureOverrideComponent(\d+)", name, re.IGNORECASE)
            if (not match):
                continue
            component = int(match.group(1))
            for line in lines:
                k, value = keyValue(line)
                if (k is None):
                    continue
                k = k.lower().replace(chr(92), "/")
                role = SourceRegisterRoles.get(component, {}).get(k)
                if (role is None and k in RabbitFXRoles):
                    role = RabbitFXRoles[k].get(component)
                if (role is None):
                    continue
                resource = value[4:].strip() if (value.lower().startswith("ref ")) else value
                f = self.fileOfResource.get(resource.lower())
                if (f is None):
                    continue
                self.resourceOfRole[role] = resource
                self.fileOfRole[role] = f
                unreferenced.pop(role, None)
                print(f"    {role}: bound by component {component}'s own section ({k} = {resource})")
        for role, f in self.fileOfRole.items():
            if (any((other != role and g == f) for other, g in self.fileOfRole.items())):
                self.borrowed.add(role)

        # the [TextureOverrideTexture] each role is read from: ONE per role, the first whose `this` names
        #   the role's resource -- a mod that declares one resource under several hashes (its character
        #   screen and battle-damage copies) would otherwise hand the fixer the same list several times
        thisOf: Dict[str, List[str]] = {}
        for name, lines in self.sections.items():
            if (name.lower().startswith("textureoverridetexture")):
                thisOf[name] = [v.lower() for k, v in map(keyValue, lines) if (k == "this")]
        self.sectionOfRole: Dict[str, str] = {}
        for role, res in self.resourceOfRole.items():
            sec = next((n for n, vals in thisOf.items() if (res.lower() in vals)), None)
            if (sec):
                self.sectionOfRole[role] = sec
        self.rolesOfSection: Dict[str, List[str]] = {}
        for role, sec in self.sectionOfRole.items():
            self.rolesOfSection.setdefault(sec, []).append(role)

        # the roles that become a parser download: a file no resource names, or none at all
        self.downloads: Dict[str, str] = {}         # role -> the file
        self.downloadUrls: Dict[str, str] = {}      # role -> where the library fetches it from
        for role in plannedRoles:
            if (role in self.sectionOfRole):
                continue
            if (role in unreferenced):
                self.downloads[role] = unreferenced[role]
                self.downloadUrls[role] = fileUrl(unreferenced[role])
            elif (role in self.resourceOfRole):
                continue
            elif (role in FallbackTextures):
                asset = os.path.join(AssetsFolder, f"{SourceName}Texture{FallbackTextures[role]}.dds")
                if (os.path.isfile(asset)):
                    self.downloads[role] = asset
                    self.downloadUrls[role] = assetUrl(asset)
                    self.fileOfRole.setdefault(role, asset)
                else:
                    print(f"    WARNING: no {os.path.basename(asset)} under {AssetsFolder} for the {role} the mod lacks")
        self.graphRoles = set(self.sectionOfRole) | set(self.downloads)


_rolesByIni: Dict[str, TextureRoles] = {}


def rolesOf(ini) -> TextureRoles:
    return _rolesByIni[os.path.normcase(os.path.abspath(ini.file))]


class ModFiles():
    """One mod's draws per source component and the folder its textures live in, read off the .ini file
    the API parsed; which texture each role is lives in TextureRoles"""
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
        index = textureIndex()
        iniPath = os.path.normcase(os.path.abspath(ini.file))
        iniFolder = os.path.dirname(iniPath)
        files = list(index.resourcesByIni.get(iniPath, {}).values())
        self.textureFolder = next((os.path.dirname(os.path.relpath(index.real.get(f, f), iniFolder)).replace("\\", "/") for f in files), "") or "Textures"
        self.unknownTextures: List[str] = [os.path.relpath(f, index.root) for f in index.unresolved]


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


VertexVGFile = "BlendRemapVertexVG.buf"     # WWMI's per-vertex 16-bit merged bone ids, beside Blend.buf


def componentDrawsOf(sections) -> Dict[int, List[Tuple[int, int]]]:
    """{source component: [(count, start) of every drawindexed in its own TextureOverrideComponent<N>]}"""
    out: Dict[int, List[Tuple[int, int]]] = {}
    for name, lines in sections.items():
        match = re.fullmatch(r"TextureOverrideComponent(\d+)", name, re.IGNORECASE)
        if (not match):
            continue
        for line in lines:
            draw = re.match(r"\s*drawindexed\s*=\s*(\d+)\s*,\s*(\d+)", line, re.IGNORECASE)
            if (draw):
                out.setdefault(int(match.group(1)), []).append((int(draw.group(1)), int(draw.group(2))))
    return out


def weightsPerVertexOf(sections) -> Optional[int]:
    """`$\\WWMIv1\\weights_per_vertex_count` out of the mod's own .ini, wherever it sets it.

    WWMI writes it beside `custom_vertex_count` in whichever command list configures the mesh, so
    this scans every section rather than naming one."""
    for lines in sections.values():
        for line in lines:
            key, value = keyValue(line)
            if (key is not None and key.replace("\\", "/").endswith("WWMIv1/weights_per_vertex_count")):
                try:
                    return int(value)
                except ValueError:
                    return None
    return None


def remapWWMIBlend(vgRemap, forced: bool, declared: Optional[int], libraryVertexCount: int,
                   componentDraws: Optional[Dict[int, List[Tuple[int, int]]]] = None):
    """A RemapBlendResource fixFunc: Blend.buf -> the resource's fixed path, indices through 'vgRemap'
    (the library's row the resource carries, unless 'forced' says the script's table wins).

    A MOD WHOSE MERGED SKELETON PASSES 256 BONES DOES NOT KEEP ITS BONE IDS IN Blend.buf (Chisa: 420
    bones, and her identity mod carries WWMI's blend remap for components 3, 4 and 5). There the
    8-bit ids are the merged ones TRUNCATED, and the real ones are the 16-bit
    BlendRemapVertexVG.buf beside them, which WWMI's BlendRemapper compute shader sends through a
    per-component reverse map into a private copy of Blend.buf at load -- so remapping Blend.buf's
    own bytes would be remapping numbers the game never reads. When that file is there it is what
    this reads, for every vertex (it holds all of them, not only the remapped components'), and the
    weights come from Blend.buf unchanged. The result fits 8 bits because the TARGET's merged
    skeleton is small (the Parfait skin reaches bone 250), which is also why the remapped sections
    can drop the remap machinery altogether -- see neutraliseBlendRemap."""
    def fix(resource) -> bool:
        remap = vgRemap if (forced or getattr(resource, "vgRemap", None) is None) else resource.vgRemap
        vertexVG = os.path.join(os.path.dirname(resource.srcPath), VertexVGFile)
        if (not os.path.isfile(vertexVG)):
            FRB.BlendFile(resource.srcPath, wwmiBlendElements()).remap(remap, fixedBlendFile = resource.fixedPath)
            return True

        blend = np.fromfile(resource.srcPath, dtype = np.uint8)
        ids16 = np.fromfile(vertexVG, dtype = "<u2")

        # THE INFLUENCES A VERTEX HAS COME FROM THE MOD, AND SO DOES ITS VERTEX COUNT (2026-09-20).
        #   Blend.buf is 2n bytes a vertex and VertexVG n uint16s, so ONE of n and the vertex count
        #   gives the other -- and this used to take the count from the library's VertexCountData
        #   row. That is the SOURCE CHARACTER's count, which is the mod's only when the mod is the
        #   identity mod: a real Chisa mod came in at 74835 vertices against her own 64588, nothing
        #   divided, the blend was skipped, and the .ini it had already written bound a
        #   RemapBlend.buf that was never created -- so the model did not draw at all. The mod
        #   states n itself (`$\WWMIv1\weights_per_vertex_count`), which is the only figure here
        #   that is a property of the export rather than of the character.
        weights = declared if (declared and declared > 0) else None
        if (weights is None and libraryVertexCount > 0 and not blend.size % (2 * libraryVertexCount)):
            weights = blend.size // (2 * libraryVertexCount)      # a mod that does not say, at the library's count
        if (not weights or blend.size % (2 * weights) or ids16.size % weights):
            raise SystemExit(f"'{os.path.basename(resource.srcPath)}' is {blend.size} bytes and '{VertexVGFile}' "
                             f"{ids16.size * 2} at {weights} influences a vertex: neither divides evenly")
        vertexCount = blend.size // (2 * weights)
        if (ids16.size // weights != vertexCount):
            raise SystemExit(f"'{VertexVGFile}' is {ids16.size // weights} vertices where Blend.buf is {vertexCount}")
        rows = blend.reshape(vertexCount, 2 * weights)
        ids = ids16.reshape(vertexCount, weights).astype(np.int64)

        # 'remap' is this script's dict when the run forced one and the resource's own VGRemap
        #   otherwise -- the API's BlendFile takes either, a dict lookup does not
        remapRows = dict(getattr(remap, "remap", remap))
        missing = sorted({int(i) for i in np.unique(ids) if int(i) not in remapRows})
        if (missing):
            raise SystemExit(f"the remap has no row for source vertex groups {missing[:10]} that the mod's blend uses")
        table = np.zeros(int(max(remapRows)) + 1, dtype = np.int64)
        for source, target in remapRows.items():
            table[int(source)] = int(target)
        mapped = table[ids]

        # ...then the per-slot stand-ins (SlotBoneStandIns), over each source component's own vertices
        indexPath = os.path.join(os.path.dirname(resource.srcPath), "Index.buf")
        if (SlotBoneStandIns and componentDraws and os.path.isfile(indexPath)):
            index = np.fromfile(indexPath, dtype = "<u4").astype(np.int64)
            for component, standIns in SlotBoneStandIns.items():
                ranges = [index[start:start + count] for count, start in componentDraws.get(component, [])]
                if (not ranges):
                    continue
                verts = np.unique(np.concatenate(ranges))
                verts = verts[verts < vertexCount]
                rowsOf = mapped[verts]
                for old, new in standIns.items():
                    hit = rowsOf == old
                    if (hit.any()):
                        rowsOf[hit] = new
                        print(f"    component {component}: target bone {old} -> {new} on "
                              f"{int(hit.any(axis = 1).sum())} vertices (the slot that draws them has no bone {old})")
                mapped[verts] = rowsOf

        if (int(mapped.max()) > 255):
            raise SystemExit(f"a remapped bone index of {int(mapped.max())} does not fit the 8-bit Blend.buf; "
                             f"{TargetName} would need blend remap buffers of its own, which this prototype does not write")

        out = np.concatenate([mapped.astype(np.uint8), rows[:, weights:]], axis = 1)
        out.tofile(resource.fixedPath)
        note = "" if (rows.shape[0] == libraryVertexCount) else f" (the LIBRARY's row for {SourceName} says {libraryVertexCount})"
        print(f"    blend: {rows.shape[0]} vertices x {weights} influences{note}, ids taken from {VertexVGFile} "
              f"(the mod carries a blend remap), highest remapped bone {int(mapped.max())}")
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
        remap = anchoredRemap(remap, anchor) if (anchor != "none") else remap
    targetSlots = int(target["components"][-1]["vg_offset"]) + int(target["components"][-1]["vg_count"])
    if (max(remap.values()) >= targetSlots):
        raise SystemExit(f"the remap points outside the Parfait skin's {targetSlots} merged slots")
    changed = sum(1 for g in base if remap.get(g) != base[g])
    forced = (remapOverride is not None) or bool(anchor)
    print(f"  vertex group remap: {'the library row' if not forced else ('--vgRemap' if remapOverride is not None else 'the library row')}"
          + (f", --anchor {anchor}" if anchor else "") + f" ({len(remap)} rows, {changed} differ from the library row)")
    return FRB.VGRemap(remap), forced


def makeFixer(sourceType, targetType, remapOverride: Optional[Dict[int, int]] = None, anchor: Optional[str] = None, shapeKeys: bool = False, probe: object = False,
              planName: str = "default", paint: bool = False, paintPass: Optional[List[int]] = None):
    plan = Plans[planName]
    source, target = characterFromLibrary(sourceType), characterFromLibrary(targetType)
    vgRemap, forcedRemap = effectiveRemap(sourceType, target, remapOverride, anchor)
    naming = FRB.CppIniNamingTools

    def factory(parser, toModName: str, modTypeId: int):
        ini = parser._iniFile
        modType = FRB.ModTypeIdTools.getModType(modTypeId)
        files = ModFiles(ini, parser, source)
        roles = rolesOf(ini)
        if (not files.present):
            raise ValueError(f"no [TextureOverrideComponent*] section on {SourceName}'s hash {source['vb0_hash']}")
        print(f"  {os.path.relpath(ini.file, ini.folder) if ini.folder else ini.file}: {SourceName} components {files.present} -> {toModName}")

        def fixName(name: str) -> str:
            return naming.getRemapFixName(name, toModName)

        maskResource = fixName(f"Resource{SkinMask}")      # a 3dmigoto resource section's name starts with Resource

        probed: set = set()                             # --probe: the flat colours already declared in this .ini
        probeLegend: List[str] = []
        neutrals: set = set()                           # NeutralBindings: the flat map, declared once per .ini
        neutralLegend: List[str] = []
        painted: set = set()                            # --paint: the flat colours already declared
        paintLegend: List[str] = []
        extraLegend: List[str] = []                     # ExtraPassRegs: the slot's other passes
        appended: List[str] = []

        # ---- the texture command lists: one per (role, register), COPIED from the role's own graph ----
        # A list line is `run = <list>`, and the list is the mod's [TextureOverrideTexture] for that role
        #   with its `hash` lines dropped and `this` renamed to the register (see the edits below), so the
        #   mod's own `if $swapvar ...` toggles come with it untouched. Only the PASS a list runs on is
        #   written here as text: no library edit wraps content in a new `if` block (the one gap).
        clOfPair: Dict[Tuple[str, str], str] = {}           # (role, register) -> the command list's name

        def regTag(reg: str) -> str:
            return reg.replace("-", "").capitalize()

        def bindLine(role, reg) -> Optional[str]:
            """The list line binding 'role' at 'reg', or None when the mod has nothing for it"""
            if (role == SkinMask):
                return f"    {reg} = {maskResource}"
            if (role in roles.graphRoles):
                name = fixName(f"CommandList{SourceName}{role[0].upper()}{role[1:]}{regTag(reg)}")
                clOfPair[(role, reg)] = name
                return f"    run = {name}"
            if (role in roles.resourceOfRole):
                return f"    {reg} = {roles.resourceOfRole[role]}"
            return None
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

        # ---- a texcoord copy with no NaN in it, when the mod's has any (2026-09-21) ----
        # THE FOX MASK, THE HAIRPINS AND THE BELLS WERE DRAWN, PLACED AND TEXTURED CORRECTLY, AND
        #   INVISIBLE, BECAUSE THEIR SECOND UV IS NaN. A dump taken WITH the remap installed showed every
        #   one of their draws issued inside the upper-body section, and their vertices skinned by the
        #   merged skeleton the draw was actually handed land on her head at the right size. What
        #   separates them from the parts that render is the texcoord buffer: its 16 bytes a vertex are
        #   UV0 and then six more halves, and on the fox mask and the bells halves 2..3 are NaN on EVERY
        #   vertex (the pins on 41%), while the red ribbon drawn beside them has none and renders. The
        #   skin's upper-body shader reads that second UV and Chisa's evidently does not, so the same
        #   bytes that are harmless on the source poison the pixel on the target. Every other vertex of
        #   the mesh carries ~(0, 0) there, which is what a NaN becomes in this copy. The mod's own
        #   buffer, and its own half of the .ini, are left alone.
        texcoordResource = None
        # looked up without regard to case: this mod spells it ResourceTexCoordBuffer, and the first
        #   version of this step asked for ResourceTexcoordBuffer, found nothing, and said nothing
        texcoordLines = next((lines for name, lines in files.sections.items() if (name.lower() == "resourcetexcoordbuffer")), [])
        texcoordFile = next((v for k, v in map(keyValue, texcoordLines) if k == "filename"), None)
        if (texcoordFile):
            src = os.path.join(ini.folder, texcoordFile.replace(chr(92), "/"))
            if (os.path.isfile(src)):
                halves = np.fromfile(src, dtype = np.float16)
                bad = np.isnan(halves)
                if (bad.any()):
                    halves = halves.copy()
                    halves[bad] = 0
                    fixedFile = os.path.join(os.path.dirname(texcoordFile.replace(chr(92), "/")), f"{toModName}{FRB.IniKeywords.Remap.value}Texcoord.buf").replace(chr(92), "/")
                    halves.tofile(os.path.join(ini.folder, fixedFile))
                    lines = texcoordLines
                    fmt = next((v for k, v in map(keyValue, lines) if k == "format"), "DXGI_FORMAT_R16G16_FLOAT")
                    stride = next((v for k, v in map(keyValue, lines) if k == "stride"), "16")
                    texcoordResource = fixName("ResourceTexcoordNoNaN")
                    appended.append(chr(10).join([f"[{texcoordResource}]", "type = Buffer", f"format = {fmt}", f"stride = {stride}", f"filename = {fixedFile}", ""]))
                    print(f"    vb2 (texcoords): {int(bad.sum())} NaN halves set to 0 in a remap-only copy -> {fixedFile}")

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
            def bound(role, reg):
                """The list line binding a role -- or, under --paint, that slot's flat colour for a diffuse"""
                if (paint and role.lower().endswith("diffuse") and i in PaintColours):
                    name = fixName(f"ResourcePaint{PaintNames[i].capitalize()}")
                    if (name not in painted):
                        rel = os.path.join(files.textureFolder, f"Paint{PaintNames[i].capitalize()}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
                        os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                        writeSolidDds(os.path.join(ini.folder, rel), PaintColours[i])
                        appended.append("\n".join([f"[{name}]", f"filename = {rel}", ""]))
                        painted.add(name)
                        paintLegend.append(f"      component {i} ({Labels.get(i, i)}): every diffuse -> flat {PaintNames[i]}")
                    return f"    {reg} = {name}"
                return bindLine(role, reg)

            bindings = [line for reg, role in regs.items() for line in [bound(role, reg)] if line]
            if (paint):
                # --paint is a question about GEOMETRY, not about a role: bind every register the
                #   target's passes read, so anything this slot draws is unmistakable whatever the
                #   shader samples. A surface still showing its own art after this is not ours.
                painted.add(fixName(f"ResourcePaint{PaintNames[i].capitalize()}"))
                flat = fixName(f"ResourcePaint{PaintNames[i].capitalize()}")
                bindings = [f"    ps-t{n} = {flat}" for n in range(9)]
            for reg, colour in NeutralBindings.get(i, {}).items():
                if (reg in regs):
                    continue
                # ONE RESOURCE PER COLOUR, NOT PER SLOT (2026-09-20). The name used to be the bare
                #   NeutralName, so the first colour written won and every later register reused its
                #   file: ps-t2 wanted (0, 0, 0, 255) and ps-t4 / ps-t10 wanted (0, 0, 0, 0), and all
                #   three got the first. Harmless only because the RGB happened to agree -- the next
                #   neutral with a different colour would have been silently discarded. flat() below
                #   already names by colour; this now does the same.
                stem = "{}{:02X}{:02X}{:02X}{:02X}".format(NeutralName, *colour)
                resource = fixName(f"Resource{stem}")
                if (resource not in neutrals):
                    neutralFile = os.path.join(files.textureFolder, f"{stem}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
                    os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                    writeSolidDds(os.path.join(ini.folder, neutralFile), colour)
                    appended.append("\n".join([f"[{resource}]", f"filename = {neutralFile}", ""]))
                    neutrals.add(resource)
                bindings.append(f"    {reg} = {resource}")
                neutralLegend.append(f"      component {i} {reg}: the skin's own map is hers -- bound to a flat {colour[:3]}")

            probeRegs = (probe.get(i) if isinstance(probe, dict) else (None if not probe else [r for r, _c, _n in ProbeColours]))
            if (probeRegs and (isinstance(probe, dict) or i in ProbeSlots)):
                # the named registers (default: every register this slot does not plan), flat and
                #   unmistakable -- see ProbeColours
                for reg, colour, name in ProbeColours:
                    if (reg in regs or reg not in probeRegs):
                        continue
                    resource = fixName(f"ResourceProbe{name.capitalize()}")
                    if (resource not in probed):
                        probeFile = os.path.join(files.textureFolder, f"Probe{name.capitalize()}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
                        os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                        writeSolidDds(os.path.join(ini.folder, probeFile), colour)
                        appended.append("\n".join([f"[{resource}]", f"filename = {probeFile}", ""]))
                        probed.add(resource)
                    bindings.append(f"    {reg} = {resource}")
                    probeLegend.append(f"      component {i} {reg} -> {name}")
            additions: List[Tuple[str, str]] = [("vb6", zeroResource)] if (zeroResource) else []
            if (texcoordResource):
                additions.append(("vb2", texcoordResource))
            if (bindings):
                cmdList = fixName(f"CommandList{SourceName}{SlotPrefix.capitalize()}{i}Textures")
                condition = " || ".join(dict.fromkeys(passTest(ps) for ps in SlotPasses[slot]))   # two passes may share one vertex shader
                appended.append("\n".join([f"[{cmdList}]", f"if {condition}"] + bindings + ["endif", ""]))
                additions.append(("run", cmdList))
            def flat(colour):
                """A created solid-colour resource, for a register the TARGET's shader reads and the
                   SOURCE has no texture for. Leaving it unbound hands the target's own map to the
                   mod's UVs, which is the material-mask bug; a flat map in the target's own legend
                   says the one thing that is true of every pixel of the part."""
                stem = "{}{:02X}{:02X}{:02X}{:02X}".format(FlatName, *colour)
                resource = fixName(f"Resource{stem}")
                if (resource not in neutrals):
                    rel = os.path.join(files.textureFolder,
                                       f"{stem}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace(chr(92), "/")
                    os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                    writeSolidDds(os.path.join(ini.folder, rel), colour)
                    appended.append(chr(10).join([f"[{resource}]", f"filename = {rel}", ""]))
                    neutrals.add(resource)
                return resource

            for n, (ps, extraRegs) in enumerate(ExtraPassRegs.get(slot, {}).items()):
                extra = [line for reg, role in extraRegs.items()
                         for line in [f"    {reg} = {flat(role)}" if isinstance(role, tuple) else bound(role, reg)] if line]
                # a --probe spec reaches these lists too: the register that paints a surface is a
                #   property of the PASS, and the pass that paints the ribbon is one the source
                #   never draws, so probing only the primary list would answer about the wrong draw
                for reg, colour, name in ProbeColours:
                    if (reg in extraRegs or reg not in (probeRegs or [])):
                        continue
                    resource = fixName(f"ResourceProbe{name.capitalize()}")
                    if (resource not in probed):
                        probeFile = os.path.join(files.textureFolder, f"Probe{name.capitalize()}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
                        os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                        writeSolidDds(os.path.join(ini.folder, probeFile), colour)
                        appended.append(chr(10).join([f"[{resource}]", f"filename = {probeFile}", ""]))
                        probed.add(resource)
                    extra.append(f"    {reg} = {resource}")
                    probeLegend.append(f"      component {i} on {ps} {reg} -> {name}")
                if (not extra):
                    continue
                cmdList = fixName(f"CommandList{SourceName}{SlotPrefix.capitalize()}{i}TexturesPass{n}")
                appended.append("\n".join([f"[{cmdList}]", f"if {passTest(ps)}"] + extra + ["endif", ""]))
                additions.append(("run", cmdList))
                extraLegend.append(f"      component {i} on {ps}: {', '.join(f'{r}={v}' for r, v in sorted(extraRegs.items()))}")

            if (paintPass and i in paintPass):
                # one command list per pass, each a different flat colour on every register
                additions = [a for a in additions if a[0] != "run"]
                for n, ps in enumerate(TargetSlotPasses[slot]):
                    colour, name = PassPaintColours[n % len(PassPaintColours)], PassPaintNames[n % len(PassPaintNames)]
                    resource = fixName(f"ResourcePass{name.capitalize()}")
                    if (resource not in painted):
                        rel = os.path.join(files.textureFolder, f"Pass{name.capitalize()}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
                        os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                        writeSolidDds(os.path.join(ini.folder, rel), colour)
                        appended.append("\n".join([f"[{resource}]", f"filename = {rel}", ""]))
                        painted.add(resource)
                    cmdList = fixName(f"CommandList{SourceName}{SlotPrefix.capitalize()}{i}Pass{n}")
                    appended.append("\n".join([f"[{cmdList}]", f"if {passTest(ps)}"]
                                               + [f"    ps-t{r} = {resource}" for r in range(11)] + ["endif", ""]))
                    additions.append(("run", cmdList))
                    paintLegend.append(f"      component {i} on pass {ps} -> flat {name}")

            if (additions):
                # right after the shared-resource override (the mod's buffers are bound there): the EARLIEST spot
                # after it, so a component drawn in several ranges has its textures before the FIRST draw. No
                # after-register: `drawindexed` as one is a MUST fact, and behind a `$draw_x` toggle no draw is
                # certain, which parked the additions INSIDE the first toggle (header point 14)
                edits.append(FRB.RegSurroundedAdd(additions,
                                                  beforeRegs = {"run": lambda v: v == OverrideSharedResources},
                                                  latest = False))
            edits.append(FRB.RegRemove(BlendRemapOverrideRegs))
            # A MOD'S OWN `ps-tN =` LINES MUST NOT SURVIVE BESIDE THE TEXTURE LISTS (2026-09-21). A
            #   component that binds its textures in its own section (the Hanabi kimono's upper body:
            #   `ps-t2 = ResourceBase`, `ps-t0 = ResourceNormal`, `ps-t1 = ResourceSub`) kept those lines
            #   in the remapped copy, AFTER the `run =` of its lists -- so on every pass they overrode
            #   what the lists had set, and the repacked mask and the neutral ps-t2 never reached the
            #   draw. They describe the SOURCE's register layout, which the lists replace; only the
            #   registers the lists set are dropped, so a register the mod binds for a library it
            #   calls is left alone.
            listed = {reg for reg, role in regs.items() if ((role, reg) in clOfPair or role == SkinMask or role in roles.resourceOfRole)}
            listed |= {line.split("=", 1)[0].strip() for line in bindings if (not line.strip().startswith("run"))}
            if (listed):
                edits.append(FRB.RegRemove({reg: None for reg in sorted(listed)}))
            edits.append(FRB.RegNewVals({"match_first_index": str(c["index_offset"]), "match_index_count": str(c["index_count"]),
                                         "$\\WWMIv1\\vg_offset": str(c["vg_offset"]), "$\\WWMIv1\\vg_count": str(c["vg_count"])}))
            edits += [hashRemap, rename]
            perObj[("", f"{SlotPrefix}{i}")] = edits

        # ---- the command lists, as graphs: each role's graph copied once per register it lands on ----
        clRemap: Dict[Tuple[int, str, str], List[Tuple]] = {(0, *roleObj(r)): [] for r in roles.graphRoles}
        clObjOfPair: Dict[Tuple[str, str], Tuple[str, str]] = {}
        for (role, reg), name in clOfPair.items():
            obj = roleObj(role)
            clObj = ("", f"cl{role[0].upper()}{role[1:]}{regTag(reg)}")
            clObjOfPair[(role, reg)] = clObj
            roots = set(parser._sectionTargets.get(obj) or [])
            base = name[:len(name) - len(fixName(""))] if (name.endswith(fixName(""))) else name
            rename = (lambda name, base, roots: (lambda old: name if ((old in roots) or (not roots and old.startswith("TextureOverride")))
                                                  else fixName(f"{base}{old}")))(name, base, roots)
            clRemap[(0, *obj)].append((0, *clObj, rename))
            # a list is not an override: its `hash` / `match_*` go, and `this` becomes the register
            perObj[clObj] = [FRB.RegRemove({"hash": None, "match_priority": None, "match_first_index": None, "match_index_count": None}),
                             FRB.RegRemap({"this": [reg]})]

        graphEdits: List[object] = [FRB.GraphGroupRemap(remap = clRemap), FRB.GraphGroupEdit([perObj])]

        # ---- the textures whose packing or grade differs between the skins: TexReplace over the lists ----
        def listsOf(role):
            return {(0, *clObj): reg for (r, reg), clObj in clObjOfPair.items() if (r == role)}
        for role in sorted({r for r, _ in clObjOfPair}):
            lists = listsOf(role)
            label = f"{role[0].upper()}{role[1:]}"
            if (role in MaskTranslations):
                diffuse = roles.fileOfRole.get(role[:-len("Mask")] + "Diffuse")
                graphEdits.append(FRB.ResRegCollect(lists, {role: texEdit((0, "", f"res{label}"), label, maskFilter(diffuse, role))}))
            elif (role in SheenTranslations):
                print(f"    {role}: {SourceName}'s packed sheen profiles as {TargetName}'s neutral foil")
                graphEdits.append(FRB.ResRegCollect(lists, {role: texEdit((0, "", f"res{label}"), label, sheenFilter)}))
            elif (role in ColourGrades and role not in roles.borrowed):
                graphEdits.append(FRB.ResRegCollect(lists, {role: texEdit((0, "", f"res{label}"), label, gradeFilter(ColourGrades[role], role))}))
        for role in MaskTranslations | SheenTranslations:
            if (role in roles.resourceOfRole and role not in roles.graphRoles and any(r == role for r, _ in clOfPair)):
                print(f"    WARNING: {role} is declared only in a component's own section, so it is bound as-is (no graph to edit)")
        if (dropped):
            graphEdits.append(FRB.GraphRemove([(0, "", f"{SlotPrefix}{i}") for i in dropped]))

        # ---- the blend: the register bound in the shared override, collected into a RemapBlend ----
        # A mod with a blend remap binds vb4 TWICE in the shared override: `vb4 = ResourceBlendBuffer`
        #   when no remap is active, and `vb4 = ref ResourceBlendBufferOverride` when one is. Only the
        #   first names a FILE -- the second is a buffer WWMI's BlendRemapper fills at load -- so the
        #   collection takes it and leaves the other (without this the run dies looking for a section
        #   called 'Resourceref Resource...').
        blendRefs = {(0, "", f"{SlotPrefix}{i}"): (lambda reg, resName, data: not resName.strip().lower().startswith("ref "))
                     for i in files.present if (i in plan)}
        graphEdits.append(FRB.ResRegCollect({(0, "", f"{SlotPrefix}{i}"): "vb4" for i in files.present if (i in plan)},
                                            {"blend": WWMIBlendReplace((0, "", "blend"), resType = "blend",
                                                                       fixFunc = remapWWMIBlend(vgRemap, forcedRemap,
                                                                                                weightsPerVertexOf(files.sections),
                                                                                                source["vertex_count"],
                                                                                                componentDrawsOf(files.sections)))},
                                            resPredicates = blendRefs))

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

        # ---- the meshes OUTSIDE the character's own vb0, whose textures still have to be hers ----
        for n, (meshHash, byPass) in enumerate(SharedMeshes.items()):
            lines = [f"; {meshHash} is drawn by both skins with the SAME geometry and each one's own textures",
                     f"[{fixName(f'TextureOverride{SourceName}SharedMesh{n}')}]", f"hash = {meshHash}"]
            wrote = False
            for ps, regs in byPass.items():
                binds = []
                for reg, role in regs.items():
                    line = bindLine(role, reg)
                    if (line is None):
                        continue
                    if (paint and role.lower().endswith("diffuse")):
                        resource = fixName("ResourcePaintWhite")
                        if (resource not in painted):
                            rel = os.path.join(files.textureFolder, f"PaintWhite{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
                            os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                            writeSolidDds(os.path.join(ini.folder, rel), (255, 255, 255, 255))
                            appended.append("\n".join([f"[{resource}]", f"filename = {rel}", ""]))
                            painted.add(resource)
                            paintLegend.append(f"      the shared mesh {meshHash}: every diffuse -> flat white")
                        line = f"    {reg} = {resource}"
                    binds.append(line)
                if (binds):
                    lines += [f"if {passTest(ps)}"] + binds + ["endif"]
                    wrote = True
            if (wrote):
                appended.append("\n".join(lines + [""]))
                print(f"    the shared mesh {meshHash}: {len(byPass)} passes rebound to {SourceName}'s own textures")

        # ---- the shader tags the texture command lists ask about, and the invented mask ----
        for i, (shader, filterIndex) in enumerate(ShaderFilters.items()):
            appended.append("\n".join([f"[{fixName(f'ShaderOverridePass{i}')}]", f"hash = {shader}", f"filter_index = {filterIndex}", ""]))
        if (any(role == SkinMask for i in files.present if (i in plan) for role in plan[i][1].values())):
            maskFile = os.path.join(files.textureFolder, f"{SkinMask}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
            os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
            writeSolidDds(os.path.join(ini.folder, maskFile), SkinMaskColour)
            appended.append("\n".join([f"[{maskResource}]", f"filename = {maskFile}", ""]))

        if (paintLegend):
            print("    --paint: whichever colour a part takes is the slot that draws it;")
            print("             a part that keeps its own look is drawn by NO section the fix writes")
            for line in paintLegend:
                print(line)
        if (extraLegend):
            print("    the slot's OTHER passes, bound as the source's own game binds them:")
            for line in extraLegend:
                print(line)
        if (neutralLegend):
            print("    registers the target's shader reads and the source's does not, flattened:")
            for line in neutralLegend:
                print(line)
        if (probeLegend):
            print("    --probe: every unplanned register of the clothing slots is a flat colour; whichever one")
            print("             shows on a surface is the register that paints it")
            for line in probeLegend:
                print(line)

        _alive.extend(graphEdits)
        fixer = FRB.GIMIFixer(parser, graphGroupEdits = graphEdits, modsToFix = [toModName])
        fixer.appendedSections = "\n".join(appended)
        _alive.append(fixer)

        # ---- report ----
        for i in files.present:
            if (i not in plan):
                continue
            slot, regs = plan[i]
            def described(role, reg):
                if (role == SkinMask):
                    return maskResource
                if ((role, reg) in clOfPair):
                    where = roles.sectionOfRole.get(role) or f"download of {os.path.basename(roles.downloads.get(role, ''))}"
                    return f"{role}<-{where}"
                return roles.resourceOfRole.get(role, "GAME (mod has none)")
            bound = ", ".join(f"{reg}={described(role, reg)}" for reg, role in regs.items())
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

# EVERY section of WWMI's shape-key pipeline, because HALF of it disabled is worse than all of it
#   (2026-09-20). `CommandListApplyShapeKeys` was missing from this list, so `--shapeKeys hide`
#   commented out the loader, the multiplier and their two callbacks -- everything that FILLS the
#   shape-key buffers -- and left the APPLIER wired into the mod's own draw, adding whatever was
#   left in those buffers to every vertex position. See the `--shapeKeys` option for why `hide` is
#   no longer the default.
ShapeKeySections = ["TextureOverrideShapeKeyOffsets", "TextureOverrideShapeKeyScale", "CommandListSetupShapeKeys", "CommandListLoadShapeKeys",
                    "TextureOverrideShapeKeyLoaderCallback", "CommandListMultiplyShapeKeys", "TextureOverrideShapeKeyMultiplierCallback",
                    "CommandListApplyShapeKeys"]
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
        # the mod's own text, its draw sections commented out: only mod.ini serves the mod on Chisa
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

    dangling = danglingReferences(stats.ini.fixed)
    if (dangling):
        # A SKIPPED RESOURCE IS A DANGLING REFERENCE, AND THE .INI IS ALREADY WRITTEN. The service
        #   catches a resource that raises, records it under `skipped`, and moves on -- but the
        #   fixed .ini has already been written naming the file that resource was going to create,
        #   so the mod is left binding something that is not there. A buffer 3dmigoto cannot create
        #   is not a missing texture: the draw gets no vertex data and the model DOES NOT RENDER,
        #   which is how a `SKIPPED ... Blend.buf` line two screens up turns into "the character
        #   disappeared" (2026-09-20). Say it at the end, where the exit code and the last line are.
        print(f"\n!! {len(dangling)} REFERENCE(S) IN THE FIXED .ini NAME A FILE THAT IS NOT THERE -- the mod will not render !!")
        for iniPath, reference in dangling:
            print(f"  {os.path.relpath(iniPath, folder)} -> {reference}")
        print("  a resource above was skipped; fix that, or undo, before looking in game")


def danglingReferences(iniPaths) -> List[Tuple[str, str]]:
    """Every `filename = ...` of a fixed .ini whose file is not on disk (check_dangling.py's rule,
    run by the fix itself rather than remembered afterwards)"""
    out: List[Tuple[str, str]] = []
    for iniPath in sorted(iniPaths):
        folder = os.path.dirname(iniPath)
        try:
            with open(iniPath, "r", encoding = "utf-8", errors = "replace") as f:
                lines = f.readlines()
        except OSError:
            continue
        for line in lines:
            key, value = keyValue(line)
            if (key is None or key.lower() != "filename" or not value):
                continue
            if (not os.path.isfile(os.path.join(folder, value.replace("\\", os.sep).replace("/", os.sep)))):
                out.append((iniPath, value))
    return out


def main():
    # a mod folder named in a non-Latin script (the shrine maiden mod's is Chinese) cannot be printed
    # through a cp1252 pipe, and a print that raises INSIDE the fix is recorded as a skipped .ini
    for stream in (sys.stdout, sys.stderr):
        stream.reconfigure(encoding = "utf-8", errors = "replace")

    parser = argparse.ArgumentParser(description = f"{SourceName} -> {TargetName}, in place, through the API's parser, fixer and RemapService")
    parser.add_argument("mod", help = f"the mod folder (every {SourceName} .ini under it is fixed in place; DISABLED ones are skipped)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--hideOrig", action = "store_true", help = f"comment out the mod's own {SourceName} sections, so the mod renders on {TargetName} only")
    parser.add_argument("--undo", action = "store_true", help = "remove a previous fix instead of fixing")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    # DEFAULTS TO "props", because a plain re-run without it silently un-fixes her fox mask and
    #   hairpins: the anchor lives in the BLEND buffer, so the last run to touch it wins, and
    #   nothing in the output says which way it went. A mod that weights nothing to those bones is
    #   unaffected, so there is no cost to having it on. Pass "none" to turn it off.
    parser.add_argument("--standIn", default = None,
                        help = "component:bone:standIn[,...] -- replace SlotBoneStandIns for this run (diagnostic)")
    parser.add_argument("--anchor", choices = sorted(AnchorChains) + ["all", "none"], default = "props",
                        help = "pin a chain the skin has no counterpart for to one bone (default: props)")
    # `hide` WAS THE DEFAULT AND IT BROKE THE MOD ON ITS OWN CHARACTER (2026-09-20). It comments
    #   sections out of the mod's OWN text, so whatever it does it does to the source's draws as
    #   well as the remap's -- and a real mod's shape-key pipeline is not decoration: on a 114213
    #   vertex Hanabi mod with 60791 shape-key vertices, both the original and the remap came out
    #   as spaghetti. It was adopted from the maintainer's working hand remap, which has shape keys
    #   off, and every mod it was tried on before was an identity mod, whose shape-key data is the
    #   character's own. The remapped draws answer the shape-key problem their own way -- each
    #   binds a zero offset stream at vb6 -- so hiding the mod's pipeline buys the remap nothing
    #   and costs the original everything.
    parser.add_argument("--shapeKeys", choices = ["hide", "leave", "retarget"], default = "leave",
                        help = f"the mod's shape-key sections: 'leave' (default) keeps them on {SourceName}'s own hashes, 'hide' comments them out like the maintainer's working hand remap (which BREAKS a mod that really uses them, on the source as well as the target), 'retarget' copies them onto {TargetName}'s buffer -- see the header, points 8 and 9")
    parser.add_argument("--plan", choices = list(Plans), default = "default",
                        help = "which source component goes through which target slot -- only 'default' exists for this pair (one to one; see the header, point 1)")
    parser.add_argument("--hideTextureOverrides", action = "store_true",
                        help = "comment the mod's own [TextureOverrideTexture] sections out too, as the hand remap does")
    parser.add_argument("--noSplit", action = "store_true",
                        help = "keep every remapped section in mod.ini (default: one section per target draw per file, the rest in <stem>ChisaParfaitRemapFix<n>.ini -- see the header, point 11)")
    parser.add_argument("--passPaint", type = int, nargs = "+", default = None, metavar = "SLOT",
                        help = "one flat colour per PASS of each source component named, to see which pass paints a surface "
                               "(several slots at once when they cover different parts of the body)")
    parser.add_argument("--paint", action = "store_true",
                        help = "replace every slot's diffuse with a flat colour, one per source component, to see which slot draws which part")
    parser.add_argument("--probe", nargs = "?", const = True, default = False, metavar = "SPEC",
                        help = "bind a flat colour to every register the clothing slots do not plan, to see in one round which one paints a surface; "
                               "SPEC (`3:ps-t4,4:ps-t5`) narrows it to the named registers per source component")
    parser.add_argument("--localDownloads", action = "store_true",
                        help = "fetch the fallback textures from this checkout's Data/Mod Downloads instead of master (for assets not merged yet)")
    parser.add_argument("--vgRemap", default = None, metavar = "JSON",
                        help = "a {source group: target group} table to write the blend with instead of the library's VGRemaps row (e.g. a draft sheet exported to json)")
    args = parser.parse_args()
    if (args.standIn is not None):
        # --standIn component:bone:standIn[,...] replaces SlotBoneStandIns for this run -- a diagnostic
        #   knob first: throwing a part onto a bone far from the body answers "is it drawn at all"
        #   in one round, which no amount of measuring where it SHOULD land can
        global SlotBoneStandIns
        SlotBoneStandIns = {}
        for entry in filter(None, args.standIn.split(",")):
            component, bone, standIn = (int(x) for x in entry.split(":"))
            SlotBoneStandIns.setdefault(component, {})[bone] = standIn
    remapOverride = None
    if (args.vgRemap):
        with open(winToPosix(args.vgRemap), "r", encoding = "utf-8") as f:
            remapOverride = {int(k): int(v) for k, v in json.load(f).items()}
    folder = os.path.abspath(winToPosix(args.mod))
    if (not os.path.isdir(folder)):
        raise SystemExit(f"not a folder: {folder}")
    global ServiceRoot, LocalDownloads
    ServiceRoot = folder
    LocalDownloads = args.localDownloads

    sourceType, targetType = FRB.WWMIBuilder.chisa(), FRB.WWMIBuilder.chisaParfait()
    FRB.CppStrategyOverrides.clear()
    retarget = (args.shapeKeys == "retarget")
    FRB.CppStrategyOverrides.setParser(SourceName, makeParser(sourceType, retarget, plannedRolesOf(Plans[args.plan])))
    probe = parseProbeSpec(args.probe) if isinstance(args.probe, str) else args.probe
    FRB.CppStrategyOverrides.setFixer(SourceName, TargetName, makeFixer(sourceType, targetType, remapOverride, args.anchor, retarget, probe, args.plan, args.paint, args.passPaint))
    try:
        runService(folder, args)
    finally:
        FRB.CppStrategyOverrides.clear()


if (__name__ == "__main__"):
    main()
