##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
import sys
##### EndExtImports

##### LocalImports
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
##### EndLocalImports


class _FakeIni():
    """
    The only thing :meth:`RegFillMissing.editFromIni` ever reads off an `.ini` file is its
    ``downloadMode`` -- a real ``IniFile`` needs a whole classifier/parser setup to construct, and
    none of it is relevant here
    """

    def __init__(self, downloadMode):
        self.downloadMode = downloadMode


class RegFillMissingTest(BaseUnitTest):
    def makeGraph(self, name: str = "a", src = None) -> FRB.IniSectionGraph:
        if (src is None):
            src = {"x": [(0, "1")]}

        return FRB.IniSectionGraph({name: FRB.IfTemplate([FRB.IfContentPart(src, 0)], name = name)}, [name])

    def entries(self, graph: FRB.IniSectionGraph, name: str = "a", partInd: int = 0):
        return graph.getSection(name).parts[partInd].entries()

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        fillMissing = [("a", "1")]
        edit = FRB.RegFillMissing("ib", fillMissing, fillMode = FRB.RegFillMissingMode.TopdownCover,
                                  dependOnDownload = True)

        self.assertEqual(edit.reg, "ib")
        self.assertIs(edit.fillMissing, fillMissing)
        self.assertIs(edit.fillMode, FRB.RegFillMissingMode.TopdownCover)
        self.assertTrue(edit.dependOnDownload)

    def test_init_defaults_fillMissingModeAndNoDownloadDependency(self):
        edit = FRB.RegFillMissing("ib", "null")

        self.assertIs(edit.fillMode, FRB.RegFillMissingMode.FillMissing)
        self.assertFalse(edit.dependOnDownload)

    def test_init_callableFillMissing_keepsTheExactCallable(self):
        # pybind11's std::function caster cannot hand a callable back as the same callable, so the
        # binding has to remember the caller's own object
        fn = lambda part: part
        edit = FRB.RegFillMissing("ib", fn)
        self.assertIs(edit.fillMissing, fn)

    def test_isSubclassOfBaseIniGraphEdit(self):
        self.assertTrue(issubclass(FRB.RegFillMissing, FRB.BaseIniGraphEdit))
        self.assertIsInstance(FRB.RegFillMissing("ib", "null"), FRB.BaseIniGraphEdit)

    # ================================================
    # ============= edit -- FillMissing mode ==========

    def test_edit_returnsTheSameGraphInstance(self):
        graph = self.makeGraph()
        result = FRB.RegFillMissing("z", "9").edit(graph, None)
        self.assertIs(result, graph)

    def test_edit_strFillMissing_addsTheKVPToTheBackOfTheMissingPart(self):
        graph = self.makeGraph()
        FRB.RegFillMissing("z", "9").edit(graph, None)

        self.compareList(self.entries(graph), [("x", "1"), ("z", "9")])

    def test_edit_listFillMissing_addsEveryKVPToTheBackOfTheMissingPart(self):
        graph = self.makeGraph()
        FRB.RegFillMissing("z", [("z", "9"), ("w", "8")]).edit(graph, None)

        self.compareList(self.entries(graph), [("x", "1"), ("z", "9"), ("w", "8")])

    def test_edit_callableFillMissing_calledWithTheMissingPart(self):
        graph = self.makeGraph()
        seen = []

        def fill(part):
            seen.append(part)
            part.addKVP("z", "9")

        FRB.RegFillMissing("z", fill).edit(graph, None)

        self.assertEqual(len(seen), 1)
        self.compareList(self.entries(graph), [("x", "1"), ("z", "9")])

    def test_edit_keyAlreadyPresent_nothingAdded(self):
        graph = self.makeGraph()
        FRB.RegFillMissing("x", "9").edit(graph, None)

        self.compareList(self.entries(graph), [("x", "1")])

    def test_edit_manySectionsMissingTheKey_eachPartFilledExactlyOnce(self):
        # "parentA"/"parentB" both call the same "child" and none of the three has the register --
        # every missing part gets filled, and the shared "child" part exactly once even though it is
        # reachable from both parents (the `partVisited` guard the pure-Python original also had)
        sections = {
            "parentA": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)], name = "parentA"),
            "parentB": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "1")], "run": [(1, "child")]}, 0)], name = "parentB"),
            "child": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "2")]}, 0)], name = "child"),
        }
        graph = FRB.IniSectionGraph(sections, ["parentA", "parentB"])

        FRB.RegFillMissing("z", "9").edit(graph, None)

        for sectionName in ["parentA", "parentB", "child"]:
            zEntries = [entry for entry in self.entries(graph, name = sectionName) if entry[0] == "z"]
            self.compareList(zEntries, [("z", "9")])

    def test_edit_emptyGraph_noError(self):
        graph = FRB.IniSectionGraph({}, [])
        result = FRB.RegFillMissing("z", "9").edit(graph, None)
        self.assertIs(result, graph)

    # ================================================
    # ============ edit -- TopdownCover mode ==========

    def test_edit_topdownCover_addsToTheFrontOfTheRootSection(self):
        graph = self.makeGraph()
        FRB.RegFillMissing("z", "9", fillMode = FRB.RegFillMissingMode.TopdownCover).edit(graph, None)

        self.compareList(self.entries(graph), [("z", "9"), ("x", "1")])

    def test_edit_topdownCover_listFillMissing_addsEveryKVPToTheFront(self):
        graph = self.makeGraph()
        FRB.RegFillMissing("z", [("z", "9"), ("w", "8")],
                           fillMode = FRB.RegFillMissingMode.TopdownCover).edit(graph, None)

        self.compareList(self.entries(graph), [("z", "9"), ("w", "8"), ("x", "1")])

    def test_edit_topdownCover_rootsAlreadyCovered_nothingAdded(self):
        graph = self.makeGraph()
        FRB.RegFillMissing("x", "9", fillMode = FRB.RegFillMissingMode.TopdownCover).edit(graph, None)

        self.compareList(self.entries(graph), [("x", "1")])

    def test_edit_modeReassignedAfterConstruction_isHonoured(self):
        # the C++ member is re-derived from the stored Python object at the start of every edit, so
        # both the mode *and* which end of the part the value lands on have to follow the change
        graph = self.makeGraph()
        edit = FRB.RegFillMissing("z", "9")
        edit.fillMode = FRB.RegFillMissingMode.TopdownCover

        edit.edit(graph, None)

        self.compareList(self.entries(graph), [("z", "9"), ("x", "1")])

    def test_edit_fillMissingReassignedAfterConstruction_isHonoured(self):
        graph = self.makeGraph()
        edit = FRB.RegFillMissing("z", "9")
        edit.fillMissing = [("z", "later"), ("w", "8")]

        edit.edit(graph, None)

        self.compareList(self.entries(graph), [("x", "1"), ("z", "later"), ("w", "8")])

    # ================================================
    # ================== editFromIni ==================

    def test_editFromIni_noDownloadDependency_editsRegardlessOfTheDownloadMode(self):
        graph = self.makeGraph()
        edit = FRB.RegFillMissing("z", "9")

        result = edit.editFromIni(graph, _FakeIni(FRB.DownloadMode.Disabled), None)

        self.assertIs(result, graph)
        self.compareList(self.entries(graph), [("x", "1"), ("z", "9")])

    def test_editFromIni_downloadDisabled_skipsTheEditEntirely(self):
        graph = self.makeGraph()
        edit = FRB.RegFillMissing("z", "9", dependOnDownload = True)

        result = edit.editFromIni(graph, _FakeIni(FRB.DownloadMode.Disabled), None)

        self.assertIs(result, graph)
        self.compareList(self.entries(graph), [("x", "1")])

    def test_editFromIni_downloadNormal_editsWithoutNormalizing(self):
        graph = self.makeGraph()
        edit = FRB.RegFillMissing("z", "9", dependOnDownload = True)

        edit.editFromIni(graph, _FakeIni(FRB.DownloadMode.Normal), None)

        self.assertEqual(len(graph.getSection("a")), 1)
        self.compareList(self.entries(graph), [("x", "1"), ("z", "9")])

    def test_editFromIni_downloadAlways_normalizesTheGraphFirst(self):
        sections = {"a": FRB.IfTemplate([
            FRB.IfContentPart({"x": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, FRB.Z3Context()),
            FRB.IfContentPart({"y": [(0, "2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, FRB.Z3Context()),
        ], name = "a")}
        graph = FRB.IniSectionGraph(sections, ["a"])
        edit = FRB.RegFillMissing("z", "9", dependOnDownload = True)

        edit.editFromIni(graph, _FakeIni(FRB.DownloadMode.Always), None)

        # normalize() inserts the missing 'else' branch -- 4 parts become 6
        self.assertEqual(len(graph.getSection("a")), 6)

    def test_editFromIni_iniIsNone_treatedAsTheNormalDownloadMode(self):
        graph = self.makeGraph()
        edit = FRB.RegFillMissing("z", "9", dependOnDownload = True)

        edit.editFromIni(graph, None, None)

        self.compareList(self.entries(graph), [("x", "1"), ("z", "9")])

    def test_editFromIni_iniWithoutADownloadMode_treatedAsTheNormalDownloadMode(self):
        graph = self.makeGraph()
        edit = FRB.RegFillMissing("z", "9", dependOnDownload = True)

        edit.editFromIni(graph, "SOME INI", None)

        self.compareList(self.entries(graph), [("x", "1"), ("z", "9")])

    def test_editFromIni_reachesAPurePythonEditOverride(self):
        seen = []

        class SpyEdit(FRB.RegFillMissing):
            def edit(self, graph, modType, modName = "", partFilter = None, trackKeys = False, keysToTrack = None):
                seen.append(modName)
                return graph

        graph = self.makeGraph()
        SpyEdit("z", "9").editFromIni(graph, None, None, modName = "rika")

        self.compareList(seen, ["rika"])
        self.compareList(self.entries(graph), [("x", "1")])

    # ================================================
    # ================ fillMissingGraph ===============

    def test_fillMissingGraph_fillsEveryMissingPart(self):
        graph = self.makeGraph()
        result = FRB.RegFillMissing.fillMissingGraph(graph, "z", lambda part: part.addKVP("z", "9"))

        self.assertIs(result, graph)
        self.compareList(self.entries(graph), [("x", "1"), ("z", "9")])

    def test_fillMissingGraph_keyAlreadyPresent_nothingAdded(self):
        graph = self.makeGraph()
        FRB.RegFillMissing.fillMissingGraph(graph, "x", lambda part: part.addKVP("x", "9"))

        self.compareList(self.entries(graph), [("x", "1")])

    # ================================================
    # ==================== addCover ===================

    def test_addCover_rootMissingTheKey_coveredAtTheFront(self):
        graph = self.makeGraph()
        result = FRB.RegFillMissing.addCover(graph, "z", lambda part: part.addKVPToFront("z", "9"))

        self.assertIs(result, graph)
        self.compareList(self.entries(graph), [("z", "9"), ("x", "1")])

    def test_addCover_rootsAlreadyCovered_nothingAdded(self):
        graph = self.makeGraph()
        FRB.RegFillMissing.addCover(graph, "x", lambda part: part.addKVPToFront("x", "9"))

        self.compareList(self.entries(graph), [("x", "1")])

    # ================================================
    # ============ partFilter / trackKeys =============

    def makeParentChildGraph(self) -> FRB.IniSectionGraph:
        return FRB.IniSectionGraph({
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)], name = "parent"),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "child"),
        }, ["parent"])

    def test_init_trackKeysDefaults_offAndTracksEveryKey(self):
        edit = FRB.RegFillMissing("z", "9")

        self.assertFalse(edit.trackKeys)
        self.assertIsNone(edit.keysToTrack)

    def test_init_keysToTrack_keepsTheExactSet(self):
        keysToTrack = {"a"}
        edit = FRB.RegFillMissing("z", "9", trackKeys = True, keysToTrack = keysToTrack)

        self.assertTrue(edit.trackKeys)
        self.assertIs(edit.keysToTrack, keysToTrack)

    def test_edit_noPartFilter_everyMissingPartStillFilled(self):
        graph = self.makeParentChildGraph()
        FRB.RegFillMissing("z", "9").edit(graph, None)

        self.compareList(self.entries(graph, "parent"), [("a", "1"), ("run", "child"), ("z", "9")])
        self.compareList(self.entries(graph, "child"), [("b", "2"), ("z", "9")])

    def test_edit_partFilterRejectsEverything_nothingFilled(self):
        graph = self.makeParentChildGraph()
        FRB.RegFillMissing("z", "9").edit(graph, None, partFilter = lambda iterData, modType, ini: FRB.Ranges.createEmpty())

        self.compareList(self.entries(graph, "parent"), [("a", "1"), ("run", "child")])
        self.compareList(self.entries(graph, "child"), [("b", "2")])

    def test_edit_partFilterAcceptsOneSection_onlyThatSectionFilled(self):
        graph = self.makeParentChildGraph()
        FRB.RegFillMissing("z", "9").edit(
            graph, None,
            partFilter = lambda iterData, modType, ini: FRB.Ranges.createFull() if iterData.sectionName == "child" else FRB.Ranges.createEmpty())

        self.compareList(self.entries(graph, "parent"), [("a", "1"), ("run", "child")])
        self.compareList(self.entries(graph, "child"), [("b", "2"), ("z", "9")])

    def test_edit_partFilterReceivesTheModTypeItWasGiven(self):
        seen = []
        modType = object()
        graph = self.makeGraph()

        def partFilter(iterData, seenModType, ini):
            seen.append(seenModType)
            return FRB.Ranges.createFull()

        FRB.RegFillMissing("z", "9").edit(graph, modType, partFilter = partFilter)
        self.assertIs(seen[0], modType)

    def test_edit_trackKeysOff_partFilterGetsNoColouring(self):
        seen = []
        graph = self.makeParentChildGraph()

        def partFilter(iterData, modType, ini):
            seen.append(iterData.colouring)
            return FRB.Ranges.createFull()

        FRB.RegFillMissing("z", "9", trackKeys = False).edit(graph, None, partFilter = partFilter)
        self.compareList(seen, [None, None])

    def test_edit_trackKeysOn_partFilterGetsTheColouringSoFar(self):
        seen = []
        graph = self.makeParentChildGraph()

        def partFilter(iterData, modType, ini):
            seen.append((iterData.sectionName, sorted(iterData.colouring.keys())))
            return FRB.Ranges.createFull()

        FRB.RegFillMissing("z", "9", trackKeys = True).edit(graph, None, partFilter = partFilter)

        # 'child' additionally sees the 'z' that filling 'parent' just added -- the fill is
        # reflected back into the running colouring before the walk moves on
        self.compareList(seen, [("parent", ["a", "run"]), ("child", ["a", "b", "run", "z"])])

    def test_edit_keysToTrack_narrowsTheColouring(self):
        seen = []
        graph = self.makeParentChildGraph()

        def partFilter(iterData, modType, ini):
            seen.append(sorted(iterData.colouring.keys()))
            return FRB.Ranges.createFull()

        FRB.RegFillMissing("z", "9", trackKeys = True, keysToTrack = {"a"}).edit(graph, None, partFilter = partFilter)
        self.compareList(seen, [["a"], ["a"]])

    def test_edit_partFilterUsingTheColouring_selectsOnTrackedState(self):
        graph = self.makeParentChildGraph()

        # Only fill a part whose tracked state already carries a 'b' -- true for 'child' (its own
        # KVP), false for 'parent', which never sees its callee's keys
        def partFilter(iterData, modType, ini):
            return FRB.Ranges.createFull() if iterData.colouring.contains("b") else FRB.Ranges.createEmpty()

        FRB.RegFillMissing("z", "9", trackKeys = True).edit(graph, None, partFilter = partFilter)

        self.compareList(self.entries(graph, "parent"), [("a", "1"), ("run", "child")])
        self.compareList(self.entries(graph, "child"), [("b", "2"), ("z", "9")])

    def test_edit_keysToTrackMutatedAfterConstruction_isHonoured(self):
        seen = []
        keysToTrack = {"a"}
        edit = FRB.RegFillMissing("z", "9", trackKeys = True, keysToTrack = keysToTrack)
        keysToTrack.add("b")

        def partFilter(iterData, modType, ini):
            seen.append(sorted(iterData.colouring.keys()))
            return FRB.Ranges.createFull()

        edit.edit(self.makeParentChildGraph(), None, partFilter = partFilter)
        self.compareList(seen, [["a"], ["a", "b"]])

    def test_edit_topdownCover_partFilterRejectsTheRoot_nothingAdded(self):
        graph = self.makeGraph()
        FRB.RegFillMissing("z", "9", fillMode = FRB.RegFillMissingMode.TopdownCover).edit(
            graph, None, partFilter = lambda iterData, modType, ini: FRB.Ranges.createEmpty())

        self.compareList(self.entries(graph), [("x", "1")])

    def test_edit_topdownCover_partFilterAcceptsTheRoot_coveredAtTheFront(self):
        graph = self.makeGraph()
        FRB.RegFillMissing("z", "9", fillMode = FRB.RegFillMissingMode.TopdownCover).edit(
            graph, None, partFilter = lambda iterData, modType, ini: FRB.Ranges.createFull())

        self.compareList(self.entries(graph), [("z", "9"), ("x", "1")])

    def test_edit_topdownCover_partFilterSeesTheRootsOwnFirstPart(self):
        seen = []
        graph = self.makeGraph()

        def partFilter(iterData, modType, ini):
            seen.append((iterData.sectionName, iterData.part.entries()))
            return FRB.Ranges.createFull()

        FRB.RegFillMissing("z", "9", fillMode = FRB.RegFillMissingMode.TopdownCover).edit(graph, None, partFilter = partFilter)
        self.compareList(seen, [("a", [("x", "1")])])

    def test_editFromIni_forwardsThePartFilter(self):
        graph = self.makeParentChildGraph()
        FRB.RegFillMissing("z", "9").editFromIni(
            graph, _FakeIni(FRB.DownloadMode.Normal), None,
            partFilter = lambda iterData, modType, ini: FRB.Ranges.createEmpty())

        self.compareList(self.entries(graph, "parent"), [("a", "1"), ("run", "child")])
        self.compareList(self.entries(graph, "child"), [("b", "2")])

    def test_editFromIni_dependOnDownload_forwardsThePartFilter(self):
        graph = self.makeParentChildGraph()
        FRB.RegFillMissing("z", "9", dependOnDownload = True).editFromIni(
            graph, _FakeIni(FRB.DownloadMode.Normal), None,
            partFilter = lambda iterData, modType, ini: FRB.Ranges.createEmpty())

        self.compareList(self.entries(graph, "parent"), [("a", "1"), ("run", "child")])
        self.compareList(self.entries(graph, "child"), [("b", "2")])

    def test_edit_partFilterReturningNone_raisesTypeError(self):
        graph = self.makeGraph()
        edit = FRB.RegFillMissing("z", "9")

        with self.assertRaises(TypeError):
            edit.edit(graph, None, partFilter = lambda iterData, modType, ini: None)

    # ================================================
    # ====== inheriting the caller's key tracking =====

    def _colouringsUnderGroup(self, edit, groupTrackKeys = False, groupKeysToTrack = None):
        """
        Runs 'edit' under a :class:`GraphGroupEdit` carrying its own key-tracking settings, and
        reports the colouring the edit's ``partFilter`` saw for each part
        """

        seen = []

        def partFilter(iterData, modType, ini):
            seen.append(None if iterData.colouring is None else sorted(iterData.colouring.keys()))
            return FRB.Ranges.createFull()

        graph = self.makeParentChildGraph()
        graphGroups = [FRB.IniGraphGroup({("comp", "obj"): graph})]

        kwargs = {}
        if (groupKeysToTrack is not None):
            kwargs["keysToTrack"] = [{("comp", "obj"): groupKeysToTrack}]

        FRB.GraphGroupEdit([{("comp", "obj"): [edit]}],
                           trackKeys = groupTrackKeys,
                           keyFilters = [{("comp", "obj"): [partFilter]}],
                           **kwargs).edit(graphGroups, None)
        return seen

    def test_edit_callerTrackKeys_inheritedWhenTheEditHasNoneOfItsOwn(self):
        seen = self._colouringsUnderGroup(FRB.RegFillMissing("z", "9"),
                                          groupTrackKeys = True, groupKeysToTrack = {"a"})
        self.compareList(seen, [["a"], ["a"]])

    def test_edit_callerTrackKeysOff_editTrackKeysOn_stillTracks(self):
        seen = self._colouringsUnderGroup(FRB.RegFillMissing("z", "9", trackKeys = True),
                                          groupTrackKeys = False)
        self.assertIsNotNone(seen[0])

    def test_edit_neitherSideTracks_noColouring(self):
        seen = self._colouringsUnderGroup(FRB.RegFillMissing("z", "9"), groupTrackKeys = False)
        self.compareList(seen, [None, None])

    def test_edit_editKeysToTrack_overridesTheCallers(self):
        seen = self._colouringsUnderGroup(FRB.RegFillMissing("z", "9", keysToTrack = {"b"}),
                                          groupTrackKeys = True, groupKeysToTrack = {"a"})

        # 'b' is tracked instead of the group's 'a' -- 'parent' holds no 'b' of its own, so its
        # colouring comes back empty, while 'child' (whose own KVP is 'b') has it
        self.compareList(seen, [[], ["b"]])

    def test_edit_callerKeysToTrack_inheritedWhenTheEditHasNoneOfItsOwn(self):
        seen = self._colouringsUnderGroup(FRB.RegFillMissing("z", "9", trackKeys = True),
                                          groupTrackKeys = False, groupKeysToTrack = {"a"})
        self.compareList(seen, [["a"], ["a"]])

    def test_edit_trackKeysPassedDirectly_isHonoured(self):
        seen = []

        def partFilter(iterData, modType, ini):
            seen.append(None if iterData.colouring is None else sorted(iterData.colouring.keys()))
            return FRB.Ranges.createFull()

        FRB.RegFillMissing("z", "9").edit(self.makeParentChildGraph(), None, partFilter = partFilter,
                                          trackKeys = True, keysToTrack = {"a"})
        self.compareList(seen, [["a"], ["a"]])

    def test_editFromIni_forwardsTheCallersKeyTracking(self):
        seen = []

        def partFilter(iterData, modType, ini):
            seen.append(None if iterData.colouring is None else sorted(iterData.colouring.keys()))
            return FRB.Ranges.createFull()

        FRB.RegFillMissing("z", "9").editFromIni(self.makeParentChildGraph(), _FakeIni(FRB.DownloadMode.Normal), None,
                                                 partFilter = partFilter, trackKeys = True, keysToTrack = {"a"})
        self.compareList(seen, [["a"], ["a"]])
