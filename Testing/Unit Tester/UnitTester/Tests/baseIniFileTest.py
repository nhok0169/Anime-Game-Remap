import os
import shutil
import sys
import tempfile
from typing import List, Dict, Union, Tuple

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseIniFileTest(BaseUnitTest):
    """
    Base for tests that need a real :class:`FRB.IniFile` over :attr:`_iniTxt`.

    Everything here is the C++-backed API:

    - The .ini text is written to a real file in a per-test temporary folder. The C++
      :class:`FRB.IniFile` reads and writes through ``std::filesystem``, so Python-level mocks of
      ``open``/``FileService.read`` would be bypassed silently. That is also why this derives from
      :class:`BaseUnitTest` and not :class:`BaseFileUnitTest`: the latter's fake folder tree mocks
      ``os.path``/``os.remove``/``os.walk`` (and overwrites ``os.sep``), which the C++ side never
      sees and which break ``tempfile``.
    - The classifier is this class's OWN :class:`FRB.IniClassifier`, holding Raiden plus two made-up
      mod types, so nothing here depends on (or disturbs) the global classifier other tests use.
    - The made-up mod types (``rika``, ``kyrie``) are runtime :class:`FRB.ModType` objects under ids no
      shipped mod type uses, handed to each :class:`FRB.IniFile` through ``overrideModTypes`` rather
      than registered globally.
    """

    RikaModTypeId = 1000001
    KyrieModTypeId = 1000002

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        # Another test class clears the global registry; Raiden is resolved through it.
        FRB.CppGlobalModTypes.registerAll()

        gi = int(FRB.GameTypeId.GI)
        cls._raidenModTypeId = int(FRB.ModTypeId.Raiden)

        cls._iniFile = None
        cls._tempFolder = None
        cls._file = None

        cls._setupCustomModTypes()
        cls._overrideModTypes = {modType.modTypeId: modType for modType in cls._customModTypes.values()}
        cls._raidenModType = next(modType for modType in FRB.CppGlobalModTypes.all() if modType.modTypeId == cls._raidenModTypeId)

        # The made-up mod types borrow Raiden's builders. Those are the shipped TABLE builders, which
        # consult CppStrategyOverrides by mod name first -- that is how useStrategies below puts a
        # test's own parser/fixer behind IniFile.parse()/fix() for whichever of the three a .ini file
        # classifies as.
        for modType in cls._customModTypes.values():
            modType.iniParseBuilder = cls._raidenModType.iniParseBuilder
            modType.iniFixBuilder = cls._raidenModType.iniFixBuilder
            modType.iniRemoveBuilder = cls._raidenModType.iniRemoveBuilder

        cls._iniClassifier = FRB.IniClassifier()
        cls._iniClassifier.addGIModType(FRB.ModTypeIdData(gi, cls._raidenModTypeId), set(), {"raiden", "shogun"})
        cls._iniClassifier.addGIModType(FRB.ModTypeIdData(gi, cls.RikaModTypeId), set(), {"littleblacknekowitch"})
        cls._iniClassifier.addGIModType(FRB.ModTypeIdData(gi, cls.KyrieModTypeId), set(), {"agnusdei"})

        # What the .ini file may be classified as, and what it falls back to when nothing matches
        cls._modTypes = {cls._raidenModTypeId, cls.RikaModTypeId}
        cls._defaultModType = cls._customModTypes["kyrie"]
        cls._defaultIniTxt = r"""
                    [Constants]
                    global persist $swapvar = 0
                    global persist $swapvarn = 0
                    global persist $swapmain = 0
                    global persist $swapoffice = 0
                    global persist $swapglasses = 0

                    [KeyVar]
                    condition = $active == 1
                    key = VK_DOWN
                    type = cycle
                    $swapvar = 0,1,2

                    [KeyIntoTheHole]
                    condition = $active == 1
                    key = VK_RIGHT
                    type = cycle
                    $swapvarn = 0,1

                    ; The top part is not really important, so I not going to finish
                    ;   typing all the key swaps... 😋
                    ;
                    ; The bottom part is what the fix actually cares about

                    [TextureOverrideRaidenShogunBlend]
                    run = CommandListRaidenShogunBlend
                    handling = skip
                    draw = 21916,0

                    [CommandListRaidenShogunBlend]
                    if $swapmain == 0
                        if $swapvar == 0 && $swapvarn == 0
                            vb1 = ResourceRaidenShogunBlend.0
                        else
                            vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
                        endif
                    else if $swapmain == 1
                        run = SubSubTextureOverride
                    endif

                    [SubSubTextureOverride]
                    if $swapoffice == 0 && $swapglasses == 0
                        vb1 = GIMINeedsResourcesToAllStartWithResource
                    endif

                    [ResourceRaidenShogunBlend.0]
                    type = Buffer
                    stride = 32
                    filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

                    [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
                    type = Buffer
                    stride = 32
                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEi.buf
                    else
                        run = RaidenPuppetCommandResource
                    endif

                    [GIMINeedsResourcesToAllStartWithResource]
                    type = Buffer
                    stride = 32
                    filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

                    [TextureOverrideRaidenShogunRemapBlend]
                    run = CommandListRaidenShogunRemapBlend
                    handling = skip
                    draw = 21916,0
                    [RaidenPuppetCommandResource]
                    type = Buffer
                    stride = 32
                    filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

                    ; ------ some lines originally generated from the fix ---------

                    [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
                    ; she drank the smoothie

                    type = Buffer
                    stride = 32

                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf

                    else
                        run = RaidenPuppetCommandResourceRemapBlend
                    endif

                    [ResourceRaidenShogunRemapBlend.0]
                    type = Buffer
                    stride = 32

                    ; where does this go?
                    filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

                    [RaidenPuppetCommandResourceRemapBlend]
                    type = Buffer
                    stride = 32

                    # for some reason, GIMI does not work as what you expect for this case
                    filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

                    ; --------------------------------------------------------------


                    ; --------------- Raiden Boss Fix -----------------
                    ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
                    ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

                    [TextureOverrideRaidenShogunRemapBlend]
                    run = CommandListRaidenShogunRemapBlend
                    handling = skip
                    draw = 21916,0

                    [CommandListRaidenShogunRemapBlend]

                    # main swap
                    if $swapmain == 0

                        # some other subvariable swap
                        if $swapvar == 0 && $swapvarn == 0
                            vb1 = ResourceRaidenShogunRemapBlend.0

                        ; ruin the smoothie
                        else
                            vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
                        endif

                    ; some boring swap
                    else if $swapmain == 1
                        run = SubSubTextureOverrideRemapBlend
                    endif

                    [SubSubTextureOverrideRemapBlend]
                    if $swapoffice == 0 && $swapglasses == 0
                        vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
                    endif


                    [GIMINeedsResourcesToAllStartWithResourceRemapBlend]
                    type = Buffer
                    stride = 32
                    filename = ..\AAA\BBBB\CCCCCC\DDDDDRemapRemapBlend.buf

                    [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
                    type = Buffer
                    stride = 32
                    if $swapmain == 1
                        filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
                    else
                        run = RaidenPuppetCommandResourceRemapBlend
                    endif

                    [ResourceRaidenShogunRemapBlend.0]
                    type = Binaries
                    stride = 31
                    filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

                    [RaidenPuppetCommandResourceRemapBlend]
                    type = Buffer
                    stride = 32
                    filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


                    ; -------------------------------------------------"""

        cls._iniTxtLines = []
        cls.setupIniTxt(cls._defaultIniTxt)


    @classmethod
    def _setupCustomModTypes(cls):
        gi = int(FRB.GameTypeId.GI)

        rikaHashes = FRB.Hashes({"rika": ["rika"]})
        rikaHashes.addRepoRows([(["1.0", "rika", "blend_vb"], "kuroneko"), (["1.0", "rika", "draw_vb"], "hanyu"),
                                (["1.0", "rika", "texcoord_vb"], "rena's going to take you home"),
                                (["2.0", "rika", "blend_vb"], "nipah nipah!2"), (["2.0", "rika", "draw_vb"], "hanyu2"),
                                (["2.0", "rika", "ib"], "Himatsubushi-hen"),
                                (["3.0", "rika", "blend_vb"], "nipah nipah!3"), (["3.0", "rika", "texcoord_vb"], "rena's going to take you home3")])

        rikaIndices = FRB.Indices({"rika": ["rika"]})
        rikaIndices.addRepoRows([(["2.3", "rika", "", "head"], "macaron"), (["3.7", "rika", "", "body"], "uryu uryu! Slap by Rosa...")])

        kyrieHashes = FRB.Hashes({"kyrie": ["kyrie"]})
        kyrieHashes.addRepoRows([(["2.0", "kyrie", "blend_vb"], "Dies Irae"), (["2.3", "kyrie", "blend_vb"], "gloria"),
                                 (["2.4", "kyrie", "blend_vb"], "sanctus"), (["2.5", "kyrie", "blend_vb"], "credo")])

        kyrieIndices = FRB.Indices({"kyrie": ["kyrie"]})
        kyrieIndices.addRepoRows([(["3.0", "kyrie", "", "head"], "eleison"), (["3.9", "kyrie", "", "head"], "missa tota")])

        cls._customModTypes = {"rika": FRB.ModType(gi, cls.RikaModTypeId, "Bernkastel", ["Frederica Bernkastel", "Bern-chan", "Rika Furude", "Nipah!"],
                                                   rikaHashes, rikaIndices),
                               "kyrie": FRB.ModType(gi, cls.KyrieModTypeId, "Kyrie", [], kyrieHashes, kyrieIndices)}

    @classmethod
    def setupIniTxt(cls, newIniTxt: str):
        cls._iniTxt = newIniTxt
        cls._iniTxtLines = cls._iniTxt.splitlines(keepends = True)

    def writeIniTxt(self, txt: str):
        """Puts 'txt' on disk as this test's .ini file -- the C++ IniFile reads it from there"""
        with open(self._file, "w", encoding = FRB.FileEncodings.UTF8.value, newline = "") as f:
            f.write(txt)

    def createIniFile(self):
        self.writeIniTxt(self._iniTxt)
        self._iniFile = FRB.IniFile(file = self._file, iniClassifier = self._iniClassifier,
                                    filteredFromModTypeIds = self._modTypes, overrideModTypes = self._overrideModTypes)
        self._iniFile.defaultModTypeIds = [self._defaultModType.modTypeId]

    def compareIniFixResourceModel(self, model1: FRB.IniFixResourceModel, model2: FRB.IniFixResourceModel):
        self.assertEqual(model1.iniFolderPath, model2.iniFolderPath)
        self.compareDictOfDict(model1.fixedPaths, model2.fixedPaths)

        assert((model1.origPaths is not None and model2.origPaths is not None) or (model1.origPaths is None and model2.origPaths is None))
        if (model1.origPaths is not None):
            self.compareDict(model1.origPaths, model2.origPaths)
    
    def compareIfTemplateParts(self, resultParts: List[FRB.IfTemplatePart], expectedParts: List[FRB.IfTemplatePart]):
        resultLen = len(resultParts)
        expectedLen = len(expectedParts)
        
        if (resultLen != expectedLen):
            self.fail(self.getDataFailMsg(resultParts, expectedParts, f"IfTemplates have different nunmber of parts: resultSet: {resultLen}, expectedSet: {expectedLen}"))
        
        for i in range(resultLen):
            part = resultParts[i]
            expectedPart = expectedParts[i]

            self.assertIs(type(part), type(expectedPart))
            if (isinstance(part, FRB.IfPredPart)):
                self.compareIfPredPart(part, expectedPart)
            elif (isinstance(part, FRB.IfContentPart)):
                self.compareIfContentPart(part, expectedPart)

    def compareIfTemplate(self, result: FRB.IfTemplate, expected: FRB.IfTemplate):
        self.compareIfTemplateParts(result.parts, expected.parts)
        
        assert((result.calledSubCommands is None and expected.calledSubCommands is None) or (result.calledSubCommands is not None and expected.calledSubCommands is not None))
        if (result.calledSubCommands is not None):
            self.compareDict(result.calledSubCommands, expected.calledSubCommands)

    def compareDictIfTemplate(self, result: Dict[str, FRB.IfTemplate], expected: Dict[str, FRB.IfTemplate]):
        self.assertEqual(len(result), len(expected))
        for resultKey in result:
            self.assertIn(resultKey, expected)

            resultValue = result[resultKey]
            expectedValue = expected[resultKey]
            self.compareIfTemplate(resultValue, expectedValue)

    def compareIfPredPart(self, result: FRB.IfPredPart, expected: FRB.IfPredPart):
        self.assertEqual(result.type, expected.type)
        self.assertEqual(result.src, expected.src)

    def compareIfContentPartSrc(self, result: Dict[str, List[Tuple[int, str]]], expected: Dict[str, List[Tuple[int, str]]]):
        self.compareDict(result, expected, 
                    compareValues = lambda resultValLst, expectedValLst: self.compareList(resultValLst, expectedValLst, 
                                                                                        compareValues = lambda resultValData, expectedValData: self.compareList(resultValData, expectedValData)))
        
    def compareIfContentOrder(self, result: List[Tuple[str, int]], expected: List[Tuple[str, int]]):
        self.compareList(result, expected, compareValues = lambda resultKVP, expectedKVP: self.compareList(resultKVP, expectedKVP))
        
    def compareIfContentPart(self, result: FRB.IfContentPart, expected: FRB.IfContentPart):
        self.assertEqual(result.depth, expected.depth)
        self.compareIfContentPartSrc(result.src, expected.src)
        self.compareIfContentOrder(result._order, expected._order)

    def useStrategies(self, parser = None, fixer = None):
        """
        Makes :meth:`FRB.IniFile.parse`/:meth:`FRB.IniFile.fix` use 'parser'/'fixer' for every mod type
        this class's .ini files can classify as.

        The C++ :class:`FRB.IniFile` builds its strategies from each mod type's builders, so a test's
        own strategy has to be registered where those builders look: :class:`FRB.CppStrategyOverrides`,
        keyed by mod name (and, for a fixer, by every mod it remaps onto). The pure-Python
        ``IniFile`` let a test assign ``_iniParser``/``_iniFixer`` instead; those attributes are gone.
        The overrides are process-wide, so they are cleared when the test ends.
        """

        FRB.CppStrategyOverrides.clear()
        self.addCleanup(FRB.CppStrategyOverrides.clear)

        for modType in [self._raidenModType, *self._customModTypes.values()]:
            if (parser is not None):
                FRB.CppStrategyOverrides.setParser(modType.name, lambda iniFile, modTypeId: parser)

            if (fixer is not None):
                for toModName in modType.getModsToFix():
                    FRB.CppStrategyOverrides.setFixer(modType.name, toModName, lambda fixParser, toModName, modTypeId: fixer)

    def readIniFileOnDisk(self) -> str:
        """What the .ini file on disk says now -- where a fix or a removal that writes back lands"""
        with open(self._file, "r", encoding = FRB.FileEncodings.UTF8.value, newline = "") as f:
            return f.read()

    def setUp(self):
        super().setUp()
        self.maxDiff = None

        # A fresh folder per test: a fix or a removal writes the .ini file back (and may leave a
        # backup beside it), so a shared file would carry one test's result into the next.
        self._tempFolder = tempfile.mkdtemp(prefix = "AGRemapIniTest")
        self._file = FRB.FileService.parseOSPath(os.path.join(self._tempFolder, "CuteLittleEi.ini"))
        self.addCleanup(shutil.rmtree, self._tempFolder, True)