import sys

from ..constants.Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.Error import Error


# InvalidRemoveFolder: Exception when the syntax for the remove folder is invalid
class InvalidRemoveFolder(Error):
    def __init__(self, argName: str, folder: str):
        super().__init__(f"For the argument: {argName}, the folder specified by, {folder}, is invalid")