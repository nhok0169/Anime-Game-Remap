import re
from collections import deque
from typing import Optional, List, Callable, Set
from ordered_set import OrderedSet

from ..constants.BoilerPlate import CreditsFileLines
from ..constants.script.ScriptKeyWords import StartKeyWords, EndKeyWords
from ..constants.script.KeyWordTypes import KeyWordTypes
from .KeyWord import KeyWord
from ..exceptions.script.MissingKeyWord import MissingKeyWord
from ..exceptions.script.InvalidKeyWordType import InvalidKeyWordType
from .Import import Import
from .FromImportSet import FromImportSet
from ..path.ModulePathTools import ModulePathTools

from ..FileTools import FileTools


# To guarantee the dependency graph is a DAG and does not contain any directed cycles,
#   we need to ignore imports used only during TYPE_CHECKING
#
# eg. if (TYPE_CHECKING):
#       from ... import ...
FromPattern = re.compile(r"(?<=^from)(\s)*((?!import)[^\s])*")
ImportPattern = re.compile(r"(?<=import).*")


class PyFile():
    def __init__(self, file: str, module: str):
        self.extFromImports = FromImportSet()
        self.extImport = Import()
        self.localFromImports = FromImportSet()

        self.scriptSections: List[str] = []

        self._file = file
        self._module = module
        self.fileLines = []
        self._fileLinesRead = False
        self._needsUpdate = False

    
    #clear(): Clears the saved data
    def clear(self):
        self.extFromImports.clear()
        self.extImport.clear()
        self.localFromImports.clear()

        self.scriptSections.clear()
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
    

    # readImports(section, fromImports, resultImport): Reads all the different imports from 'section'
    def readImports(self, section: List[str], fromImports: FromImportSet, resultImport: Optional[Import] = None, processImportLoc: Optional[Callable[[str], str]] = None):
        for line in section:
            importObjs = re.search(ImportPattern, line)
            if (importObjs is None):
                continue
            
            importObjs = OrderedSet(importObjs.group().split(","))
            fromLoc = re.search(FromPattern, line)

            if (fromLoc is None and resultImport is not None):
                if (processImportLoc is not None):
                    importObjs = OrderedSet(map(lambda obj: processImportLoc(obj), importObjs))

                resultImport.addObjs(importObjs)

            elif (fromLoc is not None):
                fromLoc = fromLoc.group().strip()
                if (processImportLoc is not None):
                    fromLoc = processImportLoc(fromLoc)

                fromImports.addFromStr(fromLoc, importObjs)


    # replaceSection(newLines, startInd, endInd): Replaces a sub-section in the filelines with 'newLines'
    def replaceSection(self, newLines: List[str], startInd: int, endInd: int) -> int:
        self.fileLines = self.fileLines[:startInd] + newLines + self.fileLines[endInd:]
        self._needsUpdate = True
        return startInd + len(newLines)
    
    
    # readSection(type, section, startInd, endInd): Reads a particular section from a python file
    def readSection(self, type: KeyWordTypes, section: List[str], startInd: int, endInd: int) -> int:
        if (type == KeyWordTypes.Script):
            self.scriptSections.append(section)
        elif (type == KeyWordTypes.LocalImports):
            self.readImports(section, self.localFromImports)
        elif (type == KeyWordTypes.ExtImports):
            self.readImports(section, self.extFromImports, resultImport = self.extImport)
        elif (type == KeyWordTypes.Credits):
            endInd = self.replaceSection(CreditsFileLines, startInd, endInd)

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
        

    def getLocalCalledModules(self) -> OrderedSet[str]:
        result = OrderedSet()
        for importObj in self.localFromImports:
            result.add(ModulePathTools.fromRelPath(self._module, importObj.loc))
        return result
    

    def _getImportStr(self, fromImports: FromImportSet, importData: Optional[Import] = None) -> str:
        result = ""
        importDataHasObjects = bool(importData is not None and importData.objects)

        if (importDataHasObjects):
            result += importData.toStr()

        if (importDataHasObjects and fromImports):
            result += "\n"

        result += fromImports.toStr()
        return result
        

    def getExtImportStr(self) -> str:
        return self._getImportStr(self.extFromImports, importData = self.extImport)
    
    def getLocalImportStr(self) -> str:
        return self._getImportStr(self.localFromImports)    

    def getScriptStr(self) -> str:
        result = ""
        for section in self.scriptSections:
            if (result):
                result += "\n\n"

            currentStr = "".join(section)

            # remove the extra newline that precedes the ending keyword for the script section
            if (currentStr.endswith("\n")):
                currentStr = currentStr[:-1]

            result += currentStr

        return result
    

    def getLocalObjects(self) -> Set[str]:
        return self.localFromImports.getAllObjects()
