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

    def test_shaderLineRole_onlyARoleLine(self):
        lineRole = FRB.WWMIFixerConfig.Shader.lineRole
        self.assertEqual(lineRole(("ps-t5", "role ramp")), "ramp")
        self.assertEqual(lineRole(("ps-t5", "  role   ramp  ")), "ramp")
        for value in ("ref ps-t8", "stash 4 ps-t4", "role", "roles ramp"):
            self.assertIsNone(lineRole(("ps-t5", value)), value)

    def test_slotOf_theSlotsNumber_underTheConfigsPrefix(self):
        config = FRB.WWMIParserConfig()
        self.assertEqual(config.slotOf(("", "component3")), 3)
        self.assertIsNone(config.slotOf(("texture", "component3")))
        self.assertIsNone(config.slotOf(("", "component")))
        self.assertIsNone(config.slotOf(("", "head")))
        config.slotPrefix = "slot"
        self.assertEqual(config.slotOf(("", "slot12")), 12)

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
        shader.lines = [("ps-t5", "role ramp"), ("ps-t6", "ref ps-t8")]
        shader.passes = [[("ps-t7", "role glow")]]
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

    def test_names_areTheStringsTheFixWrites(self):
        self.assertEqual(FRB.WWMINames.TextureOverridesRows, "WWMIParser.textureOverrides")
        self.assertEqual(FRB.WWMINames.MergeWindowList, "CommandListMergeWindow")
        self.assertEqual(FRB.WWMINames.RoleLine, "role")
        self.assertEqual(FRB.WWMINames.LiveSuffix, "Live")
