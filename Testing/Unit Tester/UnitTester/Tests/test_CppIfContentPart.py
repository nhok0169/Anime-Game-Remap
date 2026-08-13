import sys
import copy as copyModule
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class PyListOMM(FRB.IOrderedMultiMap):
    """
    Same minimal, deliberately naive pure-Python IOrderedMultiMap used in
    test_IOrderedMultiMap.py -- exists here to prove IfContentPart works identically
    regardless of which IOrderedMultiMap implementation backs it.
    """

    def __init__(self):
        super().__init__()
        self._items = []

    def insert(self, key, value):
        self._items.append([key, value])

    def insertStart(self, key, value):
        self._items.insert(0, [key, value])

    def insertAt(self, index, key, value):
        self._items.insert(index, [key, value])

    def insertAllEnd(self, items):
        for k, v in items:
            self._items.append([k, v])

    def insertAllStart(self, items):
        for k, v in reversed(items):
            self._items.insert(0, [k, v])

    def insertAllAt(self, items, sortIndices=True, ranges=None):
        for idx, (k, v) in items:
            self._items.insert(idx, [k, v])
        return len(items)

    def reorder(self, orderMap, ranges=None):
        raise NotImplementedError

    def removeAt(self, pos, ranges=None):
        if 0 <= pos < len(self._items):
            del self._items[pos]
            return True
        return False

    def removeKey(self, key, ranges=None, check=None):
        before = len(self._items)
        self._items = [kv for kv in self._items if kv[0] != key]
        return before - len(self._items)

    def remapKeys(self, keyRemap, ranges=None):
        raise NotImplementedError

    def replaceVals(self, newVals, addNew=True, ranges=None):
        raise NotImplementedError

    def contains(self, key):
        return any(k == key for k, _ in self._items)

    def containsKey(self, key):
        return self.contains(key)

    def count(self, key):
        return sum(1 for k, _ in self._items if k == key)

    def size(self):
        return len(self._items)

    def length(self):
        return len(self._items)

    def empty(self):
        return len(self._items) == 0

    def _inRanges(self, i, ranges):
        # ranges arrives here as a plain List[Tuple[int, int]] (or None), not a bound
        # FRB.Ranges -- crossing from C++ into a Python override, IOrderedMultiMap's 'ranges'
        # params use RangeSpec (a plain vector shape), never the polymorphic Ranges<T> itself
        # (see IOrderedMultiMap.RangeSpec's own doc comment for why). Half-open [start, end).
        if ranges is None:
            return True
        return any(start <= i < end for start, end in ranges)

    def getAll(self, key, ordered=True, ranges=None):
        return [v for i, (k, v) in enumerate(self._items) if k == key and self._inRanges(i, ranges)]

    def getAllWithInds(self, key, ordered=True, ranges=None):
        return [(i, v) for i, (k, v) in enumerate(self._items) if k == key and self._inRanges(i, ranges)]

    def getByInd(self, index):
        k, v = self._items[index]
        return (k, v)

    def getByIndWithOccurrence(self, index):
        k, v = self._items[index]
        occ = sum(1 for kk, _ in self._items[:index] if kk == k)
        return (occ, v)

    def setValByInd(self, index, value):
        self._items[index][1] = value

    def entries(self):
        return [tuple(kv) for kv in self._items]

    def items(self):
        return [(k, v, 0, i) for i, (k, v) in enumerate(self._items)]

    def splitByInds(self, inds, includeSplitKVP=True, includeEmptyParts=False, sortIndices=True):
        raise NotImplementedError

    def clone(self):
        c = PyListOMM()
        c._items = [list(kv) for kv in self._items]
        return c


class CppIfContentPartTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._items = [("hash", "abc123"), ("vb0", "ResourceX"), ("vb0", "ResourceY")]

    def _makePart(self, items=None, depth=0):
        if items is None:
            items = self._items
        return FRB.CppIfContentPart(depth=depth, content=FRB.OrderedMultiMap(list(items)).asInterface())

    # ================================================
    # ========== construction / backing choice =======

    def test_backedByOrderedMultiMap_entriesMatch(self):
        part = FRB.CppIfContentPart(depth=1, content=FRB.OrderedMultiMap(self._items).asInterface())
        self.compareList(part.entries(), self._items)
        self.assertEqual(part.depth, 1)

    def test_backedByOrderedMultiMapSqrt_entriesMatch(self):
        part = FRB.CppIfContentPart(depth=1, content=FRB.OrderedMultiMapSqrt(self._items).asInterface())
        self.compareList(part.entries(), self._items)

    def test_backedByPurePythonImplementation_entriesMatch(self):
        pyomm = PyListOMM()
        for key, value in self._items:
            pyomm.insert(key, value)

        part = FRB.CppIfContentPart(depth=3, content = pyomm)
        self.compareList(part.entries(), self._items)
        self.assertEqual(part.depth, 3)

        # mutating through IfContentPart really does cross back into the Python object
        part.addKVP("newKey", "newVal")
        self.compareList(part.entries(), self._items + [("newKey", "newVal")])

    def test_backedByPurePythonImplementation_getValsWithRanges_callsThroughTrampolineCorrectly(self):
        # Regression guard: getVals()/getValsWithInds() gaining a 'ranges' parameter changed
        # IOrderedMultiMap.getAll()/getAllWithInds()'s arity -- a pure-Python override with the
        # old 2-param signature would fail on *every* call (not just ones using ranges), since
        # PYBIND11_OVERRIDE_PURE forwards all 3 args positionally regardless of what the
        # Python method declares. This part's own 'vb0' occurs at true positions 1, 2.
        pyomm = PyListOMM()
        for key, value in self._items:
            pyomm.insert(key, value)

        part = FRB.CppIfContentPart(content=pyomm)
        self.compareList(part.getVals("vb0"), ["ResourceX", "ResourceY"])
        self.compareList(part.getVals("vb0", ranges=FRB.Ranges([(1, 2)])), ["ResourceX"])
        self.compareList(part.getValsWithInds("vb0", ranges=FRB.Ranges([(2, 3)])), [(2, "ResourceY")])

    def test_depth_defaultsToZero(self):
        part = FRB.CppIfContentPart(content=FRB.OrderedMultiMap().asInterface())
        self.assertEqual(part.depth, 0)

    def test_content_omitted_defaultsToEmptyOrderedMultiMap(self):
        part = FRB.CppIfContentPart()
        self.assertTrue(part.empty())
        self.assertEqual(part.depth, 0)

        part.addKVP("hash", "abc")
        self.compareList(part.entries(), [("hash", "abc")])

    def test_content_explicitNone_defaultsToEmptyOrderedMultiMap(self):
        part = FRB.CppIfContentPart(content=None, depth=3)
        self.assertTrue(part.empty())
        self.assertEqual(part.depth, 3)

    def test_content_omittedWithOnlyDepthGiven_defaultsCorrectly(self):
        part = FRB.CppIfContentPart(depth=5)
        self.assertEqual(part.depth, 5)
        self.assertTrue(part.empty())

    def test_content_omitted_defaultedPartsAreIndependent(self):
        partA = FRB.CppIfContentPart()
        partB = FRB.CppIfContentPart()

        partA.addKVP("onlyA", "1")

        self.compareList(partA.entries(), [("onlyA", "1")])
        self.compareList(partB.entries(), [])

    def test_depth_settable(self):
        part = self._makePart()
        part.depth = 9
        self.assertEqual(part.depth, 9)

    # ================================================
    # ================== content ======================

    def test_content_exposesUnderlyingInterface(self):
        part = self._makePart()
        self.assertIsInstance(part.content, FRB.IOrderedMultiMap)
        self.compareList(part.content.entries(), self._items)

        # mutating via .content affects the part too -- it's the same underlying object
        part.content.insert("viaContent", "x")
        self.compareList(part.entries(), self._items + [("viaContent", "x")])

    # ================================================
    # ================ insert family ==================

    def test_addKVP_appendedToEnd(self):
        part = self._makePart()
        part.addKVP("new", "val")
        self.compareList(part.entries(), self._items + [("new", "val")])

    def test_addKVPToFront_prependedToFront(self):
        part = self._makePart()
        part.addKVPToFront("new", "val")
        self.compareList(part.entries(), [("new", "val")] + self._items)

    def test_addKVPAt_insertedAtPosition(self):
        part = self._makePart()
        part.addKVPAt(1, "new", "val")
        expected = list(self._items)
        expected.insert(1, ("new", "val"))
        self.compareList(part.entries(), expected)

    def test_addKVPs_appendedInOrder(self):
        part = self._makePart()
        part.addKVPs([("a", "1"), ("b", "2")])
        self.compareList(part.entries(), self._items + [("a", "1"), ("b", "2")])

    def test_addKVPsToFront_prependedInGivenOrder(self):
        part = self._makePart()
        part.addKVPsToFront([("a", "1"), ("b", "2")])
        self.compareList(part.entries(), [("a", "1"), ("b", "2")] + self._items)

    def test_addKVPsByInds_insertedAtOriginalPositions(self):
        part = FRB.CppIfContentPart(content=FRB.OrderedMultiMap([("a", "0"), ("a", "1"), ("a", "2")]).asInterface())
        count = part.addKVPsByInds({0: ("x", "x0"), 2: ("y", "y2")})
        self.assertEqual(count, 2)
        self.compareList(part.entries(), [("x", "x0"), ("a", "0"), ("a", "1"), ("y", "y2"), ("a", "2")])

    def test_addKVPsByInds_withRanges_onlyInRangeInserted(self):
        part = FRB.CppIfContentPart(content=FRB.OrderedMultiMap([("a", "0"), ("a", "1")]).asInterface())
        count = part.addKVPsByInds({0: ("x", "x0"), 1: ("y", "y1")}, ranges=FRB.Ranges([(1, 2)]))
        self.assertEqual(count, 1)
        self.compareList(part.entries(), [("a", "0"), ("y", "y1"), ("a", "1")])

    # ================================================
    # =================== removal ======================

    def test_removeKVPAt_removesEntryAtPosition(self):
        part = self._makePart()
        result = part.removeKVPAt(0)
        self.assertTrue(result)
        self.compareList(part.entries(), self._items[1:])

    def test_removeKVPAt_outOfBounds_noopReturnsFalse(self):
        part = self._makePart()
        result = part.removeKVPAt(100)
        self.assertFalse(result)
        self.compareList(part.entries(), self._items)

    def test_removeKey_removesAllOccurrences(self):
        part = self._makePart()
        count = part.removeKey("vb0")
        self.assertEqual(count, 2)
        self.compareList(part.entries(), [("hash", "abc123")])

    def test_removeKey_withCheck_onlyPassingOccurrencesRemoved(self):
        part = self._makePart()
        count = part.removeKey("vb0", check=lambda ind, value: value == "ResourceX")
        self.assertEqual(count, 1)
        self.compareList(part.entries(), [("hash", "abc123"), ("vb0", "ResourceY")])

    def test_removeKey_checkArgOrder_indexThenValue(self):
        part = self._makePart()
        seen = []
        part.removeKey("vb0", check=lambda ind, value: seen.append((ind, value)) or False)
        # true positional indices for the two "vb0" occurrences in self._items
        self.compareList(seen, [(1, "ResourceX"), (2, "ResourceY")])
        # check always returned False above -- nothing should have been removed
        self.compareList(part.entries(), self._items)

    def test_removeKeys_removesEveryGivenKey(self):
        part = self._makePart()
        count = part.removeKeys({"hash": None, "vb0": None})
        self.assertEqual(count, 3)
        self.assertTrue(part.empty())

    def test_removeKeys_withPerKeyCheck_onlyPassingOccurrencesRemoved(self):
        part = self._makePart()
        count = part.removeKeys({"vb0": lambda ind, value: value == "ResourceY"})
        self.assertEqual(count, 1)
        self.compareList(part.entries(), [("hash", "abc123"), ("vb0", "ResourceX")])

    def test_removeKeys_checkArgOrder_indexThenValue(self):
        part = self._makePart()
        seen = []
        part.removeKeys({"vb0": lambda ind, value: seen.append((ind, value)) or False})
        self.compareList(seen, [(1, "ResourceX"), (2, "ResourceY")])
        self.compareList(part.entries(), self._items)

    # ================================================
    # ================= bulk edits =====================

    def test_reorder_swapsEntries(self):
        part = FRB.CppIfContentPart(content=FRB.OrderedMultiMap([("x", "0"), ("y", "1"), ("z", "2")]).asInterface())
        part.reorder({0: 2, 2: 0})
        self.compareList(part.entries(), [("z", "2"), ("y", "1"), ("x", "0")])

    def test_remapKeys_bareKeyList_alwaysFiresInPlace(self):
        part = FRB.CppIfContentPart(content=FRB.OrderedMultiMap([("hash", "abc")]).asInterface())
        part.remapKeys({"hash": ["newHash"]})
        self.compareList(part.entries(), [("newHash", "abc")])

    def test_remapKeys_withRemappedKeyData_conditionalFiring(self):
        part = self._makePart()
        rule = FRB.RemappedKeyData("vb0New", check=lambda key, value: value == "ResourceX")
        part.remapKeys({"vb0": [rule]})
        self.compareList(part.entries(), [("hash", "abc123"), ("vb0New", "ResourceX")])

    def test_replaceVals_bareValue_allOccurrencesSet(self):
        part = self._makePart()
        part.replaceVals({"vb0": "REPLACED"})
        self.compareList(part.entries(), [("hash", "abc123"), ("vb0", "REPLACED"), ("vb0", "REPLACED")])

    def test_replaceVals_replaceList_positionalAssignment(self):
        part = self._makePart()
        part.replaceVals({"vb0": FRB.ReplaceList(["first", "second"])})
        self.compareList(part.entries(), [("hash", "abc123"), ("vb0", "first"), ("vb0", "second")])

    def test_replaceVals_replaceIf_conditionalReplacement(self):
        part = self._makePart()
        part.replaceVals({"vb0": FRB.ReplaceIf("REPLACED", lambda oldVal: oldVal == "ResourceX")})
        self.compareList(part.entries(), [("hash", "abc123"), ("vb0", "REPLACED"), ("vb0", "ResourceY")])

    # ================================================
    # =================== lookups ======================

    def test_contains_variousKeys_correctBoolReturned(self):
        part = self._makePart()
        self.assertTrue(part.contains("hash"))
        self.assertTrue(part.containsKey("vb0"))
        self.assertTrue("hash" in part)
        self.assertFalse(part.contains("nonexistent"))

    def test_count_variousKeys_correctCountReturned(self):
        part = self._makePart()
        self.assertEqual(part.count("vb0"), 2)
        self.assertEqual(part.count("nonexistent"), 0)

    def test_size_length_len_allMatch(self):
        part = self._makePart()
        self.assertEqual(part.size(), 3)
        self.assertEqual(part.length(), 3)
        self.assertEqual(len(part), 3)

    def test_empty_emptyAndNonEmptyParts(self):
        self.assertFalse(self._makePart().empty())
        self.assertTrue(FRB.CppIfContentPart(content=FRB.OrderedMultiMap().asInterface()).empty())

    def test_getVals_returnsAllValuesForKey(self):
        part = self._makePart()
        self.compareList(part.getVals("vb0"), ["ResourceX", "ResourceY"])
        self.compareList(part.getVals("nonexistent"), [])

    def test_getVals_withRanges_onlyInRangeValuesReturned(self):
        part = self._makePart()
        self.compareList(part.getVals("vb0", ranges=FRB.Ranges([(1, 2)])), ["ResourceX"])
        self.compareList(part.getVals("vb0", ranges=FRB.Ranges([(2, 3)])), ["ResourceY"])
        self.compareList(part.getVals("vb0", ranges=FRB.Ranges([(0, 1)])), [])

    def test_getVals_withRawListRanges_equivalentToRangesObject(self):
        part = self._makePart()
        self.compareList(part.getVals("vb0", ranges=[(1, 2)]), part.getVals("vb0", ranges=FRB.Ranges([(1, 2)])))

    def test_getVals_rangesNone_equivalentToOmitted(self):
        part = self._makePart()
        self.compareList(part.getVals("vb0", ranges=None), part.getVals("vb0"))

    def test_getValsWithInds_pairsValueWithTrueIndex(self):
        part = self._makePart()
        self.compareList(part.getValsWithInds("vb0"), [(1, "ResourceX"), (2, "ResourceY")])

    def test_getValsWithInds_withRanges_onlyInRangePairsReturned(self):
        part = self._makePart()
        self.compareList(part.getValsWithInds("vb0", ranges=FRB.Ranges([(1, 2)])), [(1, "ResourceX")])
        self.compareList(part.getValsWithInds("vb0", ranges=FRB.Ranges([(2, 3)])), [(2, "ResourceY")])

    def test_getKeys_returnsEveryDistinctKeyAsASet(self):
        part = self._makePart()
        result = part.getKeys()
        self.assertIsInstance(result, set)
        self.assertEqual(result, {"hash", "vb0"})

    def test_getKeys_emptyPart_emptySet(self):
        part = FRB.CppIfContentPart()
        self.assertEqual(part.getKeys(), set())

    def test_keySize_returnsNumberOfDistinctKeys(self):
        part = self._makePart()
        self.assertEqual(part.keySize(), 2)
        self.assertEqual(part.keySize(), len(part.getKeys()))

    def test_keySize_emptyPart_zero(self):
        part = FRB.CppIfContentPart()
        self.assertEqual(part.keySize(), 0)

    def test_getByInd_variousIndices(self):
        part = self._makePart()
        self.assertEqual(part.getByInd(0), ("hash", "abc123"))
        self.assertEqual(part[0], ("hash", "abc123"))
        self.assertEqual(part.getByInd(-1), ("vb0", "ResourceY"))

    def test_getByInd_outOfRange_raisesIndexError(self):
        part = self._makePart()
        with self.assertRaises(IndexError):
            part.getByInd(100)

    def test_getitem_withStrKey_equivalentToGetValsOrdered(self):
        part = self._makePart()
        self.compareList(part["vb0"], ["ResourceX", "ResourceY"])
        self.compareList(part["vb0"], part.getVals("vb0", ordered=True))

    def test_getitem_withStrKey_nonexistentKey_raisesKeyError(self):
        part = self._makePart()
        with self.assertRaises(KeyError):
            part["nonexistent"]
        # getVals itself is unaffected -- still returns an empty list, no exception
        self.compareList(part.getVals("nonexistent"), [])

    def test_getitem_unsupportedKeyType_raisesTypeError(self):
        part = self._makePart()
        with self.assertRaises(TypeError):
            part[3.14]

    # ================================================
    # ===================== get =======================

    def test_get_intKey_found_returnsSameAsGetByInd(self):
        part = self._makePart()
        self.assertEqual(part.get(0), part.getByInd(0))
        self.assertEqual(part.get(-1), part.getByInd(-1))

    def test_get_intKey_outOfRange_defaultErrorOnNotFound_returnsDefault(self):
        part = self._makePart()
        self.assertIsNone(part.get(100))
        self.assertEqual(part.get(100, default="fallback"), "fallback")

    def test_get_intKey_outOfRange_errorOnNotFoundTrue_raisesIndexError(self):
        part = self._makePart()
        with self.assertRaises(IndexError):
            part.get(100, errorOnNotFound=True)

    def test_get_strKey_defaultWithInds_returnsSameAsGetVals(self):
        part = self._makePart()
        self.compareList(part.get("vb0"), part.getVals("vb0", ordered=True))

    def test_get_strKey_found_withIndsTrue_returnsSameAsGetValsWithInds(self):
        part = self._makePart()
        self.compareList(part.get("vb0", withInds=True), part.getValsWithInds("vb0", ordered=True))

    def test_get_strKey_found_withIndsFalse_returnsSameAsGetVals(self):
        part = self._makePart()
        self.compareList(part.get("vb0", withInds=False), part.getVals("vb0", ordered=True))

    def test_get_strKey_ordered_passedThroughToBackingLookup(self):
        part = self._makePart()
        self.compareList(part.get("vb0", ordered=False, withInds=False), part.getVals("vb0", ordered=False))

    def test_get_strKey_withRanges_onlyInRangeValuesReturned(self):
        part = self._makePart()
        self.compareList(part.get("vb0", ranges=FRB.Ranges([(1, 2)])), ["ResourceX"])
        self.compareList(part.get("vb0", ranges=FRB.Ranges([(2, 3)])), ["ResourceY"])
        self.compareList(part.get("vb0", ranges=FRB.Ranges([(2, 3)]), withInds=True), [(2, "ResourceY")])

    def test_get_strKey_withRanges_equivalentToGetValsAndGetValsWithInds(self):
        part = self._makePart()
        r = FRB.Ranges([(1, 2)])
        self.compareList(part.get("vb0", ranges=r), part.getVals("vb0", ranges=r))
        self.compareList(part.get("vb0", ranges=r, withInds=True), part.getValsWithInds("vb0", ranges=r))

    def test_get_strKey_rangesFiltersOutEverything_returnsEmptyListNotDefault(self):
        part = self._makePart()
        # the key still exists -- an out-of-range filter isn't treated as "not found"
        self.compareList(part.get("vb0", ranges=FRB.Ranges([(100, 200)])), [])

    def test_get_intKey_ranges_acceptedButIgnored(self):
        part = self._makePart()
        self.assertEqual(part.get(0, ranges=FRB.Ranges([(100, 200)])), part.get(0))

    def test_get_strKey_notFound_defaultErrorOnNotFound_returnsDefault(self):
        part = self._makePart()
        self.assertIsNone(part.get("nonexistent"))
        self.assertEqual(part.get("nonexistent", default="fallback"), "fallback")

    def test_get_strKey_notFound_errorOnNotFoundTrue_raisesKeyError(self):
        part = self._makePart()
        with self.assertRaises(KeyError):
            part.get("nonexistent", errorOnNotFound=True)

    def test_get_unsupportedKeyType_raisesTypeError(self):
        part = self._makePart()
        with self.assertRaises(TypeError):
            part.get(3.14)

    def test_getByIndWithOccurrence_pairsValueWithOccurrenceIndex(self):
        part = self._makePart()
        self.assertEqual(part.getByIndWithOccurrence(2), (1, "ResourceY"))

    def test_setValByInd_variousIndices_valueUpdatedKeyUntouched(self):
        part = self._makePart()
        part.setValByInd(0, "newAbc")
        part.setValByInd(-1, "newY")
        self.compareList(part.entries(), [("hash", "newAbc"), ("vb0", "ResourceX"), ("vb0", "newY")])

    def test_setValByInd_outOfRange_raisesIndexError(self):
        part = self._makePart()
        with self.assertRaises(IndexError):
            part.setValByInd(100, "x")

    def test_entries_matchesInsertionOrder(self):
        part = self._makePart()
        self.compareList(part.entries(), self._items)

    def test_items_yieldsKeyValueOccurrenceOrderTuples(self):
        part = self._makePart()
        expected = [("hash", "abc123", 0, 0), ("vb0", "ResourceX", 0, 1), ("vb0", "ResourceY", 1, 2)]
        self.compareList(part.items(), expected)

    def test_iter_yieldsKeyValueOccurrenceOrderTuples(self):
        part = self._makePart()
        expected = [("hash", "abc123", 0, 0), ("vb0", "ResourceX", 0, 1), ("vb0", "ResourceY", 1, 2)]
        self.compareList(list(part), expected)

    # ================================================
    # ================= splitByInds =====================

    def test_splitByInds_splitsAtGivenIndexKeepingDepth(self):
        part = self._makePart(depth=4)
        parts = part.splitByInds([1])
        self.assertEqual(len(parts), 2)
        self.compareList(parts[0].entries(), [("hash", "abc123")])
        self.compareList(parts[1].entries(), [("vb0", "ResourceX"), ("vb0", "ResourceY")])
        self.assertEqual(parts[0].depth, 4)
        self.assertEqual(parts[1].depth, 4)

    def test_splitByInds_partsAreIndependent(self):
        part = self._makePart()
        parts = part.splitByInds([1])
        parts[0].addKVP("onlyInPart0", "x")
        self.compareList(part.entries(), self._items)
        self.compareList(parts[0].entries(), [("hash", "abc123"), ("onlyInPart0", "x")])

    # ================================================
    # =================== clone ========================

    def test_clone_isIndependentDeepCopy(self):
        part = self._makePart()
        clone = part.clone()

        self.compareList(clone.entries(), self._items)
        self.assertEqual(clone.depth, part.depth)

        clone.addKVP("onlyInClone", "x")
        self.compareList(part.entries(), self._items)

    def test_copyModule_copyAndDeepcopy_independentFromOriginal(self):
        part = self._makePart()

        shallowCopy = copyModule.copy(part)
        shallowCopy.addKVP("mutated1", "x")
        self.compareList(part.entries(), self._items)

        deepCopy = copyModule.deepcopy(part)
        deepCopy.addKVP("mutated2", "x")
        self.compareList(part.entries(), self._items)

    # ================================================
    # =================== toStr ========================

    def test_toStr_formatsAsKeyValueLines(self):
        part = FRB.CppIfContentPart(content=FRB.OrderedMultiMap([("hash", "abc"), ("vb0", "res")]).asInterface())
        self.assertEqual(part.toStr(), "hash = abc\nvb0 = res")

    def test_toStr_withLinePrefix_prefixesEveryLine(self):
        part = FRB.CppIfContentPart(content=FRB.OrderedMultiMap([("hash", "abc"), ("vb0", "res")]).asInterface())
        self.assertEqual(part.toStr(linePrefix="  "), "  hash = abc\n  vb0 = res")

    def test_toStr_emptyPart_emptyString(self):
        part = FRB.CppIfContentPart(content=FRB.OrderedMultiMap().asInterface())
        self.assertEqual(part.toStr(), "")
