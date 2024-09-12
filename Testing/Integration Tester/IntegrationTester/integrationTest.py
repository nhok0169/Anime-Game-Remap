from .config import Config
from .commands import Commands
from .Exceptions import NoInputsFound, ExpectedOutputsNotFound, ResultOutputsNotFound
from .tools import FileTools
import unittest
from unittest import mock
import os
import shutil
import sys
import re
from difflib import unified_diff
import directory_tree as DT
from typing import Optional, List, Dict

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )/api')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB

ExpectedTestPathPrefix = "expected_"
ResultTestPathPrefix = "output_"

FolderToReplace = FRB.FileService.parseOSPath(os.path.dirname(os.path.abspath(__file__)))
FolderToReplace = FRB.FileService.parseOSPath(os.path.dirname(FolderToReplace))

class PatchService:
    def _cleanup(self, patch, target):
        patch.stop()
        self.patches.pop(target)

    def patch(self, target, *args, **kwargs):
        p = mock.patch(target, *args, **kwargs)
        patchedMock = p.start()
        self.addCleanup(self._cleanup, *[p, target])
        self.patches[target] = patchedMock

    def patchObj(self, target, *args, **kwargs):
        p = mock.patch.object(target, *args, **kwargs)
        patchedMock = p.start()
        self.addCleanup(self._cleanup, *[p, target])
        self.patches[target] = patchedMock


class IntegrationTest(unittest.TestCase, PatchService):
    @classmethod
    def setUpClass(cls):
        cls._command = Config["command"]
        cls._testName = ""
        cls._testFolder = ""
        cls._printTxt = ""
        cls.patches: Dict[str, mock.Mock] = {}

    def tearDown(self):
        if (self._printTxt):
            FileTools.addTestPrintOutputs(f"{self._printTxt}\n")
    
    def print(self, txt: str):
        self._printTxt += f"{txt}\n"
        print(txt)

    # getTestPath(): Retrieves the absolute path for the tests
    def getTestPath(self) -> str:
        pass
    
    # getTestPath(): Retrieves the absolute path for the expected output folder
    def getExpectedPath(self) -> str:
        testFolderPath = self.getTestPath()
        return os.path.join(testFolderPath, f"{ExpectedTestPathPrefix}{self._testName}")
    
    # getResultPath(): Retrieves the absolute path for the resultant output folder
    def getResultPath(self) -> str:
        testFolderPath = self.getTestPath()
        return os.path.join(testFolderPath, f"{ResultTestPathPrefix}{self._testName}")
    
    # getInputPath(): Retrieves the input folder for the test
    def getInputPath(self) -> str:
        testFolderPath = self.getTestPath()
        return os.path.join(testFolderPath, "inputs")
    
    # isExpectedTestPath(folder): Determines if 'folder' is for an expected output folder
    def isExpectedTestPath(self, folder: str) -> bool:
        return os.path.basename(folder).startswith(ExpectedTestPathPrefix)
    
    # isResultTestPath(folder): Determines if 'folder' is a resultant output folder
    def isResultTestpath(self, folder: str) -> bool:
        return os.path.basename(folder).startswith(ResultTestPathPrefix)
    
    # getTargetOutputPath(): Retrieves the output folder for the test
    def getTargetOutputPath(self) -> Optional[str]:
        if (self._command == Commands.ProduceOutputs):
            return self.getExpectedPath()
        elif (self._command == Commands.RunSuite):
            return self.getResultPath()

        return None
    
    # cleanExpected(folderPath): Removes a test folder
    def cleanFolder(self, folderPath: str):
        try:
            shutil.rmtree(folderPath)
        except FileNotFoundError:
            pass
    
    # copyInputs(destFolder): Copies the input folder to the target test output folder
    def copyInputs(self, destFolder: str):
        inputPath = self.getInputPath()

        try:
            shutil.copytree(inputPath, destFolder)
        except FileNotFoundError as e:
            raise NoInputsFound(self._testFolder) from e

    # resetTest(): Resets the test from any previous runs
    def resetTest(self):
        targetOutputFolder = self.getTargetOutputPath()
        if (targetOutputFolder is not None):
            self.cleanFolder(targetOutputFolder)
            self.copyInputs(targetOutputFolder)

        # clears out all output folders
        elif (self._command == Commands.clearOutputs):
            dirItems = os.listdir(self._testFolder)
            for dirItem in dirItems:
                dirItemPath = os.path.join(self._testFolder, dirItem)
                if (os.path.isdir(dirItemPath) and (self.isExpectedTestPath(dirItem) or self.isResultTestpath(dirItem))):
                    shutil.rmtree(dirItemPath)

    # printOutputs(testName): Prints the outputs for a particular test
    def printOutputs(self, testName: str):
        expectedFolder = self.getExpectedPath()
        if (not os.path.exists(expectedFolder)):
            raise ExpectedOutputsNotFound(self._testFolder, testName)

        testHeading = FRB.Heading(title = testName, sideLen = 10, sideChar = "@")
        fileTreeHeading = FRB.Heading(title = "File Tree", sideLen = 8, sideChar = "=")
        fileHeading = FRB.Heading(title = "=========", sideLen = 8, sideChar = "=")
        fileContentHeading = FRB.Heading(title = "Content", sideLen = 5, sideChar = "#")

        self.print(f"{testHeading.open()}\n")

        # print the file tree
        self.print(f"{fileTreeHeading.open()}\n")
        self.print(DT.display_tree(expectedFolder, string_rep = True))
        self.print(f"\n{fileTreeHeading.close()}")

        # print the contents of each file
        for root, dirs, files in os.walk(expectedFolder, topdown = True):
            for fileName in files:
                if (FileTools.notPrintable(fileName)):
                    continue
                
                filePath = os.path.join(root, fileName)
                self.print(f"{fileHeading.close()}\n")
                self.print(f"File: {fileName}\n")

                self.print(f"{fileContentHeading.open()}\n")

                with open(filePath, "r", encoding = "utf-8") as f:
                    fileTxt = f.read()
                    self.print(fileTxt)

                self.print(f"\n{fileContentHeading.close()}")
                self.print(f"\n{fileHeading.close()}")


        self.print(f"\n{testHeading.close()}\n")

    # editLogFile(file, targetFolders): Changes the log files to not display absolute paths
    def editLogFile(self, file: str, targetFoldersReplacePattern: str):
        fileTxt = ""
        with open(file, "r", encoding = FileTools.FileEncoding) as f:
            fileTxt = f.read()

        # replace absolute paths in the log file (for hiding own builds path locations)
        fileTxt = re.sub(targetFoldersReplacePattern, "absolute/path", fileTxt)
        with open(file, "w", encoding = FileTools.FileEncoding) as f:
            f.write(fileTxt)

        # create the summary text that will be compared
        summaryFileTxt = re.split(r"\n\n#", fileTxt)[-1]
        summaryFileTxt = re.sub(targetFoldersReplacePattern, "absolute/path", summaryFileTxt)

        summaryLogFile = FRB.FileService.parseOSPath(os.path.join(os.path.dirname(file), "summaryLog.txt"))
        with open(summaryLogFile, "w", encoding = FileTools.FileEncoding) as f:
            f.write(summaryFileTxt)

    # generateOutputs(targetFolder, scriptRelPath): Executes the test script to generate outputs
    def generateOutputs(self, targetFolder: str, scriptRelPath: str):
        scriptPath = FRB.FileService.parseOSPath(os.path.join(targetFolder, scriptRelPath))
        scriptGlobals = globals()
        scriptGlobals["__file__"] = scriptPath
        exec(open(scriptPath, encoding = FileTools.FileEncoding).read(), scriptGlobals)

        # get the regex string to replace the folders
        targetFolders = [targetFolder.replace(os.sep, "\\/"), targetFolder.replace(os.sep, "\\\\"), targetFolder.replace(os.sep, "\\\\\\\\"),
                         FolderToReplace.replace(os.sep, "\\/"), FolderToReplace.replace(os.sep, "\\\\"), FolderToReplace.replace(os.sep, "\\\\\\\\")]

        targetFoldersReplacePattern = []
        for folder in targetFolders:
            targetFoldersReplacePattern.append(f"({folder})")
        targetFoldersReplacePattern = "|".join(targetFoldersReplacePattern)
        
        # edit the log files
        for root, dirs, files in os.walk(targetFolder, topdown = True):
            for fileName in files:
                if (FileTools.isLog(fileName)):
                    self.editLogFile(os.path.join(root, fileName), targetFoldersReplacePattern)

    # compareResults(testName): Compares the expected and generated results
    def compareResults(self, testName: str):
        expectedFolder = self.getExpectedPath()
        resultFolder = self.getResultPath()

        if (not os.path.exists(expectedFolder)):
            raise ExpectedOutputsNotFound(self._testFolder, testName)
        
        if (not os.path.exists(resultFolder)):
            raise ResultOutputsNotFound(self._testFolder, testName)
        
        # get the paths to all the file/folders
        expectedPaths = FileTools.getFileAndDirs(expectedFolder)
        resultPaths = FileTools.getFileAndDirs(resultFolder)

        # compare the file tree
        pathsOnlyInExpected = expectedPaths - resultPaths
        pathsOnlyInResult = resultPaths - expectedPaths

        commonPaths = expectedPaths.intersection(resultPaths)
        commonPaths = list(filter(lambda path: os.path.isfile(FRB.FileService.parseOSPath(os.path.join(expectedFolder, path))), commonPaths))
        fileDiffs = {}

        # compare the content of the files
        for path in commonPaths:
            expectedPath = os.path.join(expectedFolder, path)
            resultPath = os.path.join(resultFolder, path)

            readCode = "r"
            encoding = "utf-8"

            if (FileTools.isLog(path)):
                continue

            # compare binary files
            if (FileTools.isBinary(path)):
                readCode += "b"
                encoding = None

                expectedBytes = ""
                resultBytes = ""

                with open(expectedPath, readCode, encoding = encoding) as f:
                    expectedBytes = f.read()

                with open(resultPath, readCode, encoding = encoding) as f:
                    resultBytes = f.read()

                if (expectedBytes != resultBytes):
                    fileDiffs[path] = None

            # compare text-readable files
            else:
                expectedFileLines = []
                resultFileLines = []
                
                with open(expectedPath, readCode, encoding = encoding) as f:
                    expectedFileLines = f.readlines()

                with open(resultPath, readCode, encoding = encoding) as f:
                    resultFileLines = f.readlines()

                diffLines = list(unified_diff(resultFileLines, expectedFileLines, fromfile = os.path.join("result", path), tofile = os.path.join("expected", path)))
                if (diffLines):
                    fileDiffs[path] = diffLines

        # don't print if there are no differences between the tests
        if (not pathsOnlyInResult and not pathsOnlyInExpected and not fileDiffs):
            return
        
        # ======= print the result ==============

        failMsg = ""
        testHeading = FRB.Heading(title = testName, sideLen = 10, sideChar = "@")
        fileTreeHeading = FRB.Heading(title = "File Tree", sideLen = 8, sideChar = "=")
        fileHeading = FRB.Heading(title = "=========", sideLen = 8, sideChar = "=")
        fileContentHeading = FRB.Heading(title = "Content", sideLen = 5, sideChar = "#")

        failMsg += f"\n{testHeading.open()}\n\n"

        # print the difference in the file tree
        if (pathsOnlyInResult or pathsOnlyInExpected):
            failMsg += f"{fileTreeHeading.open()}\n\n"

            if (pathsOnlyInResult):
                failMsg += f"-- The following files/folders are not supposed to be in the result --\n"
                for path in pathsOnlyInResult:
                    failMsg += f"{path}\n"
                failMsg += "\n"

            if (pathsOnlyInExpected):
                failMsg += f"-- The following files/folders are missing from the result --\n"
                for path in pathsOnlyInExpected:
                    failMsg += f"{path}\n"
                failMsg += "\n"

            failMsg += f"Expected File Tree:\n"
            failMsg += f"{DT.display_tree(expectedFolder, string_rep = True)}\n\n"

            failMsg += f"Result File Tree:\n"
            failMsg += f"{DT.display_tree(resultFolder, string_rep = True)}\n\n"

            failMsg += f"{fileTreeHeading.close()}\n"

        # print the differences between the files
        if (fileDiffs):
            for file in fileDiffs:
                diff = fileDiffs[file]

                failMsg += f"{fileHeading.close()}\n\n"
                failMsg += f"File: {file}\n\n"
                
                failMsg += f"{fileContentHeading.open()}\n\n"
                if (diff is None):
                    failMsg += "See both files for more info\n"
                else:
                    for diffLine in diff:
                        failMsg += diffLine
                failMsg += f"\n{fileContentHeading.close()}\n"

                failMsg += f"\n{fileHeading.close()}\n"

        failMsg += f"\n{testHeading.close()}\n"
        self.fail(failMsg)

        # =======================================
        

    # runTest(name, scriptRelPath): Runs the test based off the specified command
    def runTest(self, name: str, scriptRelPath: str):
        self._testName = name
        self._testFolder = self.getTestPath()
        self.resetTest()

        if (self._command == Commands.ProduceOutputs):
            self.generateOutputs(self.getExpectedPath(), scriptRelPath)

        elif (self._command == Commands.PrintOutputs):
            self.printOutputs(name)

        elif (self._command == Commands.RunSuite):
            self.generateOutputs(self.getResultPath(), scriptRelPath)
            self.compareResults(name)





