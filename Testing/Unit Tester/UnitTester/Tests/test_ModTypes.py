import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class ModTypesTest(BaseUnitTest):
    # ====================== search ======================================
    def test_differentSearchKeyWords_modTypeFoundOrNot(self):
        testSearches = {"": None,
                        "Raiden": FRB.ModTypes.Raiden.value,
                        "rAiDeN": FRB.ModTypes.Raiden.value,
                        "SmolEi": FRB.ModTypes.Raiden.value,
                        "ei": FRB.ModTypes.Raiden.value,
                        "makoto": None}
        
        for keyword in testSearches:
            result = FRB.ModTypes.search(keyword)
            expected = testSearches[keyword]

            self.assertEqual(type(result), type(expected))
            if (expected is not None):
                self.assertEqual(result.name, expected.name)

    # ====================================================================