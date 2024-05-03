import unittest
from Tests import *

UnitTestResultsFileName = 'unitTestResults.txt'
fileEncoding = "utf-8"

# TesterFailed: Exception when there exists at least 1 test that failed
class TesterFailed(Exception):
    def __init__(self) -> None:
        super().__init__(f"There are some unit tests that failed")


if __name__ == '__main__':
    with open(UnitTestResultsFileName, "w", encoding = fileEncoding) as f:
        runner = unittest.TextTestRunner(f)
        unittest.main(testRunner=runner, exit = False)

    with open(UnitTestResultsFileName, "r", encoding = fileEncoding) as f:
        fileTxt = f.read()
        print(fileTxt)

        testScore = fileTxt.split("\n", 1)[0]
        if (testScore.find("F") > -1 or testScore.find("E") > -1):
            raise TesterFailed()

