import sys
import unittest.mock as mock
from typing import List, Dict, Tuple

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IfContentPartTest(BaseUnitTest):

    def compareSrc(self, result: Dict[str, List[Tuple[int, str]]], expected: Dict[str, List[Tuple[int, str]]]):
        lstComp = lambda resLst, expectedLst: self.compareList(resLst, expectedLst, compareValues = lambda resData, expectedData: self.compareList(resData, expectedData))
        self.compareDict(result, expected, compareValues = lstComp)

    def compareOrder(self, result: List[Tuple[str, int]], expected: List[Tuple[str, int]]):
        self.compareList(result, expected, compareValues = lambda resData, expData: self.compareList(resData, expData))

    def compareIfContentPart(self, result: FRB.IfContentPart, expected: FRB.IfContentPart):
        self.assertEqual(result.depth, expected.depth)
        self.compareSrc(result.src, expected.src)
        self.compareOrder(result._order, expected._order)

    # ============ __init__ ==========================

    def test_differentKVPs_ifContentPartCreated(self):
        depth = 1
        tests = [
            [{}, []],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, [("a", 0), ("b", 0), ("a", 1)]]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)
            expectedOrder = test[1]

            self.compareSrc(ifContentPart.src, src)
            self.compareOrder(ifContentPart._order, expectedOrder)

    # ================================================
    # ============ __iter__ ==========================

    def test_differentKVPs_ifContentIterated(self):
        depth = 1
        tests = [
            [{}, []],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, 
             [("a", "aVal", 0, 0), ("b", "bVal", 0, 1), ("a", "a2Val", 1, 2)]]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)

            result = [(key, val, keyInd, orderInd) for key, val, keyInd, orderInd in ifContentPart]
            expected = test[1]

            self.compareOrder(result, expected)
    # ================================================
    # ============ __contains__ ======================

    def test_differentKeys_keyInPart(self):
        depth = 1
        tests = [
            [{}, "a", False],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "a", True],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "c", True]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)

            result = test[1] in ifContentPart
            expected = test[2]
            self.assertEqual(result, expected)

    # ================================================
    # ============ __getitem__ =======================

    def test_differentKeys_getKeyParts(self):
        depth = 1
        tests = [
            [{}, "a", None],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "a", [(0, "aVal"), (2, "a2Val")]],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "c", []],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "x", None]
        ]

        for test in tests:
            src = test[0]
            key = test[1]
            ifContentPart = FRB.IfContentPart(src, depth)

            expected = test[2]
            result = None

            try:
                result = ifContentPart[key]
            except:
                pass

            if (result is None):
                self.assertIsNone(expected)
            else:
                self.compareSrc({key: result}, {key: expected})

    def test_differentInds_getKeyParts(self):
        depth = 1
        tests = [
            [{}, 0, None],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, 2, ("a", "a2Val", 1)],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, -2, ("b", "bVal", 0)]
        ]

        for test in tests:
            src = test[0]
            ind = test[1]
            ifContentPart = FRB.IfContentPart(src, depth)

            expected = test[2]
            result = None

            try:
                result = ifContentPart[ind]
            except:
                pass

            if (result is None):
                self.assertIsNone(expected)
            else:
                self.compareList(result, expected)

    # ================================================
    # ============ get ===============================

    def test_differentKeys_getKeyPartsByGetItem(self):
        depth = 1
        default = "Not Found!"
        tests = [
            [{}, "a", False, default],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "a", True, [(0, "aVal"), (2, "a2Val")]],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "c", True, []],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "x", False, default]
        ]

        for test in tests:
            src = test[0]
            key = test[1]
            ifContentPart = FRB.IfContentPart(src, depth)

            expectedSuccess = test[2]
            expected = test[3]
            result = ifContentPart.get(key, default = default)
            

            if (not expectedSuccess):
                self.assertEqual(result, default)
            else:
                self.compareSrc({key: result}, {key: expected})

    def test_differentInds_getKeyPartsByGetItem(self):
        depth = 1
        default = "Not Found!"
        tests = [
            [{}, 0, False, default],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, 2, True, ("a", "a2Val", 1)],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, -2, True, ("b", "bVal", 0)]
        ]

        for test in tests:
            src = test[0]
            ind = test[1]
            ifContentPart = FRB.IfContentPart(src, depth)

            expectedSuccess = test[2]
            expected = test[3]
            result = ifContentPart.get(ind, default = default)

            if (not expectedSuccess):
                self.assertEqual(result, default)
            else:
                self.compareList(result, expected)

    # ================================================
    # ============ src.setter ========================

    @mock.patch("src.FixRaidenBoss2.IfContentPart._setupOrder")
    def test_newSrc_orderSetup(self, m_setupOrder):
        ifContentPart = FRB.IfContentPart({}, 1)

        tests = [
            [{}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}]
        ]

        callCount = 2
        for test in tests:
            ifContentPart.src = test[0]
            self.assertEqual(m_setupOrder.call_count, callCount)
            callCount += 1

    # ================================================
    # ============ toStr =============================

    def test_differentKVPs_kvpsToStr(self):
        depth = 1
        linePrefix = "PREF --> "

        tests = [
            [{}, ""],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, 
             (f"{linePrefix}a = aVal\n"
              f"{linePrefix}b = bVal\n"
              f"{linePrefix}a = a2Val")]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)
            
            expectedStr = test[1]
            resultStr = ifContentPart.toStr(linePrefix = linePrefix)

            self.assertEqual(resultStr, expectedStr)

    # ================================================
    # ============ getVals ===========================

    def test_differentKeys_valsRetrieved(self):
        depth = 1
        tests = [
            [{}, "a", []],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "a", ["aVal", "a2Val"]],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "b", ["bVal"]],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "x", []]
        ]

        for test in tests:
            src = test[0]
            key = test[1]
            ifContentPart = FRB.IfContentPart(src, depth)

            expected = test[2]
            result = ifContentPart.getVals(key)

            self.compareList(result, expected)

    # ================================================
    # ============ removeKey =========================

    def test_noChecks_keyRemoved(self):
        depth = 1
        tests = [
            [{}, "a", {}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "a", {"b": [(0, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "b", {"a": [(0, "aVal"), (1, "a2Val")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "c", {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "x", {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}]
        ]

        for test in tests:
            result = FRB.IfContentPart(test[0], depth)
            expected = FRB.IfContentPart(test[2], depth)

            result.removeKey(test[1])
            self.compareIfContentPart(result, expected)

    def test_Checks_keyRemoved(self):
        depth = 1
        tests = [
            [{}, ("a", lambda valData: True), {}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, ("a", lambda valData: valData[1] == "aVal"), {"a": [(1, "a2Val")], "b": [(0, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, ("a", lambda valData: len(valData[1]) > 0), {"b": [(0, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, ("a", lambda valData: False), {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, ("c", lambda valData: True), {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, ("c", lambda valData: False), {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "x", {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}]
        ]

        for test in tests:
            result = FRB.IfContentPart(test[0], depth)
            expected = FRB.IfContentPart(test[2], depth)

            result.removeKey(test[1])
            self.compareIfContentPart(result, expected)

    # ================================================
    # ============ removeKeys ========================

    def test_differentKeysAndChecks_keysRemoved(self):
        depth = 1
        tests = [
            [{}, {("a", lambda valData: True)}, {}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, set(), 
             {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {("a", lambda valData: valData[1] == "aVal")}, 
             {"a": [(1, "a2Val")], "b": [(0, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {("a", lambda valData: valData[1] == "aVal"), ("a", lambda valData: valData[1] == "a2Val")}, 
             {"b": [(0, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {("a", lambda valData: valData[1] == "aVal"), "a"}, 
             {"b": [(0, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {("a", lambda valData: valData[1] == "aVal"), "b"}, 
             {"a": [(0, "a2Val")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {("a", lambda valData: False)}, 
             {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {("c", lambda valData: True), ("a", lambda valData: False)}, {
                "a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {("c", lambda valData: False)}, 
             {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"x", "y", "z"}, 
             {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}]
        ]

        for test in tests:
            result = FRB.IfContentPart(test[0], depth)
            expected = FRB.IfContentPart(test[2], depth)

            result.removeKeys(test[1])
            self.compareIfContentPart(result, expected)

    # ================================================
    # ============ addKVPToFront =====================

    def test_differentKeysToAdd_keysAddedToFront(self):
        depth = 1
        tests = [
            [{}, "a", "a3Val", {"a": [(0, "a3Val")]}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "a", "a3Val", {"a": [(0, "a3Val"), (1, "aVal"), (3, "a2Val")], "b": [(2, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "c", "cVal", {"a": [(1, "aVal"), (3, "a2Val")], "b": [(2, "bVal")], "c": [(0, "cVal")], "d": []}]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)

            ifContentPart.addKVPToFront(test[1], test[2])
            expected = FRB.IfContentPart(test[3], depth)

            self.compareIfContentPart(ifContentPart, expected)

    # ================================================
    # ============ addKVP ============================

    def test_differentKeysToAddNotFront_keysAdded(self):
        depth = 1
        tests = [
            [{}, "a", "a3Val", {"a": [(0, "a3Val")]}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "a", "a3Val", {"a": [(0, "aVal"), (2, "a2Val"), (3, "a3Val")], "b": [(1, "bVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "c", "cVal", {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [(3, "cVal")], "d": []}]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)

            ifContentPart.addKVP(test[1], test[2])
            expected = FRB.IfContentPart(test[3], depth)

            self.compareIfContentPart(ifContentPart, expected)

    @mock.patch("src.FixRaidenBoss2.IfContentPart.addKVPToFront")
    def test_differentKeysToAddToFront_keysAddedToFront(self, m_addKVPToFront):
        depth = 1
        tests = [
            [{}, "a", "a3Val"],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "a", "a3Val"],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, "c", "cVal"]
        ]

        callCount = 1
        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)

            ifContentPart.addKVP(test[1], test[2], toFront= True)

            self.assertEqual(m_addKVPToFront.call_count, callCount)
            callCount += 1

    # ================================================
    # ============ replaceVals =======================

    def test_differentValsToReplace_valsReplaced(self):
        depth = 1
        tests = [
            [{}, {"a": "newAVal"}, {}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": "newAVal", "b": ["newBVal"]}, 
             {"a": [(0, "newAVal"), (2, "newAVal")], "b": [(1, "newBVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": ("newAVal", lambda val: val == "a2Val"), "b": ["newBVal", "newB2Val", "newB3Val"]},
             {"a": [(0, "aVal"), (2, "newAVal")], "b": [(1, "newBVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": []},
             {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)

            ifContentPart.replaceVals(test[1], addNewKVPs = False)
            expected = FRB.IfContentPart(test[2], depth)

            self.compareIfContentPart(ifContentPart, expected)

    def test_differentValsToReplace_valsReplacedOrAdded(self):
        depth = 1
        tests = [
            [{}, {"a": "newAVal"}, {"a": [(0, "newAVal")]}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": "newAVal", "b": ["newBVal"]}, 
             {"a": [(0, "newAVal"), (2, "newAVal")], "b": [(1, "newBVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": ("newAVal", lambda val: val == "a2Val"), "b": ["newBVal", "newB2Val", "newB3Val"]},
             {"a": [(0, "aVal"), (2, "newAVal")], "b": [(1, "newBVal")], "c": [], "d": []}],
            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": [], "c": ("cVal", lambda val: False), "x": ("xVal", lambda val: False)},
             {"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": [], "x": [(3, "xVal")]}]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)

            ifContentPart.replaceVals(test[1], addNewKVPs = True)
            expected = FRB.IfContentPart(test[2], depth)

            self.compareIfContentPart(ifContentPart, expected)

    # ================================================
    # ============ remapKeys =========================

    def test_differentRemaps_keysRemapped(self):
        depth = 1
        tests = [
            [{}, {"a": ["b"]}, {}],

            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": ["b"], "b": ["a"]}, 
             {"b": [(0, "aVal"), (2, "a2Val")], "a": [(1, "bVal")], "c": [], "d": []}],

            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": ["a1", "a2"], "b": []},
             {"a1": [(0, "aVal"), (2, "a2Val")], "a2": [(1, "aVal"), (3, "a2Val")], "c": [], "d": []}],

            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": FRB.KeyRemapData.build(["a1", "a2"], keepKeyWithoutRemap = True), "b": FRB.KeyRemapData.build([], keepKeyWithoutRemap = True)},
             {"a1": [(0, "aVal"), (3, "a2Val")], "a2": [(1, "aVal"), (4, "a2Val")], "b": [(2, "bVal")], "c": [], "d": []}],

            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": FRB.KeyRemapData.build([("a1", lambda key, val: False)], keepKeyWithoutRemap = True), "b": FRB.KeyRemapData.build([], keepKeyWithoutRemap = False)},
             {"a": [(0, "aVal"), (1, "a2Val")], "c": [], "d": []}],

            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": [FRB.RemappedKeyData("a1", toInd = -1), FRB.RemappedKeyData("a2", toInd = 1)], "b": [FRB.RemappedKeyData("b1", toInd = 0)]},
             {"a2": [(1, "aVal"), (2, "a2Val")], "b1": [(0, "bVal")], "a1": [(3, "aVal"), (4, "a2Val")], "c": [], "d": []}],

            [{"a": [(0, "aVal"), (2, "a2Val")], "b": [(1, "bVal")], "c": [], "d": []}, {"a": [FRB.RemappedKeyData("a1", toInd = -1), FRB.RemappedKeyData("a2", toInd = 1)]},
             {"a2": [(1, "aVal"), (2, "a2Val")], "b": [(0, "bVal")], "a1": [(3, "aVal"), (4, "a2Val")], "c": [], "d": []}],

            [{"temp": [(0, "a")], "run": [(1, "b"), (2, "c")]}, {"temp": [FRB.RemappedKeyData("run", toInd = -1)]},
             {"run": [(2, "a"), (0, "b"), (1, "c")]}]
        ]

        for test in tests:
            src = test[0]
            ifContentPart = FRB.IfContentPart(src, depth)

            ifContentPart.remapKeys(test[1])
            expected = FRB.IfContentPart(test[2], depth)

            self.compareIfContentPart(ifContentPart, expected)
    # ================================================