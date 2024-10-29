import sys
import os

from IntegrationTester.src.constants.ConfigKeys import ConfigKeys
from IntegrationTester.src.Config import Config

sys.path.insert(1, Config[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


srcBlend = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../LittleEiBlend.buf"))
dstBlend = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "RemappedBytes.buf"))

inputBytes = None
with open(srcBlend, "rb") as f:
    inputBytes = f.read()

fixedBytes = FRB.Mod.blendCorrection(inputBytes, FRB.ModTypes.Raiden.value, "RaidenBoss")
with open(dstBlend, "wb") as f:
    f.write(fixedBytes)