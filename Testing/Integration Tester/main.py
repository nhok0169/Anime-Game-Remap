import unittest, sys
from IntegrationTester.src import TestFileTools, UtilitiesPath, IntegrationTestProgram
from IntegrationTester.Tests import *

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.TesterFailed import TesterFailed


def main():
    with open(TestFileTools.IntegrationTestResultsFileName, "w", encoding = TestFileTools.FileEncoding) as f:
        runner = unittest.TextTestRunner(f)
        IntegrationTestProgram(testRunner=runner, exit = False)

    testResults = TestFileTools.readTestResults()
    print(testResults)

    testScore = testResults.split("\n", 1)[0]
    if (testScore.find("F") > -1 or testScore.find("E") > -1):
        raise TesterFailed("integration")


if __name__ == '__main__':
    main()