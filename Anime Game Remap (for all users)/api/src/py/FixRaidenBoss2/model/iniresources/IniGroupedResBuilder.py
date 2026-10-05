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
from typing import Type, List, Dict, Any
##### EndExtImports

##### CppLocalImports
from ...core import IniGroupedResource
##### EndCppLocalImports

##### LocalImports
from ...tools.Builder import Builder
##### EndLocalImports


##### Script
class IniGroupedResBuilder(Builder):
    """
    This class inherits from :class:`Builder`

    A class to build a group of resources for a .ini file

    Parameters
    ----------
    buildCls: Type[:class:`IniGroupedResource`]
        The class for the grouped resource

    args: Optional[List[Any]]
        The constant arguments used to build the grouped resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    kwargs: Optional[Dict[str, Any]]
        The constant keyword arguments used to build the grouped resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """
    
    def __init__(self, buildCls: Type[IniGroupedResource], args: List[str] = None, kwargs: Dict[str, Any] = None):
        super().__init__(buildCls, args, kwargs)
##### EndScript