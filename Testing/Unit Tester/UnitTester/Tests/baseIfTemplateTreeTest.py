import sys
from typing import List

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class BaseIfTemplateTreeTest(BaseUnitTest):

    def _compareTree(self, node: FRB.IfTemplateNode, rawTreeNode, ifTemplateParts: List[FRB.IfTemplatePart]):
        rawNodeIfPredInd = rawTreeNode[0]
        nodeIfPredPart = node.ifPredPart

        if (rawNodeIfPredInd is None):
            self.assertIsNone(nodeIfPredPart)
        else:
            self.assertIsInstance(nodeIfPredPart, FRB.IfPredPart)

        rawParts = rawTreeNode[1]
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

        rawChildren = rawTreeNode[2]
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