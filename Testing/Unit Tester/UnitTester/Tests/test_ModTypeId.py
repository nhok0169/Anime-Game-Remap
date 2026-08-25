import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModTypeIdTest(BaseUnitTest):
    """
    Tests for :class:`ModTypeIdTools` -- bare-named (no 'Cpp' prefix), no pure-Python class of
    this exact name exists to shadow it
    """

    def setUp(self):
        super().setUp()
        # ModTypeIdTools' registry (getModType/registerModType/findByName) is process-global
        # static state, shared by every test in the whole suite -- mirrors HashTools.clear()/
        # CppHashTools.clear()'s own reason for existing; without this, tests here (and any other
        # test file that happens to register a ModType) would leak state into each other.
        FRB.ModTypeIdTools.clear()

    # ================== getEnum =======================

    def test_declaredValue_getEnumReturnsCorrectEnum(self):
        self.assertEqual(FRB.ModTypeIdTools.getEnum(int(FRB.ModTypeId.Amber)), FRB.ModTypeId.Amber)
        self.assertEqual(FRB.ModTypeIdTools.getEnum(int(FRB.ModTypeId.XingqiuBamboo)), FRB.ModTypeId.XingqiuBamboo)

    def test_undeclaredValue_getEnumReturnsNone(self):
        self.assertIsNone(FRB.ModTypeIdTools.getEnum(-999))

    # ================================================
    # =================== getName ======================

    def test_declaredValue_getNameReturnsCorrectName(self):
        self.assertEqual(FRB.ModTypeIdTools.getName(FRB.ModTypeId.Amber), "Amber")
        # the one real naming discrepancy: enum member is lowercase 'b', name is capital 'B'
        self.assertEqual(FRB.ModTypeIdTools.getName(FRB.ModTypeId.AyakaSpringbloom), "AyakaSpringBloom")

    # ================================================
    # ================= getModType =====================

    def test_unregisteredModTypeId_getModTypeReturnsNone(self):
        self.assertIsNone(FRB.ModTypeIdTools.getModType(FRB.ModTypeId.Amber))

    def test_registeredModTypeId_getModTypeReturnsIt(self):
        amber = FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber", ["BaronBunny"])
        FRB.ModTypeIdTools.registerModType(amber)

        result = FRB.ModTypeIdTools.getModType(FRB.ModTypeId.Amber)

        self.assertIsNotNone(result)
        self.assertEqual(result.gameTypeId, int(FRB.GameTypeId.GI))
        self.assertEqual(result.modTypeId, int(FRB.ModTypeId.Amber))
        self.assertEqual(result.name, "Amber")

    # ================================================
    # ================ registerModType ==================

    def test_registerTwice_secondRegistrationOverwritesFirst(self):
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber", ["BaronBunny"]))
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "AmberV2", ["NewAlias"]))

        result = FRB.ModTypeIdTools.getModType(FRB.ModTypeId.Amber)

        self.assertEqual(result.name, "AmberV2")
        self.compareList(list(result.aliases), ["NewAlias"])

    def test_registerDifferentModTypeIds_bothIndependentlyRetrievable(self):
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber"))
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Raiden), "Raiden"))

        self.assertEqual(FRB.ModTypeIdTools.getModType(FRB.ModTypeId.Amber).name, "Amber")
        self.assertEqual(FRB.ModTypeIdTools.getModType(FRB.ModTypeId.Raiden).name, "Raiden")

    # ================================================
    # ================== findByName ====================

    def test_unregisteredName_findByNameReturnsNone(self):
        self.assertIsNone(FRB.ModTypeIdTools.findByName("TotallyUnregisteredName"))

    def test_registeredNameAndAlias_findByNameReturnsCorrectModTypeId(self):
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber", ["BaronBunny", "ColleisBestie"]))

        self.assertEqual(FRB.ModTypeIdTools.findByName("Amber"), FRB.ModTypeId.Amber)
        self.assertEqual(FRB.ModTypeIdTools.findByName("BaronBunny"), FRB.ModTypeId.Amber)

    def test_nameAsSubstring_findByNameMaximallyMatches(self):
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber"))

        self.assertEqual(FRB.ModTypeIdTools.findByName("TextureOverride_Amber_Blend"), FRB.ModTypeId.Amber)

    def test_gameTypeIdFilter_onlyMatchesTheCorrectGame(self):
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber"))

        self.assertEqual(FRB.ModTypeIdTools.findByName("Amber", FRB.GameTypeId.GI), FRB.ModTypeId.Amber)
        self.assertIsNone(FRB.ModTypeIdTools.findByName("Amber", FRB.GameTypeId.WuWa))

    def test_ambiguousNameSharedByTwoModTypeIds_findByNameReturnsNoneRatherThanGuessing(self):
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber", ["SharedAlias"]))
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Ayaka), "Ayaka", ["SharedAlias"]))

        self.assertIsNone(FRB.ModTypeIdTools.findByName("SharedAlias"))
        self.assertIsNone(FRB.ModTypeIdTools.findByName("SharedAlias", FRB.GameTypeId.GI))

    # ================================================
    # ==================== clear ========================

    def test_clear_registryForgotten(self):
        FRB.ModTypeIdTools.registerModType(FRB.CppModType(int(FRB.GameTypeId.GI), int(FRB.ModTypeId.Amber), "Amber", ["BaronBunny"]))
        self.assertIsNotNone(FRB.ModTypeIdTools.getModType(FRB.ModTypeId.Amber))
        self.assertIsNotNone(FRB.ModTypeIdTools.findByName("BaronBunny"))

        FRB.ModTypeIdTools.clear()

        self.assertIsNone(FRB.ModTypeIdTools.getModType(FRB.ModTypeId.Amber))
        self.assertIsNone(FRB.ModTypeIdTools.findByName("BaronBunny"))
        self.assertIsNone(FRB.ModTypeIdTools.findByName("Amber"))

    # ================================================
