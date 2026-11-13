import os
import struct
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _ddsHeader(width, height, mips, dxgi = None, bits = 0, red = 0):
    """The 148 header bytes of a .dds file: a DX10 one when 'dxgi' is given, else a legacy one"""
    h = bytearray(148)
    h[0:4] = b"DDS "
    struct.pack_into("<I", h, 12, height)
    struct.pack_into("<I", h, 16, width)
    struct.pack_into("<I", h, 28, mips)
    if dxgi is not None:
        h[84:88] = b"DX10"
        struct.pack_into("<I", h, 128, dxgi)
    else:
        struct.pack_into("<I", h, 88, bits)
        struct.pack_into("<I", h, 92, red)
    return bytes(h)


class _WithFolder(BaseUnitTest):
    def setUp(self):
        self._dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self._dir.cleanup()

    def _write(self, name, data):
        path = os.path.join(self._dir.name, name)
        with open(path, "wb") as f:
            f.write(data)
        return path


class DDSHeaderTest(_WithFolder):
    """
    Tests for :meth:`CppTextureFile.readDDSHeader`
    """

    def test_dx10(self):
        header = FRB.CppTextureFile.readDDSHeader(self._write("a.dds", _ddsHeader(512, 256, 10, dxgi = 98)))
        self.assertEqual((header.width, header.height, header.mipCount, header.dxgiFormat), (512, 256, 10, 98))

    def test_legacyFormats(self):
        r8 = FRB.CppTextureFile.readDDSHeader(self._write("r8.dds", _ddsHeader(4, 4, 0, bits = 8, red = 0xff)))
        bgra = FRB.CppTextureFile.readDDSHeader(self._write("bgra.dds", _ddsHeader(4, 4, 1, bits = 32, red = 0x00ff0000)))
        self.assertEqual((r8.dxgiFormat, r8.mipCount), (61, 1))
        self.assertEqual(bgra.dxgiFormat, 87)

    def test_notADDS_none(self):
        self.assertIsNone(FRB.CppTextureFile.readDDSHeader(self._write("x.dds", b"PNG" + b"\0" * 200)))
        self.assertIsNone(FRB.CppTextureFile.readDDSHeader(self._write("short.dds", b"DDS ")))


class RabbitFXTest(_WithFolder):
    """
    Tests for :class:`RabbitFX`
    """

    def test_textureTag(self):
        tag = lambda name, data: FRB.RabbitFX.textureTag(self._write(name, data))
        self.assertEqual(tag("bc7Mips.dds", _ddsHeader(1024, 1024, 11, dxgi = 98)), 3)
        self.assertEqual(tag("bc7Small.dds", _ddsHeader(512, 512, 1, dxgi = 98)), 6)
        self.assertEqual(tag("r8.dds", _ddsHeader(256, 256, 9, dxgi = 61)), 4)
        self.assertEqual(tag("bgraPixel.dds", _ddsHeader(1, 1, 1, dxgi = 87)), 5)
        self.assertEqual(tag("notSquare.dds", _ddsHeader(512, 256, 1, dxgi = 98)), 0)

    def test_routingFlags(self):
        self.assertEqual(FRB.RabbitFX.routingFlags([3, 3, 4, 0]), [1, 1, 0, 1])
        self.assertEqual(FRB.RabbitFX.routingFlags([6, 0, 6, 0]), [1, 0, 1, 0])
        self.assertEqual(FRB.RabbitFX.routingFlags([3, 5, 3, 0]), [1, 1, 1, 0])
        self.assertEqual(FRB.RabbitFX.routingFlags([3, 5, 0, 3]), [1, 0, 1, 0])

    def test_flagKeys(self):
        self.assertEqual(list(FRB.RabbitFX.FlagKeys), ["x187", "y187", "z187", "w187"])

    def test_names_and_isOwnKVP(self):
        self.assertTrue(FRB.RabbitFX.names("run = CommandList\\RABBITFX\\SetTextures"))
        self.assertFalse(FRB.RabbitFX.names("ps-t0 = ResourceDiffuse"))
        self.assertTrue(FRB.RabbitFX.isOwnKVP("Resource\\RabbitFX\\Diffuse", "ref ResourceDiffuse"))
        self.assertTrue(FRB.RabbitFX.isOwnKVP("run", FRB.IniKeywords.RabbitFXSetTexturesPath))
        self.assertFalse(FRB.RabbitFX.isOwnKVP("ps-t0", "ResourceRabbitFXLike"))


class DrawUVCoverageTest(BaseUnitTest):
    """
    Tests for :class:`DrawUVCoverage`
    """

    def test_texelOf_wrapsAndClamps(self):
        self.assertEqual(FRB.DrawUVCoverage.texelOf(0.5, 0.25, 4, 4), (2, 1))
        self.assertEqual(FRB.DrawUVCoverage.texelOf(1.5, -0.25, 4, 4), (2, 3))
        self.assertEqual(FRB.DrawUVCoverage.texelOf(float("nan"), 0.999999, 4, 4), (0, 3))

    def test_count_distinctVerticesOfTheRange(self):
        # triangles (0, 1, 2) and (2, 3, 0); vertices 0 and 2 sit in the left half of a 2-wide texture
        coverage = FRB.DrawUVCoverage([0, 1, 2, 2, 3, 0], [0.1, 0.9, 0.2, 0.8], [0.5, 0.5, 0.5, 0.5], 4)
        left = lambda x, y: x == 0

        whole = coverage.count(0, 6, 2, 2, left)
        self.assertEqual((whole.vertices, whole.matched), (4, 2))

        second = coverage.count(3, 3, 2, 2, left)
        self.assertEqual((second.vertices, second.matched), (3, 2))

    def test_count_pastTheEnd_andOutOfRangeVertices(self):
        coverage = FRB.DrawUVCoverage([0, 7, 1], [0.0, 0.0], [0.0, 0.0], 2)
        counted = coverage.count(0, 10, 1, 1, lambda x, y: True)
        self.assertEqual((counted.vertices, counted.matched), (2, 2))

    def test_coveredRanges_keepsTheRangesPastTheShare(self):
        # the two triangles of test_count_distinctVerticesOfTheRange: the whole mesh has 2 of 4 vertices on the left,
        # each triangle 2 of 3
        coverage = FRB.DrawUVCoverage([0, 1, 2, 2, 3, 0], [0.1, 0.9, 0.2, 0.8], [0.5, 0.5, 0.5, 0.5], 4)
        left = lambda x, y: x == 0
        ranges = [(6, 0), (3, 3), (3, 0)]
        self.assertEqual(coverage.coveredRanges(ranges, 2, 2, left, 0.5), [(3, 3), (3, 0)])     # more than half, not half
        self.assertEqual(coverage.coveredRanges(ranges, 2, 2, left, 0.4), [(6, 0), (3, 3), (3, 0)])
        self.assertEqual(coverage.coveredRanges(ranges, 2, 2, left, 0.7), [])
        self.assertEqual(coverage.coveredRanges([(0, 0)], 2, 2, left, 0.0), [])        # a range of no vertices

    def test_triangles_skipTrianglesOutsideTheMesh(self):
        # the second triangle names vertex 9 of a 4-vertex mesh, the third runs past the index list
        coverage = FRB.DrawUVCoverage([0, 1, 2, 1, 9, 3, 2, 3], [0.0] * 4, [0.0] * 4, 4)
        self.assertEqual(coverage.triangles([(9, 0)]), [[0, 1, 2]])
        self.assertEqual(coverage.triangles([(3, 0), (3, 0)]), [[0, 1, 2], [0, 1, 2]])

    def test_islands_joinTrianglesThatShareAVertex(self):
        triangles = [(0, 1, 2), (5, 6, 7), (2, 3, 4), (7, 8, 9), (10, 11, 12)]
        self.assertEqual(FRB.DrawUVCoverage.islands(triangles), [0, 1, 0, 1, 2])
        # a later triangle joining two pieces puts both in one
        self.assertEqual(FRB.DrawUVCoverage.islands([(0, 1, 2), (3, 4, 5), (2, 3, 6)]), [0, 0, 0])

    def test_region_coversTheAcceptedTriangles_dropsShared_andGrowsIntoTheGutter(self):
        # the left half of a 4 x 4 texture as two triangles, and the right half's first column as two more
        us = [0.0, 0.5, 0.0, 0.5, 0.5, 0.75, 0.5, 0.75]
        vs = [0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0]
        coverage = FRB.DrawUVCoverage([], us, vs, 8)
        left = [(0, 1, 2), (1, 3, 2)]
        both = left + [(4, 5, 6), (5, 7, 6)]
        rows = lambda region: ["".join("x" if region[y * 4 + x] else "." for x in range(4)) for y in range(4)]

        self.assertEqual(rows(coverage.region(left, [True, True], 4, 4)), ["xx.."] * 4)
        self.assertEqual(rows(coverage.region(both, [True, True, False, False], 4, 4)), ["xx.."] * 4)
        # grown, but never into the column the rejected triangle paints
        self.assertEqual(rows(coverage.region(both, [True, True, False, False], 4, 4, 1)), ["xx.."] * 4)
        self.assertEqual(rows(coverage.region(left, [True, True], 4, 4, 1)), ["xxx."] * 4)
        # one triangle of the two: the texels on the diagonal both cover belong to neither
        self.assertEqual(rows(coverage.region(left, [True, False], 4, 4)), ["xx..", "x...", "x...", "...."])

    def test_region_wrapsATriangleIntoTheTexture(self):
        coverage = FRB.DrawUVCoverage([], [1.0, 1.5, 1.0, 1.5], [2.0, 2.0, 3.0, 3.0], 4)
        region = coverage.region([(0, 1, 2), (1, 3, 2)], [True, True], 4, 4)
        self.assertEqual(region, [x < 2 for y in range(4) for x in range(4)])

    def test_ofMesh_readsTheIndexAndTheFirstUV(self):
        folder = tempfile.mkdtemp()
        index, texcoord = os.path.join(folder, "Index.buf"), os.path.join(folder, "Texcoord.buf")
        with open(index, "wb") as file:
            file.write(struct.pack("<6I", 0, 1, 2, 2, 3, 0))
        # 8 bytes a vertex: the first UV, then a second channel the coverage does not read
        with open(texcoord, "wb") as file:
            for u in (0.125, 0.875, 0.25, 0.75):
                file.write(struct.pack("<4e", u, 0.5, 0.9, 0.9))

        coverage = FRB.DrawUVCoverage.ofMesh(index, texcoord, 4)
        counted = coverage.count(0, 6, 2, 2, lambda x, y: x == 0)
        self.assertEqual((counted.vertices, counted.matched), (4, 2))

        with self.assertRaises(RuntimeError):
            FRB.DrawUVCoverage.ofMesh(index, texcoord, 3)        # 32 bytes are not 3 whole vertices


class WWMIKeywordsTest(BaseUnitTest):
    """
    The WWMI and RabbitFX names :class:`IniKeywords` spells for the fixers
    """

    def test_values(self):
        self.assertEqual(FRB.IniKeywords.WWMINamespace, "$\\WWMIv1")
        self.assertEqual(FRB.IniKeywords.WWMIVgOffset, "$\\WWMIv1\\vg_offset")
        self.assertEqual(FRB.IniKeywords.WWMIShapeKeyDispatchSizeYOriginalBatch, "$\\WWMIv1\\shapekey_dispatch_size_y_original_batch")
        self.assertEqual(FRB.IniKeywords.WWMIObjectDetected, "$object_detected")
        self.assertEqual(FRB.IniKeywords.WWMIBlendBuffer, "ResourceBlendBuffer")
        self.assertEqual(FRB.IniKeywords.WWMISkeletonMerger, "CustomShader\\WWMIv1\\SkeletonMerger")
        self.assertEqual(FRB.IniKeywords.RabbitFXSetTexturesPath, "CommandList\\RabbitFX\\SetTextures")
        self.assertEqual((FRB.IniKeywords.Global, FRB.IniKeywords.Local, FRB.IniKeywords.Post, FRB.IniKeywords.Skip),
                         ("global", "local", "post", "skip"))
