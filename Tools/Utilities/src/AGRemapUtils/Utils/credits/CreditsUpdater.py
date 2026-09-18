import os
from typing import Dict, List, Optional

from ..constants.BoilerPlate import getCreditsFileLines
from ..constants.FileExts import SrcFileCommentPrefixes
from ..files.SourceFile import SourceFile


# CreditsUpdater: Updates the credits boiler plate for the source files of a project
#
# note: only the text between the '##### Credits' and the '##### EndCredits' comments of a source file
#   gets updated. Source files without those comments are left alone.
class CreditsUpdater():
    def __init__(self, srcFolders: List[str], commentPrefixes: Optional[Dict[str, str]] = None, silent: bool = False):
        if (commentPrefixes is None):
            commentPrefixes = SrcFileCommentPrefixes

        self._srcFolders = srcFolders
        self._commentPrefixes = commentPrefixes
        self._silent = silent

        # the credit lines needed for the different languages, cached by the language's comment prefix
        self._creditsFileLines: Dict[str, List[str]] = {}


    def _print(self, txt: str):
        if (not self._silent):
            print(txt)


    # getCreditsFileLines(commentPrefix): Retrieves the lines of the credits for a language that uses
    #   'commentPrefix' to write a single line comment
    def getCreditsFileLines(self, commentPrefix: str) -> List[str]:
        try:
            return self._creditsFileLines[commentPrefix]
        except KeyError:
            result = getCreditsFileLines(commentPrefix)
            self._creditsFileLines[commentPrefix] = result
            return result


    # updateFile(file): Updates the credits within a single source file
    #
    # Returns whether the file got changed
    def updateFile(self, file: str) -> bool:
        ext = os.path.splitext(file)[1]

        try:
            commentPrefix = self._commentPrefixes[ext]
        except KeyError:
            return False

        srcFile = SourceFile(file, creditsLines = self.getCreditsFileLines(commentPrefix))
        srcFile.read()

        result = srcFile.needsUpdate
        srcFile.update()

        if (result):
            self._print(f"Updated credits at {file}")

        return result


    # updateFolder(srcFolder): Updates the credits for all the source files within 'srcFolder'
    #
    # Returns the number of files that got changed
    def updateFolder(self, srcFolder: str) -> int:
        result = 0
        for root, dirs, files in os.walk(srcFolder):
            for file in files:
                if (self.updateFile(os.path.join(root, file))):
                    result += 1

        return result


    # update(): Updates the credits for all the source files of the project
    #
    # Returns the number of files that got changed
    def update(self) -> int:
        result = 0

        for srcFolder in self._srcFolders:
            if (not os.path.isdir(srcFolder)):
                continue

            result += self.updateFolder(srcFolder)

        self._print(f"Updated the credits for {result} file(s)")
        return result
