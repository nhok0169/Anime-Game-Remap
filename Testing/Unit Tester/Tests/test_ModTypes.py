import sys
from .baseUnitTest import BaseUnitTest

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


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