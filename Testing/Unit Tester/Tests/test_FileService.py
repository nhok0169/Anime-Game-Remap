import sys
import unittest.mock as mock
from .baseFileUnitTest import BaseFileUnitTest

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


class FileServiceTest(BaseFileUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._folderTree1 = {"a.txt": None,
                            "b.haku": None,
                            "c": {"folder": {"innerFolder": {"core.ini": None,
                                                             "energy.md": None,
                                                             "cradle": {}}},
                                  "helloWorld.rst": None},
                            "d": {},
                            "sounds": {"hello.ogg": None}}
        
        cls.setupFolderTree(cls._folderTree1)

    # =========== getFilesAndDirs =========================

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_folderTreeFromCWD_FileAndDirsFromCWD(self, m_getPath):
        self.setupFolderTree(self._folderTree1)

        files, dirs = FRB.FileService.getFilesAndDirs()
        self.compareList(files, ["a.txt", "b.haku"])
        self.compareList(dirs, ["c", "d", "sounds"])

    def test_folderTreeFromDir_FileAndDirsFromDir(self):
        self.setupFolderTree(self._folderTree1)

        files, dirs = FRB.FileService.getFilesAndDirs(path = "some/dir")
        self.compareList(files, [r"some/dir/a.txt", r"some/dir/b.haku"])
        self.compareList(dirs, [r"some/dir/c", r"some/dir/d", r"some/dir/sounds"])

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_emptyFolderTree_noFilesAndNoDirs(self, m_getPath):
        self.setupFolderTree({})

        files, dirs = FRB.FileService.getFilesAndDirs()
        self.compareList(files, [])
        self.compareList(dirs, [])

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = ".")
    def test_folderTreeFromCWDRecursive_allFileAndDirsFromCWD(self, m_getPath):
        self.setupFolderTree(self._folderTree1)

        files, dirs = FRB.FileService.getFilesAndDirs(recursive = True)
        self.compareList(files, ["./a.txt", "./b.haku", "./c/helloWorld.rst", "./c/folder/innerFolder/core.ini", "./c/folder/innerFolder/energy.md", "./sounds/hello.ogg"])
        self.compareList(dirs, ["./c", "./d", "./sounds", "./c/folder", "./c/folder/innerFolder", "./c/folder/innerFolder/cradle"])

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = ".")
    def test_emptyfolderTreeFromCWDRecursive_noFilesAndNoDirs(self, m_getPath):
        self.setupFolderTree({})

        files, dirs = FRB.FileService.getFilesAndDirs(recursive = True)
        self.compareList(files, [])
        self.compareList(dirs, [])

    # =====================================================
    # =========== getFiles ================================
    
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_folderTreeFromCWDNoFilters_AllFilesFromCWD(self, m_getPath):
        self.setupFolderTree(self._folderTree1)
        files = FRB.FileService.getFiles()
        self.compareList(files, ["a.txt", "b.haku"])

    def test_folderTreeFromDirNoFilters_AllFilesFromDir(self):
        self.setupFolderTree(self._folderTree1)
        files = FRB.FileService.getFiles(path = "dir")
        self.compareList(files, ["dir/a.txt", "dir/b.haku"])

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_emptyfolderTreeFromCWDNoFilters_NoFiles(self, m_getPath):
        self.setupFolderTree({})
        files = FRB.FileService.getFiles()
        self.compareList(files, [])

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_fileListNoFilters_AllFiles(self, m_getPath):
        folderFiles = ["hello.ts", "boo.h"]
        files = FRB.FileService.getFiles(files = folderFiles)
        self.compareList(files, folderFiles)

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_emptyFileListNoFilters_NoFiles(self, m_getPath):
        folderFiles = []
        files = FRB.FileService.getFiles(files = folderFiles)
        self.compareList(files, folderFiles)

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_folderTreeFromCWDEmptyFilters_AllFilesFromCWD(self, m_getPath):
        self.setupFolderTree(self._folderTree1)
        files = FRB.FileService.getFiles(filters = [])
        self.compareList(files, ["a.txt", "b.haku"])

    def test_folderTreeFromDirIndependentFilters_PartitionedFiles(self):
        self.setupFolderTree(self._folderTree1)

        files = FRB.FileService.getFiles(path = "dir", filters = [lambda file: file == "a.txt", lambda file: file == "b.haku"])
        self.compareList(files[0], ["dir/a.txt"])
        self.compareList(files[1],  ["dir/b.haku"])

    def test_fileListIndependentFilters_PartitionedFiles(self):
        files = FRB.FileService.getFiles(path = "dir", filters = [lambda file: file == "a.txt", lambda file: file == "b.haku"], files = ["a.txt", "b.haku", "c.html"])
        self.assertEqual(len(files), 2)
        self.compareList(files[0], ["dir/a.txt"])
        self.compareList(files[1],  ["dir/b.haku"])

    def test_folderTreeFromDirIndependentFilters_RepeatedFilesInPartitions(self):
        self.setupFolderTree(self._folderTree1)

        files = FRB.FileService.getFiles(path = "dir", filters = [lambda file: True, lambda file: file.find(".") > -1, lambda file: file.endswith(".txt")])
        self.assertEqual(len(files), 3)
        self.compareList(files[0],  ["dir/a.txt", "dir/b.haku"])
        self.compareList(files[1],  ["dir/a.txt", "dir/b.haku"])
        self.compareList(files[2],  ["dir/a.txt"])

    def test_fileListCommonFileFilters_RepeatedFilesInPartitions(self):
        files = FRB.FileService.getFiles(path = "dir", filters = [lambda file: True, lambda file: file.find(".") > -1, lambda file: file.endswith(".txt")], files = ["a.txt", "b.haku", "c.html"])
        self.assertEqual(len(files), 3)
        self.compareList(files[0],  ["dir/a.txt", "dir/b.haku", "dir/c.html"])
        self.compareList(files[1],  ["dir/a.txt", "dir/b.haku", "dir/c.html"])
        self.compareList(files[2],  ["dir/a.txt"])
        
    # =====================================================
    # =========== getSingleFiles ==========================
        
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_folderTreeFromCWDNoFiltersOptionalSingle_FirstFile(self, m_getPath):
        self.setupFolderTree(self._folderTree1)
        file = FRB.FileService.getSingleFiles(optional = True)
        self.assertEqual(file, "a.txt")

    def test_folderTreeFromDirNoFiltersOptionalSingle_FirstFile(self):
        self.setupFolderTree(self._folderTree1)
        file = FRB.FileService.getSingleFiles(path = "dir", optional = True)
        self.assertEqual(file, "dir/a.txt")

    def test_fileListNoFiltersOptionalSingle_FirstFile(self):
        self.setupFolderTree(self._folderTree1)
        file = FRB.FileService.getSingleFiles(path = "dir", optional = True, files = self._currentDirItems)
        self.assertEqual(file, "dir/a.txt")

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_folderTreeFromCWDNoFiltersMustBeSingle_DuplicateFileError(self, m_getPath):
        self.setupFolderTree(self._folderTree1)

        error = None
        try:
            FRB.FileService.getSingleFiles()
        except BaseException as e:
            error = e

        self.assertIsInstance(error, FRB.DuplicateFileException)

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_emptyFolderTreeFromCWDNoFiltersOptionalSingle_NoFile(self, m_getPath):
        self.setupFolderTree({})
        file = FRB.FileService.getSingleFiles(optional = True)
        self.assertIs(file, None)

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_folderTreeFromCWDNoFiltersMustBeSingle_MissingFileError(self, m_getPath):
        self.setupFolderTree({})

        error = None
        try:
            FRB.FileService.getSingleFiles()
        except BaseException as e:
            error = e

        self.assertIsInstance(error, FRB.MissingFileException)

    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.FileService.getPath" , return_value = "")
    def test_folderTreeFromCWDFiltersOptionalSingle_PartitionedFiles(self, m_getPath):
        self.setupFolderTree(self._folderTree1)
        files = FRB.FileService.getSingleFiles(filters = {"fileA": lambda file:  file == "a.txt", "fileB": lambda file: file == "b.haku", 
                                                          "fileC": lambda file: file == "c.yaml", "allFile": lambda file: file.find(".") > -1}, optional = True)
        self.compareList(files, ["a.txt", "b.haku", None, "a.txt"])

    # =====================================================
    # =========== rename ==================================
        
    def test_folderTreeOldUniqueNameNewUniqueName_OldNameRenamedToNewName(self):
        self.setupFolderTree(self._folderTree1)
        oldName = "./c/folder/innerFolder/core.ini"
        newName = "./c/folder/outerCore.config"
        FRB.FileService.rename(oldName, newName)
        self.assertIn(newName, self._flattendDirItems)
        self.assertNotIn(oldName, self._flattendDirItems)
        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.os.rename"].assert_called_once()

    def test_folderTreeOldNameDoesNotExist_FileNotFoundError(self):
        self.setupFolderTree(self._folderTree1)
        error = None
        try:
            FRB.FileService.rename("./c/folder/innerFolder/nonExistentCore.ini", "lalalalala")
        except BaseException as e:
            error = e

        self.assertIsInstance(error, FileNotFoundError)

    def test_folderTreeOldUniqueNameNewNameExists_OldNameRenamedOriginalNewNameDeleted(self):
        self.setupFolderTree(self._folderTree1)
        oldName = "./c/folder/innerFolder/core.ini"
        newName = "./a.txt"
        FRB.FileService.rename(oldName, newName)
        self.assertIn(newName, self._flattendDirItems)
        self.assertNotIn(oldName, self._flattendDirItems)
        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.os.remove"].assert_called_once()

    def test_folderTreeSameOldAndNewName_NoChange(self):
        self.setupFolderTree(self._folderTree1)
        oldName = "./c/folder/innerFolder/core.ini"
        newName = "./c/folder/innerFolder/core.ini"
        FRB.FileService.rename(oldName, newName)
        self.assertIn(newName, self._flattendDirItems)
        self.assertIn(oldName, self._flattendDirItems)
        self.patches["src.FixRaidenBoss2.FixRaidenBoss2.os.rename"].assert_not_called()
        
    # =====================================================
    # =========== changeExt ===============================

    def test_fileSameExt_SameFile(self):
        file = "helloWorld.rkt"
        ext = "rkt"
        result = FRB.FileService.changeExt(file, ext)
        self.assertEqual(result, file)

    def test_fileDiffExt_FileWithChangedExt(self):
        file = "helloWorld.rkt"
        ext = "java"
        result = FRB.FileService.changeExt(file, ext)
        self.assertEqual(result, "helloWorld.java")

    def test_fileWithoutDot_SameFile(self):
        file = "runSuite"
        ext = ".cpp"
        result = FRB.FileService.changeExt(file, ext)
        self.assertEqual(result, file)

    # =====================================================
    # =========== disableFile =============================
    
    def test_folderTreeFileNotTxt_RaidenDisabledFile(self):
        self.setupFolderTree(self._folderTree1)
        file = "./c/helloWorld.rst"
        FRB.FileService.disableFile(file)
        self.assertIn("./c/DISABLED_BossFixBackup_helloWorld.txt", self._flattendDirItems)
        self.assertNotIn(file, self._flattendDirItems)

    def test_folderTreeFileNotTxtNoFilePrefix_TxtFile(self):
        self.setupFolderTree(self._folderTree1)
        file = "./c/helloWorld.rst"
        FRB.FileService.disableFile(file, filePrefix = "")
        self.assertIn("./c/helloWorld.txt", self._flattendDirItems)

    def test_folderTreeFileTxtNoFilePrefix_TxtFile(self):
        self.setupFolderTree(self._folderTree1)
        file = "./a.txt"
        FRB.FileService.disableFile(file, filePrefix = "")
        self.assertIn(file, self._flattendDirItems)

    # =====================================================
    # =========== ntPathToPosix ===========================
        
    def test_absFilePath_samePath(self):
        path = "C:/OneDrive/Documents/Homework/notSus.mp4"
        result = FRB.FileService.ntPathToPosix(path)
        self.assertEqual(result, path)

    def test_absFolderPath_samePath(self):
        path = "C:/OneDrive/Documents/Homework"
        result = FRB.FileService.ntPathToPosix(path)
        self.assertEqual(result, path)

    def test_absFilePathWithDiffSeperators_pathWithSameSeperators(self):
        path = r"C:/OneDrive\Documents/Homework\notSus.mp4"
        result = FRB.FileService.ntPathToPosix(path)
        self.assertEqual(result, "C:/OneDrive/Documents/Homework/notSus.mp4")

    # =====================================================
    # =========== parseOSPath =============================
        
    def test_absFilePath_sameOSPath(self):
        path = "C:/OneDrive/Documents/Homework/notSus.mp4"
        result = FRB.FileService.parseOSPath(path)
        self.assertEqual(result, path)

    def test_absFolderPath_sameOSPath(self):
        path = "C:/OneDrive/Documents/Homework"
        result = FRB.FileService.parseOSPath(path)
        self.assertEqual(result, path)

    def test_absFilePathCycle_shortestFilePath(self):
        path = r"C:/OneDrive\Documents/Homework\../../Videos\notSus.mp4"
        result = FRB.FileService.parseOSPath(path)
        self.assertEqual(result, "C:/OneDrive/Videos/notSus.mp4")

    # =====================================================
    # =========== absPathOfRelPath ========================
        
    def test_relPath_absPathOfRelPath(self):
        result = FRB.FileService.absPathOfRelPath(r"some/folder\bang.txt", r"another\folder")
        self.assertEqual(result, f"{self.absPath}/another/folder/some/folder/bang.txt")

    def test_dstPathIsAbsPath_samePathAsDstPath(self):
        result = FRB.FileService.absPathOfRelPath(self.osPathJoin(self.absPath, r"some/folder\bang.txt"), r"another\folder")
        self.assertEqual(result, self.osPathJoin(self.absPath, r"some/folder/bang.txt"))

    def test_dstPathCycle_absPathOfRelPath(self):
        result = FRB.FileService.absPathOfRelPath(r"hello/../../lol/../../tada\bang.txt", r"another\folder")
        self.assertEqual(result, r"/the/absolute/path/tada/bang.txt")

    def test_dstPathCycleRelFolderCycle_absPathOfRelPath(self):
        result = FRB.FileService.absPathOfRelPath(r"hello/../../lol/../../tada\bang.txt", r"..\mama/..\dada/another\folder")
        self.assertEqual(result, r"/the/absolute/dada/tada/bang.txt")

    # =====================================================
    # =========== getRelPath ==============================
        
    def test_relPath_combinedPath(self):
        result = FRB.FileService.getRelPath(r"some/folder\bang.txt", r"another\folder")
        self.assertEqual(result, r"another/folder/some/folder/bang.txt")

    def test_relPathCycle_combinedPath(self):
        result = FRB.FileService.getRelPath(r"some/../../folder\boyah/../new\bang.txt", r"another\folder")
        self.assertEqual(result, r"another/folder/new/bang.txt")

    def test_relPathFromDifferentMounts_AbsPathFromMount(self):
        result = FRB.FileService.getRelPath(r"C:/new\bang.txt", r"D:\another\folder")
        self.assertEqual(result, r"C:/new/bang.txt")

    def test_relPathFromSameMounts_combinedPath(self):
        result = FRB.FileService.getRelPath(r"C:/new\bang.txt", r"C:\another\folder")
        self.assertEqual(result, r"../../new/bang.txt")
        
    # =====================================================