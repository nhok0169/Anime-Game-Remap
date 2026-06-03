import sys

from ..constants.Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.Error import Error


# InvalidSystem: Exception when the name of some build environment is not found
class InvalidBuildEnv(Error):
    def __init__(self, searchedEnv: str):
        super().__init__(f"Unable to find the buile environment by the name, '{searchedEnv}'")