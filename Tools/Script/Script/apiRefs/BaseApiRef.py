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
import importlib
from types import ModuleType
from typing import Any, List
##### EndExtImports

##### LocalImports
from ..controller.CommandOption import CommandOption
##### EndLocalImports


##### Script
class BaseApiRef():
    """
    Base class for how a build of this script reaches the API

    :raw-html:`<br />`

    .. note::
        The script is only a thin front for the API now. It cannot simply *contain* the API any more:
        the API is a C++/Cython/python project, and a single ``.py`` file cannot carry a compiled
        extension module.

    Parameters
    ----------
    package: :class:`str`
        The API, both as a module to import and as an installation name

    Attributes
    ----------
    package: :class:`str`
        The API, both as a module to import and as an installation name
    """

    def __init__(self, package: str):
        self.package = package

    def getOptions(self) -> List[CommandOption]:
        """
        The command line options this way of reaching the API needs

        Returns
        -------
        List[:class:`CommandOption`]
            The options, which are empty for a reference that needs none
        """

        return []

    def prepare(self, args: Any):
        """
        Makes the API importable

        Parameters
        ----------
        args: `Namespace`_
            This script's own options, already read
        """

        pass

    def load(self, args: Any) -> ModuleType:
        """
        Retrieves the API

        Parameters
        ----------
        args: `Namespace`_
            This script's own options, already read

        Returns
        -------
        `Module`_
            The API's module
        """

        self.prepare(args)
        return importlib.import_module(self.package)
##### EndScript