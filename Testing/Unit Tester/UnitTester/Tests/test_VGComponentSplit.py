import os
import struct
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


# A 6-vertex mod: vertices 0-2 on the Body's groups {0, 1}, 3-5 on the Bang's group 9 (5 also
# carries group 1). Two index buffers: the "head" one holds the body triangle (0,1,2) and the seam
# triangle (2,3,4); the "body" one holds the bang triangle (3,4,5)
Weights = [[1, 0, 0, 0], [0.5, 0.5, 0, 0], [1, 0, 0, 0], [1, 0, 0, 0], [1, 0, 0, 0], [0.7, 0.3, 0, 0]]
Indices = [[0, 0, 0, 0], [0, 1, 0, 0], [1, 0, 0, 0], [9, 0, 0, 0], [9, 0, 0, 0], [9, 1, 0, 0]]
Ibs = [[[0, 1, 2], [2, 3, 4]], [[3, 4, 5]]]


def makeSpecs():
    return [FRB.VGComponentSpec("Body", {0: 10, 1: 11}),
            FRB.VGComponentSpec("Bang", {9: 0}, secondary = {1: 3}, negativeIndex = True)]


class VGComponentSplitTest(BaseUnitTest):
    """
    Tests for :class:`VGComponentSplit` -- the joint split of a mod's buffers across a target's components
    """

    def setUp(self):
        super().setUp()
        self._split = FRB.VGComponentSplit(Weights, Indices, Ibs, makeSpecs())

    # ================ VGComponentSpec ===============

    def test_spec_remapFromDictOrVGRemap(self):
        fromDict = FRB.VGComponentSpec("A", {1: 2})
        fromRemap = FRB.VGComponentSpec("A", FRB.VGRemap({1: 2}))
        self.assertEqual(fromDict.remap.remap, {1: 2})
        self.assertEqual(fromRemap.remap.remap, {1: 2})
        self.assertFalse(fromDict.negativeIndex)
        self.assertEqual(fromDict.secondary, {})

    # ================ negative index ================

    def test_negativeIndex_sentinelsLivenessAndTrimming(self):
        bang = self._split.split("Bang")

        # every vertex, with foreign bones as -index-1 and the secondary bone honoured only where anchored
        self.assertEqual(bang.vertices, [0, 1, 2, 3, 4, 5])
        self.assertEqual(bang.indices, [[-1, 0, 0, 0], [-1, -2, 0, 0], [-2, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 3, 0, 0]])
        self.assertEqual(bang.weights, Weights)
        self.assertEqual(bang.live, [False, False, False, True, True, True])

        # trimmed to the fully-live triangles, in the mod's own numbering
        self.assertEqual(bang.ibs, [[], [[3, 4, 5]]])
        self.assertEqual(bang.stats.trianglesKept, [0, 1])
        self.assertEqual(bang.stats.trianglesDropped, [2, 0])
        self.assertEqual(bang.stats.sentinels, 4)
        self.assertEqual(bang.keptTriangleIds, [[], [0]])

    # ================ graph cut (fill) ==============

    def test_fill_takesWhatNegativeLeavesAndSkinsOrphans(self):
        body = self._split.split("Body")

        # the bang triangle is excluded (all corners live in the Bang); the seam triangle is the Body's
        # (2 of 3 corners owned) and drags vertices 3 and 4 in
        self.assertEqual(body.vertices, [0, 1, 2, 3, 4])
        self.assertEqual(body.ibs, [[[0, 1, 2], [2, 3, 4]], []])
        self.assertEqual(body.stats.trianglesKept, [2, 0])
        self.assertEqual(body.stats.trianglesDropped, [0, 1])
        self.assertEqual(body.keptTriangleIds, [[0, 1], []])

        # weights renormalised in the Body's bones; the two orphans skinned to their neighbour's bone 11
        self.assertEqual(body.indices, [[10, 0, 0, 0], [10, 11, 0, 0], [11, 0, 0, 0], [11, 0, 0, 0], [11, 0, 0, 0]])
        self.assertEqual([[round(w, 6) for w in row] for row in body.weights],
                         [[1, 0, 0, 0], [0.5, 0.5, 0, 0], [1, 0, 0, 0], [1, 0, 0, 0], [1, 0, 0, 0]])
        self.assertEqual(body.stats.neighbourSkinned, 2)
        self.assertEqual(body.stats.keptVertices, 5)
        self.assertEqual(body.live, [])

    def test_keptTriangleIds_sourceIndexOfEachKeptTriangle(self):
        # The Bang's triangle sits in the MIDDLE of one index buffer here, so what the Body keeps is not
        # a prefix: its ids skip 1. A mod's own `drawindexed = <count>, <start>` ranges are remapped
        # through exactly these (the component fixer's DrawRangeRemap), and a split that removed from
        # the middle is the case where the mod's own numbers would otherwise draw the wrong triangles.
        split = FRB.VGComponentSplit(Weights, Indices, [[[0, 1, 2], [3, 4, 5], [2, 3, 4]]], makeSpecs())
        body, bang = split.split("Body"), split.split("Bang")

        self.assertEqual(body.keptTriangleIds, [[0, 2]])
        self.assertEqual(bang.keptTriangleIds, [[1]])
        self.assertEqual(len(body.keptTriangleIds[0]), body.stats.trianglesKept[0])
        self.assertEqual(len(bang.keptTriangleIds[0]), bang.stats.trianglesKept[0])

    # ================ seams between two cut components ================

    # A strip of four triangles over six vertices: 0-1 wholly on the Main's group 0, 4-5 wholly on the Coat's
    # group 1, and 2-3 on both (0.4 Main, 0.6 Coat) -- one surface blended across two components, the shape
    # of a cape weighted to the spine and to the coat chains at once
    SeamWeights = [[1, 0, 0, 0], [1, 0, 0, 0], [0.4, 0.6, 0, 0], [0.4, 0.6, 0, 0], [1, 0, 0, 0], [1, 0, 0, 0]]
    SeamIndices = [[0, 0, 0, 0], [0, 0, 0, 0], [0, 1, 0, 0], [0, 1, 0, 0], [1, 0, 0, 0], [1, 0, 0, 0]]
    SeamIbs = [[[0, 1, 2], [1, 2, 3], [2, 3, 4], [3, 4, 5]]]

    def _seam(self, main = None, coat = None):
        main = main or {}; coat = coat or {}
        specs = [FRB.VGComponentSpec("", {0: 10}, **main), FRB.VGComponentSpec("Coat", {1: 20}, **coat)]
        split = FRB.VGComponentSplit(self.SeamWeights, self.SeamIndices, self.SeamIbs, specs)
        return split.split(""), split.split("Coat")

    def test_seam_plainMajority(self):
        # the mixed vertices are the Coat's (0.6 > 0.4), so the seam runs through the middle of the blend
        main, coat = self._seam()
        self.assertEqual(main.keptTriangleIds, [[0]])
        self.assertEqual(coat.keptTriangleIds, [[1, 2, 3]])
        self.assertEqual(main.stats.overlapTriangles, 0)

    def test_seam_claimShare_movesTheSeamToCleanWeights(self):
        # a Coat that claims only what is 90% its own leaves the mixed vertices to the Main
        main, coat = self._seam(coat = {"claimShare": 0.9})
        self.assertEqual(main.keptTriangleIds, [[0, 1, 2]])
        self.assertEqual(coat.keptTriangleIds, [[3]])

        # and a share nothing else can claim still falls back to the majority
        spec = FRB.VGComponentSpec("Coat", {1: 20}, claimShare = 0.9)
        self.assertAlmostEqual(spec.claimShare, 0.9)
        self.assertEqual(FRB.VGComponentSpec("A", {1: 2}).claimShare, 0.0)

    def test_seam_secondaryOnACut_keepsTheForeignWeightOnAStandIn(self):
        # without a stand-in the Main drops vertex 2's Coat weight and renormalises the rest to 1
        main, _ = self._seam()
        row = main.vertices.index(2)
        self.assertEqual(main.indices[row][0], 10)
        self.assertAlmostEqual(main.weights[row][0], 1.0, places = 5)

        # with one, the weight stays -- on the stand-in bone, unrenormalised -- and ownership does not move
        main, coat = self._seam(main = {"secondary": {1: 99}})
        row = main.vertices.index(2)
        self.assertEqual(main.indices[row][:2], [10, 99])
        self.assertEqual([round(w, 5) for w in main.weights[row][:2]], [0.4, 0.6])
        self.assertEqual(main.keptTriangleIds, [[0]])
        self.assertEqual(coat.keptTriangleIds, [[1, 2, 3]])

    def test_seam_overlapRings_drawsTheNeighboursBandAsWell(self):
        # one ring: every Coat triangle touching a vertex the Main draws; ownership is unchanged
        main, coat = self._seam(main = {"overlapRings": 1})
        self.assertEqual(main.keptTriangleIds, [[0, 1, 2]])
        self.assertEqual(main.stats.overlapTriangles, 2)
        self.assertEqual(main.stats.trianglesKept, [3])
        self.assertEqual(coat.keptTriangleIds, [[1, 2, 3]])
        self.assertEqual(coat.stats.overlapTriangles, 0)

        # a second ring reaches one step further through the shared vertices
        main, _ = self._seam(main = {"overlapRings": 2})
        self.assertEqual(main.keptTriangleIds, [[0, 1, 2, 3]])
        self.assertEqual(main.stats.overlapTriangles, 3)
        self.assertEqual(sorted(main.vertices), [0, 1, 2, 3, 4, 5])

    # ================ the mirrored inner layer ================

    def _mirrorSpecs(self, mirrored):
        specs = makeSpecs()
        specs[0].mirroredIbs = mirrored
        specs[1].mirroredIbs = mirrored
        return specs

    def test_mirroredIbs_eachTriangleFollowedByItsTwinWoundBack(self):
        body = FRB.VGComponentSplit(Weights, Indices, Ibs, self._mirrorSpecs([0])).split("Body")

        # every corner copied once, after the kept vertices, with its weights; a corner two triangles share
        # (2) is one copy
        self.assertEqual(body.vertices, [0, 1, 2, 3, 4, 0, 1, 2, 3, 4])
        self.assertEqual(body.mirrored, [False] * 5 + [True] * 5)
        self.assertEqual(body.indices[5:], body.indices[:5])
        self.assertEqual(body.weights[5:], body.weights[:5])

        # each triangle followed by its twin, wound the other way, under the SAME source id
        self.assertEqual(body.ibs, [[[0, 1, 2], [5, 7, 6], [2, 3, 4], [7, 9, 8]], []])
        self.assertEqual(body.keptTriangleIds, [[0, 0, 1, 1], []])
        self.assertEqual((body.stats.mirroredVertices, body.stats.mirroredTriangles), (5, 2))
        self.assertEqual(body.stats.keptVertices, 10)
        self.assertEqual(body.stats.trianglesKept, [4, 0])

    def test_mirroredIbs_emptyOrOnANegativeIndexComponent_changesNothing(self):
        plain = self._split.split("Body")
        self.assertEqual(plain.mirrored, [])
        self.assertEqual(plain.stats.mirroredTriangles, 0)

        # a negative-index component's vertex buffers are not rewritten, so it has nowhere to put copies
        bang = FRB.VGComponentSplit(Weights, Indices, Ibs, self._mirrorSpecs([1])).split("Bang")
        self.assertEqual(bang.ibs, self._split.split("Bang").ibs)
        self.assertEqual(bang.mirrored, [])

    def test_mirrorPositionLine_normalTurnedRoundAndMovedInside(self):
        line = struct.pack("<3f3f4f", 1, 2, 3, 0, 0, 1, 1, 0, 0, 1)
        out = struct.unpack("<3f3f4f", FRB.VGComponentSplit.mirrorPositionLine(line, 0.5))
        self.assertEqual(out[:3], (1.0, 2.0, 2.5))
        self.assertEqual(out[3:6], (0.0, 0.0, -1.0))
        self.assertEqual(out[6:], (1.0, 0.0, 0.0, 1.0))

        # too short to hold a normal: as it is
        self.assertEqual(FRB.VGComponentSplit.mirrorPositionLine(b"\x01" * 12, 0.5), b"\x01" * 12)

    # A coat panel A with its own lining B 4 mm behind it, facing the other way; a single-layer panel C; and a panel
    # E 4 mm under D facing the SAME way (cloth over a body). Every vertex on the one component's group 0.
    #   A (0,1,2) z=0 facing +z   B (3,5,4) z=-0.004 facing -z   C (6,7,8) facing +z   D (9,10,11) z=0 / E (12,13,14) z=-0.004, both +z
    BackedPositions = [(0, 0, 0), (1, 0, 0), (0, 1, 0),
                       (0, 0, -0.004), (1, 0, -0.004), (0, 1, -0.004),
                       (5, 0, 0), (6, 0, 0), (5, 1, 0),
                       (10, 0, 0), (11, 0, 0), (10, 1, 0),
                       (10, 0, -0.004), (11, 0, -0.004), (10, 1, -0.004)]
    BackedNormals = [(0, 0, 1)] * 3 + [(0, 0, -1)] * 3 + [(0, 0, 1)] * 9
    BackedIbs = [[[0, 1, 2], [3, 5, 4], [6, 7, 8], [9, 10, 11], [12, 13, 14]]]

    def _backedSplit(self, reach, geometry = True):
        spec = FRB.VGComponentSpec("Body", {0: 10})
        spec.mirroredIbs = [0]
        spec.mirrorBackedReach = reach
        count = len(self.BackedPositions)
        split = FRB.VGComponentSplit([[1, 0, 0, 0]] * count, [[0, 0, 0, 0]] * count, self.BackedIbs, [spec])
        if (geometry):
            split.setGeometry(self.BackedPositions, self.BackedNormals)
        return split.split("Body")

    def test_mirrorBackedReach_aTriangleWithItsOwnLiningGetsNoTwin(self):
        body = self._backedSplit(0.01)

        # A and B back each other: no twins. C has nothing behind it; D has E behind it, but E faces the SAME way
        # (a twin pushed into it hides behind it); E has nothing behind it -- those three keep their twins
        self.assertEqual(body.keptTriangleIds, [[0, 1, 2, 2, 3, 3, 4, 4]])
        self.assertEqual((body.stats.mirrorBacked, body.stats.mirroredTriangles), (2, 3))
        self.assertEqual(body.stats.trianglesKept, [8])

        # the twin is still the triangle wound the other way, over copies
        ibs = body.ibs[0]
        self.assertEqual(ibs[:2], [[0, 1, 2], [3, 5, 4]])
        self.assertEqual(len(ibs), 8)
        twinC = ibs[3]
        self.assertEqual([body.vertices[v] for v in twinC], [6, 8, 7])
        self.assertTrue(all(body.mirrored[v] for v in twinC))

    def test_mirrorBackedReach_offOutOfReachOrWithoutGeometry_mirrorsEveryTriangle(self):
        # 0 (the default), a lining further away than the reach, and no positions handed over: every triangle mirrored
        for body in (self._backedSplit(0.0), self._backedSplit(0.003), self._backedSplit(0.01, geometry = False)):
            self.assertEqual(body.keptTriangleIds, [[0, 0, 1, 1, 2, 2, 3, 3, 4, 4]])
            self.assertEqual((body.stats.mirrorBacked, body.stats.mirroredTriangles), (0, 5))
        self.assertEqual(FRB.VGComponentSpec("A", {1: 2}).mirrorBackedReach, 0.0)

    # ================ shared groups ================

    def test_splitGroups_aGroupsWeightSharedAmongBones(self):
        specs = makeSpecs()
        specs[0].splitGroups = {0: [(10, 0.25), (99, 0.75)]}
        body = FRB.VGComponentSplit(Weights, Indices, Ibs, specs).split("Body")

        # vertex 0 is wholly group 0: its weight is shared 0.75 / 0.25, the larger first
        self.assertEqual(body.indices[0][:2], [99, 10])
        self.assertEqual([round(w, 6) for w in body.weights[0][:2]], [0.75, 0.25])
        # vertex 1 is half 0, half 1 (-> 11): 0.375 on 99, 0.5 on 11, 0.125 on 10
        self.assertEqual(body.indices[1][:3], [11, 99, 10])
        self.assertEqual([round(w, 6) for w in body.weights[1][:3]], [0.5, 0.375, 0.125])
        # a vertex with no share untouched, and the count
        self.assertEqual(body.indices[2], [11, 0, 0, 0])
        self.assertEqual(body.stats.splitVertices, 2)

    def test_splitGroups_keepsTheFourLargest(self):
        specs = makeSpecs()
        specs[0].splitGroups = {1: [(20, 0.4), (21, 0.3), (22, 0.2), (23, 0.1)]}
        body = FRB.VGComponentSplit(Weights, Indices, Ibs, specs).split("Body")

        # vertex 1: 0.5 on group 0 (-> 10) and 0.5 shared four ways -- five influences, the smallest dropped
        self.assertEqual(body.indices[1], [10, 20, 21, 22])
        self.assertAlmostEqual(sum(body.weights[1]), 1.0, places = 5)
        self.assertEqual(self._split.split("Body").stats.splitVertices, 0)

    def test_unknownComponent_raises(self):
        with self.assertRaises(ValueError):
            self._split.split("Nope")

    # ================ encoding ======================

    def test_encodeBlend_fourFloatsThenFourInts(self):
        data = FRB.VGComponentSplit.encodeBlend([[0.25, 0.75, 0, 0]], [[3, -2, 0, 0]])
        self.assertEqual(data, struct.pack("<4f4i", 0.25, 0.75, 0, 0, 3, -2, 0, 0))

    def test_encodeIb_threeUnsignedInts(self):
        self.assertEqual(FRB.VGComponentSplit.encodeIb([[1, 2, 3]]), struct.pack("<3I", 1, 2, 3))

    def test_keepLines_orderAndBounds(self):
        self.assertEqual(FRB.VGComponentSplit.keepLines(b"aabbccdd", 2, [3, 0]), b"ddaa")
        with self.assertRaises(ValueError):
            FRB.VGComponentSplit.keepLines(b"aabb", 2, [2])


class VGSplitGroupResourceTest(BaseUnitTest):
    """
    Tests for :class:`VGSplitGroupResource` -- the grouped resource that writes one component's buffers together
    """

    def setUp(self):
        super().setUp()
        self._folder = tempfile.mkdtemp()
        with open(os.path.join(self._folder, "Blend.buf"), "wb") as f:
            f.write(FRB.VGComponentSplit.encodeBlend(Weights, Indices))
        with open(os.path.join(self._folder, "Head.ib"), "wb") as f:
            f.write(FRB.VGComponentSplit.encodeIb(Ibs[0]))
        with open(os.path.join(self._folder, "Body.ib"), "wb") as f:
            f.write(FRB.VGComponentSplit.encodeIb(Ibs[1]))
        with open(os.path.join(self._folder, "Position.buf"), "wb") as f:
            f.write(b"".join(struct.pack("<3f", i, i, i) for i in range(6)))
        with open(os.path.join(self._folder, "Texcoord.buf"), "wb") as f:
            f.write(bytes(range(24)))

    def _makeGroup(self, component, ibPaths = None, **kwargs):
        group = FRB.VGSplitGroupResource("group", component = component, specs = makeSpecs(),
                                         ibPaths = ibPaths if (ibPaths is not None) else [os.path.join(self._folder, "Head.ib"), os.path.join(self._folder, "Body.ib")],
                                         **kwargs)
        group.addResource((0, "", "blend"), FRB.RemapIniFixResource("blend", self._folder, "Blend.buf", "BlendFixed.buf"))
        group.addResource((0, "", "position"), FRB.RemapIniFixResource("position", self._folder, "Position.buf", "PositionFixed.buf"))
        group.addResource((0, "", "texcoord"), FRB.RemapIniFixResource("texcoord", self._folder, "Texcoord.buf", "TexcoordFixed.buf"))
        group.addResource((0, "", "ib"), FRB.RemapIniFixResource("buf", self._folder, "Head.ib", "HeadFixed.ib"))
        return group

    def _read(self, name):
        with open(os.path.join(self._folder, name), "rb") as f:
            return f.read()

    def test_fix_cutComponent_everyBufferFollowsTheKeptVertices(self):
        group = self._makeGroup("Body", texcoordLineEdit = lambda line: bytes([line[0], 128, line[2], line[3]]))
        self.assertTrue(group.fix())

        body = FRB.VGComponentSplit(Weights, Indices, Ibs, makeSpecs()).split("Body")
        self.assertEqual(self._read("BlendFixed.buf"), FRB.VGComponentSplit.encodeBlend(body.weights, body.indices))
        self.assertEqual(self._read("HeadFixed.ib"), FRB.VGComponentSplit.encodeIb(body.ibs[0]))
        self.assertEqual(self._read("PositionFixed.buf"), b"".join(struct.pack("<3f", i, i, i) for i in [0, 1, 2, 3, 4]))
        self.assertEqual(self._read("TexcoordFixed.buf"), bytes(sum(([4 * i, 128, 4 * i + 2, 4 * i + 3] for i in [0, 1, 2, 3, 4]), [])))

    def test_fix_mirrorLineEdit_onlyOnTheMirroredLines(self):
        group = self._makeGroup("Body")
        specs = makeSpecs()
        specs[0].mirroredIbs = [0]
        group.specs = specs
        group.mirrorLineEdit = lambda line: b"\xff" * len(line)
        self.assertTrue(group.fix())

        # the kept vertices' lines, then their copies' -- edited
        kept = b"".join(struct.pack("<3f", i, i, i) for i in [0, 1, 2, 3, 4])
        self.assertEqual(self._read("PositionFixed.buf"), kept + b"\xff" * len(kept))
        body = FRB.VGComponentSplit(Weights, Indices, Ibs, specs).split("Body")
        self.assertEqual(self._read("HeadFixed.ib"), FRB.VGComponentSplit.encodeIb(body.ibs[0]))

    def test_fix_pushAway_byWeightShareHorizontally(self):
        group = self._makeGroup("Body")
        group.pushAway = [FRB.VGPushAway([0], [-1.0, 0.0, 0.0], 0.5)]
        self.assertTrue(group.fix())

        lines = [struct.unpack("<3f", self._read("PositionFixed.buf")[12 * i: 12 * i + 12]) for i in range(5)]
        # vertex 0, at the origin and wholly on group 0: 0.5 along +x, away from (-1, _, 0)
        self.assertEqual(lines[0], (0.5, 0.0, 0.0))
        # vertex 1, (1, 1, 1) half on group 0: 0.25 along (2, 0, 1) / sqrt(5) -- height never moves
        self.assertAlmostEqual(lines[1][0], 1.0 + 0.5 / 5 ** 0.5, places = 5)
        self.assertEqual(lines[1][1], 1.0)
        self.assertAlmostEqual(lines[1][2], 1.0 + 0.25 / 5 ** 0.5, places = 5)
        # vertex 2 carries no group 0: untouched
        self.assertEqual(lines[2], (2.0, 2.0, 2.0))

    def test_fix_pushAway_sideLimitsToOneHalf(self):
        group = self._makeGroup("Body")
        group.pushAway = [FRB.VGPushAway([0], [-1.0, 0.0, 0.0], 0.5, -1)]
        self.assertTrue(group.fix())
        # every vertex here has x >= 0, so a push limited to x < 0 moves nothing
        self.assertEqual(self._read("PositionFixed.buf"), b"".join(struct.pack("<3f", i, i, i) for i in [0, 1, 2, 3, 4]))

    def test_fix_negativeComponent_wholeVertexBuffersTrimmedIb(self):
        group = self._makeGroup("Bang")
        self.assertTrue(group.fix())

        bang = FRB.VGComponentSplit(Weights, Indices, Ibs, makeSpecs()).split("Bang")
        self.assertEqual(self._read("BlendFixed.buf"), FRB.VGComponentSplit.encodeBlend(bang.weights, bang.indices))
        self.assertEqual(self._read("HeadFixed.ib"), b"")
        self.assertEqual(self._read("PositionFixed.buf"), self._read("Position.buf"))
        self.assertEqual(self._read("TexcoordFixed.buf"), self._read("Texcoord.buf"))

    def test_fix_ibPathsDefaultToTheGroupsOwnMembers(self):
        # with only the head's ib known, the bang triangle is not there to be excluded or kept
        group = self._makeGroup("Body", ibPaths = [])
        self.assertTrue(group.fix())
        self.assertEqual(len(self._read("HeadFixed.ib")), 2 * 12)

    def test_fix_noBlendMember_raises(self):
        group = FRB.VGSplitGroupResource("group", component = "Body", specs = makeSpecs())
        group.addResource((0, "", "ib"), FRB.RemapIniFixResource("buf", self._folder, "Head.ib", "HeadFixed.ib"))
        with self.assertRaises(ValueError):
            group.fix()

    def test_fixFunc_overridesTheSplit(self):
        group = self._makeGroup("Body", fixFunc = lambda g: False)
        self.assertFalse(group.fix())
        self.assertFalse(os.path.exists(os.path.join(self._folder, "BlendFixed.buf")))

    def test_properties_roundTrip(self):
        edit = lambda line: line
        group = FRB.VGSplitGroupResource("group", component = "Body", specs = makeSpecs(), ibPaths = ["a.ib"], texcoordLineEdit = edit)
        self.assertEqual(group.component, "Body")
        self.assertEqual([s.name for s in group.specs], ["Body", "Bang"])
        self.assertEqual(group.ibPaths, ["a.ib"])
        self.assertIs(group.texcoordLineEdit, edit)
        self.assertIsNone(group.positionLineEdit)
        self.assertIsInstance(group, FRB.IniGroupedResource)
        self.assertIsInstance(group, FRB.RemapIniResourceMixin)


class BufReplaceTest(BaseUnitTest):
    """
    Tests for :class:`BufReplace` -- the resource edit that names one of a mod's buffers by its kind
    """

    def test_kinds_namingAndResType(self):
        # a trailing element name is dropped before the suffix goes on, the way getRemapBlendName does
        tests = [("blend", "blend", "ResourceYelanRikaRemapBlend", "YelanRikaRemapBlend.buf"),
                 ("position", "position", "ResourceYelanBlendRikaRemapPosition", "YelanBlendRikaRemapPosition.buf"),
                 ("texcoord", "texcoord", "ResourceYelanBlendRikaRemapTexcoord", "YelanBlendRikaRemapTexcoord.buf"),
                 ("ib", "buf", "ResourceYelanBlendRikaRemapIB", "YelanBlendRikaRemapIB.ib")]
        for kind, resType, resourceName, fileName in tests:
            edit = FRB.BufReplace((0, "", "x"), kind)
            self.assertEqual(edit.kind, kind)
            self.assertEqual(edit.resType, resType)
            self.assertEqual(edit.getFixResourceName("ResourceYelanBlend", modName = "rika"), resourceName)
            self.assertEqual(edit.getFixFile("YelanBlend.ib" if (kind == "ib") else "YelanBlend.buf", modName = "rika"), fileName)

    def test_resSubType_betweenModAndKind(self):
        edit = FRB.BufReplace((0, "", "x"), "ib", resSubType = "head")
        self.assertEqual(edit.resSubType, "head")
        self.assertEqual(edit.getFixResourceName("ResourceYelanHeadIB", modName = "rika"), "ResourceYelanHeadRikaHeadRemapIB")

    def test_unknownKind_raises(self):
        with self.assertRaises(ValueError):
            FRB.BufReplace((0, "", "x"), "normal")

    def test_buildResModel_typedByKind(self):
        class FakeIni:
            folder = "C:/mods"

        edit = FRB.BufReplace((0, "", "x"), "texcoord")
        resource = edit.buildResModel("resourceRemapBlend", FakeIni(), "Texcoord.buf", "TexcoordFixed.buf", None, modName = "rika")
        self.assertIsInstance(resource, FRB.RemapIniFixResource)
        self.assertEqual(resource.type, "texcoord")
        self.assertTrue(resource.fixedPath.replace("\\", "/").endswith("C:/mods/TexcoordFixed.buf"))
        self.assertIsNone(edit.buildResModel("x", None, "a", "b"))
