from .TestError import TestError


# TestNoInputsFound: Exception when the test does not have any inputs
class TestNoInputsFound(TestError):
    def __init__(self, testFolder: str):
        super().__init__(testFolder, f"No Inputs found")