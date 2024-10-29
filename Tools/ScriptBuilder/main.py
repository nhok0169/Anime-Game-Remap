import sys

from ScriptBuilder.constants.Paths import APIPath, ModulePath, ScriptFolderPath, APIFullPath, UtilitiesPath
from ScriptBuilder.constants.StrReplacements import ScriptBuildHashReplace, ScriptBuiltDateTimeReplace, ScriptVersionReplace
from ScriptBuilder.constants.StrReplacements import BuilderBuildHashReplace, BuilderRanDateTimeReplace, BuilderVersionReplace
from ScriptBuilder.constants.ScriptBoilerPlate import ScriptPreamble, ScriptPostamble, Credits, ScriptPreambleScriptStats
from ScriptBuilder.constants.FileExts import FileExts
from ScriptBuilder.ScriptBuilder import ScriptBuilder
from ScriptBuilder.tools.PathTools import ModulePathTools
from ScriptBuilder.tools.BuildMetadata import BuildMetadata

sys.path.insert(1, UtilitiesPath)
from Utils.constants.toolStats import ScriptStats, ScriptBuilderStats

sys.path.insert(1, APIPath)
import src.FixRaidenBoss2.main as FRBMain


ScriptName = f"FixRaidenBoss2{FileExts.Py.value}"


if __name__ == "__main__":
    scriptBuildStats = BuildMetadata.fromSoftwareMetadata(ScriptStats)
    scriptBuilderBuildStats = BuildMetadata.fromSoftwareMetadata(ScriptBuilderStats)

    # get all the modules from the API
    modules = {}
    for name, mod in sys.modules.items():
        if (name.startswith(ModulePath)):
            modules[name] = mod

    frontPreamble = ScriptPreamble.replace(BuilderVersionReplace, scriptBuilderBuildStats.version)
    frontPreamble = frontPreamble.replace(BuilderRanDateTimeReplace, scriptBuilderBuildStats.buildDateTime.strftime("%A, %B %d, %Y %I:%M:%S %p %Z"))
    frontPreamble = frontPreamble.replace(BuilderBuildHashReplace, scriptBuilderBuildStats.buildHash)

    backPreamble = ScriptPreambleScriptStats.replace(ScriptVersionReplace, scriptBuildStats.version)
    backPreamble = backPreamble.replace(ScriptBuiltDateTimeReplace, scriptBuildStats.buildDateTime.strftime("%A, %B %d, %Y %I:%M:%S %p %Z"))
    backPreamble = backPreamble.replace(ScriptBuildHashReplace, scriptBuildStats.buildHash)

    preamble = f"{frontPreamble}\n{Credits}"[:-2]
    preamble += f"{backPreamble}\n"
    postamble = f"\n\n{ScriptPostamble}"

    rootModule = ModulePathTools.join(ModulePath, "main")
    builder = ScriptBuilder(ScriptFolderPath, ScriptName, modules, rootModule, APIFullPath,
                            scriptPreamble = preamble, scriptPostAmble = postamble)
    builder.build()