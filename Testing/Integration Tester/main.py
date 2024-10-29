import unittest, sys
from IntegrationTester.src import TestFileTools, UtilitiesPath, IntegrationTestProgram

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.TesterFailed import TesterFailed


if __name__ == '__main__':
    with open(TestFileTools.IntegrationTestResultsFileName, "w", encoding = TestFileTools.FileEncoding) as f:
        runner = unittest.TextTestRunner(f)
        integrationTester = IntegrationTestProgram(testRunner=runner, exit = False)
        integrationTester.testCommandBuilder.parse()

        from IntegrationTester.Tests import *
        integrationTester.run()

    testResults = TestFileTools.readTestResults()
    print(testResults)

    testScore = testResults.split("\n", 1)[0]
    if (testScore.find("F") > -1 or testScore.find("E") > -1):
        raise TesterFailed("integration")