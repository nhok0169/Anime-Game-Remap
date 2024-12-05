##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### ExtImports
import os
from collections import defaultdict
from typing import Dict, Set, DefaultDict, Optional
##### EndExtImports

##### LocalImports
from ..tools.DictTools import DictTools
##### EndLocalImports


##### Script
class FileStats():
    """
    Keeps track of different types of files encountered by the program

    Attributes
    ----------
    fixed: Set[:class:`str`]
        The paths to the fixed files

    skipped: Dict[:class:`str`, :class:`Exception`]
        The exceptions to files paths that were skipped due to errors

    skippedByMods: DefaultDict[:class:`str`, Dict[:class:`str`, :class:`Exception`]]
        The exceptions to file paths that were skipped due to errors, grouped for each mod folder paths :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names to the mod folders
        * The inner keys are the names of the file paths
        * The inner values are the errors encountered

    removed: Set[:class:`str`]
        The file paths for files that got removed

    undoed: Set[:class:`str`]
        The file paths for files that got undoed to a previous state before the software was ran

    visitedAtRemoval: Set[:class:`str`]
        The file paths for files that got visited when attempting to remove those files
    """

    def __init__(self):
        self.fixed: Set[str] = set()
        self.skipped: Dict[str, Exception] = {}
        self.skippedByMods: DefaultDict[str, Dict[str, Exception]] = defaultdict(lambda: {})
        self.removed: Set[str] = set()
        self.undoed: Set[str] = set()
        self.visitedAtRemoval: Set[str] = set()

    def clear(self):
        """
        Clears out all saved data about the files
        """

        self.fixed.clear()
        self.skipped.clear()
        self.skippedByMods.clear()
        self.removed.clear()
        self.undoed.clear()
        self.visitedAtRemoval.clear()

    def updateFixed(self, newFixed: Set[str]):
        """
        Adds in new fixed file paths

        Parameters
        ----------
        newFixed: Set[:class:`str`]
            The newly added file paths that got fixed      
        """

        self.fixed.update(newFixed)

    def addFixed(self, filePath: str):
        """
        Adds a new file path to the paths of fixed files

        Parameters
        ----------
        filePath: :class:`str`
            the new file path to a fixed file
        """
        
        self.fixed.add(filePath)

    def updateSkipped(self, newSkipped: Dict[str, Exception], modFolder: Optional[str] = None):
        """
        Adds in new file paths that got skipped due to errors

        Parameters
        ----------
        newSkipped: Dict[:class:`str`, :class:`Exception`]
            The newly skipped file paths due to errors within a particular mod folder

        modFolder: Optional[:class:`str`]
            The folder where the files got skipped. If this argument is ``None``, will read the folder from
            the provided file pahts in `newSkipped`
        """

        if (modFolder is not None): 
            DictTools.update(self.skipped, newSkipped)
            if (newSkipped):
                DictTools.update(self.skippedByMods[modFolder], newSkipped)
            return
        
        for skippedFile in newSkipped:
            self.addSkipped(skippedFile, newSkipped[skippedFile], modFolder = modFolder)

    def addSkipped(self, filePath: str, error: Exception, modFolder: Optional[str] = None):
        """
        Adds a new file path to the paths of skipped files

        Parameters
        ----------
        filePath: :class:`str`
            the new file path that got skipped

        error: :class:`Exception`
            The exception that caused the file to be skipped

        modFolder: Optional[:class:`str`]
            The mod folder that contains the file path. If this argument is ``None``, will read the folder from
            the provided argument in `filePath` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (modFolder is None):
            modFolder = os.path.dirname(filePath)
        
        self.skipped[filePath] = error
        self.skippedByMods[modFolder][filePath] = error

    def updateRemoved(self, newRemoved: Set[str]):
        """
        Adds in new file paths that got removed

        Parameters
        ----------
        newRemoved: Set[:class:`str`]
            The newly added file paths that got removed
        """

        self.removed.update(newRemoved)

    def addRemoved(self, filePath: str):
        """
        Adds in a new file path that got removed

        Parameters
        ----------
        filePath: :class:`str`
            The file path that got removed
        """

        self.removed.add(filePath)

    def updateUndoed(self, newUndoed: Set[str]):
        """
        Adds in new file paths that got contents undoed to a previous state before the software was ran

        Parameters
        ----------
        newRemoved: Set[:class:`str`]
            The newly added file paths that got contents undoed to a previous state before the software was ran
        """

        self.undoed.update(newUndoed)

    def addUndoed(self, filePath: str):
        """
        Adds in a new file path that got undoeds

        Parameters
        ----------
        filePath: :class:`str`
            The file path that got undoed
        """

        self.undoed.add(filePath)

    def updateVisitedAtRemoval(self, newVisitedAtRemoval: Set[str]):
        """
        Adds in new file paths that got visited when the software attempts to remove those files

        Parameters
        ----------
        newVisitedAtRemoved: Set[:class:`str`]
            The newly added file paths that got visited when the software attempts to remove those files
        """

        self.visitedAtRemoval.update(newVisitedAtRemoval)

    def addVisitedAtRemoval(self, filePath: str):
        """
        Adds in a new file path that got visited when the software attempts to remove the file

        Parameters
        ----------
        filePath: :class:`str`
            The file path that got visited when the software attempts to remove the file
        """

        self.visitedAtRemoval.add(filePath)

    def update(self, modFolder: Optional[str] = None, newFixed: Optional[Set[str]] = None, 
               newSkipped: Optional[Dict[str, Exception]] = None, newRemoved: Optional[Set[str]] = None, 
               newUndoed: Optional[Set[str]] = None, newVisitedAtRemoval: Optional[Set[str]] = None):
        """
        Updates the overall file paths in this class

        .. note::
            See :meth:`FileStats.updateFixed`, :meth:`FileStats.updateSkipped` and :meth:`FileStats.updateRemoved` for more details

        Parameters
        ----------
        modFolder: Optional[:class:`str`]
            The folder where the files got skipped :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        newFixed: Optional[Set[:class:`str`]]
            The newly added file paths that got fixed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        newSkipped: Optional[Dict[:class:`str`, :class:`Exception`]]
            The newly skipped file paths due to errors within a particular mod folder :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        newRemoved: Optional[Set[:class:`str`]]
            The newly added file paths that got removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        newUndoed: Optional[Set[:class:`str`]]
             The newly added file paths that got contents undoed to a previous state before the software was ran :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``None``

        newVisitedAtRemoved: Optional[Set[:class:`str`]]
            The newly added file paths that got visited when the software attempts to remove those files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (newFixed is not None):
            self.updateFixed(newFixed)

        if (newSkipped is not None):
            self.updateSkipped(newSkipped, modFolder = modFolder)

        if (newRemoved is not None):
            self.updateRemoved(newRemoved)

        if (newUndoed is not None):
            self.updateUndoed(newUndoed)

        if (newVisitedAtRemoval is not None):
            self.updateVisitedAtRemoval(newVisitedAtRemoval)
##### EndScript