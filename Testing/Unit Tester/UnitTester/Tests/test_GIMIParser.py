import sys, os

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class GIMIParserTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile)

    def create(self):
        self.createIniFile()
        self.createParser()
        self._iniFile._iniParser = self._parser

    # ====================== _makeRemapModels ============================

    def test_differentSavedResourceIfTemplates_remapModelsCreated(self):
        self.create()
        modType = "Kyrie"

        testObjs = [[{}, {}],
                    [{"hello": FRB.IfTemplate([])}, {"hello": FRB.IniResourceModel("", {}, origPaths = {})}],
                    [{"Mahler": FRB.IfTemplate([FRB.IfPredPart("Bolero", FRB.IfPredPartType.If), 
                                                FRB.IfContentPart({"filename": [(0, "./hello/world.haku")]}, 2), 
                                                FRB.IfContentPart({"filename": [(0, "../../Backups/Buffers/Ei.elf")]}, 3), 
                                                FRB.IfPredPart("dfdfdf", FRB.IfPredPartType.EndIf), 
                                                FRB.IfContentPart({"peepeepoopoo": [(0, "rip piggy")]}, 3)]),
                      "Ravel": FRB.IfTemplate([FRB.IfContentPart({"Jeux D'eau": [(0, "Une piece difficile pour la piano")]}, 0)]),
                      "Debussy": FRB.IfTemplate([FRB.IfContentPart({"Reverie": [(0, "Je reve d'etre ailleurs")], "filename": [(1, "poopoopeepee/piggy rip")]}, 0)])}, 
                      {"Mahler": FRB.IniResourceModel("", {1: {modType: ["hello/worldKyrieRemapBlend.buf"]}, 2: {modType: ["../../Backups/Buffers/EiKyrieRemapBlend.buf"]}}, origPaths = {1: ["hello/world.haku"], 2: ["../../Backups/Buffers/Ei.elf"]}),
                       "Ravel": FRB.IniResourceModel("", {}, origPaths = {}),
                       "Debussy": FRB.IniResourceModel("", {0: {modType: ["poopoopeepee/piggy ripKyrieRemapBlend.buf"]}}, origPaths = {0: ["poopoopeepee/piggy rip"]})}]]
        
        for testObj in testObjs:
            self._iniFile.clear()
            self._parser.resourceCommandsGraph._sections = testObj[0]
            self._parser._modsToFix = {modType}
            self._parser._makeRemapModels(self._parser.resourceCommandsGraph)
            expected = testObj[1]
            expectedLen = len(expected)

            self.assertEqual(len(self._iniFile.remapBlendModels), expectedLen)
            self.compareSet(set(self._iniFile.remapBlendModels.keys()), set(expected.keys()))

            for sectionName in self._iniFile.remapBlendModels:
                resultModel = self._iniFile.remapBlendModels[sectionName]
                expected[sectionName].iniFolderPath = os.path.dirname(self._file)
                self.compareIniResourceModel(resultModel, expected[sectionName])

    # ====================================================================
    # ====================== parse =======================================

    def test_textureOverrideRootFound_parsedDataFromIniTxt(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.create()
        self._iniFile.parse()

        expectedBlendCommands = {"TextureOverrideRaidenShogunBlend": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "CommandListRaidenShogunBlend")],
                                                                                      "handling": [(1, "skip")],
                                                                                      "draw": [(2, "21916,0")]}, 0)]),
                                 "CommandListRaidenShogunBlend": FRB.IfTemplate([FRB.IfPredPart("                    if $swapmain == 0\n", FRB.IfPredPartType.If),
                                                                                    FRB.IfPredPart("                        if $swapvar == 0 && $swapvarn == 0\n", FRB.IfPredPartType.If),
                                                                                        FRB.IfContentPart({"vb1": [(0, "ResourceRaidenShogunBlend.0")]}, 2),
                                                                                    FRB.IfPredPart("                        else\n", FRB.IfPredPartType.Else),
                                                                                        FRB.IfContentPart({"vb1": [(0, "ResourceEiBlendsHerBlenderInsteadOfHerSmoothie")]}, 2),
                                                                                    FRB.IfPredPart("                        endif\n", FRB.IfPredPartType.EndIf),
                                                                                 FRB.IfPredPart("                    else if $swapmain == 1\n", FRB.IfPredPartType.Else),
                                                                                    FRB.IfContentPart({"run": [(0, "SubSubTextureOverride")] }, 1),
                                                                                 FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)]),
                                 "SubSubTextureOverride": FRB.IfTemplate([FRB.IfPredPart("                    if $swapoffice == 0 && $swapglasses == 0\n", FRB.IfPredPartType.If),
                                                                            FRB.IfContentPart({"vb1": [(0, "GIMINeedsResourcesToAllStartWithResource")]}, 1),
                                                                          FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)])}
        expectedBlendRemapNames = {"TextureOverrideRaidenShogunBlend": {"RaidenBoss": "TextureOverrideRaidenShogunRaidenBossRemapBlend"},
                                   "CommandListRaidenShogunBlend": {"RaidenBoss": "CommandListRaidenShogunRaidenBossRemapBlend"},
                                   "SubSubTextureOverride": {"RaidenBoss": "SubSubTextureOverrideRaidenBossRemapBlend"}}
        
        expectedResourceCommands = {"ResourceRaidenShogunBlend.0": FRB.IfTemplate([FRB.IfContentPart({"type": [(0, "Buffer")],
                                                                                    "stride": [(1, "32")],
                                                                                    "filename": [(2, "..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf")]}, 0)]),
                                    "ResourceEiBlendsHerBlenderInsteadOfHerSmoothie": FRB.IfTemplate([FRB.IfContentPart({"type": [(0, "Buffer")],
                                                                                                       "stride": [(1, "32")]}, 0),
                                                                                                      FRB.IfPredPart("                    if $swapmain == 1\n", FRB.IfPredPartType.If),
                                                                                                            FRB.IfContentPart({"filename": [(0, "M:\AnotherDrive\CuteLittleEi.buf")]}, 1),
                                                                                                      FRB.IfPredPart("                    else\n", FRB.IfPredPartType.Else),
                                                                                                            FRB.IfContentPart({"run": [(0, "RaidenPuppetCommandResource")]}, 1),
                                                                                                      FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)]),
                                    "GIMINeedsResourcesToAllStartWithResource": FRB.IfTemplate([FRB.IfContentPart({"type": [(0, "Buffer")],
                                                                                                 "stride": [(1, "32")],
                                                                                                 "filename": [(2, "./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf")]}, 0)]),
                                    "RaidenPuppetCommandResource": FRB.IfTemplate([FRB.IfContentPart({"type": [(0, "Buffer")],
                                                                                    "stride": [(1, "32")],
                                                                                    "filename": [(2, "./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf")]}, 0)])}
        expectedResourceCommandsRemapNames = {"ResourceRaidenShogunBlend.0": {"RaidenBoss": "ResourceRaidenShogunRaidenBossRemapBlend.0"},
                                              "ResourceEiBlendsHerBlenderInsteadOfHerSmoothie": {"RaidenBoss": "ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie"},
                                              "GIMINeedsResourcesToAllStartWithResource": {"RaidenBoss": "ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend"},
                                              "RaidenPuppetCommandResource": {"RaidenBoss": "ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend"}}
        
        self.compareDictIfTemplate(self._parser.blendCommandsGraph.sections, expectedBlendCommands)
        self.compareDictOfDict(self._parser.blendCommandsGraph.remapNames, expectedBlendRemapNames)
        self.compareDictIfTemplate(self._parser.resourceCommandsGraph.sections, expectedResourceCommands)
        self.compareDictOfDict(self._parser.resourceCommandsGraph.remapNames, expectedResourceCommandsRemapNames)

        self._iniFile.fileTxt = ""
        self._iniFile.parse()

        self.compareDict(self._parser.blendCommandsGraph.sections, {})
        self.compareDict(self._parser.blendCommandsGraph.remapNames, {})
        self.compareDict(self._parser.resourceCommandsGraph.sections, {})
        self.compareDict(self._parser.resourceCommandsGraph.remapNames, {})
        self.compareList(self._parser.blendCommandsGraph.runSequence, [])
        self.compareList(self._parser.resourceCommandsGraph.runSequence, [])

        # TODO: Add case for getting the sections not related to [TextureOverride.*Blend]

    # ====================================================================