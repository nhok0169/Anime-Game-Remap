import sys
import unittest

from UnitTester.src.UnitTestProgram import UnitTestProgram
from UnitTester.src.constants.Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.TesterFailed import TesterFailed


UnitTestResultsFileName = 'unitTestResults.txt'
fileEncoding = "utf-8"


if __name__ == '__main__':
    with open(UnitTestResultsFileName, "w", encoding = fileEncoding) as f:
        runner = unittest.TextTestRunner(f)
        unitTester = UnitTestProgram(testRunner=runner, exit = False)
        unitTester.testCommandBuilder.parse()

        from UnitTester.Tests import *
        unitTester.run()

    with open(UnitTestResultsFileName, "r", encoding = fileEncoding) as f:
        fileTxt = f.read()
        print(fileTxt)

        testScore = fileTxt.split("\n", 1)[0]
        if (testScore.find("F") > -1 or testScore.find("E") > -1):
            raise TesterFailed("unit")


