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


class BaseIniGraphEditTest(BaseUnitTest):
    def makeGraph(self, name: str = "root") -> FRB.IniSectionGraph:
        return FRB.IniSectionGraph({name: FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = name)}, [name])

    # ================================================
    # ================= inheritance ===================

    def test_isSubclassOfTheGraphPartEditBases(self):
        self.assertTrue(issubclass(FRB.BaseIniGraphEdit, FRB.CppBaseIniGraphPartEdit))
        self.assertTrue(issubclass(FRB.BaseIniGraphEdit, FRB.CppBaseIniPartEdit))

    def test_construct_noArgs_isInstanceOfItsBases(self):
        edit = FRB.BaseIniGraphEdit()
        self.assertIsInstance(edit, FRB.BaseIniGraphEdit)
        self.assertIsInstance(edit, FRB.CppBaseIniGraphPartEdit)

    def test_clear_inheritedFromBaseIniPartEdit_isANoOp(self):
        edit = FRB.BaseIniGraphEdit()
        self.assertIsNone(edit.clear())

    # ================================================
    # ===================== edit ======================

    def test_edit_isANoOpReturningTheSameGraph(self):
        graph = self.makeGraph()
        edit = FRB.BaseIniGraphEdit()

        result = edit.edit(graph, None)

        self.assertIs(result, graph)
        self.compareList(sorted(graph.sections.keys()), ["root"])
        self.compareList(graph.getSection("root").parts[0].entries(), [("a", "1")])

    def test_edit_acceptsModNameAndPartFilterKeywords(self):
        graph = self.makeGraph()
        edit = FRB.BaseIniGraphEdit()

        result = edit.edit(graph, None, modName = "rika", partFilter = lambda iterData, modType, ini: FRB.Ranges.createFull())

        self.assertIs(result, graph)

    # ================================================
    # ================== editFromIni ==================

    def test_editFromIni_forwardsToEditAndIgnoresIni(self):
        graph = self.makeGraph()
        edit = FRB.BaseIniGraphEdit()

        result = edit.editFromIni(graph, "SOME INI", None, modName = "rika")

        self.assertIs(result, graph)

    def test_editFromIni_reachesAPurePythonEditOverride(self):
        # editFromIni has to reach 'edit' through real Python attribute lookup -- a C++-internal
        # virtual call would silently run the no-op base implementation, since there is no
        # trampoline for this class.
        seen = []

        class SpyEdit(FRB.BaseIniGraphEdit):
            def edit(self, graph, modType, modName = "", partFilter = None, trackKeys = False, keysToTrack = None):
                seen.append((modType, modName))
                return graph

        graph = self.makeGraph()
        result = SpyEdit().editFromIni(graph, "SOME INI", "MODTYPE", modName = "rika")

        self.assertIs(result, graph)
        self.compareList(seen, [("MODTYPE", "rika")])

    # ================================================
    # ============= pure-Python subclassing ===========

    def test_subclass_withoutOwnInit_constructsAndOverridesEdit(self):
        class Doubler(FRB.BaseIniGraphEdit):
            def edit(self, graph, modType, modName = "", partFilter = None, trackKeys = False, keysToTrack = None):
                graph.getSection("root").parts[0].addKVP("b", "2")
                return graph

        graph = self.makeGraph()
        Doubler().edit(graph, None)

        self.compareList(graph.getSection("root").parts[0].entries(), [("a", "1"), ("b", "2")])

    def test_subclass_withOwnInitCallingSuper_keepsPythonOnlyState(self):
        class Stateful(FRB.BaseIniGraphEdit):
            def __init__(self, tag):
                super().__init__()
                self.tag = tag

        edit = Stateful("hello")
        self.assertEqual(edit.tag, "hello")
        self.assertIsInstance(edit, FRB.BaseIniGraphEdit)
