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

    # ================================================
    # ======= one component's own mesh blend =========

    def test_reverseOf_theRowsTargetsInOrder_everythingElseZero(self):
        reverse = FRB.WWMIBlend.reverseOf({0: 400, 1: 300, 2: 300, 3: -1, 4: 600})
        self.assertEqual(len(reverse), 512)
        self.assertEqual((reverse[300], reverse[400]), (0, 1))
        self.assertEqual(sum(reverse), 1)                   # 600 cannot be held, and nothing else is named

    def test_componentBlend_liftsThroughTheVgMapAndTheRow_toLocalIds(self):
        position = self._position(2)
        # ids start at the component's vg_offset (10): local 0 and 1, then 1 alone
        blend = self._write("Own.buf", _blendLine([10, 11], [200, 55]) + _blendLine([11, 10], [255, 0]))
        vgMap = [7, 9]                                      # local -> merged source bone
        row = {7: 400, 9: 300}

        data, unmapped = FRB.WWMIBlend.componentBlend(blend, position, 10, vgMap, row, 2)
        # the row's targets in order (300, 400) are locals 0 and 1; a weight-0 slot reads 0
        self.assertEqual(data, _blendLine([1, 0], [200, 55]) + _blendLine([0, 0], [255, 0]))
        self.assertEqual(unmapped, [])

    def test_componentBlend_widensToTheTarget_andReportsWhatItCannotPlace(self):
        position = self._position(1)
        blend = self._write("Own.buf", _blendLine([5, 9], [128, 127]))
        data, unmapped = FRB.WWMIBlend.componentBlend(blend, position, 5, [3], {3: 20}, 4)
        # 9 - 5 = local 4, past the vg map: local 0, its weight kept; two empty slots after
        self.assertEqual(data, _blendLine([0, 0, 0, 0], [128, 127, 0, 0]))
        self.assertEqual(unmapped, [4])

    def test_componentBlend_buffersThatDisagree_raise(self):
        position = self._position(2)
        blend = self._write("Own.buf", b"\0" * 3)
        with self.assertRaises(RuntimeError):
            FRB.WWMIBlend.componentBlend(blend, position, 0, [], {}, 4)

    def test_writeRemap_unaddressableBone_raises(self):
        position = self._position(1)
        blend = self._write("Blend.buf", _blendLine([0], [255]))
        out = FRB.WWMIBlend.RemapFiles(os.path.join(self.folder, "VG.buf"), os.path.join(self.folder, "Fwd.buf"),
                                       os.path.join(self.folder, "Rev.buf"))

        with self.assertRaises(RuntimeError):
            FRB.WWMIBlend.writeRemap(blend, os.path.join(self.folder, "F.buf"), "", position, {0: FRB.WWMIBlend.RemapSize}, out)

    # ================================================
    # ============== widening/narrowing ==============

    # A WuWa blend at 'influences' slots, through BlendFile.setInfluences
    def _blend(self, data, influences):
        return FRB.BlendFile(data, FRB.WWMIBlend.blendElements(influences))

    def test_setInfluences_wider_padsWithBoneZeroAtWeightZero(self):
        blend = self._blend(_blendLine([1, 2], [100, 155]), 2)

        kept = blend.setInfluences(4)

        self.assertEqual(bytes(blend.data), _blendLine([1, 2, 0, 0], [100, 155, 0, 0]))
        self.assertEqual(kept, [[0, 1, None, None]])
        self.assertEqual(blend.influences(), 4)

    def test_setInfluences_sameWidth_leftAlone(self):
        blend = self._blend(_blendLine([1, 2, 0, 0], [100, 155, 0, 0]), 4)

        self.assertEqual(blend.setInfluences(4), [[0, 1, 2, 3]])
        self.assertEqual(bytes(blend.data), _blendLine([1, 2, 0, 0], [100, 155, 0, 0]))

    def test_setInfluences_narrower_mergesRepeatsAndKeepsTheHeaviest(self):
        # bone 5 twice (50 + 40 = 90), then 100, 60: keeps 100 and 90, scaled to the total of 250
        blend = self._blend(_blendLine([5, 7, 5, 9], [50, 100, 40, 60]), 4)

        kept = blend.setInfluences(2)

        ids = list(bytes(blend.data)[:2])
        weights = list(bytes(blend.data)[2:])
        self.assertEqual(ids, [7, 5])
        self.assertEqual(sum(weights), 250)
        self.assertEqual(weights, [132, 118])
        self.assertEqual(kept, [[1, 0]])

    def test_setInfluences_narrower_bonesFromBesideTheBlend(self):
        # the blend's own ids all differ, but the ids beside it name bone 3 twice: those merge
        blend = self._blend(_blendLine([1, 2, 3, 4], [50, 100, 40, 60]), 4)

        blend.setInfluences(2, [[3, 7, 3, 9]])

        self.assertEqual(list(bytes(blend.data)), [2, 1, 132, 118])

    def test_setInfluences_aMergedSum_cappedAtTheWeightsLargestValue(self):
        blend = self._blend(_blendLine([5, 5, 7, 9], [200, 200, 0, 0]), 4)

        blend.setInfluences(2)

        # 200 + 200 caps at 255 on the first slot of bone 5; the rest tie at 0 and keep their order
        self.assertEqual(list(bytes(blend.data)), [5, 5, 255, 0])

    def test_setInfluences_floatWeights_scaledExactly(self):
        # the GI layout: four float weights, then four int indices
        line = struct.pack("<4f4i", 0.5, 0.25, 0.15, 0.1, 10, 11, 12, 13)
        blend = FRB.BlendFile(line)

        blend.setInfluences(2)

        values = struct.unpack("<2f2i", bytes(blend.data))
        self.assertAlmostEqual(values[0], 0.5 / 0.75, places = 6)
        self.assertAlmostEqual(values[1], 0.25 / 0.75, places = 6)
        self.assertEqual(values[2:], (10, 11))

    def test_keepSlots_followsTheKeptSlots_newSlotsZero(self):
        ids = struct.pack("<4H", 300, 400, 500, 600)

        self.assertEqual(FRB.BlendFile.keepSlots(ids, 2, 4, [[3, 0]]), struct.pack("<2H", 600, 300))
        self.assertEqual(FRB.BlendFile.keepSlots(struct.pack("<2H", 300, 400), 2, 2, [[0, 1, None]]),
                         struct.pack("<3H", 300, 400, 0))

    def test_keepSlots_wrongSize_raises(self):
        with self.assertRaises(RuntimeError):
            FRB.BlendFile.keepSlots(b"\0" * 6, 2, 4, [[0, 1]])


class SkeletonNumberingProbeTest(BaseUnitTest):
    """
    Tests for :class:`SkeletonNumberingProbe` -- which numbering a mod's blend gives its bone ids in
    """

    def setUp(self):
        self._dir = tempfile.TemporaryDirectory()
        self.folder = self._dir.name

    def tearDown(self):
        self._dir.cleanup()

    # 'vertices' vertices at the origin, each wholly on bone 'bone' (4 influences a vertex)
    def _write(self, vertices, bone):
        position = os.path.join(self.folder, "Position.buf")
        blend = os.path.join(self.folder, "Blend.buf")
        with open(position, "wb") as f:
            f.write(struct.pack("<%df" % (3 * vertices), *([0.0] * (3 * vertices))))
        with open(blend, "wb") as f:
            f.write(bytes([bone, 0, 0, 0, 255, 0, 0, 0]) * vertices)
        return position, blend

    def _numberings(self):
        # the second numbering calls bone 1 what the reference calls bone 2
        N = FRB.SkeletonNumberingProbe.Numbering
        return [N("1.0", {}), N("2.0", {1: 2})]

    def test_probe_picksTheNumberingPuttingVerticesNearTheirBone(self):
        position, blend = self._write(30, 1)
        centroids = {1: (0.0, 0.0, 0.0), 2: (10.0, 0.0, 0.0)}

        result = FRB.SkeletonNumberingProbe.probe(position, blend, None, self._numberings(), centroids)

        self.assertEqual(result.best, 0)
        self.assertEqual(result.vertices, 30)
        self.assertAlmostEqual(result.medians[0], 0.0)
        self.assertAlmostEqual(result.medians[1], 10.0)

    def test_probe_tooFewVerticesOnDisputedBones_none(self):
        position, blend = self._write(FRB.SkeletonNumberingProbe.MinVertices - 1, 1)
        centroids = {1: (0.0, 0.0, 0.0), 2: (10.0, 0.0, 0.0)}
        self.assertIsNone(FRB.SkeletonNumberingProbe.probe(position, blend, None, self._numberings(), centroids))

    def test_probe_oneNumbering_none(self):
        position, blend = self._write(30, 1)
        self.assertIsNone(FRB.SkeletonNumberingProbe.probe(position, blend, None, self._numberings()[:1], {1: (0.0, 0.0, 0.0)}))


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

    def test_reordered_isWhatReorderWrites(self):
        files = self._files("src")
        out = self._files("out")
        self._write(files, [0, 1, 3], [10, 11, 12, 13], b"AABBCCDD")

        bytesOut = FRB.WWMIShapeKeys.reordered(files, [1, -1])
        FRB.WWMIShapeKeys.reorder(files, [1, -1], out)

        self.assertEqual(bytesOut.offsets, self._read(out.offsets))
        self.assertEqual(bytesOut.vertexIds, self._read(out.vertexIds))
        self.assertEqual(bytesOut.vertexOffsets, self._read(out.vertexOffsets))

