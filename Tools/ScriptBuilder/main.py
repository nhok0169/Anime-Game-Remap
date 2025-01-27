import sys
import copy

from ScriptBuilder.constants.UtilitiesPath import UtilitiesPath
from ScriptBuilder.constants.Paths import APIPath, ScriptFolderPath, APIFullPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.ScriptPartNames import ScriptPartNames
from Utils.constants.FileExts import FileExts
from Utils.constants.toolStats import ScriptStats, ScriptBuilderStats, ScriptBuildStats, ScriptBuilderBuildStats
from Utils.path.ModulePathTools import ModulePathTools
from Utils.constants.StrReplacements import VersionReplace, RanDateTimeReplace, BuildHashReplace, RanHashReplace, BuiltDateTimeReplace
from Utils.constants.BoilerPlate import ScriptPreamble, ScriptPostamble, Credits, ScriptPreambleScriptStats
from Utils.constants.Paths import ModulePath
from Utils.scriptBuilder.ScriptBuilder import ScriptBuilder

sys.path.insert(1, APIPath)
import src.FixRaidenBoss2.main as FRBMain


ScriptName = f"AGRemap{FileExts.Py.value}"


if __name__ == "__main__":
    scriptBuildStats = copy.deepcopy(ScriptBuildStats)
    scriptBuilderBuildStats = copy.deepcopy(ScriptBuilderBuildStats)
    scriptBuildStats.refresh()
    scriptBuilderBuildStats.refresh()

    # get all the modules from the API
    modules = {}
    for name, mod in sys.modules.items():
        if (name.startswith(ModulePath)):
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

    rootModule = ModulePathTools.join(ModulePath, ScriptPartNames.MainFile.value)
    builder = ScriptBuilder(ScriptFolderPath, ScriptName, modules, rootModule, APIFullPath,
                            scriptPreamble = preamble, scriptPostAmble = postamble)
    builder.build()