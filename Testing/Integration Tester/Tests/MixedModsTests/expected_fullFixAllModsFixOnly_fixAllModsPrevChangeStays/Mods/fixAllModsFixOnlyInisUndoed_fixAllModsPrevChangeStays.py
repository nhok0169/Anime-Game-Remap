import sys
import os

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )/api')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


iniRunPath = FRB.FileService.parseOSPath(os.path.dirname(os.path.abspath(__file__)))
prevLogPath = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../Logs/prevLog"))
logPath = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../Logs"))

fixService = FRB.RemapService(path = iniRunPath, verbose = False, log = prevLogPath, readAllInis = True)
fixService.fix()

# undo the change from the .ini files only
fixedInis = fixService.inisFixed
for ini in fixedInis:
    iniFile = FRB.IniFile(file = ini, modTypes = FRB.ModTypes.getAll(), defaultModType = FRB.ModTypes.Raiden.value)
    iniFile.removeFix()

fixService.clear()
fixService.log = logPath
fixService.fixOnly = True
fixService.fix()