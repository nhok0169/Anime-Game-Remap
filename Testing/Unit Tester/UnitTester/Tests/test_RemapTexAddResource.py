import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RemapTexAddResourceTest(BaseUnitTest):
    """
    Tests for :class:`RemapTexAddResource` -- the C++-backed replacement for the pure-Python
    original -- the pure-Python original (renamed to ``RemapTexAddResourceOld`` mid-migration) has
    since been deleted outright
    """

    def test_construction_isInstanceOfRemapIniResource(self):
        creator = FRB.TexCreator(4, 4)
        r = FRB.RemapTexAddResource("C:/mods/EiRemap", "NewTex.dds", creator)
        self.assertIsInstance(r, FRB.RemapIniResource)

    def test_construction_srcPathResolved(self):
        creator = FRB.TexCreator(4, 4)
        r = FRB.RemapTexAddResource("C:/mods/EiRemap", "NewTex.dds", creator)
        self.assertEqual(r.srcPath.replace("\\", "/"), "C:/mods/EiRemap/NewTex.dds")

    def test_construction_defaultType(self):
        creator = FRB.TexCreator(4, 4)
        r = FRB.RemapTexAddResource("C:/mods/EiRemap", "NewTex.dds", creator)
        self.assertEqual(r.type, "resourceRemapTexAdd")

    def test_fix_withFixFunc_invokedFromPython(self):
        calls = []

        def fixFunc(self):
            calls.append(self.type)
            return True

        creator = FRB.TexCreator(4, 4)
        r = FRB.RemapTexAddResource("C:/mods/EiRemap", "NewTex.dds", creator, type = "resourceTexAdd", fixFunc = fixFunc)
        result = r.fix()

        self.assertTrue(result)
        self.assertEqual(calls, ["resourceTexAdd"])
