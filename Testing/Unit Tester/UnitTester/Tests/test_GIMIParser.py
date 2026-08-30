import sys, os
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GIMIParserTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None

    def _getIbOnly(self, iterData: FRB.SectionIterData, modType: FRB.ModType, ini: FRB.IniFile):
        result = iterData.colouring.getRanges(keysExists = {"hash": True, "match_first_index": False}, keyFilters = {"hash": lambda ind, val: modType.hashes.hasFrom(val, version = ini.version, nonVersionVals = {"type": "ib"})}, includeKeyDefs = False)
        return result

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("", "texcoord"), ("", "body"), ("", "ib")]), 
                                      downloads = {("", "texcoord"): {"vb0": FRB.DownloadData("testPosition", FRB.FileDownload("anotherURL", "anotherBaseFile")),
                                                                      "vb1": FRB.DownloadData("testTexture", FRB.FileDownload("someURL", "someBaseFile"))},
                                                   ("", "blend"): {"ps-t0": FRB.DownloadData("testDiffuse", FRB.FileDownload("unknownURL", "unknownBaseFile"), refToSection = True),
                                                                   "ps-t1": FRB.DownloadData("testLightMap", FRB.FileDownload("uniqueURL", "uniqueBaseFile"), refToSection = False)}},
                                      commandEdits = FRB.GraphGroupEdit(edits = [{("", "blend"): [FRB.RegFillMissing("handling2", [("handling2", "skip"), ("drawindexed2", "auto")], fillMode = FRB.RegFillMissingMode.TopdownCover, dependOnDownload = True),
                                                                                                  FRB.RegFillMissing("ib", "null", dependOnDownload = True)],
                                                                                  ("", "body"): [FRB.RegFillMissing("ps-t999", "DigitOverflow")],
                                                                                  ("", "ib"): [FRB.RegFillMissing("ps-t2,147,483,647", "DigitUnderflow"), FRB.RegNewVals({"hash": "Matsuribayashi-hen"})]}],
                                                                        trackKeys = [{("", "ib"): True}],
                                                                        keysToTrack = [{("", "ib"): {"hash", "match_first_index"}}],
                                                                        keyFilters = [{("", "ib"): self._getIbOnly}]))

    def create(self):
        self.createIniFile()
        self.createParser()
        self._iniFile._iniParser = self._parser

    def createNamedParser(self):
        self.create()
        self._parser.trackKeys = False
        self._parser.objTargetFuncs = []

    def createKeyedParser(self):
        self.create()

        sectionClassifier = FRB.GIMISectionClassifier.buildDefaultClassifierFromIni(self._iniFile)
        sectionClassifier.hashKeyOnlyToModObj = {
            "blend_vb": ("", "blend"), 
            "texcoord_vb": ("", "texcoord"),
            "ib": ("", "ib")
        }

        sectionClassifier.indexKeyToModObj = {
            "ib": {("", "body"): ("", "body")}
        }

        self._parser.trackKeys = True
        self._parser.keysToTrack = {FRB.IniKeywords.Hash.value, FRB.IniKeywords.MatchFirstIndex.value}
        self._parser.objTargetFuncs = [sectionClassifier]

    # ====================== parse =======================================

    def test_textureOverrideRootFoundByName_parsedDataFromIniTxt(self):
        tests = [
                 [self._defaultIniTxt, 
"""[TextureOverrideRaidenShogunBlend]
handling2 = skip
drawindexed2 = auto
ps-t0 = ResourceRaidenTestDiffuseRemapDL
run = CommandListRaidenShogunBlend
handling = skip
draw = 21916,0

[CommandListRaidenShogunBlend]
if $swapmain == 0
\tif $swapvar == 0 && $swapvarn == 0
\t\tvb1 = ResourceRaidenShogunBlend.0
\t\tps-t1 = ResourceRaidenTestLightMapRemapDL
\t\tib = null
\telse
\t\tvb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
\t\tps-t1 = ResourceRaidenTestLightMapRemapDL
\t\tib = null
\tendif
else if $swapmain == 1
\trun = SubSubTextureOverride
endif

[SubSubTextureOverride]
if $swapoffice == 0 && $swapglasses == 0
\tvb1 = GIMINeedsResourcesToAllStartWithResource
\tps-t1 = ResourceRaidenTestLightMapRemapDL
\tib = null
endif

[TextureOverrideRaidenTexcoordRemapFix]
vb0 = ResourceRaidenTestPositionRemapDL
vb1 = ResourceRaidenTestTextureRemapDL

[ResourceRaidenTestPositionRemapDL]
filename = anotherBaseFile

[ResourceRaidenTestTextureRemapDL]
filename = someBaseFile

[ResourceRaidenTestDiffuseRemapDL]
filename = unknownBaseFile

[ResourceRaidenTestLightMapRemapDL]
filename = uniqueBaseFile""", 4]]

        for test in tests:
            iniTxt = test[0]
            self.setupIniTxt(iniTxt)
            self.createNamedParser()
            self._iniFile.parse()

            expected = test[1]
            expectedDownloadCount = test[2]

            result = []
            graphs = self._parser.commandGraphs
            for modObj in graphs:
                graphStr = graphs[modObj].toStr()
                if (graphStr):
                    result.append(graphs[modObj].toStr())

            downloadGraphs = self._parser.downloadResourceGraphs
            for modObj in downloadGraphs:
                for reg in downloadGraphs[modObj]:
                    graphStr = downloadGraphs[modObj][reg].toStr()
                    if (graphStr):
                        result.append(graphStr)

            result = "\n\n".join(result)

            self.assertEqual(result, expected)
            self.assertEqual(len(self._iniFile.fileDownloads), expectedDownloadCount)

    def test_textureOverrideRootFoundByKVP_parsedDataFromIniTxt(self):
        tests = [
                 [self._defaultIniTxt, 
"""[TextureOverrideRaidenBlendRemapFix]
handling2 = skip
drawindexed2 = auto
ps-t0 = ResourceRaidenTestDiffuseRemapDL
ps-t1 = ResourceRaidenTestLightMapRemapDL
ib = null

[TextureOverrideRaidenTexcoordRemapFix]
vb0 = ResourceRaidenTestPositionRemapDL
vb1 = ResourceRaidenTestTextureRemapDL

[ResourceRaidenTestPositionRemapDL]
filename = anotherBaseFile

[ResourceRaidenTestTextureRemapDL]
filename = someBaseFile

[ResourceRaidenTestDiffuseRemapDL]
filename = unknownBaseFile

[ResourceRaidenTestLightMapRemapDL]
filename = uniqueBaseFile""", 4],

[
"""
[TextureOverridelittleblacknekowitchBlend]
hash = rikaTheWitchOfFate

[GoogooGaaGaaBlend]
hash = kuroneko

[NanaTex]
hash = rena's going to take you home3

[DaDaIb]
hash = Himatsubushi-hen

[DaDaIb2]
hash = Himatsubushi-hen
match_first_index = protocolSignalGenerator

[DaDaBody]
hash = Himatsubushi-hen
match_first_index = uryu uryu! Slap by Rosa...
"""
,
"""[GoogooGaaGaaBlend]
handling2 = skip
drawindexed2 = auto
ps-t0 = ResourceBernkastelTestDiffuseRemapDL
hash = kuroneko
ps-t1 = ResourceBernkastelTestLightMapRemapDL
ib = null

[NanaTex]
hash = rena's going to take you home3
vb0 = ResourceBernkastelTestPositionRemapDL

[DaDaBody]
hash = Himatsubushi-hen
match_first_index = uryu uryu! Slap by Rosa...
ps-t999 = DigitOverflow

[DaDaIb]
hash = Matsuribayashi-hen
ps-t2,147,483,647 = DigitUnderflow

[DaDaIb2]
hash = Himatsubushi-hen
match_first_index = protocolSignalGenerator
ps-t2,147,483,647 = DigitUnderflow

[ResourceBernkastelTestPositionRemapDL]
filename = anotherBaseFile

[ResourceBernkastelTestTextureRemapDL]
filename = someBaseFile

[ResourceBernkastelTestDiffuseRemapDL]
filename = unknownBaseFile

[ResourceBernkastelTestLightMapRemapDL]
filename = uniqueBaseFile""", 4]]

        for test in tests:
            iniTxt = test[0]
            self.setupIniTxt(iniTxt)
            
            self.createKeyedParser()
            self._parser.trackKeys = True
            self._parser.keysToTrack = {FRB.IniKeywords.Hash.value, FRB.IniKeywords.MatchFirstIndex.value}

            self._iniFile.parse()

            expected = test[1]
            expectedDownloadCount = test[2]

            result = []
            graphs = self._parser.commandGraphs
            for modObj in graphs:
                graphStr = graphs[modObj].toStr()
                if (graphStr):
                    result.append(graphStr)

            downloadGraphs = self._parser.downloadResourceGraphs
            for modObj in downloadGraphs:
                for reg in downloadGraphs[modObj]:
                    downloadStr = downloadGraphs[modObj][reg].toStr()
                    if (downloadStr):
                        result.append(downloadStr)

            result = "\n\n".join(result)

            self.assertEqual(result, expected)
            self.assertEqual(len(self._iniFile.fileDownloads), expectedDownloadCount)

    # ====================================================================
    # ==================== structure / attributes ========================

    def test_parser_isABaseIniParser(self):
        self.create()

        # The C++ GIMIParser is registered with BaseIniParser as its real pybind11 base, so this is
        # genuine inheritance, not just a documented claim.
        self.assertIsInstance(self._parser, FRB.BaseIniParser)
        self.assertIs(self._parser._iniFile, self._iniFile)

    def test_modObjs_isTheCallersOwnObject(self):
        self.create()
        modObjs = OrderedSet([("bang", "B"), ("", "head")])

        self._parser.modObjs = modObjs
        self.assertIs(self._parser.modObjs, modObjs)

    def test_components_derivedFromModObjs(self):
        self.create()
        self._parser.modObjs = OrderedSet([("bang", "B"), ("bang", "C"), ("", "head")])
        self.compareSet(self._parser.components, {"bang", ""})

    def test_objTargetFuncs_isTheCallersOwnList(self):
        self.create()
        funcs = []

        self._parser.objTargetFuncs = funcs
        self.assertIs(self._parser.objTargetFuncs, funcs)

    def test_downloads_isTheCallersOwnDict(self):
        self.create()
        downloads = {}

        self._parser.downloads = downloads
        self.assertIs(self._parser.downloads, downloads)

    def test_commandGraphs_isTheSameDictEveryAccess(self):
        self.create()

        # editCommands() hands this exact dict to an IniGraphGroup and reads it back out, so the
        # aliasing has to survive -- a fresh copy per access would silently break every edit.
        self.assertIs(self._parser.commandGraphs, self._parser.commandGraphs)

    def test_commandGraphs_assignable(self):
        self.create()
        graphs = {}

        self._parser.commandGraphs = graphs
        self.assertIs(self._parser.commandGraphs, graphs)

    def test_tempKwargs_startsEmptyAndIsClearedByClear(self):
        self.create()
        self.compareDict(self._parser.tempKwargs, {})

        self._parser.tempKwargs["scratch"] = 42
        self.assertEqual(self._parser.tempKwargs["scratch"], 42)

        self._parser.clear()
        self.compareDict(self._parser.tempKwargs, {})

    def test_clear_emptiesTheParsedGraphs(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createNamedParser()
        self._iniFile.parse()

        self.assertTrue(len(self._parser.commandGraphs) > 0)

        self._parser.clear()
        self.compareDict(self._parser.commandGraphs, {})
        self.compareDict(self._parser.downloadResourceGraphs, {})
        self.assertIsNone(self._parser.globalGraph)

    # ====================================================================
    # ============== classifyByTextureOverrideName =======================

    def test_classifyByTextureOverrideName_matchingSuffix_classified(self):
        self.create()
        result = FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "TextureOverrideRaidenShogunBlend")
        self.compareList(result, [("", "blend")])

    def test_classifyByTextureOverrideName_caseAndWhitespaceInsensitive(self):
        self.create()
        result = FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "   textureoverrideRAIDENSHOGUNblend  ")
        self.compareList(result, [("", "blend")])

    def test_classifyByTextureOverrideName_alreadyRemapped_notClassified(self):
        self.create()

        # A section this software wrote itself -- 'remap' anywhere after the prefix disqualifies it.
        result = FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "TextureOverrideRaidenShogunRemapBlend")
        self.compareList(result, [])

    def test_classifyByTextureOverrideName_notATextureOverride_notClassified(self):
        self.create()
        self.compareList(FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "CommandListRaidenShogunBlend"), [])
        self.compareList(FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "ResourceRaidenShogunBlend.0"), [])

    def test_classifyByTextureOverrideName_noMatchingModObj_notClassified(self):
        self.create()
        self.compareList(FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "TextureOverrideRaidenShogunDress"), [])

    def test_classifyByTextureOverrideName_matchMustBeASuffix(self):
        self.create()

        # 'blend' occurs, but not at the end -- it names some other object, not this one.
        self.compareList(FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "TextureOverrideRaidenBlendExtras"), [])

    def test_classifyByTextureOverrideName_explicitModObjs_overridesTheParsers(self):
        self.create()
        result = FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "TextureOverrideHuTaoBody",
                                                              modObjs = OrderedSet([("", "Body")]))
        self.compareList(result, [("", "Body")])

    def test_classifyByTextureOverrideName_componentAndObjectConcatenated(self):
        self.create()
        result = FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "TextureOverrideYelanBangB",
                                                              modObjs = OrderedSet([("Bang", "B")]))
        self.compareList(result, [("Bang", "B")])

    def test_classifyByTextureOverrideName_fromRoots_buildsTheGlobalGraph(self):
        self.create()
        self._parser.clear()
        self.assertIsNone(self._parser.globalGraph)

        FRB.GIMIParser.classifyByTextureOverrideName(self._parser, "TextureOverrideRaidenShogunBlend", fromRoots = True)
        self.assertIsNotNone(self._parser.globalGraph)

    # ====================================================================
    # ========================= parse's result ===========================

    def _parseResult(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createNamedParser()
        return self._parser.parse()

    def test_parse_returnsExactlyOneGraphGroup(self):
        result = self._parseResult()

        self.assertIsInstance(result, list)
        self.assertEqual(len(result), 1)
        self.assertIsInstance(result[0], FRB.IniGraphGroup)

    def test_parse_groupHoldsCommandGraphsThenDownloadGraphs(self):
        result = self._parseResult()
        graphs = result[0].graphs

        # Command graphs keep their own (component, mod object) key; download resource graphs get
        # the reserved "download" component plus the download's own name.
        expected = [("", "blend"), ("", "texcoord"), ("", "body"), ("", "ib")]
        expected = [modObj for modObj in expected if modObj in self._parser.commandGraphs]
        expected += [(FRB.IniGraphModObjKeywords.Download.value, name)
                     for name in ["testPosition", "testTexture", "testDiffuse", "testLightMap"]]

        self.compareList(list(graphs.keys()), expected)

    def test_parse_groupSharesTheParsersOwnGraphObjects(self):
        result = self._parseResult()
        graphs = result[0].graphs

        for modObj in self._parser.commandGraphs:
            # Identity: a Python IniGraphGroup holds references, so nothing is copied on the way out.
            self.assertIs(graphs[modObj], self._parser.commandGraphs[modObj])

        downloadGraphs = self._parser.downloadResourceGraphs
        for modObj in downloadGraphs:
            for reg in downloadGraphs[modObj]:
                name = self._parser.downloads[modObj][reg].name
                self.assertIs(graphs[(FRB.IniGraphModObjKeywords.Download.value, name)], downloadGraphs[modObj][reg])

    def test_parse_groupsDictIsFresh_notCommandGraphsItself(self):
        result = self._parseResult()

        # Adding to the returned group must not also add to the parser's own commandGraphs.
        self.assertIsNot(result[0].graphs, self._parser.commandGraphs)

        before = len(self._parser.commandGraphs)
        result[0].graphs[("scratch", "entry")] = None
        self.assertEqual(len(self._parser.commandGraphs), before)

    def test_parse_oneDownloadSharedByTwoRegisters_builtAndDownloadedOnce(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createNamedParser()

        # The same DownloadData under two registers of one mod object. A .ini file can only hold
        # one section of a given name, so it is built -- and downloaded -- once, and both
        # registers' resource graphs point at that one section.
        shared = FRB.DownloadData("sharedDownload", FRB.FileDownload("sharedURL", "sharedBaseFile"))
        self._parser.downloads = {("", "texcoord"): {"vb0": shared, "vb1": shared}}

        self._iniFile.parse()

        self.assertEqual(len(self._iniFile.fileDownloads), 1)

        graphs = self._parser.collectParseResult()[0].graphs
        downloadKeys = [modObj for modObj in graphs if modObj[0] == FRB.IniGraphModObjKeywords.Download.value]
        self.compareList(downloadKeys, [(FRB.IniGraphModObjKeywords.Download.value, "sharedDownload")])

        resourceGraphs = self._parser.downloadResourceGraphs[("", "texcoord")]
        self.assertIsNot(resourceGraphs["vb0"], resourceGraphs["vb1"])
        self.compareList(sorted(resourceGraphs["vb0"].sections.keys()),
                         sorted(resourceGraphs["vb1"].sections.keys()))

    def test_parse_downloadGraphsKeyedByTheDownloadsNameNotItsRegister(self):
        result = self._parseResult()
        graphs = result[0].graphs

        # The register a download is referenced from ("vb0", "ps-t1", ...) never appears in the
        # key -- only the download's own name does, so the same resource reached from two places
        # would be one entry.
        downloadKeys = [modObj for modObj in graphs if modObj[0] == FRB.IniGraphModObjKeywords.Download.value]
        self.compareList(sorted(name for _, name in downloadKeys),
                         sorted(["testPosition", "testTexture", "testDiffuse", "testLightMap"]))

    def test_parse_noDownloads_groupIsJustTheCommandGraphs(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.createNamedParser()
        self._parser.downloads = {}

        graphs = self._parser.parse()[0].graphs
        self.compareList(list(graphs.keys()), list(self._parser.commandGraphs.keys()))

    # ====================================================================
