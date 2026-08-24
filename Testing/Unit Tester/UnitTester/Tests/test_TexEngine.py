import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class TexEngineTest(BaseUnitTest):
    """
    Tests for the :class:`TexEngine` enum
    """

    def test_values(self):
        self.assertEqual(FRB.TexEngine.Compressonator.value, "compressonator")
        self.assertEqual(FRB.TexEngine.Pillow.value, "pillow")

    def test_membersAreDistinct(self):
        self.assertNotEqual(FRB.TexEngine.Compressonator, FRB.TexEngine.Pillow)
