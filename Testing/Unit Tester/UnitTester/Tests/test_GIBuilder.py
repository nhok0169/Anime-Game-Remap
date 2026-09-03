import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB

# Every method GIBuilder is expected to expose, one per GI mod type it builds (mirrors the
# pure-Python GIBuilder's own classmethods) -- kept as a plain literal list (not derived by
# introspecting the pure-Python GIBuilder) since that class is currently broken for unrelated
# reasons (VGRemaps/ModMappedAssets), and this file deliberately never imports/calls it.
ALL_METHOD_NAMES = [
    "amber", "amberCN", "ayaka", "ayakaSpringBloom", "arlecchino", "barbara",
    "barbaraSummerTime", "cherryHutao", "diluc", "dilucFlamme", "fischl", "fischlHighness",
    "ganyu", "ganyuTwilight", "huTao", "jean", "jeanCN", "jeanSea", "kaeya",
    "kaeyaSailwind", "keqing", "keqingOpulent", "kirara", "kiraraBoots", "klee",
    "kleeBlossomingStarlight", "lisa", "lisaStudent", "mona", "monaCN", "nilou",
    "nilouBreeze", "ningguang", "ningguangOrchid", "raiden", "rosaria", "rosariaCN",
    "shenhe", "shenheFrostFlower", "xiangling", "xianglingCheer", "xingqiu", "xingqiuBamboo",
]

# method name -> the ModTypeId it's expected to build
METHOD_MOD_TYPE_IDS = {
    "amber": FRB.ModTypeId.Amber,
    "ayaka": FRB.ModTypeId.Ayaka,
    "ayakaSpringBloom": FRB.ModTypeId.AyakaSpringbloom,
    "fischl": FRB.ModTypeId.Fischl,
    "raiden": FRB.ModTypeId.Raiden,
    "xingqiuBamboo": FRB.ModTypeId.XingqiuBamboo,
}


class GIBuilderTest(BaseUnitTest):
    """
    Tests for :class:`GIBuilder` -- 'Cpp'-prefixed since it collides with the live pure-Python
    :class:`GIBuilder`. Deliberately does NOT cross-check against the pure-Python ``GIBuilder``
    (currently broken for unrelated reasons -- ``VGRemaps``/``ModMappedAssets``, see
    Testing/CLAUDE.md's "known-broken" section); everything here is checked against hardcoded
    literals or :class:`ModTypeIdTools` instead.
    """

    # ============ every GI mod type's builder method =

    def test_everyMethod_returnsCorrectlyShapedCppModType(self):
        for methodName in ALL_METHOD_NAMES:
            method = getattr(FRB.GIBuilder, methodName, None)
            self.assertIsNotNone(method, f"GIBuilder has no method '{methodName}'")

            modType = method()

            self.assertIsInstance(modType, FRB.ModType, f"{methodName}(): did not return a ModType")
            self.assertEqual(modType.gameTypeId, int(FRB.GameTypeId.GI), f"{methodName}(): gameTypeId is not GameTypeId.GI")

            expectedEnum = FRB.ModTypeIdTools.getEnum(modType.modTypeId)
            self.assertIsNotNone(expectedEnum, f"{methodName}(): modTypeId {modType.modTypeId} is not a declared ModTypeId")
            self.assertEqual(modType.name, FRB.ModTypeIdTools.getName(expectedEnum), f"{methodName}(): name does not match ModTypeIdTools.getName(modTypeId)")

    def test_everyMethod_modTypeIdMatchesItsOwnCharacter(self):
        for methodName, expectedModTypeId in METHOD_MOD_TYPE_IDS.items():
            modType = getattr(FRB.GIBuilder, methodName)()
            self.assertEqual(modType.modTypeId, int(expectedModTypeId), f"{methodName}(): unexpected modTypeId")

    # ================================================
    # =================== amber =======================

    def test_amber_nameAndAliases(self):
        modType = FRB.GIBuilder.amber()

        self.assertEqual(modType.name, "Amber")
        self.compareSet(set(modType.aliases), {"BaronBunny", "ColleisBestie"})

    # ================================================
    # ============ ayakaSpringBloom ===================

    def test_ayakaSpringBloom_nameHasCapitalB(self):
        # AyakaSpringbloom (enum member, lowercase 'b') -> "AyakaSpringBloom" (name, capital 'B')
        modType = FRB.GIBuilder.ayakaSpringBloom()

        self.assertEqual(modType.name, "AyakaSpringBloom")

    # ================================================
    # ==================== fischl ======================

    def test_fischl_aliasesDoNotHaveTheStrayLeadingSpaceTypo(self):
        # regression test: "FischlvonLuftschlossNarfidort" used to have a stray leading space in
        # the original data (both the Python and C++ sides); confirm the fix stuck
        modType = FRB.GIBuilder.fischl()

        self.assertIn("FischlvonLuftschlossNarfidort", modType.aliases)
        self.assertNotIn(" FischlvonLuftschlossNarfidort", modType.aliases)

    # ================================================
    # ================ xingqiuBamboo ===================

    def test_xingqiuBamboo_lastEnumMemberBuildsCorrectly(self):
        # XingqiuBamboo is the last declared ModTypeId enum member (no trailing comma in the C++
        # enum) -- a real, previously-hit source of a false-alarm parsing gap while generating
        # this class; confirm it still builds correctly
        modType = FRB.GIBuilder.xingqiuBamboo()

        self.assertEqual(modType.name, "XingqiuBamboo")
        self.assertEqual(len(modType.aliases), 14)

    # ================================================
