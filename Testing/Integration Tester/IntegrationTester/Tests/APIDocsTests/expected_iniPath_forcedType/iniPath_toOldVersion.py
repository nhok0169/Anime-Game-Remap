import sys
import os

from IntegrationTester.src.constants.ConfigKeys import ConfigKeys
from IntegrationTester.src.Config import Config

sys.path.insert(1, Config[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


iniPath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "changeVersionKeqing.ini")
iniFile = FRB.IniFile(iniPath, modTypes = FRB.ModTypes.getAll(), version = 4.0)
iniFile.parse()
iniFile.fix()