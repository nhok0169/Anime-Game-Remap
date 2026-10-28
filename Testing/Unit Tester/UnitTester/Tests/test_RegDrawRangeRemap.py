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


# triangles 3 and 4 removed
Kept = [0, 1, 2, 5, 6, 7]


class RegDrawRangeRemapTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        reroute = lambda first, count, remapped: None
        edit = FRB.RegDrawRangeRemap(Kept, reroute)

        self.assertEqual(edit.keptTriangleIds, Kept)
        self.assertIs(edit.reroute, reroute)

    def test_init_defaults(self):
        edit = FRB.RegDrawRangeRemap()

        self.assertEqual(edit.keptTriangleIds, [])
        self.assertIsNone(edit.reroute)

    def test_isSubclassOfBaseRegEdit(self):
        self.assertTrue(issubclass(FRB.RegDrawRangeRemap, FRB.BaseRegEdit))

    # ================================================
    # ============= parseRange / remapRange ==========

    def test_parseRange_numbersInOrder(self):
        self.assertEqual(FRB.RegDrawRangeRemap.parseRange("9, 3, 0"), [9, 3, 0])
        self.assertEqual(FRB.RegDrawRangeRemap.parseRange(" 6 ,0"), [6, 0])

    def test_parseRange_notARange(self):
        for value in ["auto", "9", "9, x", "-3, 0", "3, -1", "", "9,,0"]:
            self.assertIsNone(FRB.RegDrawRangeRemap.parseRange(value), value)

    def test_remapRange_throughTheKeptTriangles(self):
        self.assertEqual(FRB.RegDrawRangeRemap.remapRange("9, 0, 0", Kept), "9, 0, 0")
        self.assertEqual(FRB.RegDrawRangeRemap.remapRange("9, 9, 0", Kept), "3, 9, 0")
        self.assertEqual(FRB.RegDrawRangeRemap.remapRange("6, 18", Kept), "6, 12")
        self.assertEqual(FRB.RegDrawRangeRemap.remapRange("6, 9, 4, 7", Kept), "0, 9, 4, 7")

    def test_remapRange_notARange_isNone(self):
        self.assertIsNone(FRB.RegDrawRangeRemap.remapRange("auto", Kept))

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSamePartInstance(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "9, 0, 0")]}, 0)
        self.assertIs(FRB.RegDrawRangeRemap(Kept).edit(part, "root", None), part)

    def test_edit_everyRangeRemapped_autoAndOtherKeysUntouched(self):
        part = FRB.IfContentPart({"ib": [(0, "Ib")], "drawindexed": [(1, "9, 0, 0"), (2, "9, 9, 0"), (3, "auto")]}, 0)

        FRB.RegDrawRangeRemap(Kept).edit(part, "root", None)

        self.compareList(part.entries(), [("ib", "Ib"), ("drawindexed", "9, 0, 0"), ("drawindexed", "3, 9, 0"),
                                          ("drawindexed", "auto")])

    def test_edit_reroute_replacesTheLineByARunAtItsPlace(self):
        asked = []

        def reroute(first, count, remapped):
            asked.append((first, count, remapped))
            return "CustomShaderX" if first == 9 else None

        part = FRB.IfContentPart({"drawindexed": [(0, "9, 0, 0"), (2, "9, 9, 0"), (3, "auto")], "ps-t0": [(1, "A")]}, 0)

        FRB.RegDrawRangeRemap(Kept, reroute).edit(part, "root", None)

        self.compareList(asked, [(0, 9, "9, 0, 0"), (9, 9, "3, 9, 0")])
        self.compareList(part.entries(), [("drawindexed", "9, 0, 0"), ("ps-t0", "A"), ("run", "CustomShaderX"),
                                          ("drawindexed", "auto")])

    def test_edit_noDraws_changesNothing(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "A")]}, 0)

        FRB.RegDrawRangeRemap(Kept, lambda first, count, remapped: "X").edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t0", "A")])

    def test_edit_reassignedAttributes_takeEffect(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "6, 6, 0")]}, 0)
        edit = FRB.RegDrawRangeRemap([0, 1, 2])

        edit.keptTriangleIds = [0, 2]
        edit.reroute = lambda first, count, remapped: "Run" + remapped.replace(", ", "_")
        edit.edit(part, "root", None)

        self.compareList(part.entries(), [("run", "Run3_3_0")])
