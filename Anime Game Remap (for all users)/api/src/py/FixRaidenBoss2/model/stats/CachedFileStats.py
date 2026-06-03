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
from typing import Dict, Set, Optional
##### EndExtImports

##### LocalImports
from .FileStats import FileStats
##### EndLocalImports


##### Script
class CachedFileStats(FileStats):
    """
    Attributes
    ----------
    fixed: Set[:class:`str`]
        The paths to the files retrieved during a cache miss

    hit: Set[:class:`str`]
        The paths to the files retrieved during a cache hit
    """

    def __init__(self):
        super().__init__()
        self.hit: Set[str] = set()

    def clear(self):
        super().clear()
        self.hit.clear()

    def updateHit(self, newHit: Set[str]):
        """
        Updates the file paths that have a cache hit

        Parameters
        ----------
        newHit: Set[:class:`str`]
            The new file paths that got a hit      
        """

        self.hit.update(newHit)

    def addHit(self, filePath: str):
        """
        Adds a new file path to the paths of cache hit files

        Parameters
        ----------
        filePath: :class:`str`
            the new file path to that was hit
        """
        
        self.hit.add(filePath)

    def update(self, modFolder: Optional[str] = None, newFixed: Optional[Set[str]] = None, 
               newSkipped: Optional[Dict[str, Exception]] = None, newRemoved: Optional[Set[str]] = None, 
               newUndoed: Optional[Set[str]] = None, newVisitedAtRemoval: Optional[Set[str]] = None,
               newHit: Optional[Set[str]] = None):
        super().update(modFolder = modFolder, newFixed = newFixed, newSkipped = newSkipped, 
                       newRemoved = newRemoved, newUndoed = newUndoed, newVisitedAtRemoval = newVisitedAtRemoval)

        if (newHit is not None):
            self.updateHit(newHit)
##### EndScript