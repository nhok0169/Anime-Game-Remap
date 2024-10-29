import sys

from .baseIniObjTest import BaseIniObjTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class GIMIObjParserTest(BaseIniObjTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None

    def createParser(self):
        self._parser = FRB.GIMIObjParser(self._iniFile, {"body"})

    def create(self):
        self.createIniFile()
        self.createParser()
        self._iniFile._iniParser = self._parser

    # ====================== parse =======================================

    def test_textureOverrideRootFound_parsedDataFromIniTxt(self):
        self.setupIniTxt(self._defaultIniTxt)
        self.create()
        self._iniFile.parse()
        expectedBlendCommands = {"TextureOverrideKeqingOpulentBlend": FRB.IfTemplate([{"run": "CommandListKeqingOpulentBlend",
                                                                                      "hash": "6f010b58"}], {0: "CommandListKeqingOpulentBlend"}),
                                 "CommandListKeqingOpulentBlend": FRB.IfTemplate(["                    if $swapvar == 0\n",
                                                                                        {"vb1": "ResourceKeqingOpulentBlend.0",
                                                                                         "handling": "skip",
                                                                                         "draw": "125644,0"},
                                                                                    "                    else if $swapvar == 1\n",
                                                                                        {"vb1": "ResourceKeqingOpulentBlend.1",
                                                                                         "handling": "skip",
                                                                                         "draw": "129460,0"},
                                                                                 "                    endif\n"])}
        expectedBlendRemapNames = {"TextureOverrideKeqingOpulentBlend": {"Keqing": "TextureOverrideKeqingOpulentKeqingRemapBlend"},
                                   "CommandListKeqingOpulentBlend": {"Keqing": "CommandListKeqingOpulentKeqingRemapBlend"}}
        
        expectedResourceCommands = {"ResourceKeqingOpulentBlend.0": FRB.IfTemplate([{"type": "Buffer",
                                                                                    "stride": "32",
                                                                                    "filename": ".\Keqing 0\KeqingOpulentBlend.buf"}]),
                                    "ResourceKeqingOpulentBlend.1": FRB.IfTemplate([{"type": "Buffer",
                                                                                    "stride": "32",
                                                                                    "filename": ".\Keqing 1\KeqingOpulentBlend.buf"}])}
        expectedResourceCommandsRemapNames = {"ResourceKeqingOpulentBlend.0": {"Keqing": "ResourceKeqingOpulentKeqingRemapBlend.0"},
                                              "ResourceKeqingOpulentBlend.1": {"Keqing": "ResourceKeqingOpulentKeqingRemapBlend.1"}}
        
        expectedNonBlendHashCommands = {"TextureOverrideKeqingOpulentPosition": FRB.IfTemplate([{"hash": "0d7e3cc5",
                                                                                                 "run": "CommandListKeqingOpulentPosition",
                                                                                                 "$active": "1"}], {0: "CommandListKeqingOpulentPosition"}),
                                        "CommandListKeqingOpulentPosition": FRB.IfTemplate(["                    if $swapvar == 0\n",
                                                                                            {"vb0": "ResourceKeqingOpulentPosition.0",
                                                                                             "$ActiveCharacter": "1"},
                                                                                             "                    else if $swapvar == 1\n",
                                                                                             {"vb0": "ResourceKeqingOpulentPosition.1",
                                                                                              "$ActiveCharacter": "1"},
                                                                                              "                    endif\n"]),
                                        "TextureOverrideKeqingOpulentTexcoord": FRB.IfTemplate([{"hash": "52f78cb7", 
                                                                                                 "run": "CommandListKeqingOpulentTexcoord"}], {0: "CommandListKeqingOpulentTexcoord"}),
                                        "CommandListKeqingOpulentTexcoord": FRB.IfTemplate(["                    if $swapvar == 0\n",
                                                                                            {"vb1": "ResourceKeqingOpulentTexcoord.0"},
                                                                                            "                    else if $swapvar == 1\n", 
                                                                                            {"vb1": "ResourceKeqingOpulentTexcoord.1"},
                                                                                            "                    endif\n"]),
                                        "TextureOverrideKeqingOpulentVertexLimitRaise": FRB.IfTemplate([{"hash": "efcc8769"}]),
                                        "TextureOverrideKeqingOpulentIB": FRB.IfTemplate([{"hash": "7c6fc8c3",
                                                                                           "run": "CommandListKeqingOpulentIB"}], {0: "CommandListKeqingOpulentIB"}),
                                        "CommandListKeqingOpulentIB": FRB.IfTemplate(["                    if $swapvar == 0\n",
                                                                                      {"handling": "skip",
                                                                                       "drawindexed": "auto"},
                                                                                       "                    else if $swapvar == 1\n",
                                                                                      {"handling": "skip",
                                                                                       "drawindexed": "auto"},
                                                                                       "                    endif\n"]),
                                        "TextureOverrideKeqingOpulentHead": FRB.IfTemplate([{"hash": "7c6fc8c3",
                                                                                             "match_first_index": "0",
                                                                                             "run": "CommandListKeqingOpulentHead"}], {0: "CommandListKeqingOpulentHead"}),
                                        "CommandListKeqingOpulentHead": FRB.IfTemplate(["                    if $swapvar == 0\n",
                                                                                        {"ib": "ResourceKeqingOpulentHeadIB.0",
                                                                                         "ps-t0": "ResourceKeqingOpulentHeadDiffuse.0",
                                                                                         "ps-t1": "ResourceKeqingOpulentHeadLightMap.0"},
                                                                                         "                    else if $swapvar == 1\n",
                                                                                         {"ib": "ResourceKeqingOpulentHeadIB.1",
                                                                                          "ps-t0": "ResourceKeqingOpulentHeadDiffuse.1",
                                                                                          "ps-t1": "ResourceKeqingOpulentHeadLightMap.1"},
                                                                                          "                    endif\n"]),
                                        "TextureOverrideKeqingOpulentBody": FRB.IfTemplate([{"hash": "7c6fc8c3",
                                                                                             "match_first_index": "19623",
                                                                                             "run": "CommandListKeqingOpulentBody"}], {0: "CommandListKeqingOpulentBody"}),
                                        "CommandListKeqingOpulentBody": FRB.IfTemplate(["                    if $swapvar == 0\n",
                                                                                        {"ib": "ResourceKeqingOpulentBodyIB.0",
                                                                                         "ps-t0": "ResourceKeqingOpulentBodyDiffuse.0",
                                                                                         "ps-t1": "ResourceKeqingOpulentBodyLightMap.0"},
                                                                                         "                    else if $swapvar == 1\n",
                                                                                         {"ib": "ResourceKeqingOpulentBodyIB.1",
                                                                                          "ps-t0": "ResourceKeqingOpulentBodyDiffuse.1",
                                                                                          "ps-t1": "ResourceKeqingOpulentBodyLightMap.1"},
                                                                                          "                    endif\n"]),
                                        "TextureOverrideKeqingOpulentFaceHeadDiffuse": FRB.IfTemplate([{"hash": "c2b17f84",
                                                                                                        "run": "CommandListKeqingOpulentFaceHeadDiffuse"}], {0: "CommandListKeqingOpulentFaceHeadDiffuse"}),
                                        "CommandListKeqingOpulentFaceHeadDiffuse": FRB.IfTemplate(["                    if $swapvar == 0\n",
                                                                                                   {"ps-t0": "ResourceKeqingOpulentFaceHeadDiffuse.0"},
                                                                                                   "                    else if $swapvar == 1\n",
                                                                                                   {"ps-t0": "ResourceKeqingOpulentFaceHeadDiffuse.1"},
                                                                                                   "                    endif\n"]),
                                        "TextureOverride41FixVertexLimitRaise": FRB.IfTemplate([{"hash": "6629a84e"}])}
        
        expectedNonBlendHashRemapNames = {"TextureOverrideKeqingOpulentPosition": {"Keqing": "TextureOverrideKeqingOpulentPositionKeqingRemapFix"},
                                          "CommandListKeqingOpulentPosition": {"Keqing": "CommandListKeqingOpulentPositionKeqingRemapFix"},
                                          "TextureOverrideKeqingOpulentTexcoord": {"Keqing": "TextureOverrideKeqingOpulentTexcoordKeqingRemapFix"},
                                          "CommandListKeqingOpulentTexcoord": {"Keqing": "CommandListKeqingOpulentTexcoordKeqingRemapFix"},
                                          "TextureOverrideKeqingOpulentVertexLimitRaise": {"Keqing": "TextureOverrideKeqingOpulentVertexLimitRaiseKeqingRemapFix"},
                                          "TextureOverrideKeqingOpulentIB": {"Keqing": "TextureOverrideKeqingOpulentIBKeqingRemapFix"},
                                          "CommandListKeqingOpulentIB": {"Keqing": "CommandListKeqingOpulentIBKeqingRemapFix"},
                                          "TextureOverrideKeqingOpulentHead": {"Keqing": "TextureOverrideKeqingOpulentHeadKeqingRemapFix"},
                                          "CommandListKeqingOpulentHead": {"Keqing": "CommandListKeqingOpulentHeadKeqingRemapFix"},
                                          "TextureOverrideKeqingOpulentBody": {"Keqing": "TextureOverrideKeqingOpulentBodyKeqingRemapFix"},
                                          "CommandListKeqingOpulentBody": {"Keqing": "CommandListKeqingOpulentBodyKeqingRemapFix"},
                                          "TextureOverrideKeqingOpulentFaceHeadDiffuse": {"Keqing": "TextureOverrideKeqingOpulentFaceHeadDiffuseKeqingRemapFix"},
                                          "CommandListKeqingOpulentFaceHeadDiffuse": {"Keqing": "CommandListKeqingOpulentFaceHeadDiffuseKeqingRemapFix"},
                                          "TextureOverride41FixVertexLimitRaise": {"Keqing": "TextureOverride41FixVertexLimitRaiseKeqingRemapFix"}}
        
        expectedBodyCommands = {"TextureOverrideKeqingOpulentBody": FRB.IfTemplate([{"hash": "7c6fc8c3",
                                                                                     "match_first_index": "19623",
                                                                                     "run": "CommandListKeqingOpulentBody"}], {0: "CommandListKeqingOpulentBody"}),
                                "CommandListKeqingOpulentBody": FRB.IfTemplate(["                    if $swapvar == 0\n",
                                                                                {"ib": "ResourceKeqingOpulentBodyIB.0",
                                                                                    "ps-t0": "ResourceKeqingOpulentBodyDiffuse.0",
                                                                                    "ps-t1": "ResourceKeqingOpulentBodyLightMap.0"},
                                                                                    "                    else if $swapvar == 1\n",
                                                                                    {"ib": "ResourceKeqingOpulentBodyIB.1",
                                                                                    "ps-t0": "ResourceKeqingOpulentBodyDiffuse.1",
                                                                                    "ps-t1": "ResourceKeqingOpulentBodyLightMap.1"},
                                                                                    "                    endif\n"])}

        self.compareDictIfTemplate(self._parser.blendCommandsGraph.sections, expectedBlendCommands)
        self.compareDictOfDict(self._parser.blendCommandsGraph.remapNames, expectedBlendRemapNames)
        self.compareDictIfTemplate(self._parser.resourceCommandsGraph.sections, expectedResourceCommands)
        self.compareDictOfDict(self._parser.resourceCommandsGraph.remapNames, expectedResourceCommandsRemapNames)
        self.compareDictIfTemplate(self._parser.nonBlendHashIndexCommandsGraph.sections, expectedNonBlendHashCommands)
        self.compareDictOfDict(self._parser.nonBlendHashIndexCommandsGraph.remapNames, expectedNonBlendHashRemapNames)
        self.compareDictIfTemplate(self._parser.objGraphs["body"].sections, expectedBodyCommands)
        self.compareDictOfDict(self._parser.objGraphs["body"].remapNames, {})

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