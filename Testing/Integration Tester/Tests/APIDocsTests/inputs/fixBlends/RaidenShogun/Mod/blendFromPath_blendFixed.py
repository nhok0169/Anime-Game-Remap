import sys
import os

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )/api')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB

srcBlend = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../LittleEiBlend.buf"))
dstBlend = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "PuppetEiGotRemapped.buf"))
FRB.Mod.blendCorrection(srcBlend, FRB.ModTypes.Raiden.value, "RaidenBoss", fixedBlendFile = dstBlend)