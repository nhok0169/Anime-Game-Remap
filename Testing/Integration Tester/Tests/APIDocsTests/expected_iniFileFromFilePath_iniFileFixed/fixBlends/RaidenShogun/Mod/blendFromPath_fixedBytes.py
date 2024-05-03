import sys
import os

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


srcBlend = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../LittleEiBlend.buf"))
dstBlend = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "RemappedBytes.buf"))

inputBytes = None
with open(srcBlend, "rb") as f:
    inputBytes = f.read()

fixedBytes = FRB.Mod.blendCorrection(inputBytes, FRB.ModTypes.Raiden.value)
with open(dstBlend, "wb") as f:
    f.write(fixedBytes)