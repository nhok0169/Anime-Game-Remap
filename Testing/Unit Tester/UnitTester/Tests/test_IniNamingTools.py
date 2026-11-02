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

    # ====================== isDisabled / isRemapCopyFile ===============

    def test_isDisabled_aPrefixInAnyCase(self):
        self.assertTrue(FRB.IniNamingTools.isDisabled("DISABLED_mod.ini"))
        self.assertTrue(FRB.IniNamingTools.isDisabled("disabledMod"))
        self.assertFalse(FRB.IniNamingTools.isDisabled("mod_disabled.ini"))

    def test_isRemapCopyFile_aGeneratedCopy(self):
        self.assertTrue(FRB.IniNamingTools.isRemapCopyFile("KeqingRemapFix1.ini"))
        self.assertTrue(FRB.IniNamingTools.isRemapCopyFile("some/folder/keqingremapfix12.INI"))

    def test_isRemapCopyFile_notTheModsOwnFile(self):
        self.assertFalse(FRB.IniNamingTools.isRemapCopyFile("Keqing.ini"))
        self.assertFalse(FRB.IniNamingTools.isRemapCopyFile("MyRemapFixes.ini"))
        self.assertFalse(FRB.IniNamingTools.isRemapCopyFile("KeqingRemapFix1.txt"))
