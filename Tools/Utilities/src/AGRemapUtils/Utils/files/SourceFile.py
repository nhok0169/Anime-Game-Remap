from collections import deque
from typing import Optional, List

from ..constants.BoilerPlate import CreditsFileLines
from ..constants.script.ScriptKeyWords import StartKeyWords, EndKeyWords
from ..constants.script.KeyWordTypes import KeyWordTypes
from .KeyWord import KeyWord
from ..exceptions.script.MissingKeyWord import MissingKeyWord
from ..exceptions.script.InvalidKeyWordType import InvalidKeyWordType

from ..FileTools import FileTools


# SourceFile: A source code file that is divided into sections by the keyword comments of this project
#   (eg. the section between the '##### Credits' and the '##### EndCredits' comments)
#
# note: the keywords are only matched as a substring of a line, so the same keywords work for any language,
#   as long as the keyword is written within a single line comment of that language
#   (eg. '##### Credits' for python and '// ##### Credits' for C++)
class SourceFile():
    def __init__(self, file: str, creditsLines: Optional[List[str]] = None):
        if (creditsLines is None):
            creditsLines = CreditsFileLines

        self.creditsLines = creditsLines

        self._file = file
        self.fileLines = []
        self._fileLinesRead = False
        self._needsUpdate = False

    @property
    def file(self) -> str:
        """
        The path to the file being read
        """

        return self._file

    @property
    def needsUpdate(self) -> bool:
        """
        Whether the file has any pending changes that have not been written back yet
        """

        return self._needsUpdate


    #clear(): Clears the saved data
    def clear(self):
        self.fileLines.clear()
        self._fileLinesRead = False
        self._needsUpdate = False


    # readFileLines(): Reads the lines for the target file
    def readFileLines(self):
        self.fileLines = FileTools.readFile(self._file, lambda filePtr: filePtr.readlines())
        self._fileLinesRead = True


    # writeFileLines(file): Writes back to the file
    def writeFileLines(self):
        txt = "".join(self.fileLines)
        self.write(txt)
        self._needsUpdate = False

    # write(txt): Writes 'txt' into the file
    def write(self, txt: str):
        FileTools.writeFile(self._file, lambda filePtr: filePtr.write(txt))


    def update(self):
        if (self._needsUpdate):
            self.writeFileLines()

    # _readLines(): Decorator to read the lines in the target file
    def _readLines(func):
        def readLinesWrapper(self, *args, **kwargs):
            if (not self._fileLinesRead):
                self.readFileLines()
            return func(self, *args, **kwargs)
        return readLinesWrapper


    # _getKeyWord(line, lineInd): Retrives the corresponding keyword
    def _getKeyWord(self, line: str, lineInd: int) -> Optional[KeyWord]:
        startType = StartKeyWords.getType(line)
        if (startType is not None):
            return KeyWord(startType, lineInd, isStart = True)

        endType = EndKeyWords.getType(line)
        if (endType is not None):
            return KeyWord(endType, lineInd, isStart = False)

        return None


    # replaceSection(newLines, startInd, endInd): Replaces a sub-section in the filelines with 'newLines'
    def replaceSection(self, newLines: List[str], startInd: int, endInd: int) -> int:
        self.fileLines = self.fileLines[:startInd] + newLines + self.fileLines[endInd:]
        self._needsUpdate = True
        return startInd + len(newLines)


    # readSection(type, section, startInd, endInd): Reads a particular section from a source file
    def readSection(self, type: KeyWordTypes, section: List[str], startInd: int, endInd: int) -> int:
        if (type == KeyWordTypes.Credits and section != self.creditsLines):
            endInd = self.replaceSection(self.creditsLines, startInd, endInd)

        return endInd

    @_readLines
    def read(self):
        startKeyStack = deque()
        fileLinesLen = len(self.fileLines)
        i = 0

        while (i < fileLinesLen):
            line = self.fileLines[i]
            keyWord = self._getKeyWord(line, i)

            if (keyWord is None):
                i += 1
                continue

            # opening keyword detected
            if (keyWord.isStart):
                startKeyStack.append(keyWord)
                i += 1
                continue

            # missing opening keyword
            if (not startKeyStack):
                raise MissingKeyWord(keyWord.type, isStart = True)

            startKeyWord = startKeyStack.pop()

            # mismatch types of opening and ending keywords
            if (startKeyWord.type != keyWord.type):
                raise InvalidKeyWordType(startKeyWord.type, keyWord.type)

            sectionStartInd = startKeyWord.lineInd + 1
            sectionEndInd = keyWord.lineInd
            section = self.fileLines[sectionStartInd: sectionEndInd]

            newI = self.readSection(keyWord.type, section, sectionStartInd, sectionEndInd)
            if (i != newI):
                fileLinesLen += (newI - i)
                i = newI

            i += 1

        # missing closing keyword
        if (startKeyStack):
            keyWord = startKeyStack.pop()
            raise MissingKeyWord(keyWord.type, isStart = False)
