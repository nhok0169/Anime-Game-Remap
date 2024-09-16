import argparse
from typing import Dict, Optional, Any

from .CommandFormatter import CommandFormatter


# BaseCommandBuilder: Base class for building commands
class BaseCommandBuilder():
    def __init__(self, argParser: Optional[argparse.ArgumentParser] = None, argParserKwargs: Optional[Dict[str, Any]] = None):
        if (argParserKwargs is None):
            argParserKwargs = {}

        if (argParser is None):
            self._argParser = argparse.ArgumentParser(formatter_class=CommandFormatter, **argParserKwargs)
        else:
            self._argParser = argParser

        self._addArguments()
        self._args = argparse.Namespace()

    @property
    def args(self):
        return self._args
    
    @property
    def argParser(self):
        return self._argParser

    def parseArgs(self) -> argparse.Namespace:
        self._args = self._argParser.parse_args()
        return self._args
    
    def addArgument(self, *args, **kwargs):
        self._argParser.add_argument(*args, **kwargs)

    def _addArguments(self):
        """
        Adds all the arguments definitions necessary for the command
        """
        pass

    def addEpilog(self, epilog: str):
        self._argParser.epilog = epilog