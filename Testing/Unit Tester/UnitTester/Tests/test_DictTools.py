import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


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
        resultDict = FRB.DictTools.combine(dict1, dict2, combineDuplicate = lambda value1, value2: (value1 + value2) / 2)
        self.compareDict(resultDict, {"a": 2, "b": 3, "c": 100, "d": 500})

    # TODO: Add tests for updating a dictionary

    # ================================================
    # ============ update ============================

    # TODO: Add tests for updating a dictionary


    # ================================================
    # ============ invert ============================

    # TODO: Add tests for inverting a dictionary

    # ================================================
