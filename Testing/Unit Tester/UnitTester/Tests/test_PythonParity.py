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
    copy preamble, RabbitFX's name, texture thumbprints, running a parser the builder made, removing one key twice, a
    file's texture overrides, WWMI's own sections, skipping a `ref` when collecting, and a register edit on some sections"""

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

    def test_wwmiSections_hideSkipsTheSlotAndMergesItsBones(self):
        hide = FRB.WWMISections.Hide()
        hide.name, hide.comment, hide.vb0Hash = "TextureOverrideXComponent4RemapHide", "; hidden", "b101dcf3"
        hide.indexOffset, hide.indexCount, hide.vgOffset, hide.vgCount = "10", "20", "127", "63"
        hide.slot, hide.mergeSkeletonList = "4", "CommandListMergeSkeletonXRemapFix"
        section = FRB.WWMISections.hideSection(FRB.Z3Context(), hide)
        self.assertEqual(section.name, "TextureOverrideXComponent4RemapHide")
        text = section.toStr()
        for line in ("hash = b101dcf3", "match_first_index = 10", "run = CommandListMergeSkeletonXRemapFix", "handling = skip"):
            self.assertIn(line, text)

    def test_wwmiSections_hideMergeOnly_mergesTheWindowWithoutSkipping(self):
        merge = FRB.WWMISections.Hide()
        merge.name, merge.comment, merge.vb0Hash = "TextureOverrideXComponent2RemapMerge", "; merged", "b101dcf3"
        merge.indexOffset, merge.indexCount, merge.vgOffset, merge.vgCount = "10", "20", "127", "63"
        merge.mergeWindowList, merge.mergeOnly = "CommandListMergeWindowRemapFix", True
        text = FRB.WWMISections.hideSection(FRB.Z3Context(), merge).toStr()
        for line in ("hash = b101dcf3", "match_first_index = 10", "vg_offset = 127", "run = CommandListMergeWindowRemapFix"):
            self.assertIn(line, text)
        self.assertNotIn("handling = skip", text)
        self.assertNotIn("$object_detected", text)

        merge.mergeOnly = False
        text = FRB.WWMISections.hideSection(FRB.Z3Context(), merge).toStr()
        self.assertIn("handling = skip", text)
        self.assertIn("$object_detected = 1", text)

    def test_wwmiSections_mergeListNameGoesThroughTheSkeletonsName(self):
        skeleton = FRB.WWMISections.Skeleton()
        skeleton.name = lambda name: name + "RemapFix"
        self.assertEqual(FRB.WWMISections.mergeListName(skeleton, 3), "CommandListMergeSlot3RemapFix")

    @classmethod
    def shaderDraw(cls) -> FRB.WWMISections.ShaderDraw:
        draw = FRB.WWMISections.ShaderDraw()
        draw.name = lambda kind, suffix: kind + "XShader5Draw0" + suffix + "RemapFix"
        draw.draw, draw.onPlan, draw.fallback = "300, 0, 0", "vs == 3381.61", ("drawindexed", "300, 0, 0")
        draw.passes = [[("ps", "XShader.hlsl"), ("ps-t5", "ResourceRamp")], [("ps", "XShaderPass1.hlsl")]]
        draw.assigned = [("ps-t5", "role ramp")]
        return draw

    def test_wwmiSections_shaderDraw_drawsEachPassOnThePlanAndFallsBackElsewhere(self):
        backups = FRB.CustomShaderBackups(lambda reg: "ResourceBackup" + reg.replace("-", ""))
        sections = FRB.WWMISections.shaderDrawSections(FRB.Z3Context(), self.shaderDraw(), backups)
        self.assertEqual([section.name for section in sections],
                         ["CommandListXShader5Draw0RemapFix", "CustomShaderXShader5Draw0RemapFix", "CustomShaderXShader5Draw0RemapFixPass1"])
        listText = sections[0].toStr()
        for line in ("if vs == 3381.61", "ResourceBackuppst5 = ref ps-t5", "run = CustomShaderXShader5Draw0RemapFix",
                     "run = CustomShaderXShader5Draw0RemapFixPass1", "ps-t5 = ref ResourceBackuppst5", "else", "drawindexed = 300, 0, 0"):
            self.assertIn(line, listText)
        self.assertIn("ps-t5 = ResourceRamp", sections[1].toStr())
        self.assertIn("drawindexed = 300, 0, 0", sections[2].toStr())
        self.assertEqual(backups.registers(), ["ps-t5"])
        self.assertEqual(FRB.WWMISections.shaderDrawConstants(self.shaderDraw()), [])

    def test_wwmiSections_shaderDraw_noFallbackDrawsNothingOffThePlan(self):
        draw = self.shaderDraw()
        draw.fallback = ("", "")
        sections = FRB.WWMISections.shaderDrawSections(FRB.Z3Context(), draw, FRB.CustomShaderBackups(lambda reg: "B" + reg))
        listText = sections[0].toStr()
        self.assertIn("run = CustomShaderXShader5Draw0RemapFix", listText)
        self.assertNotIn("else", listText)
        self.assertEqual(listText.count("drawindexed"), 0)

        draw.onlyWith = "o3"
        listText = FRB.WWMISections.shaderDrawSections(FRB.Z3Context(), draw, FRB.CustomShaderBackups(lambda reg: "B" + reg))[0].toStr()
        self.assertIn("if o3 !== null", listText)
        self.assertNotIn("else", listText)

    def test_wwmiSections_shaderDraw_heldBackCountsTheAwaitedDraws(self):
        draw = self.shaderDraw()
        draw.afterTarget, draw.afterDraws, draw.afterCarried = "o3", 2, ["ps-t0"]
        sections = FRB.WWMISections.shaderDrawSections(FRB.Z3Context(), draw, FRB.CustomShaderBackups(lambda reg: "B" + reg))
        names = [section.name for section in sections]
        self.assertEqual(names[:5], ["CommandListXShader5Draw0RemapFix", "CommandListXShader5Draw0AfterRemapFix",
                                     "ResourceXShader5Draw0SeenRemapFix", "ResourceXShader5Draw0PendingRemapFix",
                                     "ResourceXShader5Draw0CarriedPst0RemapFix"])
        self.assertIn("$XShader5Draw0CountRemapFix >= 2", sections[1].toStr())
        self.assertEqual(FRB.WWMISections.shaderDrawConstants(draw),
                         [("global $XShader5Draw0FrameRemapFix", "-1"), ("global $XShader5Draw0CountRemapFix", "0")])

    def test_wwmiFixerConfig_roleKind_andRegisterOf(self):
        Config = FRB.WWMIFixerConfig
        self.assertEqual(Config.roleKind("panelDiffuse"), "Diffuse")
        self.assertEqual(Config.roleKind("ramp"), "")

        bindings = [Config.Binding("ps-t0", "upperNormal"), Config.Binding("ps-t3", "upperDiffuse"),
                    Config.Binding("ps-t5", "null"), Config.Binding("ps-t1", "irisDiffuse")]
        self.assertEqual(Config.registerOf(bindings, "upperNormal"), "ps-t0")
        self.assertEqual(Config.registerOf(bindings, "lowerNormal"), "ps-t0")      # by kind
        self.assertIsNone(Config.registerOf(bindings, "eyeDiffuse"))                # two registers bind a Diffuse
        self.assertIsNone(Config.registerOf(bindings, "ramp"))                      # no kind, no role of its own
        self.assertIsNone(Config.registerOf(bindings, "null"))                      # a null binding binds nothing

    def test_resRegCollect_skipsKeywords_skipsARef(self):
        skips = FRB.ResRegCollect.skipsKeywords()
        self.assertFalse(skips("vb4", "ref ResourceBlendBufferOverride", None))
        self.assertTrue(skips("vb4", "ResourceBlendBuffer", None))

    def test_regPartEdit_keepsItsEditAndFilter(self):
        add = FRB.RegAdd([("run", "CommandListX")], True, True)
        part = FRB.RegPartEdit(add, lambda name: name == "CommandListCleanup")
        self.assertIs(part.edit, add)
        self.assertTrue(part.sectionFilter("CommandListCleanup"))
        self.assertFalse(part.sectionFilter("CommandListOverride"))

    def test_wwmiConfigs_of_theShippedPairsAndCharacters(self):
        ids = FRB.ModTypeId
        fix = FRB.WWMIFixerConfig.of(ids.Chisa, ids.ChisaParfait)
        self.assertIsNotNone(fix)
        self.assertEqual(fix.targetId, ids.ChisaParfait)
        self.assertIsNone(FRB.WWMIFixerConfig.of(ids.Chisa, ids.Sanhua))
        parse = FRB.WWMIParserConfig.of(ids.Sanhua)
        self.assertEqual(parse.version, "2.5")
        self.assertIsNone(FRB.WWMIParserConfig.of(ids.Amber))
