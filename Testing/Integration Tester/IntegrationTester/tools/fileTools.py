import re
import os
import sys
from typing import Set

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


# FileTools: Tools for file manipulation for the tester
class FileTools():
    IntegrationTestResultsFileName = 'integrationTestResults.txt'
    IntegrationTestOutputsFileName = 'integrationTestOutputs.txt'
    FileEncoding = "utf-8"

    BinaryFiles = re.compile("\.(buf)$")
    FilesToNotPrintContentPattern = re.compile("\.(buf|py)$")
    LogFiles = re.compile("RemapFixLog\.txt$")

    # readTestResults(): Reads the integration test results
    @classmethod
    def readTestResults(cls) -> str:
        result = ""
        with open(cls.IntegrationTestResultsFileName, "r", encoding = cls.FileEncoding) as f:
            result = f.read()
        return result

    # clearTestPrintOutputs(): Clears out the printing outputs
    @classmethod
    def clearTestPrintOutputs(cls):
        with open(cls.IntegrationTestOutputsFileName, "w", encoding = cls.FileEncoding) as f:
            f.write("")

    # addTestPrintOutputs(): Adds text into the printing outputs
    @classmethod
    def addTestPrintOutputs(cls, txt: str):
        with open(cls.IntegrationTestOutputsFileName, "a", encoding = cls.FileEncoding) as f:
            f.write(txt)

    # notPrintable(): Whether the file should not have its content printed
    @classmethod
    def notPrintable(cls, file: str) -> bool:
        return bool(cls.FilesToNotPrintContentPattern.search(file))
    
    # isBinary(file): Whether the file is a binary file
    @classmethod
    def isBinary(cls, file: str) -> bool:
        return bool(cls.BinaryFiles.search(file))
    
    # isLog(file): Whether the file is a log file
    @classmethod
    def isLog(cls, file: str) -> bool:
        return bool(cls.LogFiles.search(file))
    
    # getFilesAndDirs(folder): Retrives all the files and folders from a folder
    @classmethod
    def getFileAndDirs(cls, folder: str, withRoot: bool = False) -> Set[str]:
        processPath = lambda root, dirItem: FRB.FileService.parseOSPath(os.path.relpath(FRB.FileService.parseOSPath(os.path.join(root, dirItem)), folder))
        if (withRoot):
            processPath = lambda root, dirItem: os.path.join(root, dirItem)

        result = set()
        for root, dirs, files in os.walk(folder, topdown = True):
            for fileName in files:
                result.add(processPath(root, fileName))

            for dirName in dirs:
                result.add(processPath(root, dirName))

        return result

