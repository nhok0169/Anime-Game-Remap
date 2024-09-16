import sys
import os
import shutil
import ntpath
from .baseUnitTest import BaseUnitTest
import ntpath
from typing import List, Union


sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )/api')
import src.FixRaidenBoss2 as FRB

class BaseFileUnitTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.OsSep = "/"
        cls.NtPathSep = "\\"
        cls.absPath = "/the/absolute/path"

        cls._currentFolderTree = {}
        cls._currentDirItems = []
        cls._flattendDirItems = set()

    @classmethod
    def listDir(cls, folderTree) -> List[str]:
        return list(folderTree.keys())
    
    @classmethod
    def isFile(cls, file: str) -> bool:
        return file.rfind(".") > -1
    
    @classmethod
    def isAbsPath(cls, path: str) -> bool:
        return path.startswith(cls.OsSep) or path.startswith(cls.NtPathSep) or path.find(":") > -1
    
    @classmethod
    def getAbsPath(cls, path: str) -> bool:
        if (path.startswith(cls.absPath)):
            return path
        return cls.osPathJoin(cls.absPath, path)
    
    @classmethod
    def _fileWalk(cls, root: str, folderTree, result: List[Union[str, List[str]]]):
        currentFiles = []
        currentDirs = []
        subTreesToVisit = {}

        for dirItem in folderTree:
            dirItemValues = folderTree[dirItem]
            if (dirItemValues is None):
                currentFiles.append(dirItem)
                continue

            currentDirs.append(dirItem)
            subTreesToVisit[root + cls.OsSep + dirItem] = dirItemValues

        result.append([root, currentDirs, currentFiles])

        for folder in subTreesToVisit:
            cls._fileWalk(folder, subTreesToVisit[folder], result)

    @classmethod
    def fileWalk(cls, path: str, folderTree) -> List[Union[str, List[str]]]:
        result = []
        cls._fileWalk(path, folderTree, result)
        return result
    
    @classmethod
    def osPathJoin(cls, path1: str, path2: str):
        if (path1 == ""):
            return path2
        elif (path2 == ""):
            return path1
        return path1 + cls.OsSep + path2
    
    @classmethod
    def getRelPath(cls, path: str, start: str) -> str:
        pathDriveEndInd = path.find(":")
        startDriveEndInd = start.find(":")
        pathsHaveMounts = pathDriveEndInd > -1 and startDriveEndInd > -1
        if (pathsHaveMounts  and path[:pathDriveEndInd] != start[:startDriveEndInd]):
            raise ValueError
        elif (pathsHaveMounts):
            start = ntpath.normpath(start)
            startParts = start[startDriveEndInd + 2:].split(cls.NtPathSep)
            start = []
            for part in startParts:
                start.append("..")
            start = cls.OsSep.join(start)
            path = path[pathDriveEndInd + 2:]

        joinedPath = cls.osPathJoin(start, path)
        result = ntpath.normpath(joinedPath).replace(cls.NtPathSep, cls.OsSep)
        return result
    
    @classmethod
    def fileRename(cls, oldFile: str, newFile: str):
        if (oldFile not in cls._flattendDirItems):
            raise FileNotFoundError(f"{oldFile} is not found")

        if (newFile in cls._flattendDirItems):
            raise FileExistsError(f"{newFile} already exists")
        
        cls._flattendDirItems.remove(oldFile)
        cls._flattendDirItems.add(newFile)

    def fileRemove(self, file: str):
        if (file not in self._flattendDirItems):
            raise FileNotFoundError(f"{file} is not found")
        self._flattendDirItems.remove(file)

    @classmethod
    def setupFolderTree(cls, newFolderTree):
        cls._currentFolderTree = newFolderTree
        cls._currentDirItems = cls.listDir(newFolderTree)

        cls._flattendDirItems = set()
        for root, currentDirs, currentFiles in cls.fileWalk(".", cls._currentFolderTree):
            currentDirFullPaths = map(lambda dir: f"{root}/{dir}", currentDirs)
            currentFileFullPaths = map(lambda file: f"{root}/{file}", currentFiles)

            cls._flattendDirItems = cls._flattendDirItems.union(set(currentDirFullPaths))
            cls._flattendDirItems = cls._flattendDirItems.union(set(currentFileFullPaths))

    @classmethod
    def copy(cls, src: str, dest: str):
        cls._flattendDirItems.add(dest)

    def setUp(self):
        os.sep = self.OsSep
        ntpath.sep = self.NtPathSep

        self.patch("os.listdir", side_effect = lambda root: self.listDir(self._currentFolderTree))
        self.patch("os.path.isfile", side_effect = self.isFile)
        self.patch("os.path.join", side_effect = lambda path1, path2: self.osPathJoin(path1, path2))
        self.patch("os.walk", side_effect = lambda path, topdown: self.fileWalk(path, self._currentFolderTree))
        self.patch("os.rename", side_effect = lambda oldFile, newFile: self.fileRename(oldFile, newFile))
        self.patch("os.remove", side_effect = lambda file: self.fileRemove(file))
        self.patch("os.path.isabs", side_effect = lambda path: self.isAbsPath(path))
        self.patch("os.path.abspath", side_effect = lambda path: self.getAbsPath(path))
        self.patch("os.path.relpath", side_effect = lambda path, start: self.getRelPath(path, start))
        self.patch("shutil.copy2", side_effect = lambda src, dest: self.copy(src, dest))