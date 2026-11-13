import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GameDataTablesTest(BaseUnitTest):
    """
    The versioned tables of a character's GAME data (DrawPasses, TextureRegisters, GameShaders, ShapeKeyDispatchSizes,
    BoneCentroids, ShapeKeyRemaps, MaterialCodes / MaterialColours, TextureDescs, TextureThumbprints), ModType's getters
    over them, ModMappedAssets.versionOf, and WWMIFixerConfig / WWMIParserConfig.withGameData, which fills a config's
    game data from them. The tables were generated from the data the WWMI fix configs carried until 2026-11-08
    (Tools/Misc/Diagnostics/genWWMIAssetData.py); the configs carry none of it now, so the values pinned here are the ones
    they carried
    """

    V36, V37, V38 = FRB.Version.parse("3.6"), FRB.Version.parse("3.7"), FRB.Version.parse("3.8")

    def modType(self, name):
        return FRB.ModTypes[name].value

    def fixConfig(self, src, tgt):
        return FRB.WWMIFixerConfig.ofFactory(self.modType(src).iniFixBuilder.factoryOf(src, tgt))

    def filledConfig(self, src, tgt, toVersion = None):
        """A pair's fix config with its game data read off the tables, at its own versions -- made for the 3.7 game unless
        'toVersion' says otherwise, the game the values pinned here describe"""
        config = self.fixConfig(src, tgt)
        toVersion = toVersion if toVersion is not None else self.V37
        return config.withGameData(self.modType(src), self.modType(tgt), FRB.Version.parse(config.sourceVersion) if config.sourceVersion else toVersion,
                                   toVersion)

    def filledParseConfig(self, name):
        return FRB.WWMIParserConfig.ofFactory(self.modType(name).iniParseBuilder.factoryOf(name)).withGameData(self.modType(name))

    # =========================== DrawPasses ===========================

    def test_drawPasses_eachTargetSlotsMainPassIsTheConfigs(self):
        config = self.filledConfig("Lynae", "LynaePeppermint")
        target = self.modType("LynaePeppermint")
        self.assertEqual(len(config.slotPasses), 8)
        for slot, passes in enumerate(config.slotPasses):
            component = "component" + str(slot)
            self.assertEqual(target.getDrawPass(component, "main.ps", self.V37), passes[0])
            self.assertEqual(target.getDrawPass(component, "main", self.V37), ",".join(config.passVertexShaders[passes[0]]))

    def test_drawPasses_tagsLineUpWithTheVertexShaders(self):
        target = self.modType("LynaePeppermint")
        shaders = target.getDrawPass("component7", "main", self.V37).split(",")
        tags = target.getDrawPass("component7", "main.tags", self.V37).split(",")
        self.assertEqual(shaders, ["c32a69851757154a", "ceff9a81afb1de70"])
        self.assertEqual(tags, ["3381.615", "3381.625"])

    def test_drawPasses_otherPassIsTheOutline(self):
        self.assertEqual(self.modType("LynaePeppermint").getDrawPass("component3", "other.ps", self.V37), "f8c96a270bf847dd")
        self.assertEqual(self.modType("LynaePeppermint").getDrawPass("component3", "other", self.V37), "6a6650a9db8983ce,e4a3da6d1d1068b9")

    def test_drawPasses_aLaterVersionReadsTheNewestRowBelowIt(self):
        target = self.modType("LynaePeppermint")
        self.assertEqual(target.getDrawPass("component3", "main", FRB.Version.parse("9.9")), target.getDrawPass("component3", "main", self.V38))
        self.assertEqual(target.getDrawPass("component3", "main", FRB.Version.parse("3.7.5")), target.getDrawPass("component3", "main", self.V37))

    def test_drawPasses_theGame38UpdateIsANewerRow(self):
        # 3.8 rehashed every pass shader and moved no model: the 3.7 rows stay for a fix made for 3.7
        target = self.modType("LynaePeppermint")
        self.assertEqual(target.getDrawPass("component3", "main", self.V37), "1f324d354402418c")
        self.assertEqual(target.getDrawPass("component3", "main", self.V38), "2b1664133319ac36")

    # =========================== the version a fix is made at ===========================

    def test_newestVersionOf_isTheNewestDrawPassesRowPastHerNewestModel(self):
        for name in ["Sanhua", "Chisa", "ChisaParfait", "Lynae", "LynaePeppermint", "Changli", "ChangliNymph"]:
            self.assertEqual(str(FRB.WWMIFixerConfig.newestVersionOf(self.modType(name))), "3.8", name)

    def test_newestVersionOf_aCharacterWithNoTablesIsNone(self):
        self.assertIsNone(FRB.WWMIFixerConfig.newestVersionOf(self.modType("Amber")))

    def test_withGameData_aLaterGameMovesTheConfigsRegistersByRole(self):
        # LynaePeppermint -> Lynae binds her lower body's noise and foil at ps-t7 / t8 of Lynae's 3.7 layout; 3.8 put one
        # more texture before them, and her TextureRegisters move both roles one register on
        at37 = {component: [(b.reg, b.role) for b in entry.bindings] for component, entry in self.filledConfig("LynaePeppermint", "Lynae").plan.items()}
        at38 = {component: [(b.reg, b.role) for b in entry.bindings] for component, entry in self.filledConfig("LynaePeppermint", "Lynae", self.V38).plan.items()}
        self.assertIn(("ps-t7", "jacketNoise"), at37[4])
        self.assertIn(("ps-t8", "jacketFoil"), at37[4])
        self.assertIn(("ps-t8", "jacketNoise"), at38[4])
        self.assertIn(("ps-t9", "jacketFoil"), at38[4])
        self.assertEqual(at37[3], at38[3])          # her upper body's registers did not move

    def test_drawPasses_noRowIsNone(self):
        self.assertIsNone(self.modType("LynaePeppermint").getDrawPass("component3", "noSuchPass", self.V37))
        self.assertIsNone(self.modType("Amber").getDrawPass("component3", "main"))

    # =========================== TextureRegisters ===========================

    def test_textureRegisters_roleToRegisterAndBack(self):
        lynae = self.modType("Lynae")
        self.assertEqual(lynae.getTextureRegister("component3", "upperDiffuse", self.V36), "ps-t3")
        self.assertEqual(lynae.getTextureRole("component3", "ps-t3", self.V36), "upperDiffuse")

    def test_textureRegisters_rabbitFXKeyCarriesItsRole(self):
        lynae = self.modType("Lynae")
        self.assertEqual(lynae.getTextureRegister("component6", "propsDiffuse@rabbitfx", self.V36), "Resource\\RabbitFX\\Diffuse")
        self.assertEqual(lynae.getTextureRole("component6", "Resource\\RabbitFX\\Diffuse", self.V36), "propsDiffuse")

    def test_textureRegisters_matchTheParserConfigs(self):
        for name in ("Sanhua", "Chisa", "Lynae", "LynaePeppermint", "Changli"):
            parse = FRB.WWMIParserConfig.ofFactory(self.modType(name).iniParseBuilder.factoryOf(name))
            version = FRB.Version.parse(parse.version)
            for slot, table in parse.textures.registerRoles.items():
                for reg, role in dict(table).items():
                    type_ = role + "@rabbitfx" if reg.startswith("Resource\\RabbitFX") else role
                    self.assertEqual(self.modType(name).getTextureRegister("component" + str(slot), type_, version), reg, (name, slot, role))

    def test_textureRegisters_ownShaderReadsBesides(self):
        # her own lower-body and jacket shaders read her film foil, which no draw of her mods binds
        lynae = self.modType("Lynae")
        self.assertEqual(lynae.getTextureRegister("component4", "filmFoil", self.V36), "ps-t7")
        self.assertEqual(lynae.getTextureRegister("component5", "filmFoil", self.V36), "ps-t4")

    def test_textureRegisters_otherPassIsTheExtraPassRegs(self):
        # every target slot's other pass, as the Lynae -> LynaePeppermint fix binds it, under the target's own role
        config = self.fixConfig("Lynae", "LynaePeppermint")
        target = self.modType("LynaePeppermint")
        for slot, passes in config.extraPassRegs.items():
            for bindings in passes.values():
                for b in bindings:
                    role = target.getTextureRole("component" + str(slot), b.reg, self.V37, "other")
                    self.assertIsNotNone(role, (slot, b.role))
                    self.assertEqual(target.getTextureRegister("component" + str(slot), role + "@other", self.V37), b.reg)

    def test_textureRegisters_aRoleOnAnotherPassIsNotTheMainPasss(self):
        # ps-t0 of the skin's upper body is its normal map on the main pass and its diffuse on the outline
        target = self.modType("LynaePeppermint")
        self.assertEqual(target.getTextureRole("component3", "ps-t0", self.V37), "upperNormal")
        self.assertEqual(target.getTextureRole("component3", "ps-t0", self.V37, "other"), "upperDiffuse")
        self.assertIsNone(target.getTextureRole("component2", "ps-t0", self.V37, "other"))
        self.assertEqual(target.getTextureRegister("component5", "pouchFoil", self.V37), "ps-t5")

    def test_textureRegisters_gameDefaultWhereTheModHasNoRow(self):
        lynae = self.modType("Lynae")
        table = FRB.TextureRegisters()
        table.addRepoRows({"3.0": {"WuWa": {"": {"glowMap": "ps-t9"}}},
                           "4.0": {"WuWa": {"": {"glowMap": "ps-t10"}}}})
        modType = FRB.ModType(lynae.gameTypeId, lynae.modTypeId, "Lynae", textureRegisters = table)
        self.assertEqual(modType.getTextureRegister("component3", "glowMap", self.V36), "ps-t9")
        self.assertEqual(modType.getTextureRegister("component3", "glowMap", FRB.Version.parse("4.0")), "ps-t10")
        self.assertEqual(modType.getTextureRole("component3", "ps-t10", FRB.Version.parse("4.0")), "glowMap")
        self.assertIsNone(modType.getTextureRole("component3", "ps-t10", self.V36))
        # the character's own row wins over the game's
        self.assertEqual(modType.getTextureRegister("component3", "upperDiffuse", self.V36), "ps-t3")

    # =========================== GameShaders ===========================

    def test_gameShaders_areHerGeneratedShaders(self):
        lynae = self.modType("Lynae")
        shader = FRB.LynaeData.upperShader()
        self.assertEqual(lynae.getGameShader("LynaeUpper", self.V37), shader.ps)
        words = lynae.getGameShader("LynaeUpper.ps-cb4", self.V37).split(",")
        self.assertEqual([int(word, 16) for word in words], list(shader.constantBuffers["ps-cb4"]))

    def test_gameShaders_anOlderVersionReadsTheOldestRow(self):
        lynae = self.modType("Lynae")
        self.assertEqual(lynae.getGameShader("LynaeUpper", self.V36), lynae.getGameShader("LynaeUpper", self.V37))

    # =========================== ShapeKeyDispatchSizes ===========================

    def test_shapeKeyDispatchSizes_areTheConfigs(self):
        for src, tgt, size in (("Lynae", "LynaePeppermint", "1854"), ("Sanhua", "SanhuaExorcist", "853"), ("Chisa", "ChisaParfait", "1301")):
            config = self.filledConfig(src, tgt)
            self.assertEqual(self.modType(tgt).getShapeKeyDispatchSize(FRB.Version.parse(config.version)), size)
            self.assertEqual(config.shapeKeyDispatchSize, size)

    # =========================== BoneCentroids ===========================

    def test_boneCentroids_areTheParsersReference(self):
        parse = self.filledParseConfig("Lynae")
        got = self.modType("Lynae").getBoneCentroids()
        self.assertEqual(len(got), 333)
        self.assertEqual(set(got), set(parse.referenceBoneCentroids))
        self.assertEqual([round(v, 4) for v in got[0]], [1.0817, -1.8451, 142.3504])
        for bone, centroid in parse.referenceBoneCentroids.items():
            for a, b in zip(got[bone], centroid):
                self.assertAlmostEqual(a, b, places = 4)

    def test_boneCentroids_noneForACharacterWithout(self):
        self.assertEqual(self.modType("Amber").getBoneCentroids(), {})

    # =========================== ShapeKeyRemaps ===========================

    def test_shapeKeyRemaps_areTheConfigsOrder(self):
        order = self.modType("Lynae").getShapeKeyRemap("LynaePeppermint", self.V36, self.V37)
        self.assertEqual(len(order), 105)
        self.assertEqual(order[:8], [5, 1, 3, 2, 4, 0, 23, 22])
        self.assertEqual(list(self.filledConfig("Lynae", "LynaePeppermint").shapeKeyOrder), order)

    def test_shapeKeyRemaps_aLaterSourceVersionReadsTheNewestRowBelowIt(self):
        self.assertEqual(self.modType("Lynae").getShapeKeyRemap("LynaePeppermint", self.V37, self.V37),
                         self.modType("Lynae").getShapeKeyRemap("LynaePeppermint", self.V36, self.V37))

    def test_shapeKeyRemaps_noneForAPairWithout(self):
        self.assertIsNone(self.modType("Lynae").getShapeKeyRemap("Sanhua", self.V37, FRB.Version.parse("2.5")))

    def test_shapeKeyRemaps_tableGet(self):
        table = FRB.ShapeKeyRemaps()
        self.assertGreater(len(table), 0)
        self.assertTrue(table.get("Lynae", "LynaePeppermint", self.V36, self.V37).startswith("5,1,3,2"))

    # =========================== MaterialCodes / MaterialColours ===========================

    def test_materialCodes_rebuildHerPropsCodeTable(self):
        # her film flag kept, any other code's material made the pouch's plain one: the table the fix had carried (LynaeFixer
        # builds it from these rows since 2026-11-09)
        lynae, skin = self.modType("Lynae"), self.modType("LynaePeppermint")
        film = lynae.getMaterialCode("component6", "propsDetail.flags.film", self.V36)
        material = lynae.getMaterialCode("component6", "propsDetail.material", self.V36)
        plain = skin.getMaterialCode("component5", "pouchDetail.material.plain", self.V37)
        self.assertEqual((film, material, plain), (0x10, 0x0F, 15))
        self.assertEqual([c if c & film else (c & ~material) | plain for c in range(256)], [c if c & 0x10 else (c & 0xF0) | 0x0F for c in range(256)])

    def test_materialCodes_noRowIsNone(self):
        self.assertIsNone(self.modType("Lynae").getMaterialCode("component6", "propsDetail.flags.noSuchFlag", self.V36))

    def test_materialColours_aRowIsAColour(self):
        colour = self.modType("SanhuaExorcist").getMaterialColour("component3", "torsoMask.skin", FRB.Version.parse("2.5"))
        self.assertEqual(tuple(colour.getTuple()), (255, 77, 0, 255))
        self.assertIsNone(self.modType("SanhuaExorcist").getMaterialColour("component3", "torsoMask.noSuch"))

    def test_materialColours_valueRoundTrips(self):
        colour = FRB.CppColour(1, 2, 3, 4)
        self.assertEqual(FRB.MaterialColours.valueOf(colour), "1,2,3,4")
        self.assertEqual(tuple(FRB.MaterialColours.colourOf("1,2,3,4").getTuple()), (1, 2, 3, 4))
        self.assertRaises(ValueError, FRB.MaterialColours.colourOf, "1,2,3")
        self.assertRaises(ValueError, FRB.MaterialColours.colourOf, "1,2,x,4")

    # =========================== withGameData ===========================

    def test_withGameData_aConfigCarriesNoGameData(self):
        config = self.fixConfig("Sanhua", "SanhuaExorcist")
        self.assertEqual((list(config.slotPasses), dict(config.passVertexShaders), config.shapeKeyDispatchSize), ([], {}, ""))

    def test_withGameData_fillsTheTargetsPasses(self):
        config = self.filledConfig("Sanhua", "SanhuaExorcist")
        self.assertEqual([list(p) for p in config.slotPasses],
                         [["0f1752e476c3804d", "394feb3747bda7a0"], ["0f1752e476c3804d"], ["ed1c0f8b2ba08ac4"], ["fbe2f2f061949559"],
                          ["348ac69e57e38686"], ["0512a7e61d6bd01d"]])
        # the target's passes' tags, and the SOURCE's own (Sanhua's, which her mod's sections may carry)
        self.assertEqual(dict(config.filterIndices), {"02f5c4b333a78fa0": "3381.94", "15a2c90dd35b8c07": "3381.95", "4ce754b5b9574b1a": "3381.769",
                                                      "7c0b4db32cee62d3": "3381.76", "a695e6d5f9d489cf": "3381.91", "a79e471015b2a16c": "3381.96",
                                                      "0b25e6cbdb8cc86f": "3381.83", "ad59bcdcf9ce68fa": "3381.84"})
        self.assertEqual(list(config.passVertexShaders["ed1c0f8b2ba08ac4"]), ["7c0b4db32cee62d3", "4ce754b5b9574b1a"])

    def test_withGameData_anOtherPassNamedIsItsShader(self):
        raw = self.fixConfig("Lynae", "LynaePeppermint")
        self.assertEqual(list(raw.extraPassRegs[0]), ["other"])
        self.assertEqual(list(self.filledConfig("Lynae", "LynaePeppermint").extraPassRegs[0]), ["bce1512f1c6b82fe"])

    def test_withGameData_aPassKeyedTwiceIsRefused(self):
        config = self.fixConfig("Lynae", "LynaePeppermint")
        extra = config.extraPassRegs
        extra[0]["bce1512f1c6b82fe"] = list(extra[0]["other"])
        config.extraPassRegs = extra
        self.assertRaises(ValueError, config.withGameData, self.modType("Lynae"), self.modType("LynaePeppermint"), self.V36, self.V37)

    def test_withGameData_fillsTheGameShaders(self):
        lynae = self.modType("Lynae")
        config = self.filledConfig("Lynae", "LynaePeppermint")
        upper = config.plan[3].shader
        self.assertEqual((upper.name, upper.game, upper.ps), ("LynaeUpper", "LynaeUpper", lynae.getGameShader("LynaeUpper", self.V36)))
        self.assertEqual(list(upper.constantBuffers["ps-cb4"]), [int(w, 16) for w in lynae.getGameShader("LynaeUpper.ps-cb4").split(",")])
        jacket = config.plan[5].shader                   # her film shader under another name
        self.assertEqual((jacket.name, jacket.game, jacket.ps), ("LynaeJacket", "LynaeFilm", lynae.getGameShader("LynaeFilm")))

    def test_withGameData_fillsAPassesOwnShader(self):
        config = self.filledConfig("LynaePeppermint", "Lynae")
        sheer = config.plan[6].shader
        self.assertEqual(list(sheer.passGame), ["", "LynaePeppermintSolid"])
        self.assertEqual(sheer.passPs[1], self.modType("LynaePeppermint").getGameShader("LynaePeppermintSolid"))

    def test_withGameData_aShaderWithNoRowIsRefused(self):
        config = self.fixConfig("Sanhua", "SanhuaExorcist")
        plan = config.plan
        plan[0].shader = FRB.WWMIFixerConfig.Shader.ofGame("NoSuchShader")
        config.plan = plan
        self.assertRaises(ValueError, config.withGameData, self.modType("Sanhua"), self.modType("SanhuaExorcist"))

    def test_withGameData_fillsTheStashAndTheCreatedColour(self):
        self.assertEqual({k: list(v) for k, v in self.filledConfig("Lynae", "LynaePeppermint").stashRegs.items()}, {4: ["ps-t4", "ps-t5", "ps-t6"]})
        created = self.filledConfig("Sanhua", "SanhuaExorcist").createdTextures[0]
        self.assertEqual((created.role, tuple(created.material), tuple(created.colour.getTuple())),
                         ("SkinMask", ("component3", "torsoMask.skin"), (255, 77, 0, 255)))

    def test_typedGetters_passNamesShadersTagsAndBuffers(self):
        target = self.modType("LynaePeppermint")
        self.assertEqual(list(target.getDrawPassShaders("component3", "other", self.V37)), ["6a6650a9db8983ce", "e4a3da6d1d1068b9"])
        self.assertEqual(list(target.getDrawPassShaders("component3", "noSuchPass", self.V37)), [])
        self.assertEqual(target.getDrawPassNames("component3", "other", self.V37)[0], "other")
        self.assertEqual(len(target.getDrawPassTags("component3", "main", self.V37)), len(target.getDrawPassShaders("component3", "main", self.V37)))
        lynae = self.modType("Lynae")
        self.assertEqual(list(lynae.getGameShaderBuffer("LynaeUpper", "ps-cb4", self.V37)),
                         [int(w, 16) for w in lynae.getGameShader("LynaeUpper.ps-cb4", self.V37).split(",")])
        self.assertIsNone(lynae.getGameShaderBuffer("LynaeUpper", "ps-cb9", self.V37))

    def test_textureKinds_aRolesKind_onItsSlotElseTheCharacters(self):
        lynae = self.modType("Lynae")
        self.assertEqual(lynae.getTextureKind("component3", "upperDiffuse", self.V36), "Diffuse")
        self.assertEqual(lynae.getTextureKind("component3", "ramp", self.V36), "")          # a game texture: no kind
        self.assertIsNone(lynae.getTextureKind("component3", "noSuchRole", self.V36))
        self.assertEqual(lynae.getTextureKinds(self.V36)["jacketDetail"], "Detail")

    def test_withGameData_fillsTheRoleKinds_andRefusesARoleWithNoRow(self):
        self.assertEqual(self.filledConfig("Lynae", "LynaePeppermint").kindOf("lowerSheer"), "Sheer")
        config = self.fixConfig("Sanhua", "SanhuaExorcist")
        plan = config.plan
        plan[0].bindings = list(plan[0].bindings) + [FRB.WWMIFixerConfig.Binding("ps-t9", "noSuchRole")]
        config.plan = plan
        self.assertRaises(ValueError, config.withGameData, self.modType("Sanhua"), self.modType("SanhuaExorcist"))

    def test_withGameData_fillsAShadersRegistersFromItsOwnDraw(self):
        # a copied shader's empty registers are where its own draw binds each role (the source's TextureRegisters), and
        # a capture's / a ref's slot register the target's
        plan = self.filledConfig("Lynae", "LynaePeppermint").plan
        lower = plan[4].shader
        self.assertEqual([(s.role, s.slot, s.reg, s.slotReg) for s in lower.stashed],
                         [("glow", 4, "ps-t4", "ps-t4"), ("flow", 4, "ps-t5", "ps-t5"), ("noise", 4, "ps-t6", "ps-t6")])
        self.assertEqual([(b.reg, b.role) for b in lower.registers], [("ps-t7", "filmFoil"), ("ps-t8", "lowerSheer")])
        self.assertEqual(list(lower.gameRoles), [("ps-t9", "ramp")])
        self.assertEqual(list(plan[3].shader.lines)[0], ("ps-t4", "ref ps-t8"))     # her matcap, off the skin's slot
        self.assertEqual(list(plan[106].shader.gameRoles), [("ps-t5", "ramp")])   # her jacket's shader, on the props

    def test_withGameData_sourceVersionFromVb0(self):
        self.assertEqual(dict(self.filledConfig("Lynae", "LynaePeppermint").sourceVersionByVb0), {"0c33d628": "3.6", "7e400733": "3.7"})
        self.assertEqual(dict(self.filledConfig("Sanhua", "SanhuaExorcist").sourceVersionByVb0), {})
        self.assertEqual(dict(FRB.WWMIFixerConfig.vb0VersionsOf(self.modType("Changli"))), {"77975500": "1.1", "5f8aac45": "1.2", "d14bed8b": "1.4"})

    def test_withGameData_aParserWithoutARenumberingKeepsNone(self):
        parse = self.filledParseConfig("ChisaParfait")      # two models, no renumbering between them
        self.assertEqual(list(parse.skeletonNumberings), [])

    # =========================== TextureThumbprints ===========================

    def test_textureThumbprints_areSixteenBySixteen(self):
        prints = self.modType("Sanhua").getTextureThumbprints()
        self.assertEqual(len(prints), 20)
        self.assertTrue(all(len(p) == 256 for p in prints.values()))
        self.assertEqual(list(prints["1035197c"])[:4], [10, 10, 10, 10])
        self.assertEqual(self.modType("Amber").getTextureThumbprints(), {})

    def test_textureThumbprints_fillBothConfigs(self):
        prints = {k: list(v) for k, v in self.modType("Lynae").getTextureThumbprints().items()}
        self.assertEqual({k: list(v) for k, v in self.filledParseConfig("Lynae").textures.textureThumbprints.items()}, prints)
        self.assertEqual({k: list(v) for k, v in self.filledConfig("Lynae", "LynaePeppermint").sourceTextures.textureThumbprints.items()}, prints)

    # =========================== TextureDescs / RabbitFX tags ===========================

    def test_textureDescs_aRowIsTheGamesDescription(self):
        self.assertEqual(self.modType("Lynae").getTextureDesc("7d044e58", self.V37), ("BC7_TYPELESS", 2048, 2048, 12))
        self.assertIsNone(self.modType("Lynae").getTextureDesc("00000000", self.V37))

    def test_rabbitFXTags_ofHerDrawsAreTheConfigs(self):
        # what her own draws bind (DrawPasses' main.ps-tN), described (TextureDescs), tagged (RabbitFX.tagOf): the tags her
        # fix config had carried as numbers, and withGameData fills them from
        lynae = self.modType("Lynae")
        config = self.filledConfig("Lynae", "LynaePeppermint")
        for slot in (0, 1, 3, 4, 5):
            component = "component" + str(slot)
            tags = [FRB.RabbitFX.tagOf(*lynae.getTextureDesc(lynae.getDrawPass(component, "main.ps-t" + str(k), self.V37), self.V37)) for k in range(4)]
            self.assertEqual(tags, [dict(config.plan[slot].rabbitFX.gameTags).get("ps-t" + str(k), 0) for k in range(4)], component)

    def test_rabbitFXTagOf_byNameAndNumber(self):
        self.assertEqual(FRB.RabbitFX.tagOf("BC7_TYPELESS", 2048, 2048, 12), 3)
        self.assertEqual(FRB.RabbitFX.tagOf(97, 2048, 2048, 12), 3)
        self.assertEqual(FRB.RabbitFX.tagOf("r8_unorm", 2048, 2048, 12), 4)
        self.assertEqual(FRB.RabbitFX.tagOf("B8G8R8A8_TYPELESS", 256, 128, 1), 0)    # not square
        self.assertEqual(FRB.RabbitFX.tagOf("BC3_TYPELESS", 512, 512, 10), 0)

    # =========================== Colours / ColourRanges ===========================

    def test_colours_areTheSharedOnes(self):
        self.assertEqual(tuple(FRB.Colours.NormalMapYellow.getTuple()), (128, 128, 0, 255))
        self.assertEqual(tuple(FRB.Colours.NormalMapPurple1.getTuple()), (128, 98, 128, 255))
        self.assertEqual(tuple(FRB.Colours.LightMapGreen.getTuple()), (0, 128, 0, 255))
        self.assertEqual(tuple(FRB.Colours.White.getTuple()), (255, 255, 255, 255))

    def test_colourRanges_spanTheirEnds(self):
        band = FRB.ColourRanges.LightMapGreen
        self.assertTrue(band.match(FRB.CppColour(25, 140, 25, 200)))
        self.assertFalse(band.match(FRB.Colours.White))

    # =========================== vertex group maps ===========================

    def test_vgMaps_areHerComponentsOntoHerMergedSkeleton(self):
        lynae = self.modType("Lynae")
        maps = self.filledConfig("Lynae", "LynaePeppermint").sourceVgMaps
        self.assertEqual({c: len(groups) for c, groups in maps.items()}, {0: 45, 1: 72, 2: 1, 3: 111, 4: 93, 5: 48, 6: 30, 7: 1})
        self.assertEqual(list(maps[3])[:4], [106, 119, 120, 121])
        for component, groups in maps.items():
            row = lynae.getVGRemap("Lynae", self.V36, self.V36, "component" + str(component), "")
            self.assertEqual([row.remap[i] for i in range(len(groups))], list(groups), component)
        # ...and at 3.7 her own maps, not the 3.6 ones (component 3's local bone 11 is merged bone 0 at 3.7)
        self.assertEqual(lynae.getVGRemap("Lynae", self.V37, self.V37, "component3", "").remap[11], 0)

    # =========================== versionOf / numberings ===========================

    def test_versionOf_aModsVb0TellsItsVersion(self):
        hashes = self.modType("Lynae").hashes
        self.assertEqual(hashes.versionOf("0c33d628", ["Lynae", None]), self.V36)
        self.assertEqual(hashes.versionOf("7e400733", ["Lynae", None]), self.V37)
        self.assertIsNone(hashes.versionOf("00000000", ["Lynae", None]))

    def test_skeletonNumbering_theReferenceVersionIsNotRenumbered(self):
        # her reference numbering's own row stops the lookup falling back to the older numbering's
        row = self.modType("Lynae").getVGRemap("Lynae", self.V37, self.V37)
        self.assertTrue(row is not None and all(k == v for k, v in row.remap.items()))

    def test_skeletonNumbering_isAVGRemapOntoTheReferenceVersion(self):
        row = self.modType("Lynae").getVGRemap("Lynae", self.V36, self.V37)
        moved = {k: v for k, v in row.remap.items() if k != v}
        self.assertEqual(len(moved), 71)                 # 71 of her merged ids moved at 3.7
        self.assertEqual([moved[k] for k in (144, 145, 146)], [145, 146, 147])
        numberings = self.filledParseConfig("Lynae").skeletonNumberings
        self.assertEqual([n.version for n in numberings], ["3.6", "3.7"])
        self.assertEqual(dict(numberings[0].toReference), moved)
        self.assertEqual(dict(numberings[1].toReference), {})
