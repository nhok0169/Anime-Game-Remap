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
                    [{"hello": FRB.IfTemplate([])}, {"hello": FRB.RemapBlendModel("", {}, origBlendPaths = {})}],
                    [{"Mahler": FRB.IfTemplate(["Bolero", {"filename": "./hello/world.haku"}, {"filename": "../../Backups/Buffers/Ei.elf"}, "dfdfdf", {"peepeepoopoo": "rip piggy"}]),
                      "Ravel": FRB.IfTemplate([{"Jeux D'eau": "Une piece difficile pour la piano"}]),
                      "Debussy": FRB.IfTemplate([{"Reverie": "Je reve d'etre ailleurs", "filename": "poopoopeepee/piggy rip"}])}, 
                      {"Mahler": FRB.RemapBlendModel("", {1: {modType: "hello/worldKyrieRemapBlend.buf"}, 2: {modType: "../../Backups/Buffers/EiKyrieRemapBlend.buf"}}, origBlendPaths = {1: "hello/world.haku", 2: "../../Backups/Buffers/Ei.elf"}),
                       "Ravel": FRB.RemapBlendModel("", {}, origBlendPaths = {}),
                       "Debussy": FRB.RemapBlendModel("", {0: {modType: "poopoopeepee/piggy ripKyrieRemapBlend.buf"}}, origBlendPaths = {0: "poopoopeepee/piggy rip"})}]]
        
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
                self.compareRemapBlendModel(resultModel, expected[sectionName])

    # ====================================================================
    # ====================== parse =======================================

    def test_textureOverrideRootFound_parsedDataFromIniTxt(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.create()
        self._iniFile.parse()

        expectedBlendCommands = {"TextureOverrideRaidenShogunBlend": FRB.IfTemplate([{"run": "CommandListRaidenShogunBlend",
                                                                                      "handling": "skip",
                                                                                      "draw": "21916,0"}], {0: "CommandListRaidenShogunBlend"}),
                                 "CommandListRaidenShogunBlend": FRB.IfTemplate(["                    if $swapmain == 0\n",
                                                                                    "                        if $swapvar == 0 && $swapvarn == 0\n",
                                                                                        {"vb1": "ResourceRaidenShogunBlend.0"},
                                                                                    "                        else\n",
                                                                                        {"vb1": "ResourceEiBlendsHerBlenderInsteadOfHerSmoothie"},
                                                                                    "                        endif\n",
                                                                                 "                    else if $swapmain == 1\n",
                                                                                    {"run": "SubSubTextureOverride" },
                                                                                 "                    endif\n"], {7: "SubSubTextureOverride"}),
                                 "SubSubTextureOverride": FRB.IfTemplate(["                    if $swapoffice == 0 && $swapglasses == 0\n",
                                                                            {"vb1": "GIMINeedsResourcesToAllStartWithResource"},
                                                                          "                    endif\n"])}
        expectedBlendRemapNames = {"TextureOverrideRaidenShogunBlend": {"RaidenBoss": "TextureOverrideRaidenShogunRaidenBossRemapBlend"},
                                   "CommandListRaidenShogunBlend": {"RaidenBoss": "CommandListRaidenShogunRaidenBossRemapBlend"},
                                   "SubSubTextureOverride": {"RaidenBoss": "SubSubTextureOverrideRaidenBossRemapBlend"}}
        
        expectedResourceCommands = {"ResourceRaidenShogunBlend.0": FRB.IfTemplate([{"type": "Buffer",
                                                                                    "stride": "32",
                                                                                    "filename": "..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf"}]),
                                    "ResourceEiBlendsHerBlenderInsteadOfHerSmoothie": FRB.IfTemplate([{"type": "Buffer",
                                                                                                       "stride": "32"},
                                                                                                       "                    if $swapmain == 1\n",
                                                                                                            {"filename": "M:\AnotherDrive\CuteLittleEi.buf"},
                                                                                                       "                    else\n",
                                                                                                            {"run": "RaidenPuppetCommandResource"},
                                                                                                       "                    endif\n"], {4: "RaidenPuppetCommandResource"}),
                                    "GIMINeedsResourcesToAllStartWithResource": FRB.IfTemplate([{"type": "Buffer",
                                                                                                 "stride": "32",
                                                                                                 "filename": "./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf"}]),
                                    "RaidenPuppetCommandResource": FRB.IfTemplate([{"type": "Buffer",
                                                                                    "stride": "32",
                                                                                    "filename": "./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf"}])}
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