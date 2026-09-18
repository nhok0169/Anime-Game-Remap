#
# ===== mainUpdated =====
#
# The alternative Kirara -> KiraraBoots fix, written for the C++-backed API. main.py beside this file
# is the ORIGINAL version of the same fix, written for the old pure-Python API, and no longer runs.
#
# What it does differently from the default Kirara fix: Kirara's body is drawn a second time as part of
# KiraraBoots' head (with its diffuse darkened so the skin tone matches her face), while KiraraBoots'
# own body is hidden. Everything else is the default 6.1 fix.
#
# Reference: https://gamebanana.com/posts/12191289
#
#   py -3 mainUpdated.py                 fix the Kirara mod in this script's folder
#   py -3 mainUpdated.py <mod folder>    fix a different folder
#   py -3 mainUpdated.py --wsl           from Windows: run this same command under WSL instead -- only
#                                        needed while the Windows .pyd is older than the Linux build
#                                        (AG_REMAP_WSL_DISTRO, default Ubuntu-22.04;
#                                        AG_REMAP_WSL_VENV, default ~/agremap-venv)
#
# The API is imported from the repo at AG_REMAP_REPO, else from its Windows path (translated to
# /mnt/<drive>/... on Linux). It needs a core module built on or after 2026-09-17 -- the
# GIMICharFixerConfig.RegRef / TexEdit(srcObj, toReg) bindings this uses did not exist before.
#

import argparse
import os
import sys

OnWindows = (sys.platform == "win32")


def winToPosix(path: str) -> str:
    """`E:\\a\\b` -> `/mnt/e/a/b` on Linux (WSL's default drive mounts); any other path is returned as is"""
    if (OnWindows or (len(path) < 2) or (path[1] != ":") or (not path[0].isalpha())):
        return path
    return "/mnt/" + path[0].lower() + path[2:].replace("\\", "/")


Repo = os.environ.get("AG_REMAP_REPO") or winToPosix(r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss")
APISrc = os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py")


def importAPI():
    if (not os.path.isdir(APISrc)):
        raise SystemExit(f"the API's package folder is not at {APISrc}; set AG_REMAP_REPO to the repo's path on this OS")

    sys.path.insert(0, APISrc)
    if (hasattr(os, "add_dll_directory")):
        os.add_dll_directory(os.path.join(APISrc, "FixRaidenBoss2"))

    import FixRaidenBoss2 as FRB
    return FRB


def kiraraAltFixer(FRB):
    RegRef = FRB.GIMICharFixerConfig.RegRef
    RegValChecks = FRB.GIMICharFixerConfig.RegValChecks
    TexEdit = FRB.GIMICharFixerConfig.TexEdit

    ORFix = FRB.IniKeywords.ORFixPath.value
    NNFix = r"CommandList\global\ORFix\NNFix"
    TexFx = r"CommandList\TexFx\TN.0"

    # Edit Kirara's body so that her body's skin tone matches with her face
    #
    # -- Notes --:
    # If you do not like how we edit her body, you can play around with her BodyDiffuse.dds or her BodyLightMap.dds
    #   in your favourite image editor (Paint.net, Photoshop, etc...) or you can tweak the code below
    #
    # A filter is handed the texture itself, so it edits the texture in place
    def darkenDiffuse(texFile):
        FRB.GammaFilter(FRB.ColourConsts.SRGBGamma.value).transform(texFile)

    def makeFaceOpaque(texFile):
        pixels = bytearray(texFile.getPixels())
        pixels[3::4] = bytes([1]) * (len(pixels) // 4)
        texFile.setPixels(bytes(pixels), texFile.width, texFile.height)

    def reflectionKeys(obj: str):
        return [f"ResourceRef{obj}Diffuse", f"ResourceRef{obj}LightMap", "$CharacterIB"]

    config = FRB.GIMICharFixerConfig()
    config.drawnObjs = ["head", "body", "dress"]

    # Kirara's body is drawn a second time as part of KiraraBoots' head, while KiraraBoots' own body is hidden
    config.objSplits = [("head", ["head"]), ("body", ["body", "head"]), ("dress", ["dress"])]
    config.objNewRegVals = [("body", [("ib", "null")])]

    # only darken the copy of Kirara's body that is drawn as KiraraBoots' head
    config.texEdits = [TexEdit("head", "ps-t1", "DarkenDiffuse", darkenDiffuse, srcObj = "body"),
                       TexEdit("face", "ps-t0", "OpaqueFaceDiffuse", makeFaceOpaque, toReg = "ps-t1")]

    # ---- the rest is the same as the default fix ----
    config.objRegRemovals = [("head", reflectionKeys("Head")), ("body", reflectionKeys("Body")), ("dress", reflectionKeys("Dress"))]
    config.objRegRemaps = [("dress", [("ps-t1", [RegRef("ps-t0", RegValChecks.isDiffuse)], True),
                                      ("ps-t2", [RegRef("ps-t1", RegValChecks.isLightMap)], True)])]
    config.objFixCalls = [("head", [ORFix, TexFx]), ("body", [ORFix, TexFx]), ("dress", [NNFix, TexFx])]

    return FRB.makeGIMICharFixer(config)


def relaunchUnderWsl(folder: str) -> int:
    import shlex
    import subprocess

    if (not OnWindows):
        raise SystemExit("--wsl is for a Windows shell; this is already Linux")

    distro = os.environ.get("AG_REMAP_WSL_DISTRO", "Ubuntu-22.04")
    venv = os.environ.get("AG_REMAP_WSL_VENV", "~/agremap-venv")
    toPosix = lambda p: "/mnt/" + p[0].lower() + p[2:].replace("\\", "/")
    script = toPosix(os.path.abspath(__file__))
    command = (f"source {venv}/bin/activate && AG_REMAP_REPO={shlex.quote(toPosix(os.path.abspath(Repo)))} "
               f"python {shlex.quote(script)} {shlex.quote(toPosix(os.path.abspath(folder)))}")
    print(f"wsl -d {distro}: {command}")
    return subprocess.call(["wsl", "-d", distro, "--", "bash", "-lc", command])


def main() -> int:
    parser = argparse.ArgumentParser(description = "The alternative Kirara -> KiraraBoots fix, for the C++-backed API")
    parser.add_argument("folder", nargs = "?", default = os.path.dirname(os.path.abspath(__file__)),
                        help = "the mod folder to fix (default: this script's folder)")
    parser.add_argument("--wsl", action = "store_true", help = "from Windows: run this same command under WSL")
    args = parser.parse_args()

    if (args.wsl):
        return relaunchUnderWsl(args.folder)

    folder = winToPosix(args.folder)
    if (not os.path.isdir(folder)):
        raise SystemExit(f"mainUpdated: no such folder: {folder}")

    FRB = importAPI()

    # ==== Override how Kirara is fixed ======
    FRB.CppStrategyOverrides.setFixer("Kirara", "KiraraBoots", kiraraAltFixer(FRB))

    # fix the mod
    try:
        remapService = FRB.RemapServiceCLI(path = folder, types = ["Kirara"], handleExceptions = True)
        remapService.fix()
    finally:
        # go back to the default fix
        FRB.CppStrategyOverrides.clear()

    return 0


if (__name__ == "__main__"):
    sys.exit(main())
