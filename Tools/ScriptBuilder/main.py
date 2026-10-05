import os
import sys
import copy

from ScriptBuilder.constants.UtilitiesPath import UtilitiesPath
from ScriptBuilder.constants.Paths import ScriptFolderPath, ScriptToolPath, ScriptModuleFolder, APIFullPath
from ScriptBuilder.CommandBuilder import CommandBuilder

sys.path.insert(1, UtilitiesPath)
from Utils.enums.ScriptPartNames import ScriptPartNames
from Utils.constants.FileExts import FileExts
from Utils.constants.toolStats import ScriptStats, ScriptBuildStats, ScriptBuilderBuildStats, APIStats
from Utils.path.ModulePathTools import ModulePathTools
from Utils.constants.StrReplacements import VersionReplace, RanDateTimeReplace, BuildHashReplace, RanHashReplace, BuiltDateTimeReplace, BuildEnvReplace, APIRelPathReplace, APIPackageReplace
from Utils.constants.BoilerPlate import ScriptPreamble, ScriptPostamble, Credits, ScriptPreambleScriptStats
from Utils.constants.Paths import ScriptModulePath
from Utils.scriptBuilder.ScriptBuilder import ScriptBuilder

sys.path.insert(1, ScriptToolPath)
import Script.main as ScriptMain


ScriptName = f"AGRemap{FileExts.Py.value}"


# getApiRelPath(): Retrieves where the API's package sits, relative to the compiled script
#
# note: written with '/' rather than os.sep, so a script compiled on one OS still finds the API on
#   another
def getApiRelPath() -> str:
    result = os.path.relpath(os.path.abspath(APIFullPath), os.path.abspath(ScriptFolderPath))
    return result.replace(os.sep, "/")


if __name__ == "__main__":
    command = CommandBuilder()
    args = command.parse()

    scriptBuildStats = copy.deepcopy(ScriptBuildStats)
    scriptBuilderBuildStats = copy.deepcopy(ScriptBuilderBuildStats)
    scriptBuildStats.refresh()
    scriptBuilderBuildStats.refresh()

    # get all the modules from the script's own source
    modules = {}
    for name, mod in sys.modules.items():
        if (name == ScriptModulePath or name.startswith(f"{ScriptModulePath}.")):
            modules[name] = mod

    frontPreamble = ScriptPreamble.replace(VersionReplace, scriptBuilderBuildStats.version)
    frontPreamble = frontPreamble.replace(RanDateTimeReplace, scriptBuilderBuildStats.getFormattedDatetime())
    frontPreamble = frontPreamble.replace(RanHashReplace, scriptBuilderBuildStats.buildHash)

    backPreamble = ScriptPreambleScriptStats.replace(VersionReplace, scriptBuildStats.version)
    backPreamble = backPreamble.replace(BuiltDateTimeReplace, scriptBuildStats.getFormattedDatetime())
    backPreamble = backPreamble.replace(BuildHashReplace, scriptBuildStats.buildHash)

    preamble = f"{frontPreamble}\n{Credits}"[:-2]
    preamble += f"{backPreamble}\n"
    postamble = f"\n\n{ScriptPostamble}"

    # what this build of the script needs to know about reaching the API
    replacements = {
        BuildEnvReplace: args.env.value,
        APIRelPathReplace: getApiRelPath(),
        APIPackageReplace: APIStats.name
    }

    print(f"Building the script for the '{args.env.value}' environment")

    rootModule = ModulePathTools.join(ScriptModulePath, ScriptPartNames.MainFile.value)
    builder = ScriptBuilder(ScriptFolderPath, ScriptName, modules, rootModule, ScriptModuleFolder,
                            scriptPreamble = preamble, scriptPostAmble = postamble, replacements = replacements)
    builder.build()
