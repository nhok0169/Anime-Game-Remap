import sys
import os

from IntegrationTester.src.constants.ConfigKeys import ConfigKeys
from IntegrationTester.src.Config import Config

sys.path.insert(1, Config[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


iniRunPath = FRB.FileService.parseOSPath(os.path.dirname(os.path.abspath(__file__)))
prevLogPath = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../Logs/prevLog"))
logPath = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../Logs"))

fixService = FRB.RemapService(path = iniRunPath, verbose = False, log = prevLogPath, readAllInis = True)
fixService.fix()

# undo the change from the .ini files only
fixedInis = list(fixService.iniStats.fixed)
fixedInis.sort()

for ini in fixedInis:
    iniFile = FRB.IniFile(file = ini, modTypes = FRB.ModTypes.getAll(), defaultModType = FRB.ModTypes.Raiden.value)
    iniFile.removeFix()

fixService.clear()
fixService.log = logPath
fixService.fixOnly = True
fixService.fix()