import os
import re
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

    # =========================== hideOrig =====================================

    def _hiddenHeaderPattern(self, sectionName):
        # The comment goes in front of the *whole* line, indentation included, so a hidden section
        # header reads ";RemapFixHideOrig -->                    [Name]".
        comment = FRB.IniKeywords.HideOriginalComment.value
        return re.compile(re.escape(comment) + r"[ 	]*\[" + re.escape(sectionName) + r"\]")

    def _touchedSections(self):
        result = set()
        for modObj in self._parser.commandGraphs:
            result.update(self._parser.commandGraphs[modObj].sections.keys())
        return result

    def _sectionsInTheOriginalTxt(self, sectionNames):
        # A parser synthesizes a "...RemapFix" section for a mod object the .ini file has none of,
        # and that lands in the command graphs too. There is no original line of it to comment out,
        # so hiding one is a no-op rather than a miss.
        origTxt = self._iniFile.fileTxt
        return {name for name in sectionNames if re.search(r"^[ 	]*\[" + re.escape(name) + r"\]", origTxt, re.MULTILINE)}

    def test_fix_hideOrig_commentsOutTheSectionsTheFixTouched(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        touched = self._sectionsInTheOriginalTxt(self._touchedSections())
        self.assertTrue(touched)

        content = self._iniFile.fix(hideOrig = True)[self._iniFile.filePath.path]

        # Every command section the fix rewrote is commented out inside it, so the original mod
        # stops being displayed and only the remap shows.
        for sectionName in touched:
            self.assertRegex(content, self._hiddenHeaderPattern(sectionName))

    def test_fix_hideOrig_leavesEveryOtherSectionAlone(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        touched = self._touchedSections()
        untouched = self._sectionsInTheOriginalTxt(set(self._iniFile.sectionIfTemplates) - touched)
        self.assertTrue(untouched)

        content = self._iniFile.fix(hideOrig = True)[self._iniFile.filePath.path]

        # A fix can carry a register over verbatim, still pointing at one of the original mod's own
        # resource sections -- commenting those out would break the fix itself.
        for sectionName in untouched:
            self.assertNotRegex(content, self._hiddenHeaderPattern(sectionName))

    def _assertRemapKeyedGraphIsNotHidden(self, remapModObj):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        # Re-key the real blend graph under a mod object naming itself a remap. commandGraphs is
        # the parser's own live dict, so this is the same graph, just relabelled.
        graphs = self._parser.commandGraphs
        graph = graphs.pop(("", "blend"))
        graphs[remapModObj] = graph

        sections = self._sectionsInTheOriginalTxt(set(graph.sections.keys()))
        self.assertTrue(sections)

        content = self._iniFile.fix(hideOrig = True)[self._iniFile.filePath.path]

        for sectionName in sections:
            self.assertNotRegex(content, self._hiddenHeaderPattern(sectionName))

    def test_fix_hideOrig_ignoresAGraphWhoseObjectNameIsARemap(self):
        # A mod object naming itself a remap is this software's own output, not part of the
        # original mod -- hiding it would hide the fix.
        self._assertRemapKeyedGraphIsNotHidden(("", "blendRemapBlend"))

    def test_fix_hideOrig_ignoresAGraphWhoseComponentNameIsARemap(self):
        # Either half of the (component, object) name counts, and the match is case-insensitive.
        self._assertRemapKeyedGraphIsNotHidden(("someremapcomp", "blend"))

    def test_fix_hideOrig_isSkippedWhenThisIsNotTheLastModType(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        touched = self._sectionsInTheOriginalTxt(self._touchedSections())
        self.assertTrue(touched)

        # Several fixers chain over one .ini file -- one per mod type it was classified as -- and
        # only the last of them may rewrite the file. See IniFixingContext.isLastModType.
        content = self._fixer.fix(hideOrig = True, context = FRB.IniFixingContext(isLastModType = False))
        content = content[self._iniFile.filePath.path]

        self.assertNotIn(FRB.IniKeywords.HideOriginalComment.value, content)

        # ...and it still produced its own fix.
        self.assertIn("Albert Gold#2696", content)

    def test_fix_defaultContext_saysThisIsTheFirstAndLastModType(self):
        # A fixer driven directly is the only one, so it is both -- hideOrig and keepBackup both
        # have to work with no context given.
        default = FRB.IniFixingContext()
        self.assertTrue(default.isFirstModType)
        self.assertTrue(default.isLastModType)

    def test_fixingContext_flagsAreIndependentlySettable(self):
        context = FRB.IniFixingContext(isFirstModType = False, isLastModType = True)
        self.assertFalse(context.isFirstModType)
        self.assertTrue(context.isLastModType)

        context.isFirstModType = True
        self.assertTrue(context.isFirstModType)

    def test_fix_keepBackup_isSkippedWhenThisIsNotTheFirstModType(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()
        self._pretendTheIniFileIsOnDisk()

        disabled = []
        self._iniFile.disIni = lambda makeCopy = False: disabled.append(makeCopy)

        # Only the first mod type's fixer moves the .ini file aside; a later one would be backing up
        # a file the first pass already moved. See IniFixingContext.isFirstModType.
        self._fixer.fix(keepBackup = True, fixOnly = True,
                        context = FRB.IniFixingContext(isFirstModType = False, isLastModType = True))

        self.compareList(disabled, [])

    def _pretendTheIniFileIsOnDisk(self):
        # keepBackup only does anything for an .ini file that already exists, and the fixer asks
        # Python's own os.path.exists for that. Narrowed to this .ini file's path so the rest of the
        # fix still sees the real filesystem.
        realExists = os.path.exists
        iniPath = self._iniFile.filePath.path
        self.patch("os.path.exists", side_effect = lambda path: True if path == iniPath else realExists(path))

    def test_fix_keepBackup_isTakenWhenThisIsTheFirstModType(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()
        self._pretendTheIniFileIsOnDisk()

        disabled = []
        self._iniFile.disIni = lambda makeCopy = False: disabled.append(makeCopy)

        self._fixer.fix(keepBackup = True, fixOnly = True,
                        context = FRB.IniFixingContext(isFirstModType = True, isLastModType = True))

        self.assertEqual(len(disabled), 1)

    def test_fix_defaultContext_saysThisIsTheLastModType(self):
        # A fixer driven directly is the only one, so hideOrig has to work with no context given.
        self.assertTrue(FRB.IniFixingContext().isLastModType)

        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        touched = self._sectionsInTheOriginalTxt(self._touchedSections())
        content = self._fixer.fix(hideOrig = True)[self._iniFile.filePath.path]

        for sectionName in touched:
            self.assertRegex(content, self._hiddenHeaderPattern(sectionName))

    def test_fix_hideOrig_leavesTheIniFilesOwnTextAlone(self):
        self.create(modsToFix = ["rika"])
        self._iniFile.parse()

        before = self._iniFile.fileTxt
        self._iniFile.fix(hideOrig = True)

        # Only the copy that went into the fix was commented out.
        self.assertEqual(self._iniFile.fileTxt, before)

    def test_fix_withoutHideOrig_nothingIsCommentedOut(self):
        self.create(modsToFix = ["rika"])
        content = self.parseAndFix()[self._iniFile.filePath.path]

        self.assertNotIn(FRB.IniKeywords.HideOriginalComment.value, content)

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
