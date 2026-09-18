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
from ..constants.BuildData import ApiPackage, EnvName
from ..constants.BuildEnvs import BuildEnv
from .BaseApiRef import BaseApiRef
from .PackageApiRef import PackageApiRef
from .PathApiRef import PathApiRef
##### EndLocalImports


##### Script
class ApiRefBuilder():
    """
    Picks how this build of the script reaches the API
    """

    @classmethod
    def build(cls) -> BaseApiRef:
        """
        Retrieves the way this build of the script reaches the API

        Returns
        -------
        :class:`BaseApiRef`
            How to reach the API
        """

        if (BuildEnv.match(EnvName) == BuildEnv.Prod):
            return PackageApiRef(ApiPackage)

        return PathApiRef(ApiPackage)
##### EndScript