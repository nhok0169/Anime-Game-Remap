import sys
from typing import Dict, Any, Union, Callable, Optional, List

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IfTemplateTreeTest(BaseUnitTest):

    def _compareTree(self, node: FRB.IfTemplateNode, rawTreeNode, ifTemplateParts: List[FRB.IfTemplatePart]):
        rawParts = rawTreeNode[0]
        nodeParts = node.parts

        rawPartsLen = len(rawParts)
        nodePartsLen = len(nodeParts)

        self.assertEqual(nodePartsLen, rawPartsLen)

        for i in range(nodePartsLen):
            rawPart = rawParts[i]
            nodePart = nodeParts[i]

            if (rawPart is None):
                self.assertIsInstance(nodePart, FRB.IfTemplateNode)
            else:
                self.assertIsInstance(nodePart, FRB.IfContentPart)

        rawChildren = rawTreeNode[1]
        nodeChildren = node.children

        rawChildrenLen = len(rawChildren)
        nodeChildrenLen = len(nodeChildren)
        self.assertEqual(nodeChildrenLen, rawChildrenLen)

        nodeChildrenKeys = list(nodeChildren.keys())

        for i in range(nodeChildrenLen):
            rawChild = rawChildren[i]

            nodeChildKey = nodeChildrenKeys[i]
            nodeChild = nodeChildren[nodeChildKey]
            self._compareTree(nodeChild, rawChild, ifTemplateParts)
        
    def compareTree(self, root: FRB.IfTemplateNode, rawTreeRoot, ifTemplateParts: List[FRB.IfTemplatePart]):
        self._compareTree(root,  rawTreeRoot, ifTemplateParts)


    # ========= construct ====================================
    
    def test_validIfTemplates_TreeConstructed(self):
        tests = [
                [[FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)], 
                ([0], [])],

                 [[FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                  FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If),
                  FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                  FRB.IfPredPart("else", FRB.IfPredPartType.Else),
                  FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                  FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], 
                 ([0, None, None], [([2], []),
                                    ([4], [])])],
                                    
                [[FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                  FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If),
                      FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                          FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                  FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Else),
                      FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If),
                      FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                  FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                      FRB.IfPredPart("if $x % 7 == 0", FRB.IfPredPartType.If),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                  FRB.IfPredPart("else if $x % 10 == 0", FRB.IfPredPartType.Else),
                      FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                      FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                      FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                      FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                      FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                      FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                  FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)],
                  ([0, None, None, None], [([None], [([3], [])]),
                                           ([None, 9, None], [([7], []),
                                                              ([], [])]),
                                           ([None, None, 19], [([14], []),
                                                               ([17], [])])])]]
        
        for test in tests:
            ifTemplateParts = test[0]
            expectedTree = test[1]
            resultTree = FRB.IfTemplateTree.construct(ifTemplateParts)

            self.compareTree(resultTree.root, expectedTree, ifTemplateParts)


    # ========================================================