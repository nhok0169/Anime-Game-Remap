import sys
import os

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


iniRunPath = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../"))

fixService = FRB.BossFixService(path = iniRunPath, verbose = False, keepBackups = False)
fixService.fix()

fixService.clear()
fixService.undoOnly = True
fixService.fix()