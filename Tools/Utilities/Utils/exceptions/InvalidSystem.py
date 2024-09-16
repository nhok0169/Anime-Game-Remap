from .Error import Error


# InvalidSystem: Exception when the name of the system some system is not found
class InvalidSystem(Error):
    def __init__(self, searchedSys: str):
        super().__init__(f"Unable to find the system by the name, '{searchedSys}'")