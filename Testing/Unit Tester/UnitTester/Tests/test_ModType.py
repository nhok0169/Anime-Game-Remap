import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModTypeTest(BaseUnitTest):
    """
    Tests for :class:`ModType` -- the heavier, C++-side data for a type of mod (gameTypeId,
    modTypeId, name, aliases), 'Cpp'-prefixed since it collides with the live pure-Python
    :class:`ModType` (see ``FixRaidenBoss2/model/strategies/ModType.py``)
    """

    # ============ __init__ ===========================

    def test_construct_fieldsSetAsGiven(self):
        modType = FRB.ModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber", ["BaronBunny", "ColleisBestie"])

        self.assertEqual(modType.gameTypeId, int(FRB.GameTypeId.GI))
        self.assertEqual(modType.modTypeId, int(FRB.ModTypeId.Amber))
        self.assertEqual(modType.name, "Amber")
        self.compareList(list(modType.aliases), ["BaronBunny", "ColleisBestie"])

    def test_construct_aliasesDefaultToEmptyList(self):
        modType = FRB.ModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Raiden), "Raiden")

        self.compareList(list(modType.aliases), [])

    # ================================================
    # === gameTypeId/modTypeId/name/aliases (readwrite) ===

    def test_fieldsAreReadWrite(self):
        modType = FRB.ModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber", ["BaronBunny"])

        modType.gameTypeId = int(FRB.GameTypeId.WuWa)
        modType.modTypeId = int(FRB.ModTypeId.Raiden)
        modType.name = "Raiden"
        modType.aliases = ["Ei"]

        self.assertEqual(modType.gameTypeId, int(FRB.GameTypeId.WuWa))
        self.assertEqual(modType.modTypeId, int(FRB.ModTypeId.Raiden))
        self.assertEqual(modType.name, "Raiden")
        self.compareList(list(modType.aliases), ["Ei"])

    # ================================================
    # =========== pure-Python ModType still separate ==

