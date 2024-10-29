import sys
import os

from IntegrationTester.src.constants.ConfigKeys import ConfigKeys
from IntegrationTester.src.Config import Config

sys.path.insert(1, Config[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB



iniRunPath = FRB.FileService.parseOSPath(os.path.dirname(os.path.abspath(__file__)))
logPath = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../Logs"))

fixService = FRB.RemapService(path = iniRunPath, verbose = False, keepBackups = False, log = logPath, readAllInis = True)
fixService.fix()