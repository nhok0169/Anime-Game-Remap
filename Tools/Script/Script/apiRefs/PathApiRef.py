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
import os
import sys
from typing import Any, List, Optional
##### EndExtImports

##### LocalImports
from ..constants.BuildData import ApiRelPath
from .BaseApiRef import BaseApiRef
##### EndLocalImports


##### Script
class PathApiRef(BaseApiRef):
    """
    This class inherits from :class:`BaseApiRef`

    :raw-html:`<br />`

    Reaches the API by a path on this machine, the way every tool in this repo reaches the others

    Parameters
    ----------
    package: :class:`str`
        The API, both as a module to import and as an installation name

    apiPath: Optional[:class:`str`]
        The folder holding the API's package :raw-html:`<br />` :raw-html:`<br />`

        When this is ``None``, the path is worked out from where this script sits, using the
        relative path the ScriptBuilder compiled in :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, package: str, apiPath: Optional[str] = None):
        super().__init__(package)
        self._apiPath = apiPath

    def getApiPath(self) -> str:
        """
        The folder holding the API's package

        Returns
        -------
        :class:`str`
            The folder
        """

        if (self._apiPath is not None):
            return self._apiPath

        scriptFolder = os.path.dirname(os.path.abspath(__file__))
        return os.path.abspath(os.path.join(scriptFolder, *ApiRelPath.split("/")))

    def prepare(self, args: Any):
        apiPath = self.getApiPath()

        if (not os.path.isdir(apiPath)):
            raise FileNotFoundError(f"This is a 'dev' build of the script, which expects the API's source at: {apiPath}")

        sys.path.insert(1, apiPath)
##### EndScript