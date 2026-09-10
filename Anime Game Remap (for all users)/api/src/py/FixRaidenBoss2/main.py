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
import os
from typing import Any, Callable, Optional
##### EndExtImports

##### LocalImports
from .controller.CommandBuilder import CommandBuilder
from .constants.GameTypes import GameTypes
from .constants.ModTypes import ModTypes
from .remapServiceCLI import RemapServiceCLI
##### EndLocalImports

##### Script
def remapMain(commandSetup: Optional[Callable[[CommandBuilder], Any]] = None):
    """
    Runs the CLI

    Parameters
    ----------
    commandSetup: Optional[Callable[[:class:`CommandBuilder`], Any]]
        A function run against the command *before* the command line is parsed, for a caller that
        needs options of its own alongside this CLI's :raw-html:`<br />` :raw-html:`<br />`

        The script build uses this to add its package options, so they appear in the same
        ``--help`` as the options below :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    command = CommandBuilder()
    command.addEpilogs([ModTypes.getHelpStr(), GameTypes.getHelpStr()])

    if (commandSetup is not None):
        commandSetup(command)

    args = command.parse()
    readAllInis = args.all
    defaultType = args.defaultType
    forcedType = args.forceType

    remapService = RemapServiceCLI(path = args.src, keepBackups = not args.deleteBackup, fixOnly = args.fixOnly, hideOrig = args.hideOriginal,
                                   undoOnly = args.undo, readAllInis = readAllInis, types = args.types, defaultType = defaultType, forcedType = forcedType,
                                   log = args.log, verbose = True, handleExceptions = True, remappedTypes = args.remappedTypes,
                                   version = args.version, proxy = args.proxy, downloadMode = args.download,
                                   gameTypes = args.game, compressTextures = args.compressTextures)
    remapService.fix()
    remapService.logger.waitExit()


# Main Driver Code
if __name__ == "__main__":
    remapMain()
##### EndScript