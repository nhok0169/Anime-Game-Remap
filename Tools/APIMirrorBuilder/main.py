import sys

from APIMirrorBuilder.constants.Paths import APIFullPath, MirrorFullPath, UtilitiesPath
from APIMirrorBuilder.APIMirrorBuilder import APIMirrorBuilder

sys.path.insert(1, UtilitiesPath)
from Utils.enums.ScriptPartNames import ScriptPartNames
from Utils.ModulePathTools import ModulePathTools
from Utils.constants.Paths import ModulePath


if __name__ == "__main__":
    rootModule = ModulePathTools.join(ModulePath, ScriptPartNames.MainFile.value)

    apiMirrorBuilder = APIMirrorBuilder(APIFullPath, MirrorFullPath, rootModule)
    apiMirrorBuilder.build()