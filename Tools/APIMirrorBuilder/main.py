import sys
import copy

from APIMirrorBuilder.constants.Paths import APIFullPath, MirrorFullPath, UtilitiesPath
from APIMirrorBuilder.APIMirrorBuilder import APIMirrorBuilder

sys.path.insert(1, UtilitiesPath)
from Utils.enums.ScriptPartNames import ScriptPartNames
from Utils.path.ModulePathTools import ModulePathTools
from Utils.constants.Paths import ModulePath
from Utils.constants.toolStats import APIStats, APIMirrorStats


if __name__ == "__main__":
    rootModule = ModulePathTools.join(ModulePath, ScriptPartNames.MainFile.value)

    apiMirrorBuilder = APIMirrorBuilder(APIFullPath, MirrorFullPath, rootModule, copy.deepcopy(APIStats), copy.deepcopy(APIMirrorStats))
    apiMirrorBuilder.build()