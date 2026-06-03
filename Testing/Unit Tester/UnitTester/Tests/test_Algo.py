import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


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

    # TODO: Add tests for binary search

    # ================================================
    # ============ binaryInsert ======================

    # TODO: add tests for binary insert

    # ================================================