import sys

from ..constants.Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.Error import Error


# InvalidSuffixName: Exception when the name of some suffix is not valid
class InvalidSuffixName(Error):
    def __init__(self, argName: str, suffix: str):
        super().__init__(f"For the argument: {argName}, the suffix by the name, {suffix}, has invalid syntax")