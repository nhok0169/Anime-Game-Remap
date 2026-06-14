import sys
import os
import re

from .constants.Paths import UtilitiesPath, APIPyFolderPath, APITopBuildFolderPath, APITopPreInstallFolderPath, APITopPreBuildFolderPath, PathToProject, PreBuildFolder, PreInstallFolder, BuildFolder, RemoveAllFolder
from .constants.CommandOpts import CommandOpts, ShortCommandOpts
from .constants.BuildEnv import BuildEnv

from .exceptions.InvalidBuildEnv import InvalidBuildEnv
from .exceptions.InvalidSuffixName import InvalidSuffixName
from .exceptions.InvaildRemoveFolder import InvalidRemoveFolder

sys.path.insert(1, UtilitiesPath)
from Utils.commands.BaseCommandBuilder import BaseCommandBuilder


# ConfigBuilder: Handles the configurations
class CommandBuilder(BaseCommandBuilder):
    SuffixParseRegex = re.compile(r'[/\s\\]')
    SlashSuffix = re.compile(r'[/\\]')

    def _addArguments(self):
        exampleFolderSuffix = "someName"
        examplePrebuildFolderPath = os.path.join(PathToProject, f"{PreBuildFolder}{exampleFolderSuffix}")
        examplePreinstallFolderPath = os.path.join(PathToProject, f"{PreInstallFolder}{exampleFolderSuffix}")
        exampleBuildFolderPath = os.path.join(PathToProject, f"{BuildFolder}{exampleFolderSuffix}")

        self._argParser.add_argument(ShortCommandOpts.Env.value, CommandOpts.Env.value, action='store', type=str, help=f"""The environment mode to build the API. Below are the available environment modes:

{BuildEnv.Dev}: Locally builds the API
{BuildEnv.Core}: Builds only the API's C++ core to be integrated into other external C++ projects
{BuildEnv.CIBuildWheel}: Builds the API for production using CIBuildWheel

By default, will use the {BuildEnv.Dev} environment mode""")
        self._argParser.add_argument(ShortCommandOpts.PrebuildRemove.value, CommandOpts.PrebuildRemove.value, action="store", type=str, help=f"""The specific external prebuild folder to remove. By default, will not remove any prebuild folders. Below are the following accepted syntax:

{exampleFolderSuffix}/ : Deletes the prebuild folder at {examplePrebuildFolderPath}. Note that the name for the prebuild suffix name folder cannot contain whitespace or slashes.
/ : Deletes the prebuild folder at {APITopPreBuildFolderPath}
* : Deletes all the prebuild folders at {PathToProject}                                
""")
        self._argParser.add_argument(ShortCommandOpts.BuildRemove.value, CommandOpts.BuildRemove.value, action='store', type=str, help=f"""The specific build folder to remove. By default, will not remove any build folders. Below are the following accepted syntax:

{exampleFolderSuffix}/ : Deletes the prebuild folder at {exampleBuildFolderPath}. Note that the name the build folder suffix name cannot contain whitespace or slashes.
/ : Deletes the prebuild folder at {APITopBuildFolderPath}
* : Deletes all the prebuild folders at {PathToProject}                                                                       
""")
        self._argParser.add_argument(ShortCommandOpts.PreInstallRemove.value, CommandOpts.PreInstallRemove.value, action='store', type=str, help=f"""The specific external preinstall folder to remove. By default, will not remove any preinstall folders. Below are the following accepted syntax:
                                     
{exampleFolderSuffix}/ : Deletes the prebuild folder at {examplePreinstallFolderPath}. Note that the name for the prebuild suffix name folder cannot contain whitespace or slashes.
/ : Deletes the prebuild folder at {APITopPreInstallFolderPath}
* : Deletes all the preinstall folders at {PathToProject}     
""")
        self._argParser.add_argument(ShortCommandOpts.InstallKeep.value, CommandOpts.InstallKeep.value, action='store_true', help="Whether to keep the previous installed binaries")
        self._argParser.add_argument(ShortCommandOpts.SkipBuild.value, CommandOpts.SkipBuild.value, action='store_true', help="Whether to skip the compiliation and installation of the binaries")
        self._argParser.add_argument(ShortCommandOpts.AddDocs.value, CommandOpts.AddDocs.value, action='store_true', help=f"Whether to add documentations related files to the installed binaries. If the {CommandOpts.Env} argument is set to {BuildEnv.Core}, then this option will be set to false.")
        self._argParser.add_argument(ShortCommandOpts.InstallFolder.value, CommandOpts.InstallFolder.value, action='store', type=str, help=f"The folder location of where to store the installed binaries. By default, the folder is set to {APIPyFolderPath}")
        self._argParser.add_argument(ShortCommandOpts.MakePreBuild.value, CommandOpts.MakePreBuild.value, action='store_true', help=f"Whether to generate the required files needed to prebuild the external libraries")
        self._argParser.add_argument(ShortCommandOpts.MakePreInstall.value, CommandOpts.MakePreInstall.value, action='store_true', help=f"Whether to install the external libraries")
        self._argParser.add_argument(ShortCommandOpts.PrebuildSuffix.value, CommandOpts.PrebuildSuffix.value, action="store", type= str, help=f"""The suffix name to add to the prebuild folder path. By default, the prebuild folder is specified at: {APITopPreBuildFolderPath}. 
Note that the suffix name cannot contain whitespaces or slashes""")
        self._argParser.add_argument(ShortCommandOpts.PreinstallSuffix.value, CommandOpts.PreinstallSuffix.value, action="store", type=str, help=f"""The suffix name to add to the preinstall folder path. By default, the preinstall folder is specified at: {APITopPreInstallFolderPath}.
Note that the suffix name cannot contain whitepspsace or slashes""")
        self._argParser.add_argument(ShortCommandOpts.BuildSuffix.value, CommandOpts.BuildSuffix.value, action="store", type=str, help=f"""The suffix name to add to the build folder path. By default, the build folder is specified at: {APITopBuildFolderPath}
Note that the suffix name cannot contain whitespace or slashes
""")

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

    def _isValidSuffixName(self, suffixName: str):
        return not bool(self.SuffixParseRegex.search(suffixName))
    
    def _parseSuffixName(self, argName: str):
        suffixName = getattr(self._args, argName)
        if (suffixName is None):
            suffixName = ""
            setattr(self._args, argName, suffixName)

        if (not self._isValidSuffixName(suffixName)):
            raise InvalidSuffixName(argName, suffixName)
        
    def _parsePreBuildSuffix(self):
        self._parseSuffixName("prebuildSuffix")

    def _parsePreInstallSuffix(self):
        self._parseSuffixName("preinstallSuffix")

    def _parseBuildSuffix(self):
        self._parseSuffixName("buildSuffix")

    def _parseRemoveFolder(self, argName: str):
        folder = getattr(self._args, argName)
        if (folder is None):
            return

        folderParts = re.split(self.SlashSuffix, folder, maxsplit = 1)
        folderPartsLen = len(folderParts)
        if (folderPartsLen <= 1 and folder != RemoveAllFolder):
            raise InvalidRemoveFolder(argName, folder)
        else:
            folder = folderParts[0]

        if (folder == RemoveAllFolder):
            return

        isValidFolder = self._isValidSuffixName(folder)
        if (not isValidFolder):
            raise InvalidSuffixName(argName, folder)
        
        setattr(self._args, argName, f"{folder}{os.sep}")

    def _parsePreBuildRemove(self):
        self._parseRemoveFolder("prebuildRemove")

    def _parsePreInstallRemove(self):
        self._parseRemoveFolder("preinstallRemove")

    def _parseBuildRemove(self):
        self._parseRemoveFolder("buildRemove")

    def parse(self):
        self.parseArgs()
        self._parseEnv()
        self._parseInstallFolder()

        self._parsePreBuildSuffix()
        self._parseBuildSuffix()
        self._parsePreInstallSuffix()

        self._parsePreBuildRemove()
        self._parsePreInstallRemove()
        self._parseBuildRemove()

        return self._args
    
