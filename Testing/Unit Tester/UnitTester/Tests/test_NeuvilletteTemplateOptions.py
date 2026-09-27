import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class NeuvilletteTemplateOptionsTest(BaseUnitTest):
    """
    The template options the Neuvillette <-> NeuvilletteMelusent pair added, at the binding: each field's
    default is the behaviour every earlier config was confirmed with (so no compiled character moved), and each
    one set from Python reads back. The behaviour behind them is pinned by the compiled pair's A/B against its
    prototypes and by ``core/tests/VGComponentMerge_test.cpp``; the merge's texcoord floor is checked here too,
    because a stride is only visible in the bytes
    """

    def test_objDownloadRegs_coverRegs(self):
        regs = FRB.GIMICharParserConfig.ObjDownloadRegs("head")
        self.assertEqual(regs.coverRegs, [])

        regs = FRB.GIMICharParserConfig.ObjDownloadRegs("head", "ps-t0", "ps-t1", "", ["ps-t0", "ps-t1", "ps-t2"])
        self.assertEqual(regs.coverRegs, ["ps-t0", "ps-t1", "ps-t2"])
        regs.coverRegs = ["ps-t1"]
        self.assertEqual(regs.coverRegs, ["ps-t1"])

    def test_componentFixerConfig_texRegsByName(self):
        config = FRB.GIMIComponentFixerConfig()
        self.assertFalse(config.texRegsByName)
        config.texRegsByName = True
        self.assertTrue(config.texRegsByName)

    def test_componentFixerConfig_slotIndices(self):
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertEqual(component.slotIndices, [])
        component.slotIndices = ["0", "46620", "71025"]
        self.assertEqual(component.slotIndices, ["0", "46620", "71025"])

    def test_componentFixerConfig_positionOffset(self):
        # all zeros writes the mod's own positions (no line edit at all), which every earlier config relies on
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertEqual(list(component.positionOffset), [0.0, 0.0, 0.0])
        component.positionOffset = [0.0, -0.01237, -0.00021]
        self.assertAlmostEqual(component.positionOffset[1], -0.01237, places = 6)
        self.assertAlmostEqual(component.positionOffset[2], -0.00021, places = 6)

    def test_componentFixerConfig_offsetOnlyWithGameFace(self):
        # off by default: the offset always applies, as every earlier config was confirmed with
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertFalse(component.offsetOnlyWithGameFace)
        component.offsetOnlyWithGameFace = True
        self.assertTrue(component.offsetOnlyWithGameFace)

    def test_componentFixerConfig_seamOptions(self):
        # all off by default: the plain-majority split every earlier config was confirmed with
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertEqual((component.claimShare, component.standIns, component.overlapRings), (0.0, {}, 0))
        component.claimShare = 0.9
        component.standIns = {32: 85, 35: 73}
        component.overlapRings = 1
        self.assertAlmostEqual(component.claimShare, 0.9)
        self.assertEqual(component.standIns, {32: 85, 35: 73})
        self.assertEqual(component.overlapRings, 1)

    def test_componentFixerConfig_dropTexFx(self):
        # off by default: a mod's TexFx lines are carried as before
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertFalse(component.dropTexFx)
        component.dropTexFx = True
        self.assertTrue(component.dropTexFx)

    def test_componentFixerConfig_texFxBlend(self):
        # 0 by default: a dropped TexFx draw stays opaque, as before
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertEqual(component.texFxBlend, 0.0)
        component.texFxBlend = 0.9
        self.assertAlmostEqual(component.texFxBlend, 0.9, places = 6)

    def test_componentFixerConfig_mirroredObjs(self):
        # empty by default: no object gets an inner layer
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertEqual(component.mirroredObjs, [])
        self.assertAlmostEqual(component.mirrorOffset, 0.005, places = 6)
        component.mirroredObjs = ["dress"]
        component.mirrorOffset = 0.002
        self.assertEqual(component.mirroredObjs, ["dress"])
        self.assertAlmostEqual(component.mirrorOffset, 0.002, places = 6)

    def test_componentFixerConfig_sideMeshes(self):
        # empty by default: no earlier config re-issues any section on a side mesh
        config = FRB.GIMIComponentFixerConfig()
        self.assertEqual(config.sideMeshes, [])
        config.sideMeshes = ["ib_face", "ib_headupper"]
        self.assertEqual(config.sideMeshes, ["ib_face", "ib_headupper"])

    def test_mergeFixerConfig_eyeOffsetSideMeshesTexFxGuard(self):
        # all off by default: no merge config before these writes anything new
        component = FRB.GIMIMergeFixerConfig.Component()
        self.assertEqual(list(component.positionOffset), [0.0, 0.0, 0.0])
        self.assertFalse(component.offsetOnlyWithGameFace)
        component.positionOffset = [0.0, 0.01237, 0.00021]
        component.offsetOnlyWithGameFace = True
        self.assertAlmostEqual(component.positionOffset[1], 0.01237, places = 6)
        self.assertTrue(component.offsetOnlyWithGameFace)

        config = FRB.GIMIMergeFixerConfig()
        self.assertEqual(config.sideMeshes, [])
        self.assertFalse(config.texFxGuardUnreached)
        config.sideMeshes = ["ib_face"]
        config.texFxGuardUnreached = True
        self.assertEqual((config.sideMeshes, config.texFxGuardUnreached), (["ib_face"], True))

    def test_mergeComponentFiles_positionLineEdit(self):
        files = FRB.VGMergeComponentFiles(FRB.VGMergeComponentSpec("Eye", FRB.VGRemap({})))
        self.assertIsNone(files.positionLineEdit)
        files.positionLineEdit = lambda line: line[::-1]
        self.assertEqual(files.positionLineEdit(b"\x01\x02\x03"), b"\x03\x02\x01")

    def test_componentFixerConfig_splitGroups(self):
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertEqual(component.splitGroups, {})
        component.splitGroups = {49: [(0, 0.75), (23, 0.25)]}
        self.assertEqual(component.splitGroups, {49: [(0, 0.75), (23, 0.25)]})

    # ---- the side meshes, as both templates write them ----

    SideMeshIni = "\n".join(["[TextureOverrideNFace]", "hash = 24f8b383", "ib = null", "", "; a trailing comment", "",
                              "[TextureOverrideNEyebrows]", "hash = f151ddf7", "ib = null", "",
                              "[TextureOverrideOther]", "hash = 12345678", "ib = null", ""])

    def test_sideMeshes_reissuedOnTheTargetsHash(self):
        out = FRB.SideMeshes.build(self.SideMeshIni, FRB.Hashes(), "Neuvillette", None, ["ib_face", "ib_headupper"],
                                   "NeuvilletteMelusent", None)
        self.assertIn("[TextureOverrideNFaceNeuvilletteMelusentRemap]\nhash = 97cd1620\nib = null\n", out)
        self.assertNotIn("a trailing comment", out)
        # the eyebrows are shared (the same hash on both), and a hash that is no side mesh is none of this
        self.assertNotIn("NEyebrows", out)
        self.assertNotIn("TextureOverrideOther", out)

    def test_sideMeshes_bothWaysAndNothingWithoutTypes(self):
        back = "[TextureOverrideMask]\nhash = 81780578\nib = null\n"
        out = FRB.SideMeshes.build(back, FRB.Hashes(), "NeuvilletteMelusent", None, ["ib_face", "ib_headupper"], "Neuvillette", None)
        self.assertIn("hash = 8559c8e2", out)
        self.assertEqual(FRB.SideMeshes.build(back, FRB.Hashes(), "NeuvilletteMelusent", None, [], "Neuvillette", None), "")

    def test_mergeFixerConfig_componentModTypeName(self):
        component = FRB.GIMIMergeFixerConfig.Component()
        self.assertEqual(component.modTypeName, "")
        component.name = ""
        component.modTypeName = "NeuvilletteMelusentMain"
        self.assertEqual((component.name, component.modTypeName), ("", "NeuvilletteMelusentMain"))

    def test_mergeFixerConfig_texcoordStride(self):
        config = FRB.GIMIMergeFixerConfig()
        self.assertEqual(config.texcoordStride, 0)
        config.texcoordStride = 20
        self.assertEqual(config.texcoordStride, 20)

    def _component(self, name, texcoordStride, fill):
        spec = FRB.VGMergeComponentSpec(name, FRB.VGRemap({0: 5}))
        return FRB.VGMergeComponent(spec, [[1.0, 0.0, 0.0, 0.0]] * 2, [[0, 0, 0, 0]] * 2, bytes(2 * 40),
                                    bytes([fill]) * (2 * texcoordStride))

    def test_merge_texcoordFloor_padsEveryLineAtItsEnd(self):
        merge = FRB.VGComponentMerge([self._component("", 12, 0xAA), self._component("Coat", 12, 0xBB)], 20)

        self.assertEqual(merge.stats.texcoordStride, 20)
        self.assertEqual(len(merge.texcoord), 4 * 20)
        self.assertEqual(merge.texcoord[:20], bytes([0xAA]) * 12 + bytes(8))
        self.assertEqual(merge.texcoord[40:60], bytes([0xBB]) * 12 + bytes(8))

    def test_merge_texcoordFloor_isNeverACut(self):
        merge = FRB.VGComponentMerge([self._component("", 12, 0xAA)], 8)
        self.assertEqual(merge.stats.texcoordStride, 12)

        # and without one, the widest component's, as before
        merge = FRB.VGComponentMerge([self._component("", 12, 0xAA), self._component("Coat", 8, 0xBB)])
        self.assertEqual(merge.stats.texcoordStride, 12)

    def test_mergeGroupResource_texcoordStride(self):
        group = FRB.VGMergeGroupResource("NeuvilletteMelusentNeuvilletteBuffers")
        self.assertEqual(group.texcoordStride, 0)
        group.texcoordStride = 20
        self.assertEqual(group.texcoordStride, 20)
