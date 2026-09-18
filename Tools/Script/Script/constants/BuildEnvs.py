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

##### ExtImports
from enum import Enum
from typing import Optional
##### EndExtImports


##### Script
class BuildEnv(Enum):
    """
    The environment a build of this script was made for
    """

    Dev = "dev"
    """
    Reaches the API by a path on this machine, the way the tools in this repo reach each other
    """

    Prod = "prod"
    """
    Downloads the API from `pypi`_ at runtime
    """

    @classmethod
    def match(cls, name: str) -> Optional["BuildEnv"]:
        """
        Retrieves the environment by name

        Parameters
        ----------
        name: :class:`str`
            The name of the environment

        Returns
        -------
        Optional[:class:`BuildEnv`]
            The found environment, or ``None``
        """

        name = name.lower().strip()
        for env in cls:
            if (env.value == name):
                return env

        return None
##### EndScript