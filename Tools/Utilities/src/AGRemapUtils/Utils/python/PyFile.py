import re
from typing import Optional, List, Callable, Set
from ordered_set import OrderedSet

from ..constants.script.KeyWordTypes import KeyWordTypes
from ..files.SourceFile import SourceFile
from .Import import Import
from .FromImportSet import FromImportSet
from ..path.ModulePathTools import ModulePathTools


# To guarantee the dependency graph is a DAG and does not contain any directed cycles,
#   we need to ignore imports used only during TYPE_CHECKING
#
# eg. if (TYPE_CHECKING):
#       from ... import ...
FromPattern = re.compile(r"(?<=^from)(\s)*((?!import)[^\s])*")
ImportPattern = re.compile(r"(?<=import).*")


class PyFile(SourceFile):
    def __init__(self, file: str, module: str, creditsLines: Optional[List[str]] = None):
        super().__init__(file, creditsLines = creditsLines)

        self.extFromImports = FromImportSet()
        self.extImport = Import()
        self.localFromImports = FromImportSet()

        self.scriptSections: List[str] = []

        self._module = module


    #clear(): Clears the saved data
    def clear(self):
        self.extFromImports.clear()
        self.extImport.clear()
        self.localFromImports.clear()

        self.scriptSections.clear()
        super().clear()


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


    # readSection(type, section, startInd, endInd): Reads a particular section from a python file
    def readSection(self, type: KeyWordTypes, section: List[str], startInd: int, endInd: int) -> int:
        if (type == KeyWordTypes.Script):
            self.scriptSections.append(section)
        elif (type == KeyWordTypes.LocalImports):
            self.readImports(section, self.localFromImports)
        elif (type == KeyWordTypes.ExtImports):
            self.readImports(section, self.extFromImports, resultImport = self.extImport)
        else:
            endInd = super().readSection(type, section, startInd, endInd)

        return endInd


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
