import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GraphRenameTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        fn = lambda name: name
        edit = FRB.GraphRename(fn)
        self.assertIs(edit.renameFunc, fn)

    def test_isSubclassOfBaseIniGraphEdit(self):
        self.assertTrue(issubclass(FRB.GraphRename, FRB.BaseIniGraphEdit))
        self.assertIsInstance(FRB.GraphRename(lambda name: name), FRB.BaseIniGraphEdit)

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSameGraphInstance(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "root")}, ["root"])
        edit = FRB.GraphRename(lambda name: f"{name}_new")

        result = edit.edit(graph, None)
        self.assertIs(result, graph)

    def test_edit_renamesSingleSection_updatesRootsAndSections(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "root")}, ["root"])
        edit = FRB.GraphRename(lambda name: f"{name}_new")
        edit.edit(graph, None)

        self.compareList(graph.roots, ["root_new"])
        self.assertIn("root_new", graph.sections)
        self.assertNotIn("root", graph.sections)
        self.assertEqual(graph.getSection("root_new").name, "root_new")

    def test_edit_parentChild_updatesRunReferenceToNewChildName(self):
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)], name = "parent"),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "child"),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])
        edit = FRB.GraphRename(lambda name: f"{name}_x")
        edit.edit(graph, None)

        self.compareList(graph.roots, ["parent_x"])
        parentSection = graph.getSection("parent_x")
        self.compareList(parentSection.parts[0].entries(), [("a", "1"), ("run", "child_x")])
        self.assertIn("child_x", graph.sections)

    def test_edit_multipleRootSections_allRenamed(self):
        sections = {
            "rootA": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "rootA"),
            "rootB": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "rootB"),
        }
        graph = FRB.IniSectionGraph(sections, ["rootA", "rootB"])
        edit = FRB.GraphRename(lambda name: f"{name}_x")
        edit.edit(graph, None)

        self.compareList(sorted(graph.roots), sorted(["rootA_x", "rootB_x"]))
        self.assertIn("rootA_x", graph.sections)
        self.assertIn("rootB_x", graph.sections)

    def test_edit_renameFuncReturnsSameName_graphUnchanged(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "root")}, ["root"])
        edit = FRB.GraphRename(lambda name: name)
        edit.edit(graph, None)

        self.compareList(graph.roots, ["root"])
        self.assertIn("root", graph.sections)
        self.assertEqual(graph.getSection("root").name, "root")

    def test_edit_renameFuncReassignedAfterConstruction_isHonoured(self):
        # the C++ member is re-derived from the stored Python object at the start of every edit
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "root")}, ["root"])
        edit = FRB.GraphRename(lambda name: name)
        edit.renameFunc = lambda name: f"{name}_late"

        edit.edit(graph, None)

        self.compareList(graph.roots, ["root_late"])

    # ================================================
    # ================== editFromIni ==================

    def test_editFromIni_forwardsToEditAndIgnoresIni(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "root")}, ["root"])
        edit = FRB.GraphRename(lambda name: f"{name}_new")

        result = edit.editFromIni(graph, "SOME INI", None, modName = "rika")

        self.assertIs(result, graph)
        self.compareList(graph.roots, ["root_new"])
