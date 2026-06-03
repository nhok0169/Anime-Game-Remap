import sys

from .constants.Paths import UtilitiesPath, APIPyFolderPath
from .constants.CommandOpts import CommandOpts, ShortCommandOpts
from .constants.BuildEnv import BuildEnv
from .exceptions.InvalidBuildEnv import InvalidBuildEnv

sys.path.insert(1, UtilitiesPath)
from Utils.commands.BaseCommandBuilder import BaseCommandBuilder


# ConfigBuilder: Handles the configurations
class CommandBuilder(BaseCommandBuilder):
    def _addArguments(self):
        self._argParser.add_argument(ShortCommandOpts.Env.value, CommandOpts.Env.value, action='store', type=str, help=f"""The environment mode to build the API. Below are the available environment modes:

{BuildEnv.Dev}: Locally builds the API
{BuildEnv.Core}: Builds only the API's C++ core to be integrated into other external C++ projects
{BuildEnv.CIBuildWheel}: Builds the API for production using CIBuildWheel

By default, will use the {BuildEnv.Dev} environment mode""")
        self._argParser.add_argument(ShortCommandOpts.BuildKeep.value, CommandOpts.BuildKeep.value, action='store_true', help="Whether to keep the folder containing all build related files.")
        self._argParser.add_argument(ShortCommandOpts.InstallKeep.value, CommandOpts.InstallKeep.value, action='store_true', help="Whether to keep the previous installed binaries")
        self._argParser.add_argument(ShortCommandOpts.SkipBuild.value, CommandOpts.SkipBuild.value, action='store_true', help="Whether to skip the compiliation and installation of the binaries")
        self._argParser.add_argument(ShortCommandOpts.AddDocs.value, CommandOpts.AddDocs.value, action='store_true', help=f"Whether to add documentations related files to the installed binaries. If the {CommandOpts.Env} argument is set to {BuildEnv.Core}, then this option will be set to false.")
        self._argParser.add_argument(ShortCommandOpts.InstallFolder.value, CommandOpts.InstallFolder.value, action='store', type=str, help=f"The folder location of where to store the installed binaries. By default, the folder is set to {APIPyFolderPath}")

    def _parseEnv(self):
        env = self._args.env
        if (env is None):
            self._args.env = BuildEnv.Dev
            return

        foundEnv = BuildEnv.match(env)

        if (foundEnv is None):
            raise InvalidBuildEnv(env)
        else:
            self._args.env = foundEnv

    def _parseInstallFolder(self):
        installFolder = self._args.installFolder
        if (installFolder is None):
            self._args.installFolder = APIPyFolderPath

    def parse(self):
        self.parseArgs()
        self._parseEnv()
        self._parseInstallFolder()
        return self._args
