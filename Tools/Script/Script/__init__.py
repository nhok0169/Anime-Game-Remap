##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### LocalImports
from .constants.BuildEnvs import BuildEnv
from .controller.CommandOption import CommandOption
from .controller.CommandBuilder import CommandBuilder
from .apiRefs.BaseApiRef import BaseApiRef
from .apiRefs.PathApiRef import PathApiRef
from .apiRefs.PackageApiRef import PackageApiRef
from .apiRefs.ApiRefBuilder import ApiRefBuilder
from .main import remapMain
##### EndLocalImports


##### Script
__all__ = ["BuildEnv", "CommandOption", "CommandBuilder", "BaseApiRef", "PathApiRef",
           "PackageApiRef", "ApiRefBuilder", "remapMain"]
##### EndScript