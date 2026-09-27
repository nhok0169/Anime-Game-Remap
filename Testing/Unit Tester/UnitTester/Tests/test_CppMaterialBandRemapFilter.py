import os
import shutil
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


Band = FRB.CppMaterialBandRemapFilter.Band


def texOf(pixels):
    """A 1-row texture of RGBA pixels, in memory only"""
    tf = FRB.CppTextureFile("never_touched.dds")
    tf.setPixels(bytes([c for px in pixels for c in px]), len(pixels), 1)
    return tf


def alphasOf(tf):
    px = tf.getPixels()
    return [px[i + 3] for i in range(0, len(px), 4)]


class CppMaterialBandRemapFilterTest(BaseUnitTest):
    """
    Tests for :class:`CppMaterialBandRemapFilter` and its pass-through subclass :class:`MaterialBandRemapFilter`. The core behaviour is
    pinned by ``core/tests/MaterialBandRemapFilter_test.cpp``; these pin the BINDING -- that a table built from
    Python reaches the C++ filter with its bands, ranges, order and gates intact
    """

    def test_singleBand_movesOnlyThatBand_andNothingButAlpha(self):
        tf = texOf([(10, 20, 30, 128), (10, 20, 30, 127), (10, 20, 30, 0)])
        FRB.CppMaterialBandRemapFilter([Band(128, 255)]).transform(tf)

        self.assertEqual(alphasOf(tf), [255, 127, 0])
        self.assertEqual(tf.getPixels()[:3], bytes([10, 20, 30]))

    def test_range_movesEveryBandInside_inclusive(self):
        tf = texOf([(0, 0, 0, 125), (0, 0, 0, 126), (0, 0, 0, 128), (0, 0, 0, 129)])
        FRB.CppMaterialBandRemapFilter([Band(126, 128, 255)]).transform(tf)

        self.assertEqual(alphasOf(tf), [125, 255, 255, 129])

    def test_permutation_decidesFromTheOriginalAlpha(self):
        # applied in sequence 0 -> 255 -> 121 would chase itself onto 121
        tf = texOf([(0, 0, 0, 0), (0, 0, 0, 255)])
        FRB.CppMaterialBandRemapFilter([Band(0, 255), Band(255, 121)]).transform(tf)

        self.assertEqual(alphasOf(tf), [255, 121])

    def test_firstMatchingBandWins(self):
        tf = texOf([(0, 0, 0, 128)])
        FRB.CppMaterialBandRemapFilter([Band(120, 130, 78), Band(128, 255)]).transform(tf)

        self.assertEqual(alphasOf(tf), [78])

    def test_noDiffuse_everyGatePasses(self):
        tf = texOf([(0, 0, 0, 128)])
        FRB.CppMaterialBandRemapFilter([Band(128, 255, FRB.CppMaterialBandRemapFilter.skinColoured)], "").transform(tf)

        self.assertEqual(alphasOf(tf), [255])

    def test_gate_readsTheDiffuseUnderThePixel(self):
        folder = tempfile.mkdtemp()
        try:
            diffusePath = os.path.join(folder, "diffuse.dds")
            diffuse = FRB.CppTextureFile(diffusePath)
            diffuse.setPixels(bytes([224, 180, 160, 255,   128, 128, 128, 255]), 2, 1)   # skin, then grey
            diffuse.save(False)

            tf = texOf([(0, 0, 0, 128), (0, 0, 0, 128)])
            FRB.CppMaterialBandRemapFilter([Band(128, 255, FRB.CppMaterialBandRemapFilter.skinColoured)],
                                           diffusePath).transform(tf)
            self.assertEqual(alphasOf(tf), [255, 128])

            tf = texOf([(0, 0, 0, 128), (0, 0, 0, 128)])
            FRB.CppMaterialBandRemapFilter([Band(128, 255, FRB.CppMaterialBandRemapFilter.skinColoured, negate = True)],
                                           diffusePath).transform(tf)
            self.assertEqual(alphasOf(tf), [128, 255])

            # a Python predicate reaches the filter too
            tf = texOf([(0, 0, 0, 128), (0, 0, 0, 128)])
            FRB.CppMaterialBandRemapFilter([Band(128, 7, lambda r, g, b: r == g == b)], diffusePath).transform(tf)
            self.assertEqual(alphasOf(tf), [128, 7])
        finally:
            shutil.rmtree(folder, ignore_errors = True)

    def test_band_readwrite(self):
        band = Band(126, 128, 255)
        self.assertEqual((band.low, band.high, band.to, band.negate, band.hasTest), (126, 128, 255, False, False))

        band.to = 78
        band.negate = True
        self.assertEqual((band.to, band.negate), (78, True))

        single = Band(77, 178, FRB.CppMaterialBandRemapFilter.skinColoured)
        self.assertEqual((single.low, single.high, single.to, single.hasTest), (77, 77, 178, True))

    def test_filter_readwrite(self):
        f = FRB.CppMaterialBandRemapFilter()
        self.assertEqual(f.bands, [])
        self.assertEqual(f.diffusePath, "")

        f.bands = [Band(1, 2)]
        f.diffusePath = "x.dds"
        self.assertEqual((len(f.bands), f.bands[0].to, f.diffusePath), (1, 2, "x.dds"))

    def test_colourPredicates(self):
        self.assertTrue(FRB.CppMaterialBandRemapFilter.skinColoured(224, 180, 160))
        self.assertFalse(FRB.CppMaterialBandRemapFilter.skinColoured(128, 128, 128))
        self.assertTrue(FRB.CppMaterialBandRemapFilter.whiteFurColoured(235, 235, 235))
        self.assertFalse(FRB.CppMaterialBandRemapFilter.whiteFurColoured(40, 40, 40))

    def test_isCallableAsATexFilter(self):
        tf = texOf([(0, 0, 0, 128)])
        FRB.CppMaterialBandRemapFilter([Band(128, 255)])(tf)

        self.assertEqual(alphasOf(tf), [255])

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.MaterialBandRemapFilter, FRB.CppMaterialBandRemapFilter))
        self.assertTrue(issubclass(FRB.CppMaterialBandRemapFilter, FRB.CppBaseTexFilter))

        tf = texOf([(0, 0, 0, 128)])
        FRB.MaterialBandRemapFilter([Band(128, 255)]).transform(tf)
        self.assertEqual(alphasOf(tf), [255])
