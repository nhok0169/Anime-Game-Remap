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
##### EndExtImports

##### LocalImports
from .controller.CommandBuilder import CommandBuilder
from .constants.ModTypes import ModTypes
from .remapService import RemapService
##### EndLocalImports

##### Script
def remapMain():

    command = CommandBuilder()
    command.addEpilog(ModTypes.getHelpStr())

    args = command.parse()
    readAllInis = args.all
    defaultType = args.defaultType
    forcedType = args.forceType

    remapService = RemapService(path = args.src, keepBackups = not args.deleteBackup, fixOnly = args.fixOnly, hideOrig = args.hideOriginal,
                                undoOnly = args.undo, readAllInis = readAllInis, types = args.types, defaultType = defaultType, forcedType = forcedType,
                                log = args.log, verbose = True, handleExceptions = True, remappedTypes = args.remappedTypes,
                                version = args.version, proxy = args.proxy, downloadMode = args.download)
    remapService.fix()
    remapService.logger.waitExit()


# Main Driver Code
if __name__ == "__main__":
    remapMain()
##### EndScript