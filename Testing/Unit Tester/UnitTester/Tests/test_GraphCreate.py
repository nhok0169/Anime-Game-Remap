import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GraphCreateTest(BaseUnitTest):
    def _makeGraph(self, name: str, value: str = "1", z3Ctx = None) -> FRB.IniSectionGraph:
        sections = {name: FRB.IfTemplate([FRB.IfContentPart({"a": [(0, value)]}, 0)], name = name)}
        if (z3Ctx is None):
            return FRB.IniSectionGraph(sections, [name])

        return FRB.IniSectionGraph(sections, [name], z3Ctx = z3Ctx)

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        graphId = (0, "comp", "a")
        graph = self._makeGraph("a")
        edit = FRB.GraphCreate(graphId, graph)

        self.assertIs(edit.graphId, graphId)
        self.assertIs(edit.graph, graph)
        self.assertTrue(edit.minimal)
        self.assertTrue(edit.newPartIds)

    def test_init_copyOptions_areKept(self):
        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a"), minimal = False, newPartIds = False)

        self.assertFalse(edit.minimal)
        self.assertFalse(edit.newPartIds)

    def test_init_graphDefaultsToNone(self):
        edit = FRB.GraphCreate((0, "comp", "a"))
        self.assertIsNone(edit.graph)

    # ================================================
    # ===================== edit ======================

    def test_edit_addsGraphToAnExistingGroup(self):
        group = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})
        graphGroups = [group]

        edit = FRB.GraphCreate((0, "comp", "b"), self._makeGraph("b"))
        edit.edit(graphGroups, None)

        self.assertIn(("comp", "a"), group.graphs)
        self.assertIn(("comp", "b"), group.graphs)

    def test_edit_existingGroup_doesNotAppendAnotherGroup(self):
        # The group count is the half of "appends EXACTLY the one group the index asks for" that the
        # other tests miss: a condition that inserts whenever the index is in range still adds the
        # graph to the right place, and only the length shows it
        group = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})
        graphGroups = [group]

        FRB.GraphCreate((0, "comp", "b"), self._makeGraph("b")).edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)
        self.assertIs(graphGroups[0], group)

    def test_edit_returnsTheSameGraphGroupsList(self):
        graphGroups = [FRB.IniGraphGroup({})]

        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a"))
        result = edit.edit(graphGroups, None)

        self.assertIs(result, graphGroups)

    def test_edit_iniIndexEqualToTheLength_appendsAGroup(self):
        graphGroups = [FRB.IniGraphGroup({})]

        edit = FRB.GraphCreate((1, "comp", "a"), self._makeGraph("a"))
        edit.edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 2)
        self.assertIn(("comp", "a"), graphGroups[1].graphs)

    def test_edit_emptyGraphGroups_appendsTheFirstGroup(self):
        graphGroups = []

        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a"))
        edit.edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)
        self.assertIn(("comp", "a"), graphGroups[0].graphs)

    def test_edit_iniIndexPastTheEnd_skippedSilently(self):
        graphGroups = [FRB.IniGraphGroup({})]

        edit = FRB.GraphCreate((5, "comp", "a"), self._makeGraph("a"))
        edit.edit(graphGroups, None)

        # no exception, no thousands of empty groups, and nothing added
        self.assertEqual(len(graphGroups), 1)
        self.assertNotIn(("comp", "a"), graphGroups[0].graphs)

    def test_edit_negativeIniIndex_skippedSilently(self):
        graphGroups = [FRB.IniGraphGroup({})]

        edit = FRB.GraphCreate((-1, "comp", "a"), self._makeGraph("a"))
        edit.edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)
        self.assertEqual(len(graphGroups[0].graphs), 0)

    def test_edit_graphIsNone_noOp(self):
        graphGroups = [FRB.IniGraphGroup({})]

        edit = FRB.GraphCreate((0, "comp", "a"), None)
        edit.edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)
        self.assertEqual(len(graphGroups[0].graphs), 0)

    def test_edit_addsACopyRatherThanTheGraphItself(self):
        # The group owns its graphs' lifetimes, so it can only be handed a graph it already owns --
        # the edit therefore adds a COPY. Storing the caller's pointer would dangle.
        graph = self._makeGraph("a")
        graphGroups = [FRB.IniGraphGroup({})]

        edit = FRB.GraphCreate((0, "comp", "a"), graph)
        edit.edit(graphGroups, None)

        added = graphGroups[0].graphs[("comp", "a")]
        self.assertIsNotNone(added)
        self.assertIsNot(added, graph)
        self.assertIs(edit.graph, graph)

    def test_edit_oneGraphReusedForSeveralIds(self):
        # What the copy buys: the caller keeps its graph and can add it under more than one id
        graph = self._makeGraph("a")
        graphGroups = [FRB.IniGraphGroup({}), FRB.IniGraphGroup({})]

        FRB.GraphCreate((0, "comp", "a"), graph).edit(graphGroups, None)
        FRB.GraphCreate((1, "comp", "a"), graph).edit(graphGroups, None)

        first = graphGroups[0].graphs[("comp", "a")]
        second = graphGroups[1].graphs[("comp", "a")]
        self.assertIsNotNone(first)
        self.assertIsNotNone(second)
        self.assertIsNot(first, second)

    def test_edit_replacesAnExistingGraphAtTheSameId(self):
        original = self._makeGraph("a", value = "original")
        group = FRB.IniGraphGroup({("comp", "a"): original})
        graphGroups = [group]

        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a", value = "replacement"))
        edit.edit(graphGroups, None)

        self.assertIsNot(group.graphs[("comp", "a")], original)
        self.assertEqual(len(group.graphs), 1)

    def test_edit_reassignedGraph_takesEffect(self):
        # The core member is re-derived from the Python object at the start of every edit, which is
        # what makes a reassignment take effect rather than being read once in the constructor
        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a"))
        replacement = self._makeGraph("b")
        edit.graph = replacement

        graphGroups = [FRB.IniGraphGroup({})]
        edit.edit(graphGroups, None)

        self.assertIs(edit.graph, replacement)
        self.assertIn(("comp", "a"), graphGroups[0].graphs)

    def test_edit_reassignedGraphId_takesEffect(self):
        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a"))
        edit.graphId = (0, "other", "b")

        graphGroups = [FRB.IniGraphGroup({})]
        edit.edit(graphGroups, None)

        self.assertIn(("other", "b"), graphGroups[0].graphs)
        self.assertNotIn(("comp", "a"), graphGroups[0].graphs)

    def test_edit_reassignedGraphToNone_becomesANoOp(self):
        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a"))
        edit.graph = None

        graphGroups = [FRB.IniGraphGroup({})]
        edit.edit(graphGroups, None)

        self.assertEqual(len(graphGroups[0].graphs), 0)

    def test_edit_graphWithAZ3Context_isAdded(self):
        # A graph built with a Z3 context has a strong reference to it propagated into the copy; a
        # graph edit's test needs the context whenever anything downstream asks a part for its query
        z3Ctx = FRB.Z3Context()
        graphGroups = [FRB.IniGraphGroup({})]

        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a", z3Ctx = z3Ctx))
        edit.edit(graphGroups, None)

        self.assertIn(("comp", "a"), graphGroups[0].graphs)

    def test_edit_modNameAndModTypeAreUnused(self):
        graphGroups = [FRB.IniGraphGroup({})]

        edit = FRB.GraphCreate((0, "comp", "a"), self._makeGraph("a"))
        edit.edit(graphGroups, None, "SomeMod")

        self.assertIn(("comp", "a"), graphGroups[0].graphs)
