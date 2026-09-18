import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ListToolsTest(BaseUnitTest):

    # ============= filterInPlace =====================

    def test_filterInPlace_keepsOnlyMatchingElements_inRelativeOrder(self):
        lst = [1, 2, 3, 4, 5, 6, 7, 8]
        result = FRB.ListTools.filterInPlace(lst, lambda x: x % 2 == 0)
        self.compareList(lst, [2, 4, 6, 8])
        self.assertIs(result, lst)

    def test_filterInPlace_noneMatch_emptiesList(self):
        lst = [1, 3, 5, 7]
        FRB.ListTools.filterInPlace(lst, lambda x: x % 2 == 0)
        self.compareList(lst, [])

    def test_filterInPlace_allMatch_unchanged(self):
        lst = [2, 4, 6, 8]
        FRB.ListTools.filterInPlace(lst, lambda x: x % 2 == 0)
        self.compareList(lst, [2, 4, 6, 8])

    def test_filterInPlace_emptyList_staysEmpty(self):
        lst = []
        FRB.ListTools.filterInPlace(lst, lambda x: True)
        self.compareList(lst, [])

    def test_filterInPlace_mutatesSameObject_referencedElsewhereSeesChange(self):
        lst = [1, 2, 3, 4, 5]
        alias = lst
        FRB.ListTools.filterInPlace(lst, lambda x: x > 3)
        self.compareList(alias, [4, 5])

    def test_filterInPlace_matchesBuiltinFilter(self):
        tests = [
            ([1, 2, 3, 4, 5, 6], lambda x: x % 2 == 0),
            (["a", "bb", "ccc", "dddd"], lambda s: len(s) > 2),
            ([], lambda x: True),
            ([1, 1, 2, 2, 3], lambda x: x != 2),
        ]

        for data, predicate in tests:
            expected = list(filter(predicate, data))
            lst = list(data)
            FRB.ListTools.filterInPlace(lst, predicate)
            self.compareList(lst, expected)

    # ================================================
    # ============= updateMany ========================

    def test_updateMany_appendsAllListsInOrder(self):
        srcList = [1, 2]
        result = FRB.ListTools.updateMany(srcList, [[3, 4], [5], [6, 7]])
        self.assertIs(result, srcList)
        self.compareList(srcList, [1, 2, 3, 4, 5, 6, 7])

    def test_updateMany_emptyLstOfLists_unchanged(self):
        srcList = [1, 2]
        result = FRB.ListTools.updateMany(srcList, [])
        self.assertIs(result, srcList)
        self.compareList(srcList, [1, 2])

    def test_updateMany_emptySrcList_becomesConcatenation(self):
        srcList = []
        FRB.ListTools.updateMany(srcList, [[1, 2], [3]])
        self.compareList(srcList, [1, 2, 3])

    def test_updateMany_listsContainingEmptyLists_skippedOverCleanly(self):
        srcList = [1]
        FRB.ListTools.updateMany(srcList, [[], [2, 3], []])
        self.compareList(srcList, [1, 2, 3])

    def test_updateMany_singleSublist_sameAsExtend(self):
        srcList = [1]
        FRB.ListTools.updateMany(srcList, [[2, 3]])
        self.compareList(srcList, [1, 2, 3])

    def test_updateMany_mutatesSameObject_referencedElsewhereSeesChange(self):
        srcList = [1]
        alias = srcList
        FRB.ListTools.updateMany(srcList, [[2, 3]])
        self.compareList(alias, [1, 2, 3])

    # ================================================
