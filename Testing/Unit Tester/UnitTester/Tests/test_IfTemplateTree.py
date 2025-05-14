import sys

from .baseIfTemplateTreeTest import BaseIfTemplateTreeTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IfTemplateTreeTest(BaseIfTemplateTreeTest):

    # ========= construct ====================================
    
    def test_validIfTemplates_TreeConstructed(self):
        tests = [
                [[FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)], 
                (None, [0], [])],

                 [[FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                  FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If),
                  FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                  FRB.IfPredPart("else", FRB.IfPredPartType.Else),
                  FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                  FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], 
                 (None, [0, None, None], [(1, [2], []),
                                          (3, [4], [])])],
                                    
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
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                      FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                  FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)],
                  (None, [0, None, None, None], [(1, [None], [(2, [3], [])]),
                                                 (5, [None, 9, None], [(6, [7], []),
                                                                       (10, [], [])]),
                                                 (12, [None, None, 19], [(13, [14], []),
                                                                         (16, [17], [])])])]]
        
        for test in tests:
            ifTemplateParts = test[0]
            expectedTree = test[1]
            resultTree = FRB.IfTemplateTree.construct(ifTemplateParts)

            self.compareTree(resultTree.root, expectedTree, ifTemplateParts)

    # ========================================================