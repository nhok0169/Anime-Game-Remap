import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RegAddTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        edit = FRB.RegAdd([("a", "1"), ("b", "2")], latest = False)
        self.compareList(edit.vals, [("a", "1"), ("b", "2")])
        self.assertFalse(edit.latest)

    def test_init_latestDefaultsTrue(self):
        edit = FRB.RegAdd([("a", "1")])
        self.assertTrue(edit.latest)

    # ================================================
    # ===================== edit ======================

    def test_edit_emptyVals_partUnchanged(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegAdd([], latest = True)

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "1")])

    def test_edit_returnsTheSamePartInstance(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegAdd([("b", "2")], latest = True)

        result = edit.edit(part, "root", None)
        self.assertIs(result, part)

    def test_edit_noPartRangesLatestTrue_appendsToTrueEnd(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit = FRB.RegAdd([("c", "3"), ("d", "4")], latest = True)

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "1"), ("b", "2"), ("c", "3"), ("d", "4")])

    def test_edit_noPartRangesLatestFalse_prependsToTrueBeginning(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit = FRB.RegAdd([("c", "3"), ("d", "4")], latest = False)

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("c", "3"), ("d", "4"), ("a", "1"), ("b", "2")])

    def test_edit_partRangesLatestTrue_insertsRightAfterWindowEnd(self):
        # window covers true positional indices [1, 3) ("b", "c") -- the new vals must land right after "c",
        # before "d"/"e", not at the true end of the part
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")], "d": [(3, "4")], "e": [(4, "5")]}, 0)
        edit = FRB.RegAdd([("x", "10"), ("y", "20")], latest = True)

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(1, 3)]))
        self.compareList(part.entries(), [("a", "1"), ("b", "2"), ("c", "3"), ("x", "10"), ("y", "20"), ("d", "4"), ("e", "5")])

    def test_edit_partRangesLatestFalse_insertsRightBeforeWindowStart(self):
        # same window as above, but the new vals must land right before "b" (the window's own start), not at the
        # true beginning of the part
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")], "d": [(3, "4")], "e": [(4, "5")]}, 0)
        edit = FRB.RegAdd([("x", "10"), ("y", "20")], latest = False)

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(1, 3)]))
        self.compareList(part.entries(), [("a", "1"), ("x", "10"), ("y", "20"), ("b", "2"), ("c", "3"), ("d", "4"), ("e", "5")])

    def test_edit_partRangesUnboundedEndLatestTrue_fallsBackToTrueEnd(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit = FRB.RegAdd([("z", "9")], latest = True)

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(1, None)]))
        self.compareList(part.entries(), [("a", "1"), ("b", "2"), ("z", "9")])

    def test_edit_partRangesUnboundedStartLatestFalse_fallsBackToTrueBeginning(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit = FRB.RegAdd([("z", "9")], latest = False)

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(None, 1)]))
        self.compareList(part.entries(), [("z", "9"), ("a", "1"), ("b", "2")])

    def test_edit_multipleDisjointRangesLatestTrue_usesEndOfTheLastRange(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")], "d": [(3, "4")],
                                   "e": [(4, "5")], "f": [(5, "6")], "g": [(6, "7")]}, 0)
        edit = FRB.RegAdd([("x", "10")], latest = True)

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(1, 3), (5, 7)]))
        self.compareList(part.entries(), [("a", "1"), ("b", "2"), ("c", "3"), ("d", "4"), ("e", "5"), ("f", "6"),
                                           ("g", "7"), ("x", "10")])

    def test_edit_multipleDisjointRangesLatestFalse_usesStartOfTheFirstRange(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")], "d": [(3, "4")],
                                   "e": [(4, "5")], "f": [(5, "6")], "g": [(6, "7")]}, 0)
        edit = FRB.RegAdd([("x", "10")], latest = False)

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(1, 3), (5, 7)]))
        self.compareList(part.entries(), [("a", "1"), ("x", "10"), ("b", "2"), ("c", "3"), ("d", "4"), ("e", "5"),
                                           ("f", "6"), ("g", "7")])

    def test_edit_emptyPartRanges_partUnchanged(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegAdd([("z", "9")], latest = True)

        edit.edit(part, "root", None, partRanges = FRB.Ranges.createEmpty())
        self.compareList(part.entries(), [("a", "1")])

    def test_edit_fullPartRanges_behavesLikeNoPartRanges(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegAdd([("z", "9")], latest = True)

        edit.edit(part, "root", None, partRanges = FRB.Ranges.createFull())
        self.compareList(part.entries(), [("a", "1"), ("z", "9")])
