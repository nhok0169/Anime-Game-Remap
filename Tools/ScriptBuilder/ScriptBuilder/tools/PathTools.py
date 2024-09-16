import os
import ntpath
from pathlib import Path

from ..constants.FileExts import FileExts

ModuleSep = "."
ModuleParentDir = f"{ModuleSep}{ModuleSep}"
ModuleSepLen = len(ModuleSep)
ParentDir = f"..{os.sep}"
CurrentDir = f".{os.sep}"


# FilePathTools: Tools for dealing with file paths
class FilePathTools():
    @classmethod
    def toModulePath(cls, filePath: str):
        result = cls.getNoExtPath(filePath)
        result = result.replace(ParentDir, ModuleParentDir)
        return result.replace(os.sep, ModuleSep)

    @classmethod
    def parseOSPath(cls, path: str):
        result = ntpath.normpath(path)
        result = cls.ntPathToPosix(result)
        return result

    @classmethod
    def ntPathToPosix(cls, path: str) -> str:
        return path.replace(ntpath.sep, os.sep)
    
    @classmethod
    def getNoExtPath(cls, filePath: str) -> str:
        basename = Path(filePath).stem
        dirname = ntpath.dirname(filePath)
        result = ntpath.join(dirname, basename)
        result = cls.parseOSPath(result)
        return result
    

class PyPathTools(FilePathTools):
    @classmethod
    def getInitPath(cls, folder: str):
        return os.path.join(folder, f"__init__{FileExts.Py.value}")
    
    @classmethod
    def getMainPath(cls, folder: str):
        return os.path.join(folder, f"__main__{FileExts.Py.value}")


# ModulePathTools: Tools for dealing with python module paths
class ModulePathTools():
    @classmethod
    def toFilePath(cls, modulePath: str):
        return modulePath.replace(ModuleSep, os.sep)
    
    @classmethod
    def currentPath(cls, path: str):
        return f"{ModuleSep}{path}"
    
    @classmethod
    def join(cls, path: str, *paths):
        for p in paths:
            path += f".{p}"
        return path
    
    @classmethod
    def dirname(cls, modulePath: str):
        result = modulePath.rsplit(ModuleSep, 1)
        if (len(result) > 1):
            return result[0]
        return ""
    
    @classmethod
    def fromRelPath(cls, currentPath: str, relPath: str):
        if (relPath.startswith(ModuleSep)):
            relPath = relPath[ModuleSepLen:]

        relPathParts = relPath.split(ModuleSep)
        pathParts = currentPath.split(ModuleSep)
        pathParts.pop()

        for part in relPathParts:
            if (part):
                pathParts.append(part)
            else:
                pathParts.pop()

        result = ModuleSep.join(pathParts)
        return result

        