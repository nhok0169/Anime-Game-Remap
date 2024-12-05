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
        expectedBlendCommands = {"TextureOverrideKeqingOpulentBlend": FRB.IfTemplate([FRB.IfContentPart({"run": [(1, "CommandListKeqingOpulentBlend")], "hash": [(0, "6f010b58")]}, 0)], 
                                                                                     {0: "CommandListKeqingOpulentBlend"}),
                                 "CommandListKeqingOpulentBlend": FRB.IfTemplate([FRB.IfPredPart("                    if $swapvar == 0\n", FRB.IfPredPartType.If),
                                                                                        FRB.IfContentPart({"vb1": [(0, "ResourceKeqingOpulentBlend.0")],
                                                                                         "handling": [(1, "skip")],
                                                                                         "draw": [(2, "125644,0")]}, 1),
                                                                                  FRB.IfPredPart("                    else if $swapvar == 1\n", FRB.IfPredPartType.Else),
                                                                                        FRB.IfContentPart({"vb1": [(0, "ResourceKeqingOpulentBlend.1")],
                                                                                         "handling": [(1, "skip")],
                                                                                         "draw": [(2, "129460,0")]}, 1),
                                                                                  FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)])}
        expectedBlendRemapNames = {"TextureOverrideKeqingOpulentBlend": {"Keqing": "TextureOverrideKeqingOpulentKeqingRemapBlend"},
                                   "CommandListKeqingOpulentBlend": {"Keqing": "CommandListKeqingOpulentKeqingRemapBlend"}}
        
        expectedResourceCommands = {"ResourceKeqingOpulentBlend.0": FRB.IfTemplate([FRB.IfContentPart({"type": [(0, "Buffer")],
                                                                                    "stride": [(1, "32")],
                                                                                    "filename": [(2, ".\Keqing 0\KeqingOpulentBlend.buf")]}, 0)]),
                                    "ResourceKeqingOpulentBlend.1": FRB.IfTemplate([FRB.IfContentPart({"type": [(0, "Buffer")],
                                                                                    "stride": [(1, "32")],
                                                                                    "filename": [(2, ".\Keqing 1\KeqingOpulentBlend.buf")]}, 0)])}
        expectedResourceCommandsRemapNames = {"ResourceKeqingOpulentBlend.0": {"Keqing": "ResourceKeqingOpulentKeqingRemapBlend.0"},
                                              "ResourceKeqingOpulentBlend.1": {"Keqing": "ResourceKeqingOpulentKeqingRemapBlend.1"}}
        
        expectedNonBlendHashCommands = {"TextureOverrideKeqingOpulentPosition": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "0d7e3cc5")],
                                                                                                 "run": [(1, "CommandListKeqingOpulentPosition")],
                                                                                                 "$active": [(2, "1")]}, 0)]),
                                        "CommandListKeqingOpulentPosition": FRB.IfTemplate([FRB.IfPredPart("                    if $swapvar == 0\n", FRB.IfPredPartType.If),
                                                                                            FRB.IfContentPart({"vb0": [(0, "ResourceKeqingOpulentPosition.0")],
                                                                                             "$ActiveCharacter": [(1, "1")]}, 1),
                                                                                            FRB.IfPredPart("                    else if $swapvar == 1\n", FRB.IfPredPartType.Else),
                                                                                            FRB.IfContentPart({"vb0": [(0, "ResourceKeqingOpulentPosition.1")],
                                                                                              "$ActiveCharacter": [(1, "1")]}, 1),
                                                                                            FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)]),
                                        "TextureOverrideKeqingOpulentTexcoord": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "52f78cb7")], 
                                                                                                 "run": [(1, "CommandListKeqingOpulentTexcoord")]}, 0)]),
                                        "CommandListKeqingOpulentTexcoord": FRB.IfTemplate([FRB.IfPredPart("                    if $swapvar == 0\n", FRB.IfPredPartType.If),
                                                                                            FRB.IfContentPart({"vb1": [(0, "ResourceKeqingOpulentTexcoord.0")]}, 1),
                                                                                            FRB.IfPredPart("                    else if $swapvar == 1\n", FRB.IfPredPartType.Else), 
                                                                                            FRB.IfContentPart({"vb1": [(0, "ResourceKeqingOpulentTexcoord.1")]}, 1),
                                                                                            FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)]),
                                        "TextureOverrideKeqingOpulentVertexLimitRaise": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "efcc8769")]}, 0)]),
                                        "TextureOverrideKeqingOpulentIB": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "7c6fc8c3")],
                                                                                           "run": [(1, "CommandListKeqingOpulentIB")]}, 0)]),
                                        "CommandListKeqingOpulentIB": FRB.IfTemplate([FRB.IfPredPart("                    if $swapvar == 0\n", FRB.IfPredPartType.If),
                                                                                      FRB.IfContentPart({"handling": [(0, "skip")],
                                                                                       "drawindexed": [(1, "auto")]}, 1),
                                                                                      FRB.IfPredPart("                    else if $swapvar == 1\n", FRB.IfPredPartType.Else),
                                                                                      FRB.IfContentPart({"handling": [(0, "skip")],
                                                                                       "drawindexed": [(1, "auto")]}, 1),
                                                                                      FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)]),
                                        "TextureOverrideKeqingOpulentHead": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "7c6fc8c3")],
                                                                                             "match_first_index": [(1, "0")],
                                                                                             "run": [(2, "CommandListKeqingOpulentHead")]}, 0)]),
                                        "CommandListKeqingOpulentHead": FRB.IfTemplate([FRB.IfPredPart("                    if $swapvar == 0\n", FRB.IfPredPartType.If),
                                                                                        FRB.IfContentPart({"ib": [(0, "ResourceKeqingOpulentHeadIB.0")],
                                                                                         "ps-t0": [(1, "ResourceKeqingOpulentHeadDiffuse.0")],
                                                                                         "ps-t1": [(2, "ResourceKeqingOpulentHeadLightMap.0")]}, 1),
                                                                                        FRB.IfPredPart("                    else if $swapvar == 1\n", FRB.IfPredPartType.Else),
                                                                                        FRB.IfContentPart({"ib": [(0, "ResourceKeqingOpulentHeadIB.1")],
                                                                                          "ps-t0": [(1, "ResourceKeqingOpulentHeadDiffuse.1")],
                                                                                          "ps-t1": [(2, "ResourceKeqingOpulentHeadLightMap.1")]}, 1),
                                                                                        FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)]),
                                        "TextureOverrideKeqingOpulentBody": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "7c6fc8c3")],
                                                                                             "match_first_index": [(1, "19623")],
                                                                                             "run": [(2, "CommandListKeqingOpulentBody")]}, 0)]),
                                        "CommandListKeqingOpulentBody": FRB.IfTemplate([FRB.IfPredPart("                    if $swapvar == 0\n", FRB.IfPredPartType.If),
                                                                                        FRB.IfContentPart({"ib": [(0, "ResourceKeqingOpulentBodyIB.0")],
                                                                                         "ps-t0": [(1, "ResourceKeqingOpulentBodyDiffuse.0")],
                                                                                         "ps-t1": [(2, "ResourceKeqingOpulentBodyLightMap.0")]}, 1),
                                                                                        FRB.IfPredPart("                    else if $swapvar == 1\n", FRB.IfPredPartType.Else),
                                                                                        FRB.IfContentPart({"ib": [(0, "ResourceKeqingOpulentBodyIB.1")],
                                                                                          "ps-t0": [(1, "ResourceKeqingOpulentBodyDiffuse.1")],
                                                                                          "ps-t1": [(2, "ResourceKeqingOpulentBodyLightMap.1")]}, 1),
                                                                                          FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)]),
                                        "TextureOverrideKeqingOpulentFaceHeadDiffuse": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "c2b17f84")],
                                                                                                        "run": [(1, "CommandListKeqingOpulentFaceHeadDiffuse")]}, 0)]),
                                        "CommandListKeqingOpulentFaceHeadDiffuse": FRB.IfTemplate([FRB.IfPredPart("                    if $swapvar == 0\n", FRB.IfPredPartType.If),
                                                                                                   FRB.IfContentPart({"ps-t0": [(0, "ResourceKeqingOpulentFaceHeadDiffuse.0")]}, 1),
                                                                                                   FRB.IfPredPart("                    else if $swapvar == 1\n", FRB.IfPredPartType.Else),
                                                                                                   FRB.IfContentPart({"ps-t0": [(0, "ResourceKeqingOpulentFaceHeadDiffuse.1")]}, 1),
                                                                                                   FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)]),
                                        "TextureOverride41FixVertexLimitRaise": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "6629a84e")]}, 0)])}
        
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
        
        expectedBodyCommands = {"TextureOverrideKeqingOpulentBody": FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "7c6fc8c3")],
                                                                                     "match_first_index": [(1, "19623")],
                                                                                     "run": [(2, "CommandListKeqingOpulentBody")]}, 0)]),
                                "CommandListKeqingOpulentBody": FRB.IfTemplate([FRB.IfPredPart("                    if $swapvar == 0\n", FRB.IfPredPartType.If),
                                                                                FRB.IfContentPart({"ib": [(0, "ResourceKeqingOpulentBodyIB.0")],
                                                                                    "ps-t0": [(1, "ResourceKeqingOpulentBodyDiffuse.0")],
                                                                                    "ps-t1": [(2, "ResourceKeqingOpulentBodyLightMap.0")]}, 1),
                                                                                FRB.IfPredPart("                    else if $swapvar == 1\n", FRB.IfPredPartType.Else),
                                                                                FRB.IfContentPart({"ib": [(0, "ResourceKeqingOpulentBodyIB.1")],
                                                                                    "ps-t0": [(1, "ResourceKeqingOpulentBodyDiffuse.1")],
                                                                                    "ps-t1": [(2, "ResourceKeqingOpulentBodyLightMap.1")]}, 1),
                                                                                FRB.IfPredPart("                    endif\n", FRB.IfPredPartType.EndIf)])}

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