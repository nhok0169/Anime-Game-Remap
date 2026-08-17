import sys
import bisect
import random
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _compare(a, b):
    return a - b


class AlgoTest(BaseUnitTest):

    # =============== merge ==========================

    def test_differentSortedLsts_mergedLstAllSorted(self):
        tests = [[[], lambda a, b: a - b, []],
                 [[[], [], []], lambda a, b: a - b, []],
                 [[[2,4,6], [1,3,5]], lambda a, b: a - b, [1,2,3,4,5,6]],
                 [[[7, 2], [9,8,6,5,3,1], [], [11, 7,-1], [0]], lambda a, b: b - a, [11,9,8,7,7,6,5,3,2,1,0,-1]]]
        
        for test in tests:
            sortedLsts = test[0]
            compare = test[1]
            
            expected = test[2]
            result = FRB.Algo.merge(sortedLsts, compare)
            self.compareList(result, expected)

    # ================================================
    # ============ binarySearch ======================

    def test_emptyLst_notFoundAtIndex0(self):
        found, ind = FRB.Algo.binarySearch([], 5, _compare)
        self.assertFalse(found)
        self.assertEqual(ind, 0)

    def test_explicitCases_foundOrInsertionPointCorrect(self):
        tests = [
            # lst, target, expectedFound, expectedInd
            ([5], 5, True, 0),
            ([5], 3, False, 0),
            ([5], 7, False, 1),
            ([1, 3, 5, 7], 1, True, 0),
            ([1, 3, 5, 7], 7, True, 3),
            ([1, 3, 5, 7], 5, True, 2),
            ([1, 3, 5, 7], 0, False, 0),
            ([1, 3, 5, 7], 8, False, 4),
            ([1, 3, 5, 7], 4, False, 2),
            ([1, 3, 5, 7, 9, 11], 9, True, 4),
        ]

        for lst, target, expectedFound, expectedInd in tests:
            found, ind = FRB.Algo.binarySearch(lst, target, _compare)
            self.assertEqual(found, expectedFound, f"lst={lst}, target={target}")
            self.assertEqual(ind, expectedInd, f"lst={lst}, target={target}")

    def test_randomDistinctSortedLsts_matchesBisect(self):
        rand = random.Random(1234)

        for _ in range(200):
            n = rand.randint(0, 50)
            lst = sorted(rand.sample(range(0, 200), n))

            for target in range(-5, 205, 5):
                found, ind = FRB.Algo.binarySearch(lst, target, _compare)

                if target in lst:
                    self.assertTrue(found)
                    self.assertEqual(lst[ind], target)
                else:
                    self.assertFalse(found)
                    self.assertEqual(ind, bisect.bisect_left(lst, target))

    # ================================================
    # ============ binaryInsert ======================

    def test_optionalInsertFalse_alwaysInserts(self):
        lst = [1, 3, 5, 7]
        inserted = FRB.Algo.binaryInsert(lst, 5, _compare, optionalInsert = False)
        self.assertTrue(inserted)
        self.compareList(lst, [1, 3, 5, 5, 7])

    def test_optionalInsertTrue_notFound_inserts(self):
        lst = [1, 3, 5, 7]
        inserted = FRB.Algo.binaryInsert(lst, 4, _compare, optionalInsert = True)
        self.assertTrue(inserted)
        self.compareList(lst, [1, 3, 4, 5, 7])

    def test_optionalInsertTrue_found_doesNotInsert(self):
        lst = [1, 3, 5, 7]
        inserted = FRB.Algo.binaryInsert(lst, 5, _compare, optionalInsert = True)
        self.assertFalse(inserted)
        self.compareList(lst, [1, 3, 5, 7])

    def test_insertIntoEmptyLst(self):
        lst = []
        inserted = FRB.Algo.binaryInsert(lst, 5, _compare)
        self.assertTrue(inserted)
        self.compareList(lst, [5])

    def test_insertAtFrontAndBack(self):
        lst = [3, 5, 7]
        FRB.Algo.binaryInsert(lst, 1, _compare)
        self.compareList(lst, [1, 3, 5, 7])

        FRB.Algo.binaryInsert(lst, 9, _compare)
        self.compareList(lst, [1, 3, 5, 7, 9])

    # ================================================