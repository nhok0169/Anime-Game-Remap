import sys
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class SpyGroupEdit(FRB.BaseIniGraphGroupEdit):
    """
    Records which mods it was run for, and hands the groups straight back.

    ``BaseIniGraphGroupEdit.editFromIni`` forwards to ``self.edit`` through genuine Python
    attribute lookup, so overriding only ``edit`` here is enough to be reached from the C++ side.
    """

    def __init__(self):
        super().__init__()
        self.calls = []

    def edit(self, graphGroups, modType, modName = ""):
        self.calls.append(modName)
        return graphGroups


class GIMIFixerTest(BaseIniFileTest):
    """
    Tests for :class:`FRB.GIMIFixer` -- the half of the pipeline that turns what
    :class:`FRB.GIMIParser` found into the new .ini file's text.

    .. note::
        These avoid asserting the exact rendered .ini text: what a `section`_ looks like belongs to
        :class:`FRB.IfTemplate`/:class:`FRB.IniSectionGraph` and is covered by their own tests. What
        is pinned here is the fixer's own contract -- which groups it builds, that they are copies,
        which mods the edits run for, and how the result is keyed.
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("", "texcoord")]),
                                      downloads = {("", "texcoord"): {"vb0": FRB.DownloadData("testPosition", FRB.FileDownload("anotherURL", "anotherBaseFile"))}},
                                      trackKeys = False)

    def createFixer(self, **kwargs):
        self._fixer = FRB.GIMIFixer(self._parser, **kwargs)

    def create(self, **kwargs):
        self.createIniFile()
        self.createParser()
        self.createFixer(**kwargs)
        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer

    def parseAndFix(self):
        self._iniFile.parse()
        return self._iniFile.fix()

    # =========================== construction =====================================

    def test_fixer_isABaseIniFixer(self):
        self.create()

        # The C++ GIMIFixer is registered with BaseIniFixer as its real pybind11 base, so this is
        # genuine inheritance, not just a documented claim.
        self.assertIsInstance(self._fixer, FRB.BaseIniFixer)

    def test_construct_parserAndIniFileTakenFromTheParser(self):
        self.create()

        self.assertIs(self._fixer._parser, self._parser)
        self.assertIs(self._fixer._iniFile, self._iniFile)

    def test_construct_keepsTheCallersOwnObjects(self):
        self.createIniFile()
        self.createParser()

        edits = [SpyGroupEdit()]
        modsToFix = ["rika"]
        fixer = FRB.GIMIFixer(self._parser, graphGroupEdits = edits, modsToFix = modsToFix)

        self.assertIs(fixer.graphGroupEdits, edits)
        self.assertIs(fixer.modsToFix, modsToFix)
        self.assertIsNone(fixer.prevFixer)

    def test_construct_defaults_emptyEditsNoModsToFix(self):
        self.create()

        self.compareList(self._fixer.graphGroupEdits, [])
        self.assertIsNone(self._fixer.modsToFix)
        self.compareList(self._fixer.graphGroups, [])

    # =========================== getModsToFix =====================================

    def test_getModsToFix_explicitList_usedAsIs(self):
        self.create(modsToFix = ["rika", "kyrie"])
        self.compareList(self._fixer.getModsToFix(), ["rika", "kyrie"])

    def test_getModsToFix_noExplicitList_takenFromTheIniFilesModType(self):
        self.create()

        expected = self._iniFile.availableType.getModsToFix()
        self.compareList(self._fixer.getModsToFix(), list(expected))

    def test_getModsToFix_assignedAfterConstruction_isHonoured(self):
        self.create()

        self._fixer.modsToFix = ["lateMod"]
        self.compareList(self._fixer.getModsToFix(), ["lateMod"])

    # =========================== getFix =====================================

    def test_getFix_groupsKeyedByTheIniFilePath(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        result = self._fixer.getFix()

        self.compareList(list(result.keys()), [self._iniFile.filePath.path])
        self.assertIsInstance(result[self._iniFile.filePath.path], FRB.IniGraphGroup)

    def test_getFix_groupHoldsTheParsersCommandAndDownloadGraphs(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        self._fixer.getFix()
        graphs = self._fixer.graphGroups[0].graphs

        expected = list(self._parser.commandGraphs.keys())
        expected.append((FRB.IniGraphModObjKeywords.Download.value, "testPosition"))
        self.compareList(list(graphs.keys()), expected)

    def test_getFix_graphsAreCopiesOfTheParsers(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        self._fixer.getFix()
        graphs = self._fixer.graphGroups[0].graphs

        # Editing the fixer's groups must not write through to the parser -- a second fixer over
        # the same .ini file has to start from the same place.
        for modObj in self._parser.commandGraphs:
            self.assertIsNot(graphs[modObj], self._parser.commandGraphs[modObj])

    def test_getFix_onlyEditObjGraphs_returnsNothingButStillBuildsTheGroups(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        self.assertIsNone(self._fixer.getFix(onlyEditObjGraphs = True))
        self.assertEqual(len(self._fixer.graphGroups), 1)

    def test_getFix_runsEveryEditOncePerModToFix(self):
        edit = SpyGroupEdit()
        self.create(graphGroupEdits = [edit], modsToFix = ["rika", "kyrie"])
        self._iniFile.parse()

        self._fixer.getFix()

        self.compareList(edit.calls, ["rika", "kyrie"])

    def test_getFix_noModsToFix_noEditsRun(self):
        edit = SpyGroupEdit()
        self.create(graphGroupEdits = [edit], modsToFix = [])
        self._iniFile.parse()

        self._fixer.getFix()

        self.compareList(edit.calls, [])

    def test_getFix_editsAssignedAfterConstruction_areHonoured(self):
        self.create(modsToFix = ["rika"])
        edit = SpyGroupEdit()
        self._fixer.graphGroupEdits = [edit]

        self._iniFile.parse()
        self._fixer.getFix()

        self.compareList(edit.calls, ["rika"])

    # =========================== fix =====================================

    def test_fix_resultKeyedByPathAndIncludesTheOriginalContent(self):
        self.create(modsToFix = ["rika"])
        result = self.parseAndFix()

        path = self._iniFile.filePath.path
        self.compareList(list(result.keys()), [path])
        self.assertIsInstance(result[path], str)

        # withSrc/withBoilerPlate are both on for the public fix(), so the .ini file's own text
        # leads and this software's own credit block follows.
        self.assertTrue(result[path].startswith(self._iniFile.fileTxt))

    def test_fix_boilerPlateIsBuiltInCppNotByTheIniFile(self):
        self.create(modsToFix = ["rika"])
        result = self.parseAndFix()
        content = result[self._iniFile.filePath.path]

        # ``PyIniFixContext`` inherits ``RemapIniFixContext``'s boilerplate rather than forwarding
        # to ``IniFile.addFixBoilerPlate``, so this is the C++ text -- byte-identical to what the
        # .ini file would have built, which is exactly why the swap is safe.
        modTypeName = self._iniFile.availableType.name
        side = "-" * 15
        self.assertIn(f"; {side} {modTypeName} Remap {side}", content)
        self.assertIn("Albert Gold#2696", content)

    def test_fix_theIniFilesOwnBoilerPlateIsNoLongerConsulted(self):
        self.create(modsToFix = ["rika"])

        # Replacing the .ini file's own boilerplate no longer changes what the fixer writes. This
        # is the one deliberate behaviour change from making ``PyIniFixContext`` a
        # ``RemapIniFixContext``: ``IniFile.addFixBoilerPlate`` is still there and still called by
        # ``MultiModFixer``, but the fixer does not go through it.
        self._iniFile.addFixBoilerPlate = lambda fix = "": "SHOULD-NOT-APPEAR"

        result = self.parseAndFix()
        content = result[self._iniFile.filePath.path]

        self.assertNotIn("SHOULD-NOT-APPEAR", content)
        self.assertIn("Albert Gold#2696", content)

    def test_fix_marksTheIniFileAsFixed(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        # Cleared by hand first: the default .ini text already contains this software's own
        # RemapBlend sections, so classify() legitimately flags it as already-fixed before a fixer
        # ever runs.
        self._iniFile._isFixed = False

        self._fixer.fix()
        self.assertTrue(self._iniFile._isFixed)

    def test_fix_populatesGraphGroups(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        self.compareList(self._fixer.graphGroups, [])
        self._fixer.fix()
        self.assertEqual(len(self._fixer.graphGroups), 1)

    # =========================== groupToStr =====================================

    def test_groupToStr_rendersTheGroupsGraphs(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()
        self._fixer.getFix()

        rendered = self._fixer.groupToStr(0)

        # Same text the group renders itself, since both walk the same graphs.
        self.assertEqual(rendered, self._fixer.graphGroups[0].toStr())

    # =========================== clear =====================================

    def test_clear_dropsTheGraphGroups(self):
        self.create(modsToFix = ["rika"])
        self.parseAndFix()
        self.assertEqual(len(self._fixer.graphGroups), 1)

        self._fixer.clear()
        self.compareList(self._fixer.graphGroups, [])

    def test_clear_keepsTheParserAndIniFile(self):
        self.create(modsToFix = ["rika"])
        self._fixer.clear()

        self.assertIs(self._fixer._parser, self._parser)
        self.assertIs(self._fixer._iniFile, self._iniFile)

    # =========================== prevFixer =====================================

    def test_prevFixer_itsEditedGroupsAreTakenOver(self):
        self.create(modsToFix = ["rika"])

        prevEdit = SpyGroupEdit()
        prev = FRB.GIMIFixer(self._parser, graphGroupEdits = [prevEdit], modsToFix = ["rika"])

        ownEdit = SpyGroupEdit()
        self._fixer.graphGroupEdits = [ownEdit]
        self._fixer.prevFixer = prev

        self._iniFile.parse()
        self._fixer.getFix()

        # The previous fixer runs its own edit pass first, then hands its groups over and is left
        # empty -- so both edits ran, and only this fixer still holds anything.
        self.compareList(prevEdit.calls, ["rika"])
        self.compareList(ownEdit.calls, ["rika"])
        self.assertEqual(len(self._fixer.graphGroups), 1)
        self.compareList(prev.graphGroups, [])
