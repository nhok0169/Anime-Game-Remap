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


# RegSurroundedAdd is now C++-backed (see AI Agent Help/IniGraphEditing/CLAUDE.md) -- unlike the
# pure-Python original (RegSurroundedAddOld, since deleted), its private helpers (_buildKeyFilters,
# _getSatisfiedRange, _keysExistSomewhere, _pickInsertInd, ...) aren't bound and so aren't
# separately testable here. Every one of those helpers is exercised indirectly, and exhaustively,
# through the public edit() tests below -- which is also all a caller (whether pure Python or C++)
# can actually see.
class RegSurroundedAddTest(BaseUnitTest):
    def _getContentPart(self, graph: FRB.IniSectionGraph, sectionName: str, partInd: int = 0) -> FRB.IfContentPart:
        return graph.getSection(sectionName).parts[partInd]

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None}, latest = True)

        self.compareList(edit.additions, [("addition", "yay")])
        self.compareDict(edit.beforeRegs, {"a": None})
        self.compareDict(edit.afterRegs, {"c": None})
        self.assertTrue(edit.latest)

    def test_init_defaults_emptyRegsAndLatestFalse(self):
        edit = FRB.RegSurroundedAdd(("addition", "yay"))

        self.compareDict(edit.beforeRegs, {})
        self.compareDict(edit.afterRegs, {})
        self.assertFalse(edit.latest)

    def test_isSubclassOfBaseIniGraphEdit(self):
        self.assertTrue(issubclass(FRB.RegSurroundedAdd, FRB.BaseIniGraphEdit))
        self.assertIsInstance(FRB.RegSurroundedAdd(("addition", "yay")), FRB.BaseIniGraphEdit)

    # ================================================
    # ===================== edit ======================
    # NOTE: 'beforeRegs' are the registers that must come *before* 'addition' (addition ends up after them),
    # and 'afterRegs' are the registers that must come *after* 'addition' (addition ends up before them).

    def test_edit_singlePartSurrounded_insertsBetweenBeforeAndAfterRegs(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("addition", "yay"), ("b", "2"), ("c", "3")])

    def test_edit_beforeRegsTwoKeys_insertsAfterTheLaterOfTheTwo(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "x": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None, "b": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2"), ("addition", "yay"), ("x", "3")])

    def test_edit_afterRegsTwoKeys_insertsBeforeTheEarlierOfTheTwo(self):
        # latest=True is used here specifically to pin the insertion right up against the boundary -- with the
        # default (earliest), 'addition' would just land at the very front (index 0, before "x" too), which
        # wouldn't actually demonstrate that the bound tracks "c" (the earlier key), not "d" (the later one)
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")], "c": [(1, "2")], "d": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"c": None, "d": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "1"), ("addition", "yay"), ("c", "2"), ("d", "3")])

    def test_edit_beforeRegsWouldLandAfterAnAfterRegsKey_contradicts_noInsertion(self):
        # 'beforeRegs' pushes the earliest valid index to right after "b" (index 2) -- but "c" (one of the two
        # 'afterRegs' keys) already appeared *before* "b", at index 1. Since afterRegs requires 'addition' to
        # precede *every* one of its keys individually (not just the group as a whole becoming satisfied), being
        # after "b" and before "c" can't both hold here -- "c" already happened first -- so nothing gets inserted
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart(
            {"a": [(0, "1")], "c": [(1, "c-val")], "b": [(2, "2")], "x": [(3, "mid")], "d": [(4, "d-val")]}, 0
        )])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None, "b": None}, afterRegs = {"c": None, "d": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(),
                         [("a", "1"), ("c", "c-val"), ("b", "2"), ("x", "mid"), ("d", "d-val")])

    def test_edit_beforeAndAfterRegsBothMultipleKeys_insertsBetweenBothBounds(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart(
            {"a": [(0, "1")], "b": [(1, "2")], "x": [(2, "mid")], "c": [(3, "3")], "d": [(4, "4")]}, 0
        )])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None, "b": None}, afterRegs = {"c": None, "d": None})
        edit.edit(graph, None)

        # after "b" (the later beforeRegs key); before "c" (the earlier afterRegs key) -- here that still leaves
        # the same window as before "d" would have, since "x" sits entirely inside both bounds either way
        self.compareList(self._getContentPart(graph, "root").entries(),
                         [("a", "1"), ("b", "2"), ("addition", "yay"), ("x", "mid"), ("c", "3"), ("d", "4")])

    def test_edit_beforeRegsMultipleKeysOneNeverPresent_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None, "b": None, "zzz": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2")])

    def test_edit_afterRegsMultipleKeysOneNeverPresentAnywhere_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "c": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None, "zzz": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("c", "2")])

    def test_edit_beforeRegsMultipleKeysMixedPredicates_onlyAcceptedOccurenceCounts(self):
        # "a" has no predicate (any occurence accepted); "b" only accepts "good" -- the window shouldn't open
        # until "b" shows an accepted value, even though "a" was satisfied much earlier and "b"'s first (rejected)
        # occurence came before "x". Note "b"'s two occurences and "x" end up at true positions 1, 2, 3 (not the
        # raw 1, 3, 4 passed into the dict) -- IfContentPart's src index only controls insertion order, not the
        # literal stored position (see Testing/CLAUDE.md's note on this)
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart(
            {"a": [(0, "1")], "b": [(1, "bad"), (3, "good")], "x": [(4, "mid")]}, 0
        )])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None, "b": lambda val: val == "good"})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(),
                         [("a", "1"), ("b", "bad"), ("b", "good"), ("addition", "yay"), ("x", "mid")])

    def test_edit_neitherBeforeNorAfterRegsGivenAcrossMultipleParts_stillInsertsExactlyOnce(self):
        # both empty -- the window is unconstrained/open from the very first reachable part; the predecessor-graph
        # dedupe (see IniSectionGraph.buildPartPredecessorGraph) still applies even with no registers at all, so
        # only "parent" (the earliest reachable part) gets the insertion, not "child" too
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"))
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "parent").entries(), [("addition", "yay"), ("a", "1"), ("run", "child")])
        self.compareList(self._getContentPart(graph, "child").entries(), [("b", "2")])

    def test_edit_beforeAndAfterRegsContradict_noInsertion(self):
        # 'beforeRegs' and 'afterRegs' pointing at registers in the "wrong" relative order can never both be
        # satisfied at the same index, so nothing gets added
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"c": None}, afterRegs = {"a": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2"), ("c", "3")])

    def test_edit_onlyBeforeRegsGiven_insertsRightAfterIt(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("addition", "yay"), ("b", "2")])

    def test_edit_onlyAfterRegsGiven_insertsAtTheEarliestValidIndex(self):
        # with no 'beforeRegs' lower bound, the earliest valid index (0) wins -- 'addition' lands at the very
        # front of the part, not tight against "b" (any index up to and including "b"'s own would be equally
        # valid "before b", but this class always picks the earliest one within the open window)
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"b": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("addition", "yay"), ("a", "1"), ("b", "2")])

    def test_edit_beforeRegPredicateRejectsOnlyValue_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": lambda val: val == "never"}, afterRegs = {"c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2"), ("c", "3")])

    def test_edit_afterRegKeyNeverPresent_noInsertion(self):
        # "a" satisfies 'beforeRegs', but "zzz" (the only 'afterRegs' key) never shows up at all -- that
        # condition can never be satisfied, so nothing gets added despite 'beforeRegs' being met
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"zzz": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2")])

    def test_edit_beforeRegKeyNeverPresent_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"zzz": None}, afterRegs = {"b": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2")])

    def test_edit_branchingIfElse_onlyQualifyingBranchesGetTheirOwnInsertion(self):
        # "vb1" is set independently in each branch (satisfying beforeRegs on both paths), while "ib" only
        # gets set once both branches have merged back together after the endIf -- so each branch closes its
        # own window independently, and the part after the endIf never re-opens one (its "vb1" tracking was
        # scoped to whichever branch set it, and gets restored away once that branch's subtree finishes)
        sections = {"root": FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"ib": [(0, "9")]}, 0),
        ])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"vb1": None}, afterRegs = {"ib": None})
        edit.edit(graph, None)

        parts = sections["root"].parts
        self.compareList(parts[0].entries(), [("a", "1")])
        self.compareList(parts[2].entries(), [("vb1", "branch1"), ("addition", "yay")])
        self.compareList(parts[4].entries(), [("vb1", "branch2"), ("addition", "yay")])
        self.compareList(parts[6].entries(), [("ib", "9")])

    def test_edit_branchMissingBeforeRegKey_onlyOtherBranchGetsInsertion(self):
        sections = {"root": FRB.IfTemplate([
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfContentPart({"other": [(0, "branch2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
        ])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"vb1": None})
        edit.edit(graph, None)

        parts = sections["root"].parts
        self.compareList(parts[1].entries(), [("vb1", "branch1"), ("addition", "yay")])
        self.compareList(parts[3].entries(), [("other", "branch2")])

    def test_edit_partFilterReturnsEmptyRange_suppressesInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        edit.edit(graph, None, partFilter = lambda iterData, modType, ini: FRB.Ranges.createEmpty())

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2"), ("c", "3")])

    def test_edit_partFilterExcludesOtherwiseValidWindow_suppressesInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        # the natural window here is indices [1, 3) (from right after "a" up to and including "c"'s own
        # position); a partFilter disjoint from that window should suppress the insertion entirely
        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        edit.edit(graph, None, partFilter = lambda iterData, modType, ini: FRB.Ranges([(5, None)]))

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2"), ("c", "3")])

    def test_edit_partFilterReceivesIterDataAndModType(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "c": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])
        modType = object()
        seen = []

        def partFilter(iterData, currentModType, ini):
            seen.append((iterData.sectionName, currentModType, ini))
            return FRB.Ranges.createFull()

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        edit.edit(graph, modType, partFilter = partFilter)

        self.assertTrue(len(seen) > 0)
        self.assertEqual(seen[0], ("root", modType, None))

    def test_edit_windowSpansMultipleSequentialPartsLatestFalse_insertsOnlyAtTheEarliestPart(self):
        # "a" (beforeRegs) is satisfied within "parent" itself, but by the time "child" is reached, "a" is only
        # present as a carried-over (ancestor) value -- meaning "parent" already had, and (since latest=False)
        # already took, the earliest opportunity for this still-open window. "child" must not insert again.
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")], "c": [(1, "3")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "parent").entries(), [("a", "1"), ("addition", "yay"), ("run", "child")])
        self.compareList(self._getContentPart(graph, "child").entries(), [("b", "2"), ("c", "3")])

    def test_edit_windowSpansMultipleSequentialPartsLatestTrue_insertsOnlyAtTheLatestPart(self):
        # "c" (afterRegs) only closes once "child" is reached -- since latest=True prefers later locations, and
        # "child" is nested (via the "run =" call) within "parent"'s own still-open window, only "child" should
        # get the insertion, deferred there instead of committed early at "parent"
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")], "c": [(1, "3")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "parent").entries(), [("a", "1"), ("run", "child")])
        self.compareList(self._getContentPart(graph, "child").entries(), [("b", "2"), ("addition", "yay"), ("c", "3")])

    def test_edit_windowSpansMultipleSequentialPartsNoBeforeRegsLatestFalse_insertsOnlyAtTheEarliestPart(self):
        # with no 'beforeRegs' at all, the window is trivially open starting from the very first reachable part
        # ("parent") -- with latest=False (the default), that's exactly where the earliest valid location is, so
        # "child" (nested within "parent"'s still-open window via the "run =" call) must not insert again
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")], "c": [(1, "3")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "parent").entries(), [("addition", "yay"), ("a", "1"), ("run", "child")])
        self.compareList(self._getContentPart(graph, "child").entries(), [("b", "2"), ("c", "3")])

    def test_edit_windowSpansMultipleSequentialPartsNoBeforeRegsLatestTrue_insertsOnlyAtTheLatestPart(self):
        # mirror of the above -- with latest=True, "child" (the latest valid part in this still-open window) is
        # preferred over "parent"
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")], "c": [(1, "3")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"c": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "parent").entries(), [("a", "1"), ("run", "child")])
        self.compareList(self._getContentPart(graph, "child").entries(), [("b", "2"), ("addition", "yay"), ("c", "3")])

    def test_edit_crossRootDisconnectedRoots_eachRootInsertsIndependently(self):
        # two entirely disconnected roots, each independently satisfying the same window -- neither should
        # suppress the other; IfContentPartColouring is confirmed (see baseUnitTest coverage above) to not leak
        # state across independent roots
        sections = {
            "rootA": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "c": [(1, "2")]}, 0)]),
            "rootB": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "c": [(1, "2")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["rootA", "rootB"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "rootA").entries(), [("a", "1"), ("addition", "yay"), ("c", "2")])
        self.compareList(self._getContentPart(graph, "rootB").entries(), [("a", "1"), ("addition", "yay"), ("c", "2")])

    def test_edit_selfReferencingSection_insertsExactlyOnceAndTerminates(self):
        # a section that calls itself via "run =" -- IniSectionGraph.iterByContentPart's own cycle pruning means
        # this part is only ever visited once, so there's no risk of an infinite loop here; the more interesting
        # check is that the self-referential predecessor edge (see
        # test_buildPartPredecessorGraph_selfReferencingRunCall_doesNotHang in test_IniSectionGraph.py) doesn't
        # somehow suppress the part's own, only insertion
        sections = {"loop": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "loop")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["loop"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "loop").entries(), [("a", "1"), ("addition", "yay"), ("run", "loop")])

    def test_edit_selfReferencingSectionBeforeAndAfterRegsSatisfiableOnlyThroughTheCycle_insertsAfterExploitingIt(self):
        # same self-referencing section as above, with 'beforeRegs' ("a") and 'afterRegs' ("c") in the "wrong"
        # relative order WITHIN the one part ("c" at index 0, "a" at index 1) -- taken as a single, non-looping
        # pass this looks like the same contradiction as test_edit_beforeAndAfterRegsContradict_noInsertion (must
        # be after "a" at >=2, but at-or-before "c" at <=0). But this part loops back into itself via "run =", so
        # going around the cycle once more, "c" reappears *after* "a" -- there really is a point on the cycle
        # with "a" behind it and "c" ahead of it, it's just past the "run =" call, not within this single textual
        # pass. The forward/backward fixpoint (which models "run =" as call-with-return and iterates to a fixpoint
        # over the whole call graph, not just a single linear scan) finds this window and inserts there
        sections = {"loop": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "1")], "a": [(1, "2")], "run": [(2, "loop")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["loop"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "loop").entries(), [("c", "1"), ("a", "2"), ("addition", "yay"), ("run", "loop")])

    def test_edit_mutualRunCallCycleLatestFalse_insertsOnlyAtTheFirstReachablePart(self):
        # A calls B, and B calls back into A -- "A" is reached first (it's the root), claims the window, and by
        # the time DFS would otherwise revisit "A" through "B"'s own "run =" call, "A" is already fully visited
        # (cycle-pruned), so there's no second decision to make for it either way
        sections = {
            "A": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "B")]}, 0)]),
            "B": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "2")], "run": [(1, "A")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["A"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "A").entries(), [("a", "1"), ("addition", "yay"), ("run", "B")])
        self.compareList(self._getContentPart(graph, "B").entries(), [("c", "2"), ("run", "A")])

    def test_edit_mutualRunCallCycleMultiKeyBeforeAndAfterRegsLatestFalse_insertsOnlyAtTheFirstReachablePart(self):
        # same mutual A<->B cycle as above, but exercising the fixpoint/dedup machinery together with MULTI-key
        # beforeRegs/afterRegs (each previously only tested separately -- multi-key against a non-cyclic graph,
        # or a single key against a cyclic one)
        #
        # "a" and "b" are both local to A, so A's own beforeRegs bound is after the later of the two (its own
        # "b"). "c" and "d" are only ever defined in B, reachable purely via A's own call -- since neither is
        # satisfiable locally within A at all, A's afterRegs side needs the call-graph credit (both "c" and "d"
        # are reachable via the call, so the pre-call zone is valid); B's own call back into A never returns
        # (unconditional 2-cycle), so the position after A's own call in A remains correctly excluded either way.
        # "A" is reached first and claims the window; "B" (also independently satisfiable, purely locally) never
        # gets a turn, since it's already claimed by the time it's visited
        sections = {
            "A": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "B")]}, 0)]),
            "B": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "3")], "d": [(1, "4")], "run": [(2, "A")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["A"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None, "b": None}, afterRegs = {"c": None, "d": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "A").entries(), [("a", "1"), ("b", "2"), ("addition", "yay"), ("run", "B")])
        self.compareList(self._getContentPart(graph, "B").entries(), [("c", "3"), ("d", "4"), ("run", "A")])

    def test_edit_mutualRunCallCycleMultiKeyBeforeAndAfterRegsLatestTrue_insertsOnlyAtTheLastReachablePart(self):
        # same graph as directly above, but latest=True -- this time "B" gets to decide first (reversed visiting
        # order), and its own window is satisfiable purely locally (before both "c" and "d", which are already
        # right there in "B" itself) without leaning on any call/return credit at all, so it claims the window;
        # "A" is deferred once "B" (its successor) has already claimed it
        sections = {
            "A": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "B")]}, 0)]),
            "B": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "3")], "d": [(1, "4")], "run": [(2, "A")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["A"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None, "b": None}, afterRegs = {"c": None, "d": None},
                                    latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "A").entries(), [("a", "1"), ("b", "2"), ("run", "B")])
        self.compareList(self._getContentPart(graph, "B").entries(), [("addition", "yay"), ("c", "3"), ("d", "4"), ("run", "A")])

    def test_edit_threeSectionRunCallCycleLatestTrue_insertsOnlyAtTheLastReachablePart(self):
        # X -> run=Y -> run=Z -> run=X (back to the start), with NO conditional anywhere -- ie. the recursion
        # never escapes, "run = X" in Z never actually returns, so nothing after it is reachable (see
        # _getBackwardValidRangeForPart's call/return split and _computeReachableNodes)
        #
        # "c" (Z's own afterRegs key) is satisfied locally by Z itself, right there before the call -- so even
        # though *some* "c" is also reachable by looping all the way back around via the call (X -> Y -> Z again),
        # _getValidRangeForPart prefers the nearer, already-present local "c" over that rolled-forward one, since
        # the local one alone is already enough on its own. So latest=True still lands before "c", not after it
        sections = {
            "X": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "Y")]}, 0)]),
            "Y": FRB.IfTemplate([FRB.IfContentPart({"m": [(0, "mid")], "run": [(1, "Z")]}, 0)]),
            "Z": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "2")], "run": [(1, "X")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["X"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "X").entries(), [("a", "1"), ("run", "Y")])
        self.compareList(self._getContentPart(graph, "Y").entries(), [("m", "mid"), ("run", "Z")])
        self.compareList(self._getContentPart(graph, "Z").entries(), [("addition", "yay"), ("c", "2"), ("run", "X")])

    def test_edit_windowSpansSequentialPartsWithinOneSectionAroundIfElseLatestFalse_insertsOnlyAtTheEarliestPart(self):
        # "a" (before any "if") opens the window unconditionally, before either branch is even reached -- so the
        # earliest valid location is there, and neither branch nor the part after "endIf" should insert again.
        # This dedupe is driven by IniSectionGraph.buildPartPredecessorGraph, not by iterByContentPart's own DFS
        # pre/post-order pairing (which doesn't nest content parts that are merely sequential siblings within one
        # section, eg. before an "if", each branch, and after the matching "endIf" -- unlike a "run =" call,
        # which it does nest) -- see the "run =" tests above for that half of the dedupe.
        sections = {"root": FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"c": [(0, "9")]}, 0),
        ])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"c": None})
        edit.edit(graph, None)

        parts = sections["root"].parts
        self.compareList(parts[0].entries(), [("addition", "yay"), ("a", "1")])
        self.compareList(parts[2].entries(), [("vb1", "branch1")])
        self.compareList(parts[4].entries(), [("vb1", "branch2")])
        self.compareList(parts[6].entries(), [("c", "9")])

    def test_edit_windowSpansSequentialPartsWithinOneSectionAroundIfElseLatestTrue_insertsOnlyAtTheLatestPart(self):
        # mirror of the above -- with latest=True, the part after "endIf" (the latest valid location, common to
        # both branches) is preferred over "a"
        sections = {"root": FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"c": [(0, "9")]}, 0),
        ])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"c": None}, latest = True)
        edit.edit(graph, None)

        parts = sections["root"].parts
        self.compareList(parts[0].entries(), [("a", "1")])
        self.compareList(parts[2].entries(), [("vb1", "branch1")])
        self.compareList(parts[4].entries(), [("vb1", "branch2")])
        self.compareList(parts[6].entries(), [("addition", "yay"), ("c", "9")])

    def test_edit_windowOpensSeparatelyInEachBranch_eachBranchInsertsIndependently(self):
        # unlike the tests above, here "vb1" (beforeRegs) is only satisfied *within* each branch (not before the
        # "if" at all) -- so each branch opens its own, independent window and must insert on its own; the part
        # after "endIf" is common to both branches and doesn't need its own insertion, since both already got one
        sections = {"root": FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"c": [(0, "9")]}, 0),
        ])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"vb1": None}, afterRegs = {"c": None})
        edit.edit(graph, None)

        parts = sections["root"].parts
        self.compareList(parts[0].entries(), [("a", "1")])
        self.compareList(parts[2].entries(), [("vb1", "branch1"), ("addition", "yay")])
        self.compareList(parts[4].entries(), [("vb1", "branch2"), ("addition", "yay")])
        self.compareList(parts[6].entries(), [("c", "9")])

    def test_edit_ifWithNoElse_endIfInheritsTheSkippedPathToo(self):
        # with no "else", the part after "endIf" must also be reachable *without* taking the "if" branch at all --
        # so it must inherit "a"'s claim directly, not just through whichever branch happens to run
        sections = {"root": FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"c": [(0, "9")]}, 0),
        ])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"c": None})
        edit.edit(graph, None)

        parts = sections["root"].parts
        self.compareList(parts[0].entries(), [("addition", "yay"), ("a", "1")])
        self.compareList(parts[2].entries(), [("vb1", "branch1")])
        self.compareList(parts[4].entries(), [("c", "9")])

    # ================================================
    # ===================== latest ======================

    def test_edit_latestDefault_insertsAtEarliestValidIndex(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None})
        self.assertFalse(edit.latest)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("addition", "yay"), ("b", "2"), ("c", "3")])

    def test_edit_latestTrue_insertsAtLatestValidIndex(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2"), ("addition", "yay"), ("c", "3")])

    def test_edit_latestTrueOnlyBeforeRegsGiven_insertsAtTheVeryEnd(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2"), ("addition", "yay")])

    def test_edit_latestTrueOnlyAfterRegsGiven_insertsRightBeforeIt(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"b": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("addition", "yay"), ("b", "2")])

    def test_edit_latestTruePartFilterNarrowsWindow_respectsFilteredBound(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        # the natural window is [1, 3); restricting it to [1, 2) via partFilter should push the latest choice
        # back to index 1 instead of index 2
        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"c": None}, latest = True)
        edit.edit(graph, None, partFilter = lambda iterData, modType, ini: FRB.Ranges([(0, 2)]))

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("addition", "yay"), ("b", "2"), ("c", "3")])

    # ================================================
    # ================= optBeforeRegs ==================
    # 'optBeforeRegs' is an "any of" group: the window only needs at least one of its registers seen
    # (and accepted), on top of every register in 'beforeRegs'.

    def test_init_optBeforeRegs_storedAndDefaultsToEmpty(self):
        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None})
        self.compareDict(edit.optBeforeRegs, {"a": None, "b": None})

        self.compareDict(FRB.RegSurroundedAdd(("addition", "yay")).optBeforeRegs, {})

    def test_edit_optBeforeRegs_onlyOneOfTheGroupPresent_insertsAfterIt(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")], "b": [(1, "2")], "y": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "0"), ("b", "2"), ("addition", "yay"), ("y", "3")])

    def test_edit_optBeforeRegs_bothPresent_insertsAfterTheEarlierOne(self):
        # "any of" opens the window at the *first* register of the group seen, unlike beforeRegs
        # (which waits for the later one -- see test_edit_beforeRegsTwoKeys_insertsAfterTheLaterOfTheTwo)
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "x": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("addition", "yay"), ("b", "2"), ("x", "3")])

    def test_edit_optBeforeRegs_nonePresent_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")], "y": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "1"), ("y", "2")])

    def test_edit_optBeforeRegs_combinedWithBeforeRegs_needsAllOfBeforeAndAnyOfOpt(self):
        # "a" (beforeRegs) is satisfied at index 0, but the optional group ("b" or "c") only at "b"
        # (index 2) -- the window opens after whichever bound is later
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart(
            {"a": [(0, "1")], "x": [(1, "mid")], "b": [(2, "2")], "y": [(3, "3")]}, 0
        )])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, optBeforeRegs = {"b": None, "c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(),
                         [("a", "1"), ("x", "mid"), ("b", "2"), ("addition", "yay"), ("y", "3")])

    def test_edit_optBeforeRegs_optSatisfiedButBeforeRegMissing_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")], "y": [(1, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, optBeforeRegs = {"b": None, "c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("b", "2"), ("y", "3")])

    def test_edit_optBeforeRegs_predicateRejectsTheOnlyPresentOne_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "bad")], "y": [(1, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": lambda val: val == "good"})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("b", "bad"), ("y", "3")])

    def test_edit_optBeforeRegs_predicateAcceptsLaterOccurence_insertsAfterThatOne(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "bad"), (1, "good")], "y": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": lambda val: val == "good"})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("b", "bad"), ("b", "good"), ("addition", "yay"), ("y", "3")])

    def test_edit_optBeforeRegs_withAfterRegs_insertsInsideTheWindow(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")], "x": [(1, "mid")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None}, afterRegs = {"c": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("b", "2"), ("addition", "yay"), ("x", "mid"), ("c", "3")])

    def test_edit_optBeforeRegs_latestTrue_insertsAtLatestValidIndex(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")], "x": [(1, "mid")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None}, afterRegs = {"c": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("b", "2"), ("x", "mid"), ("addition", "yay"), ("c", "3")])

    def test_edit_optBeforeRegs_satisfiedInParentViaRunCall_childDoesNotInsertAgain(self):
        # the group is satisfied in "parent" ("b"), which claims the window; "child" (reached via the
        # run = call, with "b" only carried in) must not insert a second time
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"y": [(0, "3")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "parent").entries(), [("b", "2"), ("addition", "yay"), ("run", "child")])
        self.compareList(self._getContentPart(graph, "child").entries(), [("y", "3")])

    def test_edit_optBeforeRegs_satisfiedOnlyInChild_parentInsertsOnceTheCallHasReturned(self):
        # same rule as beforeRegs (see _getForwardValidRangeForPart's call/return split): "a" is
        # guaranteed once "parent"'s run = call has *returned*, so the position right after that call
        # is valid, and "parent" (visited first) claims the window there -- "child" is then already
        # claimed. Pinned against beforeRegs to make sure the group follows the exact same rule
        def makeGraph():
            sections = {
                "parent": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")], "run": [(1, "child")]}, 0)]),
                "child": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "2")], "y": [(1, "3")]}, 0)]),
            }
            return FRB.IniSectionGraph(sections, ["parent"])

        graph = makeGraph()
        FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None}).edit(graph, None)
        self.compareList(self._getContentPart(graph, "parent").entries(), [("x", "1"), ("run", "child"), ("addition", "yay")])
        self.compareList(self._getContentPart(graph, "child").entries(), [("a", "2"), ("y", "3")])

        reference = makeGraph()
        FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}).edit(reference, None)
        self.compareList(self._getContentPart(reference, "parent").entries(), self._getContentPart(graph, "parent").entries())
        self.compareList(self._getContentPart(reference, "child").entries(), self._getContentPart(graph, "child").entries())

    def test_edit_optBeforeRegs_eachBranchSatisfiesADifferentMember_eachBranchInserts(self):
        # "a" in the if-branch and "b" in the else-branch both satisfy the group, so each branch opens
        # its own window and inserts; the part after the endIf is then already claimed by both
        sections = {"root": FRB.IfTemplate([
            FRB.IfContentPart({"x": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"a": [(0, "branch1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfContentPart({"b": [(0, "branch2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"y": [(0, "9")]}, 0),
        ])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optBeforeRegs = {"a": None, "b": None})
        edit.edit(graph, None)

        parts = sections["root"].parts
        self.compareList(parts[0].entries(), [("x", "1")])
        self.compareList(parts[2].entries(), [("a", "branch1"), ("addition", "yay")])
        self.compareList(parts[4].entries(), [("b", "branch2"), ("addition", "yay")])
        self.compareList(parts[6].entries(), [("y", "9")])

    # ================================================
    # ================= optAfterRegs ===================
    # the "any of" counterpart of 'afterRegs': at least one of its registers must still come after
    # 'addition', on top of every register in 'afterRegs'

    def test_init_optAfterRegs_storedAndDefaultsToEmpty(self):
        edit = FRB.RegSurroundedAdd(("addition", "yay"), optAfterRegs = {"c": None, "d": None})
        self.compareDict(edit.optAfterRegs, {"c": None, "d": None})

        self.compareDict(FRB.RegSurroundedAdd(("addition", "yay")).optAfterRegs, {})

    def test_edit_optAfterRegs_onlyOneOfTheGroupPresent_insertsBeforeIt(self):
        # latest=True pins the insertion against the bound (see test_edit_afterRegsTwoKeys_...)
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")], "d": [(1, "2")], "y": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optAfterRegs = {"c": None, "d": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "0"), ("addition", "yay"), ("d", "2"), ("y", "3")])

    def test_edit_optAfterRegs_bothPresent_insertsBeforeTheLaterOne(self):
        # "any of" only needs *some* member still ahead, so the window stays open up to the *later*
        # member -- unlike afterRegs, which closes at the earlier one
        # (see test_edit_afterRegsTwoKeys_insertsBeforeTheEarlierOfTheTwo)
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")], "c": [(1, "2")], "d": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optAfterRegs = {"c": None, "d": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "1"), ("c", "2"), ("addition", "yay"), ("d", "3")])

    def test_edit_optAfterRegs_nonePresentAnywhere_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "y": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, optAfterRegs = {"c": None, "d": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("y", "2")])

    def test_edit_optAfterRegs_combinedWithAfterRegs_needsAllOfAfterAndAnyOfOpt(self):
        # "c" (afterRegs) closes the window at index 1; the optional group ("d" or "e") is still ahead
        # there via "d" -- the window is whichever bound is *earlier*
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart(
            {"x": [(0, "0")], "c": [(1, "1")], "y": [(2, "mid")], "d": [(3, "2")]}, 0
        )])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"c": None}, optAfterRegs = {"d": None, "e": None}, latest = True)
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(),
                         [("x", "0"), ("addition", "yay"), ("c", "1"), ("y", "mid"), ("d", "2")])

    def test_edit_optAfterRegs_optSatisfiedButAfterRegMissing_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")], "d": [(1, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), afterRegs = {"c": None}, optAfterRegs = {"d": None, "e": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "0"), ("d", "2")])

    def test_edit_optAfterRegs_predicateRejectsTheOnlyPresentOne_noInsertion(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")], "d": [(1, "bad")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optAfterRegs = {"c": None, "d": lambda val: val == "good"})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("x", "0"), ("d", "bad")])

    def test_edit_optAfterRegs_withBeforeRegs_insertsInsideTheWindow(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "x": [(1, "mid")], "d": [(2, "2")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, optAfterRegs = {"c": None, "d": None})
        edit.edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("addition", "yay"), ("x", "mid"), ("d", "2")])

    def test_edit_optAfterRegs_satisfiedOnlyInChildViaRunCall_matchesAfterRegs(self):
        # "d" is only reachable through "parent"'s run = call -- the group must follow the same
        # call/return rule afterRegs does, so both are pinned to the same result
        def makeGraph():
            sections = {
                "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
                "child": FRB.IfTemplate([FRB.IfContentPart({"d": [(0, "2")], "y": [(1, "3")]}, 0)]),
            }
            return FRB.IniSectionGraph(sections, ["parent"])

        graph = makeGraph()
        FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, optAfterRegs = {"c": None, "d": None}).edit(graph, None)
        self.compareList(self._getContentPart(graph, "parent").entries(), [("a", "1"), ("addition", "yay"), ("run", "child")])
        self.compareList(self._getContentPart(graph, "child").entries(), [("d", "2"), ("y", "3")])

        reference = makeGraph()
        FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"a": None}, afterRegs = {"d": None}).edit(reference, None)
        self.compareList(self._getContentPart(reference, "parent").entries(), self._getContentPart(graph, "parent").entries())
        self.compareList(self._getContentPart(reference, "child").entries(), self._getContentPart(graph, "child").entries())

    def test_edit_optAfterRegs_eachBranchProvidesADifferentMember_eachBranchInserts(self):
        # "c" in the if-branch and "d" in the else-branch: the part before the "if" is followed by
        # *some* member on every path, but by a *different* one on each, so no single member is
        # guaranteed after it and the group (an "or" of per-register guarantees -- see the
        # attribute's own note) does not credit it. Each branch part then opens its own window
        # right before its member, and the part after the endIf is already claimed by both
        sections = {"root": FRB.IfTemplate([
            FRB.IfContentPart({"x": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"c": [(0, "branch1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
            FRB.IfContentPart({"d": [(0, "branch2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"y": [(0, "9")]}, 0),
        ])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        edit = FRB.RegSurroundedAdd(("addition", "yay"), optAfterRegs = {"c": None, "d": None})
        edit.edit(graph, None)

        parts = sections["root"].parts
        self.compareList(parts[0].entries(), [("x", "1")])
        self.compareList(parts[2].entries(), [("addition", "yay"), ("c", "branch1")])
        self.compareList(parts[4].entries(), [("addition", "yay"), ("d", "branch2")])
        self.compareList(parts[6].entries(), [("y", "9")])

    def test_edit_cycle_afterRegNotOnTheCycle_neverEndingCycleCountsAsClosingTheWindow(self):
        # A calls B, B calls back into A, and no drawindexed sits anywhere on the cycle (the only one
        # is in an unrelated root, so the "exists nowhere" early exit does not fire). The after-side
        # analysis is a MUST ("on every path ahead") fixpoint, and a `run =` cycle that never ends has
        # no path that escapes it, so "drawindexed lies ahead on every path" is vacuously true for the
        # nodes on the cycle: the window counts as closing inside the cycle and the ordinary
        # once-per-window insertion applies -- B gets it as late as possible (right before it calls
        # back into A), and A, whose window B claimed, gets nothing. Pinned so the pre-existing
        # semantics of the analysis on a never-ending cycle (an infinitely recursive .ini, which no
        # real mod has) stay visible; see IniGraphEditing/CLAUDE.md
        sections = {
            "A": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "h")], "run": [(1, "B")]}, 0)]),
            "B": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "0")], "run": [(1, "A")]}, 0)]),
            "unrelated": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "d")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["A", "unrelated"])

        FRB.RegSurroundedAdd(("addition", "yay"), beforeRegs = {"hash": None}, afterRegs = {"drawindexed": None}, latest = True).edit(graph, None)

        self.compareList(self._getContentPart(graph, "A").entries(), [("hash", "h"), ("run", "B")])
        self.compareList(self._getContentPart(graph, "B").entries(), [("x", "0"), ("addition", "yay"), ("run", "A")])
        self.compareList(self._getContentPart(graph, "unrelated").entries(), [("drawindexed", "d")])

    # ================================================
    # ================== additions ====================
    # 'additions' is a list of KVPs: every entry lands together at the chosen position, as
    # consecutive lines in list order. A single (key, value) tuple is still accepted and reads
    # back as a one-entry list.

    def test_init_additions_singleTupleReadsBackAsAList(self):
        edit = FRB.RegSurroundedAdd(("addition", "yay"))

        self.compareList(edit.additions, [("addition", "yay")])

    def test_init_additions_list(self):
        edit = FRB.RegSurroundedAdd([("a", "1"), ("b", "2")])

        self.compareList(edit.additions, [("a", "1"), ("b", "2")])

    def test_init_additions_reassign(self):
        edit = FRB.RegSurroundedAdd(("addition", "yay"))
        edit.additions = [("a", "1"), ("b", "2")]
        self.compareList(edit.additions, [("a", "1"), ("b", "2")])

        edit.additions = ("c", "3")
        self.compareList(edit.additions, [("c", "3")])

    def test_init_additions_badShape_raises(self):
        with self.assertRaises(TypeError):
            FRB.RegSurroundedAdd("addition")
        with self.assertRaises(TypeError):
            FRB.RegSurroundedAdd([("a", "1", "extra")])

    def test_edit_additions_severalRows_landTogetherInOrder_earliest(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "c": [(1, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        FRB.RegSurroundedAdd([("x", "1"), ("y", "2"), ("z", "3")], beforeRegs = {"a": None}, afterRegs = {"c": None}).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("x", "1"), ("y", "2"), ("z", "3"), ("c", "3")])

    def test_edit_additions_severalRows_landTogetherInOrder_latest(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        FRB.RegSurroundedAdd([("x", "1"), ("y", "2")], beforeRegs = {"a": None}, afterRegs = {"c": None}, latest = True).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("b", "2"), ("x", "1"), ("y", "2"), ("c", "3")])

    def test_edit_additions_severalRows_stillOncePerWindowAcrossACall(self):
        sections = {
            "override": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "h")], "run": [(1, "cmd")]}, 0)]),
            "cmd": FRB.IfTemplate([FRB.IfContentPart({"drawindexed": [(0, "d")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["override"])

        FRB.RegSurroundedAdd([("x", "1"), ("y", "2")], beforeRegs = {"hash": None}, afterRegs = {"drawindexed": None}, latest = True).edit(graph, None)

        self.compareList(self._getContentPart(graph, "override").entries(), [("hash", "h"), ("run", "cmd")])
        self.compareList(self._getContentPart(graph, "cmd").entries(), [("x", "1"), ("y", "2"), ("drawindexed", "d")])

    def test_edit_additions_empty_noOp(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "c": [(1, "3")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])

        FRB.RegSurroundedAdd([], beforeRegs = {"a": None}, afterRegs = {"c": None}).edit(graph, None)

        self.compareList(self._getContentPart(graph, "root").entries(), [("a", "1"), ("c", "3")])
