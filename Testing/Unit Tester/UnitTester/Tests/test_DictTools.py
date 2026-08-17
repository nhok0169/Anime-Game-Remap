import sys
from collections import defaultdict
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class DictToolsTest(BaseUnitTest):

    # ============ getFirstKey =======================

    def test_nonEmptyDict_getFirstKey(self):
        testDict = {1: "a", "a": 1, "b": 2}
        result = FRB.DictTools.getFirstKey(testDict)
        self.assertEqual(result, 1)

    def test_emptyDict_stopIterationWithoutKey(self):
        testDict = {}
        exception = None

        try:
            FRB.DictTools.getFirstKey(testDict)
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, StopIteration)

    # ================================================
    # ============ getFirstValue =====================

    def test_nonEmptyDict_getFirstValue(self):
        testDict = {1: "a", "a": 1, "b": 2}
        result = FRB.DictTools.getFirstValue(testDict)
        self.assertEqual(result, "a")

    def test_emptyDict_stopIterationWithoutValue(self):
        testDict = {}
        exception = None

        try:
            FRB.DictTools.getFirstValue(testDict)
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, StopIteration)

    # ================================================
    # ============ combine ===========================
        
    def test_dictsWithUniqueKeys_combinedDict(self):
        dict1 = {"a": 1, "b": 2}
        dict2 = {"c": 3, "d": 4, "e": 5}
        resultDict = FRB.DictTools.combine(dict1, dict2)
        self.compareDict(resultDict, {"a": 1, "b": 2, "c": 3, "d": 4, "e": 5})

    def test_emptyDstDict_onlySrcDict(self):
        dict1 = {}
        dict2 = {"c": 3, "d": 4, "e": 5}
        resultDict = FRB.DictTools.combine(dict1, dict2)
        self.compareDict(resultDict, {"c": 3, "d": 4, "e": 5})

    def test_emptySrcDict_onlyDstDict(self):
        dict1 = {"a": 1, "b": 2}
        dict2 = {}
        resultDict = FRB.DictTools.combine(dict1, dict2)
        self.compareDict(resultDict, {"a": 1, "b": 2})

    def test_emptyDicts_combinedEmptyDicts(self):
        dict1 = {}
        dict2 = {}
        resultDict = FRB.DictTools.combine(dict1, dict2)
        self.compareDict(resultDict, {})

    def test_dictsWithSameKeysDefaultCombineFunc_combinedDictWithValuesFromSrc(self):
        dict1 = {"a": 1, "b": 2, "c": 100}
        dict2 = {"a": 3, "b": 4, "d": 500}
        resultDict = FRB.DictTools.combine(dict1, dict2)
        self.compareDict(resultDict, {"a": 3, "b": 4, "c": 100, "d": 500})

    def test_dictsWithSameKeysAverageFunc_combinedDictWithAveragedValues(self):
        dict1 = {"a": 1, "b": 2, "c": 100}
        dict2 = {"a": 3, "b": 4, "d": 500}
        resultDict = FRB.DictTools.combine(dict1, dict2, combineDuplicate = lambda key, value1, value2: (value1 + value2) / 2)
        self.compareDict(resultDict, {"a": 2, "b": 3, "c": 100, "d": 500})

    def test_combineDuplicate_argOrderIsDict1ThenDict2(self):
        dict1 = {"a": 1, "b": 2, "c": 100}
        dict2 = {"a": 3, "b": 4, "d": 500}

        keepFromDict1 = FRB.DictTools.combine(dict1, dict2, combineDuplicate = lambda key, dict1Val, dict2Val: dict1Val)
        self.compareDict(keepFromDict1, {"a": 1, "b": 2, "c": 100, "d": 500})

        keepFromDict2 = FRB.DictTools.combine(dict1, dict2, combineDuplicate = lambda key, dict1Val, dict2Val: dict2Val)
        self.compareDict(keepFromDict2, {"a": 3, "b": 4, "c": 100, "d": 500})

    def test_combineDuplicate_keyPassedIsSharedKey(self):
        dict1 = {"a": 1, "b": 2}
        dict2 = {"a": 3, "c": 4}
        seenKeys = []

        def combineDuplicate(key, dict1Val, dict2Val):
            seenKeys.append(key)
            return dict1Val

        FRB.DictTools.combine(dict1, dict2, combineDuplicate = combineDuplicate)
        self.compareList(seenKeys, ["a"])

    def test_makeNewCopyDefaultTrue_dict1Unmutated_resultIsNewObject(self):
        dict1 = {"a": 1, "b": 2}
        dict2 = {"b": 20, "c": 3}

        result = FRB.DictTools.combine(dict1, dict2)
        self.assertIsNot(result, dict1)
        self.compareDict(dict1, {"a": 1, "b": 2})
        self.compareDict(result, {"a": 1, "b": 20, "c": 3})

    def test_makeNewCopyFalse_dict1MutatedInPlace_resultIsDict1(self):
        dict1 = {"a": 1, "b": 2}
        dict2 = {"b": 20, "c": 3}

        result = FRB.DictTools.combine(dict1, dict2, makeNewCopy = False)
        self.assertIs(result, dict1)
        self.compareDict(dict1, {"a": 1, "b": 20, "c": 3})

    def test_makeNewCopyFalse_withCombineDuplicate_dict1MutatedCorrectly(self):
        dict1 = {"a": 1, "b": 2}
        dict2 = {"b": 20, "c": 3}

        result = FRB.DictTools.combine(dict1, dict2, combineDuplicate = lambda key, dict1Val, dict2Val: dict1Val + dict2Val, makeNewCopy = False)
        self.assertIs(result, dict1)
        self.compareDict(dict1, {"a": 1, "b": 22, "c": 3})

    def test_dict1IsDefaultDict_makeNewCopyTrue_plainDictResult(self):
        dict1 = defaultdict(int, {"a": 1, "b": 2})
        dict2 = {"b": 20, "c": 3}

        result = FRB.DictTools.combine(dict1, dict2)
        self.compareDict(result, {"a": 1, "b": 20, "c": 3})

    def test_dict1IsDefaultDict_makeNewCopyFalse_resultPreservesDefaultDict(self):
        dict1 = defaultdict(int, {"a": 1, "b": 2})
        dict2 = {"b": 20, "c": 3}

        result = FRB.DictTools.combine(dict1, dict2, makeNewCopy = False)
        self.assertIs(result, dict1)
        self.assertIsInstance(result, defaultdict)
        self.compareDict(dict1, {"a": 1, "b": 20, "c": 3})

    def test_dict2IsDefaultDict_combinedCorrectlyNoAutoVivify(self):
        dict1 = {"a": 1, "b": 2}
        dict2 = defaultdict(int, {"b": 20, "c": 3})

        result = FRB.DictTools.combine(dict1, dict2, combineDuplicate = lambda key, dict1Val, dict2Val: dict1Val + dict2Val)
        self.compareDict(result, {"a": 1, "b": 22, "c": 3})
        self.compareSet(set(dict2.keys()), {"b", "c"})

    def test_dict1NotADict_typeError(self):
        exception = None

        try:
            FRB.DictTools.combine("not a dict", {"a": 1})
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    def test_dict2NotADict_typeError(self):
        exception = None

        try:
            FRB.DictTools.combine({"a": 1}, "not a dict")
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    # ================================================
    # ============ combineMany =======================

    def test_noCombineDuplicate_lastDictWins(self):
        dict1 = {"a": 1, "b": 2}
        dictList = [{"b": 20, "c": 3}, {"c": 30, "d": 4}]
        result = FRB.DictTools.combineMany(dict1, dictList)
        self.compareDict(result, {"a": 1, "b": 20, "c": 30, "d": 4})
        self.compareDict(dict1, {"a": 1, "b": 2})

    def test_combineDuplicate_indexSchemeDict1IsZero(self):
        dict1 = {"a": 1}
        dictList = [{"a": 2}, {"a": 3}]
        calls = []

        def combineDuplicate(key, indexToValue):
            calls.append((key, dict(indexToValue)))
            return sum(indexToValue.values())

        result = FRB.DictTools.combineMany(dict1, dictList, combineDuplicate = combineDuplicate)
        self.compareDict(result, {"a": 6})
        self.compareList(calls, [("a", {0: 1, 1: 2, 2: 3})])

    def test_mixedOnlyInDict1AndSharedKeys_correctHandling(self):
        dict1 = {"onlyInDict1": 99, "shared": 1}
        dictList = [{"shared": 2}]
        calls = []

        def combineDuplicate(key, indexToValue):
            calls.append(key)
            return sum(indexToValue.values())

        result = FRB.DictTools.combineMany(dict1, dictList, combineDuplicate = combineDuplicate)
        self.compareDict(result, {"onlyInDict1": 99, "shared": 3})
        self.compareList(calls, ["shared"])

    def test_keyOnlyInOneListEntry_simpleCopyNoCombineDuplicateCall(self):
        dict1 = {}
        dictList = [{"onlyHere": 5}, {}]
        calls = []

        def combineDuplicate(key, indexToValue):
            calls.append(key)
            return None

        result = FRB.DictTools.combineMany(dict1, dictList, combineDuplicate = combineDuplicate)
        self.compareDict(result, {"onlyHere": 5})
        self.compareList(calls, [])

    def test_emptyDictList_noChange(self):
        dict1 = {"a": 1}
        result = FRB.DictTools.combineMany(dict1, [])
        self.compareDict(result, {"a": 1})

    def test_makeNewCopyDefaultTrue_dict1Unmutated(self):
        dict1 = {"a": 1}
        dictList = [{"a": 2}]
        result = FRB.DictTools.combineMany(dict1, dictList, combineDuplicate = lambda key, itv: sum(itv.values()))
        self.assertIsNot(result, dict1)
        self.compareDict(dict1, {"a": 1})
        self.compareDict(result, {"a": 3})

    def test_makeNewCopyFalse_dict1MutatedInPlace(self):
        dict1 = {"a": 1}
        dictList = [{"a": 2}]
        result = FRB.DictTools.combineMany(dict1, dictList, combineDuplicate = lambda key, itv: sum(itv.values()), makeNewCopy = False)
        self.assertIs(result, dict1)
        self.compareDict(dict1, {"a": 3})

    def test_dict1IsDefaultDict_makeNewCopyFalse_typePreserved(self):
        dict1 = defaultdict(int, {"a": 1})
        dictList = [{"a": 2}, {"a": 3}]
        result = FRB.DictTools.combineMany(dict1, dictList, combineDuplicate = lambda key, itv: sum(itv.values()), makeNewCopy = False)
        self.assertIs(result, dict1)
        self.assertIsInstance(result, defaultdict)
        self.compareDict(dict(dict1), {"a": 6})

    def test_dict1NotADict_typeError(self):
        exception = None

        try:
            FRB.DictTools.combineMany("not a dict", [{"a": 1}])
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    def test_dictListEntryNotADict_typeError(self):
        exception = None

        try:
            FRB.DictTools.combineMany({"a": 1}, [{"b": 2}, "not a dict"])
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    # ================================================
    # ============ update ============================

    def test_noCombineDuplicate_srcOverwrittenByNew(self):
        srcDict = {"a": 1, "b": 2}
        newDict = {"b": 20, "c": 3}
        result = FRB.DictTools.update(srcDict, newDict)
        self.assertIs(result, srcDict)
        self.compareDict(srcDict, {"a": 1, "b": 20, "c": 3})

    def test_combineDuplicate_srcShorterOrEqual_averagedSharedKeys(self):
        srcDict = {"a": 1, "b": 2, "c": 100}
        newDict = {"a": 3, "b": 4, "d": 500}
        result = FRB.DictTools.update(srcDict, newDict, combineDuplicate = lambda key, srcVal, newVal: (srcVal + newVal) / 2)
        self.assertIs(result, srcDict)
        self.compareDict(srcDict, {"a": 2, "b": 3, "c": 100, "d": 500})

    def test_combineDuplicate_newShorter_averagedSharedKeys(self):
        srcDict = {"a": 1, "b": 2, "c": 100, "e": 7, "f": 8}
        newDict = {"a": 3, "b": 4}
        result = FRB.DictTools.update(srcDict, newDict, combineDuplicate = lambda key, srcVal, newVal: (srcVal + newVal) / 2)
        self.assertIs(result, srcDict)
        self.compareDict(srcDict, {"a": 2, "b": 3, "c": 100, "e": 7, "f": 8})

    def test_combineDuplicate_argOrderIsSrcThenNew(self):
        srcDict = {"a": 1, "b": 2}
        newDict = {"a": 3, "c": 4}
        seenKeys = []

        def combineDuplicate(key, srcVal, newVal):
            seenKeys.append((key, srcVal, newVal))
            return srcVal

        FRB.DictTools.update(srcDict, newDict, combineDuplicate = combineDuplicate)
        self.compareList(seenKeys, [("a", 1, 3)])

    def test_srcIsDefaultDict_typePreserved(self):
        srcDict = defaultdict(int, {"a": 1})
        newDict = {"a": 2, "b": 3}
        result = FRB.DictTools.update(srcDict, newDict, combineDuplicate = lambda key, srcVal, newVal: srcVal + newVal)
        self.assertIs(result, srcDict)
        self.assertIsInstance(result, defaultdict)
        self.compareDict(dict(srcDict), {"a": 3, "b": 3})

    def test_srcDictNotADict_typeError(self):
        exception = None

        try:
            FRB.DictTools.update("not a dict", {"a": 1})
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    def test_newDictNotADict_typeError(self):
        exception = None

        try:
            FRB.DictTools.update({"a": 1}, "not a dict")
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    # ================================================
    # ============ updateMany ========================

    def test_noCombineDuplicate_lastDictWinsInPlace(self):
        srcDict = {"a": 1, "b": 2}
        dictList = [{"b": 20, "c": 3}, {"c": 30, "d": 4}]
        result = FRB.DictTools.updateMany(srcDict, dictList)
        self.assertIs(result, srcDict)
        self.compareDict(srcDict, {"a": 1, "b": 20, "c": 30, "d": 4})

    def test_combineDuplicate_indexSchemeSrcDictIsZero(self):
        srcDict = {"a": 1}
        dictList = [{"a": 2}, {"a": 3}]
        calls = []

        def combineDuplicate(key, indexToValue):
            calls.append((key, dict(indexToValue)))
            return sum(indexToValue.values())

        result = FRB.DictTools.updateMany(srcDict, dictList, combineDuplicate = combineDuplicate)
        self.assertIs(result, srcDict)
        self.compareDict(srcDict, {"a": 6})
        self.compareList(calls, [("a", {0: 1, 1: 2, 2: 3})])

    def test_mixedOnlyInSrcDictAndSharedKeys_correctHandling(self):
        srcDict = {"onlyInSrc": 99, "shared": 1}
        dictList = [{"shared": 2}]
        calls = []

        def combineDuplicate(key, indexToValue):
            calls.append(key)
            return sum(indexToValue.values())

        FRB.DictTools.updateMany(srcDict, dictList, combineDuplicate = combineDuplicate)
        self.compareDict(srcDict, {"onlyInSrc": 99, "shared": 3})
        self.compareList(calls, ["shared"])

    def test_keyOnlyInOneListEntry_simpleCopyNoCombineDuplicateCall(self):
        srcDict = {}
        dictList = [{"onlyHere": 5}, {}]
        calls = []

        def combineDuplicate(key, indexToValue):
            calls.append(key)
            return None

        FRB.DictTools.updateMany(srcDict, dictList, combineDuplicate = combineDuplicate)
        self.compareDict(srcDict, {"onlyHere": 5})
        self.compareList(calls, [])

    def test_emptyDictList_noChange(self):
        srcDict = {"a": 1}
        result = FRB.DictTools.updateMany(srcDict, [])
        self.assertIs(result, srcDict)
        self.compareDict(srcDict, {"a": 1})

    def test_srcIsDefaultDict_typePreserved(self):
        srcDict = defaultdict(int, {"a": 1})
        dictList = [{"a": 2}, {"a": 3}]
        result = FRB.DictTools.updateMany(srcDict, dictList, combineDuplicate = lambda key, itv: sum(itv.values()))
        self.assertIs(result, srcDict)
        self.assertIsInstance(result, defaultdict)
        self.compareDict(dict(srcDict), {"a": 6})

    def test_srcDictNotADict_typeError(self):
        exception = None

        try:
            FRB.DictTools.updateMany("not a dict", [{"a": 1}])
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    def test_dictListEntryNotADict_typeError(self):
        exception = None

        try:
            FRB.DictTools.updateMany({"a": 1}, [{"b": 2}, "not a dict"])
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    # ================================================
    # ============ invert ============================

    # TODO: Add tests for inverting a dictionary

    # ================================================
    # ============ filter ============================

    # TODO: Add tests for filtering a dictionary

    # ================================================
    # ============= forDict ==========================

    # TODO: Add tests for iterating over a nested dictionary

    # ================================================
    # ============= getVal ===========================

    def test_fullKeys_leafValue(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ["a", "b", "c"])
        self.assertEqual(result, 1)

    def test_partialKeys_subDict(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ["a", "b"])
        self.compareDict(result, {"c": 1})

    def test_noKeys_wholeDict(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, [])
        self.compareDict(result, testDict)

    def test_fullKeysAsTuple_leafValue(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ("a", "b", "c"))
        self.assertEqual(result, 1)

    def test_partialKeysAsTuple_subDict(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ("a", "b"))
        self.compareDict(result, {"c": 1})

    def test_noKeysAsTuple_wholeDict(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ())
        self.compareDict(result, testDict)

    def test_missingKeyAsTuple_defaultVal(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ("x",), default = "someDefault")
        self.assertEqual(result, "someDefault")

    def test_missingKeyDefaultErrorOnNotFound_defaultVal(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ["x"])
        self.assertIsNone(result)

    def test_missingKeyCustomDefault_customDefaultVal(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ["x"], default = "someDefault")
        self.assertEqual(result, "someDefault")

    def test_tooManyKeysPastLeaf_defaultVal(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.getVal(testDict, ["a", "b", "c", "d"], default = "someDefault")
        self.assertEqual(result, "someDefault")

    def test_missingKeyErrorOnNotFound_keyError(self):
        testDict = {"a": {"b": {"c": 1}}}
        exception = None

        try:
            FRB.DictTools.getVal(testDict, ["x"], errorOnNotFound = True)
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, KeyError)

    def test_emptyDict_defaultVal(self):
        testDict = {}
        result = FRB.DictTools.getVal(testDict, ["a"], default = "someDefault")
        self.assertEqual(result, "someDefault")

    def test_topLevelDefaultDict_realPathReturnsValue(self):
        testDict = defaultdict(dict)
        testDict["a"] = {"b": 1}
        result = FRB.DictTools.getVal(testDict, ["a", "b"])
        self.assertEqual(result, 1)

    def test_topLevelDefaultDict_missingKeyDefaultValNoAutoVivify(self):
        testDict = defaultdict(dict)
        testDict["a"] = {"b": 1}
        result = FRB.DictTools.getVal(testDict, ["a", "x"], default = "someDefault")
        self.assertEqual(result, "someDefault")
        self.compareDict(testDict["a"], {"b": 1})

    def test_nestedDefaultDictValue_missingKeyDefaultValNoAutoVivify(self):
        testDict = {"a": defaultdict(dict, {"b": 1})}
        result = FRB.DictTools.getVal(testDict, ["a", "x"], default = "someDefault")
        self.assertEqual(result, "someDefault")
        self.compareDict(testDict["a"], {"b": 1})

    def test_nonDictTopLevel_defaultVal(self):
        result = FRB.DictTools.getVal("not a dict", ["a"], default = "someDefault")
        self.assertEqual(result, "someDefault")

    def test_nonDictTopLevelErrorOnNotFound_keyError(self):
        exception = None

        try:
            FRB.DictTools.getVal("not a dict", ["a"], errorOnNotFound = True)
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, KeyError)

    # ================================================
    # ============= contains =========================

    def test_fullKeysExist_true(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ["a", "b", "c"])
        self.assertTrue(result)

    def test_partialKeysExist_true(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ["a", "b"])
        self.assertTrue(result)

    def test_noKeys_true(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, [])
        self.assertTrue(result)

    def test_fullKeysAsTupleExist_true(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ("a", "b", "c"))
        self.assertTrue(result)

    def test_partialKeysAsTupleExist_true(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ("a", "b"))
        self.assertTrue(result)

    def test_noKeysAsTuple_true(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ())
        self.assertTrue(result)

    def test_missingKeyAsTuple_false(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ("x",))
        self.assertFalse(result)

    def test_missingKey_false(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ["x"])
        self.assertFalse(result)

    def test_keysPastLeaf_false(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ["a", "b", "c", "d"])
        self.assertFalse(result)

    def test_emptyDict_false(self):
        testDict = {}
        result = FRB.DictTools.contains(testDict, ["a"])
        self.assertFalse(result)

    def test_partialPathMismatchLaterKey_false(self):
        testDict = {"a": {"b": {"c": 1}}}
        result = FRB.DictTools.contains(testDict, ["a", "x"])
        self.assertFalse(result)

    def test_topLevelDefaultDict_realPathTrue(self):
        testDict = defaultdict(dict)
        testDict["a"] = {"b": 1}
        result = FRB.DictTools.contains(testDict, ["a", "b"])
        self.assertTrue(result)

    def test_topLevelDefaultDict_missingPathFalseNoAutoVivify(self):
        testDict = defaultdict(dict)
        testDict["a"] = {"b": 1}
        result = FRB.DictTools.contains(testDict, ["a", "x"])
        self.assertFalse(result)
        self.compareDict(testDict["a"], {"b": 1})

    def test_nestedDefaultDictValue_realPathTrue(self):
        testDict = {"a": defaultdict(dict, {"b": {"c": 1}})}
        result = FRB.DictTools.contains(testDict, ["a", "b", "c"])
        self.assertTrue(result)

    def test_nestedDefaultDictValue_missingPathFalseNoAutoVivify(self):
        testDict = {"a": defaultdict(dict, {"b": {"c": 1}})}
        result = FRB.DictTools.contains(testDict, ["a", "x"])
        self.assertFalse(result)
        self.compareDict(testDict["a"], {"b": {"c": 1}})

    def test_nonDictTopLevel_false(self):
        result = FRB.DictTools.contains("not a dict", ["a"])
        self.assertFalse(result)

    # ================================================
    # ============= setVal ===========================

    def test_missingIntermediateKeys_autoCreated(self):
        testDict = {}
        FRB.DictTools.setVal(testDict, ["a", "b"], 1)
        self.compareDict(testDict, {"a": {"b": 1}})

    def test_blockingNonDictValue_overwritten(self):
        testDict = {"a": 5}
        FRB.DictTools.setVal(testDict, ["a", "b"], 1)
        self.compareDict(testDict, {"a": {"b": 1}})

    def test_existingPath_updatesInPlace(self):
        inner = {"b": 2}
        testDict = {"a": inner}
        FRB.DictTools.setVal(testDict, ["a", "b"], 99)
        self.compareDict(testDict, {"a": {"b": 99}})
        self.assertIs(testDict["a"], inner)

    def test_emptyKeys_noOp(self):
        testDict = {"x": 1}
        FRB.DictTools.setVal(testDict, [], 999)
        self.compareDict(testDict, {"x": 1})

    def test_singleKey_setsTopLevel(self):
        testDict = {}
        FRB.DictTools.setVal(testDict, ["a"], 42)
        self.compareDict(testDict, {"a": 42})

    def test_keysAsTuple_deepSet(self):
        testDict = {}
        FRB.DictTools.setVal(testDict, ("a", "b", "c"), "deep")
        self.compareDict(testDict, {"a": {"b": {"c": "deep"}}})

    def test_topLevelDefaultDict_setsCorrectly(self):
        testDict = defaultdict(dict)
        FRB.DictTools.setVal(testDict, ["a", "b"], 1)
        self.compareDict(dict(testDict), {"a": {"b": 1}})
        self.assertIsInstance(testDict, defaultdict)

    def test_nonDictTopLevel_typeError(self):
        exception = None

        try:
            FRB.DictTools.setVal("not a dict", ["a"], 1)
        except BaseException as e:
            exception = e

        self.assertIsInstance(exception, TypeError)

    def test_returnValue_none(self):
        result = FRB.DictTools.setVal({}, ["a"], 1)
        self.assertIsNone(result)

    # ================================================
    # ============= getCommonKeys ====================

    def test_dictsWithFullKeyOverlap_allKeys(self):
        dictList = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.assertIsInstance(result, list)
        self.compareSet(set(result), {"a", "b"})

    def test_dictsWithPartialKeyOverlap_sharedKeysOnly(self):
        dictList = [{"a": 1, "b": 2}, {"b": 3, "c": 4}, {"b": 5, "d": 6}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareSet(set(result), {"b"})

    def test_dictsWithNoKeyOverlap_empty(self):
        dictList = [{"a": 1}, {"b": 2}, {"c": 3}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareList(result, [])

    def test_emptyDictList_empty(self):
        result = FRB.DictTools.getCommonKeys([])
        self.compareList(result, [])

    def test_singleDict_allItsKeys(self):
        dictList = [{"a": 1, "b": 2}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareSet(set(result), {"a", "b"})

    def test_listContainingEmptyDict_empty(self):
        dictList = [{"a": 1, "b": 2}, {}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareList(result, [])

    def test_orderedDefaultTrue_followsFirstDictInsertionOrder(self):
        dictA = {}
        dictA["z"] = 1
        dictA["a"] = 2
        dictA["m"] = 3
        dictB = {"z": 10, "a": 20, "m": 30, "extra": 40}

        result = FRB.DictTools.getCommonKeys([dictA, dictB])
        self.compareList(result, ["z", "a", "m"])

    def test_orderedExplicitTrue_sameAsDefault(self):
        dictA = {}
        dictA["z"] = 1
        dictA["a"] = 2
        dictB = {"z": 10, "a": 20}

        result = FRB.DictTools.getCommonKeys([dictA, dictB], ordered = True)
        self.compareList(result, ["z", "a"])

    def test_orderedFalse_stillCorrectSetJustUnspecifiedOrder(self):
        dictA = {"z": 1, "a": 2, "m": 3}
        dictB = {"z": 10, "a": 20, "m": 30}

        result = FRB.DictTools.getCommonKeys([dictA, dictB], ordered = False)
        self.compareSet(set(result), {"z", "a", "m"})

    def test_nonDictEntriesIgnored_onlyRealDictsCompared(self):
        dictList = [{"a": 1}, "not a dict", {"a": 1}, None]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareList(result, ["a"])

    # ================================================
    # ============= getCommonPaths ===================

    def comparePaths(self, resultPaths, expectedPaths):
        self.compareSet(set(map(tuple, resultPaths)), set(map(tuple, expectedPaths)))

    def test_singleCommonPath_deepestPathOnly(self):
        dictA = {"1": {"2": {"3": "x"}}}
        dictB = {"1": {"2": {"3": "y"}}}
        result = FRB.DictTools.getCommonPaths([dictA, dictB])
        self.comparePaths(result, [["1", "2", "3"]])

    def test_branchingCommonPaths_bothMaximalBranches(self):
        dictA = {"1": {"2": {"3": "x"}, "4": "y"}}
        dictB = {"1": {"2": {"3": "z"}, "4": "w"}}
        result = FRB.DictTools.getCommonPaths([dictA, dictB])
        self.comparePaths(result, [["1", "2", "3"], ["1", "4"]])

    def test_divergingDepth_stopsAtShallowerDict(self):
        dictA = {"1": {"2": "leaf"}}
        dictB = {"1": {"2": {"3": "y"}}}
        result = FRB.DictTools.getCommonPaths([dictA, dictB])
        self.comparePaths(result, [["1", "2"]])

    def test_noOverlap_emptyList(self):
        result = FRB.DictTools.getCommonPaths([{"a": 1}, {"b": 2}])
        self.comparePaths(result, [])

    def test_emptyDictList_emptyList(self):
        result = FRB.DictTools.getCommonPaths([])
        self.comparePaths(result, [])

    def test_singleDict_allItsMaximalPaths(self):
        dictList = [{"a": {"b": 1}, "c": 2}]
        result = FRB.DictTools.getCommonPaths(dictList)
        self.comparePaths(result, [["a", "b"], ["c"]])

    def test_orderedDefaultTrue_followsFirstDictInsertionOrder(self):
        dictA = {}
        dictA["z"] = {"1": "x"}
        dictA["a"] = {"1": "y"}
        dictA["m"] = {"1": "w"}
        dictB = {"z": {"1": "v"}, "a": {"1": "u"}, "m": {"1": "t"}}

        result = FRB.DictTools.getCommonPaths([dictA, dictB])
        self.compareList(result, [["z", "1"], ["a", "1"], ["m", "1"]])

    def test_orderedExplicitTrue_sameAsDefault(self):
        dictA = {}
        dictA["z"] = {"1": "x"}
        dictA["a"] = {"1": "y"}
        dictB = {"z": {"1": "v"}, "a": {"1": "u"}}

        result = FRB.DictTools.getCommonPaths([dictA, dictB], ordered = True)
        self.compareList(result, [["z", "1"], ["a", "1"]])

    def test_orderedFalse_stillCorrectSetJustUnspecifiedOrder(self):
        dictA = {"z": {"1": "x"}, "a": {"1": "y"}, "m": {"1": "w"}}
        dictB = {"z": {"1": "v"}, "a": {"1": "u"}, "m": {"1": "t"}}

        result = FRB.DictTools.getCommonPaths([dictA, dictB], ordered = False)
        self.comparePaths(result, [["z", "1"], ["a", "1"], ["m", "1"]])

    def test_orderedDeepBranching_nestedOrderPreserved(self):
        dictC = {}
        dictC["1"] = {}
        dictC["1"]["z"] = "v1"
        dictC["1"]["a"] = "v2"
        dictD = {"1": {"z": "w1", "a": "w2"}}

        result = FRB.DictTools.getCommonPaths([dictC, dictD])
        self.compareList(result, [["1", "z"], ["1", "a"]])

    def test_nonDictEntriesIgnored_onlyRealDictsCompared(self):
        dictList = [{"a": 1}, {"a": 1}, "not a dict", None]
        result = FRB.DictTools.getCommonPaths(dictList)
        self.comparePaths(result, [["a"]])

    def test_listContainingEmptyDict_emptyList(self):
        dictList = [{"a": 1}, {}]
        result = FRB.DictTools.getCommonPaths(dictList)
        self.comparePaths(result, [])

    def test_topLevelDefaultDictEntry_treatedAsRegularDict(self):
        dictA = defaultdict(dict, {"1": {"2": {"3": "x"}}})
        dictB = {"1": {"2": {"3": "y"}}}
        result = FRB.DictTools.getCommonPaths([dictA, dictB])
        self.comparePaths(result, [["1", "2", "3"]])

    def test_nestedDefaultDictTree_noAutoVivify(self):
        def factory():
            return defaultdict(factory)

        ddTree = defaultdict(factory)
        ddTree["x"]["y"] = 1
        ddTree["x"]["z"] = 2

        plainCounterpart = {"x": {"y": 10, "w": 20}}

        result = FRB.DictTools.getCommonPaths([ddTree, plainCounterpart])
        self.comparePaths(result, [["x", "y"]])
        self.compareSet(set(ddTree["x"].keys()), {"y", "z"})

    # ================================================
    # ============= iterPaths ========================

    def test_basicNestedDict_leafPaths(self):
        testDict = {"a": {"b": 1, "c": 2}, "d": 3}
        result = list(FRB.DictTools.iterPaths(testDict))
        self.comparePaths(result, [["a", "b"], ["a", "c"], ["d"]])

    def test_nestedEmptyDict_treatedAsLeaf(self):
        testDict = {"a": {}, "b": 1}
        result = list(FRB.DictTools.iterPaths(testDict))
        self.comparePaths(result, [["a"], ["b"]])

    def test_rootEmptyDict_nothingYielded(self):
        result = list(FRB.DictTools.iterPaths({}))
        self.compareList(result, [])

    def test_rootNonDict_nothingYielded(self):
        result = list(FRB.DictTools.iterPaths(5))
        self.compareList(result, [])

    def test_deeplyNestedBranching_allLeafPaths(self):
        testDict = {"1": {"2": {"3": "x"}, "4": "y"}}
        result = list(FRB.DictTools.iterPaths(testDict))
        self.comparePaths(result, [["1", "2", "3"], ["1", "4"]])

    def test_topLevelDefaultDict_leafPaths(self):
        testDict = defaultdict(dict)
        testDict["a"]["b"] = 1
        testDict["a"]["c"] = 2
        result = list(FRB.DictTools.iterPaths(testDict))
        self.comparePaths(result, [["a", "b"], ["a", "c"]])

    def test_insertionOrderPreserved(self):
        testDict = {}
        testDict["z"] = 1
        testDict["a"] = 2
        testDict["m"] = 3
        result = list(FRB.DictTools.iterPaths(testDict))
        self.compareList(result, [["z"], ["a"], ["m"]])

    def test_isGenerator_lazilyIterable(self):
        testDict = {"a": 1}
        result = FRB.DictTools.iterPaths(testDict)
        self.assertTrue(hasattr(result, "__next__"))
        self.compareList(list(result), [["a"]])

    # ================================================
    # ============= getPaths =========================

    def test_returnsAList_notAGenerator(self):
        testDict = {"a": 1}
        result = FRB.DictTools.getPaths(testDict)
        self.assertIsInstance(result, list)

    def test_basicNestedDict_leafPaths(self):
        testDict = {"a": {"b": 1, "c": 2}, "d": 3}
        result = FRB.DictTools.getPaths(testDict)
        self.comparePaths(result, [["a", "b"], ["a", "c"], ["d"]])

    def test_nestedEmptyDict_treatedAsLeaf(self):
        testDict = {"a": {}, "b": 1}
        result = FRB.DictTools.getPaths(testDict)
        self.comparePaths(result, [["a"], ["b"]])

    def test_rootEmptyDict_empty(self):
        result = FRB.DictTools.getPaths({})
        self.compareList(result, [])

    def test_rootNonDict_empty(self):
        result = FRB.DictTools.getPaths(5)
        self.compareList(result, [])

    def test_deeplyNestedBranching_allLeafPaths(self):
        testDict = {"1": {"2": {"3": "x"}, "4": "y"}}
        result = FRB.DictTools.getPaths(testDict)
        self.comparePaths(result, [["1", "2", "3"], ["1", "4"]])

    def test_topLevelDefaultDict_leafPaths(self):
        testDict = defaultdict(dict)
        testDict["a"]["b"] = 1
        testDict["a"]["c"] = 2
        result = FRB.DictTools.getPaths(testDict)
        self.comparePaths(result, [["a", "b"], ["a", "c"]])

    def test_orderedDefaultTrue_insertionOrderPreserved(self):
        testDict = {}
        testDict["z"] = 1
        testDict["a"] = 2
        testDict["m"] = 3
        result = FRB.DictTools.getPaths(testDict)
        self.compareList(result, [["z"], ["a"], ["m"]])

    def test_orderedExplicitTrue_matchesIterPaths(self):
        testDict = {}
        testDict["z"] = {"1": "x"}
        testDict["a"] = {"2": "y", "3": "w"}
        testDict["m"] = "leaf"

        result = FRB.DictTools.getPaths(testDict, ordered = True)
        expected = list(FRB.DictTools.iterPaths(testDict))
        self.compareList(result, expected)

    def test_orderedFalse_stillCorrectSetJustUnspecifiedOrder(self):
        testDict = {"z": {"1": "x"}, "a": {"2": "y", "3": "w"}, "m": "leaf"}
        result = FRB.DictTools.getPaths(testDict, ordered = False)
        self.comparePaths(result, [["z", "1"], ["a", "2"], ["a", "3"], ["m"]])

    # ================================================

