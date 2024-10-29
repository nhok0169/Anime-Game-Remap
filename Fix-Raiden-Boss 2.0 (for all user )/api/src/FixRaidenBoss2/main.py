##### Credits

# ===== Raiden Boss Fix =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### LocalImports
from .controller.CommandBuilder import CommandBuilder
from .constants.ModTypes import ModTypes
from .RemapService import RemapService
##### EndLocalImports

##### Script
def main():
    command = CommandBuilder()
    command.addEpilog(ModTypes.getHelpStr())

    args = command.parse()
    readAllInis = args.all
    defaultType = args.defaultType

    remapService = RemapService(path = args.src, keepBackups = not args.deleteBackup, fixOnly = args.fixOnly, 
                                undoOnly = args.undo, readAllInis = readAllInis, types = args.types, defaultType = defaultType,
                                log = args.log, verbose = True, handleExceptions = True, remappedTypes = args.remappedTypes,
                                version = args.version)
    remapService.fix()
    remapService.logger.waitExit()


# Main Driver Code
if __name__ == "__main__":
    main()
##### EndScript