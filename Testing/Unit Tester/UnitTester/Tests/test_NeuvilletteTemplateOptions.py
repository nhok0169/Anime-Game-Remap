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
