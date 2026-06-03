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
from .FileStats import FileStats
from .CachedFileStats import CachedFileStats
##### EndLocalImports


##### Script
class RemapStats():
    """
    The file stats for the overall remap process at :class:`RemapService`

    Attributes
    ----------
    blend: :class:`FileStats`
        Stats about whether some Blend.buf files got fixed/skipped/removed

        .. note::
            * removed Blend.buf files refer to RemapBlend.buf files that were previously made by this software on a previous run

    position: :class:`FileStats`
        Stats about whether some Position.buf files got fixed/skipped/removed

        .. note::
            * removed Position.buf files refer to RemapPosition.buf files that were previously made by this software on a previous run

    ini: :class:`FileStats`
        Stats about whether some .ini files got fixed/skipped/undoed

        .. note::
            * The skipped .ini files may or may not have been previously fixed. A path to some .ini file in this attribute **DOES NOT** imply that the .ini file previously had a fix

    mod: :class:`FileStats`
        Stats about whether a mod has been fixed/skipped

    texAdd: :class:`FileStats`
        Stats about whether an existing texture file has been editted/removed

    texEdit: :class:`FileStats`
        Stats about whether some brand new texture file created by this software has been created/removed

    download: :class:`CachedFileStats`
        Stats about whether some downloaded mod files have been recently downloaded/removed
    """

    def __init__(self):
        self.blend = FileStats()
        self.position = FileStats()
        self.ini = FileStats()
        self.mod = FileStats()
        self.texEdit = FileStats()
        self.texAdd = FileStats()
        self.download = CachedFileStats()

    def clear(self):
        """
        Clears all the stats for the remap process
        """

        self.blend.clear()
        self.position.clear()
        self.ini.clear()
        self.mod.clear()
        self.texEdit.clear()
        self.texAdd.clear()
        self.download.clear()
##### EndScript