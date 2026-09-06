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


# RegDelimitedAdd is C++-backed (see AI Agent Help/IniGraphEditing/CLAUDE.md). Its invariant: cut
# every execution path at each accepted delimiter; every resulting delimiter-free segment (start ->
# first delimiter, delimiter -> delimiter, last delimiter -> end of path) holds the addition exactly
# once, as late as possible. Two placement rules realise it: right before every delimiter, and at the
# end of every path-terminal part. The tests below check the invariant on the placed output rather
# than any internal state, since that is all a caller can see.
class RegDelimitedAddTest(BaseUnitTest):
    _DRAW = {"drawindexed": None}

    def _getContentPart(self, graph: FRB.IniSectionGraph, sectionName: str, partInd: int = 0) -> FRB.IfContentPart:
        return graph.getSection(sectionName).parts[partInd]

    def _contentEntries(self, section: FRB.IfTemplate):
        return [p.entries() for p in section.parts if isinstance(p, FRB.IfContentPart)]

    def _ifElse(self, ifPart: FRB.IfContentPart, elsePart: FRB.IfContentPart, pred: str = "if $x == 2"):
        return [
            FRB.IfPredPart(pred, FRB.IfPredPartType.If, _Z3CTX),
            ifPart,
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            elsePart,
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
        ]

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        edit = FRB.RegDelimitedAdd(("insertion", "NNFix"), delimiterRegs = {"drawindexed": None, "drawindexedinstanced": None})

        self.assertEqual(edit.addition, ("insertion", "NNFix"))
        self.compareDict(edit.delimiterRegs, {"drawindexed": None, "drawindexedinstanced": None})

    def test_init_defaults_noDelimiters(self):
        edit = FRB.RegDelimitedAdd(("insertion", "NNFix"))

        self.compareDict(edit.delimiterRegs, {})

    def test_init_reassignDelimiterRegs_takesEffect(self):
        edit = FRB.RegDelimitedAdd(("insertion", "NNFix"))
        edit.delimiterRegs = self._DRAW
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)])}, ["root"])

        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("insertion", "NNFix"), ("drawindexed", "a"), ("insertion", "NNFix")])

    def test_isSubclassOfBaseIniGraphEdit(self):
        self.assertTrue(issubclass(FRB.RegDelimitedAdd, FRB.BaseIniGraphEdit))
        self.assertIsInstance(FRB.RegDelimitedAdd(("insertion", "NNFix")), FRB.BaseIniGraphEdit)

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSameGraphObject(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")]}, 0)])}, ["root"])

        self.assertIs(FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None), graph)

    def test_edit_theMotivatingExample(self):
        #   a = 1
        #   if $x == 2
        #      drawindexed = a
        #   else
        #     foo = 2
        #   endif
        #   b = 3
        #
        # the draw gets the addition right before it; the shared end gets it once, covering both the
        # post-draw stretch of the if-path and the whole draw-free else-path. Nothing after "a = 1"
        # (that would double the if-path's pre-draw stretch) and nothing after "foo = 2" (that would
        # double the else-path)
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            *self._ifElse(FRB.IfContentPart({"drawindexed": [(0, "a")]}, 1), FRB.IfContentPart({"foo": [(0, "2")]}, 1)),
            FRB.IfContentPart({"b": [(0, "3")]}, 0),
        ])
        graph = FRB.IniSectionGraph({"root": section}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._contentEntries(section), [
            [("a", "1")],
            [("insertion", "NNFix"), ("drawindexed", "a")],
            [("foo", "2")],
            [("b", "3"), ("insertion", "NNFix")],
        ])

    def test_edit_noDelimiterOnThePath_onceAtTheEnd(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")], "y": [(1, "1")]}, 0)])}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "0"), ("y", "1"), ("insertion", "NNFix")])

    def test_edit_severalDelimitersInOnePart_beforeEachAndOnceAfterTheLast(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a"), (2, "b")], "x": [(1, "0")]}, 0)])}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [
            ("insertion", "NNFix"), ("drawindexed", "a"),
            ("x", "0"),
            ("insertion", "NNFix"), ("drawindexed", "b"),
            ("insertion", "NNFix"),
        ])

    def test_edit_adjacentDelimiters_oneBetweenThem(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a"), (1, "b")]}, 0)])}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [
            ("insertion", "NNFix"), ("drawindexed", "a"), ("insertion", "NNFix"), ("drawindexed", "b"), ("insertion", "NNFix"),
        ])

    def test_edit_multipleDelimiterRegisters(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"drawindexedinstanced": [(0, "i")], "x": [(1, "0")], "drawindexed": [(2, "a")]}, 0)])}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), {"drawindexed": None, "drawindexedinstanced": None}).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [
            ("insertion", "NNFix"), ("drawindexedinstanced", "i"),
            ("x", "0"),
            ("insertion", "NNFix"), ("drawindexed", "a"),
            ("insertion", "NNFix"),
        ])

    def test_edit_predicate_onlyAcceptedOccurencesDelimit(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "skip"), (1, "auto")]}, 0)])}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), {"drawindexed": lambda val: val == "auto"}).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [
            ("drawindexed", "skip"), ("insertion", "NNFix"), ("drawindexed", "auto"), ("insertion", "NNFix"),
        ])

    def test_edit_noDelimiterRegs_onceAtTheEndOfEveryPath(self):
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            *self._ifElse(FRB.IfContentPart({"drawindexed": [(0, "a")]}, 1), FRB.IfContentPart({"foo": [(0, "2")]}, 1)),
        ])
        graph = FRB.IniSectionGraph({"root": section}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix")).edit(graph, None)

        self.compareList(self._contentEntries(section), [
            [("a", "1")],
            [("drawindexed", "a"), ("insertion", "NNFix")],
            [("foo", "2"), ("insertion", "NNFix")],
        ])

    def test_edit_unreachableSection_untouched(self):
        # IniSectionGraph only keeps the sections reachable from its roots, so the orphan never even
        # enters the graph -- it is checked through the object the test still holds
        orphan = FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)])
        sections = {
            "root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")]}, 0)]),
            "orphan": orphan,
        }
        graph = FRB.IniSectionGraph(sections, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "0"), ("insertion", "NNFix")])
        self.compareList(orphan.parts[0].entries(), [("drawindexed", "a")])
        with self.assertRaises(IndexError):
            graph.getSection("orphan")

    def test_edit_editFromIni_forwardsToEdit(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)])}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).editFromIni(graph, None, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("insertion", "NNFix"), ("drawindexed", "a"), ("insertion", "NNFix")])

    # ================================================
    # ==================== trees ======================

    def test_edit_branchEndingRightAfterItsDraw_trailingInsertionInsideTheBranch(self):
        # nothing after the endif, so each branch's last part is a path end: the draw branch gets one
        # before the draw and one after it (the post-draw stretch ends there), the other gets its one
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            *self._ifElse(FRB.IfContentPart({"drawindexed": [(0, "a")]}, 1), FRB.IfContentPart({"foo": [(0, "2")]}, 1)),
        ])
        graph = FRB.IniSectionGraph({"root": section}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._contentEntries(section), [
            [("a", "1")],
            [("insertion", "NNFix"), ("drawindexed", "a"), ("insertion", "NNFix")],
            [("foo", "2"), ("insertion", "NNFix")],
        ])

    def test_edit_ifWithoutElse_fallThroughPathCoveredByTheSharedEnd(self):
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 2", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"drawindexed": [(0, "a")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"c": [(0, "9")]}, 0),
        ])
        graph = FRB.IniSectionGraph({"root": section}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._contentEntries(section), [
            [("a", "1")],
            [("insertion", "NNFix"), ("drawindexed", "a")],
            [("c", "9"), ("insertion", "NNFix")],
        ])

    def test_edit_nestedBranches(self):
        #   if $x == 1
        #     drawindexed = a
        #   else
        #      if $y == 2
        #          foo = 2
        #      else
        #          foo = 1
        #          drawindexed = b
        #      endif
        #      bar = 1
        #   endif
        section = FRB.IfTemplate([
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"drawindexed": [(0, "a")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfPredPart("if $y == 2", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"foo": [(0, "2")]}, 2),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfContentPart({"foo": [(0, "1")], "drawindexed": [(1, "b")]}, 2),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"bar": [(0, "1")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
        ])
        graph = FRB.IniSectionGraph({"root": section}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        # every path: [NNFix, draw a, NNFix] / [foo 2, bar 1, NNFix] / [foo 1, NNFix, draw b, bar 1, NNFix]
        self.compareList(self._contentEntries(section), [
            [("insertion", "NNFix"), ("drawindexed", "a"), ("insertion", "NNFix")],
            [("foo", "2")],
            [("foo", "1"), ("insertion", "NNFix"), ("drawindexed", "b")],
            [("bar", "1"), ("insertion", "NNFix")],
        ])

    # ================================================
    # ================== run calls ====================

    def test_edit_runCall_drawInTheCallee_trailingInsertionInTheCaller(self):
        # the callee's end is not a path end -- the caller continues after the call returns
        sections = {
            "override": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "h")], "run": [(1, "cmd")]}, 0)]),
            "cmd": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["override"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "override").entries(), [("hash", "h"), ("run", "cmd"), ("insertion", "NNFix")])
        self.compareList(self._getContentPart(graph, "cmd").entries(), [("insertion", "NNFix"), ("drawindexed", "a")])

    def test_edit_runCall_contentAfterTheCall_trailingInsertionAtTheVeryEnd(self):
        sections = {
            "override": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "cmd")], "b": [(1, "3")]}, 0)]),
            "cmd": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["override"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "override").entries(), [("run", "cmd"), ("b", "3"), ("insertion", "NNFix")])
        self.compareList(self._getContentPart(graph, "cmd").entries(), [("insertion", "NNFix"), ("drawindexed", "a")])

    def test_edit_sharedCallee_editedOnce_everyCallerGetsItsOwnEnd(self):
        sections = {
            "A": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "cmd")]}, 0)]),
            "B": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "cmd")]}, 0)]),
            "cmd": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["A", "B"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "A").entries(), [("run", "cmd"), ("insertion", "NNFix")])
        self.compareList(self._getContentPart(graph, "B").entries(), [("run", "cmd"), ("insertion", "NNFix")])
        self.compareList(self._getContentPart(graph, "cmd").entries(), [("insertion", "NNFix"), ("drawindexed", "a")])

    def test_edit_calleeWithBranches_noTrailingInsertionInTheCallee(self):
        cmd = FRB.IfTemplate(self._ifElse(FRB.IfContentPart({"drawindexed": [(0, "a")]}, 1), FRB.IfContentPart({"foo": [(0, "2")]}, 1), "if $c == 1"))
        sections = {
            "override": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "cmd")]}, 0)]),
            "cmd": cmd,
        }
        graph = FRB.IniSectionGraph(sections, ["override"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "override").entries(), [("run", "cmd"), ("insertion", "NNFix")])
        self.compareList(self._contentEntries(cmd), [
            [("insertion", "NNFix"), ("drawindexed", "a")],
            [("foo", "2")],
        ])

    def test_edit_drawInTheCallerBeforeTheCall_delimiterBeforeItAndEndAfterTheCall(self):
        sections = {
            "override": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")], "run": [(1, "cmd")]}, 0)]),
            "cmd": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["override"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "override").entries(), [("insertion", "NNFix"), ("drawindexed", "a"), ("run", "cmd"), ("insertion", "NNFix")])
        self.compareList(self._getContentPart(graph, "cmd").entries(), [("x", "0")])

    def test_edit_sectionBothRootAndCallee_noTrailingInsertionThere_theDocumentedLimit(self):
        # "cmd" is run by A and is also a root of its own: its end is a path end on the direct path
        # but not on A's, and one position cannot be both. The edit never doubles the addition, so
        # "cmd" gets no trailing insertion at all (only A's own end does)
        sections = {
            "A": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "cmd")]}, 0)]),
            "cmd": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["A", "cmd"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "A").entries(), [("run", "cmd"), ("insertion", "NNFix")])
        self.compareList(self._getContentPart(graph, "cmd").entries(), [("insertion", "NNFix"), ("drawindexed", "a")])

    def test_edit_mutualRunCallCycle_delimitersOnly_noPathEnds(self):
        # every part on a never-ending cycle has a successor, so nothing is a path end
        sections = {
            "A": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "h")], "run": [(1, "B")]}, 0)]),
            "B": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")], "run": [(1, "A")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["A"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None)

        self.compareList(self._getContentPart(graph, "A").entries(), [("hash", "h"), ("run", "B")])
        self.compareList(self._getContentPart(graph, "B").entries(), [("insertion", "NNFix"), ("drawindexed", "a"), ("run", "A")])

    # ================================================
    # ================== partFilter ===================

    def test_edit_partFilter_gatesEachPositionOnItsOwn(self):
        # the part wants [NNFix, draw a, x, NNFix, draw b, NNFix] (positions 0, 2 and 3); restricting
        # the part to [0, 3) keeps the two pre-draw ones and drops the trailing one
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a"), (2, "b")], "x": [(1, "0")]}, 0)])}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None, partFilter = lambda iterData, modType, ini: FRB.Ranges([(0, 3)]))

        self.compareList(self._getContentPart(graph, "root").entries(), [
            ("insertion", "NNFix"), ("drawindexed", "a"), ("x", "0"), ("insertion", "NNFix"), ("drawindexed", "b"),
        ])

    def test_edit_partFilter_empty_skipsThePart(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)])}, ["root"])

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None, partFilter = lambda iterData, modType, ini: FRB.Ranges.createEmpty())

        self.compareList(self._getContentPart(graph, "root").entries(), [("drawindexed", "a")])

    def test_edit_partFilter_perSection(self):
        sections = {
            "keep": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "a")]}, 0)]),
            "skip": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "b")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["keep", "skip"])

        def partFilter(iterData, modType, ini):
            return FRB.Ranges.createFull() if iterData.sectionName == "keep" else FRB.Ranges.createEmpty()

        FRB.RegDelimitedAdd(("insertion", "NNFix"), self._DRAW).edit(graph, None, partFilter = partFilter)

        self.compareList(self._getContentPart(graph, "keep").entries(), [("insertion", "NNFix"), ("drawindexed", "a"), ("insertion", "NNFix")])
        self.compareList(self._getContentPart(graph, "skip").entries(), [("drawindexed", "b")])
