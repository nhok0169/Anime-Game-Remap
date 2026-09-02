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

##### CppLocalImports
from ..core import RemapIniRemover
##### EndCppLocalImports

##### LocalImports
from ..model.strategies.iniRemovers.IniRemoveBuilder import IniRemoveBuilder
from ..tools.enums.DeferredEnum import DeferredEnum
##### EndLocalImports


##### Script
class GlobalIniRemoveBuilders(DeferredEnum):
    """
    This class inherits from :class:`DeferredEnum`

    Global builders used by the software to dynamically create modules to remove fixes from the .ini file

    Attributes
    ----------
    RemoveBuilder: :class:`IniRemoveBuilder`
        The builder to dynamically create modules that remove fixes from the .ini file
    """

    RemoveBuilder = (lambda: IniRemoveBuilder(RemapIniRemover), )
    """
    .. note::
        The :class:`RemapIniRemover` built here is the **C++** one (``FixRaidenBoss2.core.RemapIniRemover``),
        not the pure-Python class of the same name. The two find the fix by genuinely different
        rules -- see that class's own documentation -- so what a removal leaves behind differs
    """
##### EndScript