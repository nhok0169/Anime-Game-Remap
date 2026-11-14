import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IterGraphGroupEditTest(BaseUnitTest):
    def _makeGraph(self, name: str) -> FRB.IniSectionGraph:
        return FRB.IniSectionGraph({name: FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = name)}, [name])

    def _makeGroups(self, count: int):
        return [FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a"), ("comp", "b"): self._makeGraph("b")}) for _ in range(count)]

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        def makeEdits(iniIndex, graphGroups):
            return None

        edit = FRB.IterGraphGroupEdit(makeEdits)
        self.assertIs(edit.makeEdits, makeEdits)

    def test_init_noFactory_editsNothing(self):
        graphGroups = self._makeGroups(2)
        result = FRB.IterGraphGroupEdit().edit(graphGroups, None)

        self.assertIs(result, graphGroups)
        for group in graphGroups:
            self.assertIn(("comp", "a"), group.graphs)

    # ================================================
    # ===================== edit ======================

    def test_edit_runsTheFactorysEditsOnEveryGroup(self):
        graphGroups = self._makeGroups(3)
        edit = FRB.IterGraphGroupEdit(lambda iniIndex, groups: [FRB.GraphRemove([(iniIndex, "comp", "a")])])
        edit.edit(graphGroups, None)

        for group in graphGroups:
            self.assertNotIn(("comp", "a"), group.graphs)
            self.assertIn(("comp", "b"), group.graphs)

    def test_edit_callsTheFactoryOncePerGroupInOrder(self):
        graphGroups = self._makeGroups(3)
        calls = []

        def makeEdits(iniIndex, groups):
            calls.append(iniIndex)
            self.assertIs(groups, graphGroups)
            return []

        FRB.IterGraphGroupEdit(makeEdits).edit(graphGroups, None)
        self.assertEqual(calls, [0, 1, 2])

    def test_edit_noEditsForAGroup_leavesItAlone(self):
        graphGroups = self._makeGroups(2)
        edit = FRB.IterGraphGroupEdit(lambda iniIndex, groups: None if iniIndex == 0 else [FRB.GraphRemove([(iniIndex, "comp", "a")])])
        edit.edit(graphGroups, None)

        self.assertIn(("comp", "a"), graphGroups[0].graphs)
        self.assertNotIn(("comp", "a"), graphGroups[1].graphs)

    def test_edit_runsAGroupsEditsInOrder(self):
        graphGroups = self._makeGroups(1)
        seen = []

        class Record(FRB.BaseIniGraphGroupEdit):
            def __init__(self, label):
                super().__init__()
                self.label = label

            def edit(self, graphGroups, modType, modName = ""):
                seen.append((self.label, ("comp", "a") in graphGroups[0].graphs))
                return graphGroups

        edit = FRB.IterGraphGroupEdit(lambda iniIndex, groups: [Record("before"), FRB.GraphRemove([(0, "comp", "a")]), Record("after")])
        edit.edit(graphGroups, None)

        self.assertEqual(seen, [("before", True), ("after", False)])

    def test_edit_groupAddedDuringTheRun_notVisited(self):
        graphGroups = self._makeGroups(1)
        calls = []

        class AddGroup(FRB.BaseIniGraphGroupEdit):
            def edit(self, graphGroups, modType, modName = ""):
                graphGroups.append(FRB.IniGraphGroup({}))
                return graphGroups

        def makeEdits(iniIndex, groups):
            calls.append(iniIndex)
            return [AddGroup()]

        FRB.IterGraphGroupEdit(makeEdits).edit(graphGroups, None)
        self.assertEqual(calls, [0])
        self.assertEqual(len(graphGroups), 2)

    # ================================================
    # ================= editFromIni ===================

    def test_editFromIni_handsTheIniToEachEdit(self):
        graphGroups = self._makeGroups(2)
        seen = []
        ini = object()

        class Record(FRB.BaseIniGraphGroupEdit):
            def editFromIni(self, graphGroups, ini, modType, modName = ""):
                seen.append((ini, modName))
                return graphGroups

        FRB.IterGraphGroupEdit(lambda iniIndex, groups: [Record()]).editFromIni(graphGroups, ini, None, modName = "Target")
        self.assertEqual(seen, [(ini, "Target"), (ini, "Target")])
