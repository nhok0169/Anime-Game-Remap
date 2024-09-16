from typing import Dict, Any, TypeVar, Optional, Generic, List
import unittest

from ..commands.TesterCommandBuilder import TesterCommandBuilder
from ..enums.SysEnum import SysEnum

ConfigKey = TypeVar("ConfigKey")


# BaseTestProgram: Class used as the framework for running tests
class BaseTestProgram(unittest.TestProgram, Generic[ConfigKey]):
    def __init__(self, description: str, cmdBuilderArgs: Optional[List[Any]] = None, cmdBuilderKwargs: Optional[Dict[str, Any]] = None, cmdBuilderCls = TesterCommandBuilder, *args, **kwargs):
        self._cmdBuilderCls = cmdBuilderCls
        self._cmdBuilderKwargs = cmdBuilderKwargs if (cmdBuilderKwargs is not None) else {}
        self._cmdBuilderArgs = cmdBuilderArgs if (cmdBuilderArgs is not None) else []

        self.description = description
        self._testCommandBuilder: Optional[TesterCommandBuilder] = None
        super().__init__(*args, **kwargs)


    def _initArgParsers(self):
        self._main_parser = self._getParentArgParser()
        self._testCommandBuilder = self._cmdBuilderCls(*self._cmdBuilderArgs, argParser = self._main_parser, **self._cmdBuilderKwargs)

        self._main_parser = self._testCommandBuilder.argParser
        self._main_parser = self._getMainArgParser(self._main_parser)
        self._main_parser.description = self.description
        self._testCommandBuilder._argParser = self._main_parser

        self._discovery_parser = self._getParentArgParser()
        self._discovery_parser = self._getDiscoveryArgParser(self._discovery_parser)


    def parseArgs(self, argv):
        super().parseArgs(argv)
        self._testCommandBuilder.parse()
