import os
from ordered_set import OrderedSet
from typing import Dict, Set, DefaultDict, Callable, Optional
from types import ModuleType

from ..FileTools import FileTools
from ..Algo import Algo
from ..DFSData import DFSData
from ..path.FilePathTools import FilePathTools
from ..constants.FileExts import FileExts
from ..constants.BoilerPlate import ScriptChangeDir
from ..enums.ScriptPartNames import ScriptPartNames
from ..python.PyFile import PyFile
from ..python.Import import Import
from ..python.FromImportSet import FromImportSet
from ..python.FromImport import FromImport
from ..path.ModulePathTools import ModulePathTools
from ..path.PyPathTools import PyPathTools


# ScriptBuilder: Class to build that transforms a library into a single script
class ScriptBuilder():
    def __init__(self, scriptFolder: str, scriptBasePath: str, modules: Dict[str, ModuleType], rootModule: str,
                 moduleFolder: str, scriptPreamble: str = "", scriptPostAmble: str = ""):
        self._scriptFolder = scriptFolder
        self._scriptBasePath = scriptBasePath
        self._scriptModule = ModulePathTools.currentPath(FilePathTools.toModulePath(self._scriptBasePath))
        self._scriptPath = os.path.join(scriptFolder, scriptBasePath)
        self._scriptInitPath = PyPathTools.getInitPath(scriptFolder)
        self._scriptMainPath = PyPathTools.getMainPath(scriptFolder)

        self._modules = modules
        self._rootModule = rootModule

        self._module = ModulePathTools.dirname(rootModule)
        self._moduleFolder = moduleFolder
        self._moduleInitPath = PyPathTools.getInitPath(moduleFolder)
        self._moduleMainPath = PyPathTools.getMainPath(moduleFolder)
        
        self._scriptPreamble = scriptPreamble
        self._scriptPostamble = scriptPostAmble

        self._extImport = Import()
        self._extFromImports = FromImportSet()
        self._moduleFiles: Dict[str, PyFile] = {}
        self._moduleTopoOrder: Optional[DefaultDict[str, DFSData]] = None


    # getModuleFilePath(module): Retrieves the file path for a given module
    def getModuleFilePath(self, module: str) -> str:
        try:
            return self._modules[module].__file__
        except KeyError:
            result = f"{ModulePathTools.toFilePath(module)}{FileExts.Py}"
            self._modules[module] = result
            return result


    # addModuleFile(module, moduleFile): Keeps track of the file data related to a module, if it does not exist
    def addModuleFile(self, module: str, moduleFile: PyFile):
        try:
            self._moduleFiles[module]
        except KeyError:
            self._moduleFiles[module] = moduleFile


    # _getNeighbourModules(module): Retrieves the depended resources that the module depends on
    def _getNeighbourModules(self, module: str) -> Set[str]:
        filePath = self.getModuleFilePath(module)
        file = PyFile(filePath, module)
        file.read()
        self.addModuleFile(module, file)

        self._extImport += file.extImport
        self._extFromImports += file.extFromImports

        result = file.getLocalCalledModules()
        return result

    # readModules(): Decorator to first read in all the modules before building the required script files
    def readModules(func: Callable[..., None]):
        def readModulesWrapper(self, *args, **kwargs):
            if (self._moduleTopoOrder is None):
                error, self._moduleTopoOrder = Algo.topoSort(self._rootModule, self._getNeighbourModules, vertices = set(self._modules.keys()), reverse = True)

            return func(self, *args, **kwargs)
        return readModulesWrapper
    

    @readModules
    def getScriptStr(self) -> str:
        scriptStr = self._scriptPreamble
        scriptStr += f"{self._extImport.toStr()}\n\n"
        scriptStr += f"{self._extFromImports.toStr()}\n\n\n"
        scriptStr += f"{ScriptChangeDir}\n\n"

        scriptStrParts = []
        for module, dfsData in self._moduleTopoOrder.items():
            if (not dfsData.visisted):
                continue

            moduleFile = self._moduleFiles[module]
            scriptStrParts.append(moduleFile.getScriptStr())
        
        scriptStr += "\n\n\n".join(scriptStrParts)
        scriptStr += self._scriptPostamble
        return scriptStr
    

    @readModules
    def writeModules(self):
        for module, dfsData in self._moduleTopoOrder.items():
            if (not dfsData.visisted):
                continue

            moduleFile = self._moduleFiles[module]
            moduleFile.update()


    def buildScript(self):
        scriptStr = self.getScriptStr()

        print("Creating Script...")
        FileTools.writeFile(self._scriptPath, lambda filePtr: filePtr.write(scriptStr))

        print("Updating Module Files...")
        self.writeModules()


    def buildScriptInit(self):
        file = PyFile(self._moduleInitPath, self._module)
        file.read()

        allObjects = file.getLocalObjects()
        fromImport = FromImport(self._scriptModule, objects = allObjects)
        
        # get the __all__ text for the init file
        allModulesTxt = "__all__ = ["
        allObjects = map(lambda ob: f'"{ob}"', allObjects)
        allModulesTxt += ", ".join(allObjects)
        allModulesTxt += "]"

        print(f"Creating __init__.py")
        initTxt = f"{fromImport.toStr()}\n\n{allModulesTxt}"
        scriptInitFile = PyFile(self._scriptInitPath, self._scriptModule)
        scriptInitFile.write(initTxt)


    def buildScriptMain(self):
        file = PyFile(self._moduleMainPath, self._module)
        file.read()

        fromImport = FromImport(self._scriptModule, objects = OrderedSet([ScriptPartNames.MainFunc.value]))

        print(f"Creating __main__.py")
        mainTxt = f"{fromImport.toStr()}\n\n{file.getScriptStr()}"
        scriptMainFile = PyFile(self._scriptMainPath, self._scriptModule)
        scriptMainFile.write(mainTxt)


    def build(self):
        self.buildScript()
        self.buildScriptInit()
        self.buildScriptMain()
