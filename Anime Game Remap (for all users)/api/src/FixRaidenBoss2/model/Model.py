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
from typing import Optional, TYPE_CHECKING
##### EndExtImports

##### LocalImports
if (TYPE_CHECKING):
    from ..view.Logger import Logger
##### EndLocalImports


##### Script
# our model objects in MVC
class Model():
    """
    Generic class used for any data models in the fix

    Parameters
    ----------
    logger: Optional[:class:`Logger`]
        The logger used to print messages to the console :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    logger: Optional[:class:`Logger`]
        The logger used to print messages to the console
    """
    def __init__(self, logger: Optional["Logger"] = None):
        self.logger = logger

    def print(self, funcName: str, *args, **kwargs):
        """
        Prints out output

        Parameters
        ----------
        funcName: :class:`str`
            The name of the function in the logger for printing out the output

        \*args: List[:class:`str`]
            Arguments to pass to the function in the logger

        \*\*kwargs: Dict[:class:`str`, Any]
            Keyword arguments to pass to the function in the logger

        Returns
        -------
        :class:`Any`
            The return value from running the corresponding function in the logger 
        """

        if (self.logger is not None):
            func = getattr(self.logger, funcName)
            return func(*args, **kwargs)
##### EndScript