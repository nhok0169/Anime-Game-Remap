import sys
import copy as copyModule
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseOrderedMultiMapTest(BaseUnitTest):
    """
    Shared test battery for the ordered-multimap backing structures (:class:`FRB.OrderedMultiMap`,
    :class:`FRB.OrderedMultiMapSqrt`). Both are meant to be behaviorally interchangeable, so this
    class holds every test once; concrete subclasses (see test_OrderedMultiMap.py,
    test_OrderedMultiMapSqrt.py) just point '_mapClass' at the class under test.
    """

    _mapClass = None  # set by subclasses

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._items = [("a", 1), ("b", 2), ("a", 3), ("c", 4), ("a", 5)]

    def setUp(self):
        super().setUp()
        self._map = self._mapClass(self._items)

    # ================================================
    # ================ __init__ ======================

    def test_noArgs_emptyMapCreated(self):
        m = self._mapClass()
        self.assertEqual(len(m), 0)
        self.assertTrue(m.empty())

    def test_itemsGiven_mapBuiltInOrder(self):
        m = self._mapClass(self._items)
        self.compareList(m.entries(), self._items)

    # ================================================
    # ================ fromIndexed ===================

    def test_indexedDict_mapBuiltSortedByIndex(self):
        tests = [
            [{}, []],
            [{"x": [(0, "x0")]}, [("x", "x0")]],
            [{"a": [(2, "a2"), (0, "a0")], "b": [(1, "b1")]}, [("a", "a0"), ("b", "b1"), ("a", "a2")]],
            # gaps/ties -- index is a sort key, not a strict position; ties broken by dict order,
            # then list order within a key
            [{"a": [(0, "a0"), (0, "a0dup")], "b": [(0, "b0")]}, [("a", "a0"), ("a", "a0dup"), ("b", "b0")]],
        ]

        for data, expected in tests:
            m = self._mapClass.fromIndexed(data)
            self.compareList(m.entries(), expected)

    # ================================================
    # ================== insert ======================

    def test_insert_appendedToEnd(self):
        m = self._mapClass()
        m.insert("x", 1)
        m.insert("y", 2)
        self.compareList(m.entries(), [("x", 1), ("y", 2)])

    def test_insertStart_prependedToFront(self):
        m = self._mapClass([("x", 1)])
        m.insertStart("y", 2)
        m.insertStart("z", 3)
        self.compareList(m.entries(), [("z", 3), ("y", 2), ("x", 1)])

    def test_insertAt_variousIndices_insertedAtNormalizedPosition(self):
        tests = [
            # (index, expected entries after inserting ("Z", 0) into self._items)
            [0, [("Z", 0), ("a", 1), ("b", 2), ("a", 3), ("c", 4), ("a", 5)]],
            [2, [("a", 1), ("b", 2), ("Z", 0), ("a", 3), ("c", 4), ("a", 5)]],
            [5, [("a", 1), ("b", 2), ("a", 3), ("c", 4), ("a", 5), ("Z", 0)]],
            # -1 is a Python-style INSERTION-SLOT index (over the size()+1 valid slots 0..size()),
            # not list.insert()'s "before the last element" -- -1 means "append at the end", same
            # as index 5 or 100 below
            [-1, [("a", 1), ("b", 2), ("a", 3), ("c", 4), ("a", 5), ("Z", 0)]],
            [-6, [("Z", 0), ("a", 1), ("b", 2), ("a", 3), ("c", 4), ("a", 5)]],
            # out-of-range indices clamp, not throw
            [100, [("a", 1), ("b", 2), ("a", 3), ("c", 4), ("a", 5), ("Z", 0)]],
            [-100, [("Z", 0), ("a", 1), ("b", 2), ("a", 3), ("c", 4), ("a", 5)]],
        ]

        for index, expected in tests:
            m = self._mapClass(self._items)
            m.insertAt(index, "Z", 0)
            self.compareList(m.entries(), expected)

    def test_insertAllEnd_appendedInOrder(self):
        m = self._mapClass([("x", 1)])
        m.insertAllEnd([("y", 2), ("z", 3)])
        self.compareList(m.entries(), [("x", 1), ("y", 2), ("z", 3)])

    def test_insertAllStart_prependedInGivenOrder(self):
        m = self._mapClass([("x", 1)])
        m.insertAllStart([("y", 2), ("z", 3)])
        self.compareList(m.entries(), [("y", 2), ("z", 3), ("x", 1)])

    # ================================================
    # ================ insertAllAt ====================

    def test_insertAllAt_originalPositionSemantics_insertedAtOldPositions(self):
        # base = [a0, a1, a2] (indices refer to positions in the ORIGINAL 3-item sequence,
        # not the growing result)
        m = self._mapClass([("a", 0), ("a", 1), ("a", 2)])
        count = m.insertAllAt({0: ("x", "x0"), 2: ("y", "y2"), 3: ("z", "z3")})

        self.assertEqual(count, 3)
        self.compareList(m.entries(), [("x", "x0"), ("a", 0), ("a", 1), ("y", "y2"), ("a", 2), ("z", "z3")])

    def test_insertAllAt_withRanges_onlyInRangeIndicesInserted(self):
        m = self._mapClass([("a", 0), ("a", 1), ("a", 2)])
        count = m.insertAllAt({0: ("x", "x0"), 1: ("y", "y1"), 2: ("z", "z2")}, ranges=FRB.Ranges([(1, 2)]))

        self.assertEqual(count, 1)
        self.compareList(m.entries(), [("a", 0), ("y", "y1"), ("a", 1), ("a", 2)])

    def test_insertAllAt_emptyItems_nothingInserted(self):
        m = self._mapClass(self._items)
        count = m.insertAllAt({})
        self.assertEqual(count, 0)
        self.compareList(m.entries(), self._items)

    # ================================================
    # ================== reorder =====================

    def test_reorder_simpleSwap_entriesSwapped(self):
        m = self._mapClass([("x", 0), ("y", 1), ("z", 2)])
        m.reorder({0: 2, 2: 0})
        self.compareList(m.entries(), [("z", 2), ("y", 1), ("x", 0)])

    def test_reorder_negativeIndices_treatedPythonStyle(self):
        m = self._mapClass([("x", 0), ("y", 1), ("z", 2)])
        m.reorder({-1: -3})
        self.compareList(m.entries(), [("z", 2), ("x", 0), ("y", 1)])

    def test_reorder_overflowTargets_clusterAtFrontOrBack(self):
        m = self._mapClass([("w", 0), ("x", 1), ("y", 2), ("z", 3)])
        # index 0 sent far past the end -> back cluster; index 3 sent far before the front -> front cluster
        m.reorder({0: 100, 3: -100})
        self.compareList(m.entries(), [("z", 3), ("x", 1), ("y", 2), ("w", 0)])

    def test_reorder_conflictingOldIndex_firstInDictOrderWins(self):
        # keys 0 and -3 both refer to the same physical entry (size 3) -- first one in dict
        # iteration order wins, the later one for the same physical entry is ignored
        m = self._mapClass([("x", 0), ("y", 1), ("z", 2)])
        m.reorder({0: 2, -3: 0})
        self.compareList(m.entries(), [("y", 1), ("z", 2), ("x", 0)])

    def test_reorder_unmentionedEntries_keepRelativeOrderInLeftoverSlots(self):
        m = self._mapClass([("w", 0), ("x", 1), ("y", 2), ("z", 3)])
        m.reorder({3: 0})
        self.compareList(m.entries(), [("z", 3), ("w", 0), ("x", 1), ("y", 2)])

    def test_reorder_outOfRangeOldIndex_raisesIndexError(self):
        m = self._mapClass([("x", 0)])
        with self.assertRaises(IndexError):
            m.reorder({5: 0})

    def test_reorder_withRanges_ineligibleEntriesFloatInstead(self):
        m = self._mapClass([("w", 0), ("x", 1), ("y", 2), ("z", 3)])
        # only old index 0 is in range -- index 3's mapping is ignored entirely (floats)
        m.reorder({0: 3, 3: 0}, ranges=FRB.Ranges([(0, 1)]))
        self.compareList(m.entries(), [("x", 1), ("y", 2), ("z", 3), ("w", 0)])

    # ================================================
    # ================== removeAt ====================

    def test_removeAt_validPos_entryRemoved(self):
        m = self._mapClass(self._items)
        result = m.removeAt(1)
        self.assertTrue(result)
        self.compareList(m.entries(), [("a", 1), ("a", 3), ("c", 4), ("a", 5)])

    def test_removeAt_outOfBoundsPos_noopReturnsFalse(self):
        m = self._mapClass(self._items)
        result = m.removeAt(100)
        self.assertFalse(result)
        self.compareList(m.entries(), self._items)

    def test_removeAt_outsideRanges_noopReturnsFalse(self):
        m = self._mapClass(self._items)
        result = m.removeAt(0, ranges=FRB.Ranges([(1, 5)]))
        self.assertFalse(result)
        self.compareList(m.entries(), self._items)

    # ================================================
    # ================= removeKey ====================

    def test_removeKey_noFilters_allOccurrencesRemoved(self):
        m = self._mapClass(self._items)
        count = m.removeKey("a")
        self.assertEqual(count, 3)
        self.compareList(m.entries(), [("b", 2), ("c", 4)])

    def test_removeKey_missingKey_noopReturnsZero(self):
        m = self._mapClass(self._items)
        count = m.removeKey("nonexistent")
        self.assertEqual(count, 0)
        self.compareList(m.entries(), self._items)

    def test_removeKey_withRanges_onlyInRangeOccurrencesRemoved(self):
        m = self._mapClass(self._items)
        # "a" occurs at true positions 0, 2, 4 -- only position 0 is in [0, 1)
        count = m.removeKey("a", ranges=FRB.Ranges([(0, 1)]))
        self.assertEqual(count, 1)
        self.compareList(m.entries(), [("b", 2), ("a", 3), ("c", 4), ("a", 5)])

    def test_removeKey_checkArgOrder_indexThenValue(self):
        m = self._mapClass(self._items)
        seen = []
        m.removeKey("a", check=lambda ind, value: seen.append((ind, value)) or False)
        # "a" occurs at true positions 0, 2, 4
        self.compareList(seen, [(0, 1), (2, 3), (4, 5)])
        # check always returned False above -- nothing should have been removed
        self.compareList(m.entries(), self._items)

    def test_removeKey_withCheck_onlyPassingOccurrencesRemoved(self):
        m = self._mapClass(self._items)
        count = m.removeKey("a", check=lambda ind, value: value > 2)
        self.assertEqual(count, 2)
        self.compareList(m.entries(), [("a", 1), ("b", 2), ("c", 4)])

    # ================================================
    # ================= remapKeys ====================

    def test_remapKeys_bareKeyList_alwaysFiresInPlace(self):
        m = self._mapClass([("a", 1), ("b", 2)])
        m.remapKeys({"a": ["z"]})
        self.compareList(m.entries(), [("z", 1), ("b", 2)])

    def test_remapKeys_missingOldKey_neverTriggered(self):
        m = self._mapClass(self._items)
        m.remapKeys({"nonexistent": ["z"]})
        self.compareList(m.entries(), self._items)

    def test_remapKeys_conditionalRule_onlyFiresWhenCheckTrue(self):
        m = self._mapClass(self._items)
        rule = FRB.RemappedKeyData("bigA", check=lambda key, value: value > 2)
        m.remapKeys({"a": [rule]})
        # "a" occurrences with value <= 2 (val 1) don't fire -> removed (bare list default);
        # value 3 and 5 fire -> renamed
        self.compareList(m.entries(), [("b", 2), ("bigA", 3), ("c", 4), ("bigA", 5)])

    def test_remapKeys_keepKeyWithoutRemap_nonFiringOccurrenceKept(self):
        m = self._mapClass(self._items)
        rule = FRB.RemappedKeyData("bigA", check=lambda key, value: value > 2)
        m.remapKeys({"a": FRB.KeyRemapData([rule], keepKeyWithoutRemap=True)})
        self.compareList(m.entries(), [("a", 1), ("b", 2), ("bigA", 3), ("c", 4), ("bigA", 5)])

    def test_remapKeys_toIndRepositionsProducedGroup(self):
        m = self._mapClass([("a", 1), ("b", 2), ("a", 3)])
        rule = FRB.RemappedKeyData("z", toInd=0)
        m.remapKeys({"a": [rule]})
        # both "a" occurrences produce "z" entries, grouped and moved to the front as a unit
        self.compareList(m.entries(), [("z", 1), ("z", 3), ("b", 2)])

    def test_remapKeys_multipleRulesPerOccurrence_eachFiringRuleProducesAnEntry(self):
        m = self._mapClass([("a", 1)])
        m.remapKeys({"a": ["x", "y"]})
        self.compareList(m.entries(), [("x", 1), ("y", 1)])

    def test_remapKeys_withRanges_outOfRangeOccurrencePassesThrough(self):
        m = self._mapClass(self._items)
        # only true position 0 ("a", 1) is in range -- other "a" occurrences pass through unchanged
        m.remapKeys({"a": ["z"]}, ranges=FRB.Ranges([(0, 1)]))
        self.compareList(m.entries(), [("z", 1), ("b", 2), ("a", 3), ("c", 4), ("a", 5)])

    # ================================================
    # ================ replaceVals ====================

    def test_replaceVals_bareValue_allOccurrencesSet(self):
        m = self._mapClass(self._items)
        m.replaceVals({"a": 999})
        self.compareList(m.entries(), [("a", 999), ("b", 2), ("a", 999), ("c", 4), ("a", 999)])

    def test_replaceVals_replaceList_positionalAssignment(self):
        m = self._mapClass(self._items)
        m.replaceVals({"a": FRB.ReplaceList([10, 30, 50])})
        self.compareList(m.entries(), [("a", 10), ("b", 2), ("a", 30), ("c", 4), ("a", 50)])

    def test_replaceVals_replaceListShorterThanCount_extrasUntouched(self):
        m = self._mapClass(self._items)
        m.replaceVals({"a": FRB.ReplaceList([10])})
        self.compareList(m.entries(), [("a", 10), ("b", 2), ("a", 3), ("c", 4), ("a", 5)])

    def test_replaceVals_replaceIf_onlyPredicatePassingReplaced(self):
        m = self._mapClass(self._items)
        m.replaceVals({"a": FRB.ReplaceIf(999, lambda oldVal: oldVal > 2)})
        self.compareList(m.entries(), [("a", 1), ("b", 2), ("a", 999), ("c", 4), ("a", 999)])

    def test_replaceVals_missingKeyWithAddNewTrue_appendedAtEnd(self):
        m = self._mapClass([("a", 1)])
        m.replaceVals({"z": 100})
        self.compareList(m.entries(), [("a", 1), ("z", 100)])

    def test_replaceVals_missingKeyWithReplaceListAndAddNewTrue_oneEntryPerValue(self):
        m = self._mapClass([("a", 1)])
        m.replaceVals({"z": FRB.ReplaceList([10, 20, 30])})
        self.compareList(m.entries(), [("a", 1), ("z", 10), ("z", 20), ("z", 30)])

    def test_replaceVals_missingKeyWithAddNewFalse_skipped(self):
        m = self._mapClass([("a", 1)])
        m.replaceVals({"z": 100}, addNew=False)
        self.compareList(m.entries(), [("a", 1)])

    def test_replaceVals_withRanges_gatesExistingEntryUpdateOnly(self):
        m = self._mapClass(self._items)
        # only true position 0 in range -- only the first "a" entry updates
        m.replaceVals({"a": 999}, ranges=FRB.Ranges([(0, 1)]))
        self.compareList(m.entries(), [("a", 999), ("b", 2), ("a", 3), ("c", 4), ("a", 5)])

    def test_replaceVals_replaceListWithRanges_doesNotReindexSkippedSlots(self):
        # ranges gates whether an already-paired (i-th entry, list[i]) update fires -- it does NOT
        # reindex to skip ineligible entries and shift list[i] onto the next eligible one
        m = self._mapClass(self._items)
        # "a" occurrences are at true positions 0, 2, 4; only position 2 is in range
        m.replaceVals({"a": FRB.ReplaceList([10, 30, 50])}, ranges=FRB.Ranges([(2, 3)]))
        self.compareList(m.entries(), [("a", 1), ("b", 2), ("a", 30), ("c", 4), ("a", 5)])

    # ================================================
    # ========= contains/containsKey/count ===========

    def test_contains_variousKeys_correctBoolReturned(self):
        tests = [["a", True], ["b", True], ["nonexistent", False]]
        for key, expected in tests:
            self.assertEqual(self._map.contains(key), expected)
            self.assertEqual(self._map.containsKey(key), expected)
            self.assertEqual(key in self._map, expected)

    def test_count_variousKeys_correctCountReturned(self):
        tests = [["a", 3], ["b", 1], ["nonexistent", 0]]
        for key, expected in tests:
            self.assertEqual(self._map.count(key), expected)

    # ================================================
    # ========= size/length/empty/__len__ ============

    def test_size_matchesEntryCount(self):
        self.assertEqual(self._map.size(), len(self._items))
        self.assertEqual(self._map.length(), len(self._items))
        self.assertEqual(len(self._map), len(self._items))

    def test_empty_emptyAndNonEmptyMaps_correctBoolReturned(self):
        self.assertFalse(self._map.empty())
        self.assertTrue(self._mapClass().empty())

    # ================================================
    # ================== getAll ======================

    def test_getAll_ordered_returnsValuesInPositionalOrder(self):
        self.compareList(self._map.getAll("a"), [1, 3, 5])

    def test_getAll_unordered_returnsValuesInCallOrder(self):
        m = self._mapClass()
        m.insert("a", 3)
        m.insertStart("a", 1)  # call order: 3 added first, then 1 -- but 1 is positionally first
        self.compareList(m.getAll("a", ordered=False), [3, 1])
        self.compareList(m.getAll("a", ordered=True), [1, 3])

    def test_getAll_missingKey_emptyList(self):
        self.compareList(self._map.getAll("nonexistent"), [])

    def test_getAll_withRanges_onlyInRangeValuesReturned(self):
        self.compareList(self._map.getAll("a", ranges=FRB.Ranges([(0, 3)])), [1, 3])
        self.compareList(self._map.getAll("a", ranges=FRB.Ranges([(4, 5)])), [5])
        self.compareList(self._map.getAll("a", ranges=FRB.Ranges([(100, 200)])), [])

    def test_getAll_rangesNone_equivalentToOmitted(self):
        self.compareList(self._map.getAll("a", ranges=None), self._map.getAll("a"))

    def test_getAllWithInds_pairsValueWithTruePositionalIndex(self):
        self.compareList(self._map.getAllWithInds("a"), [(0, 1), (2, 3), (4, 5)])

    def test_getAllWithInds_withRanges_onlyInRangePairsReturned(self):
        self.compareList(self._map.getAllWithInds("a", ranges=FRB.Ranges([(0, 3)])), [(0, 1), (2, 3)])
        self.compareList(self._map.getAllWithInds("a", ranges=FRB.Ranges([(4, 5)])), [(4, 5)])

    # ================================================
    # ================== getKeys ======================

    def test_getKeys_returnsEveryDistinctKeyAsASet(self):
        result = self._map.getKeys()
        self.assertIsInstance(result, set)
        self.assertEqual(result, {"a", "b", "c"})

    def test_getKeys_emptyMap_emptySet(self):
        m = self._mapClass([])
        self.assertEqual(m.getKeys(), set())

    # ================================================
    # ================== keySize ======================

    def test_keySize_returnsNumberOfDistinctKeys(self):
        self.assertEqual(self._map.keySize(), 3)
        self.assertEqual(self._map.keySize(), len(self._map.getKeys()))

    def test_keySize_emptyMap_zero(self):
        m = self._mapClass([])
        self.assertEqual(m.keySize(), 0)

    # ================================================
    # ================= getByInd =====================

    def test_getByInd_variousIndices_correctEntryReturned(self):
        tests = [
            [0, ("a", 1)],
            [4, ("a", 5)],
            [-1, ("a", 5)],
            [-5, ("a", 1)],
        ]
        for index, expected in tests:
            self.assertEqual(self._map.getByInd(index), expected)
            self.assertEqual(self._map[index], expected)

    def test_getByInd_outOfRange_raisesIndexError(self):
        tests = [5, -6, 100, -100]
        for index in tests:
            with self.assertRaises(IndexError):
                self._map.getByInd(index)

    def test_getByInd_emptyMap_raisesIndexError(self):
        m = self._mapClass()
        with self.assertRaises(IndexError):
            m.getByInd(0)

    def test_getByIndWithOccurrence_pairsValueWithOccurrenceIndex(self):
        tests = [
            [0, (0, 1)],  # first "a" -- occurrence 0
            [1, (0, 2)],  # "b" -- occurrence 0
            [2, (1, 3)],  # second "a" -- occurrence 1
            [4, (2, 5)],  # third "a" -- occurrence 2
        ]
        for index, expected in tests:
            self.assertEqual(self._map.getByIndWithOccurrence(index), expected)

    # ================================================
    # ================ setValByInd =====================

    def test_setValByInd_variousIndices_valueUpdatedKeyUntouched(self):
        self._map.setValByInd(0, 100)
        self._map.setValByInd(-1, 500)
        self.compareList(self._map.entries(), [("a", 100), ("b", 2), ("a", 3), ("c", 4), ("a", 500)])

    def test_setValByInd_doesNotDisturbOtherLookups(self):
        self._map.setValByInd(2, 300)
        # true positional indices for "a"'s occurrences are unaffected by a value-only change
        self.compareList(self._map.getAllWithInds("a"), [(0, 1), (2, 300), (4, 5)])

    def test_setValByInd_outOfRange_raisesIndexError(self):
        tests = [5, -6, 100, -100]
        for index in tests:
            with self.assertRaises(IndexError):
                self._map.setValByInd(index, 999)

    def test_setValByInd_emptyMap_raisesIndexError(self):
        m = self._mapClass()
        with self.assertRaises(IndexError):
            m.setValByInd(0, 1)

    # ================================================
    # ================== entries =====================

    def test_entries_matchesInsertionOrder(self):
        self.compareList(self._map.entries(), self._items)

    def test_entries_isACopy_mutatingResultDoesNotAffectMap(self):
        result = self._map.entries()
        result.append(("z", 999))
        self.compareList(self._map.entries(), self._items)

    # ================================================
    # ================= splitByInds ==================

    def test_splitByInds_singleSplitPoint_twoParts(self):
        m = self._mapClass(self._items)
        parts = m.splitByInds([2])
        self.assertEqual(len(parts), 2)
        self.compareList(parts[0].entries(), [("a", 1), ("b", 2)])
        self.compareList(parts[1].entries(), [("a", 3), ("c", 4), ("a", 5)])

    def test_splitByInds_excludeSplitKVP_splitEntryDropped(self):
        m = self._mapClass(self._items)
        parts = m.splitByInds([2], includeSplitKVP=False)
        self.assertEqual(len(parts), 2)
        self.compareList(parts[0].entries(), [("a", 1), ("b", 2)])
        self.compareList(parts[1].entries(), [("c", 4), ("a", 5)])

    def test_splitByInds_includeEmptyParts_emptyPartsKept(self):
        m = self._mapClass([("a", 1)])
        parts = m.splitByInds([0], includeEmptyParts=True)
        self.assertEqual(len(parts), 2)
        self.compareList(parts[0].entries(), [])
        self.compareList(parts[1].entries(), [("a", 1)])

    def test_splitByInds_excludeEmptyParts_emptyPartsDropped(self):
        m = self._mapClass([("a", 1)])
        parts = m.splitByInds([0], includeEmptyParts=False)
        self.assertEqual(len(parts), 1)
        self.compareList(parts[0].entries(), [("a", 1)])

    def test_splitByInds_outOfRangeIndex_raisesIndexError(self):
        m = self._mapClass(self._items)
        with self.assertRaises(IndexError):
            m.splitByInds([100])

    def test_splitByInds_partsAreIndependent_mutatingOnePartDoesNotAffectOthers(self):
        m = self._mapClass(self._items)
        parts = m.splitByInds([2])

        parts[0].insert("mutated", 111)
        parts[1].insert("mutated", 222)

        self.compareList(m.entries(), self._items)
        self.compareList(parts[0].entries(), [("a", 1), ("b", 2), ("mutated", 111)])
        self.compareList(parts[1].entries(), [("a", 3), ("c", 4), ("a", 5), ("mutated", 222)])

    # ================================================
    # =================== __iter__ ===================

    def test_iter_yieldsKeyValueOccurrenceIndexOrderIndexTuples(self):
        result = list(self._map)
        expected = [("a", 1, 0, 0), ("b", 2, 0, 1), ("a", 3, 1, 2), ("c", 4, 0, 3), ("a", 5, 2, 4)]
        self.compareList(result, expected)

    def test_iter_emptyMap_noIterations(self):
        m = self._mapClass()
        self.compareList(list(m), [])

    # ================================================
    # =========== copy/__copy__/__deepcopy__ =========

    def test_copy_independentFromOriginal(self):
        original = self._mapClass(self._items)
        result = original.copy()

        self.compareList(result.entries(), self._items)

        result.insert("mutated", 999)
        self.compareList(original.entries(), self._items)
        self.assertEqual(len(result), len(self._items) + 1)

    def test_copyModule_copy_independentFromOriginal(self):
        original = self._mapClass(self._items)
        result = copyModule.copy(original)

        result.insert("mutated", 999)
        self.compareList(original.entries(), self._items)

    def test_copyModule_deepcopy_independentFromOriginal(self):
        original = self._mapClass(self._items)
        result = copyModule.deepcopy(original)

        result.insert("mutated", 999)
        self.compareList(original.entries(), self._items)
