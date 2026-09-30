import os
import struct
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


# Two sheets facing +z: the INNER one at z = 0 (vertices 0-3) and, 5 cm along its normal, a larger OUTER one at
# z = 0.05 (vertices 4-7) that covers it. All on vertex group 0.
def twoSheets():
    positions, normals, inner, outer = [], [], [], []
    for tris, (x, y, z, size) in ((inner, (-0.5, -0.5, 0.0, 1.0)), (outer, (-0.6, -0.6, 0.05, 1.2))):
        base = len(positions)
        positions += [(x, y, z), (x + size, y, z), (x + size, y + size, z), (x, y + size, z)]
        normals += [(0.0, 0.0, 1.0)] * 4
        tris += [(base, base + 1, base + 2), (base, base + 2, base + 3)]
    return positions, normals, inner, outer


class InnerLayerOutlineTest(BaseUnitTest):
    """
    Tests for :class:`InnerLayerOutline` -- which vertices of a mesh's inner layers draw no outline (Yaoyao5's hair on
    YaoyaoBamboo). The per-triangle rule and the facing-axis test are pinned by ``core/tests/InnerLayerOutline_test.cpp``,
    which fails against a per-vertex build and against a one-corner build; here, the binding and the split group
    """

    def test_find_coveredSheetLosesItsOutline_theOneOnTopKeepsIt(self):
        positions, normals, inner, outer = twoSheets()
        rule = FRB.InnerLayerOutline(facingAxis = False)
        result = rule.find(positions, normals, [inner, outer], [inner, outer])
        self.assertEqual(result, [True] * 4 + [False] * 4)
        self.assertEqual(rule.covered(positions, normals, [inner, outer], [inner, outer]), [True] * 4 + [False] * 4)

    def test_find_reachShorterThanTheGap_coversNothing(self):
        positions, normals, inner, outer = twoSheets()
        rule = FRB.InnerLayerOutline(reach = 0.03, facingAxis = False)
        self.assertEqual(rule.find(positions, normals, [inner, outer], [inner, outer]), [False] * 8)

    def test_find_onlyTheTargetsAreTouched(self):
        positions, normals, inner, outer = twoSheets()
        rule = FRB.InnerLayerOutline(facingAxis = False)
        self.assertEqual(rule.find(positions, normals, [inner, outer], [outer]), [False] * 8)

    def test_defaults(self):
        rule = FRB.InnerLayerOutline()
        self.assertAlmostEqual(rule.reach, 0.1, places = 6)
        self.assertTrue(rule.facingAxis)
        self.assertAlmostEqual(rule.facingCos, 0.2, places = 6)

    def test_readPositions(self):
        lines = [struct.pack("<10f", 1, 2, 3, 0, 0, 1, 0, 0, 1, -1), struct.pack("<10f", 4, 5, 6, 1, 0, 0, 1, 0, 0, -1)]
        positions, normals = FRB.InnerLayerOutline.readPositions(b"".join(lines), 40)
        self.assertEqual([tuple(p) for p in positions], [(1, 2, 3), (4, 5, 6)])
        self.assertEqual([tuple(n) for n in normals], [(0, 0, 1), (1, 0, 0)])
        with self.assertRaises(ValueError):
            FRB.InnerLayerOutline.readPositions(b"".join(lines), 12)

    def test_componentFixerConfig_innerOutline_offByDefault(self):
        # empty by default: no component's texcoord moves, as every earlier config was confirmed with
        component = FRB.GIMIComponentFixerConfig.Component()
        self.assertEqual(component.innerOutlineObjs, [])
        self.assertAlmostEqual(component.innerOutlineReach, 0.1, places = 6)
        self.assertTrue(component.innerOutlineFacingAxis)
        component.innerOutlineObjs = ["head"]
        component.innerOutlineReach = 0.05
        component.innerOutlineFacingAxis = False
        self.assertEqual(component.innerOutlineObjs, ["head"])
        self.assertAlmostEqual(component.innerOutlineReach, 0.05, places = 6)
        self.assertFalse(component.innerOutlineFacingAxis)


class VGSplitGroupInnerOutlineTest(BaseUnitTest):
    """
    :attr:`VGSplitGroupResource.innerOutline` -- the written ``Texcoord.buf`` loses the vertex colour's ALPHA (byte 3)
    on the inner layer and nothing else
    """

    def setUp(self):
        super().setUp()
        self._folder = tempfile.mkdtemp()
        positions, normals, inner, outer = twoSheets()
        self._ibs = [inner + outer]
        with open(os.path.join(self._folder, "Blend.buf"), "wb") as f:
            f.write(FRB.VGComponentSplit.encodeBlend([[1, 0, 0, 0]] * 8, [[0, 0, 0, 0]] * 8))
        with open(os.path.join(self._folder, "Head.ib"), "wb") as f:
            f.write(FRB.VGComponentSplit.encodeIb(self._ibs[0]))
        with open(os.path.join(self._folder, "Position.buf"), "wb") as f:
            f.write(b"".join(struct.pack("<10f", *p, *n, *n, -1.0) for p, n in zip(positions, normals)))
        # 12-byte lines: vertex colour (255, 128, 128, 64), then the UV
        self._texcoord = b"".join(bytes([255, 128, 128, 64]) + struct.pack("<2f", i / 8, 0.5) for i in range(8))
        with open(os.path.join(self._folder, "Texcoord.buf"), "wb") as f:
            f.write(self._texcoord)

    def _fix(self, **props):
        group = FRB.VGSplitGroupResource("group", component = "Main", specs = [FRB.VGComponentSpec("Main", {0: 0})],
                                         ibPaths = [os.path.join(self._folder, "Head.ib")])
        for key, val in props.items():
            setattr(group, key, val)
        group.addResource((0, "", "blend"), FRB.RemapIniFixResource("blend", self._folder, "Blend.buf", "BlendFixed.buf"))
        group.addResource((0, "", "position"), FRB.RemapIniFixResource("position", self._folder, "Position.buf", "PositionFixed.buf"))
        group.addResource((0, "", "texcoord"), FRB.RemapIniFixResource("texcoord", self._folder, "Texcoord.buf", "TexcoordFixed.buf"))
        group.addResource((0, "", "ib"), FRB.RemapIniFixResource("buf", self._folder, "Head.ib", "HeadFixed.ib"))
        self.assertTrue(group.fix())
        with open(os.path.join(self._folder, "TexcoordFixed.buf"), "rb") as f:
            return f.read()

    def test_innerLayer_alphaZero_everythingElseKept(self):
        out = self._fix(innerOutline = FRB.InnerLayerOutline(facingAxis = False))
        self.assertEqual(len(out), len(self._texcoord))
        expected = bytearray(self._texcoord)
        for v in range(4):
            expected[12 * v + 3] = 0
        self.assertEqual(out, bytes(expected))

    def test_unset_changesNothing(self):
        self.assertEqual(self._fix(), self._texcoord)

    def test_innerOutlineIbs_notTheTarget_changesNothing(self):
        # index 1 names no buffer of this group's one: nothing is asked about
        self.assertEqual(self._fix(innerOutline = FRB.InnerLayerOutline(facingAxis = False), innerOutlineIbs = [1]), self._texcoord)

    def test_properties_roundTrip(self):
        group = FRB.VGSplitGroupResource("group", component = "Main", specs = [FRB.VGComponentSpec("Main", {0: 0})])
        self.assertIsNone(group.innerOutline)
        self.assertEqual(group.innerOutlineIbs, [])
        group.innerOutline = FRB.InnerLayerOutline(reach = 0.2)
        group.innerOutlineIbs = [0, 2]
        self.assertAlmostEqual(group.innerOutline.reach, 0.2, places = 6)
        self.assertEqual(group.innerOutlineIbs, [0, 2])
        group.innerOutline = None
        self.assertIsNone(group.innerOutline)
