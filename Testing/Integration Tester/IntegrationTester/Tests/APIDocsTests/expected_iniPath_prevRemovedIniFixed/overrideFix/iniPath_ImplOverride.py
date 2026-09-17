import sys
import os

from IntegrationTester.src.constants.ConfigKeys import ConfigKeys
from IntegrationTester.src.Config import Config

sys.path.insert(1, Config[ConfigKeys.SysPath])
import FixRaidenBoss2 as FRB


iniRunPath = FRB.FileService.parseOSPath(os.path.dirname(os.path.abspath(__file__)))

RegRef = FRB.GIMICharFixerConfig.RegRef
RegValChecks = FRB.GIMICharFixerConfig.RegValChecks
TexEdit = FRB.GIMICharFixerConfig.TexEdit

ORFix = FRB.IniKeywords.ORFixPath.value
NNFix = r"CommandList\global\ORFix\NNFix"
TexFx = r"CommandList\TexFx\TN.0"


def darkenDiffuse(texFile):
    FRB.GammaFilter(FRB.ColourConsts.SRGBGamma.value).transform(texFile)

def makeFaceOpaque(texFile):
    pixels = bytearray(texFile.getPixels())
    pixels[3::4] = bytes([1]) * (len(pixels) // 4)
    texFile.setPixels(bytes(pixels), texFile.width, texFile.height)

def reflectionKeys(obj: str):
    return [f"ResourceRef{obj}Diffuse", f"ResourceRef{obj}LightMap", "$CharacterIB"]


# ==== Override how Kirara is fixed ======

config = FRB.GIMICharFixerConfig()
config.drawnObjs = ["head", "body", "dress"]

# Kirara's body is drawn a second time as part of KiraraBoots' head, while KiraraBoots' own body is hidden
config.objSplits = [("head", ["head"]), ("body", ["body", "head"]), ("dress", ["dress"])]
config.objNewRegVals = [("body", [("ib", "null")])]

# Edit Kirara's body so that her body's skin tone matches with her face
config.texEdits = [TexEdit("head", "ps-t1", "DarkenDiffuse", darkenDiffuse, srcObj = "body"),
                   TexEdit("face", "ps-t0", "OpaqueFaceDiffuse", makeFaceOpaque, toReg = "ps-t1")]

# ---- the rest is the same as the default fix ----
config.objRegRemovals = [("head", reflectionKeys("Head")), ("body", reflectionKeys("Body")), ("dress", reflectionKeys("Dress"))]
config.objRegRemaps = [("dress", [("ps-t1", [RegRef("ps-t0", RegValChecks.isDiffuse)], True),
                                  ("ps-t2", [RegRef("ps-t1", RegValChecks.isLightMap)], True)])]
config.objFixCalls = [("head", [ORFix, TexFx]), ("body", [ORFix, TexFx]), ("dress", [NNFix, TexFx])]

FRB.CppStrategyOverrides.setFixer("Kirara", "KiraraBoots", FRB.makeGIMICharFixer(config))

# ========================================

remapService = FRB.RemapServiceCLI(path = iniRunPath, verbose = False, keepBackups = False)
remapService.fix()

FRB.CppStrategyOverrides.clear()
