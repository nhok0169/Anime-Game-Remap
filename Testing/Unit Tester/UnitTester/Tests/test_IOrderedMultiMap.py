import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class PyListOMM(FRB.IOrderedMultiMap):
    """
    A minimal, deliberately naive pure-Python implementation of FRB.IOrderedMultiMap (a plain
    list of [key, value] pairs, O(n) everywhere) -- exists purely to prove a Python subclass can
    satisfy the interface and be driven by C++ code through the trampoline, not to be a real
    backing structure.
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


class IOrderedMultiMapTest(BaseUnitTest):
    # ================================================
    # ========== OrderedMultiMap.asInterface ======

    def test_OrderedMultiMap_asInterface_returnsWorkingIndependentSnapshot(self):
        original = FRB.OrderedMultiMap([("a", 1), ("b", 2)])
        iface = original.asInterface()

        self.assertIsInstance(iface, FRB.IOrderedMultiMap)
        self.compareList(iface.entries(), [("a", 1), ("b", 2)])

        iface.insert("mutated", 999)
        self.compareList(original.entries(), [("a", 1), ("b", 2)])
        self.assertEqual(len(iface), 3)

    def test_OrderedMultiMapSqrt_asInterface_returnsWorkingIndependentSnapshot(self):
        original = FRB.OrderedMultiMapSqrt([("x", 1), ("y", 2)])
        iface = original.asInterface()

        self.assertIsInstance(iface, FRB.IOrderedMultiMap)
        self.compareList(iface.entries(), [("x", 1), ("y", 2)])

        iface.insert("mutated", 999)
        self.compareList(original.entries(), [("x", 1), ("y", 2)])

    # ================================================
    # =========== interface method surface ===========

    def test_asInterface_basicOperations_matchUnderlyingBehavior(self):
        iface = FRB.OrderedMultiMap([("a", 1), ("b", 2), ("a", 3)]).asInterface()

        self.assertEqual(iface.size(), 3)
        self.assertEqual(iface.count("a"), 2)
        self.assertTrue(iface.contains("a"))
        self.assertFalse(iface.contains("nonexistent"))
        self.compareList(iface.getAll("a"), [1, 3])
        self.assertEqual(iface.getByInd(0), ("a", 1))

        removed = iface.removeAt(0)
        self.assertTrue(removed)
        self.compareList(iface.entries(), [("b", 2), ("a", 3)])

    def test_asInterface_iteration_yieldsKeyValueOccurrenceOrderTuples(self):
        iface = FRB.OrderedMultiMap([("a", 1), ("b", 2), ("a", 3)]).asInterface()
        result = list(iface)
        expected = [("a", 1, 0, 0), ("b", 2, 0, 1), ("a", 3, 1, 2)]
        self.compareList(result, expected)

    def test_asInterface_rangesAsListOfTuples_filtersCorrectly(self):
        iface = FRB.OrderedMultiMap([("a", 1), ("b", 2), ("c", 3)]).asInterface()
        # ranges accepts a raw list of (start, end) tuples, not just a bound Ranges instance
        result = iface.removeAt(0, ranges=[(1, 3)])
        self.assertFalse(result)
        self.compareList(iface.entries(), [("a", 1), ("b", 2), ("c", 3)])

    def test_asInterface_rangesAsBoundRangesInstance_filtersCorrectly(self):
        iface = FRB.OrderedMultiMap([("a", 1), ("b", 2), ("c", 3)]).asInterface()
        result = iface.removeAt(0, ranges=FRB.Ranges([(0, 1)]))
        self.assertTrue(result)
        self.compareList(iface.entries(), [("b", 2), ("c", 3)])

    def test_asInterface_clone_isIndependent(self):
        iface = FRB.OrderedMultiMap([("a", 1)]).asInterface()
        clone = iface.clone()

        clone.insert("mutated", 999)
        self.compareList(iface.entries(), [("a", 1)])
        self.assertEqual(len(clone), 2)

    # ================================================
    # ========== appendAllToOrderedMultiMap ==========

    def test_appendAllToOrderedMultiMap_onNativeImplementation_appendsInOrder(self):
        iface = FRB.OrderedMultiMap([("a", 1)]).asInterface()
        FRB.appendAllToOrderedMultiMap(iface, [("b", 2), ("c", 3)])
        self.compareList(iface.entries(), [("a", 1), ("b", 2), ("c", 3)])

    def test_appendAllToOrderedMultiMap_onPurePythonImplementation_roundTripsThroughCpp(self):
        # The real proof: appendAllToOrderedMultiMap is implemented entirely in C++ and calls
        # .insert() on 'target' through the C++ virtual interface -- for this to mutate pyomm's
        # own _items list, the call must actually cross back into Python via the pybind11
        # trampoline (PyBindIOrderedMultiMap), not just call some C++-side stub.
        pyomm = PyListOMM()
        self.assertEqual(pyomm.size(), 0)

        FRB.appendAllToOrderedMultiMap(pyomm, [("p", 1), ("q", 2), ("r", 3)])

        self.compareList(pyomm.entries(), [("p", 1), ("q", 2), ("r", 3)])
        # also verify through the C++-bound accessor methods (not just Python's own _items),
        # to confirm the C++ side's view is consistent too
        self.assertEqual(len(pyomm), 3)
        self.assertEqual(pyomm.getByInd(1), ("q", 2))
        self.assertTrue(pyomm.contains("p"))
        self.assertTrue("p" in pyomm)

    def test_purePythonImplementation_cloneAndDirectMutation_workCorrectly(self):
        pyomm = PyListOMM()
        pyomm.insert("a", 1)
        pyomm.insert("b", 2)

        clone = pyomm.clone()
        clone.insert("onlyInClone", -1)

        self.compareList(pyomm.entries(), [("a", 1), ("b", 2)])
        self.compareList(clone.entries(), [("a", 1), ("b", 2), ("onlyInClone", -1)])

    def test_purePythonImplementation_incompleteSubclass_raisesWhenUnimplementedMethodCalled(self):
        # IOrderedMultiMap's methods are pure virtual. pybind11 doesn't stop Python from
        # instantiating a subclass that leaves some unimplemented (Python itself has no notion
        # of "abstract") -- but calling an unoverridden one through the C++ side raises, rather
        # than silently doing nothing.
        class IncompleteOMM(FRB.IOrderedMultiMap):
            def insert(self, key, value):
                pass

        instance = IncompleteOMM()  # instantiation itself succeeds
        instance.insert("a", 1)  # the one overridden method works fine

        error = None
        try:
            instance.size()  # never overridden
        except Exception as e:
            error = e

        self.assertIsNotNone(error)
