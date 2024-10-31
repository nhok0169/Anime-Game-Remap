##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from typing import Type, List, Any, Dict, TYPE_CHECKING, Optional
##### EndExtImports

##### LocalImports
from ....tools.FlyweightBuilder import FlyweightBuilder
from .BaseIniRemover import BaseIniRemover

if (TYPE_CHECKING):
    from ...IniFile import IniFile
##### EndLocalImports


##### Script
class IniRemoveBuilder(FlyweightBuilder[BaseIniRemover]):
    """
    This class inherits from :class:`FlyweightBuilder`

    A class to help dynamically build a :class:`BaseIniRemover`

    Parameters
    ----------
    cls: Type[:class:`BaseIniRemover`]
        The class to construct a :class:`BaseIniRemover` 

    args: Optional[List[Any]]
        The constant arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    kwargs: Optional[Dict[str, Any]]
        The constant keyword arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    cache: :class:`bool`
        Whether to cache the built object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    cache: :class:`bool`
        Whether to cache the built object
    """

    def __init__(self, cls: Type[BaseIniRemover], args: Optional[List[Any]] = None, kwargs: Optional[Dict[str, Any]] = None, cache: bool = True):
        super().__init__(cls, args, kwargs)
        self.cache = cache

    def build(self, iniFile: "IniFile") -> BaseIniRemover:
        """
        Builds the remover

        Parameters
        ----------
        iniFile: :class:`IniFile`
            The .ini file to parse

        Returns
        -------
        :class:`BaseIniRemover`
            The built remover
        """

        id = self._cls.__name__
        result = super().build(args = [iniFile], id = id, cache = self.cache)
        result.iniFile = iniFile
        return result
##### EndScript