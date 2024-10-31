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
from typing import Type, List, Any, Dict, Optional
##### EndExtImports

##### LocalImports
from .BaseIniFixer import BaseIniFixer
from ..iniParsers.BaseIniParser import BaseIniParser
from ....tools.Builder import Builder
##### EndLocalImports


##### Script
class IniFixBuilder(Builder[BaseIniFixer]):
    """
    This class inherits from :class:`Builder`

    Class to dynamically build a :class:`BaseIniFixer` to fix .ini files

    Parameters
    ----------
    cls: Type[:class:`BaseIniFixer`]
        The class to construct a :class:`BaseIniFixer` 

    args: Optional[List[Any]]
        The constant arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    kwargs: Optional[Dict[str, Any]]
        The constant keyword arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, cls: Type[BaseIniFixer], args: Optional[List[Any]] = None, kwargs: Optional[Dict[str, Any]] = None):
        super().__init__(cls, args, kwargs)

    def build(self, parser: BaseIniParser) -> BaseIniFixer:
        """
        Builds the fixer

        Parameters
        ----------
        parser: :class:`BaseIniParser`
            The corresponding parser for the .ini file

        Returns
        -------
        :class:`BaseIniFixer`
            The built fixer
        """

        return super().build(parser)
##### EndScript