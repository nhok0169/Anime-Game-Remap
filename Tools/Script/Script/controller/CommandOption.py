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
from typing import Any, Callable, Dict, Optional, Tuple
##### EndExtImports


##### Script
class CommandOption():
    """
    A single command line option, declared once and registered in more than one place

    :raw-html:`<br />`

    .. note::
        This script has to know some of its options *before* the API exists to parse them -- whether
        to update the API's package is the option that decides whether the package gets downloaded at
        all. So each option is registered twice: once into a throwaway parser that reads it early,
        and once into the API's own command, so that it still shows up in ``--help`` next to every
        API option instead of being rejected as unrecognised.

    Parameters
    ----------
    args: Tuple[Any, ...]
        The positional arguments to hand to `argparse's add_argument`_

    kwargs: Optional[Dict[:class:`str`, Any]]
        The keyword arguments to hand to `argparse's add_argument`_
    """

    def __init__(self, args: Tuple[Any, ...], kwargs: Optional[Dict[str, Any]] = None):
        self.args = args
        self.kwargs = {} if (kwargs is None) else kwargs

    def add(self, addFunc: Callable[..., Any]):
        """
        Registers this option

        Parameters
        ----------
        addFunc: Callable[..., Any]
            The function that adds an option, taking the same arguments as
            `argparse's add_argument`_
        """

        addFunc(*self.args, **self.kwargs)
##### EndScript