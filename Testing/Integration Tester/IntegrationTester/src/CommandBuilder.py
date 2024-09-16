import sys, argparse
from typing import Optional

from .constants.Commands import Commands
from .constants.Paths import UtilitiesPath, SysPaths
from .constants.ConfigKeys import ConfigKeys
from .Config import Config
from .Exceptions import InvalidCommand

sys.path.insert(1, UtilitiesPath)
from Utils.commands.TesterCommandBuilder import TesterCommandBuilder


# ConfigBuilder: Handles the configurations
class ConfigBuilder(TesterCommandBuilder[ConfigKeys]):
    def __init__(self, description: str, argParser: Optional[argparse.ArgumentParser] = None):
        super().__init__(description, Config, SysPaths, ConfigKeys.SysPath, ConfigKeys.System, argParser = argParser)
        

    def _addArguments(self):
        self._argParser.add_argument("command", type=str, help="The command to run the integration tester")
        super()._addArguments()


    def _parseCommand(self):
        commandName = self._args.command
        command = Commands.match(commandName)

        if (command is None):
            raise InvalidCommand(commandName)
        else:
            Config[ConfigKeys.Command] = command

    def parse(self):
        super().parse()
        self._parseCommand()
