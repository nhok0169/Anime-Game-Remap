import sys
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

    # TODO: Add tests for updating a dictionary

    # ================================================
    # ============ update ============================

    # TODO: Add tests for updating a dictionary


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

    # ================================================
    # ============= getCommonKeys ====================

    def test_dictsWithFullKeyOverlap_allKeys(self):
        dictList = [{"a": 1, "b": 2}, {"a": 3, "b": 4}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareSet(result, {"a", "b"})

    def test_dictsWithPartialKeyOverlap_sharedKeysOnly(self):
        dictList = [{"a": 1, "b": 2}, {"b": 3, "c": 4}, {"b": 5, "d": 6}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareSet(result, {"b"})

    def test_dictsWithNoKeyOverlap_emptySet(self):
        dictList = [{"a": 1}, {"b": 2}, {"c": 3}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareSet(result, set())

    def test_emptyDictList_emptySet(self):
        result = FRB.DictTools.getCommonKeys([])
        self.compareSet(result, set())

    def test_singleDict_allItsKeys(self):
        dictList = [{"a": 1, "b": 2}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareSet(result, {"a", "b"})

    def test_listContainingEmptyDict_emptySet(self):
        dictList = [{"a": 1, "b": 2}, {}]
        result = FRB.DictTools.getCommonKeys(dictList)
        self.compareSet(result, set())

    # ================================================

