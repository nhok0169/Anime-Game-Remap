import sys
from .baseOrderedMultiMapTest import BaseOrderedMultiMapTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class OrderedMultiMapTest(BaseOrderedMultiMapTest):
    _mapClass = FRB.OrderedMultiMap
