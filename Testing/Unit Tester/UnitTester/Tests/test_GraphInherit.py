import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB



_Z3CTX = FRB.Z3Context()  # shared across every IfPredPart built in this test file
class GraphInheritTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        partFilter = lambda iterData, modType, ini: FRB.Ranges.createFull()
        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = False, partFilter = partFilter)

        self.assertEqual(edit.src, (0, "comp", "src"))
        self.assertEqual(edit.dst, (0, "comp", "dst"))
        self.assertEqual(edit.reg, "run")
        self.assertFalse(edit.latest)
        self.assertIs(edit.partFilter, partFilter)

    def test_init_defaults(self):
        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run")
        self.assertTrue(edit.latest)
        self.assertIsNone(edit.partFilter)
        self.assertIsNone(edit.adder)

    def test_init_setsAdder(self):
        adder = lambda srcGraph, kvps, ini, modType, modName: None
        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", adder = adder)
        self.assertIs(edit.adder, adder)

    # ================================================
    # ===================== edit ======================

    def _makeGraphGroups(self, graphsByModObj):
        return [FRB.IniGraphGroup(dict(graphsByModObj))]

    def _getRootPart(self, graph: FRB.IniSectionGraph, sectionName: str, partInd: int = 0) -> FRB.IfContentPart:
        return graph.getSection(sectionName).parts[partInd]

    def test_edit_noPartFilterLatestTrue_appendsRunKVPToBackOfSrcRoot(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1"), ("run", "dstRoot")])
        # 'dst' itself is left untouched
        self.compareList(self._getRootPart(dstGraph, "dstRoot").entries(), [("b", "2")])

    def test_edit_noPartFilterLatestFalse_prependsRunKVPToFrontOfSrcRoot(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = False)
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("run", "dstRoot"), ("a", "1")])

    def test_edit_dstMultipleRoots_insertsOneKVPPerRootInOrder(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({
            "dstRootA": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)], name = "dstRootA"),
            "dstRootB": FRB.IfTemplate([FRB.IfContentPart({"y": [(0, "2")]}, 0)], name = "dstRootB"),
        }, ["dstRootA", "dstRootB"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        expected = [("a", "1")] + [("run", root) for root in dstGraph.roots]
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), expected)

    def test_edit_srcMultipleRootSections_eachRootGetsTheKVPs(self):
        srcGraph = FRB.IniSectionGraph({
            "srcRootA": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRootA"),
            "srcRootB": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "3")]}, 0)], name = "srcRootB"),
        }, ["srcRootA", "srcRootB"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRootA").entries(), [("a", "1"), ("run", "dstRoot")])
        self.compareList(self._getRootPart(srcGraph, "srcRootB").entries(), [("c", "3"), ("run", "dstRoot")])

    def test_edit_partFilterGiven_insertsWithinFilteredRangeInsteadOfTrueEnd(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart(
            {"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")], "d": [(3, "4")]}, 0
        )], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        # only indices [1, 3) ("b", "c") are a valid insertion window -- the true end of the part is index 4
        partFilter = lambda iterData, modType, ini: FRB.Ranges([(1, 3)])
        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True, partFilter = partFilter)
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1"), ("b", "2"), ("c", "3"), ("run", "dstRoot"), ("d", "4")])

    def test_edit_partFilterReturnsEmptyRange_noInsertionAnywhere(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True, partFilter = lambda iterData, modType, ini: FRB.Ranges.createEmpty())
        edit.edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1")])

    def test_edit_dstGraphHasNoRoots_srcUntouched(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({}, [])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        section = srcGraph.getSection("srcRoot")
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1")])
        self.assertEqual(len(section.parts), 1)

    def test_edit_srcNotFound_returnsGraphGroupsUnchanged(self):
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "missing"), (0, "comp", "dst"), "run")
        result = edit.edit(graphGroups, None)

        self.assertIs(result, graphGroups)
        self.compareList(self._getRootPart(dstGraph, "dstRoot").entries(), [("b", "2")])

    def test_edit_dstNotFound_returnsGraphGroupsUnchanged(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "missing"), "run")
        result = edit.edit(graphGroups, None)

        self.assertIs(result, graphGroups)
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1")])

    def test_edit_rootEndsInConditional_backInsertLandsAfterEndifNotInsideBranch(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
        ], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        edit.edit(graphGroups, None)

        section = srcGraph.getSection("srcRoot")
        self.assertEqual(len(section.parts), 5)
        self.compareList(section.parts[-1].entries(), [("run", "dstRoot")])
        # the branch's own content must be untouched by the new bottom part
        self.compareList(section.parts[2].entries(), [("vb1", "branch1")])

    def test_edit_rootStartsWithConditional_frontInsertLandsBeforeIfNotInsideBranch(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
        ], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = False)
        edit.edit(graphGroups, None)

        section = srcGraph.getSection("srcRoot")
        self.assertEqual(len(section.parts), 5)
        self.compareList(section.parts[0].entries(), [("run", "dstRoot")])
        # the branch's own content must be untouched by the new front part
        self.compareList(section.parts[2].entries(), [("vb1", "branch1")])
        # the trailing content after the conditional must also be untouched
        self.compareList(section.parts[-1].entries(), [("a", "1")])

    def test_edit_returnsTheSameGraphGroupsList(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", latest = True)
        result = edit.edit(graphGroups, None)

        self.assertIs(result, graphGroups)

    def test_editFromIni_noAdder_partFilterIsHandedTheIni(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})
        ini = object()
        seenInis = []

        def partFilter(iterData, modType, filterIni):
            seenInis.append(filterIni)
            return FRB.Ranges.createFull()

        FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "run", partFilter = partFilter).editFromIni(graphGroups, ini, None)

        self.assertEqual(len(seenInis), 1)
        self.assertIs(seenInis[0], ini)
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1"), ("run", "dstRoot")])

    # ================================================
    # ===================== adder =====================

    # A WuWa component: its TextureOverride binds no textures of its own, and the textures are
    # separate resource sections. Composing the texture graphs into it should give it the GI-style
    # 'ps-t0'/'ps-t1' bindings, after its hash and match_* KVPs and outside every 'if'.
    _WUWA_INI = """[TextureOverrideComponent0]
hash = 33e4890f
match_first_index = 0
match_index_count = 8199
{bindings}$object_detected = 1
if $mod_enabled
    local $state_id_0
    if $state_id_0 != $state_id
        $state_id_0 = $state_id
        $\\WWMIv1\\vg_offset = 0
        $\\WWMIv1\\vg_count = 19
        run = CommandListMergeSkeleton
    endif
    if ResourceMergedSkeleton !== null
        handling = skip
        run = CommandListTriggerResourceOverrides
        run = CommandListOverrideSharedResources
        ; Draw Component 0
        drawindexed = 8199, 0, 0
        run = CommandListCleanupSharedResources
    endif
endif

[ResourceHeadDiffuse]
filename = Textures/HeadDiffuse.dds

[ResourceHeadLightMap]
filename = Textures/HeadLightMap.dds
"""

    # Every KVP of the part holding the hash, after its last hash/match_*/ps-t KVP
    @classmethod
    def _afterHeader(cls, iterData, modType, ini):
        part = iterData.part
        if ("hash" not in part):
            return FRB.Ranges.createEmpty()

        header = [i for i in range(len(part)) if (part[i][0] in {"hash", "match_first_index", "match_index_count"} or part[i][0].startswith("ps-t"))]
        return FRB.Ranges([(max(header) + 1, None)])

    @classmethod
    def _addAtFront(cls, srcGraph, kvps, ini, modType, modName):
        return FRB.RegAdd(kvps, latest = False)

    def _makeWuWaGraphGroups(self, ini: FRB.IniFile):
        sections = ini.getIfTemplates()
        graphs = {("", "Component0"): FRB.IniSectionGraph({"TextureOverrideComponent0": sections["TextureOverrideComponent0"]}, ["TextureOverrideComponent0"]),
                  ("", "HeadDiffuse"): FRB.IniSectionGraph({"ResourceHeadDiffuse": sections["ResourceHeadDiffuse"]}, ["ResourceHeadDiffuse"]),
                  ("", "HeadLightMap"): FRB.IniSectionGraph({"ResourceHeadLightMap": sections["ResourceHeadLightMap"]}, ["ResourceHeadLightMap"])}
        return self._makeGraphGroups(graphs), graphs

    def test_edit_adderReturnsRegAdd_wuwaComponentGainsTextureBindingsAfterItsMatchKeys(self):
        ini = FRB.IniFile(txt = self._WUWA_INI.format(bindings = ""))
        graphGroups, graphs = self._makeWuWaGraphGroups(ini)

        for dst, reg in (("HeadDiffuse", "ps-t0"), ("HeadLightMap", "ps-t1")):
            edit = FRB.GraphInherit((0, "", "Component0"), (0, "", dst), reg, partFilter = self._afterHeader, adder = self._addAtFront)
            edit.edit(graphGroups, None)

        expectedIni = FRB.IniFile(txt = self._WUWA_INI.format(bindings = "ps-t0 = ResourceHeadDiffuse\nps-t1 = ResourceHeadLightMap\n"))
        expected = expectedIni.getIfTemplates()["TextureOverrideComponent0"]
        result = graphs[("", "Component0")].getSection("TextureOverrideComponent0")

        self.assertEqual(result.toStr(), expected.toStr())
        self.compareList(result.parts[0].entries(), [("hash", "33e4890f"), ("match_first_index", "0"), ("match_index_count", "8199"),
                                                     ("ps-t0", "ResourceHeadDiffuse"), ("ps-t1", "ResourceHeadLightMap"), ("$object_detected", "1")])

        # the texture graphs themselves are left untouched
        self.compareList(graphs[("", "HeadDiffuse")].getSection("ResourceHeadDiffuse").parts[0].entries(), [("filename", "Textures/HeadDiffuse.dds")])

    def test_edit_adderReturnsRegAddWithoutPartFilter_addsToEveryPart(self):
        # With no partFilter, a register edit runs over every part -- the same as a GraphGroupEdit with no key filter
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"vb1": [(0, "branch1")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
        ], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", adder = lambda srcGraph, kvps, ini, modType, modName: FRB.RegAdd(kvps))
        edit.edit(graphGroups, None)

        section = srcGraph.getSection("srcRoot")
        self.compareList(section.parts[0].entries(), [("a", "1"), ("ps-t0", "dstRoot")])
        self.compareList(section.parts[2].entries(), [("vb1", "branch1"), ("ps-t0", "dstRoot")])

    def test_edit_adderIsHandedTheSrcGraphAndKVPsInRootOrder(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({
            "dstRootA": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)], name = "dstRootA"),
            "dstRootB": FRB.IfTemplate([FRB.IfContentPart({"y": [(0, "2")]}, 0)], name = "dstRootB"),
        }, ["dstRootA", "dstRootB"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})
        modType = object()
        calls = []

        def adder(adderSrcGraph, kvps, ini, adderModType, modName):
            calls.append((adderSrcGraph, kvps, ini, adderModType, modName))
            return None

        FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", adder = adder).edit(graphGroups, modType, modName = "Target")

        self.assertEqual(len(calls), 1)
        adderSrcGraph, kvps, ini, adderModType, modName = calls[0]
        self.assertIs(adderSrcGraph, srcGraph)
        self.compareList(kvps, [("ps-t0", root) for root in dstGraph.roots])
        self.assertIsNone(ini)
        self.assertIs(adderModType, modType)
        self.assertEqual(modName, "Target")

    def test_edit_adderReturnsNone_adderDoesTheInsertionItself(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        def adder(adderSrcGraph, kvps, ini, modType, modName):
            part = adderSrcGraph.getSection("srcRoot").parts[0]
            for i, (key, val) in enumerate(kvps):
                part.addKVPAt(1 + i, key, val)

        FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", adder = adder).edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1"), ("ps-t0", "dstRoot"), ("b", "2")])

    def test_edit_adderReturnsGraphEdit_graphEditRunsOverSrc(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([
            FRB.IfContentPart({"hash": [(0, "abc")], "vb0": [(1, "ResourcePosition")], "drawindexed": [(2, "auto")]}, 0)
        ], name = "srcRoot")}, ["srcRoot"], z3Ctx = _Z3CTX)
        dstGraph = FRB.IniSectionGraph({"ResourceDiffuse": FRB.IfTemplate([FRB.IfContentPart({"filename": [(0, "a.dds")]}, 0)], name = "ResourceDiffuse")}, ["ResourceDiffuse"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        # bound before the draw that reads it
        adder = lambda adderSrcGraph, kvps, ini, modType, modName: FRB.RegSurroundedAdd(kvps, afterRegs = {"drawindexed": None}, latest = True)
        FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", adder = adder).edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(),
                         [("hash", "abc"), ("vb0", "ResourcePosition"), ("ps-t0", "ResourceDiffuse"), ("drawindexed", "auto")])

    def test_edit_adderReturnsPurePythonGraphEdit_itsOwnEditRunsWithThePartFilter(self):
        class RootFrontAdd(FRB.BaseIniGraphEdit):
            def __init__(self, kvps):
                super().__init__()
                self.kvps = kvps
                self.partFilters = []

            def edit(self, graph, modType, modName = "", partFilter = None, trackKeys = False, keysToTrack = None):
                self.partFilters.append(partFilter)
                for section in graph.getRootSections():
                    section.parts[0].addKVPsToFront(self.kvps)
                return graph

        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})
        partFilter = lambda iterData, modType, ini: FRB.Ranges.createFull()
        edits = []

        def adder(adderSrcGraph, kvps, ini, modType, modName):
            edits.append(RootFrontAdd(kvps))
            return edits[-1]

        FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", partFilter = partFilter, adder = adder).edit(graphGroups, None)

        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("ps-t0", "dstRoot"), ("a", "1")])
        self.assertEqual(len(edits[0].partFilters), 1)
        self.assertIs(edits[0].partFilters[0], partFilter)

    def test_editFromIni_adder_adderAndItsEditAreHandedTheIni(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})
        ini = object()
        adderInis = []
        filterInis = []

        def adder(adderSrcGraph, kvps, adderIni, modType, modName):
            adderInis.append(adderIni)
            return FRB.RegAdd(kvps)

        def partFilter(iterData, modType, filterIni):
            filterInis.append(filterIni)
            return FRB.Ranges.createFull()

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", partFilter = partFilter, adder = adder)
        result = edit.editFromIni(graphGroups, ini, None)

        self.assertIs(result, graphGroups)
        self.assertEqual(len(adderInis), 1)
        self.assertIs(adderInis[0], ini)
        self.assertEqual(len(filterInis), 1)
        self.assertIs(filterInis[0], ini)
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1"), ("ps-t0", "dstRoot")])

    def test_edit_adderReturnsSomethingElse_raisesTypeError(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", adder = lambda srcGraph, kvps, ini, modType, modName: kvps)
        with self.assertRaises(TypeError):
            edit.edit(graphGroups, None)

    def test_edit_adderDstGraphHasNoRoots_adderNotCalled(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({}, [])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})
        calls = []

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", adder = lambda *args: calls.append(args))
        edit.edit(graphGroups, None)

        self.assertEqual(calls, [])
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("a", "1")])

    def test_edit_adderReassigned_newAdderTakesEffect(self):
        srcGraph = FRB.IniSectionGraph({"srcRoot": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "srcRoot")}, ["srcRoot"])
        dstGraph = FRB.IniSectionGraph({"dstRoot": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "dstRoot")}, ["dstRoot"])
        graphGroups = self._makeGraphGroups({("comp", "src"): srcGraph, ("comp", "dst"): dstGraph})

        edit = FRB.GraphInherit((0, "comp", "src"), (0, "comp", "dst"), "ps-t0", latest = True)
        edit.adder = lambda adderSrcGraph, kvps, ini, modType, modName: FRB.RegAdd(kvps, latest = False)
        edit.edit(graphGroups, None)

        # the adder's front insertion, not 'latest = True''s back insertion
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("ps-t0", "dstRoot"), ("a", "1")])

        edit.adder = None
        edit.edit(graphGroups, None)
        self.compareList(self._getRootPart(srcGraph, "srcRoot").entries(), [("ps-t0", "dstRoot"), ("a", "1"), ("ps-t0", "dstRoot")])
