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


_Z3CTX = FRB.Z3Context()  # shared across every IfPredPart built in this test file


# RegBottomAdd is C++-backed: its additions land in a fresh last part at each root's own depth -- or,
# with a condition, in a new `if <condition>` ... `endif` block there.
class RegBottomAddTest(BaseUnitTest):
    def _toggled(self):
        """a draw, then `if $x == 1` another draw `endif` -- the shape whose last part is inside a block"""
        return FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "10, 0, 0")]}, 0),
                               FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
                               FRB.IfContentPart({"drawindexed": [(0, "5, 10, 0")]}, 1),
                               FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX)])

    def _shape(self, section: FRB.IfTemplate):
        """each part as ('if'/'endif' src) or (depth, entries)"""
        result = []
        for part in section.parts:
            if (isinstance(part, FRB.IfContentPart)):
                result.append((part.depth, part.entries()))
            else:
                result.append(part.src.strip())
        return result

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        edit = FRB.RegBottomAdd([("run", "CommandListX")], condition = "vs != 037730.0")
        self.compareList(edit.additions, [("run", "CommandListX")])
        self.assertEqual(edit.condition, "vs != 037730.0")

    def test_init_default_noCondition(self):
        self.assertEqual(FRB.RegBottomAdd([("run", "CommandListX")]).condition, "")

    def test_isSubclassOfBaseIniGraphEdit(self):
        self.assertTrue(issubclass(FRB.RegBottomAdd, FRB.BaseIniGraphEdit))

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSameGraphObject(self):
        graph = FRB.IniSectionGraph({"root": self._toggled()}, ["root"], z3Ctx = _Z3CTX)
        self.assertIs(FRB.RegBottomAdd([("x", "1")]).edit(graph, None), graph)

    def test_edit_noCondition_freshPartAtTheSectionsDepth(self):
        section = self._toggled()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegBottomAdd([("drawindexed", "3, 15, 0")]).edit(graph, None)

        self.compareList(self._shape(section), [
            (0, [("drawindexed", "10, 0, 0")]),
            "if $x == 1",
            (1, [("drawindexed", "5, 10, 0")]),
            "endif",
            (0, [("drawindexed", "3, 15, 0")])])

    def test_edit_condition_newIfBlockAtTheSectionsDepth(self):
        section = self._toggled()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegBottomAdd([("ps-t1", "ResourceDress"), ("drawindexed", "3, 15, 0")], condition = "vs != 037730.0").edit(graph, None)

        self.compareList(self._shape(section), [
            (0, [("drawindexed", "10, 0, 0")]),
            "if $x == 1",
            (1, [("drawindexed", "5, 10, 0")]),
            "endif",
            "if vs != 037730.0",
            (1, [("ps-t1", "ResourceDress"), ("drawindexed", "3, 15, 0")]),
            "endif"])

    def test_edit_condition_nothingExistingMovesIntoTheBlock(self):
        section = FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "10, 0, 0")]}, 0)])
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegBottomAdd([("drawindexed", "3, 10, 0")], condition = "vs != 037730.0").edit(graph, None)

        self.compareList(self._shape(section), [
            (0, [("drawindexed", "10, 0, 0")]),
            "if vs != 037730.0",
            (1, [("drawindexed", "3, 10, 0")]),
            "endif"])

    def test_edit_twoEdits_blocksInOrder(self):
        section = FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "10, 0, 0")]}, 0)])
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegBottomAdd([("drawindexed", "3, 10, 0")], condition = "vs != 037730.0").edit(graph, None)
        FRB.RegBottomAdd([("drawindexed", "2, 13, 0")]).edit(graph, None)

        self.compareList(self._shape(section), [
            (0, [("drawindexed", "10, 0, 0")]),
            "if vs != 037730.0",
            (1, [("drawindexed", "3, 10, 0")]),
            "endif",
            (0, [("drawindexed", "2, 13, 0")])])

    def test_edit_noAdditions_noOp(self):
        section = self._toggled()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegBottomAdd([], condition = "vs != 037730.0").edit(graph, None)

        self.assertEqual(len(section.parts), 4)
