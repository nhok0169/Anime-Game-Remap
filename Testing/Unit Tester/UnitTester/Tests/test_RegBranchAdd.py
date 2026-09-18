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


# RegBranchAdd is C++-backed. It is handed the predicate each part runs under and asks 'branchOf' what
# belongs there, so the tests decide branches the way a real caller does: by which branch's condition
# the part's predicate can hold together with -- exactly one, or none at all.
class RegBranchAddTest(BaseUnitTest):
    def _chain(self, branchParts):
        """`if $swapvar == 0 ... else if $swapvar == 1 ... endif`, with the given content per branch."""
        parts = []
        for i, content in enumerate(branchParts):
            kind = FRB.IfPredPartType.If if (i == 0) else FRB.IfPredPartType.Elif
            keyword = "if" if (i == 0) else "else if"
            parts.append(FRB.IfPredPart(f"{keyword} $swapvar == {i}", kind, _Z3CTX))
            parts.extend(content if isinstance(content, list) else [content])

        parts.append(FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX))
        return parts

    def _branchOf(self, branches: int, makeEntries):
        """A branchOf that names the ONE branch a query can hold with, and declines anything else."""
        def branchOf(query: FRB.Z3Predicate, iterData: FRB.SectionIterData):
            matches = []
            for i in range(branches):
                pred = FRB.IfPredPart.getLogicQuery(FRB.ParseContext(f"$swapvar == {i}"), _Z3CTX)
                if ((query & pred).isSatisfiable()):
                    matches.append(i)

            if (len(matches) != 1):
                return None
            return makeEntries(matches[0])

        return branchOf

    def _contentEntries(self, section: FRB.IfTemplate):
        return [p.entries() for p in section.parts if isinstance(p, FRB.IfContentPart)]

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        branchOf = lambda query, iterData: None
        self.assertIs(FRB.RegBranchAdd(branchOf).branchOf, branchOf)

    def test_init_default_noBranchOf(self):
        self.assertIsNone(FRB.RegBranchAdd().branchOf)

    def test_isSubclassOfBaseIniGraphEdit(self):
        self.assertTrue(issubclass(FRB.RegBranchAdd, FRB.BaseIniGraphEdit))

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSameGraphObject(self):
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")]}, 0)])}, ["root"], z3Ctx = _Z3CTX)
        self.assertIs(FRB.RegBranchAdd().edit(graph, None), graph)

    def test_edit_noBranchOf_noOp(self):
        section = FRB.IfTemplate(self._chain([FRB.IfContentPart({"ib": [(0, "Ib0")]}, 1)]))
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegBranchAdd().edit(graph, None)

        self.compareList(self._contentEntries(section), [[("ib", "Ib0")]])

    def test_edit_eachBranchGetsItsOwnEntries(self):
        section = FRB.IfTemplate(self._chain([FRB.IfContentPart({"ib": [(0, f"Ib{i}")]}, 1) for i in range(3)]))
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        branchOf = self._branchOf(3, lambda i: (f"body;{i}", [("drawindexed", f"{100 + i}, 0, 0")]))
        FRB.RegBranchAdd(branchOf).edit(graph, None)

        self.compareList(self._contentEntries(section), [
            [("ib", "Ib0"), ("drawindexed", "100, 0, 0")],
            [("ib", "Ib1"), ("drawindexed", "101, 0, 0")],
            [("ib", "Ib2"), ("drawindexed", "102, 0, 0")],
        ])

    def test_edit_replacements_setTheBranchesOwnValueRatherThanAddingASecond(self):
        section = FRB.IfTemplate(self._chain([FRB.IfContentPart({"vb1": [(0, f"Blend{i}")], "draw": [(1, "10,0")]}, 1) for i in range(2)]))
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        branchOf = self._branchOf(2, lambda i: (f"blend;{i}", [], [("draw", f"{500 + i},0")]))
        FRB.RegBranchAdd(branchOf).edit(graph, None)

        self.compareList(self._contentEntries(section), [
            [("vb1", "Blend0"), ("draw", "500,0")],
            [("vb1", "Blend1"), ("draw", "501,0")],
        ])

    def test_edit_unconditionalPreamble_declinedByBranchOf_leftAlone(self):
        section = FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "abc")]}, 0),
                                  *self._chain([FRB.IfContentPart({"ib": [(0, f"Ib{i}")]}, 1) for i in range(2)])])
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        branchOf = self._branchOf(2, lambda i: (f"body;{i}", [("drawindexed", f"{i}, 0, 0")]))
        FRB.RegBranchAdd(branchOf).edit(graph, None)

        self.compareList(self._contentEntries(section), [
            [("hash", "abc")],
            [("ib", "Ib0"), ("drawindexed", "0, 0, 0")],
            [("ib", "Ib1"), ("drawindexed", "1, 0, 0")],
        ])

    def test_edit_emptyKey_addsNothing(self):
        section = FRB.IfTemplate(self._chain([FRB.IfContentPart({"ib": [(0, "Ib0")]}, 1)]))
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegBranchAdd(lambda query, iterData: ("", [("drawindexed", "1, 0, 0")])).edit(graph, None)

        self.compareList(self._contentEntries(section), [[("ib", "Ib0")]])

    def test_edit_branchSpanningSeveralParts_entriesGoInTheLast(self):
        # branch 0 carries its ib in a nested `if` and its draw after it: the addition belongs after both
        nested = [FRB.IfPredPart("if $swapvar == 0", FRB.IfPredPartType.If, _Z3CTX),
                  FRB.IfContentPart({"ib": [(0, "Ib0")]}, 2),
                  FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
                  FRB.IfContentPart({"drawindexed": [(0, "5, 0, 0")]}, 1)]
        section = FRB.IfTemplate(self._chain([nested, FRB.IfContentPart({"ib": [(0, "Ib1")]}, 1)]))
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        branchOf = self._branchOf(2, lambda i: (f"body;{i}", [("drawindexed", f"9{i}, 5, 0")]))
        FRB.RegBranchAdd(branchOf).edit(graph, None)

        self.compareList(self._contentEntries(section), [
            [("ib", "Ib0")],
            [("drawindexed", "5, 0, 0"), ("drawindexed", "90, 5, 0")],
            [("ib", "Ib1"), ("drawindexed", "91, 5, 0")],
        ])

    def test_edit_branchOfSeesThePartItIsAskedAbout(self):
        section = FRB.IfTemplate(self._chain([FRB.IfContentPart({"ib": [(0, f"Ib{i}")]}, 1) for i in range(2)]))
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)
        seen = []

        def branchOf(query, iterData):
            self.assertIsInstance(query, FRB.Z3Predicate)
            seen.append(iterData.part.entries())
            return None

        FRB.RegBranchAdd(branchOf).edit(graph, None)

        self.compareList(seen, [[("ib", "Ib0")], [("ib", "Ib1")]])

    def test_edit_badReturn_raisesTypeError(self):
        section = FRB.IfTemplate(self._chain([FRB.IfContentPart({"ib": [(0, "Ib0")]}, 1)]))
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        with self.assertRaises(TypeError):
            FRB.RegBranchAdd(lambda query, iterData: ("a", [], [], "too many")).edit(graph, None)

    def test_edit_reassignedBranchOf_takesEffect(self):
        section = FRB.IfTemplate(self._chain([FRB.IfContentPart({"ib": [(0, "Ib0")]}, 1)]))
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)
        edit = FRB.RegBranchAdd(lambda query, iterData: None)

        edit.branchOf = lambda query, iterData: ("only", [("drawindexed", "7, 0, 0")])
        edit.edit(graph, None)

        self.compareList(self._contentEntries(section), [[("ib", "Ib0"), ("drawindexed", "7, 0, 0")]])
