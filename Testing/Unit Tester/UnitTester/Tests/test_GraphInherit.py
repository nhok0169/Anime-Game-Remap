import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GraphInheritTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        partFilter = lambda iterData, modType, ini: FRB.Ranges.createFull()
        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = False, partFilter = partFilter)

        self.assertEqual(edit.src, (0, "comp", "src"))
        self.assertEqual(edit.dst, (0, "comp", "dst"))
        self.assertEqual(edit.reg, "run")
        self.assertFalse(edit.latest)
        self.assertIs(edit.partFilter, partFilter)

    def test_init_defaults(self):
        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run")
        self.assertTrue(edit.latest)
        self.assertIsNone(edit.partFilter)

    # ================================================
    # ===================== edit ======================

    def _makeGraphGroups(self, graphsByModObj):
        return [FRB.IniGraphGroup(dict(graphsByModObj))]

    def _getRootPart(self, graph: FRB.IniSectionGraph, sectionName: str, partInd: int = 0) -> FRB.IfContentPart:
        return graph.getSection(sectionName).parts[partInd]

    def test_edit_noPartFilterLatestTrue_appendsRunKVPToBackOfSrcRoot(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1"), ("run", "dstRoot")])
        # 'dst' itself is left untouched
        self.compareList(self._getRootPart(dstGraph, "dstRoot").entries(), [("b", "2")])

    def test_edit_noPartFilterLatestFalse_prependsRunKVPToFrontOfSrcRoot(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = False)
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("run", "dstRoot"), ("a", "1")])

    def test_edit_dstMultipleRoots_insertsOneKVPPerRootInOrder(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({
            "dstRootA": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)], name = "dstRootA"),
            "dstRootB": FRB.IfTemplate([FRB.IfContentPart({"y": [(0, "2")]}, 0)], name = "dstRootB"),
        }, ["dstRootA", "dstRootB"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        expected = [("a", "1")] + [("run", root) for root in dstGraph.roots]
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), expected)

    def test_edit_srcMultipleRootSections_eachRootGetsTheKVPs(self):
        srcGraph = FRB.IniSectionGraph({
            "srcRootA": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRootA"),
            "srcRootB": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "3")]}, 0)], name = "srcRootB"),
        }, ["srcRootA", "srcRootB"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRootA").entries(), [("a", "1"), ("run", "dstRoot")])
        self.compareList(self._getRootPart(srcGraph, "srcRootB").entries(), [("c", "3"), ("run", "dstRoot")])

    def test_edit_partFilterGiven_insertsWithinFilteredRangeInsteadOfTrueEnd(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart(
            {"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")], "d": [(3, "4")]}, 0
        )], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        # only indices [1, 3) ("b", "c") are a valid insertion window -- the true end of the part is index 4
        partFilter = lambda iterData, modType, ini: FRB.Ranges([(1, 3)])
        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True, partFilter = partFilter)
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1"), ("b", "2"), ("c", "3"), ("run", "dstRoot"), ("d", "4")])

    def test_edit_partFilterReturnsEmptyRange_noInsertionAnywhere(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True, partFilter = lambda iterData, modType, ini: FRB.Ranges.createEmpty())
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1")])

    def test_edit_dstGraphHasNoRoots_srcUntouched(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({}, [])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        section = srcGraph.getSection("srcRoot")
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1")])
        self.assertEqual(len(section.parts), 1)

    def test_edit_srcNotFound_returnsGraphGroupsUnchanged(self):
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "missing"), (0, "comp", "dst"), "run")
        result = edit.edit(graphGroups, None)

        self.assertIs(result, graphGroups)
        self.compareList(self._getRootPart(dstGraph, "dstRoot").entries(), [("b", "2")])

    def test_edit_dstNotFound_returnsGraphGroupsUnchanged(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "missing"), "run")
        result = edit.edit(graphGroups, None)

        self.assertIs(result, graphGroups)
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1")])

    def test_edit_rootEndsInConditional_backInsertLandsAfterEndifNotInsideBranch(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf),
        ], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        section = srcGraph.getSection("srcRoot")
        self.assertEqual(len(section.parts), 5)
        self.compareList(section.parts[-1].entries(), [("run", "dstRoot")])
        # the branch's own content must be untouched by the new bottom part
        self.compareList(section.parts[2].entries(), [("vb1", "branch1")])

    def test_edit_rootStartsWithConditional_frontInsertLandsBeforeIfNotInsideBranch(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf),
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
        ], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = False)
        edit.edit(graphGroups, None)

        section = srcGraph.getSection("srcRoot")
        self.assertEqual(len(section.parts), 5)
        self.compareList(section.parts[0].entries(), [("run", "dstRoot")])
        # the branch's own content must be untouched by the new front part
        self.compareList(section.parts[2].entries(), [("vb1", "branch1")])
        # the trailing content after the conditional must also be untouched
        self.compareList(section.parts[-1].entries(), [("a", "1")])

    def test_edit_returnsTheSameGraphGroupsList(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        result = edit.edit(graphGroups, None)

        self.assertIs(result, graphGroups)
