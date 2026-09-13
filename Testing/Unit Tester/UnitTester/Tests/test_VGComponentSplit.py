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

    # ================ graph cut (fill) ==============

    def test_fill_takesWhatNegativeLeavesAndSkinsOrphans(self):
        body = self._split.split("Body")

        # the bang triangle is excluded (all corners live in the Bang); the seam triangle is the Body's
        # (2 of 3 corners owned) and drags vertices 3 and 4 in
        self.assertEqual(body.vertices, [0, 1, 2, 3, 4])
        self.assertEqual(body.ibs, [[[0, 1, 2], [2, 3, 4]], []])
        self.assertEqual(body.stats.trianglesKept, [2, 0])
        self.assertEqual(body.stats.trianglesDropped, [0, 1])

        # weights renormalised in the Body's bones; the two orphans skinned to their neighbour's bone 11
        self.assertEqual(body.indices, [[10, 0, 0, 0], [10, 11, 0, 0], [11, 0, 0, 0], [11, 0, 0, 0], [11, 0, 0, 0]])
        self.assertEqual([[round(w, 6) for w in row] for row in body.weights],
                         [[1, 0, 0, 0], [0.5, 0.5, 0, 0], [1, 0, 0, 0], [1, 0, 0, 0], [1, 0, 0, 0]])
        self.assertEqual(body.stats.neighbourSkinned, 2)
        self.assertEqual(body.stats.keptVertices, 5)
        self.assertEqual(body.live, [])

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
