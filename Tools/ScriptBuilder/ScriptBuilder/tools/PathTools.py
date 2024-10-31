import os
import sys
import ntpath
from pathlib import Path

from ..constants.UtilitiesPath import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.FileTools import FileTools
from Utils.constants.PathConstants import ParentDir, ModuleParentDir, ModuleSep


# FilePathTools: Tools for dealing with file paths
class FilePathTools():
    @classmethod
    def toModulePath(cls, filePath: str):
        result = cls.getNoExtPath(filePath)
        result = result.replace(ParentDir, ModuleParentDir)
        return result.replace(os.sep, ModuleSep)
    
    @classmethod
    def getNoExtPath(cls, filePath: str) -> str:
        basename = Path(filePath).stem
        dirname = ntpath.dirname(filePath)
        result = ntpath.join(dirname, basename)
        result = FileTools.parseOSPath(result)
        return result