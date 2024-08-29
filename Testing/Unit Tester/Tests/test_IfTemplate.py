import sys
from .baseUnitTest import BaseUnitTest
from typing import Dict, Any, Union, Callable, Optional, List

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
from src.FixRaidenBoss2 import FixRaidenBoss2 as FRB

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
        cls._parts = ["some string",
                      {"commandName": "Project",
                       "dotnetRunMessages": True,
                        "launchBrowser": True,
                        "applicationUrl": "https://localhost:7152;http://localhost:5105",
                        "environmentVariables": {
                            "ASPNETCORE_ENVIRONMENT": "Development"
                        }
                      },
                      {},
                      "",
                      {"commandName": "img_dither",
                       "params": {
                           "imgs": ["some link", "another_link"],
                            "width": [200, 1000],
                            "height": [700, 900, 190]
                       }},
                       "Totally not some basic ASP.net settings",
                       {"LogLevel": {
                            "Default": "Information",
                            "Microsoft": "Warning",
                            "Microsoft.Hosting.Lifetime": "Information"
                        }}]
        
    def addSubCommand(self, ifTemplate: FRB.IfTemplate, partInd, part: Part):
        commandKey = "commandName"
        if (commandKey in part):
            command = part[commandKey]
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

        newParts = ["a new part", {"Title": "My INI Config title", "Name": "My INI Config name"}]
        for part in newParts:
            self.ifTemplate.add(part)

        self.setDefaultAtts()
        self.compareList(self.ifTemplate.parts, self._parts + newParts)
        
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

        pred = lambda ifTemplate, partInd, part: isinstance(part, dict)
        postProcessor = lambda ifTemplate, partInd, part: self.addSubCommand(ifTemplate, partInd, part)
        result = self.ifTemplate.find(pred, postProcessor = postProcessor)
        self.compareDict(result, {1: "Project", 2: None, 4: "img_dither", 6: None})
        self.compareDict(self.ifTemplate.calledSubCommands, {1: "Project", 4: "img_dither"})
        
    # ========================================================
