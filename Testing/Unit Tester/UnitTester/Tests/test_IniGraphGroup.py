import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class _FakeGraph:
    """
    Stand-in for a real :class:`IniSectionGraph` -- only needs a ``toStr(autoindent)`` method for
    :meth:`IniGraphGroup.toStr`'s own tests. Real :class:`IniSectionGraph` doesn't expose a
    ``toStr`` binding to Python at all (a pre-existing gap from an earlier, separate migration --
    see :meth:`IniGraphGroup.toStr`'s own doc comment), so a real graph can't be used here.
    """

    def __init__(self, text: str):
        self.text = text

    def toStr(self, autoindent: bool = True) -> str:
        return self.text


class IniGraphGroupTest(BaseUnitTest):
    """
    Tests for :class:`IniGraphGroup` -- the C++-backed replacement for the pure-Python original,
    now deleted outright (was briefly renamed to ``IniGraphGroupOld`` mid-migration)
    """

    def test_defaultConstruction_emptyDict(self):
        g = FRB.IniGraphGroup()
        self.compareDict(g.graphs, {})

    def test_noSharedMutableDefaultDict(self):
        g1 = FRB.IniGraphGroup()
        g1.graphs[("comp", "obj")] = "sentinel"
        g2 = FRB.IniGraphGroup()
        self.compareDict(g2.graphs, {})

    def test_construction_aliasesPassedDict(self):
        # Real GIMIParser.py usage relies on the constructed IniGraphGroup and the caller's own
        # dict being the exact same object afterward (IniGraphGroup(graphs = self.commandGraphs),
        # later self.commandGraphs = graphGroups[0].graphs) -- matches the pure-Python original's
        # own bare 'self.graphs = graphs' reference assignment.
        d = {}
        g = FRB.IniGraphGroup(graphs = d)
        g.addGraph(("comp", "obj"), "graphObj")

        self.assertIs(g.graphs, d)
        self.compareDict(d, {("comp", "obj"): "graphObj"})

    def test_addGraph_thenDictGet(self):
        g = FRB.IniGraphGroup()
        g.addGraph(("comp", "obj"), "graphObj")

        self.assertEqual(g.graphs.get(("comp", "obj")), "graphObj")
        self.assertIsNone(g.graphs.get(("other", "obj")))

    def test_inOperator(self):
        g = FRB.IniGraphGroup()
        g.addGraph(("comp", "obj"), "graphObj")

        self.assertIn(("comp", "obj"), g.graphs)
        self.assertNotIn(("other", "obj"), g.graphs)

    def test_removeGraph_found_returnsGraphAndRemoves(self):
        g = FRB.IniGraphGroup()
        g.addGraph(("comp", "obj"), "graphObj")

        removed = g.removeGraph(("comp", "obj"))

        self.assertEqual(removed, "graphObj")
        self.assertNotIn(("comp", "obj"), g.graphs)

    def test_removeGraph_notFound_returnsNone(self):
        g = FRB.IniGraphGroup()
        self.assertIsNone(g.removeGraph(("comp", "obj")))

    def test_toStr_joinsNonEmptyResultsWithBlankLine(self):
        g = FRB.IniGraphGroup()
        g.addGraph(("a", "1"), _FakeGraph("first"))
        g.addGraph(("b", "2"), _FakeGraph(""))
        g.addGraph(("c", "3"), _FakeGraph("third"))

        self.assertEqual(g.toStr(), "first\n\nthird")

    def test_toStr_emptyGraphs_emptyString(self):
        g = FRB.IniGraphGroup()
        self.assertEqual(g.toStr(), "")

    def test_toStr_autoindentPassedThrough(self):
        calls = []

        class _RecordingGraph:
            def toStr(self, autoindent: bool = True) -> str:
                calls.append(autoindent)
                return "x"

        g = FRB.IniGraphGroup()
        g.addGraph(("a", "1"), _RecordingGraph())
        g.toStr(autoindent = False)

        self.assertEqual(calls, [False])
