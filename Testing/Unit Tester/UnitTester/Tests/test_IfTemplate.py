import sys
from typing import Dict, Any, Union, Callable, Optional, List

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


_Z3CTX = FRB.Z3Context()  # shared across every IfPredPart built in this test file
Part = Union[str, Dict[str, Any]]


class IfTemplateTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._parts = []
        cls._partsDict = {}
        cls.ifTemplate: FRB.IfTemplate = None

    @classmethod
    def setDefaultAtts(cls):
        cls._parts = [FRB.IfPredPart("some string", FRB.IfPredPartType.If, _Z3CTX),
                      FRB.IfContentPart({"commandName": [(0, "Project")],
                                         "dotnetRunMessages": [(1, "True")],
                                         "launchBrowser": [(2, "True")],
                                         "applicationUrl": [(3, "https://localhost:7152;http://localhost:5105")],
                                         "environmentVariables": [(4, "ASPNETCORE_ENVIRONMENT: Development")]}, 1),
                      FRB.IfPredPart("", FRB.IfPredPartType.Else, _Z3CTX),
                      FRB.IfContentPart({"commandName": [(0, "img_dither")],
                                         "params": [(1, '"imgs": ["some link", "another_link"]'),
                                                    (2, '"width": [200, 1000]'),
                                                    (3, '"height": [700, 900, 190]')] }, 1),
                      FRB.IfPredPart("Totally not some basic ASP.net settings", FRB.IfPredPartType.EndIf, _Z3CTX),
                      FRB.IfContentPart({"LogLevel": [(0, '"Default": "Information"'),
                                                      (1, '"Microsoft": "Warning"'),
                                                      (2, '"Microsoft.Hosting.Lifetime": "Information"')]}, 0)]
        
    def addSubCommand(self, ifTemplate: FRB.IfTemplate, partInd, part: FRB.IfTemplatePart):
        commandKey = "commandName"
        if (commandKey in part):
            command = part[commandKey][0][1]
            ifTemplate.calledSubCommands[partInd] = command
            return command

    def setPartsDict(self, ifTemplate: Optional[FRB.IfTemplate] = None, pred: Optional[Callable[[FRB.IfTemplate, int, Part], bool]] = None, parts: Optional[List[Part]] = None) -> Dict[int, Part]:
        self._partsDict = {}
        if (ifTemplate is None):
            ifTemplate = self.ifTemplate

        if (pred is None):
            pred = lambda ifTemplate, partInd, part: True

        if (parts is None):
            parts = self._parts

        partsLen = len(parts)
        for i in range(partsLen):
            part = parts[i]
            if (pred(ifTemplate, i, part)):
                self._partsDict[i] = part

        return self._partsDict

    def createIfTemplate(self):
        self.ifTemplate = FRB.IfTemplate(self._parts)
        self.setPartsDict()

    def compareIfTemplatePart(self, resultPart: FRB.IfTemplatePart, expectedPart: FRB.IfTemplatePart):
        self.assertIsInstance(resultPart, FRB.IfTemplatePart)
        self.assertEqual(type(resultPart), type(expectedPart))
        
        if (isinstance(resultPart, FRB.IfPredPart)):
            self.assertEqual(resultPart.src, expectedPart.src)
            self.assertEqual(resultPart.type, expectedPart.type)

        elif (isinstance(resultPart, FRB.IfContentPart)):
            self.compareDict(resultPart.src, expectedPart.src, 
                             lambda resultKVPs, expectedKVPs: self.compareList(resultKVPs, expectedKVPs, 
                                                                               lambda resultValData, expectedValData: self.compareList(resultValData, expectedValData)))
            
            self.compareList(resultPart._order, expectedPart._order, lambda resultValData, expectedValData: self.compareList(resultValData, expectedValData))

    # ========= __iter__ =====================================
    
    def test_emptyParts_noIteration(self):
        self._parts = []
        self.createIfTemplate()

        result = []
        for part in self.ifTemplate:
            result.append(part)

        self.compareList(result, [])

    def test_hasParts_iterateOverParts(self):
        self.setDefaultAtts()
        self.createIfTemplate()

        result = []
        for part in self.ifTemplate:
            result.append(part)

        self.compareList(result, self._parts)

    # ========================================================
    # ========= __getitem__ ==================================
        
    def test_itemNotExist_indexError(self):
        self._parts = []
        self.createIfTemplate()

        error = None
        try:
            self.ifTemplate[0]
        except IndexError as e:
            error = e

        self.assertIsInstance(error, IndexError)

    def test_itemExists_itemAtIndex(self):
        self.setDefaultAtts()
        self.createIfTemplate()

        result = self.ifTemplate[0]
        self.assertEqual(result, self._parts[0])
        
    # ========================================================
    # ========= __setitem__ ==================================
        
    def test_indexOutOfRange_indexError(self):
        self._parts = []
        self.createIfTemplate()

        error = None
        try:
            self.ifTemplate[0] = {"a": 1}
        except IndexError as e:
            error = e

        self.assertIsInstance(error, IndexError)

    def test_itemExists_newItemSetAtIndex(self):
        self.setDefaultAtts()
        self.createIfTemplate()

        newValue = {"a": 1}
        self.ifTemplate[0] = newValue

        result = self.ifTemplate[0]
        self.assertIsInstance(result, dict)
        self.compareDict(result, newValue)
        
    # ========================================================
    # ========= add ==========================================
        
    def test_addParts_newPartsAddedToEnd(self):
        self.setDefaultAtts()
        self.createIfTemplate()

        newParts = [FRB.IfPredPart("a new part", FRB.IfPredPartType.If, _Z3CTX), 
                    FRB.IfContentPart({"Title": [(0, "My INI Config title")], "Name": [(1, "My INI Config name")]}, 0),
                    FRB.IfPredPart("ending part", FRB.IfPredPartType.EndIf, _Z3CTX)]

        for part in newParts:
            self.ifTemplate.add(part)

        self.setDefaultAtts()
        self.compareList(self.ifTemplate.parts, self._parts + newParts, lambda resultPart, expectedPart: self.compareIfTemplatePart(resultPart, expectedPart))
        
    # ========================================================
    # ========= find =========================================
        
    def test_emptyParts_noPartsFound(self):
        self._parts = []
        self.createIfTemplate()

        expected = {}
        pred = lambda part: isinstance(part, str)
        postProcessor = lambda part: "ICUP"

        result = self.ifTemplate.find()
        self.compareDict(result, expected)

        result = self.ifTemplate.find(pred)
        self.compareDict(result, expected)

        result = self.ifTemplate.find(pred, postProcessor)
        self.compareDict(result, expected)

    def test_hasParts_filteredParts(self):
        self.setDefaultAtts()
        self.createIfTemplate()

        theAnswerToLifeTheUniverseAndEverything = 42
        postProcessor = lambda ifTemplate, partInd, part: theAnswerToLifeTheUniverseAndEverything

        result = self.ifTemplate.find()
        self.compareDict(result, self._partsDict)

        pred = lambda ifTemplate, partInd, part: False
        result = self.ifTemplate.find(pred)
        self.compareDict(result, self.setPartsDict(pred = pred))

        result = self.ifTemplate.find(pred, postProcessor = postProcessor)
        self.compareDict(result, self._partsDict)

        pred = lambda ifTemplate, partInd, part: isinstance(part, str)
        result = self.ifTemplate.find(pred)
        self.compareDict(result, self.setPartsDict(pred = pred))

        result = self.ifTemplate.find(pred, postProcessor = postProcessor)
        expected = {}
        for ind in self._partsDict:
            expected[ind] = postProcessor(self.ifTemplate, ind, self._partsDict[ind])

        self.compareDict(result, expected)

        pred = lambda ifTemplate, partInd, part: isinstance(part, FRB.IfContentPart) and (partInd == 1 or partInd == 2 or partInd == 4 or partInd == 6)
        postProcessor = lambda ifTemplate, partInd, part: self.addSubCommand(ifTemplate, partInd, part)
        result = self.ifTemplate.find(pred, postProcessor = postProcessor)
        self.compareDict(result, {1: "Project"})
        self.compareDict(self.ifTemplate.calledSubCommands, {1: "Project"})
        
    # ========================================================
    # ========= isKeyFullyCover ==============================

    def test_ifTemplatesNoSubCommands_keyFullCover(self):
        ifTemplateName = "SomeIfTemplate"

        tests = [["cacheType", FRB.IfTemplate([]), False],
                 ["cacheType", FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)]), False],
                 ["cacheType", FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": [(1, "S3FIFOd"), (2, "S3FIFO")]}, 0)]), True],
                 ["cacheType", FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": [(1, "S3FIFOd"), (2, "S3FIFO")]}, 0),
                                               FRB.IfContentPart({"FIFOVariant": [(0, "5")]}, 0)]), True],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                                         FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), True],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                                         FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), True],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")]}, 0),
                                                         FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), True],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), True],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), False],
                                                         
                 ["target", FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                            FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Else, _Z3CTX),
                                                FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x % 7 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfPredPart("else if $x % 10 == 0", FRB.IfPredPartType.Else, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), False],
                 ["target", FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                            FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Else, _Z3CTX),
                                                FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x % 7 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfPredPart("else if $x % 10 == 0", FRB.IfPredPartType.Else, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), True]]

        for test in tests:
            key = test[0]
            ifTemplate = test[1]
            sections = {ifTemplateName: ifTemplate}
            visited = set()
            sectionsKeyFullCover = {}

            result = ifTemplate.isKeyFullyCover(key, sections, visited, sectionsKeyFullCover)
            expected = test[2]

            self.assertEqual(result, expected)

    def test_ifTemplateWithSubcommands_keyFullCover(self):
        tests = [["target", {"main": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "sub")]}, 0)], name = "main"),
                             "sub": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "target": [(1, "2")]}, 0)], name = "sub")}, "main", True],
                             
                 ["target", {"loop": FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")]}, 0),
                                                     FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                     FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                        "run": [(1, "loop")]}, 1),
                                                     FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                     FRB.IfContentPart({"target": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                     FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)], name = "loop")}, "loop", False]]
        
        for test in tests:
            key = test[0]
            sections = test[1]
            targetSectionName = test[2]
            targetIfTemplate = sections[targetSectionName]
            visited = set()
            sectionsKeyFullCover = {}

            result = targetIfTemplate.isKeyFullyCover(key, sections, visited, sectionsKeyFullCover)
            expected = test[3]
            self.assertEqual(result, expected)


    # ========================================================
    # ========= getKeyMissingParts ===========================

    def test_ifTemplatesNoSubCommands_keyMissingParts(self):
        ifTemplateName = "SomeIfTemplate"

        tests = [
            ["cacheType", FRB.IfTemplate([]), set()],
                 ["cacheType", FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)]), {0}],
                 ["cacheType", FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": [(1, "S3FIFOd"), (2, "S3FIFO")]}, 0)]), set()],
                 ["cacheType", FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": [(1, "S3FIFOd"), (2, "S3FIFO")]}, 0),
                                               FRB.IfContentPart({"FIFOVariant": [(0, "5")]}, 0)]), set()],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                                         FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), set()],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                                         FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), set()],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")]}, 0),
                                                         FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), set()],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), set()],

                 ["inductiveHypothesis", FRB.IfTemplate([FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                                                         FRB.IfContentPart({"baseCase": [(0, "0")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 1),
                                                         FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                                                         FRB.IfContentPart({"inductiveStep": [(1, "true")]}, 1),
                                                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), {3}],
                                                         
                 ["target", FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                            FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Else, _Z3CTX),
                                                FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x % 7 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfPredPart("else if $x % 10 == 0", FRB.IfPredPartType.Else, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), {11}],
                 ["target", FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                            FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Else, _Z3CTX),
                                                FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x % 7 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                            FRB.IfPredPart("else if $x % 10 == 0", FRB.IfPredPartType.Else, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                                FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]), set()]
                                            ]

        for test in tests:
            key = test[0]
            ifTemplate = test[1]
            sections = {ifTemplateName: ifTemplate}
            visited = set()
            sectionsKeyFullCover = {}
            sectionAllBranchesMissing = {}

            result = ifTemplate.getKeyMissingParts(key, sections, visited, sectionsKeyFullCover, sectionAllBranchesMissing)
            expected = test[2]
            expected = set(map(lambda partInd: ifTemplate.parts[partInd], expected))

            self.compareSet(result, expected)

    # ========================================================
