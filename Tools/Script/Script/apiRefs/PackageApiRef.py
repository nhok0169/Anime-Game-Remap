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
import importlib
import importlib.metadata
import importlib.util
import re
import pip._internal as pip
from typing import Any, List, Optional, Tuple
##### EndExtImports

##### LocalImports
from ..controller.CommandOption import CommandOption
from ..controller.enums.CommandOpts import CommandOpts
from ..controller.enums.ShortCommandOpts import ShortCommandOpts
from ..constants.Links import DocsCommandOptsUrl
from .BaseApiRef import BaseApiRef
##### EndLocalImports


##### Script
# the last release of the API that was the old pure-Python library; an installed version at or below
#   this one is treated as not installed
LastPurePythonApiVersion = (4, 6, 4)


class PackageApiRef(BaseApiRef):
    """
    This class inherits from :class:`BaseApiRef`

    :raw-html:`<br />`

    Downloads the API from `pypi`_ at runtime, the same shape as the API's own ``PackageManager``:
    try to import it, and only reach for `pip`_ when it is not there

    Parameters
    ----------
    package: :class:`str`
        The API, both as a module to import and as an installation name
    """

    def getOptions(self) -> List[CommandOption]:
        return [
            CommandOption((ShortCommandOpts.DisableUpdate.value, CommandOpts.DisableUpdate.value),
                          {"action": "store_true",
                           "help": f"Skips updating '{self.package}' to its latest version before running. With this option, the package is only downloaded when it is not already installed."}),

            CommandOption((ShortCommandOpts.PreRelease.value, CommandOpts.PreRelease.value),
                          {"action": "store_true",
                           "help": f"Also considers prereleases of '{self.package}' when downloading it"})
        ]

    def getInstallOptions(self, args: Any) -> List[str]:
        """
        The options handed to `pip`_ for the download

        Parameters
        ----------
        args: `Namespace`_
            This script's own options, already read

        Returns
        -------
        List[:class:`str`]
            The options
        """

        result = ["install", "-U"]

        if (getattr(args, "preRelease", False)):
            result.append("--pre")

        return result + [self.package]

    def getInstalledVersion(self) -> Optional[Tuple[int, ...]]:
        """
        The version of the API installed by `pip`_

        Returns
        -------
        Optional[Tuple[:class:`int`, ...]]
            The numeric parts of the version (eg. ``(4, 6, 4)``), or ``None`` if the API was not
            installed by `pip`_ (eg. it is only reachable through ``PYTHONPATH``) or its version
            cannot be read
        """

        try:
            version = importlib.metadata.version(self.package)
        except importlib.metadata.PackageNotFoundError:
            return None

        match = re.match(r"\d+(\.\d+)*", version)
        if (match is None):
            return None

        return tuple(int(part) for part in match.group(0).split("."))

    def isInstalled(self) -> bool:
        """
        Whether a usable version of the API can already be imported

        :raw-html:`<br />`

        Versions up to and including ``4.6.4`` are the old pure-Python library, which this script
        cannot run, so they count as not installed

        Returns
        -------
        :class:`bool`
            Whether the API is available
        """

        # find_spec rather than an import: importing an old version here would leave it cached in
        #   sys.modules, and the script would keep running it after pip has upgraded the package
        if (importlib.util.find_spec(self.package) is None):
            return False

        version = self.getInstalledVersion()
        return version is None or version > LastPurePythonApiVersion

    def canShowFullHelp(self, args: Any) -> bool:
        # printing help never downloads or updates the API, so the full help needs it already installed
        return self.isInstalled()

    def getHelpNote(self) -> str:
        return f"""NOTE:
'{self.package}', the library that does the remapping, is not installed on this computer yet
(or only an old version that this script cannot run is), so only this script's own options are
shown above. It is downloaded the first time you run this script without {CommandOpts.Help.value}.

The full list of options is at:
{DocsCommandOptsUrl}"""

    def prepare(self, args: Any):
        # printing help is not a remap session, so it does not update an installed API either
        skipUpdate = getattr(args, "disableUpdate", False) or getattr(args, "help", False)

        if (skipUpdate and self.isInstalled()):
            return

        print(f"Getting the latest version of '{self.package}' (pass {CommandOpts.DisableUpdate.value} to skip this)...")
        pip.main(self.getInstallOptions(args))

        # a package installed during this run is not on any import path that was cached before it
        #   arrived, so the caches have to be dropped before the import below can find it
        importlib.invalidate_caches()
##### EndScript