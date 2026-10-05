#
# ===== citlaliFromWhisperFix (prototype v1) =====
#
# CitlaliWhisperofStars -> Citlali: a skin of THREE components (Body, Bangs, Eyes) back onto a
# character of one mesh. Built FROM the library -- the whole fix is a GIMIComponentParserConfig
# handed to makeGIMIComponentParser and a GIMIMergeFixerConfig handed to makeGIMIMergeFixer, the
# factories YelanTranquil -> Yelan and BennettAdventure -> Bennett are compiled from, registered
# through CppStrategyOverrides and run by RemapService. So the port is a transcription of the two
# configs below into IniParseData/CitlaliWhisperofStars/ and IniFixData/CitlaliWhisperofStars/.
#
#   py -3 citlaliFromWhisperFix.py <mod folder>                  fix every CitlaliWhisperofStars .ini under the folder
#   py -3 citlaliFromWhisperFix.py <mod folder> --noBands        DIAGNOSTIC: no light map band move
#   py -3 citlaliFromWhisperFix.py <mod folder> --keepBackups    keep the .ini backups the API makes
#   py -3 citlaliFromWhisperFix.py <mod folder> --download disabled   no downloads (for a deterministic A/B)
#   py -3 citlaliFromWhisperFix.py <mod folder> --verbose        attach the API's logger
#
# ---- What the library had to gain first (2026-09-22) ----
#
#   * GIMIMergeFixerConfig::targetLayout. The merge template assumed a PLAIN target (ps-t0 diffuse,
#     ps-t1 light map, NNFix): every source normal map dropped and the rest shifted down. Citlali
#     reads ps-t0 normal map / ps-t1 diffuse / ps-t2 light map under ORFix -- the same layout as the
#     skin's slots -- so TargetLayout::NormalMap passes those through untouched, shifts a plain-layout
#     slot (Body D) UP, and issues ORFix. Default Plain: 1588 files over ten mods, five of them through
#     this template, byte-identical before and after.
#   * GIMIMergeFixerConfig::downloadPrefix is bound (it was only reachable from C++).
#   * GIMIComponentParserConfig::Slot::donorNormalMap: a borrowing slot (the Bangs, the Eyes) fetched
#     only its donor's diffuse and light map, so on a normal-map target it drew with whatever normal
#     map the game had bound.
#   * A merged member is compared with the textures BOUND before it, not the representative's: the
#     Eyes borrow Body A's set and follow Body D, which binds its own, so they drew Body D's atlas.
#   * GIMIMergeFixerConfig::Slot::outline, on RegBottomAdd's new `condition`: a merged member kept out
#     of the target's outline pass is drawn under `if vs != 037730.0` (see NoOutline below).
#   * The classifier counts a multi-component skin's COMPONENT hashes for the skin. HashData files them
#     under CitlaliWhisperofStarsBody / Bangs / Eyes, so the skin owned none, and a mod naming its
#     sections `Citlali_WhisperOfStars...` (no skin keyword matches the underscore) classified as plain
#     Citlali and was fixed as nothing.
#
# ---- What is still written here because the library cannot yet be reached for it ----
#
#   * The light map band move. The compiled fixes use MaterialBandRemapFilter, which has no binding;
#     bandMove() below is the same rule (first band containing the pixel decides, every test reads the
#     ORIGINAL alpha, the gate passes when there is no diffuse to test) through the lightMapEdit hook
#     the binding documents. The port replaces it with MaterialBandRemapFilter::lightMapEdit(Bands).
#
# ---- Which skin slot lands on which of Citlali's objects (measured 2026-09-22) ----
#
# Each slot's vertices, by dominant bone, through the REVERSE vertex-group rows (VGRemapData), against
# the bones only Citlali's own head / only her body are weighted to (both identity mods):
#
#   Body A   16564 verts  head-only 37%  body-only 52%   her skin, cloth and her LONG hair -> body
#   Body B   17566 verts  head-only  4%  body-only 68%   the outfit                          -> body
#   Body C    2827 verts  head-only  2%  body-only 57%   the outfit                          -> body
#   Body D     676 verts  head-only  0%  body-only 61%   a small plain-shader piece           -> body
#   Bangs     3210 verts  head-only 35%  body-only  0%   her fringe                          -> body (!)
#   Eyes       255 verts  head-only  0%  body-only 100%  (Citlali's eyes are in her BODY)     -> body
#
# Body A is mixed, and the template sends a whole slot to one object; the maintainer's call was body,
# the majority. Its hair then draws with Citlali's body shader -- and that is RIGHT for hair, which is
# why the fringe goes there too, against its bones (2026-09-22). Through Citlali's HEAD draw the fringe
# came out a dirty greyish pink with a black edge while the back hair, through her body draw, matched
# the skin: her head uses its own pixel shader (ccc17504), the skin draws all of its hair with the
# normal-map body shader (92544cbc), and in the G-buffer the fringe's shading channel came out ~10%
# darker and bluer-short against the skin's. Not the light map: setting its shadow-threshold green
# 0 -> 128 over the hair band changed nothing in game. Moving the slot to body fixed it. So the
# target object is chosen by SHADER FAMILY first and bones second -- the lesson the WuWa remaps
# already taught. Citlali's head then receives nothing; the whole-ib skip keeps her own head hidden.
#
# ---- The one band move (measured 2026-09-22) ----
#
# Light map alpha, with the share of each band on skin-coloured diffuse:
#
#   skin Body A (= D = Bangs = Eyes, one texture)   255 37% (0%, hair)  78 16%  177-178 24% (92%, SKIN)  0 12%
#   skin Body B (= C)                               0 47%  78 45%  126-127 3% (79%, SKIN)
#   Citlali body                                    0 44%  255 25% (96%, SKIN)  78-80 26%
#   Citlali head                                    0 58%  255 39% (0%, hair)
#
# So both of the skin's skin bands go to Citlali's 255, gated on skin-coloured diffuse. Her hair is
# at 255 on both sides and needs nothing; the cloth at 78 is at 78 on both sides.
#
import argparse
import os
import sys

import numpy as np

Repo = os.environ.get("AG_REMAP_REPO") or r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss"
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")
if (not os.path.isdir(APISrc)):
    raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path")
sys.path.insert(0, APISrc)
if (hasattr(os, "add_dll_directory")):
    os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

import FixRaidenBoss2 as FRB     # noqa: E402

SrcName = "CitlaliWhisperofStars"
DstName = "Citlali"
Prefix = "CitlaliWhisperofStars"

# (component, slot name, match_first_index, layout, lands on, index count, borrows from)
# Index counts are the GAME model's, off the download .ib files (R32, bytes / 4); read only for a
# slot the mod does not carry. Layout from the skin's identity mod: slots on shader 2c157719180b096c
# bind normal map / diffuse / light map at ps-t0/1/2 under ORFix; Body D binds diffuse / light map at
# ps-t0/1 under NNFix. The Bangs and the Eyes are drawn by the GAME with Body A's textures (same
# hashes), so a mod may leave those sections without registers -- hence the donor.
Slots = [("Body", "A", "0", "normal", "body", 60888, ""),
         ("Body", "B", "60888", "normal", "body", 50208, ""),
         ("Body", "C", "111096", "normal", "body", 11820, ""),
         ("Body", "D", "122916", "plain", "body", 1935, ""),
         ("Bangs", "A", "0", "normal", "body", 11328, "Body;A"),
         ("Eyes", "A", "0", "normal", "body", 864, "Body;A")]

# Slots kept OUT of Citlali's outline pass (2026-09-22). The skin outlines its dress -- Body B and C
# -- with outline shaders nothing else of hers uses (077848d1 / 57c73a33 in the 6.7 dump; Body A, D
# and the Bangs share Citlali's 67fd126e). Through Citlali's outline shader the hull of the skirt's
# far panel covered the lining in black, and showed as a dark edge by the hair at the shoulders: a
# frame dump of the identity mod had the outline draw write 4890 pixels outside the silhouette and
# turn 7141 inside it near-black, and an in-game test with just these two gated cleared both.
NoOutline = {("Body", "B"), ("Body", "C")}

# The GAME model's vertex count (download Blend.buf bytes / 32) and texcoord stride (Texcoord.buf
# bytes / vertices), per component.
Components = {"Body": (37631, 20), "Bangs": (3210, 12), "Eyes": (255, 12)}

# (lowest band, highest band, becomes, gated on skin-coloured diffuse)
Bands = [(177, 178, 255, True),
         (126, 127, 255, True)]


def skinColoured(rgb: np.ndarray) -> np.ndarray:
    """MaterialBandRemapFilter::skinColoured, over an array of pixels"""
    r, g, b = (rgb[..., i].astype(np.int32) for i in range(3))
    return (r >= 96) & (r >= g) & (g >= b) & ((r - b) >= 16) & ((r - b) <= 140)


def pixels(tex: "FRB.CppTextureFile") -> np.ndarray:
    return np.frombuffer(bytes(tex.getPixels()), dtype=np.uint8).reshape(tex.height, tex.width, 4).copy()


def bandMove(diffusePath: str):
    """The lightMapEdit hook: MaterialBandRemapFilter's rule, which has no binding yet"""
    def edit(lightMap: "FRB.CppTextureFile") -> None:
        px = pixels(lightMap)
        was = px[..., 3].copy()

        skin = None
        if (diffusePath and os.path.isfile(diffusePath)):
            diffuse = FRB.CppTextureFile(diffusePath)
            diffuse.open()
            if (diffuse.hasImage):
                d = pixels(diffuse)
                ys = np.arange(px.shape[0]) * d.shape[0] // px.shape[0]
                xs = np.arange(px.shape[1]) * d.shape[1] // px.shape[1]
                skin = skinColoured(d[ys][:, xs])

        decided = np.zeros(was.shape, dtype = bool)
        for low, high, to, gated in Bands:
            inBand = (was >= low) & (was <= high) & ~decided
            passes = inBand if (not gated or skin is None) else (inBand & skin)
            px[..., 3][passes] = to
            decided |= inBand

        lightMap.setPixels(px.tobytes(), lightMap.width, lightMap.height)
    return edit


def parserConfig() -> "FRB.GIMIComponentParserConfig":
    config = FRB.GIMIComponentParserConfig()
    config.modTypeId = FRB.ModTypeId.CitlaliWhisperofStars
    config.downloadCharFolder = "CitlaliWhisperofStars"
    config.downloadVersionFolder = "6_7"
    config.downloadPrefix = Prefix

    components = []
    for name, (vertexCount, texcoordStride) in Components.items():
        component = FRB.GIMIComponentParserConfig.Component()
        component.name = name
        component.modTypeName = SrcName + name
        component.texcoordStride = texcoordStride
        component.vertexCount = vertexCount

        slots = []
        for comp, slotName, index, layout, _, _, donor in Slots:
            if (comp != name):
                continue
            slot = FRB.GIMIComponentParserConfig.Slot()
            slot.name = slotName
            slot.index = index
            if (layout == "normal"):
                slot.normalMapReg, slot.diffuseReg, slot.lightMapReg = "ps-t0", "ps-t1", "ps-t2"
            else:
                slot.normalMapReg, slot.diffuseReg, slot.lightMapReg = "", "ps-t0", "ps-t1"
            slot.noTextures = bool(donor)
            slot.textureDonor = donor
            # Citlali reads normal maps, so a borrowing slot needs the donor's too.
            slot.donorNormalMap = bool(donor)
            slots.append(slot)
        component.slots = slots
        components.append(component)

    config.components = components
    return config


def fixerConfig(bands: bool) -> "FRB.GIMIMergeFixerConfig":
    config = FRB.GIMIMergeFixerConfig()

    components = []
    for name, (vertexCount, _) in Components.items():
        component = FRB.GIMIMergeFixerConfig.Component()
        component.name = name
        component.vertexCount = vertexCount

        slots = []
        for comp, slotName, index, layout, to, indexCount, donor in Slots:
            if (comp != name):
                continue
            slot = FRB.GIMIMergeFixerConfig.Slot()
            slot.name = slotName
            slot.index = index
            slot.to = to
            slot.normalMap = (layout == "normal")
            slot.borrowFrom = donor
            slot.indexCount = indexCount
            slot.outline = (comp, slotName) not in NoOutline
            slots.append(slot)
        component.slots = slots
        components.append(component)

    config.components = components
    config.targetObjs = ["head", "body"]
    config.targetLayout = FRB.GIMIMergeFixerConfig.TargetLayout.NormalMap

    # AND EVERY CARRIED BINDING ONTO THE REGISTER ITS NAME SAYS.
    #
    # The skin's mods bind (light map, normal map, diffuse) at ps-t0/1/2 -- the order the GAME's own
    # draw of ib f117984b binds, read off a frame dump -- while ORFix reads the normal map out of
    # ps-t0, the diffuse out of ps-t1 and the light map out of ps-t2. Carried across unchanged and
    # then handed to ORFix, every role comes out of the wrong slot: the shoes, eyes and sleeping
    # mask of one mod were wrong in game while every slot the fix had downloaded and bound itself
    # was right. Citlali's OWN mods are written in ORFix's order, which is why the forward
    # direction never needed this.
    config.texRegsByName = True
    config.downloadPrefix = Prefix

    # Citlali's identity mod binds her face diffuse (9fb78572) at ps-t1, the GI 6.x layout.
    config.faceReg = "ps-t1"
    config.lightMapEdit = bandMove if bands else None

    # NOT BC7: the alpha being edited is a band selector.
    config.compressTextures = False
    return config


def main():
    parser = argparse.ArgumentParser(description = "CitlaliWhisperofStars -> Citlali, as a merge-template config")
    parser.add_argument("mod", help = "the mod folder (every CitlaliWhisperofStars .ini under it is fixed)")
    parser.add_argument("--noBands", action = "store_true", help = "DIAGNOSTIC: no light map band move")
    parser.add_argument("--keepBackups", action = "store_true", help = "keep the .ini backups the API makes")
    parser.add_argument("--verbose", action = "store_true", help = "attach the API's logger")
    # The A/B against the compiled fix needs BOTH sides given the same download setting, or a
    # download that happens not to land makes the two differ over a file neither config chose --
    # which it did, on a different mod each run (2026-09-22).
    parser.add_argument("--download", default = None,
                        help = "the API's downloadMode, eg. `disabled` -- omit for the API's own default")
    args = parser.parse_args()

    FRB.CppStrategyOverrides.clear()
    FRB.CppStrategyOverrides.setParser(SrcName, FRB.makeGIMIComponentParser(parserConfig()))
    FRB.CppStrategyOverrides.setFixer(SrcName, DstName, FRB.makeGIMIMergeFixer(fixerConfig(not args.noBands)))

    folder = os.path.abspath(args.mod)
    try:
        # NOT forcedModTypeIds, which the older prototypes pass: it makes EVERY .ini under the folder
        # the skin's, and a mod's Face.ini -- shared face meshes only, none of the skin's hashes --
        # then had a whole downloaded body invented for it, drawn over the mod's own. The classifier
        # decides, as it will for the compiled fix.
        service = FRB.RemapService(path = folder, keepBackups = args.keepBackups,
                                   **({"downloadMode": args.download} if args.download else {}),
                                   logger = FRB.Logger() if args.verbose else None)
        service.fix()

        stats = service.stats
        print(f"\n.ini fixed: {len(stats.ini.fixed)}, skipped: {len(stats.ini.skipped)}")
        for path, error in stats.ini.skipped.items():
            print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
        for label in ("blend", "position", "texcoord", "buf", "texEdit", "texAdd", "download"):
            s = getattr(stats, label)
            print(f"{label}: fixed {len(s.fixed)}, skipped {len(s.skipped)}")
            for path in sorted(s.fixed):
                print(f"  {os.path.relpath(path, folder)}")
            for path, error in s.skipped.items():
                print(f"  SKIPPED {os.path.relpath(path, folder)}: {error}")
    finally:
        FRB.CppStrategyOverrides.clear()


if (__name__ == "__main__"):
    main()
