import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GameTypeIdTest(BaseUnitTest):
    """
    Tests for :class:`GameTypeIdTools` -- the name/alias lookup for :class:`GameTypeId`, and the
    :class:`GameTypeId` counterpart of :class:`ModTypeIdTools`

    :raw-html:`<br />`

    Unlike :class:`ModTypeIdTools` there is no registry and no ``clear()`` to undo between tests --
    the games *are* the :class:`GameTypeId` members, so nothing here is process-global mutable
    state and nothing leaks between tests
    """

    # ================== getEnum =======================

    def test_declaredValue_getEnumReturnsCorrectEnum(self):
        self.assertEqual(FRB.GameTypeIdTools.getEnum(int(FRB.GameTypeId.GI)), FRB.GameTypeId.GI)
        self.assertEqual(FRB.GameTypeIdTools.getEnum(int(FRB.GameTypeId.WuWa)), FRB.GameTypeId.WuWa)

    def test_undeclaredValue_getEnumReturnsNone(self):
        self.assertIsNone(FRB.GameTypeIdTools.getEnum(-999))

    # ================================================
    # =================== getAll =======================

    def test_getAll_returnsEveryGameInDeclarationOrder(self):
        # Declaration order, not a hash order -- the CLI's --help epilog lists the games with this,
        # so it has to read the same way every run.
        self.assertEqual(FRB.GameTypeIdTools.getAll(), [FRB.GameTypeId.GI, FRB.GameTypeId.WuWa])

    # ================================================
    # ================ getName/getAliases ==============

    def test_declaredValue_getNameReturnsCorrectName(self):
        self.assertEqual(FRB.GameTypeIdTools.getName(FRB.GameTypeId.GI), "GI")
        self.assertEqual(FRB.GameTypeIdTools.getName(FRB.GameTypeId.WuWa), "WuWa")

    def test_getAliases_returnsTheOtherNamesTheGameAnswersTo(self):
        self.assertEqual(set(FRB.GameTypeIdTools.getAliases(FRB.GameTypeId.GI)), {"Genshin", "GenshinImpact"})
        self.assertEqual(set(FRB.GameTypeIdTools.getAliases(FRB.GameTypeId.WuWa)), {"WutheringWaves"})

    def test_getAliases_doesNotRepeatTheGamesOwnName(self):
        for gameTypeId in FRB.GameTypeIdTools.getAll():
            name = FRB.GameTypeIdTools.getName(gameTypeId)
            self.assertNotIn(name, FRB.GameTypeIdTools.getAliases(gameTypeId))

    # ================================================
    # ================== findByName ====================

    def test_name_findByNameResolvesIt(self):
        self.assertEqual(FRB.GameTypeIdTools.findByName("GI"), FRB.GameTypeId.GI)
        self.assertEqual(FRB.GameTypeIdTools.findByName("WuWa"), FRB.GameTypeId.WuWa)

    def test_alias_findByNameResolvesIt(self):
        self.assertEqual(FRB.GameTypeIdTools.findByName("Genshin"), FRB.GameTypeId.GI)
        self.assertEqual(FRB.GameTypeIdTools.findByName("GenshinImpact"), FRB.GameTypeId.GI)
        self.assertEqual(FRB.GameTypeIdTools.findByName("WutheringWaves"), FRB.GameTypeId.WuWa)

    def test_differentCase_findByNameStillResolvesIt(self):
        self.assertEqual(FRB.GameTypeIdTools.findByName("gi"), FRB.GameTypeId.GI)
        self.assertEqual(FRB.GameTypeIdTools.findByName("GENSHIN"), FRB.GameTypeId.GI)
        self.assertEqual(FRB.GameTypeIdTools.findByName("wUtHeRiNgWaVeS"), FRB.GameTypeId.WuWa)

    def test_surroundingWhitespace_findByNameStillResolvesIt(self):
        self.assertEqual(FRB.GameTypeIdTools.findByName("  WuWa  "), FRB.GameTypeId.WuWa)

    def test_unknownName_findByNameReturnsNone(self):
        self.assertIsNone(FRB.GameTypeIdTools.findByName("totally not a game"))
        self.assertIsNone(FRB.GameTypeIdTools.findByName(""))

    # The drift guard: names, aliases and the search DFA are all derived from ONE table in
    # GameType's constructor precisely so they cannot come apart. A name added to the table but
    # missing from the DFA (or the reverse) fails here rather than at a user's command line.
    def test_everyReportedNameAndAlias_roundTripsThroughFindByName(self):
        for gameTypeId in FRB.GameTypeIdTools.getAll():
            names = [FRB.GameTypeIdTools.getName(gameTypeId)] + list(FRB.GameTypeIdTools.getAliases(gameTypeId))

            for name in names:
                self.assertEqual(FRB.GameTypeIdTools.findByName(name), gameTypeId, f"'{name}' did not round-trip")

    # ================================================
    # ================== getHelpStr ====================

    def test_getHelpStr_namesTheGameAndListsItsAliasesSorted(self):
        result = FRB.GameTypeIdTools.getHelpStr(FRB.GameTypeId.GI)

        self.assertIn("name: GI", result)
        # Sorted, matching ModType.getHelpStr, regardless of declaration order
        self.assertIn("aliases: Genshin, GenshinImpact", result)


class GameTypesTest(BaseUnitTest):
    """
    Tests for :class:`GameTypes` -- the presentation layer over :class:`GameTypeIdTools` that the
    CLI's ``--help`` epilog is built from
    """

    def test_getAll_matchesGameTypeIdTools(self):
        self.assertEqual(FRB.GameTypes.getAll(), FRB.GameTypeIdTools.getAll())

    def test_search_resolvesNamesAndAliases(self):
        self.assertEqual(FRB.GameTypes.search("genshinimpact"), FRB.GameTypeId.GI)
        self.assertEqual(FRB.GameTypes.search("  WUWA "), FRB.GameTypeId.WuWa)
        self.assertIsNone(FRB.GameTypes.search("nope"))

    def test_getHelpStr_listsEveryGameWithItsAliases(self):
        result = FRB.GameTypes.getHelpStr()

        self.assertIn("supported types of games", result)
        self.assertIn("not case sensitive", result)

        for gameTypeId in FRB.GameTypes.getAll():
            self.assertIn(f"name: {FRB.GameTypeIdTools.getName(gameTypeId)}", result)

            for alias in FRB.GameTypeIdTools.getAliases(gameTypeId):
                self.assertIn(alias, result)

    def test_getHelpStrCondensed_listsEveryGameByNameOnly(self):
        result = FRB.GameTypes.getHelpStr(showFullGames = False)

        for gameTypeId in FRB.GameTypes.getAll():
            self.assertIn(f"- {FRB.GameTypeIdTools.getName(gameTypeId)}", result)

        self.assertNotIn("aliases:", result)
