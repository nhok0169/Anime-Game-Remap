import sys

from .constants.Paths import UtilitiesPath
from .constants.ConfigKeys import ConfigKeys
from .constants.Commands import Commands
from .Config import Config
from .CommandBuilder import ConfigBuilder
from .tools.TestFileTools import TestFileTools

sys.path.insert(1, UtilitiesPath)
from Utils.tests.BaseTestProgram import BaseTestProgram


# IntegrationTestProgram: Framework for running the overall integration tests
class IntegrationTestProgram(BaseTestProgram[ConfigKeys]):
    def __init__(self, *args, **kwargs):
        description = "Integration Tester for Fix Raiden Boss"
        super().__init__(description, cmdBuilderArgs = [description], cmdBuilderCls = ConfigBuilder, *args, **kwargs)


    def runTests(self):
        if (Config[ConfigKeys.Command] == Commands.PrintOutputs):
            TestFileTools.clearTestPrintOutputs()

        super().runTests()