import sys

from .baseIfTemplateTreeTest import BaseIfTemplateTreeTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IfTemplateNormTreeTest(BaseIfTemplateTreeTest):

    # ========= construct ====================================
    
    def test_validIfTemplates_TreeConstructed(self):
        tests = [
                [[FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)], 
                (None, [0], []),
                1],

                 [[FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                  FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If),
                  FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                  FRB.IfPredPart("else", FRB.IfPredPartType.Else),
                  FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                  FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], 
                 (None, [0, None, None], [(1, [2], []),
                                          (3, [4], [])]),
                 6],
                                    
                [[FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                  FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If),
                      FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                          FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                  FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                      FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If),
                      FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                  FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                      FRB.IfPredPart("if $x % 7 == 0", FRB.IfPredPartType.If),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                  FRB.IfPredPart("else if $x % 10 == 0", FRB.IfPredPartType.Elif),
                      FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                      FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                      FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                      FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                      FRB.IfPredPart("else", FRB.IfPredPartType.Else),
                      FRB.IfContentPart({"a": [(0, "3")], "b": [(1, "5")]}, 2),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                      FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                  FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)],
                  (None, [0, None, None, None, None], [(1, [None, None], [(2, [3], []),
                                                                    (4, [5], [])]),
                                                       (7, [None, None, 13, None, None], [(8, [9], []),
                                                                                          (10, [11], []),
                                                                                          (14, [], []),
                                                                                          (15, [16], [])]),
                                                       (17, [None, None, None, None, 28], [(18, [19], []),
                                                                                           (20, [21], []),
                                                                                           (23, [24], []),
                                                                                           (25, [26], [])]),
                                                       (29, [30], [])]),
                  33]]
        
        for test in tests:
            ifTemplateParts = test[0]
            expectedTree = test[1]
            expectedPartNum = test[2]
            resultTree = FRB.IfTemplateNormTree.construct(ifTemplateParts)

            self.compareTree(resultTree.root, expectedTree, ifTemplateParts)
            self.assertEqual(len(ifTemplateParts), expectedPartNum)

    # ========================================================