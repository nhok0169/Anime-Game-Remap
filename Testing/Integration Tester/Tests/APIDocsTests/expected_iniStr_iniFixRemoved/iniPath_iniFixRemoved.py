import sys
import os

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


iniPath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "PartiallyFixedRaiden.ini")
iniFile = FRB.IniFile(iniPath, modTypes = FRB.ModTypes.getAll())
iniFile.removeFix(keepBackups = False)