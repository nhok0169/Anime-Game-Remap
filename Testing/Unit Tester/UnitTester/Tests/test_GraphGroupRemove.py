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


class GraphGroupRemoveTest(BaseUnitTest):
    def _makeGraph(self, name: str) -> FRB.IniSectionGraph:
        return FRB.IniSectionGraph({name: FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = name)}, [name])

    def _makeGroups(self, count: int):
        return [FRB.IniGraphGroup({("comp", f"obj{i}"): self._makeGraph(f"obj{i}")}) for i in range(count)]

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        iniIndices = [0, 2]
        edit = FRB.GraphGroupRemove(iniIndices)
        self.assertIs(edit.iniIndices, iniIndices)

    def test_init_default_everyGroup(self):
        self.assertIsNone(FRB.GraphGroupRemove().iniIndices)

    def test_isSubclassOfBaseIniGraphGroupEdit(self):
        self.assertTrue(issubclass(FRB.GraphGroupRemove, FRB.BaseIniGraphGroupEdit))

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSameGraphGroupsList(self):
        graphGroups = self._makeGroups(2)
        self.assertIs(FRB.GraphGroupRemove([0]).edit(graphGroups, None), graphGroups)

    def test_edit_noIndices_removesEveryGroup(self):
        graphGroups = self._makeGroups(3)

        FRB.GraphGroupRemove().edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 0)

    def test_edit_indices_removesOnlyThoseAndKeepsTheOrder(self):
        graphGroups = self._makeGroups(4)
        kept = [graphGroups[1], graphGroups[3]]

        FRB.GraphGroupRemove([2, 0]).edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 2)
        self.assertIs(graphGroups[0], kept[0])
        self.assertIs(graphGroups[1], kept[1])

    def test_edit_indicesMeanTheGroupsBeforeTheEdit(self):
        # removing 0 first must not make "1" name what used to be group 2
        graphGroups = self._makeGroups(3)
        kept = graphGroups[2]

        FRB.GraphGroupRemove([0, 1]).edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)
        self.assertIs(graphGroups[0], kept)

    def test_edit_outOfRangeNegativeAndRepeatedIndices_skippedSilently(self):
        graphGroups = self._makeGroups(2)
        kept = graphGroups[0]

        FRB.GraphGroupRemove([1, 1, 7, -1]).edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)
        self.assertIs(graphGroups[0], kept)

    def test_edit_emptyIndices_noOp(self):
        graphGroups = self._makeGroups(2)

        FRB.GraphGroupRemove([]).edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 2)

    def test_edit_reassignedIndices_takeEffect(self):
        graphGroups = self._makeGroups(2)
        kept = graphGroups[0]
        edit = FRB.GraphGroupRemove([0])

        edit.iniIndices = [1]
        edit.edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)
        self.assertIs(graphGroups[0], kept)
