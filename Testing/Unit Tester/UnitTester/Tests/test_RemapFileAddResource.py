import os
import sys
import tempfile

from .baseUnitTest import BaseUnitTest, ModsRoot
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RemapFileAddResourceTest(BaseUnitTest):
    """
    Tests for :class:`RemapFileAddResource` -- a brand new file of known contents (a custom shader's
    source, its constant buffer) written by the service with the rest of a .ini file's resources
    """

    def test_construction_isInstanceOfRemapIniResource(self):
        r = FRB.RemapFileAddResource(f"{ModsRoot}/EiRemap", "Shader.hlsl", b"x")
        self.assertIsInstance(r, FRB.RemapIniResource)

    def test_construction_srcPathResolved(self):
        r = FRB.RemapFileAddResource(f"{ModsRoot}/EiRemap", "Shader.hlsl", b"x")
        self.assertEqual(r.srcPath.replace("\\", "/"), f"{ModsRoot}/EiRemap/Shader.hlsl")

    def test_construction_defaultType(self):
        r = FRB.RemapFileAddResource(f"{ModsRoot}/EiRemap", "Shader.hlsl", b"x")
        self.assertEqual(r.type, "resourceRemapFileAdd")

    def test_bytes_roundTrip(self):
        r = FRB.RemapFileAddResource(f"{ModsRoot}/EiRemap", "Shader.hlsl", b"\x00\x01\xff")
        self.assertEqual(r.bytes, b"\x00\x01\xff")
        r.bytes = b"abc"
        self.assertEqual(r.bytes, b"abc")

    def test_fix_writesBytes_creatingTheFolder(self):
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "Meshes", "Shader.hlsl")
            r = FRB.RemapFileAddResource(folder, path, b"float4 main() : SV_Target { return 0; }")

            self.assertTrue(r.fix())
            with open(path, "rb") as f:
                self.assertEqual(f.read(), b"float4 main() : SV_Target { return 0; }")

    def test_fix_withFixFunc_invokedFromPython(self):
        calls = []

        def fixFunc(self):
            calls.append(self.type)
            return True

        r = FRB.RemapFileAddResource(f"{ModsRoot}/EiRemap", "Shader.hlsl", b"x", type = "resourceFileAdd", fixFunc = fixFunc)
        self.assertTrue(r.fix())
        self.assertEqual(calls, ["resourceFileAdd"])

    def test_stats_countedUnderFileAdd(self):
        stats = FRB.RemapStats()
        r = FRB.RemapFileAddResource(f"{ModsRoot}/EiRemap", "Shader.hlsl", b"x")
        self.assertFalse(r.srcIsFixed(stats))

        stats.fileAdd.addFixed(r.srcPath)
        self.assertTrue(r.srcIsFixed(stats))
        self.assertTrue(r.fixIsFixed(stats))
