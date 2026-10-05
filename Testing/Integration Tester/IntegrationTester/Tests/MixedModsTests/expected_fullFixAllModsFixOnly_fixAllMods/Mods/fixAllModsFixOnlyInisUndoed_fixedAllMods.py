import sys
import os

from IntegrationTester.src.constants.ConfigKeys import ConfigKeys
from IntegrationTester.src.Config import Config

sys.path.insert(1, Config[ConfigKeys.SysPath])
import FixRaidenBoss2 as FRB


iniRunPath = FRB.FileService.parseOSPath(os.path.dirname(os.path.abspath(__file__)))
prevLogPath = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../Logs/prevLog"))
logPath = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../Logs"))

fixService = FRB.RemapServiceCLI(path = iniRunPath, verbose = False, log = prevLogPath, readAllInis = True, downloadMode = "disabled")
fixService.fix()

# undo the change from the .ini files only
fixedInis = list(fixService.service.stats.ini.fixed)
fixedInis.sort()

for ini in fixedInis:
    iniFile = FRB.IniFile(file = ini, downloadMode = "disabled")
    iniFile.defaultModTypeIds = [FRB.ModTypes.Raiden.value.modTypeId]
    iniFile.removeFix()

fixService.service.clear()
fixService.log = logPath
fixService.service.fixOnly = True
fixService.fix()
