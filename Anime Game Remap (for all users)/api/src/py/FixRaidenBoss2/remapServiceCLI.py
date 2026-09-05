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
from .core import CppRemapServiceCLI
##### EndCppLocalImports

##### LocalImports
from .constants.FileExt import FileExt
from .constants.FilePrefixes import FilePrefixes
from .constants.FileTypes import FileTypes
from .controller.enums.CommandOpts import CommandOpts
from .exceptions.ConflictingOptions import ConflictingOptions
##### EndLocalImports


##### Script
class RemapServiceCLI(CppRemapServiceCLI):
    """
    This class inherits from :class:`CppRemapServiceCLI`

    The command line front end for a remap :raw-html:`<br />` :raw-html:`<br />`

    Everything about *running* a remap and reporting it -- the folder walk, the summary, the log
    file -- is its C++ half's. What this class adds is the one thing C++ deliberately has no
    business knowing: the **names of the command line options**, which belong to the argument
    parser (:class:`CommandOpts`, see ``controller/``) rather than to the model :raw-html:`<br />`
    :raw-html:`<br />`

    .. note::
        Argument parsing itself stays outside this class, in ``main.py``, exactly as it did for the
        pure-Python :class:`RemapService` this replaced. This class takes values, not an ``argv``
    """

    def fix(self):
        """
        Runs the remap, then writes the log file

        :raw-html:`<br />`

        Adds the one check its C++ half cannot make: two options that contradict each other. Naming
        a command line option is the argument parser's business and the parser is not in C++, which
        is the same reason :meth:`addTips` lives here
        """

        service = self.service

        # Undoing and not-undoing at once. Left to the C++ half this is not an error at all -- it
        # skips the removal AND skips the fix, and reports a run that did nothing -- which reads as
        # "your mods were fine" rather than "you asked for two contradictory things".
        if (service.fixOnly and service.undoOnly):
            error = ConflictingOptions([CommandOpts.FixOnly.value, CommandOpts.Revert.value])

            # Raised or logged on the same terms as any other failure, matching what the C++ half
            # does with a conversion error -- 'handleExceptions' is the caller saying which it wants.
            if (not service.handleExceptions):
                self.createLog()
                raise error

            self.logger.handleException(error)
            self.createLog()
            return

        super().fix()

    def addTips(self):
        """
        Prints out any useful tips for the user to know

        :raw-html:`<br />`

        Called by :meth:`fix` after the remap and **before** the log file is written, so the tips
        land in the log too -- and only after a run that finished with nothing skipped
        """

        service = self.service
        logger = self.logger

        # Every tip below suggests re-running with a different option, so a run that already did the
        # thing has nothing to suggest. With all of them satisfied there is no heading either --
        # an empty "Tips" box is worse than none.
        if (service.undoOnly and not service.keepBackups):
            return

        logger.includePrefix = False

        logger.split()
        logger.openHeading("Tips", sideLen = 10)

        if (service.keepBackups):
            logger.bulletPoint(f'Hate deleting the "{FilePrefixes.BackupFilePrefix.value}" {FileExt.Ini.value}/{FileExt.Txt.value} files yourself after running this script? (cuz I know I do!) Run this script again (on CMD) using the {CommandOpts.DeleteBackup.value} option')

        if (not service.undoOnly):
            logger.bulletPoint(f"Want to undo this script's fix? Run this script again (on CMD) using the {CommandOpts.Revert.value} option")

        if (not service.hideOrig):
            logger.bulletPoint(f"Want the mod to only show on the remapped character and not the original character? Run this script again (on CMD) using the {CommandOpts.HideOriginal.value} options")

        if (not service.readAllInis):
            logger.bulletPoint(f"Were your {FileTypes.Ini.value}s not read? Run this script again (on CMD) using the {CommandOpts.All.value} option")

        logger.space()
        logger.log("For more info on command options, run this script (on CMD) using the --help option")
        logger.closeHeading()

        logger.includePrefix = True
##### EndScript
