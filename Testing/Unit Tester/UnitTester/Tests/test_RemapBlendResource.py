import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RemapBlendResourceTest(BaseUnitTest):
    """
    Tests for :class:`RemapBlendResource` -- the C++-backed replacement for the pure-Python
    original -- the pure-Python original (renamed to ``RemapBlendResourceOld`` mid-migration) has
    since been deleted outright
    """

    def test_construction_isInstanceOfRemapIniFixResource(self):
        vg = FRB.VGRemap({0: 1})
        r = FRB.RemapBlendResource("C:/mods/EiRemap", "EiBlend.buf", "RaidenRemapBlend.buf", vg)
        self.assertIsInstance(r, FRB.RemapIniFixResource)

    def test_construction_pathsResolved(self):
        vg = FRB.VGRemap({0: 1})
        r = FRB.RemapBlendResource("C:/mods/EiRemap", "EiBlend.buf", "RaidenRemapBlend.buf", vg)
        self.assertEqual(r.srcPath.replace("\\", "/"), "C:/mods/EiRemap/EiBlend.buf")
        self.assertEqual(r.fixedPath.replace("\\", "/"), "C:/mods/EiRemap/RaidenRemapBlend.buf")

    def test_fix_withFixFunc_invokedFromPythonWithRealVgRemap(self):
        calls = []

        def fixFunc(self):
            calls.append(self.vgRemap.remap)
            return True

        vg = FRB.VGRemap({0: 1, 1: 0})
        r = FRB.RemapBlendResource("C:/mods/EiRemap", "EiBlend.buf", "RaidenRemapBlend.buf", vg,
                                    type = "resourceRemapBlend", fixFunc = fixFunc)
        result = r.fix()

        self.assertTrue(result)
        self.assertEqual(calls, [{0: 1, 1: 0}])

    def test_construction_defaultType(self):
        vg = FRB.VGRemap()
        r = FRB.RemapBlendResource("C:/mods/EiRemap", "EiBlend.buf", "RaidenRemapBlend.buf", vg)
        self.assertEqual(r.type, "resourceRemapBlend")
