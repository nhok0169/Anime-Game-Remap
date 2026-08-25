import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModTypeIdDataTest(BaseUnitTest):
    """
    Tests for :class:`ModTypeIdData` -- the cheap (gameTypeId, modTypeId) data an ini classifier
    holds, as opposed to the heavier :class:`CppModType` (see ``test_CppModType.py``)
    """

    # ============ __init__ ===========================

    def test_construct_fieldsSetAsGiven(self):
        data = FRB.ModTypeIdData(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber))

        self.assertEqual(data.gameTypeId, int(FRB.GameTypeId.GI))
        self.assertEqual(data.modTypeId, int(FRB.ModTypeId.Amber))

    def test_construct_noValidationAgainstDeclaredEnums(self):
        # a custom mod type using an id not registered in ModTypeId/GameTypeId is still representable
        data = FRB.ModTypeIdData(999, 888)

        self.assertEqual(data.gameTypeId, 999)
        self.assertEqual(data.modTypeId, 888)

    # ================================================
    # ========= gameTypeId/modTypeId (readwrite) ======

    def test_fieldsAreReadWrite(self):
        data = FRB.ModTypeIdData(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber))

        data.gameTypeId = int(FRB.GameTypeId.WuWa)
        data.modTypeId = int(FRB.ModTypeId.Raiden)

        self.assertEqual(data.gameTypeId, int(FRB.GameTypeId.WuWa))
        self.assertEqual(data.modTypeId, int(FRB.ModTypeId.Raiden))

    # ================================================
