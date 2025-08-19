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
from ..model.strategies.iniRemovers.IniRemoveBuilder import IniRemoveBuilder
from ..model.strategies.iniRemovers.IniRemover import IniRemover
from ..tools.enums.DeferredEnum import DeferredEnum
##### EndLocalImports


##### Script
class GlobalIniRemoveBuilders(DeferredEnum):
    """
    Global builders used by the software to dynamically create modules to remove fixes from the .ini file

    Attributes
    ----------
    RemoveBuilder: :class:`IniRemoveBuilder`
        The builder to dynamically create modules that remove fixes from the .ini file
    """

    RemoveBuilder = (lambda: IniRemoveBuilder(IniRemover), )
##### EndScript