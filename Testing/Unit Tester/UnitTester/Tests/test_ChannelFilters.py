import math
import os
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def texOf(pixels, path = "never_touched.dds"):
    """A 1-row texture of RGBA pixels, in memory unless saved"""
    tf = FRB.CppTextureFile(path)
    tf.setPixels(bytes([c for px in pixels for c in px]), len(pixels), 1)
    return tf


def pixelsOf(tf):
    px = tf.getPixels()
    return [tuple(px[i:i + 4]) for i in range(0, len(px), 4)]


class RegionFilterTest(BaseUnitTest):
    """:class:`RegionFilter`: another filter run on one region of a texture"""

    def test_onlyTheRegionChanges(self):
        tf = texOf([(1, 2, 3, 4), (5, 6, 7, 8), (9, 10, 11, 12)])
        inner = FRB.ChannelMapFilter()
        inner.setChannel(0, 0, FRB.ChannelMapFilter.constant(255))
        sizes = []

        def region(width, height):
            sizes.append((width, height))
            return [False, True, False]

        FRB.RegionFilter(inner, region).transform(tf)
        self.assertEqual(pixelsOf(tf), [(1, 2, 3, 4), (255, 6, 7, 8), (9, 10, 11, 12)])
        self.assertEqual(sizes, [(3, 1)])

    def test_anEmptyOrMissizedRegion_leavesTheTexture(self):
        for region in (lambda w, h: [], lambda w, h: [True]):
            tf = texOf([(1, 2, 3, 4), (5, 6, 7, 8)])
            inner = FRB.ChannelMapFilter()
            inner.setChannel(0, 0, FRB.ChannelMapFilter.constant(255))
            FRB.RegionFilter(inner, region).transform(tf)
            self.assertEqual(pixelsOf(tf), [(1, 2, 3, 4), (5, 6, 7, 8)])

    def test_noFilterOrRegion_isANoOp(self):
        tf = texOf([(1, 2, 3, 4)])
        FRB.RegionFilter().transform(tf)
        FRB.RegionFilter(FRB.ChannelMapFilter()).transform(tf)
        self.assertEqual(pixelsOf(tf), [(1, 2, 3, 4)])


class ChannelMapFilterTest(BaseUnitTest):
    """:class:`ChannelMapFilter`: each output channel written from a channel of the same pixel through a table"""

    def test_aTable_mapsTheSourceChannel_andLeavesTheOthers(self):
        tf = texOf([(1, 2, 3, 4), (250, 6, 7, 8)])
        filter = FRB.ChannelMapFilter()
        filter.setChannel(0, 0, [min(255, v + 10) for v in range(256)])
        filter.transform(tf)
        self.assertEqual(pixelsOf(tf), [(11, 2, 3, 4), (255, 6, 7, 8)])

    def test_everyChannelIsReadBeforeAnyIsWritten(self):
        tf = texOf([(1, 2, 3, 4)])
        filter = FRB.ChannelMapFilter()
        filter.setChannel(0, 1)
        filter.setChannel(1, 0)
        filter.transform(tf)
        self.assertEqual(pixelsOf(tf), [(2, 1, 3, 4)])

    def test_aConstant_andOneSourceIntoSeveralChannels(self):
        tf = texOf([(9, 2, 3, 4)])
        filter = FRB.ChannelMapFilter()
        for c in (1, 2):
            filter.setChannel(c, 0)
        filter.setChannel(3, 3, FRB.ChannelMapFilter.constant(255))
        filter.transform(tf)
        self.assertEqual(pixelsOf(tf), [(9, 9, 9, 255)])

    def test_data_clearsTheGamma_colourKeepsIt(self):
        for data, expected in ((True, None), (False, 2.2)):
            tf = texOf([(1, 2, 3, 4)])
            tf.gamma = 2.2
            filter = FRB.ChannelMapFilter(data = data)
            filter.setChannel(3, 3, FRB.ChannelMapFilter.constant(0))
            filter.transform(tf)
            self.assertEqual(tf.gamma, expected)

    def test_shaderCode_readsTheDecodedCode(self):
        # stored sRGB, 250 reaches the shader as 243 (truncated)
        tf = texOf([(250, 0, 0, 255)])
        tf.gamma = 2.2
        expected = tf.shaderChannel(0)[0]
        filter = FRB.ChannelMapFilter(read = FRB.ChannelMapFilter.Read.ShaderCode, data = True)
        filter.setChannel(0, 0)
        filter.transform(tf)
        self.assertEqual(pixelsOf(tf)[0][0], expected)
        self.assertNotEqual(expected, 250)

    def test_aTableNot256Long_raises(self):
        filter = FRB.ChannelMapFilter()
        filter.setChannel(0, 0, [1, 2, 3])
        with self.assertRaises(ValueError):
            filter.transform(texOf([(1, 2, 3, 4)]))


class GuidedScaleFilterTest(BaseUnitTest):
    """:class:`GuidedScaleFilter`: each pixel's colour scaled by a factor a guide texture's channel chooses"""

    def setUp(self):
        super().setUp()
        self._folder = tempfile.TemporaryDirectory()
        self.addCleanup(self._folder.cleanup)

    def guide(self, pixels):
        path = os.path.join(self._folder.name, "guide.dds")
        tf = texOf(pixels, path)
        tf.save(compress = False)
        return path

    @classmethod
    def factors(cls, chosen):
        result = [(1.0, 1.0, 1.0)] * 256
        for value, factor in chosen.items():
            result[value] = factor
        return result

    def test_theGuidesValue_choosesTheFactor_alphaUntouched(self):
        guide = self.guide([(1, 0, 0, 255), (2, 0, 0, 255)])
        tf = texOf([(100, 100, 100, 7), (100, 100, 100, 8)])
        FRB.GuidedScaleFilter(guide, 0, self.factors({1: (2.0, 1.0, 0.5)})).transform(tf)
        self.assertEqual(pixelsOf(tf), [(200, 100, 50, 7), (100, 100, 100, 8)])

    def test_aGuideOfAnotherSize_isSampledAtTheSameUV(self):
        guide = self.guide([(1, 0, 0, 255)])
        tf = texOf([(10, 10, 10, 255), (20, 20, 20, 255)])
        FRB.GuidedScaleFilter(guide, 0, self.factors({1: (3.0, 3.0, 3.0)})).transform(tf)
        self.assertEqual(pixelsOf(tf), [(30, 30, 30, 255), (60, 60, 60, 255)])

    def test_resultsAreClamped(self):
        guide = self.guide([(1, 0, 0, 255)])
        tf = texOf([(200, 10, 10, 255)])
        FRB.GuidedScaleFilter(guide, 0, self.factors({1: (2.0, -1.0, 1.0)})).transform(tf)
        self.assertEqual(pixelsOf(tf), [(255, 0, 10, 255)])

    def test_linear_scalesTheDecodedColour_ofAnSRGBTexture(self):
        guide = self.guide([(0, 0, 0, 5)])
        tf = texOf([(200, 200, 200, 255)])
        tf.gamma = 2.2
        FRB.GuidedScaleFilter(guide, 3, self.factors({5: (0.5, 0.5, 0.5)}), linear = True).transform(tf)
        expected = math.floor(math.pow(math.pow(200 / 255, 2.2) * 0.5, 1 / 2.2) * 255 + 0.5)
        self.assertEqual(pixelsOf(tf)[0][:3], (expected, expected, expected))

    def test_noGuide_leavesTheTexture(self):
        for guide in ("", os.path.join(self._folder.name, "missing.dds")):
            tf = texOf([(100, 100, 100, 255)])
            FRB.GuidedScaleFilter(guide, 0, self.factors({0: (2.0, 2.0, 2.0)})).transform(tf)
            self.assertEqual(pixelsOf(tf), [(100, 100, 100, 255)])
