from .TestError import TestError


# TestResultOutputsNotFound: Exception when the resultant outupts for the test has not been generated yet
class TestResultOutputsNotFound(TestError):
    def __init__(self, testFolder: str, testName: str):
        super().__init__(testFolder, f"Result outputs for '{testName}' has not been generated yet")