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
from .IniClsActionArgs import IniClsActionArgs
##### EndLocalImports


##### Script
class IniClsAction():
    """
    Base class to handle any post-processing action to run after the :class:`IniClassifier` transitions 
    to a new state when a keyword is found in a line

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: x(args)

            Runs the action ``x`` on the passed in arguments from :class:`IniClsActionArgs`
    """

    def __call__(self, args: IniClsActionArgs):
        pass
##### EndScript