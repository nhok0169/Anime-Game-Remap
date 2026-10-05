import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GraphRemoveTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        graphIds = [(0, "comp", "a")]
        edit = FRB.GraphRemove(graphIds)
        self.assertIs(edit.graphIds, graphIds)

    # ================================================
    # ===================== edit ======================

    def _makeGraph(self, name: str) -> FRB.IniSectionGraph:
        return FRB.IniSectionGraph({name: FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = name)}, [name])

    def test_edit_removesExistingGraph(self):
        group = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a"), ("comp", "b"): self._makeGraph("b")})
        graphGroups = [group]

        edit = FRB.GraphRemove([(0, "comp", "a")])
        edit.edit(graphGroups, None)

        self.assertNotIn(("comp", "a"), group.graphs)
        self.assertIn(("comp", "b"), group.graphs)

    def test_edit_returnsTheSameGraphGroupsList(self):
        group = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})
        graphGroups = [group]

        edit = FRB.GraphRemove([(0, "comp", "a")])
        result = edit.edit(graphGroups, None)

        self.assertIs(result, graphGroups)

    def test_edit_idNotFound_skippedSilently(self):
        group = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})
        graphGroups = [group]

        edit = FRB.GraphRemove([(0, "comp", "missing")])
        edit.edit(graphGroups, None)

        self.assertIn(("comp", "a"), group.graphs)

    def test_edit_iniIndexOutOfBounds_skippedSilently(self):
        group = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})
        graphGroups = [group]

        edit = FRB.GraphRemove([(5, "comp", "a")])
        edit.edit(graphGroups, None)

        self.assertIn(("comp", "a"), group.graphs)

    def test_edit_multipleIds_removesOnlyTheFoundOnes(self):
        groupA = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a"), ("comp", "b"): self._makeGraph("b")})
        groupB = FRB.IniGraphGroup({("comp", "c"): self._makeGraph("c")})
        graphGroups = [groupA, groupB]

        edit = FRB.GraphRemove([(0, "comp", "a"), (0, "comp", "missing"), (1, "comp", "c"), (9, "comp", "x")])
        edit.edit(graphGroups, None)

        self.assertNotIn(("comp", "a"), groupA.graphs)
        self.assertIn(("comp", "b"), groupA.graphs)
        self.assertNotIn(("comp", "c"), groupB.graphs)

    def test_edit_emptyGraphIds_noOp(self):
        group = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})
        graphGroups = [group]

        edit = FRB.GraphRemove([])
        edit.edit(graphGroups, None)

        self.assertIn(("comp", "a"), group.graphs)
