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
from typing import Type, Any, List, Dict, Optional, Generic
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import BuildCls
##### EndLocalImports


##### Script
class Builder(Generic[BuildCls]):
    """
    Class to dynamically create a new object

    Parameters
    ----------
    buildCls: Type[T]
        The class for the objects to be built from

    args: Optional[List[Any]]
        The constant arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    kwargs: Optional[Dict[str, Any]]
        The constant keyword arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    _buildCls: Type[T]
        The class for the objects to be built from

    _args: List[Any]
        The constant arguments used to build the object

    _kwargs: Dict[str, Any]
        The constant keyword arguments used to build the object
    """
    def __init__(self, buildCls: Type[BuildCls], args: Optional[List[Any]] = None, kwargs: Optional[Dict[str, Any]] = None):
        self._buildCls = buildCls

        if (args is None):
            args = []
        self._args = args

        if (kwargs is None):
            kwargs = {}
        self._kwargs = kwargs

    def build(self, *args, **kwargs) -> BuildCls:
        """
        Creates the object

        Parameters
        ----------
        *args
            arguments to build the object

        **kwargs
            keyword arguments to build the object

        Returns
        -------
        T
            The built objects
        """

        return self._buildCls(*args, *self._args, **kwargs, **self._kwargs)
##### EndScript