import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB



_Z3CTX = FRB.Z3Context()  # shared across every IfPredPart built in this test file
class RegSurroundedAddTest(BaseUnitTest):
    # ================================================
    # ================ _buildKeyFilters =================

    def test_buildKeyFilters_emptyRegs_returnsEmptyDict(self):
        result = FRB.RegSurroundedAdd._buildKeyFilters({})
        self.compareDict(result, {})

    def test_buildKeyFilters_predicateNone_excludedFromResult(self):
        result = FRB.RegSurroundedAdd._buildKeyFilters({"a": None, "b": None})
        self.compareDict(result, {})

    def test_buildKeyFilters_predicateGiven_wrapsValueOnlyPredicate(self):
        result = FRB.RegSurroundedAdd._buildKeyFilters({"a": lambda val: val == "yes", "b": None})

        self.compareList(list(result.keys()), ["a"])
        self.assertTrue(result["a"](None, "yes"))
        self.assertFalse(result["a"](None, "no"))
        # the index argument is ignored -- only the value matters to the underlying predicate
        self.assertTrue(result["a"](5, "yes"))

    def test_buildKeyFilters_threeKeysAllWithPredicates_wrapsEveryOne(self):
        result = FRB.RegSurroundedAdd._buildKeyFilters({
            "a": lambda val: val == "1", "b": lambda val: val == "2", "c": lambda val: val == "3",
        })

        self.compareSet(set(result.keys()), {"a", "b", "c"})
        self.assertTrue(result["a"](None, "1"))
        self.assertTrue(result["b"](None, "2"))
        self.assertTrue(result["c"](None, "3"))
        self.assertFalse(result["c"](None, "1"))

    # ================================================
    # ================ _getSatisfiedRange ================

    def test_getSatisfiedRange_emptyRegs_returnsFullRange(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "1")]})
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {}, {}, True)
        self.assertTrue(result.isFull())

    def test_getSatisfiedRange_keyFromCurrentPart_satisfiedFromItsIndexOnward(self):
        colouring = FRB.IfContentPartColouring({"a": [(3, "1")]})
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {"a": None}, {}, True)

        self.assertFalse(result.has(2))
        self.assertTrue(result.has(3))
        self.assertTrue(result.has(1000))

    def test_getSatisfiedRange_keyCarriedFromPreviousPart_satisfiedThroughout(self):
        colouring = FRB.IfContentPartColouring({"a": "1"})
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {"a": None}, {}, True)
        self.assertTrue(result.isFull())

    def test_getSatisfiedRange_keyNeverSeen_returnsEmptyRange(self):
        colouring = FRB.IfContentPartColouring()
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {"a": None}, {}, True)
        self.assertTrue(result.isEmpty())

    def test_getSatisfiedRange_multipleKeys_requiresAllToBeSatisfied(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "1")], "b": [(5, "2")]})
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {"a": None, "b": None}, {}, True)

        self.assertFalse(result.has(2))
        self.assertTrue(result.has(5))

    def test_getSatisfiedRange_threeKeys_boundedByTheLastOneToAppear(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "1")], "b": [(5, "2")], "c": [(2, "3")]})
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {"a": None, "b": None, "c": None}, {}, True)

        self.assertFalse(result.has(4))
        self.assertTrue(result.has(5))

    def test_getSatisfiedRange_multipleKeysOneNeverSeen_returnsEmptyRange(self):
        # requiring "a", "b" and "zzz" all seen -- "zzz" never appearing anywhere makes the whole group
        # unsatisfiable, regardless of how many of the other keys are present
        colouring = FRB.IfContentPartColouring({"a": [(0, "1")], "b": [(1, "2")]})
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {"a": None, "b": None, "zzz": None}, {}, True)

        self.assertTrue(result.isEmpty())

    def test_getSatisfiedRange_predicateRejectsValue_excludedFromRange(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "no"), (4, "yes")]})
        filters = FRB.RegSurroundedAdd._buildKeyFilters({"a": lambda val: val == "yes"})
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {"a": lambda val: val == "yes"}, filters, True)

        self.assertFalse(result.has(0))
        self.assertFalse(result.has(3))
        self.assertTrue(result.has(4))

    def test_getSatisfiedRange_includeKeyDefsFalse_excludesDefinitionIndex(self):
        colouring = FRB.IfContentPartColouring({"a": [(3, "1")]})
        result = FRB.RegSurroundedAdd._getSatisfiedRange(colouring, {"a": None}, {}, False)

        self.assertFalse(result.has(3))
        self.assertTrue(result.has(4))

    # ================================================
    # ================ _getValidRangeForPart ===============
    # NOTE: 'beforeRegs' are the registers that must come *before* 'addition' (addition ends up after them),
    # and 'afterRegs' are the registers that must come *after* 'addition' (addition ends up before them).
    #
    # 'beforeEntryFacts'/'beforeReturnFacts'/'afterExitFacts'/'afterReturnFacts' are the (per-register) fixpoint
    # facts _computeKeyFacts would normally compute across the whole graph -- passed in directly here to test
    # _getValidRangeForPart's own WITHIN-PART logic in isolation, without needing a full graph. A register
    # missing from any of these dicts is treated as False :raw-html:`<br />` :raw-html:`<br />`
    #
    # The "Return" facts (as opposed to the "Entry"/"Exit" ones) only ever matter for a part that makes its own
    # ``run =`` call -- they answer "is this guaranteed once that call has *returned*", as distinct from
    # "guaranteed somewhere reachable via the call" (which the plain Entry/Exit facts already cover) -- see the
    # dedicated tests near the bottom of this section for parts that actually make a call
    #
    # IfContentPart's src/buildFromOrder constructor index values only control insertion *ordering*, not the
    # literal stored position (see Testing/CLAUDE.md's note on this) -- so where a test's point is a specific
    # *gap* between two registers, filler KVPs are used to actually reserve that gap in the real stored positions.

    def test_getValidRangeForPart_beforeAndAfterRegsSpecified_intersectsToAnOpenWindow(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "x": [(1, "f")], "c": [(2, "3")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None}, afterRegs = {"c": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        # index 2 (c's own position) is included -- inserting there lands 'addition' immediately before "c"
        self.compareList(result.ranges, [(1, 3)])

    def test_getValidRangeForPart_beforeRegsTwoKeys_boundedByTheLaterOfTheTwo(self):
        # "a" and "b" are both required -- the window only opens once *both* have been seen, ie. after the
        # later of the two ("b")
        part = FRB.IfContentPart({"a": [(0, "1")], "x1": [(1, "f")], "x2": [(2, "f")], "b": [(3, "2")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None, "b": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.compareList(result.ranges, [(4, None)])

    def test_getValidRangeForPart_afterRegsTwoKeys_boundedByTheEarlierOfTheTwoNotTheLater(self):
        # Unlike beforeRegs (where "after the last one to appear" already implies "after every one of them",
        # since the max is >= each individual index), afterRegs needs 'addition' to precede *each* register
        # individually -- so the bound tracks the *earlier* of "c" and "d", not the later. Bounding by the later
        # one instead ("only before the whole group being satisfied together") would let 'addition' land after
        # "c" as long as "d" hadn't shown up yet, which isn't "afterRegs" for "c" at all
        part = FRB.IfContentPart({"c": [(0, "1")], "x1": [(1, "f")], "x2": [(2, "f")], "d": [(3, "2")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), afterRegs = {"c": None, "d": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        # index 0 ("c"'s own position) is still included -- inserting there lands 'addition' immediately before it
        self.compareList(result.ranges, [(None, 1)])

    def test_getValidRangeForPart_beforeRegsThreeKeys_boundedByTheLastOfTheThree(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "x": [(1, "f")], "c": [(2, "3")], "x2": [(3, "f")], "x3": [(4, "f")], "b": [(5, "2")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None, "b": None, "c": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.compareList(result.ranges, [(6, None)])

    def test_getValidRangeForPart_beforeRegsMultipleKeysOneNeverSeen_returnsEmptyRange(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None, "b": None, "zzz": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.assertTrue(result.isEmpty())

    def test_getValidRangeForPart_beforeAndAfterRegsBothMultipleKeys_intersectsBothBounds(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "x1": [(2, "f")], "x2": [(3, "f")],
                                   "c": [(4, "3")], "x3": [(5, "f")], "d": [(6, "4")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None, "b": None}, afterRegs = {"c": None, "d": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        # lower bound after "b" (the later beforeRegs key); upper bound before "c" (the earlier afterRegs
        # key) -- not "d"
        self.compareList(result.ranges, [(2, 5)])

    def test_getValidRangeForPart_onlyBeforeRegsGiven_unboundedAbove(self):
        part = FRB.IfContentPart({"a": [(0, "1")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.compareList(result.ranges, [(1, None)])

    def test_getValidRangeForPart_onlyAfterRegsGiven_unboundedBelow(self):
        part = FRB.IfContentPart({"b": [(0, "2")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), afterRegs = {"b": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        # index 0 (b's own position) is included -- inserting there lands 'addition' immediately before "b"
        self.compareList(result.ranges, [(None, 1)])

    def test_getValidRangeForPart_neitherSpecified_returnsFullRange(self):
        part = FRB.IfContentPart({})
        edit = FRB.RegSurroundedAdd(("ADD", "X"))

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.assertTrue(result.isFull())

    def test_getValidRangeForPart_beforeRegsPredicateNeverAccepted_returnsEmptyRange(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "x": [(1, "f")], "c": [(2, "3")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": lambda val: val == "never"}, afterRegs = {"c": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.assertTrue(result.isEmpty())

    def test_getValidRangeForPart_afterRegsExitFactTrue_treatedAsStillOpen(self):
        # unlike the old, colouring-only implementation, _getValidRangeForPart can now distinguish "not reached
        # yet on this part, but guaranteed to be seen somewhere after it" (exitFact True, from _computeKeyFacts'
        # graph-wide fixpoint) from "genuinely never happens anywhere" (see the sibling test right below) --
        # this part makes no call, so afterReturnFacts is irrelevant (never consulted) and left empty
        part = FRB.IfContentPart({"a": [(0, "1")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None}, afterRegs = {"zzz": None})

        result = edit._getValidRangeForPart(part, {}, {}, {"zzz": True}, {})
        self.compareList(result.ranges, [(1, None)])

    def test_getValidRangeForPart_afterRegsExitFactFalse_returnsEmptyRange(self):
        part = FRB.IfContentPart({"a": [(0, "1")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None}, afterRegs = {"zzz": None})

        result = edit._getValidRangeForPart(part, {}, {}, {"zzz": False}, {})
        self.assertTrue(result.isEmpty())

    def test_getValidRangeForPart_beforeRegsKeyNeverSeen_returnsEmptyRange(self):
        part = FRB.IfContentPart({"b": [(0, "2")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"zzz": None}, afterRegs = {"b": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.assertTrue(result.isEmpty())

    def test_getValidRangeForPart_beforeRegsEntryFactTrue_treatedAsUnconditionallyOpen(self):
        # entryFact True (from _computeKeyFacts) means the register was already satisfied entering this part --
        # here the part doesn't even touch it itself, so the whole part is open
        part = FRB.IfContentPart({})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None})

        result = edit._getValidRangeForPart(part, {"a": True}, {}, {}, {})
        self.assertTrue(result.isFull())

    def test_getValidRangeForPart_beforeRegsEntryFactTrueAndAlsoRedefinedLocally_reinstatesCarriedSatisfaction(self):
        # "a" was already satisfied entering this part (entryFact True), *and* this part also redefines "a"
        # itself, locally, at index 1 -- IfContentPartColouring alone only sees the LOCAL redefinition, and
        # would bound the range to *after* it (ie. [(2, None)], as if position 0 -- before "a" is even locally
        # redefined -- weren't valid); _getForwardValidRangeForPart explicitly reinstates entryFact's carried-in
        # satisfaction, extending the range back down to the very start of the part instead
        part = FRB.IfContentPart({"x": [(0, "f")], "a": [(1, "2")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None})

        result = edit._getValidRangeForPart(part, {"a": True}, {}, {}, {})
        self.compareList(result.ranges, [(0, None)])

    def test_getValidRangeForPart_beforeRegsSatisfiedWithinThisPart_boundedAtItsIndex(self):
        part = FRB.IfContentPart({"a": [(0, "1")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.compareList(result.ranges, [(1, None)])

    def test_getValidRangeForPart_beforeAndAfterRegsContradict_returnsEmptyRange(self):
        # "c" must come before 'addition' (addition at-or-before index 2) while "a" must come after 'addition'
        # (addition after index 0) -- these can't both hold since "a" is already positioned before "c"
        part = FRB.IfContentPart({"a": [(0, "1")], "x": [(1, "f")], "c": [(2, "3")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"c": None}, afterRegs = {"a": None})

        result = edit._getValidRangeForPart(part, {}, {}, {}, {})
        self.assertTrue(result.isEmpty())

    # ---- call/return split: only relevant for a part that makes its own "run =" call ----
    # these exist because a part making a call can be satisfied two structurally different ways: something
    # guaranteed reachable *via* the call (Exit/Entry facts) vs something only guaranteed once that call has
    # actually *returned* (Return facts) -- and a call that never returns (eg. unconditional recursion with no
    # escape) must not let a position that can provably never execute "pass" via the wrong one of these two

    def test_getValidRangeForPart_afterRegsCallWithExitFactOnly_boundedAtOrBeforeTheCall(self):
        # "c" is guaranteed reachable via the call itself (exitFact), but NOT guaranteed once the call returns
        # (returnFact False) -- so only the position at-or-before the call ("run") itself is valid, not the
        # position after it
        part = FRB.IfContentPart({"run": [(0, "callee")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), afterRegs = {"c": None})

        result = edit._getValidRangeForPart(part, {}, {}, {"c": True}, {"c": False})
        # bounded at 0 (not unbounded below) -- the pre-call zone's own contribution is explicitly anchored at
        # the start of the part (index 0), not an unbounded-below range
        self.compareList(result.ranges, [(0, 1)])

    def test_getValidRangeForPart_afterRegsCallWithReturnFactOnly_boundedStrictlyAfterTheCall(self):
        # the reverse of the above: "c" is guaranteed only once the call has returned (returnFact True), but
        # not guaranteed via the call itself (exitFact False) -- so only the position strictly after "run" is
        # valid, not the position at-or-before it
        part = FRB.IfContentPart({"run": [(0, "callee")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), afterRegs = {"c": None})

        result = edit._getValidRangeForPart(part, {}, {}, {"c": False}, {"c": True})
        self.compareList(result.ranges, [(1, None)])

    def test_getValidRangeForPart_afterRegsCallWithNeitherFactNorLocalOccurence_returnsEmptyRange(self):
        part = FRB.IfContentPart({"run": [(0, "callee")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), afterRegs = {"c": None})

        result = edit._getValidRangeForPart(part, {}, {}, {"c": False}, {"c": False})
        self.assertTrue(result.isEmpty())

    def test_getValidRangeForPart_beforeRegsCallCalleeOnlyDefinesReg_validOnlyAfterTheCallReturns(self):
        # "a" isn't touched by 'part' at all, and isn't satisfied entering it either (entryFact False) -- it's
        # only ever satisfied because the callee itself defines it, which can only be observed once the call
        # has actually returned (returnFact True); the position at-or-before the call itself is NOT valid, since
        # "a" genuinely hasn't happened yet by then
        part = FRB.IfContentPart({"run": [(0, "callee")]})
        edit = FRB.RegSurroundedAdd(("ADD", "X"), beforeRegs = {"a": None})

        result = edit._getValidRangeForPart(part, {"a": False}, {"a": True}, {}, {})
        self.compareList(result.ranges, [(1, None)])

    # ================================================
    # ================ _keysExistSomewhere ===============

    def test_keysExistSomewhere_emptyKeys_returnsTrue(self):
        graph = FRB.IniSectionGraph({}, [])
        self.assertTrue(FRB.RegSurroundedAdd._keysExistSomewhere(graph, set()))

    def test_keysExistSomewhere_allKeysFoundAcrossParts_returnsTrue(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0),
                                             FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
                                             FRB.IfContentPart({"b": [(0, "2")]}, 1),
                                             FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)])}
        graph = FRB.IniSectionGraph(sections, ["root"])
        self.assertTrue(FRB.RegSurroundedAdd._keysExistSomewhere(graph, {"a", "b"}))

    def test_keysExistSomewhere_oneKeyMissing_returnsFalse(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])
        self.assertFalse(FRB.RegSurroundedAdd._keysExistSomewhere(graph, {"a", "zzz"}))

    # ================================================
    # ================== _pickInsertInd ==================

    def test_pickInsertInd_latestFalse_picksEarliestIndexOfFirstRange(self):
        edit = FRB.RegSurroundedAdd(("ADD", "X"))
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]})

        self.assertEqual(edit._pickInsertInd(FRB.Ranges([(1, 3), (5, 7)]), part), 1)

    def test_pickInsertInd_latestTrue_picksLatestIndexOfLastRange(self):
        edit = FRB.RegSurroundedAdd(("ADD", "X"), latest = True)
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]})

        self.assertEqual(edit._pickInsertInd(FRB.Ranges([(1, 3), (5, 7)]), part), 6)

    def test_pickInsertInd_latestTrueUnboundedEnd_fallsBackToPartLength(self):
        edit = FRB.RegSurroundedAdd(("ADD", "X"), latest = True)
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]})

        self.assertEqual(edit._pickInsertInd(FRB.Ranges([(1, None)]), part), len(part))

    # ================================================
    # ===================== edit ======================

    def _getContentPart(self, graph: FRB.IniSectionGraph, sectionName: str, partInd: int = 0) -> FRB.IfContentPart:
        return graph.getSection(sectionName).parts[partInd]

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
        # or a single key against a cyclic one) :raw-html:`<br />` :raw-html:`<br />`
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
        # _getBackwardValidRangeForPart's call/return split and _computeReachableNodes) :raw-html:`<br />` :raw-html:`<br />`
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
