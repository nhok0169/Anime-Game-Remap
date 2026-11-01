import sys
import unittest

from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IniNamingToolsTest(unittest.TestCase):
    """The reference and remap-name rules of :class:`IniNamingTools` that the parsers, fixers and the remover share"""

    # ====================== removeRefPrefix ============================

    def test_removeRefPrefix_takesOffThePrefix_inAnyCase(self):
        self.assertEqual(FRB.IniNamingTools.removeRefPrefix("ref ResourceX"), "ResourceX")
        self.assertEqual(FRB.IniNamingTools.removeRefPrefix("  REF   ResourceX  "), "ResourceX")

    def test_removeRefPrefix_noPrefix_isTheValueStripped(self):
        self.assertEqual(FRB.IniNamingTools.removeRefPrefix(" ResourceX "), "ResourceX")

    def test_removeRefPrefix_onlyAWholeWord(self):
        self.assertEqual(FRB.IniNamingTools.removeRefPrefix("reference"), "reference")
        self.assertEqual(FRB.IniNamingTools.removeRefPrefix("ref"), "ref")

    def test_removeRefPrefix_agreesWithHasKeywordPrefix(self):
        for value in ["ref ResourceX", "Ref ResourceX", "ResourceX", "reference"]:
            hasPrefix = FRB.IniNamingTools.hasKeywordPrefix(value, FRB.IniKeywords.Ref)
            self.assertEqual(FRB.IniNamingTools.removeRefPrefix(value) != value.strip(), hasPrefix, value)

    # ====================== looksRemapped ==============================

    def test_looksRemapped_aFixNameForOneOfTheMods(self):
        name = FRB.IniNamingTools.getRemapFixName("TextureOverrideAmberBody", "AmberCN")
        self.assertTrue(FRB.IniNamingTools.looksRemapped(name, ["Amber", "AmberCN"]))

    def test_looksRemapped_remapAloneIsNotEnough_whenModNamesAreGiven(self):
        # WWMI's own blend remap, on a mod that has never been fixed
        self.assertFalse(FRB.IniNamingTools.looksRemapped("ResourceBlendRemapVertexVGBuffer", ["Chisa", "ChisaParfait"]))

    def test_looksRemapped_noModNames_fallsBackToTheKeywordAnywhere(self):
        self.assertTrue(FRB.IniNamingTools.looksRemapped("ResourceBlendRemapVertexVGBuffer", []))
        self.assertFalse(FRB.IniNamingTools.looksRemapped("ResourceBlendVertexVGBuffer", []))

    def test_looksRemapped_takesTheKeywordGiven(self):
        self.assertTrue(FRB.IniNamingTools.looksRemapped("TextureOverrideBodyAmberFix", ["Amber"], remapKeyword = "Fix"))
        self.assertFalse(FRB.IniNamingTools.looksRemapped("TextureOverrideBodyAmberFix", ["Amber"]))
