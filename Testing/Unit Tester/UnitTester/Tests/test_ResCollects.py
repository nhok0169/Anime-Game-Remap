import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


_Z3CTX = FRB.Z3Context()

_BLEND = (0, "", "remapBlend")
_TEX = (0, "", "remapTex")
_HEAD = (0, "head", "")


class _FakeIni():
    """
    The narrow slice of :class:`IniFile` a resource collector actually reads -- see
    ``test_BaseResEdit.py``'s own copy for why a stand-in is used rather than the real thing
    """

    def __init__(self, sections):
        self.folder = "C:/mods/raiden"
        self.sectionIfTemplates = sections
        self._z3Ctx = _Z3CTX
        self.resources = []
        self.version = None
        self.toVersion = None


class ResCollectsTest(BaseUnitTest):
    """
    Unit-level coverage for :class:`ResRegCollect` and :class:`ResGroupCollect`, driven directly
    rather than through the whole :class:`IniFile` parse/fix pipeline

    .. note::
        ``test_ResRegCollect.py``/``test_ResGroupCollect.py`` cover the same two classes
        end-to-end, by comparing the final .ini text a real :class:`IniFile` produces. Those are
        complementary, not superseded -- this file exercises the pieces those cannot reach
        directly (attribute identity, predicates, remaps, per-phase behaviour)
    """

    # ================================================
    # ================== fixtures =====================

    def _resourceSection(self, name: str, file: str) -> FRB.IfTemplate:
        return FRB.IfTemplate([FRB.IfContentPart({"filename": [(0, file)]}, 0)], name = name)

    def _build(self, conditional: bool = False):
        """A source `section`_ referencing a blend and a texture, optionally inside an if-block"""

        if conditional:
            parts = [
                FRB.IfPredPart("if $x == 0", FRB.IfPredPartType.If, _Z3CTX),
                FRB.IfContentPart({"vb1": [(0, "ResourceHeadBlend")], "ps-t0": [(1, "ResourceHeadDiffuse")]}, 1),
                FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
            ]
        else:
            parts = [FRB.IfContentPart({"vb1": [(0, "ResourceHeadBlend")], "ps-t0": [(1, "ResourceHeadDiffuse")]}, 0)]

        sections = {
            "TextureOverrideHead": FRB.IfTemplate(parts, name = "TextureOverrideHead"),
            "ResourceHeadBlend": self._resourceSection("ResourceHeadBlend", "HeadBlend.buf"),
            "ResourceHeadDiffuse": self._resourceSection("ResourceHeadDiffuse", "HeadDiffuse.dds"),
        }

        ini = _FakeIni(sections)
        graph = FRB.IniSectionGraph(sections, ["TextureOverrideHead"], z3Ctx = _Z3CTX)
        return ini, [FRB.IniGraphGroup({_HEAD[1:]: graph})], graph

    def _sourceEntries(self, graphGroups):
        section = graphGroups[0].graphs[_HEAD[1:]].getSection("TextureOverrideHead")
        return [part.entries() for part in section.parts if isinstance(part, FRB.IfContentPart)]

    # ================================================
    # ================= ResRegCollect =================

    def test_resRegCollect_init_setsAttributes(self):
        srcRegs = {_HEAD: "vb1"}
        resEdits = {"OG": FRB.ResReplace("resourceRemapBlend", _BLEND)}

        edit = FRB.ResRegCollect(srcRegs, resEdits)

        self.assertIs(edit.srcRegs, srcRegs)
        self.assertIs(edit.resEdits, resEdits)
        self.compareDict(edit.resCalls, {})

    def test_resRegCollect_edit_returnsTheSameGraphGroupsList(self):
        _, graphGroups, _ = self._build()

        edit = FRB.ResRegCollect({_HEAD: "vb1"}, {"OG": FRB.ResReplace("t", _BLEND)})
        self.assertIs(edit.edit(graphGroups, None, "rika"), graphGroups)

    def test_resRegCollect_editWithoutIni_collectsButBuildsNothing(self):
        ini, graphGroups, _ = self._build()

        FRB.ResRegCollect({_HEAD: "vb1"}, {"OG": FRB.ResReplace("t", _BLEND)}).edit(graphGroups, None, "rika")

        self.compareSet(set(graphGroups[0].graphs.keys()), {_HEAD[1:]})
        self.compareList(self._sourceEntries(graphGroups),
                         [[("vb1", "ResourceHeadBlend"), ("ps-t0", "ResourceHeadDiffuse")]])
        self.assertEqual(len(ini.resources), 0)

    def test_resRegCollect_editFromIni_buildsTheResourceAndRewritesTheCall(self):
        ini, graphGroups, _ = self._build()

        FRB.ResRegCollect({_HEAD: "vb1"}, {"OG": FRB.ResReplace("resourceRemapBlend", _BLEND)}).editFromIni(
            graphGroups, ini, None, "rika")

        self.compareSet(set(graphGroups[0].graphs.keys()), {_HEAD[1:], _BLEND[1:]})

        entries = self._sourceEntries(graphGroups)[0]
        self.assertNotEqual(entries[0][1], "ResourceHeadBlend")
        self.assertEqual(entries[1][1], "ResourceHeadDiffuse")
        self.assertEqual(len(ini.resources), 1)

    def test_resRegCollect_resPredicateRejects_nothingCollected(self):
        ini, graphGroups, _ = self._build()

        FRB.ResRegCollect({_HEAD: "vb1"}, {"OG": FRB.ResReplace("t", _BLEND)},
                          resPredicates = {_HEAD: lambda reg, val, iterData: False}).editFromIni(
            graphGroups, ini, None, "rika")

        self.compareList(self._sourceEntries(graphGroups),
                         [[("vb1", "ResourceHeadBlend"), ("ps-t0", "ResourceHeadDiffuse")]])
        self.assertEqual(len(ini.resources), 0)

    def test_resRegCollect_partPredicateNarrowsTheSearchWindow(self):
        # 'ps-t0' sits at order index 1, so a [0, 1) window excludes it
        ini, graphGroups, _ = self._build()

        FRB.ResRegCollect({_HEAD: "ps-t0"}, {"OG": FRB.ResReplace("t", _TEX)},
                          partPredicates = {_HEAD: lambda iterData: FRB.Ranges([(0, 1)])}).editFromIni(
            graphGroups, ini, None, "rika")

        self.compareList(self._sourceEntries(graphGroups),
                         [[("vb1", "ResourceHeadBlend"), ("ps-t0", "ResourceHeadDiffuse")]])

        ini, graphGroups, _ = self._build()
        FRB.ResRegCollect({_HEAD: "ps-t0"}, {"OG": FRB.ResReplace("t", _TEX)},
                          partPredicates = {_HEAD: lambda iterData: FRB.Ranges.createFull()}).editFromIni(
            graphGroups, ini, None, "rika")

        self.assertNotEqual(self._sourceEntries(graphGroups)[0][1][1], "ResourceHeadDiffuse")

    def test_resRegCollect_twoSubtypes_shareOneCollectedSet(self):
        ini, graphGroups, _ = self._build()

        FRB.ResRegCollect({_HEAD: "vb1"},
                          {"OG": FRB.ResReplace("t", (0, "", "og")),
                           "Semi": FRB.ResReplace("t", (0, "", "semi"))}).editFromIni(graphGroups, ini, None, "rika")

        self.compareSet(set(graphGroups[0].graphs.keys()), {_HEAD[1:], ("", "og"), ("", "semi")})
        self.assertEqual(len(ini.resources), 2)

    def test_resRegCollect_remaps_routeTheSourceGraphIntoAFreshModObject(self):
        ini, graphGroups, original = self._build()

        FRB.ResRegCollect({_HEAD: "vb1"}, {"OG": FRB.ResReplace("t", _BLEND)},
                          remaps = {_HEAD: {"OG": (0, "OG", "headGone", lambda name: name + "OG")}}).editFromIni(
            graphGroups, ini, None, "rika")

        self.assertIn(("OG", "headGone"), graphGroups[0].graphs)
        self.assertNotIn(_HEAD[1:], graphGroups[0].graphs)

        remapped = graphGroups[0].graphs[("OG", "headGone")]
        self.compareList(sorted(remapped.sections.keys()), ["TextureOverrideHeadOG"])
        # the original graph object is left alone -- only its copy was remapped
        self.compareList(original.getSection("TextureOverrideHead").parts[0].entries(),
                         [("vb1", "ResourceHeadBlend"), ("ps-t0", "ResourceHeadDiffuse")])

    def test_resRegCollect_clear_resetsCollectedState(self):
        ini, graphGroups, _ = self._build()
        edit = FRB.ResRegCollect({_HEAD: "vb1"}, {"OG": FRB.ResReplace("t", _BLEND)})

        edit.editFromIni(graphGroups, ini, None, "rika")
        edit.clear()

        self.compareDict(edit.resCalls, {})

    # ================================================
    # ================ ResGroupCollect ================

    def _makeGroupCollect(self, **kwargs):
        return FRB.ResGroupCollect(
            ["OG"],
            {_BLEND: {_HEAD: "vb1"}, _TEX: {_HEAD: "ps-t0"}},
            {_BLEND: {"OG": FRB.ResReplace("resourceRemapBlend", _BLEND)},
             _TEX: {"OG": FRB.ResReplace("resourceRemapTex", _TEX)}},
            {"OG": FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["testResGroup"])},
            **kwargs)

    def test_resGroupCollect_init_setsAttributes(self):
        resGroupTypes = ["OG"]
        edit = FRB.ResGroupCollect(resGroupTypes, {}, {}, {})

        self.assertIs(edit.resGroupTypes, resGroupTypes)
        self.assertIsInstance(edit.id, int)
        self.assertFalse(edit.resGroupTypesSameTopology)
        self.compareDict(edit.resCalls, {})

    def test_resGroupCollect_autoIdsIncrement_andAnExplicitIdWins(self):
        first = FRB.ResGroupCollect([], {}, {}, {})
        second = FRB.ResGroupCollect([], {}, {}, {})

        self.assertEqual(second.id, first.id + 1)
        self.assertEqual(FRB.ResGroupCollect([], {}, {}, {}, id = 99).id, 99)

    def test_resGroupCollect_editWithoutIni_collectsButBuildsNothing(self):
        ini, graphGroups, _ = self._build()

        result = self._makeGroupCollect().edit(graphGroups, None, "rika")

        self.assertIs(result, graphGroups)
        self.compareSet(set(graphGroups[0].graphs.keys()), {_HEAD[1:]})
        self.assertEqual(len(ini.resources), 0)

    def test_resGroupCollect_editFromIni_buildsBothResourceGraphs(self):
        ini, graphGroups, _ = self._build()

        self._makeGroupCollect().editFromIni(graphGroups, ini, None, "rika")

        self.compareSet(set(graphGroups[0].graphs.keys()), {_HEAD[1:], _BLEND[1:], _TEX[1:]})

    def test_resGroupCollect_editFromIni_buildsOneGroupedResource(self):
        ini, graphGroups, _ = self._build()

        self._makeGroupCollect().editFromIni(graphGroups, ini, None, "rika")

        grouped = [resource for resource in ini.resources if isinstance(resource, FRB.RemapIniGroupedResource)]
        self.assertEqual(len(grouped), 1)
        self.assertTrue(grouped[0].isBuilt)
        self.compareSet(set(grouped[0].resources.keys()), {_BLEND, _TEX})

    def test_resGroupCollect_editFromIni_splicesTheCallSitesIntoIfBlocks(self):
        ini, graphGroups, _ = self._build()

        self._makeGroupCollect().editFromIni(graphGroups, ini, None, "rika")

        section = graphGroups[0].graphs[_HEAD[1:]].getSection("TextureOverrideHead")
        contentParts = [part for part in section.parts if isinstance(part, FRB.IfContentPart)]
        predParts = [part for part in section.parts if isinstance(part, FRB.IfPredPart)]

        # one if/endif pair per resource, each wrapping a single rewritten reference
        self.assertEqual(len(contentParts), 2)
        self.assertEqual(len(predParts), 4)

        keys = sorted(entry[0] for part in contentParts for entry in part.entries())
        self.compareList(keys, ["ps-t0", "vb1"])

        for part in contentParts:
            for key, val in part.entries():
                self.assertNotIn(val, ("ResourceHeadBlend", "ResourceHeadDiffuse"))

    def test_resGroupCollect_conditionalSource_keepsTheEnclosingBranch(self):
        ini, graphGroups, _ = self._build(conditional = True)

        self._makeGroupCollect().editFromIni(graphGroups, ini, None, "rika")

        section = graphGroups[0].graphs[_HEAD[1:]].getSection("TextureOverrideHead")
        predParts = [part for part in section.parts if isinstance(part, FRB.IfPredPart)]

        # the original if/endif plus one pair per spliced-in resource reference
        self.assertEqual(len(predParts), 6)

    def test_resGroupCollect_sameTopologyAcrossTwoGroupTypes(self):
        ini, graphGroups, _ = self._build()

        FRB.ResGroupCollect(
            ["OG", "Semi"],
            {_BLEND: {_HEAD: "vb1"}, _TEX: {_HEAD: "ps-t0"}},
            {_BLEND: {"OG": FRB.ResReplace("t", _BLEND), "Semi": FRB.ResReplace("t", (0, "Semi", "remapBlend"))},
             _TEX: {"OG": FRB.ResReplace("t", _TEX), "Semi": FRB.ResReplace("t", (0, "Semi", "remapTex"))}},
            {"OG": FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["ogGroup"]),
             "Semi": FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["semiGroup"])},
            resGroupTypesSameTopology = True).editFromIni(graphGroups, ini, None, "rika")

        self.compareSet(set(graphGroups[0].graphs.keys()),
                        {_HEAD[1:], _BLEND[1:], _TEX[1:], ("Semi", "remapBlend"), ("Semi", "remapTex")})

        grouped = [resource for resource in ini.resources if isinstance(resource, FRB.RemapIniGroupedResource)]
        self.assertEqual(len(grouped), 2)

    def test_resGroupCollect_unknownResGroupType_isSkipped(self):
        ini, graphGroups, _ = self._build()

        # "Missing" has no builder, so it is not a valid resource-group type at all
        FRB.ResGroupCollect(
            ["Missing"],
            {_BLEND: {_HEAD: "vb1"}},
            {_BLEND: {"OG": FRB.ResReplace("t", _BLEND)}},
            {"OG": FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["g"])}).editFromIni(
            graphGroups, ini, None, "rika")

        self.compareSet(set(graphGroups[0].graphs.keys()), {_HEAD[1:]})
        self.assertEqual(len(ini.resources), 0)

    def test_resGroupCollect_clear_resetsCollectedState(self):
        ini, graphGroups, _ = self._build()
        edit = self._makeGroupCollect()

        edit.editFromIni(graphGroups, ini, None, "rika")
        edit.clear()

        self.compareDict(edit.resCalls, {})
