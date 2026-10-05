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
from .CommandFormatter import CommandFormatter
from .CommandOption import CommandOption
from .enums.CommandOpts import CommandOpts
from .enums.ShortCommandOpts import ShortCommandOpts
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

    Description = "Ports mods from characters onto their skin counterparts"
    """
    The description at the top of the help page, the same as the API's
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

        # only noted here, never acted on: whether help is shown from the API's page or from
        #   this script's own is decided once it is known whether the API is there
        parser.add_argument(ShortCommandOpts.Help.value, CommandOpts.Help.value, action = "store_true")

        return parser.parse_known_args(argv)

    def printHelp(self, note: Optional[str] = None):
        """
        Prints a help page of only this script's own options

        :raw-html:`<br />`

        For when the API, which owns every other option, is not there to print its own

        Parameters
        ----------
        note: Optional[:class:`str`]
            Text to show below the options :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        parser = argparse.ArgumentParser(description = self.Description, epilog = note, formatter_class = CommandFormatter)
        for option in self.options:
            option.add(parser.add_argument)

        parser.print_help()

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