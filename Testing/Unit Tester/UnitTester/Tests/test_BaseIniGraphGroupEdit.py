import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseIniGraphGroupEditTest(BaseUnitTest):
    def _makeGraph(self, name: str) -> FRB.IniSectionGraph:
        return FRB.IniSectionGraph({name: FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = name)}, [name])

    # ================================================
    # =================== getGraph ====================

    def test_getGraph_found_returnsTheSameGraphObject(self):
        graph = self._makeGraph("a")
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): graph})]

        self.assertIs(FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (0, "comp", "a")), graph)

    def test_getGraph_notFound_raisesKeyError(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})]

        with self.assertRaises(KeyError):
            FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (0, "comp", "missing"))

    def test_getGraph_notFoundNoError_returnsTheDefault(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})]

        self.assertIsNone(FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (0, "comp", "missing"), False))
        self.assertEqual(FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (0, "comp", "missing"), False, "fallback"),
                         "fallback")

    def test_getGraph_iniIndexOutOfBounds_treatedAsNotFound(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})]

        self.assertIsNone(FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (5, "comp", "a"), False))
        with self.assertRaises(KeyError):
            FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (5, "comp", "a"))

    def test_getGraph_negativeIniIndex_doesNotWrapAround(self):
        # A negative index is "no such graph", not a Python-style index from the end -- the
        # pure-Python original compared it with >= len(graphGroups) and never intended wrapping
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})]

        self.assertIsNone(FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (-1, "comp", "a"), False))

    def test_getGraph_emptyGroups_treatedAsNotFound(self):
        self.assertIsNone(FRB.BaseIniGraphGroupEdit.getGraph([], (0, "comp", "a"), False))

    def test_getGraph_secondIniFile_looksInTheRightGroup(self):
        first = self._makeGraph("a")
        second = self._makeGraph("b")
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): first}), FRB.IniGraphGroup({("comp", "b"): second})]

        self.assertIs(FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (1, "comp", "b")), second)
        self.assertIsNone(FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (0, "comp", "b"), False))

    # ================================================
    # =================== addGraph ====================

    def test_addGraph_added_andRetrievableAsTheSameObject(self):
        group = FRB.IniGraphGroup({})
        graphGroups = [group]
        graph = self._makeGraph("a")

        self.assertTrue(FRB.BaseIniGraphGroupEdit.addGraph(graphGroups, (0, "comp", "a"), graph))
        self.assertIs(group.graphs[("comp", "a")], graph)
        self.assertIs(FRB.BaseIniGraphGroupEdit.getGraph(graphGroups, (0, "comp", "a")), graph)

    def test_addGraph_existingKey_overwrites(self):
        original = self._makeGraph("a")
        group = FRB.IniGraphGroup({("comp", "a"): original})
        replacement = self._makeGraph("b")

        self.assertTrue(FRB.BaseIniGraphGroupEdit.addGraph([group], (0, "comp", "a"), replacement))
        self.assertIs(group.graphs[("comp", "a")], replacement)

    def test_addGraph_iniIndexOutOfBounds_notAdded(self):
        group = FRB.IniGraphGroup({})

        self.assertFalse(FRB.BaseIniGraphGroupEdit.addGraph([group], (5, "comp", "a"), self._makeGraph("a")))
        self.assertEqual(len(group.graphs), 0)

    # ================================================
    # ===================== edit ======================

    def test_edit_isANoOpReturningTheSameList(self):
        group = FRB.IniGraphGroup({("comp", "a"): self._makeGraph("a")})
        graphGroups = [group]

        result = FRB.BaseIniGraphGroupEdit().edit(graphGroups, None)

        self.assertIs(result, graphGroups)
        self.assertIn(("comp", "a"), group.graphs)

    def test_editFromIni_forwardsToEditAndIgnoresTheIni(self):
        graphGroups = [FRB.IniGraphGroup({})]

        self.assertIs(FRB.BaseIniGraphGroupEdit().editFromIni(graphGroups, None, None), graphGroups)

    def test_editFromIni_pythonSubclassOverridingOnlyEdit_isStillReached(self):
        # editFromIni has to resolve 'edit' through real Python attribute lookup -- binding it
        # straight to the C++ base would silently skip a pure-Python override
        class MyEdit(FRB.BaseIniGraphGroupEdit):
            def edit(self, graphGroups, modType, modName = ""):
                return "OVERRIDDEN"

        self.assertEqual(MyEdit().editFromIni([], None, None), "OVERRIDDEN")

    def test_clear_isANoOp(self):
        self.assertIsNone(FRB.BaseIniGraphGroupEdit().clear())
