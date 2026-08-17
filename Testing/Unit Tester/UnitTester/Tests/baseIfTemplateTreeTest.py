import sys
from typing import List

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseIfTemplateTreeTest(BaseUnitTest):

    def _compareTree(self, node: FRB.IfTemplateNode, rawTreeNode, ifTemplateParts: List[FRB.IfTemplatePart]):
        self._compareIfTemplateTree(node, rawTreeNode, ifTemplateParts)
        
    def compareTree(self, root: FRB.IfTemplateNode, rawTreeRoot, ifTemplateParts: List[FRB.IfTemplatePart]):
        self.compareIfTemplateTree(root, rawTreeRoot, ifTemplateParts)