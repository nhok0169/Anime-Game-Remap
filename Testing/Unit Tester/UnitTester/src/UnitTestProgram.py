import sys

from .constants.Paths import UtilitiesPath, SysPaths
from .constants.ConfigKeys import ConfigKeys
from .Config import Configs

sys.path.insert(1, UtilitiesPath)
from Utils.tests.BaseTestProgram import BaseTestProgram


# UnitTestProgram: Framework for running the overall unit tests
class UnitTestProgram(BaseTestProgram[ConfigKeys]):
    def __init__(self, *args, **kwargs):
        description = "Unit Tester for Fix Raiden Boss"
        super().__init__(description, cmdBuilderArgs = [description, Configs, SysPaths, ConfigKeys.SysPath, ConfigKeys.System], *args, **kwargs)
