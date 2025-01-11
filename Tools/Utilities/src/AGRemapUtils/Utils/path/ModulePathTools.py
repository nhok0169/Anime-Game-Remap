import os

from ..constants.PathConstants import ModuleSep, ModuleSepLen


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
