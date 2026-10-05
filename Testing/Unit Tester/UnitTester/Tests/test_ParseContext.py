import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ParseContextTest(BaseUnitTest):

    # =============== __init__ =======================

    def test_defaultInit_emptyContext(self):
        ctx = FRB.ParseContext()
        self.compareList(ctx.lines, [])
        self.assertIsNone(ctx.file)
        self.assertEqual(ctx.startLineNo, 1)

    def test_initFromStr_splitIntoLines(self):
        # asserted directly against Python's own str.splitlines() semantics, the contract this
        # constructor is meant to match (see StringTools::splitlines's trailing-newline fix)
        tests = ["", "abc", "abc\n", "abc\ndef", "abc\ndef\n", "abc\r\ndef\r\n", "a\n\nb", "\n", "\n\n"]
        for src in tests:
            ctx = FRB.ParseContext(src)
            self.compareList(ctx.lines, src.splitlines())

    def test_initFromList_storedAsGiven(self):
        lines = ["if $swapvar$ == 5", "endif"]
        ctx = FRB.ParseContext(lines)
        self.compareList(ctx.lines, lines)

    def test_initFromEmptyList_emptyLines(self):
        ctx = FRB.ParseContext([])
        self.compareList(ctx.lines, [])

    def test_initFile_preserved(self):
        ctx = FRB.ParseContext("abc", file = "merged.ini")
        self.assertEqual(ctx.file, "merged.ini")

    def test_initFile_defaultsToNone(self):
        ctx = FRB.ParseContext("abc")
        self.assertIsNone(ctx.file)

    def test_initStartLineNo_preserved(self):
        ctx = FRB.ParseContext("abc", startLineNo = 8)
        self.assertEqual(ctx.startLineNo, 8)

    def test_initStartLineNo_defaultsToOne(self):
        ctx = FRB.ParseContext("abc")
        self.assertEqual(ctx.startLineNo, 1)

    def test_initFromList_withFileAndStartLineNo(self):
        lines = ["a", "b", "c"]
        ctx = FRB.ParseContext(lines, file = "some.ini", startLineNo = 5)
        self.compareList(ctx.lines, lines)
        self.assertEqual(ctx.file, "some.ini")
        self.assertEqual(ctx.startLineNo, 5)

    # ================================================
    # ================ getEndLineNo ===================

    def test_getEndLineNo_startPlusLineCount(self):
        tests = [(FRB.ParseContext("a\nb\nc", startLineNo = 1), 4),
                  (FRB.ParseContext("a\nb\nc", startLineNo = 8), 11),
                  (FRB.ParseContext(""), 1),
                  (FRB.ParseContext([], startLineNo = 3), 3)]

        for ctx, expected in tests:
            self.assertEqual(ctx.getEndLineNo(), expected)

    def test_getEndLineNo_reflectsMutatedLines(self):
        ctx = FRB.ParseContext("a\nb", startLineNo = 1)
        ctx.lines = ["a", "b", "c", "d"]
        self.assertEqual(ctx.getEndLineNo(), 5)

    # ================================================
    # ================ attributes =====================

    def test_mutateLines_valueUpdated(self):
        ctx = FRB.ParseContext("a\nb")
        ctx.lines = ["x", "y", "z"]
        self.compareList(ctx.lines, ["x", "y", "z"])

    def test_mutateStartLineNo_valueUpdated(self):
        ctx = FRB.ParseContext("a")
        ctx.startLineNo = 42
        self.assertEqual(ctx.startLineNo, 42)

    def test_mutateFile_valueUpdated(self):
        ctx = FRB.ParseContext("a")
        ctx.file = "new.ini"
        self.assertEqual(ctx.file, "new.ini")

        ctx.file = None
        self.assertIsNone(ctx.file)

    # ================================================
    # ================ independence ===================

    def test_separateInstances_areIndependent(self):
        ctx1 = FRB.ParseContext("a\nb")
        ctx2 = FRB.ParseContext("a\nb")

        # 'lines' is copied out to a fresh Python list on every access (the plain
        # std::vector<std::string> pybind11 caster has no live-view semantics, unlike a real
        # Python list attribute) -- so independence has to be checked via whole-value
        # reassignment, not in-place mutation like '.append()'.
        ctx1.lines = ctx1.lines + ["c"]
        self.compareList(ctx1.lines, ["a", "b", "c"])
        self.compareList(ctx2.lines, ["a", "b"])
