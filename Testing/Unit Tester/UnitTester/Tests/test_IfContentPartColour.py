import sys
import copy as copyModule
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IfContentPartColourTest(BaseUnitTest):
    """
    Tests for :class:`FRB.IfContentPartColouring`/:class:`FRB.IfContentPartColourChange`

    .. note::
        ``updateColouring()`` (and any ``targetKeys`` passed into it) iterates its keys via a
        ``std::unordered_set`` on the C++ side, so the resulting insertion order into the
        colouring is **not** deterministic across runs when more than one key is involved.
        Tests that care about the tracked keys after an ``updateColouring()`` call compare with
        ``set(...)``/``compareDict``/direct lookups rather than asserting a specific
        ``keys()``/``items()`` order. Insertion-order tests instead drive the dict protocol
        (``set``/``__setitem__``) directly, which is deterministic.
    """

    def _makePart(self, src):
        return FRB.CppIfContentPart(src=src)

    # ================================================
    # ================ construction ===================

    def test_construct_noSrc_empty(self):
        colouring = FRB.IfContentPartColouring()
        self.assertEqual(len(colouring), 0)
        self.assertTrue(colouring.empty())

    def test_construct_withSrcDict_flatAndListValues(self):
        colouring = FRB.IfContentPartColouring({"a": "flatVal", "b": [(0, "x"), (2, "y")]})
        self.assertEqual(len(colouring), 2)
        self.assertEqual(colouring["a"], "flatVal")
        self.compareList(colouring["b"], [(0, "x"), (2, "y")])

    # ================================================
    # =============== dict protocol ====================

    def test_contains_and_dunderContains(self):
        colouring = FRB.IfContentPartColouring({"a": "x"})
        self.assertTrue(colouring.contains("a"))
        self.assertIn("a", colouring)
        self.assertFalse(colouring.contains("b"))
        self.assertNotIn("b", colouring)

    def test_len_and_size_match(self):
        colouring = FRB.IfContentPartColouring({"a": "x", "b": "y"})
        self.assertEqual(len(colouring), 2)
        self.assertEqual(colouring.size(), 2)

    def test_empty_trueWhenNoKeys_falseOtherwise(self):
        self.assertTrue(FRB.IfContentPartColouring().empty())
        self.assertFalse(FRB.IfContentPartColouring({"a": "x"}).empty())

    def test_get_existingKey_returnsValue(self):
        colouring = FRB.IfContentPartColouring({"a": "x"})
        self.assertEqual(colouring.get("a"), "x")

    def test_get_missingKey_returnsNoneByDefault(self):
        colouring = FRB.IfContentPartColouring()
        self.assertIsNone(colouring.get("a"))

    def test_get_missingKey_customDefault(self):
        colouring = FRB.IfContentPartColouring()
        self.assertEqual(colouring.get("a", "fallback"), "fallback")

    def test_getitem_existingKey_returnsValue(self):
        colouring = FRB.IfContentPartColouring({"a": "x"})
        self.assertEqual(colouring["a"], "x")

    def test_getitem_missingKey_raisesKeyError(self):
        colouring = FRB.IfContentPartColouring()
        with self.assertRaises(KeyError):
            colouring["a"]

    def test_set_and_setitem_insertOrOverwrite(self):
        colouring = FRB.IfContentPartColouring()
        colouring.set("a", "x")
        self.assertEqual(colouring["a"], "x")

        colouring["a"] = "y"
        self.assertEqual(colouring["a"], "y")

        colouring["b"] = [(0, "z")]
        self.compareList(colouring["b"], [(0, "z")])

    def test_erase_existingKey_removesAndReturnsTrue(self):
        colouring = FRB.IfContentPartColouring({"a": "x"})
        self.assertTrue(colouring.erase("a"))
        self.assertNotIn("a", colouring)

    def test_erase_missingKey_returnsFalse(self):
        colouring = FRB.IfContentPartColouring()
        self.assertFalse(colouring.erase("a"))

    def test_delitem_existingKey_removes(self):
        colouring = FRB.IfContentPartColouring({"a": "x"})
        del colouring["a"]
        self.assertNotIn("a", colouring)

    def test_delitem_missingKey_raisesKeyError(self):
        colouring = FRB.IfContentPartColouring()
        with self.assertRaises(KeyError):
            del colouring["a"]

    def test_clear_removesAllKeys(self):
        colouring = FRB.IfContentPartColouring({"a": "x", "b": "y"})
        colouring.clear()
        self.assertEqual(len(colouring), 0)

    def test_keys_and_items_matchInsertionOrder(self):
        colouring = FRB.IfContentPartColouring()
        colouring["a"] = "1"
        colouring["b"] = [(0, "2")]
        colouring["c"] = "3"

        self.compareList(colouring.keys(), ["a", "b", "c"])
        self.compareList(colouring.items(), [("a", "1"), ("b", [(0, "2")]), ("c", "3")])

    def test_iter_yieldsKeysInInsertionOrder(self):
        colouring = FRB.IfContentPartColouring()
        colouring["a"] = "1"
        colouring["b"] = "2"
        self.compareList(list(colouring), ["a", "b"])

    # ================================================
    # =============== updateColouring ===================

    def test_updateColouring_firstCall_tracksEveryKeyAsIndexedList(self):
        part = self._makePart({"a": [(0, "1"), (2, "3")], "b": [(1, "2")]})
        colouring = FRB.IfContentPartColouring()

        change = colouring.updateColouring(part)

        self.compareList(colouring["a"], [(0, "1"), (2, "3")])
        self.compareList(colouring["b"], [(1, "2")])
        self.compareSet(set(change.keys()), {"a", "b"})
        self.assertIsNone(change["a"].old)
        self.assertIsNone(change["b"].old)

    def test_updateColouring_targetKeys_onlyTracksSpecifiedKeys(self):
        part = self._makePart({"a": [(0, "1")], "b": [(0, "2")]})
        colouring = FRB.IfContentPartColouring()

        change = colouring.updateColouring(part, targetKeys={"a"})

        self.assertIn("a", colouring)
        self.assertNotIn("b", colouring)
        self.compareSet(set(change.keys()), {"a"})

    def test_updateColouring_targetKeys_keyNotInPart_silentlyIgnored(self):
        part = self._makePart({"a": [(0, "1")]})
        colouring = FRB.IfContentPartColouring()

        change = colouring.updateColouring(part, targetKeys={"a", "nonexistent"})

        self.assertIn("a", colouring)
        self.assertNotIn("nonexistent", colouring)
        self.compareSet(set(change.keys()), {"a"})

    def test_updateColouring_secondCall_updatePreviousKVPsTrue_flattensPreviousListsToLastValue(self):
        part1 = self._makePart({"a": [(0, "1"), (2, "3")], "b": [(1, "2")]})
        part2 = self._makePart({"c": [(0, "9")]})
        colouring = FRB.IfContentPartColouring()
        colouring.updateColouring(part1)

        change2 = colouring.updateColouring(part2, updatePreviousKVPs=True)

        self.assertEqual(colouring["a"], "3")
        self.assertEqual(colouring["b"], "2")
        self.compareList(colouring["c"], [(0, "9")])
        self.compareSet(set(change2.keys()), {"a", "b", "c"})
        self.compareList(change2["a"].old, [(0, "1"), (2, "3")])
        self.compareList(change2["b"].old, [(1, "2")])
        self.assertIsNone(change2["c"].old)

    def test_updateColouring_secondCall_updatePreviousKVPsFalse_leavesPreviousStateUntouched(self):
        # 'a' is the only key in part1, so its two occurrences get renumbered to true positional
        # indices 0 and 1 on construction -- the raw '0'/'2' passed into 'src' only controls
        # ordering, not the final stored index (see CppIfContentPart's src/buildFromOrder docs)
        part1 = self._makePart({"a": [(0, "1"), (2, "3")]})
        part2 = self._makePart({"c": [(0, "9")]})
        colouring = FRB.IfContentPartColouring()
        colouring.updateColouring(part1)

        change2 = colouring.updateColouring(part2, updatePreviousKVPs=False)

        self.compareList(colouring["a"], [(0, "1"), (1, "3")])
        self.compareList(colouring["c"], [(0, "9")])
        self.compareSet(set(change2.keys()), {"c"})

    def test_updateColouring_keyMissingFromNewPart_notTouched(self):
        # 'a' and 'b' share the same raw index (0); stable-sort keeps 'src' dict order for ties,
        # so 'a' ends up at true positional index 0 and 'b' at true positional index 1
        part1 = self._makePart({"a": [(0, "1")], "b": [(0, "2")]})
        part2 = self._makePart({"a": [(0, "9")]})
        colouring = FRB.IfContentPartColouring()
        colouring.updateColouring(part1)

        change2 = colouring.updateColouring(part2, updatePreviousKVPs=False)

        # 'b' was never touched by part2, so it keeps its original list-shaped state
        self.compareList(colouring["b"], [(1, "2")])
        self.compareSet(set(change2.keys()), {"a"})

    # ================================================
    # =================== restore =======================

    def test_restore_undoesUpdateColouring_backToPreviousState(self):
        part1 = self._makePart({"a": [(0, "1"), (2, "3")], "b": [(1, "2")]})
        part2 = self._makePart({"c": [(0, "9")]})
        colouring = FRB.IfContentPartColouring()
        change1 = colouring.updateColouring(part1)
        change2 = colouring.updateColouring(part2, updatePreviousKVPs=True)

        colouring.restore(change2)
        self.compareList(colouring["a"], [(0, "1"), (2, "3")])
        self.compareList(colouring["b"], [(1, "2")])
        self.assertNotIn("c", colouring)

        colouring.restore(change1)
        self.assertNotIn("a", colouring)
        self.assertNotIn("b", colouring)

    def test_restore_newlyTrackedKey_removedEntirely(self):
        part = self._makePart({"a": [(0, "1")]})
        colouring = FRB.IfContentPartColouring()
        change = colouring.updateColouring(part)

        colouring.restore(change)
        self.assertNotIn("a", colouring)

    # ================================================
    # ================== getIndVals ======================

    def test_getIndVals_indexedListState_returnsIndexValuePairs(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x"), (2, "y")]})
        self.compareList(colouring.getIndVals("a"), [(0, "x"), (2, "y")])

    def test_getIndVals_flatState_returnsNoneIndexUnfiltered(self):
        # the 'filter' predicate is intentionally NOT applied when the value was carried over
        # from a previous part (a flat state) -- unlike getVals()
        colouring = FRB.IfContentPartColouring({"a": "z"})
        self.compareList(colouring.getIndVals("a", filter=lambda ind, val: False), [(None, "z")])

    def test_getIndVals_missingKey_returnsEmptyList(self):
        colouring = FRB.IfContentPartColouring()
        self.compareList(colouring.getIndVals("a"), [])

    def test_getIndVals_withFilter_onlyMatchingIndexedValuesReturned(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x"), (2, "y"), (5, "x")]})
        self.compareList(colouring.getIndVals("a", filter=lambda ind, val: val == "x"), [(0, "x"), (5, "x")])

    # ================================================
    # =================== getVals ========================

    def test_getVals_indexedListState_preservesDuplicatesAndOrder(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x"), (1, "x"), (2, "y")]})
        self.compareList(colouring.getVals("a"), ["x", "x", "y"])

    def test_getVals_flatState_filterIsApplied(self):
        # unlike getIndVals(), getVals() DOES apply 'filter' to a carried-over flat value
        colouring = FRB.IfContentPartColouring({"a": "z"})
        self.compareList(colouring.getVals("a", filter=lambda ind, val: False), [])
        self.compareList(colouring.getVals("a", filter=lambda ind, val: True), ["z"])

    def test_getVals_missingKey_returnsEmptyList(self):
        colouring = FRB.IfContentPartColouring()
        self.compareList(colouring.getVals("a"), [])

    # ================================================
    # ================= getUniqueVals ====================

    def test_getUniqueVals_dedupesToASet(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x"), (1, "x"), (2, "y")]})
        self.compareSet(colouring.getUniqueVals("a"), {"x", "y"})

    def test_getUniqueVals_missingKey_returnsEmptySet(self):
        colouring = FRB.IfContentPartColouring()
        self.compareSet(colouring.getUniqueVals("a"), set())

    # ================================================
    # =================== getRanges ======================

    def test_getRanges_noArgs_returnsFullRange(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x")]})
        ranges = colouring.getRanges()
        self.assertTrue(ranges.isFull())

    def test_getRanges_keyFilters_returnsRangeUntilNextOccurrence(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x"), (3, "y"), (7, "z")]})
        ranges = colouring.getRanges(keyFilters={"a": lambda ind, val: val == "y"})

        self.assertFalse(ranges.has(2))
        self.assertTrue(ranges.has(3))
        self.assertTrue(ranges.has(6))
        self.assertFalse(ranges.has(7))

    def test_getRanges_keysExists_true_returnsRangeFromFirstOccurrenceOnward(self):
        colouring = FRB.IfContentPartColouring({"b": [(2, "p")]})
        ranges = colouring.getRanges(keysExists={"b": True})

        self.assertFalse(ranges.has(1))
        self.assertTrue(ranges.has(2))
        self.assertTrue(ranges.has(1000))

    def test_getRanges_keysExists_false_negatesExistenceRange(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x")]})
        ranges = colouring.getRanges(keysExists={"a": False})

        self.assertTrue(ranges.has(-1))
        self.assertFalse(ranges.has(0))

    def test_getRanges_existsRequireAll_intersectsAcrossKeys(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x")], "b": [(10, "y")]})
        ranges = colouring.getRanges(keysExists={"a": True, "b": True}, existsRequireAll=True)

        self.assertFalse(ranges.has(5))
        self.assertTrue(ranges.has(10))

    def test_getRanges_existsRequireAllFalse_unionsAcrossKeys(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x")], "b": [(10, "y")]})
        ranges = colouring.getRanges(keysExists={"a": True, "b": True}, existsRequireAll=False)

        self.assertTrue(ranges.has(5))
        self.assertFalse(ranges.has(-1))

    def test_getRanges_globalRequireAllFalse_unionsExistsAndFilters(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x")], "b": [(50, "y")]})

        intersected = colouring.getRanges(keysExists={"a": True}, keyFilters={"b": lambda ind, val: val == "y"}, globalRequireAll=True)
        unioned = colouring.getRanges(keysExists={"a": True}, keyFilters={"b": lambda ind, val: val == "y"}, globalRequireAll=False)

        self.assertFalse(intersected.has(10))
        self.assertTrue(unioned.has(10))

    def test_getRanges_includeKeyDefsFalse_excludesDefinitionIndices(self):
        colouring = FRB.IfContentPartColouring({"a": [(0, "x"), (3, "x"), (7, "x")]})
        ranges = colouring.getRanges(keyFilters={"a": lambda ind, val: True}, includeKeyDefs=False)

        self.assertFalse(ranges.has(0))
        self.assertTrue(ranges.has(1))
        self.assertFalse(ranges.has(3))
        self.assertTrue(ranges.has(4))
        self.assertFalse(ranges.has(7))
        self.assertTrue(ranges.has(8))

    def test_getRanges_unknownKey_treatedAsNotExistingOrNotMatching(self):
        colouring = FRB.IfContentPartColouring()
        existRanges = colouring.getRanges(keysExists={"a": True})
        self.assertTrue(existRanges.isEmpty())

        filterRanges = colouring.getRanges(keyFilters={"a": lambda ind, val: True})
        self.assertTrue(filterRanges.isFull())

    # ================================================
    # ===================== clone ========================

    def test_clone_isIndependentCopy(self):
        colouring = FRB.IfContentPartColouring({"a": "x"})
        clone = colouring.clone()

        self.assertEqual(clone["a"], "x")

        clone["b"] = "y"
        self.assertNotIn("b", colouring)

    def test_copyModule_copyAndDeepcopy_independentFromOriginal(self):
        colouring = FRB.IfContentPartColouring({"a": "x"})

        shallowCopy = copyModule.copy(colouring)
        shallowCopy["mutated1"] = "y"
        self.assertNotIn("mutated1", colouring)

        deepCopy = copyModule.deepcopy(colouring)
        deepCopy["mutated2"] = "z"
        self.assertNotIn("mutated2", colouring)

    # ================================================
    # ================ Python subclassing =================

    def test_pythonSubclass_overriddenMethod_calledWhenInvokedFromPython(self):
        class MyColouring(FRB.IfContentPartColouring):
            def getVals(self, key, filter=None):
                return ["overridden"]

        mc = MyColouring()
        mc["z"] = "1"
        self.assertEqual(mc.getVals("z"), ["overridden"])


class IfContentPartColourChangeTest(BaseUnitTest):
    """
    Tests for :class:`FRB.IfContentPartColourChange` in isolation, beyond what
    :class:`IfContentPartColourTest` already exercises via
    :meth:`FRB.IfContentPartColouring.updateColouring`/:meth:`FRB.IfContentPartColouring.restore`.
    """

    def test_construct_defaultOld_isNone(self):
        change = FRB.IfContentPartColourChange()
        self.assertIsNone(change.old)

    def test_construct_withOld_stored(self):
        change = FRB.IfContentPartColourChange(old="previous")
        self.assertEqual(change.old, "previous")

    def test_old_settable(self):
        change = FRB.IfContentPartColourChange()
        change.old = [(0, "x")]
        self.compareList(change.old, [(0, "x")])

    def test_restore_keyNotInColouring_noop(self):
        colouring = FRB.IfContentPartColouring()
        change = FRB.IfContentPartColourChange(old="x")
        change.restore(colouring, "a")
        self.assertNotIn("a", colouring)

    def test_restore_oldNone_deletesKey(self):
        colouring = FRB.IfContentPartColouring({"a": "x"})
        change = FRB.IfContentPartColourChange(old=None)
        change.restore(colouring, "a")
        self.assertNotIn("a", colouring)

    def test_restore_oldValue_restoresValue(self):
        colouring = FRB.IfContentPartColouring({"a": "current"})
        change = FRB.IfContentPartColourChange(old="previous")
        change.restore(colouring, "a")
        self.assertEqual(colouring["a"], "previous")

    def test_clone_isIndependentCopy(self):
        change = FRB.IfContentPartColourChange(old="x")
        clone = change.clone()
        self.assertEqual(clone.old, "x")

        clone.old = "y"
        self.assertEqual(change.old, "x")

    def test_copyModule_copyAndDeepcopy_independentFromOriginal(self):
        change = FRB.IfContentPartColourChange(old="x")

        shallowCopy = copyModule.copy(change)
        shallowCopy.old = "mutated1"
        self.assertEqual(change.old, "x")

        deepCopy = copyModule.deepcopy(change)
        deepCopy.old = "mutated2"
        self.assertEqual(change.old, "x")
