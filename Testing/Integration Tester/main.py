import argparse
import unittest
from IntegrationTester import ConfigManager, Commands, Config, FileTools
from IntegrationTester.Exceptions import TesterFailed
from Tests import *


# IntegrationTesterFormatter: Text formatting for the help page of the command 
class IntegrationTesterFormatter(argparse.MetavarTypeHelpFormatter, argparse.RawTextHelpFormatter):
    pass


ArgParser = argparse.ArgumentParser(description='Integration Tester for Fix Raiden Boss', formatter_class=IntegrationTesterFormatter)


def main():
    configManager = ConfigManager()
    configManager.setup(ArgParser)

    args = ArgParser.parse_args()
    configManager.parse(args)

    if (Config["command"] == Commands.PrintOutputs):
        FileTools.clearTestPrintOutputs()

    with open(FileTools.IntegrationTestResultsFileName, "w", encoding = FileTools.FileEncoding) as f:
        runner = unittest.TextTestRunner(f)
        unittest.main(argv=['first-arg-is-ignored'], testRunner=runner, exit = False)

    testResults = FileTools.readTestResults()
    print(testResults)

    testScore = testResults.split("\n", 1)[0]
    if (testScore.find("F") > -1 or testScore.find("E") > -1):
        raise TesterFailed()


if __name__ == '__main__':
    main()