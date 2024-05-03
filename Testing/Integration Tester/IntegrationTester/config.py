from .commands import Commands
from .Exceptions import InvalidCommand
import argparse

# Configurations for the program are used as a global variable to
#   allow passing of configuration to the integration tests
Config = {"command": Commands.RunSuite}


# ConfigManager: Handles the configurations
class ConfigManager():
    def setup(self, argParser: argparse.ArgumentParser):
        argParser.add_argument("command", type=str, help="The command to run the integration tester")

    def parse(self, args: argparse.Namespace):
        commandName = args.command
        command = Commands.get(commandName)

        if (command is None):
            raise InvalidCommand(commandName)
        else:
            Config["command"] = command

