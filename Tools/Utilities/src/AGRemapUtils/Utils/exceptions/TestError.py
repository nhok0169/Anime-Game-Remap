from .Error import Error


# TestError: Exceptions related to some tests
class TestError(Error):
    def __init__(self, testFolder: str, message: str):
        super().__init__(f"{message} for the test at {testFolder}")