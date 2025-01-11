from .TestError import TestError


# TestExpectedOutputsNotFound: Exception when the expected outputs for the test has not been generated yet
class TestExpectedOutputsNotFound(TestError):
    def __init__(self, testFolder: str, testName: str):
        super().__init__(testFolder, f"Expected outputs for '{testName}' has not been generated yet")