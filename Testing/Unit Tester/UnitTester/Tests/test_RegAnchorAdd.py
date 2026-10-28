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


def markFirstDraw(key, value):
    return [("run", "CommandListMark")] if value == "6, 0, 0" else None


class RegAnchorAddTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        edit = FRB.RegAnchorAdd(["drawindexed"], markFirstDraw, FRB.RegAnchorAdd.Placement.Before)

        self.assertEqual(edit.anchorKeys, ["drawindexed"])
        self.assertIs(edit.adder, markFirstDraw)
        self.assertEqual(edit.placement, FRB.RegAnchorAdd.Placement.Before)
        self.assertIsNone(edit.anchorIf)

    def test_init_defaults(self):
        edit = FRB.RegAnchorAdd()

        self.assertEqual(edit.anchorKeys, [])
        self.assertIsNone(edit.adder)
        self.assertEqual(edit.placement, FRB.RegAnchorAdd.Placement.After)
        self.assertIsNone(edit.anchorIf)
        self.assertFalse(edit.firstOnly)
        self.assertFalse(edit.skipExisting)

    def test_isSubclassOfBaseRegEdit(self):
        self.assertTrue(issubclass(FRB.RegAnchorAdd, FRB.BaseRegEdit))

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSamePartInstance(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "6, 0, 0")]}, 0)
        self.assertIs(FRB.RegAnchorAdd(["drawindexed"], markFirstDraw).edit(part, "root", None), part)

    def test_edit_after_onlyTheAnchorsTheAdderAnswers(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "6, 0, 0"), (1, "3, 6, 0")], "ib": [(2, "Ib")]}, 0)

        FRB.RegAnchorAdd(["drawindexed"], markFirstDraw).edit(part, "root", None)

        self.compareList(part.entries(), [("drawindexed", "6, 0, 0"), ("run", "CommandListMark"), ("drawindexed", "3, 6, 0"), ("ib", "Ib")])

    def test_edit_before_linesKeepTheirOrder(self):
        part = FRB.IfContentPart({"run": [(0, "CommandListA"), (1, "CommandListCleanup")]}, 0)
        adder = lambda key, value: [("run", "CommandListFirst"), ("post $latch", "0")] if value.strip().lower() == "commandlistcleanup" else None

        FRB.RegAnchorAdd(["run"], adder, FRB.RegAnchorAdd.Placement.Before).edit(part, "root", None)

        self.compareList(part.entries(), [("run", "CommandListA"), ("run", "CommandListFirst"), ("post $latch", "0"), ("run", "CommandListCleanup")])

    def test_edit_replace_everyAnchorVisitedOnce(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "6, 0, 0"), (1, "3, 6, 0")]}, 0)
        adder = lambda key, value: [("run", "Draw" + value.split(",")[0]), ("drawindexed", value)]

        FRB.RegAnchorAdd(["drawindexed"], adder, FRB.RegAnchorAdd.Placement.Replace).edit(part, "root", None)

        self.compareList(part.entries(), [("run", "Draw6"), ("drawindexed", "6, 0, 0"), ("run", "Draw3"), ("drawindexed", "3, 6, 0")])

    def test_edit_replace_emptyListRemovesTheAnchor(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "ResourceA")], "ps-t1": [(1, "ResourceB")]}, 0)

        FRB.RegAnchorAdd(["ps-t0"], lambda key, value: [], FRB.RegAnchorAdd.Placement.Replace).edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t1", "ResourceB")])

    def test_edit_anchorKeys_matchedCaseInsensitively(self):
        part = FRB.IfContentPart({" DrawIndexed ": [(0, "6, 0, 0")]}, 0)

        FRB.RegAnchorAdd(["drawindexed"], markFirstDraw).edit(part, "root", None)

        self.compareList(part.entries(), [(" DrawIndexed ", "6, 0, 0"), ("run", "CommandListMark")])

    def test_edit_anchorIf_decidesInsteadOfTheKeys(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "ResourceA")], "ps-t1": [(1, "ResourceB")], "vb0": [(2, "ResourceC")]}, 0)
        rekey = lambda key, value: [("ps-t" + str(int(key[-1]) + 2), value)]

        FRB.RegAnchorAdd([], rekey, FRB.RegAnchorAdd.Placement.Replace, lambda key: key.startswith("ps-t")).edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t2", "ResourceA"), ("ps-t3", "ResourceB"), ("vb0", "ResourceC")])

    def test_edit_partRanges_onlyAnchorsInsideThem(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "6, 0, 0"), (1, "6, 0, 0")]}, 0)

        FRB.RegAnchorAdd(["drawindexed"], markFirstDraw).edit(part, "root", None, partRanges = FRB.Ranges([(1, 2)]))

        self.compareList(part.entries(), [("drawindexed", "6, 0, 0"), ("drawindexed", "6, 0, 0"), ("run", "CommandListMark")])

    def test_edit_firstOnly_onlyTheFirstAnchor(self):
        part = FRB.IfContentPart({"vb1": [(1, "ResourceA"), (3, "ResourceB")], "ib": [(0, "Ib"), (2, "Ib2")]}, 0)
        capture = lambda key, value: [("ResourceBypassVb1", "ref vb1")]

        FRB.RegAnchorAdd(["vb1"], capture, FRB.RegAnchorAdd.Placement.Before, firstOnly = True).edit(part, "root", None)

        self.compareList(part.entries(), [("ib", "Ib"), ("ResourceBypassVb1", "ref vb1"), ("vb1", "ResourceA"), ("ib", "Ib2"), ("vb1", "ResourceB")])

    def test_edit_skipExisting_aRepeatedVisitAddsNothing(self):
        part = FRB.IfContentPart({"vb1": [(0, "ResourceA")]}, 0)
        edit = FRB.RegAnchorAdd(["vb1"], lambda key, value: [("ResourceBypassVb1", "ref vb1")], FRB.RegAnchorAdd.Placement.Before,
                                firstOnly = True, skipExisting = True)

        edit.edit(part, "root", None)
        edit.edit(part, "root", None)

        self.compareList(part.entries(), [("ResourceBypassVb1", "ref vb1"), ("vb1", "ResourceA")])

    def test_holds_comparesCaseInsensitivelyAndStripped(self):
        part = FRB.IfContentPart({"ResourceBypassVb1": [(0, " Ref VB1 ")]}, 0)

        self.assertTrue(FRB.RegAnchorAdd.holds(part, "resourcebypassvb1", "ref vb1"))
        self.assertFalse(FRB.RegAnchorAdd.holds(part, "resourcebypassvb1", "ref vb2"))

    def test_edit_noAdder_leavesThePartAlone(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "6, 0, 0")]}, 0)

        FRB.RegAnchorAdd(["drawindexed"]).edit(part, "root", None)

        self.compareList(part.entries(), [("drawindexed", "6, 0, 0")])

    def test_edit_reassignedAttributes_takeEffect(self):
        part = FRB.IfContentPart({"drawindexed": [(0, "6, 0, 0")]}, 0)
        edit = FRB.RegAnchorAdd()

        edit.anchorKeys = ["drawindexed"]
        edit.adder = markFirstDraw
        edit.placement = FRB.RegAnchorAdd.Placement.Before
        edit.edit(part, "root", None)

        self.compareList(part.entries(), [("run", "CommandListMark"), ("drawindexed", "6, 0, 0")])
