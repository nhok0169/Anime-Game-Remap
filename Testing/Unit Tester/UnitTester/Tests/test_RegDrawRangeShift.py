##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
import sys
##### EndExtImports

##### LocalImports
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
##### EndLocalImports


TexFxStart = "$\\texfx\\_2"


class RegDrawRangeShiftTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        edit = FRB.RegDrawRangeShift(30, 12, [TexFxStart])

        self.assertEqual(edit.offset, 30)
        self.assertEqual(edit.count, 12)
        self.assertEqual(edit.startKeys, [TexFxStart])

    def test_init_defaults(self):
        edit = FRB.RegDrawRangeShift()

        self.assertEqual(edit.offset, 0)
        self.assertEqual(edit.count, 0)
        self.assertEqual(edit.startKeys, [])

    def test_isSubclassOfBaseRegEdit(self):
        self.assertTrue(issubclass(FRB.RegDrawRangeShift, FRB.BaseRegEdit))

    # ================================================
    # ================== shiftRange ===================

    def test_shiftRange_startMovedRestKept(self):
        self.assertEqual(FRB.RegDrawRangeShift.shiftRange("6, 0, 0", 30, 12), "6, 30, 0")
        self.assertEqual(FRB.RegDrawRangeShift.shiftRange(" 6 ,6", 30, 12), "6, 36")
        self.assertEqual(FRB.RegDrawRangeShift.shiftRange("$n, 4, 7, 1", 30, 12), "$n, 34, 7, 1")

    def test_shiftRange_autoIsTheWholeRange(self):
        self.assertEqual(FRB.RegDrawRangeShift.shiftRange(" Auto ", 30, 12), "12, 30, 0")

    def test_shiftRange_startNotAPlainNumber_untouched(self):
        for value in ["6", "6, $s, 0", "6, -3, 0", "6, , 0", ""]:
            self.assertEqual(FRB.RegDrawRangeShift.shiftRange(value, 30, 12), value, value)

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSamePartInstance(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "6, 0, 0")]}, 0)
        self.assertIs(FRB.RegDrawRangeShift(30, 12).edit(part, "root", None), part)

    def test_edit_everyDrawMoved_otherKeysUntouched(self):
        part = FRB.IfContentPart({"ib": [(0, "Ib")], "drawindexed": [(1, "6, 0, 0"), (2, "auto"), (3, "$n, $s, 0")]}, 0)

        FRB.RegDrawRangeShift(30, 12).edit(part, "root", None)

        self.compareList(part.entries(), [("ib", "Ib"), ("drawindexed", "6, 30, 0"), ("drawindexed", "12, 30, 0"),
                                          ("drawindexed", "$n, $s, 0")])

    def test_edit_startKeys_matchedCaseInsensitively_plainNumbersMoved(self):
        part = FRB.IfContentPart({"$\\TexFx\\_2": [(0, "6"), (2, "$other")], "$\\texfx\\_1": [(1, "9")]}, 0)

        FRB.RegDrawRangeShift(30, 12, [TexFxStart]).edit(part, "root", None)

        self.compareList(part.entries(), [("$\\TexFx\\_2", "36"), ("$\\texfx\\_1", "9"), ("$\\TexFx\\_2", "$other")])

    def test_edit_noStartKeys_leavesThemAlone(self):
        part = FRB.IfContentPart({"$\\texfx\\_2": [(0, "6")]}, 0)

        FRB.RegDrawRangeShift(30, 12).edit(part, "root", None)

        self.compareList(part.entries(), [("$\\texfx\\_2", "6")])

    def test_edit_reassignedAttributes_takeEffect(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "auto")]}, 0)
        edit = FRB.RegDrawRangeShift(1, 1)

        edit.offset = 5
        edit.count = 9
        edit.edit(part, "root", None)

        self.compareList(part.entries(), [("drawindexed", "9, 5, 0")])
