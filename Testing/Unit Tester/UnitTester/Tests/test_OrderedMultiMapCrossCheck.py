import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class OrderedMultiMapCrossCheckTest(BaseUnitTest):
    """
    OrderedMultiMap (plain linked list) and OrderedMultiMapSqrt (sqrt-decomposed blocks)
    are meant to be behaviorally interchangeable -- only their underlying complexity differs.
    This runs the same, fairly long sequence of mutating operations against both and asserts
    identical results at every step, catching binding-layer bugs (a wrong return_value_policy,
    a type caster behaving differently for one class than the other) a single-class test suite
    could miss. Uses enough elements to actually exercise OrderedMultiMapSqrt's block
    splitting/merging machinery, not just its 1-2 element edge cases.
    """

    def _assertSameState(self, mapA, mapB, msg):
        self.assertEqual(len(mapA), len(mapB), msg)
        self.compareList(mapA.entries(), mapB.entries())

    def test_longOperationSequence_bothBackingStructuresProduceIdenticalResults(self):
        items = [(f"k{i % 7}", i) for i in range(40)]

        mapA = FRB.OrderedMultiMap(items)
        mapB = FRB.OrderedMultiMapSqrt(items)
        self._assertSameState(mapA, mapB, "after construction")

        for m in (mapA, mapB):
            m.insert("tail", -1)
            m.insertStart("head", -2)
            m.insertAt(5, "mid", -3)
        self._assertSameState(mapA, mapB, "after insert/insertStart/insertAt")

        for m in (mapA, mapB):
            m.insertAllEnd([("end1", 100), ("end2", 101)])
            m.insertAllStart([("start1", 200), ("start2", 201)])
        self._assertSameState(mapA, mapB, "after insertAllEnd/insertAllStart")

        for m in (mapA, mapB):
            m.insertAllAt({0: ("ins0", 300), 10: ("ins10", 301), 20: ("ins20", 302)})
        self._assertSameState(mapA, mapB, "after insertAllAt")

        n = len(mapA)
        for m in (mapA, mapB):
            m.reorder({0: n - 1, n - 1: 0, 5: -100, 10: 100})
        self._assertSameState(mapA, mapB, "after reorder")

        for m in (mapA, mapB):
            m.removeAt(3)
            m.removeKey("k3", check=lambda ind, value: value is not None and value % 2 == 0)
        self._assertSameState(mapA, mapB, "after removeAt/removeKey")

        for m in (mapA, mapB):
            rule = FRB.RemappedKeyData("k1_remapped", check=lambda key, value: isinstance(value, int) and value > 15, toInd=0)
            m.remapKeys({"k1": FRB.KeyRemapData([rule], keepKeyWithoutRemap=True)})
        self._assertSameState(mapA, mapB, "after remapKeys")

        for m in (mapA, mapB):
            m.replaceVals({
                "k2": FRB.ReplaceList([-10, -11, -12]),
                "k4": FRB.ReplaceIf(-99, lambda oldVal: isinstance(oldVal, int) and oldVal > 20),
                "brandNewKey": -1000,
            })
        self._assertSameState(mapA, mapB, "after replaceVals")

        partsA = mapA.splitByInds([len(mapA) // 3, 2 * len(mapA) // 3])
        partsB = mapB.splitByInds([len(mapB) // 3, 2 * len(mapB) // 3])
        self.assertEqual(len(partsA), len(partsB), "splitByInds part count")
        for partA, partB in zip(partsA, partsB):
            self._assertSameState(partA, partB, "after splitByInds (per part)")

        iterResultA = list(mapA)
        iterResultB = list(mapB)
        self.compareList(iterResultA, iterResultB)

        copyA = mapA.copy()
        copyB = mapB.copy()
        copyA.insert("onlyInA", 1)
        copyB.insert("onlyInB", 1)
        self._assertSameState(mapA, mapB, "originals unaffected by copies' mutations")
