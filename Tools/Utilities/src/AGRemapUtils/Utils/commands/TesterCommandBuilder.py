import argparse
from typing import Dict, Any, TypeVar, Generic, Optional

from .BaseCommandBuilder import BaseCommandBuilder
from ..exceptions.InvalidSystem import InvalidSystem
from ..enums.SysEnum import SysEnum


ConfigKey = TypeVar("ConfigKey")


# TesterCommandBuilder: Class for building the command for some tester
class TesterCommandBuilder(BaseCommandBuilder, Generic[ConfigKey]):
    def __init__(self, description: str, configs: Dict[ConfigKey, Any], sysPaths: Dict[SysEnum, str], 
                 sysPathConfigKey: ConfigKey, sysConfigKey: ConfigKey, 
                 argParser: Optional[argparse.ArgumentParser] = None, argParserKwargs: Optional[Dict[str, Any]] = None):

        if (argParserKwargs is None):
            argParserKwargs = {}

        super().__init__(argParser = argParser, argParserKwargs = {"description": description, **argParserKwargs})
        self._configs = configs
        self._sysPaths = sysPaths
        self._sysPathConfigKey = sysPathConfigKey
        self._sysConfigKey = sysConfigKey

    def _addArguments(self):
        sysArgs = SysEnum.getAll()
        self._argParser.add_argument("-s", "--system", action='store', type=str, help=f"The system to perform the tests on. The available systems are: {sysArgs}")

    def _parseSysType(self):
        sysName = self._args.system
        if (sysName is None):
            return

        foundSys = SysEnum.match(sysName)
        if (foundSys is None):
            raise InvalidSystem(sysName)
        else:
            self._configs[self._sysConfigKey] = foundSys
            self._configs[self._sysPathConfigKey] = self._sysPaths[foundSys]


    def parse(self):
        self.parseArgs()
        self._parseSysType()
