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

NeutralBindings = {3: {"ps-t2": (0, 0, 0, 255)}, 4: {"ps-t2": (0, 0, 0, 255)}}
NeutralName = "DetailZero"      # the resource / file stem of the flat map bound there

ProbeColours = [("ps-t2", (0, 255, 0, 255), "green"), ("ps-t3", (255, 255, 255, 255), "white"),
                ("ps-t4", (0, 0, 255, 255), "blue"), ("ps-t5", (255, 255, 0, 255), "yellow"),
                ("ps-t6", (0, 255, 255, 255), "cyan"), ("ps-t7", (255, 0, 255, 255), "magenta"),
                ("ps-t8", (255, 128, 0, 255), "orange")]
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
TargetMaskCloth = (255, 0, 126, 0)              # the skin's own dominant code, ie. ordinary cloth: R 93.6%, G 80.1%, B 84.8%, A 98.2%
TargetMaskSkinR = 0                             # what she marks bare skin with, in R
SourceMaskSkinAbove = 128                       # Chisa's R at or above this is bare skin
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
              5: ["3df800c350681ec9", "87825a9a29529f9b", "ced9a47fb6ad4d16"], 6: ["da00ec8f7c73d5e3"]}
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
PassFilters = {ps: f"{FilterBase + 0.01 * i:.4f}".rstrip("0")
               for i, ps in enumerate(dict.fromkeys([ps for passes in SlotPasses.values() for ps in passes]
                                                    + [ps for byPass in ExtraPassRegs.values() for ps in byPass]
                                                    + [ps for byPass in SharedMeshes.values() for ps in byPass]))}

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
    1: (1, {"ps-t0": "hairMask", "ps-t1": "hairDiffuse", "ps-t5": "hairNormal"}),
    2: (2, {"ps-t0": "faceMask", "ps-t1": "faceDiffuse"}),
    3: (3, {"ps-t0": "upperNormal", "ps-t1": "upperMask", "ps-t3": "upperDiffuse"}),
    4: (4, {"ps-t0": "lowerNormal", "ps-t1": "lowerMask", "ps-t3": "lowerDiffuse"}),
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


def translateMaterialMask(src: str, dst: str) -> float:
    """Repack a material mask from the SOURCE's layout into the TARGET's (MaskTranslations above).

    Every pixel takes the target's own cloth code, except where the source marks bare skin, which
    keeps the target's skin value in R. The other channels are hers throughout: the source's are a
    different shader's and mean nothing here. Returns the percentage written as cloth."""
    tex = FRB.TextureFile(src)
    tex.open()
    px = np.frombuffer(tex.getPixels(), dtype = np.uint8).reshape(tex.height, tex.width, 4)
    out = np.empty_like(px)
    out[...] = np.array(TargetMaskCloth, dtype = np.uint8)
    skin = px[..., 0] >= SourceMaskSkinAbove
    out[skin, 0] = TargetMaskSkinR
    writeDds(dst, out)
    return 100.0 * float((~skin).mean())


def writeDds(path, pixels):
    """An uncompressed R8G8B8A8_UNORM DDS of 'pixels' (height x width x RGBA)"""
    h, w = pixels.shape[0], pixels.shape[1]
    header = bytearray(b"DDS ")
    flags = 0x1 | 0x2 | 0x4 | 0x1000 | 0x8
    header += struct.pack("<IIIIIII", 124, flags, h, w, w * 4, 0, 1)
    header += b"\0" * 44
    header += struct.pack("<II4sIIIII", 32, 0x4, b"DX10", 0, 0, 0, 0, 0)
    header += struct.pack("<IIIII", 0x1000, 0, 0, 0, 0)
    header += struct.pack("<IIIII", 28, 3, 0, 1, 0)
    with open(path, "wb") as f:
        f.write(bytes(header) + np.ascontiguousarray(pixels, dtype = np.uint8).tobytes())


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
AssetsFolder = os.path.join(Repo, "Data", "Mod Downloads", "WuWa", SourceName, SourceVersion.replace(".", "_"))

# A planned role the mod has NO file for is bound to the SOURCE's own game texture (the mod's UVs are the
# source's, so the target's texture -- what an unbound register samples on the target's draw -- is wrong
# by construction: the red-camellia mod ships no bodice or skirt mask, and the Exorcist's mask at its UVs
# put skin codes over cloth, a reddish hue over the whole body, 2026-09-19). Role -> the source's hash;
# the API downloads the same file from the repo, the prototype copies it out of AssetsFolder. Roles whose
# hash BOTH skins bind (eyeMask c88cc1fc, faceMask 46177147) need none: the target's texture IS the source's.
FallbackTextures: Dict[str, str] = {
    # CHISA's hashes -- this table arrived as a copy of Sanhua's and sat unmeasured until 2026-09-20,
    #   where every role name matched hers and every hash did not, so a role the mod lacked would have
    #   fetched a file that does not exist in this folder. Only `accessorySheen` is actually reached
    #   today: every other role here is one a mod of Chisa ships for itself.
    "frontHairMask": "d3b9ba76", "frontHairDiffuse": "f2646d21", "frontHairNormal": "9ccd7ea7",
    "hairMask": "a842d51f", "hairDiffuse": "cbab5910", "hairNormal": "e921181d",
    "faceMask": "6ae8dd10", "faceDiffuse": "d030af95",
    "upperNormal": "526b9ed0", "upperMask": "90196068", "upperDiffuse": "165f3a1b",
    "lowerNormal": "2b6f8bcb", "lowerMask": "3f0e6f21", "lowerDiffuse": "f642139e",
    "accessoryDiffuse": "019c268e", "accessoryNormal": "40528957", "accessorySheen": "4eaa9816",
    "irisDiffuse": "226b31fc",
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
        self.fileOfRole: Dict[str, str] = {}        # role -> the mod's own file, for a role whose packing is translated
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
            self.fileOfRole[role] = real(best)
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

    def translate(self, roles: List[str]) -> List[str]:
        """The resource sections for the planned 'roles' in MaskTranslations: the mod's own file repacked
        into the TARGET's material-mask layout, written as <Role><Target>RemapTex.dds; binds them.

        RemapTex, not RemapRef: the fix writes this file, so the undo is meant to delete it."""
        out: List[str] = []
        for role in dict.fromkeys(roles):
            src = self.fileOfRole.get(role)
            if (role not in MaskTranslations or src is None or not os.path.isfile(src)):
                continue
            fileName = f"{role[0].upper()}{role[1:]}{TargetName}{FRB.IniKeywords.RemapTex.value}.dds"
            rel = f"{self.textureFolder}/{fileName}"
            os.makedirs(os.path.join(self.iniFolder, self.textureFolder), exist_ok = True)
            moved = translateMaterialMask(src, os.path.join(self.iniFolder, self.textureFolder, fileName))
            name = f"Resource{role[0].upper()}{role[1:]}{TargetName}{FRB.IniKeywords.RemapTex.value}"
            self.resourceOfRole[role] = name
            out.append("\n".join([f"[{name}]", f"filename = {rel}", ""]))
            print(f"    {role}: repacked into {TargetName}'s layout, {moved:.1f}% of it her cloth code -> {rel}")
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


VertexVGFile = "BlendRemapVertexVG.buf"     # WWMI's per-vertex 16-bit merged bone ids, beside Blend.buf


def remapWWMIBlend(vgRemap, forced: bool, vertexCount: int):
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
        # the influences a vertex has (4 for Sanhua, 8 for Chisa) is the only unknown, and the
        #   library's vertex count gives it: Blend.buf is 2n bytes a vertex, VertexVG n uint16s
        if (vertexCount <= 0 or blend.size % (2 * vertexCount) or ids16.size % vertexCount):
            raise SystemExit(f"'{os.path.basename(resource.srcPath)}' is {blend.size} bytes and '{VertexVGFile}' {ids16.size * 2} "
                             f"for {vertexCount} vertices: neither divides evenly")
        weightsPerVertex = blend.size // (2 * vertexCount)
        if (ids16.size // vertexCount != weightsPerVertex):
            raise SystemExit(f"'{VertexVGFile}' holds {ids16.size // vertexCount} ids a vertex where Blend.buf holds {weightsPerVertex}")
        rows = blend.reshape(vertexCount, 2 * weightsPerVertex)
        ids = ids16.reshape(vertexCount, weightsPerVertex).astype(np.int64)

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
        if (int(mapped.max()) > 255):
            raise SystemExit(f"a remapped bone index of {int(mapped.max())} does not fit the 8-bit Blend.buf; "
                             f"{TargetName} would need blend remap buffers of its own, which this prototype does not write")

        out = np.concatenate([mapped.astype(np.uint8), rows[:, weightsPerVertex:]], axis = 1)
        out.tofile(resource.fixedPath)
        print(f"    blend: {rows.shape[0]} vertices x {weightsPerVertex} influences, ids taken from {VertexVGFile} "
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
        remap = anchoredRemap(remap, anchor)
    targetSlots = int(target["components"][-1]["vg_offset"]) + int(target["components"][-1]["vg_count"])
    if (max(remap.values()) >= targetSlots):
        raise SystemExit(f"the remap points outside the Parfait skin's {targetSlots} merged slots")
    changed = sum(1 for g in base if remap.get(g) != base[g])
    forced = (remapOverride is not None) or bool(anchor)
    print(f"  vertex group remap: {'the library row' if not forced else ('--vgRemap' if remapOverride is not None else 'the library row')}"
          + (f", --anchor {anchor}" if anchor else "") + f" ({len(remap)} rows, {changed} differ from the library row)")
    return FRB.VGRemap(remap), forced


def makeFixer(sourceType, targetType, remapOverride: Optional[Dict[int, int]] = None, anchor: Optional[str] = None, shapeKeys: bool = False, probe: object = False,
              planName: str = "default", paint: bool = False):
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

        probed: set = set()                             # --probe: the flat colours already declared in this .ini
        probeLegend: List[str] = []
        neutrals: set = set()                           # NeutralBindings: the flat map, declared once per .ini
        neutralLegend: List[str] = []
        painted: set = set()                            # --paint: the flat colours already declared
        paintLegend: List[str] = []
        extraLegend: List[str] = []                     # ExtraPassRegs: the slot's other passes
        appended: List[str] = files.declare(fixName)     # the textures this .ini has no resource of its own for
        appended += files.fallbacks([role for i in files.present if (i in plan) for role in plan[i][1].values()])
        # ...and the masks whose PACKING differs between the two skins, repacked (MaskTranslations)
        appended += files.translate([role for i in files.present if (i in plan) for role in plan[i][1].values()])
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
            def bound(role):
                """The resource a role binds to -- or, under --paint, that slot's flat colour for a diffuse"""
                if (paint and role.lower().endswith("diffuse") and i in PaintColours):
                    name = fixName(f"ResourcePaint{PaintNames[i].capitalize()}")
                    if (name not in painted):
                        rel = os.path.join(files.textureFolder, f"Paint{PaintNames[i].capitalize()}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
                        os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                        writeSolidDds(os.path.join(ini.folder, rel), PaintColours[i])
                        appended.append("\n".join([f"[{name}]", f"filename = {rel}", ""]))
                        painted.add(name)
                        paintLegend.append(f"      component {i} ({Labels.get(i, i)}): every diffuse -> flat {PaintNames[i]}")
                    return name
                return maskResource if (role == SkinMask) else files.resourceOfRole[role]

            bindings = [f"    {reg} = {bound(role)}"
                        for reg, role in regs.items() if (role == SkinMask or role in files.resourceOfRole)]
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
                resource = fixName(f"Resource{NeutralName}")
                if (resource not in neutrals):
                    neutralFile = os.path.join(files.textureFolder, f"{NeutralName}{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
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
            if (bindings):
                cmdList = fixName(f"CommandList{SourceName}{SlotPrefix.capitalize()}{i}Textures")
                condition = " || ".join(f"ps == {PassFilters[ps]}" for ps in SlotPasses[slot])
                appended.append("\n".join([f"[{cmdList}]", f"if {condition}"] + bindings + ["endif", ""]))
                additions.append(("run", cmdList))
            for n, (ps, extraRegs) in enumerate(ExtraPassRegs.get(slot, {}).items()):
                extra = [f"    {reg} = {bound(role)}" for reg, role in extraRegs.items()
                         if (role in files.resourceOfRole)]
                if (not extra):
                    continue
                cmdList = fixName(f"CommandList{SourceName}{SlotPrefix.capitalize()}{i}TexturesPass{n}")
                appended.append("\n".join([f"[{cmdList}]", f"if ps == {PassFilters[ps]}"] + extra + ["endif", ""]))
                additions.append(("run", cmdList))
                extraLegend.append(f"      component {i} on {ps}: {', '.join(f'{r}={v}' for r, v in sorted(extraRegs.items()))}")

            if (additions):
                # right after the shared-resource override (the mod's buffers are bound there): the EARLIEST spot
                # after it, so a component drawn in several ranges has its textures before the FIRST draw. No
                # after-register: `drawindexed` as one is a MUST fact, and behind a `$draw_x` toggle no draw is
                # certain, which parked the additions INSIDE the first toggle (header point 14)
                edits.append(FRB.RegSurroundedAdd(additions,
                                                  beforeRegs = {"run": lambda v: v == OverrideSharedResources},
                                                  latest = False))
            edits.append(FRB.RegRemove(BlendRemapOverrideRegs))
            edits.append(FRB.RegNewVals({"match_first_index": str(c["index_offset"]), "match_index_count": str(c["index_count"]),
                                         "$\\WWMIv1\\vg_offset": str(c["vg_offset"]), "$\\WWMIv1\\vg_count": str(c["vg_count"])}))
            edits += [hashRemap, rename]
            perObj[("", f"{SlotPrefix}{i}")] = edits

        graphEdits: List[object] = [FRB.GraphGroupEdit([perObj])]
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
                                            {"blend": WWMIBlendReplace((0, "", "blend"), resType = "blend", fixFunc = remapWWMIBlend(vgRemap, forcedRemap, source["vertex_count"]))},
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
                    if (role not in files.resourceOfRole):
                        continue
                    resource = files.resourceOfRole[role]
                    if (paint and role.lower().endswith("diffuse")):
                        resource = fixName("ResourcePaintWhite")
                        if (resource not in painted):
                            rel = os.path.join(files.textureFolder, f"PaintWhite{toModName}{FRB.IniKeywords.RemapTex.value}.dds").replace("\\", "/")
                            os.makedirs(os.path.join(ini.folder, files.textureFolder), exist_ok = True)
                            writeSolidDds(os.path.join(ini.folder, rel), (255, 255, 255, 255))
                            appended.append("\n".join([f"[{resource}]", f"filename = {rel}", ""]))
                            painted.add(resource)
                            paintLegend.append(f"      the shared mesh {meshHash}: every diffuse -> flat white")
                    binds.append(f"    {reg} = {resource}")
                if (binds):
                    lines += [f"if ps == {PassFilters[ps]}"] + binds + ["endif"]
                    wrote = True
            if (wrote):
                appended.append("\n".join(lines + [""]))
                print(f"    the shared mesh {meshHash}: {len(byPass)} passes rebound to {SourceName}'s own textures")

        # ---- the shader tags the texture command lists ask about, and the invented mask ----
        for i, (ps, filterIndex) in enumerate(PassFilters.items()):
            appended.append("\n".join([f"[{fixName(f'ShaderOverridePass{i}')}]", f"hash = {ps}", f"filter_index = {filterIndex}", ""]))
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


def main():
    parser = argparse.ArgumentParser(description = f"{SourceName} -> {TargetName}, in place, through the API's parser, fixer and RemapService")
    parser.add_argument("mod", help = f"the mod folder (every {SourceName} .ini under it is fixed in place; DISABLED ones are skipped)")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--hideOrig", action = "store_true", help = f"comment out the mod's own {SourceName} sections, so the mod renders on {TargetName} only")
    parser.add_argument("--undo", action = "store_true", help = "remove a previous fix instead of fixing")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    parser.add_argument("--anchor", choices = sorted(AnchorChains) + ["all"], default = None,
                        help = "pin a chain the skin has no counterpart for to its root bone -- AnchorChains is empty for this pair until an in-game round needs one")
    parser.add_argument("--shapeKeys", choices = ["hide", "leave", "retarget"], default = "hide",
                        help = f"the mod's shape-key sections: 'hide' (default) comments them out like the maintainer's working hand remap, 'leave' keeps them on {SourceName}'s own hashes, 'retarget' copies them onto {TargetName}'s buffer -- see the header, points 8 and 9")
    parser.add_argument("--plan", choices = list(Plans), default = "default",
                        help = "which source component goes through which target slot -- only 'default' exists for this pair (one to one; see the header, point 1)")
    parser.add_argument("--hideTextureOverrides", action = "store_true",
                        help = "comment the mod's own [TextureOverrideTexture] sections out too, as the hand remap does")
    parser.add_argument("--noSplit", action = "store_true",
                        help = "keep every remapped section in mod.ini (default: one section per target draw per file, the rest in <stem>ChisaParfaitRemapFix<n>.ini -- see the header, point 11)")
    parser.add_argument("--paint", action = "store_true",
                        help = "replace every slot's diffuse with a flat colour, one per source component, to see which slot draws which part")
    parser.add_argument("--probe", nargs = "?", const = True, default = False, metavar = "SPEC",
                        help = "bind a flat colour to every register the clothing slots do not plan, to see in one round which one paints a surface; "
                               "SPEC (`3:ps-t4,4:ps-t5`) narrows it to the named registers per source component")
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

    sourceType, targetType = FRB.WWMIBuilder.chisa(), FRB.WWMIBuilder.chisaParfait()
    FRB.CppStrategyOverrides.clear()
    retarget = (args.shapeKeys == "retarget")
    FRB.CppStrategyOverrides.setParser(SourceName, makeParser(sourceType, retarget))
    probe = parseProbeSpec(args.probe) if isinstance(args.probe, str) else args.probe
    FRB.CppStrategyOverrides.setFixer(SourceName, TargetName, makeFixer(sourceType, targetType, remapOverride, args.anchor, retarget, probe, args.plan, args.paint))
    try:
        runService(folder, args)
    finally:
        FRB.CppStrategyOverrides.clear()


if (__name__ == "__main__"):
    main()
