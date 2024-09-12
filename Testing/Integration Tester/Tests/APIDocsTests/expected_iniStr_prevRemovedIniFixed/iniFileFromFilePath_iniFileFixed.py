import sys
import os

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )/api')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB

iniPath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "CuteLittleRaiden.ini")
iniFile = FRB.IniFile(iniPath, modTypes = FRB.ModTypes.getAll())
iniFile.parse()
iniFile.fix()