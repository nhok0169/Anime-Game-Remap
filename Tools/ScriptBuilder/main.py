import sys

from ScriptBuilder.constants.Paths import APIPath, ModulePath, ScriptFolderPath, APIFullPath
from ScriptBuilder.constants.ScriptBoilerPlate import ScriptPreamble, ScriptPostamble, Credits
from ScriptBuilder.constants.FileExts import FileExts
from ScriptBuilder.ScriptBuilder import ScriptBuilder
from ScriptBuilder.tools.PathTools import ModulePathTools

sys.path.insert(1, APIPath)
import src.FixRaidenBoss2.main as FRBMain


ScriptName = f"FixRaidenBoss2{FileExts.Py.value}"


if __name__ == "__main__":
    # get all the modules from the API
    modules = {}
    for name, mod in sys.modules.items():
        if (name.startswith(ModulePath)):
            modules[name] = mod

    preamble = f"{ScriptPreamble}\n{Credits}\n"
    postamble = f"\n\n{ScriptPostamble}"

    rootModule = ModulePathTools.join(ModulePath, "main")
    builder = ScriptBuilder(ScriptFolderPath, ScriptName, modules, rootModule, APIFullPath,
                            scriptPreamble = preamble, scriptPostAmble = postamble)
    builder.build()