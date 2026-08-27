import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseRegEditTest(BaseUnitTest):
    # ================================================
    # ================== inheritance ===================

    def test_inheritance_derivesFromTheGraphPartEditBases(self):
        edit = FRB.BaseRegEdit()
        self.assertIsInstance(edit, FRB.CppBaseIniGraphPartEdit)
        self.assertIsInstance(edit, FRB.CppBaseIniPartEdit)

    def test_inheritance_everyConcreteRegEditIsABaseRegEdit(self):
        for edit in [FRB.RegAdd([]), FRB.RegNewVals({}), FRB.RegRemap({}), FRB.RegRemove({})]:
            self.assertIsInstance(edit, FRB.BaseRegEdit)

    # ================================================
    # ===================== clear =====================

    def test_clear_isANoOp(self):
        self.assertIsNone(FRB.BaseRegEdit().clear())
        self.assertIsNone(FRB.RegAdd([("a", "1")]).clear())

    # ================================================
    # ===================== edit ======================

    def test_edit_partUnchangedAndReturned(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.BaseRegEdit()

        result = edit.edit(part, "root", None)
        self.assertIs(result, part)
        self.compareList(part.entries(), [("a", "1")])

    def test_edit_partRangesIgnoredByTheNoOpBase(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.BaseRegEdit()

        edit.edit(part, "root", None, modName = "mod", partRanges = FRB.Ranges([(0, 1)]))
        self.compareList(part.entries(), [("a", "1")])

    # ================================================
    # ================== editFromIni ==================

    def test_editFromIni_partUnchangedAndReturned(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.BaseRegEdit()

        result = edit.editFromIni(part, "root", None, None)
        self.assertIs(result, part)
        self.compareList(part.entries(), [("a", "1")])

    def test_editFromIni_forwardsToTheConcreteSubclassEdit(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegAdd([("b", "2")])

        result = edit.editFromIni(part, "root", None, None)
        self.assertIs(result, part)
        self.compareList(part.entries(), [("a", "1"), ("b", "2")])

    def test_editFromIni_forwardsEveryArgumentToEdit(self):
        seen = {}

        class RecordingRegEdit(FRB.BaseRegEdit):
            def edit(self, part, sectionName, modType, modName = "", partRanges = None):
                seen["sectionName"] = sectionName
                seen["modType"] = modType
                seen["modName"] = modName
                seen["partRanges"] = partRanges
                return part

        partRanges = FRB.Ranges([(0, 1)])
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)

        RecordingRegEdit().editFromIni(part, "root", "theIni", "theModType", modName = "theMod", partRanges = partRanges)

        self.assertEqual(seen["sectionName"], "root")
        self.assertEqual(seen["modType"], "theModType")
        self.assertEqual(seen["modName"], "theMod")
        self.assertIs(seen["partRanges"], partRanges)

    def test_editFromIni_pythonSubclassOverridingOnlyEditIsStillCalled(self):
        # 'editFromIni' resolves 'edit' through ordinary Python attribute lookup, so a pure-Python
        # subclass that overrides only 'edit' (and not 'editFromIni') is still reached
        class AddsAMarker(FRB.BaseRegEdit):
            def edit(self, part, sectionName, modType, modName = "", partRanges = None):
                part.addKVP("marker", sectionName)
                return part

        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        result = AddsAMarker().editFromIni(part, "root", None, None)

        self.assertIs(result, part)
        self.compareList(part.entries(), [("a", "1"), ("marker", "root")])

    # ================================================
    # ================== partRanges ===================

    def test_edit_partRangesAcceptsARawListOfBounds(self):
        # every reg edit accepts either a bound Ranges or the raw list of (start, end) bounds
        # underneath it, matching how IfContentPart's own ranges-taking methods behave
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2"), (2, "3")]}, 0)
        FRB.RegRemove({"a": None}).edit(part, "root", None, partRanges = [(1, 2)])

        self.compareList(part.entries(), [("a", "1"), ("a", "3")])
