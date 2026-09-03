import os
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IniFileTest(BaseUnitTest):
    """
    Tests for the pybind11 binding over AGRemapCore::IniFile.

    This class is C++-backed all the way down, so -- unlike the pure-Python IniFile -- it does
    NOT honour the harness's Python-level ``builtins.open``/``os.path``/``FileService.read``
    mocks: a std::filesystem call inside the core bypasses every one of them. The disk-touching
    tests below therefore use a real temporary file rather than a patched one.
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._iniTxt = "[TextureOverrideBody]\nhash = abc123\nvb0 = ResourceBody\n\n[ResourceBody]\nfilename = body.buf\n"

    # ================================================
    # ================== construction ================

    def test_txtOnly_readImmediatelyWithNoFile(self):
        ini = FRB.IniFile(txt = self._iniTxt)

        self.assertIsNone(ini.file)
        self.assertTrue(ini.fileLinesRead)
        self.assertEqual(ini.fileTxt, self._iniTxt)
        self.assertEqual(len(ini.fileLines), 6)

    def test_noFile_folderIsEmpty(self):
        # A deliberate divergence from the pure-Python IniFile, whose 'folder' falls back to the
        # folder the script is run from -- see the binding's own note.
        self.assertEqual(FRB.IniFile(txt = self._iniTxt).folder, "")

    def test_defaultConstruction_isEmptyAndUnclassified(self):
        ini = FRB.IniFile()

        self.assertIsNone(ini.file)
        self.assertEqual(ini.fileTxt, "")
        self.assertFalse(ini.isClassified)
        self.assertFalse(ini.isModIni)
        self.assertFalse(ini.isFixed)

    def test_fileTxtSetter_rereadsLines(self):
        ini = FRB.IniFile(txt = "[A]\n")
        ini.fileTxt = "[B]\nhash = x\n"

        self.assertEqual(ini.fileTxt, "[B]\nhash = x\n")
        self.assertEqual(len(ini.fileLines), 2)

    # ================================================
    # ================= download mode ================

    def test_downloadMode_acceptsEnumMemberAndBareString(self):
        # DownloadMode is a plain enum in C++ but a StrEnum in Python, so it crosses by value --
        # both the member and the string it carries have to work.
        for mode in (FRB.DownloadMode.Disabled, FRB.DownloadMode.Normal, FRB.DownloadMode.Always,
                     "disabled", "normal", "always", None):
            FRB.IniFile(txt = "", downloadMode = mode)

    def test_downloadMode_unknownValueRejected(self):
        with self.assertRaises(ValueError):
            FRB.IniFile(txt = "", downloadMode = "notAMode")

    # ================================================
    # =================== sections ===================

    def test_getSectionNames_inDeclarationOrder(self):
        ini = FRB.IniFile(txt = self._iniTxt)
        self.compareList(ini.getSectionNames(), ["TextureOverrideBody", "ResourceBody"])

    def test_getSection_foundAndMissing(self):
        ini = FRB.IniFile(txt = self._iniTxt)

        self.assertIsInstance(ini.getSection("TextureOverrideBody"), FRB.IfTemplate)
        self.assertIsNone(ini.getSection("NoSuchSection"))

    def test_removeSection_keepsOrderOfTheRest(self):
        ini = FRB.IniFile(txt = self._iniTxt)
        ini.removeSection("TextureOverrideBody")

        self.compareList(ini.getSectionNames(), ["ResourceBody"])

    def test_isSectionHeaderLine(self):
        self.assertTrue(FRB.IniFile.isSectionHeaderLine("[TextureOverrideBody]"))
        self.assertFalse(FRB.IniFile.isSectionHeaderLine("hash = abc123"))

    def test_getSectionNameFromLine(self):
        self.assertEqual(FRB.IniFile.getSectionNameFromLine("[TextureOverrideBody]"), "TextureOverrideBody")

    # ================================================
    # ================== classifying =================

    def test_classify_namesTheModType(self):
        # The global classifier arrives populated with every shipped mod type, so a real section
        # name resolves all the way to a CppModType -- this was None for every input until the
        # classifier was populated.
        ini = FRB.IniFile(txt = "[TextureOverrideRaidenBody]\nhash = abc123\n")
        ini.classify()

        self.assertIsNotNone(ini.availableType)
        self.assertEqual(ini.availableType.name, "Raiden")

    def test_classify_matchesRegardlessOfCase(self):
        # Registered keywords are lowercase; real sections are not.
        for sectionName in ("TextureOverrideRaidenBody", "textureoverrideraidenbody"):
            with self.subTest(sectionName = sectionName):
                ini = FRB.IniFile(txt = f"[{sectionName}]\n")
                ini.classify()
                self.assertEqual(ini.availableType.name, "Raiden")

    def test_classify_longerKeywordWins(self):
        # What the pure-Python builder needs a negative lookahead for -- here the maximal match
        # settles it, so an AmberCN section must never come back as Amber.
        ini = FRB.IniFile(txt = "[TextureOverrideAmberCNBody]\n")
        ini.classify()

        self.assertEqual(ini.availableType.name, "AmberCN")

    def test_classify_setsIsModIni(self):
        ini = FRB.IniFile(txt = self._iniTxt)
        self.assertFalse(ini.isClassified)

        ini.classify()

        self.assertTrue(ini.isClassified)
        self.assertTrue(ini.isModIni)

    def test_classify_nonModIni(self):
        ini = FRB.IniFile(txt = "[SomethingElse]\nkey = value\n")
        ini.classify()

        self.assertTrue(ini.isClassified)
        self.assertFalse(ini.isModIni)

    def test_isFixed_settable(self):
        ini = FRB.IniFile(txt = self._iniTxt)
        ini.isFixed = True
        self.assertTrue(ini.isFixed)

    # ================================================
    # ================== resources ===================

    def test_resourcesAndDownloads_emptyBeforeParse(self):
        ini = FRB.IniFile(txt = self._iniTxt)

        self.compareList(ini.getResources(), [])
        self.compareList(ini.getFileDownloads(), [])
        self.compareList(ini.getReferencedFolders(), [])

    def test_parse_returnsNone(self):
        # The parsed graph groups stay on the C++ side: the IniGraphGroup bound to Python is a
        # separate pybind-layer class, not the instantiation IniFile::parse produces.
        ini = FRB.IniFile(txt = self._iniTxt)
        ini.classify()

        self.assertIsNone(ini.parse())

    def test_clearModels_keepsReadText(self):
        ini = FRB.IniFile(txt = self._iniTxt)
        ini.clearModels()

        self.compareList(ini.getResources(), [])
        self.assertEqual(ini.fileTxt, self._iniTxt)
        self.assertTrue(ini.fileLinesRead)

    def test_clear_dropsClassificationAndText(self):
        ini = FRB.IniFile(txt = self._iniTxt)
        ini.classify()

        ini.clear()

        self.assertFalse(ini.isClassified)
        self.assertFalse(ini.isModIni)

    # ================================================
    # ==================== on disk ===================

    def _writeTempIni(self):
        handle, path = tempfile.mkstemp(suffix = ".ini")
        with os.fdopen(handle, "w", encoding = "utf-8") as file:
            file.write(self._iniTxt)

        self.addCleanup(lambda: os.path.exists(path) and os.remove(path))
        return path

    def test_fileConstruction_isLazy(self):
        path = self._writeTempIni()
        ini = FRB.IniFile(path)

        self.assertEqual(ini.file, path)
        self.assertFalse(ini.fileLinesRead)
        self.assertEqual(ini.fileTxt, "")

        self.assertEqual(len(ini.readFileLines()), 6)
        self.assertTrue(ini.fileLinesRead)

    def test_folder_derivedFromFile(self):
        path = self._writeTempIni()
        self.assertEqual(FRB.IniFile(path).folder, os.path.dirname(path))

    def test_write_roundTrips(self):
        path = self._writeTempIni()
        ini = FRB.IniFile(path)

        written = ini.write("[Rewritten]\n")
        self.assertEqual(written, "[Rewritten]\n")

        with open(path, "r", encoding = "utf-8") as file:
            self.assertEqual(file.read(), "[Rewritten]\n")

    def test_getReferencedFolders_afterSeedingResources(self):
        path = self._writeTempIni()
        folder = os.path.dirname(path)

        ini = FRB.IniFile(path)
        ini.getResources().append(FRB.IniResource("blend", folder, "blends/one.buf"))

        # getResources() hands out borrowed pointers into the .ini file's own storage, so
        # appending to the returned list does NOT reach it -- the folder list stays empty.
        self.compareList(ini.getReferencedFolders(), [])
