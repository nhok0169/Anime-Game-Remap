import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IniResourceTest(BaseUnitTest):
    """
    Tests for :class:`IniResource`/:class:`IniFixResource` -- the C++-backed replacements for the
    pure-Python originals -- both originals (renamed to ``IniResourceOld``/``IniFixResourceOld``
    mid-migration) have since been deleted outright
    """

    def test_construction_srcPathResolvedAbsolute(self):
        r = FRB.IniResource("blend", "C:/mods/EiRemap", "EiBlend.buf")
        self.assertEqual(r.type, "blend")
        self.assertEqual(r.srcPath.replace("\\", "/"), "C:/mods/EiRemap/EiBlend.buf")

    def test_construction_relativeSrcPathResolved(self):
        r = FRB.IniResource("blend", "C:/mods/EiRemap", "../shared/EiBlend.buf")
        self.assertEqual(r.srcPath.replace("\\", "/"), "C:/mods/shared/EiBlend.buf")

    # Unlike the deprecated pure-Python original (long since deleted), the C++ port drops the generic
    # fixFunc/fix()/_fix() override mechanism at this base level entirely -- IniResource/
    # IniFixResource have no fix()/fixFunc of their own; each concrete leaf (RemapBlendResource,
    # RemapTexAddResource, RemapIniDownload) owns its own concretely-typed fixFunc + fix() pair
    # instead (see this port's own memory notes/doc comments). IniGroupedResource is the one
    # exception that keeps the generic pattern -- see test_IniGroupedResource.py.


class IniFixResourceTest(BaseUnitTest):
    def test_isInstanceOfIniResource(self):
        r = FRB.IniFixResource("blend", "C:/mods/EiRemap", "EiBlend.buf", "RaidenBlend.buf")
        self.assertIsInstance(r, FRB.IniResource)

    def test_construction_bothSrcAndFixedPathsResolved(self):
        r = FRB.IniFixResource("blend", "C:/mods/EiRemap", "EiBlend.buf", "RaidenBlend.buf")
        self.assertEqual(r.srcPath.replace("\\", "/"), "C:/mods/EiRemap/EiBlend.buf")
        self.assertEqual(r.fixedPath.replace("\\", "/"), "C:/mods/EiRemap/RaidenBlend.buf")
