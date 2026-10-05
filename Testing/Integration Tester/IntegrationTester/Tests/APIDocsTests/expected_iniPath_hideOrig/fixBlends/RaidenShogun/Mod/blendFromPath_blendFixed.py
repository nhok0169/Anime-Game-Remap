import sys
import os

from IntegrationTester.src.constants.ConfigKeys import ConfigKeys
from IntegrationTester.src.Config import Config

sys.path.insert(1, Config[ConfigKeys.SysPath])
import FixRaidenBoss2 as FRB


srcBlend = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../LittleEiBlend.buf"))
dstBlend = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "PuppetEiGotRemapped.buf"))
vgRemap = FRB.ModTypes.Raiden.value.getVGRemap("RaidenBoss")
FRB.BlendFile(srcBlend).remap(vgRemap, fixedBlendFile = dstBlend)
