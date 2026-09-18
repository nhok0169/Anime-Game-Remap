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
import argparse
from typing import Any, List, Optional, Tuple
##### EndExtImports

##### LocalImports
from .CommandOption import CommandOption
##### EndLocalImports


##### Script
class CommandBuilder():
    """
    Handles the options this script adds on top of the API's own

    Parameters
    ----------
    options: Optional[List[:class:`CommandOption`]]
        The options to add :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, options: Optional[List[CommandOption]] = None):
        self.options = [] if (options is None) else options

    def preParse(self, argv: Optional[List[str]] = None) -> Tuple[Any, List[str]]:
        """
        Reads only this script's own options, leaving everything else for the API

        :raw-html:`<br />`

        .. note::
            Abbreviations are off. With them on, argparse would happily match a shortened form of an
            *API* option onto one of ours and swallow it.

        Parameters
        ----------
        argv: Optional[List[:class:`str`]]
            The command line arguments to read :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Tuple[`Namespace`_, List[:class:`str`]]
            The options that were read, and the arguments left over
        """

        parser = argparse.ArgumentParser(add_help = False, allow_abbrev = False)
        for option in self.options:
            option.add(parser.add_argument)

        return parser.parse_known_args(argv)

    def addTo(self, command: Any):
        """
        Registers this script's options into the API's command, so they appear in the same
        ``--help`` as the API's own options

        Parameters
        ----------
        command: :class:`CommandBuilder`
            The API's command builder
        """

        for option in self.options:
            option.add(command.addArgument)
##### EndScript