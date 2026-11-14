import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class WWMIHelpersTest(BaseUnitTest):
    """The small readings a WWMI parser and fixer share with a parser or fixer built from Python"""

    def test_drawRangeOf_countAndFirst_wholeNumbersOnly(self):
        self.assertEqual(FRB.IniParseData.drawRangeOf("300, 12, 0"), (300, 12))
        self.assertEqual(FRB.IniParseData.drawRangeOf("300, 12, 5, 7"), (300, 12))
        for value in ("auto", "300, 12", "300, x, 0", "-1, 0, 0", "3, -2, 0"):
            self.assertIsNone(FRB.IniParseData.drawRangeOf(value), value)

    def test_drawRangeOf_zeroBase_onlyThreeFieldsBasedAtZero(self):
        self.assertEqual(FRB.IniParseData.drawRangeOf("300, 12, 0", zeroBase = True), (300, 12))
        self.assertIsNone(FRB.IniParseData.drawRangeOf("300, 12, 4", zeroBase = True))
        self.assertIsNone(FRB.IniParseData.drawRangeOf("300, 12, 0, 1", zeroBase = True))

    def test_roleOfHashType_theRole_itsLiveTwin_andItsOlderGenerations(self):
        roles = {"bodyDiffuse", "bodyMask"}
        role = FRB.WWMITextureFacts.roleOfHashType
        self.assertEqual(role("bodyDiffuse", roles), "bodyDiffuse")
        self.assertEqual(role("bodyDiffuseLive", roles), "bodyDiffuse")
        self.assertEqual(role("bodyMaskOld", roles), "bodyMask")
        self.assertEqual(role("bodyMaskOld2", roles), "bodyMask")
        for hashType in ("bodyDiffuseLive2", "hairDiffuse", "Live", "bodyNormalOld"):
            self.assertIsNone(role(hashType, roles), hashType)

    def test_shader_gameRolesAndStashesAreFieldsOfTheirOwn(self):
        # what a CustomShader binds is typed: a game texture by role, a capture off another slot, and plain lines
        shader = FRB.WWMIFixerConfig.Shader()
        shader.gameRoles = [("ps-t5", "ramp")]
        stash = FRB.WWMIFixerConfig.Shader.Stashed()
        stash.reg, stash.slot, stash.slotReg = "ps-t4", 4, "ps-t4"
        shader.stashed = [stash]
        shader.lines = [("ps-t6", "ref ps-t8")]
        self.assertEqual(list(shader.gameRoles), [("ps-t5", "ramp")])
        self.assertEqual([(s.reg, s.slot, s.slotReg) for s in shader.stashed], [("ps-t4", 4, "ps-t4")])
        self.assertEqual(list(shader.lines), [("ps-t6", "ref ps-t8")])

    def test_slotOf_theSlotsNumber_underTheConfigsPrefix(self):
        config = FRB.WWMIParserConfig()
        self.assertEqual(config.slotOf(("", "component3")), 3)
        self.assertIsNone(config.slotOf(("texture", "component3")))
        self.assertIsNone(config.slotOf(("", "component")))
        self.assertIsNone(config.slotOf(("", "head")))
        self.assertEqual(FRB.WWMIParserConfig.slotWithPrefix(("", "slot12"), "slot"), 12)
        self.assertIsNone(FRB.WWMIParserConfig.slotWithPrefix(("", "slot5x"), "slot"))

    # ---- what a fix decides from the mod (WWMIFixerConfig) ----

    @staticmethod
    def _config():
        B = FRB.WWMIFixerConfig.Binding
        config = FRB.WWMIFixerConfig()
        config.plan = {0: FRB.WWMIFixerConfig.SourceComponent(1, [B("ps-t0", "hairMask"), B("ps-t1", "hairDiffuse")]),
                       2: FRB.WWMIFixerConfig.SourceComponent(3, [B("ps-t0", "bodyDiffuse")])}
        return config

    def test_shaderDraws_onlyAComponentWithAShader_unlessItYieldsToRabbitFX(self):
        config = self._config()
        self.assertFalse(config.shaderDraws(0, False))
        self.assertFalse(config.shaderDraws(5, False))
        plan = config.plan
        shader = FRB.WWMIFixerConfig.Shader()
        shader.yieldToRabbitFX = True
        plan[0].shader = shader
        config.plan = plan
        self.assertTrue(config.shaderDraws(0, False))
        self.assertFalse(config.shaderDraws(0, True))

    def test_bindingsOf_theShadersRegistersReplaceThePlansAtTheSameRegister(self):
        B = FRB.WWMIFixerConfig.Binding
        config = self._config()
        plan = config.plan
        shader = FRB.WWMIFixerConfig.Shader()
        shader.registers = [B("PS-T1", "hairStrand"), B("ps-t4", "foil")]
        shader.gameRoles = [("ps-t5", "ramp"), ("ps-t7", "glow")]
        shader.lines = [("ps-t6", "ref ps-t8")]
        plan[0].shader = shader
        config.plan = plan
        pairs = lambda bindings: [(binding.reg, binding.role) for binding in bindings]
        self.assertEqual(pairs(config.bindingsOf(0, False)), [("ps-t0", "hairMask"), ("ps-t1", "hairDiffuse")])
        self.assertEqual(pairs(config.bindingsOf(0, True)), [("ps-t0", "hairMask"), ("PS-T1", "hairStrand"), ("ps-t4", "foil")])
        self.assertEqual(config.bindingsOf(9, True), [])
        self.assertEqual(config.shaderLineRoles(0), ["ramp", "glow"])
        self.assertEqual(config.shaderLineRoles(2), [])

    def test_passOnlyRoles_onlyASlotSomethingDrawnGoesThrough_eachOnce(self):
        B = FRB.WWMIFixerConfig.Binding
        config = self._config()
        config.extraPassRegs = {1: {"aaaa": [B("ps-t0", "hairDiffuse"), B("ps-t2", "outline")]},
                                3: {"bbbb": [B("ps-t0", "bodyOutline")]}}
        self.assertEqual(config.passOnlyRoles({0}), ["hairDiffuse", "outline"])
        self.assertEqual(config.passOnlyRoles({0, 2}), ["hairDiffuse", "outline", "bodyOutline"])
        self.assertEqual(config.passOnlyRoles(set()), [])

    def test_downloadOf_theSourcesGameTexture_underTheFixsDownloadNames(self):
        config = self._config()
        facts = config.sourceTextures
        facts.downloadGameFolder, facts.downloadCharFolder, facts.downloadVersionFolder = "WuWa", "Sanhua", "2_5"
        facts.downloadPrefix = "Sanhua"
        config.sourceTextures = facts
        download = config.downloadOf("BodyDiffuse", "abcd1234")
        self.assertEqual(download.file, "SanhuaBodyDiffuseRemapDL.dds")
        self.assertEqual(download.resource, FRB.IniNamingTools.getRemapDLResourceName("SanhuaBodyDiffuse"))
        self.assertIn("abcd1234", download.url)

    def test_roleResources_of_theModsOwn_thenTheDownloadForAnOwnShader_thenTheStandIn(self):
        roles = FRB.WWMIFixerConfig.RoleResources()
        roles.ofSlotRole = {("hairDiffuse", 0): "ResourceHair"}
        roles.ofRole = {"bodyDiffuse": "ResourceBodyDiffuseEdited"}
        download = FRB.WWMIFixerConfig.Download()
        download.resource = "ResourceBodyDiffuseRemapDL"
        roles.downloads = {"bodyDiffuse": download}
        self.assertEqual(roles.of("hairDiffuse", 0), "ResourceHair")
        self.assertEqual(roles.of("bodyDiffuse", 0), "ResourceBodyDiffuseEdited")
        self.assertEqual(roles.of("bodyDiffuse", 0, True), "ResourceBodyDiffuseRemapDL")
        self.assertIsNone(roles.of("eyeDiffuse", 0))
        self.assertEqual(roles.shared("hairDiffuse", {2, 0}), "ResourceHair")
        self.assertEqual(roles.shared("bodyDiffuse", {0}, {0}), "ResourceBodyDiffuseRemapDL")

    @staticmethod
    def _lines(lists, kind, component, passNum = 0):
        found = lists.find(kind, component, passNum)
        return None if found is None else [(line.reg, line.value, line.role, line.raw, line.modToggle) for line in found.lines]

    def test_textureLists_eachDrawnComponentsMainList_null_toggledGame_thenTheRolesResource(self):
        B = FRB.WWMIFixerConfig.Binding
        config = self._config()
        plan = config.plan
        plan[0].bindings = [B("ps-t0", "null"), B("ps-t1", "hairDiffuse"), B("ps-t2", "hairMask"), B("ps-t3", "eyeDiffuse")]
        config.plan = plan
        roles = FRB.WWMIFixerConfig.RoleResources()
        roles.ofSlotRole = {("hairDiffuse", 0): "ResourceHair", ("hairMask", 0): "ResourceMask"}
        defaults = FRB.WWMIFixerConfig.GameDefaults()
        defaults.toggled = {("hairMask", 0): "ResourceMaskGame"}

        lists = config.textureLists(FRB.IniParseData(), roles, defaults, {0}, {}, set(), set())
        self.assertEqual([(entry.kind, entry.component) for entry in lists.lists], [("main", 0)])
        self.assertEqual(self._lines(lists, "main", 0), [("ps-t0", "null", "", False, False),
                                                         ("ps-t1", "ResourceHair", "hairDiffuse", False, True),
                                                         ("ps-t2", "ResourceMaskGame", "hairMask", False, False)])
        self.assertEqual(lists.toggles, [])

    def test_textureLists_passLists_forTheirSource_sharedMeshes_andOverlays_inOrder(self):
        B = FRB.WWMIFixerConfig.Binding
        config = self._config()
        config.extraPassRegs = {1: {"aaaa": [B("ps-t4", "hairDiffuse"), B("ps-t5", "hairMask", 2)], "bbbb": []}}
        config.extraPassNoDraw = {1: {"bbbb"}}
        config.sharedMeshes = {"cccc": {"dddd": [B("ps-t0", "bodyDiffuse"), B("ps-t1", "nothing")]}}
        plan = config.plan
        overlay = FRB.WWMIFixerConfig.Shader()
        overlay.registers = [B("ps-t9", "null"), B("ps-t8", "hairDiffuse")]
        plan[0].overlay = overlay
        config.plan = plan
        roles = FRB.WWMIFixerConfig.RoleResources()
        roles.ofSlotRole = {("hairDiffuse", 0): "ResourceHair", ("hairMask", 0): "ResourceMask", ("bodyDiffuse", 2): "ResourceBody"}

        lists = config.textureLists(FRB.IniParseData(), roles, FRB.WWMIFixerConfig.GameDefaults(), {0, 2}, {}, {0}, {0})
        self.assertEqual([(entry.kind, entry.component, entry.onPass, entry.passNum) for entry in lists.lists],
                         [("main", 0, "", 0), ("pass", 0, "aaaa", 0), ("pass", 0, "bbbb", 1), ("main", 2, "", 0),
                          ("sharedMesh", 0, "dddd", 0), ("overlay", 0, "", 0)])
        self.assertEqual(self._lines(lists, "pass", 0), [("ps-t4", "ResourceHair", "hairDiffuse", True, True)])
        self.assertEqual(self._lines(lists, "pass", 0, 1), [("ib", "null", "", False, False)])
        self.assertEqual(self._lines(lists, "sharedMesh", 0), [("ps-t0", "ResourceBody", "bodyDiffuse", False, True)])
        self.assertEqual(self._lines(lists, "overlay", 0), [("ps-t9", "null", "", False, False),
                                                            ("ps-t8", "ResourceHair", "hairDiffuse", True, True)])
        self.assertIsNone(lists.find("game", 0))

    def test_toggleLists_aLineWithNoToggleStaysDirect(self):
        config = self._config()
        roles = FRB.WWMIFixerConfig.RoleResources()
        roles.ofSlotRole = {("hairDiffuse", 0): "ResourceHair"}
        defaults = FRB.WWMIFixerConfig.GameDefaults()
        data = FRB.IniParseData()
        lists = config.toggleLists(config.textureLists(data, roles, defaults, {0}, {}, set(), set()), data, defaults,
                                   "Sanhua", "SanhuaExorcist", "")
        self.assertEqual([line.run for line in lists.find("main", 0).lines], [""])
        self.assertEqual(lists.toggles, [])

    def test_fileGroups_theFirstClaimantOfASlotInTheModsFile_theNextInACopy_unlessSameFileSection(self):
        B = FRB.WWMIFixerConfig.Binding
        config = self._config()
        plan = config.plan
        plan[4] = FRB.WWMIFixerConfig.SourceComponent(1, [B("ps-t0", "hairMask")])
        plan[6] = FRB.WWMIFixerConfig.SourceComponent(1, [B("ps-t0", "hairMask")])
        plan[6].sameFileSection = True
        config.plan = plan

        groups = config.fileGroups({0, 2, 4, 6, 9}, "Sanhua")
        self.assertEqual(groups.groups, [[0, 2, 6], [4]])
        self.assertEqual(groups.dropped, [9])
        self.assertEqual(len(groups.notes), 1)
        self.assertIn("component9", groups.notes[0])
        self.assertEqual(config.fileGroups(set(), "Sanhua").groups, [[]])

    def test_blendWidth_pastAByteBone_unlessTheRowNamesMoreBonesThanOneRemapHolds(self):
        config = self._config()
        self.assertFalse(config.blendWidth(None).targetPast256)

        small = config.blendWidth(FRB.VGRemap({0: 3, 1: 255}))
        self.assertEqual((small.targetPast256, small.bones, small.notes), (False, 2, []))

        past = config.blendWidth(FRB.VGRemap({0: 3, 1: 256}))
        self.assertEqual((past.targetPast256, past.bones, past.notes), (True, 2, []))

        many = config.blendWidth(FRB.VGRemap({k: k + 1 for k in range(FRB.WWMIBlend.RemapSize + 1)}))
        self.assertFalse(many.targetPast256)
        self.assertEqual(many.bones, FRB.WWMIBlend.RemapSize + 1)
        self.assertEqual(len(many.notes), 1)

    def test_removedLines_perComponent_skeletonLinesOnlyPast256_rabbitFXLinesKeptByItsComponents(self):
        R = FRB.WWMIFixerConfig.RegRemoval
        config = self._config()
        config.removedRegs = [R("ResourceBlendBufferOverride", "ref"), R(FRB.IniKeywords.Run, "commandlist\\rabbitfx\\settextures")]
        config.remappedSkeletonRemovals = [R("vs-cb4", "resourcemergedskeleton")]
        keys = lambda lines: [(line.reg, line.valuePrefix) for line in lines]

        lines = config.removedLines(False, {0, 2}, {2})
        self.assertEqual(sorted(lines), [0, 2])
        self.assertEqual(keys(lines[0]), [("ResourceBlendBufferOverride", "ref"), ("run", "commandlist\\rabbitfx\\settextures")])
        self.assertEqual(keys(lines[2]), [("ResourceBlendBufferOverride", "ref")])

        past = config.removedLines(True, {0}, set())
        self.assertEqual(keys(past[0])[-1], ("vs-cb4", "resourcemergedskeleton"))

        config.removedRegs, config.remappedSkeletonRemovals = [], []
        self.assertEqual(config.removedLines(True, {0, 2}, set()), {})

    def test_modTexLines_onlyTheDrawnComponentsOfThePlan_aModWithNoLinesKeepsNone(self):
        config = self._config()
        data = FRB.IniParseData()
        roles, defaults = FRB.WWMIFixerConfig.RoleResources(), FRB.WWMIFixerConfig.GameDefaults()
        lists = config.textureLists(data, roles, defaults, {0, 5}, {}, set(), set())
        self.assertEqual(config.modTexLines(data, roles, lists, {0, 5}, {}, set()), {0: []})

        line = FRB.WWMIFixerConfig.ModTexLine()
        line.key, line.value, line.dropped = "ps-t1", "ResourceA", True
        self.assertEqual((line.key, line.value, line.dropped, line.reg, line.newValue), ("ps-t1", "ResourceA", True, "", ""))

    def test_names_areTheStringsTheFixWrites(self):
        self.assertEqual(FRB.WWMINames.TextureOverridesRows, "WWMIParser.textureOverrides")
        self.assertEqual(FRB.WWMINames.MergeWindowList, "CommandListMergeWindow")
        self.assertEqual(FRB.WWMINames.LiveSuffix, "Live")
