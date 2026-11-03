import os
import sys
import tempfile
import unittest

from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class PythonParityTest(unittest.TestCase):
    """Capabilities the C++ fixers use that Python reaches too: where downloads are fetched from, a register's tag, the
    copy preamble, RabbitFX's name, texture thumbprints, running a parser the builder made, and removing one key twice"""

    def test_downloadTools_urlIsTheFolderThenThePath(self):
        path = FRB.DownloadTools.urlPath("WuWa", "Sanhua", "2_5", "Sanhua", "Texture89ba19a1", ".dds")
        self.assertEqual(path, "WuWa/Sanhua/2_5/SanhuaTexture89ba19a1.dds")
        self.assertEqual(FRB.DownloadTools.url("WuWa", "Sanhua", "2_5", "Sanhua", "Texture89ba19a1", ".dds"),
                         FRB.DownloadTools.downloadFolder() + "/" + path)

    def test_getRegTag_dropsDashesAndCapitalises(self):
        self.assertEqual(FRB.IniNamingTools.getRegTag("ps-t0"), "Pst0")
        self.assertEqual(FRB.IniNamingTools.getRegTag("vb4"), "Vb4")

    def test_iniComments_andRabbitFXName(self):
        self.assertTrue(FRB.IniComments.GIMIObjMergerPreamble.startswith(";"))
        self.assertEqual(FRB.RabbitFX.Name, "RabbitFX")

    def test_texThumbprint_identifiesTheCleanWinnerOnly(self):
        stored = [0, 64, 128, 255]
        self.assertAlmostEqual(FRB.TexThumbprint.correlation([0.0, 64.0, 128.0, 255.0], stored), 1.0)
        table = {"same": stored, "inverse": [255, 191, 127, 0]}
        self.assertEqual(FRB.TexThumbprint.identify([1.0, 60.0, 130.0, 250.0], table, 0.9, 0.5), "same")
        self.assertIsNone(FRB.TexThumbprint.identify([1.0, 60.0, 130.0, 250.0], {"a": stored, "b": stored}, 0.9, 0.5))
        self.assertIsNone(FRB.TexThumbprint.of(os.path.join(tempfile.gettempdir(), "noSuchTexture.dds"), 16))

    def test_regRemove_aListCanNameOneKeyTwice(self):
        part = FRB.IfContentPart({"vs-cb3": [(0, "ref ResourceA"), (1, "ref ResourceB"), (2, "ResourceC")]}, 0)
        remove = FRB.RegRemove([("vs-cb3", FRB.RegRemove.valueStartsWith("ref ResourceA")),
                                ("vs-cb3", FRB.RegRemove.valueStartsWith("ref ResourceB"))])
        remove.edit(part, "TextureOverrideX", None)
        self.assertEqual(part.getVals("vs-cb3"), ["ResourceC"])

    def test_cppBaseIniParser_parse_runsABuiltParser(self):
        folder = tempfile.mkdtemp(prefix = "AGRemapParity")
        path = os.path.join(folder, "mod.ini")
        with open(path, "w", encoding = "utf-8") as file:
            file.write("[TextureOverrideRaidenShogunBody]\nhash = 428c56cd\nmatch_first_index = 17769\n")
        ini = FRB.IniFile(file = path)
        ini.classify()
        modType = FRB.ModTypes.Raiden.value
        parser = modType.iniParseBuilder.build(ini, modType.name, None, modType.modTypeId)
        parser.parse()
        self.assertIsInstance(parser.parseData, FRB.IniParseData)

    def test_gimiParser_textureOverrides_firstOwnThisPerHashedSection(self):
        folder = tempfile.mkdtemp(prefix = "AGRemapParity")
        path = os.path.join(folder, "mod.ini")
        with open(path, "w", encoding = "utf-8") as file:
            file.write("[TextureOverrideA]\nhash = ABCD1234\nthis = ResourceA\nthis = ResourceB\n\n"
                       "[TextureOverrideNoHash]\nthis = ResourceC\n\n"
                       "[TextureOverrideB]\nhash = 0000ffff\nif $x == 1\n    this = ref ResourceD\nendif\n\n"
                       "[ResourceA]\nfilename = a.dds\n")
        ini = FRB.IniFile(file = path)
        overrides = FRB.GIMIParser.textureOverrides(ini)
        self.assertEqual([(hash, row.section, row.value) for hash, row in overrides],
                         [("abcd1234", "TextureOverrideA", "ResourceA"), ("0000ffff", "TextureOverrideB", "ref ResourceD")])
        self.assertEqual(overrides[1][1].guards, ["$x == 1"])
