import sys

from ..constants.Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.Error import Error


# ConfigError: Exception for the configurations of the tester
class ConfigError(Error):
    pass


# InvalidCommand: Exception when an invalid command is entered
class InvalidCommand(ConfigError):
    def __init__(self, commandName: str):
        super().__init__(f"Unable to find command by the name '{commandName}'")