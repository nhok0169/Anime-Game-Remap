from .baseException import Error

# TestError: Exceptions related to the integration tests
class TestError(Error):
    def __init__(self, testFolder: str, message: str):
        super().__init__(f"{message} for the test at {testFolder}")


# NoInputsFound: Exception when the test does not have any inputs
class NoInputsFound(TestError):
    def __init__(self, testFolder: str):
        super().__init__(testFolder, f"No Inputs found")


# ExpectedOutputsNotFound: Exception when the expected outputs for the test has not been generated yet
class ExpectedOutputsNotFound(TestError):
    def __init__(self, testFolder: str, testName: str):
        super().__init__(testFolder, f"Expected outputs for '{testName}' has not been generated yet")


# ResultOutputsNotFound: Exception when the resultant outupts for the test has not been generated yet
class ResultOutputsNotFound(TestError):
    def __init__(self, testFolder: str, testName: str):
        super().__init__(testFolder, f"Result outputs for '{testName}' has not been generated yet")


# TesterFailed: Exception when there exists at least 1 test that failed
class TesterFailed(Error):
    def __init__(self):
        super().__init__(f"There are some integration tests that failed!")
