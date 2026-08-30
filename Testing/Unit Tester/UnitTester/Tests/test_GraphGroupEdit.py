import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GraphGroupEditTest(BaseUnitTest):
    def _makeGroups(self):
        graph = FRB.IniSectionGraph(
            {"Root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)], name = "Root")},
            ["Root"])
        return [FRB.IniGraphGroup({("comp", "obj"): graph})], graph

    def _entries(self, graph, name = "Root"):
        return graph.getSection(name).parts[0].entries()

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        edits = [{("comp", "obj"): []}]
        keyFilters = [{}]
        keysToTrack = [{}]

        edit = FRB.GraphGroupEdit(edits, trackKeys = True, keysToTrack = keysToTrack, keyFilters = keyFilters)

        self.assertIs(edit.edits, edits)
        self.assertIs(edit.keyFilters, keyFilters)
        self.assertIs(edit.keysToTrack, keysToTrack)
        self.assertTrue(edit.trackKeys)

    def test_init_defaults(self):
        edit = FRB.GraphGroupEdit([])

        self.assertFalse(edit.trackKeys)
        self.assertIsNone(edit.keysToTrack)
        self.assertIsNone(edit.keyFilters)

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSameGraphGroupsList(self):
        graphGroups, _ = self._makeGroups()

        self.assertIs(FRB.GraphGroupEdit([]).edit(graphGroups, None), graphGroups)

    def test_edit_runOfRegEdits_allAppliedInOnePass(self):
        graphGroups, graph = self._makeGroups()

        FRB.GraphGroupEdit([{("comp", "obj"): [FRB.RegAdd([("c", "3")]),
                                               FRB.RegAdd([("d", "4")], latest = False)]}]).edit(graphGroups, None)

        self.compareList(self._entries(graph), [("d", "4"), ("a", "1"), ("b", "2"), ("c", "3")])

    def test_edit_graphEdit_isApplied(self):
        # This branch raised NameError in the pure-Python original (it read a 'keyFiltersLen' only
        # ever assigned in the register-edit branch), so it never ran at all before this port
        graphGroups, _ = self._makeGroups()

        FRB.GraphGroupEdit([{("comp", "obj"): [FRB.GraphRename(lambda name: name + "Renamed")]}]).edit(graphGroups, None)

        self.compareList(sorted(graphGroups[0].graphs[("comp", "obj")].sections.keys()), ["RootRenamed"])

    def test_edit_mixedRunOfGraphThenRegEdits_bothApplied(self):
        graphGroups, _ = self._makeGroups()

        FRB.GraphGroupEdit([{("comp", "obj"): [FRB.GraphRename(lambda name: name + "X"),
                                               FRB.RegAdd([("e", "5")])]}]).edit(graphGroups, None)

        graph = graphGroups[0].graphs[("comp", "obj")]
        self.compareList(sorted(graph.sections.keys()), ["RootX"])
        self.compareList(self._entries(graph, "RootX"), [("a", "1"), ("b", "2"), ("e", "5")])

    def test_edit_graphWithoutEdits_isSkipped(self):
        graphGroups, graph = self._makeGroups()

        FRB.GraphGroupEdit([{("other", "obj"): [FRB.RegAdd([("q", "0")])]}]).edit(graphGroups, None)

        self.compareList(self._entries(graph), [("a", "1"), ("b", "2")])

    def test_edit_moreEditsThanIniFiles_extraEditsIgnored(self):
        graphGroups, graph = self._makeGroups()

        FRB.GraphGroupEdit([{("comp", "obj"): [FRB.RegAdd([("c", "3")])]},
                            {("comp", "obj"): [FRB.RegAdd([("d", "4")])]}]).edit(graphGroups, None)

        self.compareList(self._entries(graph), [("a", "1"), ("b", "2"), ("c", "3")])

    # ================================================
    # ================== keyFilters ===================

    def test_edit_emptyKeyFilter_regEditSkipped(self):
        graphGroups, graph = self._makeGroups()

        FRB.GraphGroupEdit([{("comp", "obj"): [FRB.RegAdd([("z", "9")])]}],
                           keyFilters = [{("comp", "obj"): [lambda iterData, modType, ini: FRB.Ranges.createEmpty()]}]
                           ).edit(graphGroups, None)

        self.compareList(self._entries(graph), [("a", "1"), ("b", "2")])

    def test_edit_singleCallableKeyFilter_appliedToEveryEdit(self):
        graphGroups, graph = self._makeGroups()

        FRB.GraphGroupEdit([{("comp", "obj"): [FRB.RegAdd([("z", "9")])]}],
                           keyFilters = [{("comp", "obj"): lambda iterData, modType, ini: FRB.Ranges.createFull()}]
                           ).edit(graphGroups, None)

        self.compareList(self._entries(graph), [("a", "1"), ("b", "2"), ("z", "9")])

    def test_edit_keyFilterIdentityReachesAGraphEdit(self):
        # pybind11's std::function caster cannot hand a callable back as the same callable, so the
        # binding has to remember the caller's own object
        seen = []

        class SpyEdit(FRB.BaseIniGraphEdit):
            def edit(self, graph, modType, modName = "", partFilter = None, trackKeys = False, keysToTrack = None):
                seen.append(partFilter)
                return graph

        graphGroups, _ = self._makeGroups()
        myFilter = lambda iterData, modType, ini: FRB.Ranges.createFull()

        FRB.GraphGroupEdit([{("comp", "obj"): [SpyEdit()]}],
                           keyFilters = [{("comp", "obj"): [myFilter]}]).edit(graphGroups, None)

        self.assertIs(seen[0], myFilter)

    def test_edit_noKeyFilter_graphEditStillGetsACallableDefault(self):
        seen = []

        class SpyEdit(FRB.BaseIniGraphEdit):
            def edit(self, graph, modType, modName = "", partFilter = None, trackKeys = False, keysToTrack = None):
                seen.append(partFilter)
                return graph

        graphGroups, _ = self._makeGroups()
        FRB.GraphGroupEdit([{("comp", "obj"): [SpyEdit()]}]).edit(graphGroups, None)

        self.assertIsNotNone(seen[0])
        self.assertFalse(seen[0](None, None, None).isEmpty())

    # ================================================
    # =================== trackKeys ===================

    def test_edit_globalTrackKeys_stillApplies(self):
        graphGroups, graph = self._makeGroups()

        FRB.GraphGroupEdit([{("comp", "obj"): [FRB.RegAdd([("t", "1")])]}], trackKeys = True).edit(graphGroups, None)

        self.compareList(self._entries(graph), [("a", "1"), ("b", "2"), ("t", "1")])

    def test_edit_granularTrackKeys_stillApplies(self):
        graphGroups, graph = self._makeGroups()

        FRB.GraphGroupEdit([{("comp", "obj"): [FRB.RegAdd([("t", "1")])]}],
                           trackKeys = [{("comp", "obj"): True}],
                           keysToTrack = [{("comp", "obj"): {"a"}}]).edit(graphGroups, None)

        self.compareList(self._entries(graph), [("a", "1"), ("b", "2"), ("t", "1")])

    # ================================================
    # ================== editFromIni ==================

    def test_editFromIni_forwardsIniAndModTypeToTheEdit(self):
        seen = []

        class IniSpy(FRB.BaseIniGraphEdit):
            def editFromIni(self, graph, ini, modType, modName = "", partFilter = None, trackKeys = False, keysToTrack = None):
                seen.append((ini, modType, modName))
                return graph

        graphGroups, _ = self._makeGroups()
        FRB.GraphGroupEdit([{("comp", "obj"): [IniSpy()]}]).editFromIni(graphGroups, "INI", "MODTYPE", "rika")

        self.compareList(seen, [("INI", "MODTYPE", "rika")])

    def test_editFromIni_returnsTheSameGraphGroupsList(self):
        graphGroups, _ = self._makeGroups()

        self.assertIs(FRB.GraphGroupEdit([]).editFromIni(graphGroups, None, None), graphGroups)

    # ================================================
    # ============ in-place attribute changes =========

    def test_edit_editsReassignedAfterConstruction_areHonoured(self):
        graphGroups, graph = self._makeGroups()
        edit = FRB.GraphGroupEdit([])
        edit.edits = [{("comp", "obj"): [FRB.RegAdd([("late", "1")])]}]

        edit.edit(graphGroups, None)

        self.compareList(self._entries(graph), [("a", "1"), ("b", "2"), ("late", "1")])
