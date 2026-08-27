import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IniFixResourceModelTest(BaseUnitTest):
    """
    Tests for :class:`IniFixResourceModel` -- the C++-backed replacement for the pure-Python
    original, now deleted outright (was briefly renamed to ``IniFixResourceModelOld``
    mid-migration) :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        Deliberately does **not** inherit :class:`BaseFileUnitTest` -- that class mocks
        :mod:`os`/:mod:`shutil` at the Python level, which the new C++-backed
        :meth:`FileService.absPathOfRelPath` (used internally to resolve :attr:`fullPaths`/
        :attr:`origFullPaths`) never goes through (real ``std::filesystem`` calls from C++). Uses
        real absolute paths instead, matching how this migration's other new test files
        (``test_FileDownload.py``, ``test_IniResource.py``) handle the same C++-vs-mocked-Python
        path-resolution mismatch.
    """

    def test_noOrigPaths_fullPathsResolved(self):
        fixedPaths = {1: {"Type1": ["hello.buf"], "Type2": ["bye.buf"]},
                      3: {"Calc1": ["nested/value.buf"]}}

        model = FRB.IniFixResourceModel("C:/mods/EiRemap", fixedPaths)

        self.assertEqual(model.fullPaths[1]["Type1"], ["C:\\mods\\EiRemap\\hello.buf"])
        self.assertEqual(model.fullPaths[1]["Type2"], ["C:\\mods\\EiRemap\\bye.buf"])
        self.assertEqual(model.fullPaths[3]["Calc1"], ["C:\\mods\\EiRemap\\nested\\value.buf"])
        self.compareDict(model.origFullPaths, {})

    def test_origPaths_origFullPathsAlsoResolved(self):
        fixedPaths = {1: {"Type1": ["hello.buf"]}}
        origPaths = {1: ["orig.buf"], 2: ["another/orig.buf"]}

        model = FRB.IniFixResourceModel("C:/mods/EiRemap", fixedPaths, origPaths = origPaths)

        self.assertEqual(model.origFullPaths[1], ["C:\\mods\\EiRemap\\orig.buf"])
        self.assertEqual(model.origFullPaths[2], ["C:\\mods\\EiRemap\\another\\orig.buf"])

    def test_items_yieldsFixedFullOrigOrigFullTuples(self):
        fixedPaths = {0: {"Type1": ["hello.buf"]}}
        origPaths = {0: ["orig.buf"]}

        model = FRB.IniFixResourceModel("C:/mods/EiRemap", fixedPaths, origPaths = origPaths)
        entries = model.items()

        self.assertEqual(len(entries), 1)
        fixedPath, fullPath, origPath, origFullPath = entries[0]
        self.assertEqual(fixedPath, "hello.buf")
        self.assertEqual(fullPath, "C:\\mods\\EiRemap\\hello.buf")
        self.assertEqual(origPath, "orig.buf")
        self.assertEqual(origFullPath, "C:\\mods\\EiRemap\\orig.buf")

    def test_items_noOrigPaths_origEntriesAreNone(self):
        fixedPaths = {0: {"Type1": ["hello.buf"]}}

        model = FRB.IniFixResourceModel("C:/mods/EiRemap", fixedPaths)
        _, _, origPath, origFullPath = model.items()[0]

        self.assertIsNone(origPath)
        self.assertIsNone(origFullPath)

    def test_clear_clearsAllPathData(self):
        model = FRB.IniFixResourceModel("C:/mods/EiRemap", {0: {"Type1": ["hello.buf"]}}, origPaths = {0: ["orig.buf"]})

        model.clear()

        self.compareDict(model.fixedPaths, {})
        self.compareDict(model.fullPaths, {})
        self.compareDict(model.origFullPaths, {})
        self.compareDict(model.origPaths, {})

    def test_isInstanceOfIniResourceModel(self):
        model = FRB.IniFixResourceModel("C:/mods/EiRemap", {})
        self.assertIsInstance(model, FRB.IniResourceModel)
