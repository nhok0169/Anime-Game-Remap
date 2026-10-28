import os
import struct
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB

IDS = "BLENDINDICES"
WEIGHTS = "BLENDWEIGHT"


def _blendLine(ids, weights):
    return bytes(ids) + bytes(weights)


class WWMIBlendTest(BaseUnitTest):
    """
    Tests for :class:`WWMIBlend` -- a Wuthering Waves mod's ``Blend.buf``, remapped
    """

    def setUp(self):
        self._dir = tempfile.TemporaryDirectory()
        self.folder = self._dir.name

    def tearDown(self):
        self._dir.cleanup()

    def _write(self, name, data):
        path = os.path.join(self.folder, name)
        with open(path, "wb") as f:
            f.write(data)
        return path

    def _read(self, name):
        with open(os.path.join(self.folder, name), "rb") as f:
            return f.read()

    def _position(self, vertices):
        return self._write("Position.buf", b"\0" * (12 * vertices))

    # ================================================
    # ==================== layout ====================

    def test_layoutOf_sizesGiveVerticesAndInfluences(self):
        position = self._position(3)
        blend = self._write("Blend.buf", b"\0" * (3 * 16))

        layout = FRB.WWMIBlend.layoutOf(blend, position)
        self.assertEqual((layout.vertices, layout.influences), (3, 8))

    def test_layoutOf_oddBytesPerVertex_none(self):
        position = self._position(2)
        blend = self._write("Blend.buf", b"\0" * (2 * 3))

        self.assertIsNone(FRB.WWMIBlend.layoutOf(blend, position))

    def test_influencesOf_noPosition_usesDeclaredVertices(self):
        blend = self._write("Blend.buf", b"\0" * (5 * 8))

        self.assertEqual(FRB.WWMIBlend.influencesOf(blend, 5, os.path.join(self.folder, "missing.buf")), 4)
        self.assertEqual(FRB.WWMIBlend.influencesOf(blend, 0, os.path.join(self.folder, "missing.buf"), 6), 6)

    def test_influencesOf_filesWinOverTheDeclaration(self):
        position = self._position(2)
        blend = self._write("Blend.buf", b"\0" * (2 * 16))

        self.assertEqual(FRB.WWMIBlend.influencesOf(blend, 999, position), 8)

    def test_elements_bytePerIdAndWeight(self):
        blend = FRB.CppBufFile(_blendLine([1, 2, 3, 4], [10, 20, 30, 40]), FRB.WWMIBlend.blendElements(4))
        self.assertEqual(blend.bytesPerLine, 8)
        self.assertEqual(blend.rows(IDS, 4), [[1, 2, 3, 4]])
        self.assertEqual(blend.rows(WEIGHTS, 4), [[10, 20, 30, 40]])

        vg = FRB.CppBufFile(struct.pack("<2H", 300, 7), FRB.WWMIBlend.vertexVGElements(2))
        self.assertEqual(vg.rows(IDS, 2), [[300, 7]])

    # ================================================
    # ================ remapping ids =================

    def test_remapThroughVertexVG_remapsTheTrueIdsAndKeepsWeights(self):
        position = self._position(2)
        blend = self._write("Blend.buf", _blendLine([0, 1], [200, 55]) + _blendLine([2, 3], [255, 0]))
        vg = self._write("VertexVG.buf", struct.pack("<4H", 300, 301, 302, 303))
        fixed = os.path.join(self.folder, "Fixed.buf")

        FRB.WWMIBlend.remapThroughVertexVG(blend, fixed, vg, position, {300: 5, 301: 6, 302: 7, 303: 8})

        # the weight-zero slot keeps its id
        self.assertEqual(self._read("Fixed.buf"), _blendLine([5, 6], [200, 55]) + _blendLine([7, 3], [255, 0]))

    def test_remapThroughVertexVG_wrongVertexCount_raises(self):
        position = self._position(2)
        blend = self._write("Blend.buf", _blendLine([0, 1], [1, 1]) * 2)
        vg = self._write("VertexVG.buf", struct.pack("<2H", 1, 2))

        with self.assertRaises(RuntimeError):
            FRB.WWMIBlend.remapThroughVertexVG(blend, os.path.join(self.folder, "F.buf"), vg, position, {})

    def test_componentOfVertex_readsTheDrawRanges(self):
        index = self._write("Index.buf", struct.pack("<6I", 0, 1, 2, 2, 3, 4))

        self.assertEqual(FRB.WWMIBlend.componentOfVertex(index, {0: [(3, 0)], 1: [(3, 3)]}, 6), [0, 0, 1, 1, 1, -1])

    def test_liftLegacy_liftsThroughTheComponentVgMap(self):
        position = self._position(2)
        index = self._write("Index.buf", struct.pack("<6I", 0, 0, 0, 1, 1, 1))
        blend = self._write("Blend.buf", _blendLine([0, 1], [128, 127]) + _blendLine([0, 1], [128, 127]))
        fixed = os.path.join(self.folder, "Fixed.buf")
        legacy = FRB.WWMIBlend.LegacyLift(index, {0: [(3, 0)], 1: [(3, 3)]}, {0: [10, 11], 1: [20, 21]})

        FRB.WWMIBlend.liftLegacy(blend, fixed, position, legacy, {10: 1, 11: 2, 20: 3, 21: 4})

        self.assertEqual(self._read("Fixed.buf"), _blendLine([1, 2], [128, 127]) + _blendLine([3, 4], [128, 127]))

    # ================================================
    # ============ writing a blend remap =============

    def test_writeRemap_localIdsAndTheTwoMaps(self):
        position = self._position(1)
        blend = self._write("Blend.buf", _blendLine([0, 1], [200, 55]))
        fixed = os.path.join(self.folder, "Fixed.buf")
        out = FRB.WWMIBlend.RemapFiles(os.path.join(self.folder, "VG.buf"), os.path.join(self.folder, "Fwd.buf"),
                                       os.path.join(self.folder, "Rev.buf"))

        FRB.WWMIBlend.writeRemap(blend, fixed, "", position, {0: 400, 1: 300}, out)

        # the row's targets in order (300, 400) are locals 0 and 1
        self.assertEqual(self._read("Fixed.buf"), _blendLine([1, 0], [200, 55]))
        self.assertEqual(self._read("VG.buf"), struct.pack("<2H", 400, 300))

        forward = struct.unpack("<512H", self._read("Fwd.buf"))
        reverse = struct.unpack("<512H", self._read("Rev.buf"))
        self.assertEqual(forward[:3], (300, 400, 0))
        self.assertEqual((reverse[300], reverse[400]), (0, 1))

    def test_writeRemap_unaddressableBone_raises(self):
        position = self._position(1)
        blend = self._write("Blend.buf", _blendLine([0], [255]))
        out = FRB.WWMIBlend.RemapFiles(os.path.join(self.folder, "VG.buf"), os.path.join(self.folder, "Fwd.buf"),
                                       os.path.join(self.folder, "Rev.buf"))

        with self.assertRaises(RuntimeError):
            FRB.WWMIBlend.writeRemap(blend, os.path.join(self.folder, "F.buf"), "", position, {0: FRB.WWMIBlend.RemapSize}, out)

    # ================================================
    # ============== widening/narrowing ==============

    def test_widen_padsWithBoneZeroAtWeightZero(self):
        path = self._write("Blend.buf", _blendLine([1, 2], [100, 155]))

        FRB.WWMIBlend.widen(path, 1, 2, 4, 1, True)
        self.assertEqual(self._read("Blend.buf"), _blendLine([1, 2, 0, 0], [100, 155, 0, 0]))

        FRB.WWMIBlend.widen(path, 1, 2, 4, 1, True)              # already wide: left alone
        self.assertEqual(self._read("Blend.buf"), _blendLine([1, 2, 0, 0], [100, 155, 0, 0]))

    def test_narrow_mergesRepeatsAndKeepsTheHeaviest(self):
        # bone 5 twice (50 + 40 = 90), then 100, 60: keeps 100 and 90, scaled to the total of 250
        path = self._write("Blend.buf", _blendLine([5, 7, 5, 9], [50, 100, 40, 60]))

        FRB.WWMIBlend.narrow(path, "", 1, 4, 2)

        ids = list(self._read("Blend.buf")[:2])
        weights = list(self._read("Blend.buf")[2:])
        self.assertEqual(ids, [7, 5])
        self.assertEqual(sum(weights), 250)
        self.assertEqual(weights, [132, 118])

    def test_narrow_wrongSize_raises(self):
        path = self._write("Blend.buf", b"\0" * 6)

        with self.assertRaises(RuntimeError):
            FRB.WWMIBlend.narrow(path, "", 1, 4, 2)


class WWMIShapeKeysTest(BaseUnitTest):
    """
    Tests for :class:`WWMIShapeKeys` -- a Wuthering Waves mod's three shape-key buffers, reordered
    """

    def setUp(self):
        self._dir = tempfile.TemporaryDirectory()
        self.folder = self._dir.name

    def tearDown(self):
        self._dir.cleanup()

    def _files(self, prefix):
        return FRB.WWMIShapeKeys.Files(*[os.path.join(self.folder, prefix + name) for name in ("Offset.buf", "Id.buf", "VertexOffset.buf")])

    def _write(self, files, firsts, ids, offsets):
        for path, data in zip((files.offsets, files.vertexIds, files.vertexOffsets),
                              (struct.pack("<%dI" % len(firsts), *firsts), struct.pack("<%dI" % len(ids), *ids), bytes(offsets))):
            with open(path, "wb") as f:
                f.write(data)

    def _read(self, path):
        with open(path, "rb") as f:
            return f.read()

    def test_layoutOf_slotsEntriesAndWidth(self):
        files = self._files("src")
        self._write(files, [0, 1, 3], [10, 11, 12, 13], b"ABCDEFGH")

        layout = FRB.WWMIShapeKeys.layoutOf(files)
        self.assertEqual((layout.firsts, layout.entries, layout.offsetWidth), ([0, 1, 3], 4, 2))

    def test_checkReorder_outOfRangeKey(self):
        files = self._files("src")
        self._write(files, [0, 1, 3], [10, 11, 12, 13], b"ABCDEFGH")

        self.assertEqual(FRB.WWMIShapeKeys.checkReorder(files, [1, 0]), "")
        self.assertNotEqual(FRB.WWMIShapeKeys.checkReorder(files, [5]), "")
        self.assertNotEqual(FRB.WWMIShapeKeys.checkReorder(files, [0, 1, 2, 0]), "")

    def test_reorder_swapsSlotsAndZeroesAnUntakenKey(self):
        # slot 0: entry 0; slot 1: entries 1-2; slot 2: entry 3 (past the order, kept in place)
        files = self._files("src")
        out = self._files("out")
        self._write(files, [0, 1, 3], [10, 11, 12, 13], b"AABBCCDD")

        # slot 0 takes key 1, slot 1 takes none, so key 0 is kept after slot 1 with zeroed offsets
        FRB.WWMIShapeKeys.reorder(files, [1, -1], out)

        self.assertEqual(self._read(out.offsets), struct.pack("<3I", 0, 2, 3))
        self.assertEqual(self._read(out.vertexIds), struct.pack("<4I", 11, 12, 10, 13))
        self.assertEqual(self._read(out.vertexOffsets), b"BBCC" + b"\0\0" + b"DD")

