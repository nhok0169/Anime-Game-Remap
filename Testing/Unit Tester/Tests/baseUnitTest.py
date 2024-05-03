import unittest
from unittest import mock
from typing import Dict, Any, Hashable, List, TypeVar, Set

T = TypeVar("T")

class PatchService:
    def _cleanup(self, patch, target):
        patch.stop()
        self.patches.pop(target)

    def patch(self, target, *args, **kwargs):
        p = mock.patch(target, *args, **kwargs)
        patchedMock = p.start()
        self.addCleanup(self._cleanup, *[p, target])
        self.patches[target] = patchedMock

    def patchObj(self, target, *args, **kwargs):
        p = mock.patch.object(target, *args, **kwargs)
        patchedMock = p.start()
        self.addCleanup(self._cleanup, *[p, target])
        self.patches[target] = patchedMock


class BaseUnitTest(unittest.TestCase, PatchService):
    @classmethod
    def setUpClass(cls):
        cls.patches: Dict[str, mock.Mock] = {}

    def getDataFailMsg(self, result: Any, expected: Any, msg: str):
        return f"{msg}\n\nresult: {result}\n\nexpected: {expected}"

    def compareDict(self, resultDict: Dict[Hashable, Any], expectedDict: Dict[Hashable, Any]):
        resultDictLen = len(resultDict)
        expectedDictLen = len(expectedDict)
        if (resultDictLen != expectedDictLen):
            self.fail(self.getDataFailMsg(resultDict, expectedDict, f"Dictionaries have different lengths: resultDict: {resultDictLen}, expectedDict: {expectedDictLen}"))

        for resultKey in resultDict:
            resultValue = resultDict[resultKey]
            if (resultKey not in expectedDict):
                self.fail(self.getDataFailMsg(resultDict, expectedDict, f"The key, '{resultKey}', is not in the expected dictionary"))

            expectedValue = expectedDict[resultKey]
            if (resultValue != expectedValue):
                self.fail(self.getDataFailMsg(resultDict, expectedDict, f"Different values for the key, {resultKey}, in both dictionaries: resultDict: {resultValue}, expectedDict: {expectedValue}"))

    def compareList(self, resultList: List[T], expectedList: List[T]):
        resultListLen = len(resultList)
        expectedListLen = len(expectedList)
        if (resultListLen != expectedListLen):
            self.fail(self.getDataFailMsg(resultList, expectedList, f"Lists have different lengths: resultList: {resultListLen}, expectedList: {expectedListLen}"))

        for i in range(resultListLen):
            resultValue = resultList[i]
            expectedValue = expectedList[i]
            if (resultValue != expectedValue):
                self.fail(self.getDataFailMsg(resultList, expectedList, f"Different values in both lists: resultList: {resultValue}, expectedValue: {expectedValue}"))

    def compareSet(self, resultSet: Set[T], expectedSet: Set[T]):
        resultSetLen = len(resultSet)
        expectedSetLen = len(expectedSet)
        if (resultSetLen != expectedSetLen):
            self.fail(self.getDataFailMsg(resultSet, expectedSet, f"Sets have different lengths: resultSet: {resultSetLen}, expectedSet: {expectedSetLen}"))

        resultSetDiff = resultSet - expectedSet
        if (resultSetDiff):
            self.fail(self.getDataFailMsg(resultSet, expectedSet, f"resultSet contains elements not in expectedSet: {resultSetDiff}"))

        expectedSetDiff = expectedSet - resultSet
        if (expectedSetDiff):
            self.fail(self.getDataFailMsg(resultSet, expectedSet, f"expectedSet contains elements not in resultSet: {expectedSetDiff}"))
