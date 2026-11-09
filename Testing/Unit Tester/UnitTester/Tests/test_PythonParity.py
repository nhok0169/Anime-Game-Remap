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
        self.assertEqual(FRB.RabbitFX.GlowMapKey, "Resource\\RabbitFX\\GlowMap")

    # ---- a variable of another file, carried under a name of the fix's own ----

    def test_varsIn_eachPlainVariable_lowerCased_notAnotherFilesNamespaced(self):
        self.assertEqual(FRB.IniNamingTools.varsIn("if $Break == 1 && $\\entei\\fire == 1 || $x_2"), ["break", "x_2"])
        self.assertEqual(FRB.IniNamingTools.varsIn("global persist $mask"), ["mask"])
        self.assertEqual(FRB.IniNamingTools.varsIn("no variable"), [])

    def test_renameVars_onlyTheNamedOnes_anyCase(self):
        renames = {"break": "breakFix", "breakcheck": "breakcheckFix"}
        self.assertEqual(FRB.IniNamingTools.renameVars("$BREAK != $breakCheck && $other", renames),
                         "$breakFix != $breakcheckFix && $other")
        self.assertEqual(FRB.IniNamingTools.renameVars("$\\ns\\break", renames), "$\\ns\\break")

    def _section(self, text):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        path = os.path.join(folder.name, "t.ini")
        with open(path, "w", encoding="utf-8") as f:
            f.write(text)
        ini = FRB.IniFile(path)
        self.addCleanup(lambda: ini)
        return ini, ini.getIfTemplates()

    def test_graphVarRename_keysValuesAndConditions_movedIntoTheGivenContext(self):
        ini, templates = self._section("[Present]\nif $mask != $maskCheck\n\t$mask = $maskCheck\nendif\npost $maskCheck = 0\n"
                                       "\n[Constants]\nglobal persist $mask = 0\n")
        z3 = FRB.Z3Context()
        present = templates["Present"].deepcopy()
        FRB.GraphVarRename.renameIn(present, {"mask": "maskX", "maskcheck": "maskcheckX"}, z3)
        text = present.toStr()
        for line in ("if $maskX != $maskcheckX", "$maskX = $maskcheckX", "post $maskcheckX = 0"):
            self.assertIn(line, text)
        self.assertNotIn("$mask ", text)

        constants = templates["Constants"].deepcopy()
        FRB.GraphVarRename.renameIn(constants, {"mask": "maskX"})
        self.assertIn("global persist $maskX = 0", constants.toStr())

    def test_varState_closureAndEachSectionCutToItsState_noDrawNorResource(self):
        ini, templates = self._section(
            "[Constants]\nglobal persist $mask = 0\nglobal $other = 0\n"
            "\n[Present]\nif $mask != $maskCheck\n\t$mask = $maskCheck\nendif\nrun = CommandListX\npost $maskCheck = 0\n"
            "\n[KeyMask]\nkey = VK_F1\ntype = cycle\nback = VK_F2\n$mask = 0,1\n$other = 0,1\n"
            "\n[KeyOther]\nkey = VK_F3\n$other = 0,1\n"
            "\n[TextureOverrideTrigger]\nhash = abcdef01\nps-t0 = ResourceX\n$maskCheck = 1\n"
            "\n[TextureOverrideMesh]\nhash = abcdef02\nmatch_first_index = 0\n$mask = 1\n"
            "\n[ResourceX]\nfilename = x.dds\n")
        state = ini.varState({"mask"})
        self.assertEqual(state.vars, {"mask", "maskcheck"})
        self.assertEqual(state.undeclared, {"maskcheck"})

        sections = state.takeSections()
        self.assertEqual([section.name for section in sections], ["Constants", "Present", "KeyMask", "TextureOverrideTrigger"])
        lines = lambda section: [line.strip() for line in section.toStr().splitlines() if line.strip()][1:]
        self.assertEqual(lines(sections[0]), ["global persist $mask = 0"])
        self.assertEqual(lines(sections[1]), ["if $mask != $maskCheck", "$mask = $maskCheck", "endif", "post $maskCheck = 0"])
        self.assertEqual(lines(sections[2]), ["key = VK_F1", "type = cycle", "back = VK_F2", "$mask = 0,1"])
        self.assertEqual(lines(sections[3]), ["hash = abcdef01", "$maskCheck = 1"])
        self.assertEqual(state.takeSections(), [])

        # the file's own sections are untouched
        self.assertIn("run = CommandListX", templates["Present"].toStr())

    def test_removeEmptyBlocks_anIfLeftWithNoLineGoes_andAnEnclosingOneWithIt(self):
        ini, templates = self._section("[Present]\nif $a == 1\n\tif $b == 1\n\t\tx = 1\n\tendif\nendif\nif $c == 1\n\ty = 2\nendif\nz = 3\n")
        present = templates["Present"].deepcopy()
        for part in present.parts:
            if isinstance(part, FRB.IfContentPart):
                entries = part.entries()
                for i in reversed(range(len(entries))):
                    if entries[i][0] == "x":
                        part.removeKVPAt(i)
        self.assertTrue(present.removeEmptyBlocks())
        text = present.toStr()
        self.assertNotIn("$a", text)
        self.assertNotIn("$b", text)
        self.assertIn("if $c == 1", text)
        self.assertIn("z = 3", text)
        self.assertFalse(present.removeEmptyBlocks())

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

    def test_wwmiSections_shaderDraw_beforeLinesRunInsideTheSaveAndRestore(self):
        draw = self.shaderDraw()
        draw.fallback = ("", "")
        draw.before = [("ps-t7", "ResourceGlow")]
        draw.assigned = [("ps-t7", "")]
        backups = FRB.CustomShaderBackups(lambda reg: "B" + reg.replace("-", ""))
        listText = FRB.WWMISections.shaderDrawSections(FRB.Z3Context(), draw, backups)[0].toStr()
        lines = [line.strip() for line in listText.splitlines()]
        save, bind, run = lines.index("Bpst7 = ref ps-t7"), lines.index("ps-t7 = ResourceGlow"), lines.index("run = CustomShaderXShader5Draw0RemapFix")
        restore = lines.index("ps-t7 = ref Bpst7")
        self.assertLess(save, bind)
        self.assertLess(bind, run)
        self.assertLess(run, restore)

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

    def test_wwmiConfigs_ofFactory_theShippedRowsOfTheBuilders(self):
        ids, name = FRB.ModTypeId, FRB.ModTypeIdTools.getName
        pairs = [(ids.Sanhua, ids.SanhuaExorcist), (ids.SanhuaExorcist, ids.Sanhua), (ids.Chisa, ids.ChisaParfait),
                 (ids.ChisaParfait, ids.Chisa), (ids.Changli, ids.ChangliNymph), (ids.ChangliNymph, ids.Changli),
                 (ids.Lynae, ids.LynaePeppermint), (ids.LynaePeppermint, ids.Lynae)]
        for source, target in pairs:
            modType = FRB.ModTypes[name(source)].value
            fix = FRB.WWMIFixerConfig.ofFactory(modType.iniFixBuilder.factoryOf(name(source), name(target)))
            self.assertIsNotNone(fix, name(source))
            self.assertEqual(fix.targetId, target)
            parse = FRB.WWMIParserConfig.ofFactory(modType.iniParseBuilder.factoryOf(name(source)))
            self.assertIsNotNone(parse, name(source))

        self.assertEqual(FRB.WWMIParserConfig.ofFactory(FRB.ModTypes["Sanhua"].value.iniParseBuilder.factoryOf("Sanhua")).version, "2.5")
        # another template's factory, and a pair with no fix, hold no WWMI config
        amber = FRB.ModTypes["Amber"].value
        self.assertIsNone(FRB.WWMIParserConfig.ofFactory(amber.iniParseBuilder.factoryOf("Amber")))
        chisa = FRB.ModTypes["Chisa"].value
        self.assertIsNone(FRB.WWMIFixerConfig.ofFactory(chisa.iniFixBuilder.factoryOf("Chisa", "Sanhua")))
        # a factory made from a config gives that config back
        config = FRB.WWMIFixerConfig.ofFactory(chisa.iniFixBuilder.factoryOf("Chisa", "ChisaParfait"))
        config.sourceVersion = "9.9"
        self.assertEqual(FRB.WWMIFixerConfig.ofFactory(FRB.makeWWMIFixer(config)).sourceVersion, "9.9")

    def test_steps_aGraphVarApplyIsNamedAfterItsVariable_withListTakesOnlyBoundEdits(self):
        apply = FRB.GraphVarApply(["a"], lambda a: a, "b")
        self.assertEqual(apply.name, "b")
        apply.name = "renamed"
        self.assertEqual(apply.name, "renamed")

        factory = FRB.ModTypes["Lynae"].value.iniFixBuilder.factoryOf("Lynae", "LynaePeppermint")
        tweaked = factory.withList(lambda steps: steps)
        self.assertIsInstance(tweaked, FRB.CppIniFixFactory)
        # a tweaked factory wraps the template's: it is not one of makeWWMIFixer's, so it holds no config to read back
        self.assertIsNone(FRB.WWMIFixerConfig.ofFactory(tweaked))

    def test_wwmiSteps_aPlainListTheSameForEveryMod(self):
        lynae = FRB.ModTypes["Lynae"].value
        config = FRB.WWMIFixerConfig.ofFactory(lynae.iniFixBuilder.factoryOf("Lynae", "LynaePeppermint"))
        steps = FRB.wwmiSteps(config)
        names = [step.name for step in steps]
        self.assertEqual(names[:3], ["copies", "toggles", "slotRemap"])
        self.assertEqual(names[-7:], ["blend", "rebind", "fixSections", "shaderFiles", "fixFiles", "latch", "texEdits"])

        # the slot remap is a GraphGroupRemap whose table is the config's plan, read and set like any edit's
        remap = steps[2]
        self.assertIsInstance(remap, FRB.GraphGroupRemap)
        self.assertEqual(remap.remap.keys(), FRB.wwmiSlotRemapStep(config).remap.keys())
        self.assertIn((0, "", "component0"), remap.remap)
        self.assertEqual(remap.remap[(0, "", "component0")][0][:3], (0, "LynaePeppermint", "component0"))

        # a list goes back in as it is; a factory made from it still reads back its config
        factory = FRB.makeWWMIFixer(config, steps + [FRB.GraphGroupEdit([])])
        self.assertEqual(FRB.WWMIFixerConfig.ofFactory(factory).targetId, config.targetId)

        class Mine(FRB.GraphGroupEdit):
            pass
        with self.assertRaises(TypeError):
            FRB.makeWWMIFixer(config, [Mine([])])
        with self.assertRaises(ValueError):
            FRB.makeWWMIFixer(config, [])

    def test_withList_theWholeListRunsWithEditsBuiltInPython(self):
        # the shipped AmberCN -> Amber fix, its list rewritten with edits built in Python: they run in the compiled fix
        folder = tempfile.mkdtemp(prefix = "AGRemapParity")
        with open(os.path.join(folder, "mod.ini"), "w", encoding = "utf-8") as file:
            file.write("[TextureOverrideAmberCNBody]\nhash = b41d4d94\nmatch_first_index = 5670\n"
                       "ib = ResourceAmberCNBodyIB\nps-t0 = ResourceAmberCNBodyDiffuse\n\n"
                       "[ResourceAmberCNBodyIB]\ntype = Buffer\nformat = DXGI_FORMAT_R32_UINT\nfilename = AmberCNBody.ib\n\n"
                       "[ResourceAmberCNBodyDiffuse]\nfilename = AmberCNBodyDiffuse.dds\n")

        seen = []
        body = ("", "body")

        def rewrite(steps):
            seen.extend(step.name for step in steps)
            at = seen.index("fixSections")
            add = FRB.GraphGroupEdit([{body: [FRB.RegAdd([("run", "CommandListTweak")])]}])
            collect = FRB.ResRegCollect({(0,) + body: "ps-t0"},
                                        {"tex": FRB.TexReplace((0, "", "tweakTex"), FRB.CppTexEditor(), resSubType = "Tweak")})
            return steps[:at] + [add, collect] + steps[at:]

        amberCN = FRB.ModTypes["AmberCN"].value
        FRB.CppStrategyOverrides.setFixer("AmberCN", "Amber", amberCN.iniFixBuilder.factoryOf("AmberCN", "Amber").withList(rewrite))
        try:
            service = FRB.RemapService(path = folder, downloadMode = FRB.DownloadMode.Disabled, toModTypeIds = {FRB.ModTypeId.Amber})
            service.fix()
        finally:
            FRB.CppStrategyOverrides.clear()

        # the list handed over is the whole fix: the step that writes the fix's own sections is in it
        self.assertIn("fixSections", seen)
        self.assertEqual(dict(service.stats.ini.skipped), {})
        with open(os.path.join(folder, "mod.ini"), encoding = "utf-8") as file:
            text = file.read()
        remapped = text[text.index("[TextureOverrideAmberCNBodyAmberRemapFix]"):]
        remapped = remapped[:remapped.index("\n\n")]
        self.assertIn("run = CommandListTweak", remapped)
        self.assertIn("ps-t0 = ResourceAmberCNBodyDiffuseAmberTweakRemapTex", remapped)
