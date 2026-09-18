##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
import sys
##### EndExtImports

##### LocalImports
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
##### EndLocalImports


_Z3CTX = FRB.Z3Context()  # shared across every IfPredPart built in this test file


# NOTE: FRB.IfTemplate's constructor (and .add()/__setitem__) *takes ownership* of whatever
# IfTemplatePart Python objects are passed in (the same contract as IfContentPart's own 'content'
# parameter) -- the original objects are "disowned" afterward (any further attribute access on them
# raises), and .parts/.partsById/__getitem__/find() each hand back a freshly-cast wrapper rather
# than the exact same Python object every time. So, unlike a plain Python container, comparisons
# here are necessarily structural (type + .entries()/.src), never by identity -- and every part
# used in a test is built fresh, never reused from an already-consumed list.
class IfTemplateTest(BaseUnitTest):
    def makeDefaultParts(self):
        return [FRB.IfPredPart("some string", FRB.IfPredPartType.If, _Z3CTX),
                FRB.IfContentPart({"commandName": [(0, "Project")], "dotnetRunMessages": [(1, "True")]}, 1),
                FRB.IfPredPart("", FRB.IfPredPartType.Else, _Z3CTX),
                FRB.IfContentPart({"commandName": [(0, "img_dither")]}, 1),
                FRB.IfPredPart("Totally not some basic ASP.net settings", FRB.IfPredPartType.EndIf, _Z3CTX),
                FRB.IfContentPart({"LogLevel": [(0, '"Default": "Information"')]}, 0)]

    def comparePart(self, resultPart: FRB.IfTemplatePart, expectedType: type, expectedData):
        self.assertIsInstance(resultPart, FRB.IfTemplatePart)
        self.assertEqual(type(resultPart), expectedType)

        if (expectedType is FRB.IfPredPart):
            expectedSrc, expectedPredType = expectedData
            self.assertEqual(resultPart.src, expectedSrc)
            self.assertEqual(resultPart.type, expectedPredType)
        else:
            self.compareList(resultPart.entries(), expectedData)

    # ========= __iter__ / __len__ ============================

    def test_emptyParts_noIteration(self):
        ifTemplate = FRB.IfTemplate([])
        self.assertEqual(list(ifTemplate), [])
        self.assertEqual(len(ifTemplate), 0)

    def test_hasParts_iterateOverParts(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())
        result = list(ifTemplate)

        self.assertEqual(len(result), 6)
        self.comparePart(result[0], FRB.IfPredPart, ("some string", FRB.IfPredPartType.If))
        self.comparePart(result[1], FRB.IfContentPart, [("commandName", "Project"), ("dotnetRunMessages", "True")])
        self.comparePart(result[5], FRB.IfContentPart, [("LogLevel", '"Default": "Information"')])

    # ========================================================
    # ========= __getitem__ ==================================

    def test_itemNotExist_indexError(self):
        ifTemplate = FRB.IfTemplate([])
        with self.assertRaises(IndexError):
            ifTemplate[0]

    def test_itemExists_itemAtIndex(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())
        self.comparePart(ifTemplate[1], FRB.IfContentPart, [("commandName", "Project"), ("dotnetRunMessages", "True")])

    # ========================================================
    # ========= __setitem__ ==================================

    def test_indexOutOfRange_indexError(self):
        ifTemplate = FRB.IfTemplate([])
        with self.assertRaises(IndexError):
            ifTemplate[0] = FRB.IfContentPart({"a": [(0, "1")]}, 0)

    def test_itemExists_newItemSetAtIndex(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())
        ifTemplate[0] = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        self.comparePart(ifTemplate[0], FRB.IfContentPart, [("a", "1")])

    def test_setItemToNone_slotCleared(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())
        ifTemplate[0] = None
        self.assertIsNone(ifTemplate[0])

    # ========================================================
    # ========= add ==========================================

    def test_addParts_newPartsAddedToEnd(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())

        ifTemplate.add(FRB.IfPredPart("a new part", FRB.IfPredPartType.If, _Z3CTX))
        ifTemplate.add(FRB.IfContentPart({"Title": [(0, "My INI Config title")]}, 0))
        ifTemplate.add(FRB.IfPredPart("ending part", FRB.IfPredPartType.EndIf, _Z3CTX))

        self.assertEqual(len(ifTemplate), 9)
        self.comparePart(ifTemplate[6], FRB.IfPredPart, ("a new part", FRB.IfPredPartType.If))
        self.comparePart(ifTemplate[7], FRB.IfContentPart, [("Title", "My INI Config title")])
        self.comparePart(ifTemplate[8], FRB.IfPredPart, ("ending part", FRB.IfPredPartType.EndIf))

    # ========================================================
    # ========= find =========================================

    def test_emptyParts_noPartsFound(self):
        ifTemplate = FRB.IfTemplate([])
        self.assertEqual(ifTemplate.find(), {})

        pred = lambda ifTemplate, partInd, part: True
        postProcessor = lambda ifTemplate, partInd, part: "ICUP"
        self.assertEqual(ifTemplate.find(pred), {})
        self.assertEqual(ifTemplate.find(pred, postProcessor), {})

    def test_hasParts_filteredParts_defaultsReturnEveryPart(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())
        result = ifTemplate.find()
        self.assertEqual(set(result.keys()), {0, 1, 2, 3, 4, 5})

    def test_hasParts_filteredParts_predFiltersByIndexOrType(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())

        pred = lambda ifTemplate, partInd, part: False
        self.assertEqual(ifTemplate.find(pred), {})

        pred = lambda ifTemplate, partInd, part: isinstance(part, FRB.IfPredPart)
        result = ifTemplate.find(pred)
        self.assertEqual(set(result.keys()), {0, 2, 4})

    def test_hasParts_filteredParts_postProcessorAppliedToEachMatch(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())

        theAnswerToLifeTheUniverseAndEverything = 42
        pred = lambda ifTemplate, partInd, part: isinstance(part, FRB.IfContentPart) and partInd in (1, 3)
        postProcessor = lambda ifTemplate, partInd, part: theAnswerToLifeTheUniverseAndEverything

        result = ifTemplate.find(pred, postProcessor = postProcessor)
        self.assertEqual(result, {1: theAnswerToLifeTheUniverseAndEverything, 3: theAnswerToLifeTheUniverseAndEverything})

    def test_hasParts_filteredParts_postProcessorCanExtractSubCommand(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())

        def extractCommandName(ifTemplate, partInd, part):
            return part["commandName"][0]

        pred = lambda ifTemplate, partInd, part: isinstance(part, FRB.IfContentPart) and part.contains("commandName")
        result = ifTemplate.find(pred, postProcessor = extractCommandName)
        self.assertEqual(result, {1: "Project", 3: "img_dither"})

    # ========================================================
    # ========= partsById / calledSubCommands ================

    def test_partsById_everyPartKeyedByItsOwnId(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())
        partsById = ifTemplate.partsById

        self.assertEqual(len(partsById), 6)
        for ind, part in enumerate(ifTemplate):
            self.assertIn(part.id, partsById)
            self.comparePart(partsById[part.id], type(part), part.entries() if isinstance(part, FRB.IfContentPart) else (part.src, part.type))

    def test_calledSubCommands_partWithRunKey_subCommandTracked(self):
        ifTemplate = FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "otherSection")]}, 0)])
        self.assertIn(0, ifTemplate.calledSubCommands)
        self.compareList(ifTemplate.calledSubCommands[0], ["otherSection"])

    def test_calledSubCommands_partWithoutRunKey_notTracked(self):
        ifTemplate = FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)])
        self.assertNotIn(0, ifTemplate.calledSubCommands)

    # ========================================================
    # ========= deepcopy ======================================

    def test_deepcopy_resultIsIndependentEqualCopy(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())
        copied = ifTemplate.deepcopy()

        self.assertIsNot(copied, ifTemplate)
        self.assertEqual(len(copied), len(ifTemplate))
        self.comparePart(copied[1], FRB.IfContentPart, [("commandName", "Project"), ("dotnetRunMessages", "True")])

        # mutating the copy must not affect the original
        copied[1] = None
        self.assertIsNotNone(ifTemplate[1])

    # ========================================================
    # ========= tree ==========================================

    def test_tree_rootHasNoIfPredPart(self):
        ifTemplate = FRB.IfTemplate(self.makeDefaultParts())
        self.assertIsNone(ifTemplate.tree.root.ifPredPart)

    # ========================================================
    # ========= addTopContentPart / addBottomContentPart =====

    def test_addTopContentPart_emptyIfTemplate_newPartAddedAtFront(self):
        ifTemplate = FRB.IfTemplate([])
        part = ifTemplate.addTopContentPart()
        self.assertIsInstance(part, FRB.IfContentPart)
        self.assertEqual(len(ifTemplate), 1)

    def test_addBottomContentPart_emptyIfTemplate_newPartAddedAtEnd(self):
        ifTemplate = FRB.IfTemplate([])
        part = ifTemplate.addBottomContentPart()
        self.assertIsInstance(part, FRB.IfContentPart)
        self.assertEqual(len(ifTemplate), 1)

    def test_addKVPsToFront_kvpsAddedToTopmostPart(self):
        ifTemplate = FRB.IfTemplate([])
        ifTemplate.addKVPsToFront([("a", "1"), ("b", "2")])
        self.assertTrue(ifTemplate[0].contains("a"))
        self.assertTrue(ifTemplate[0].contains("b"))

    def test_addKVPsToBack_kvpsAddedToBottommostPart(self):
        ifTemplate = FRB.IfTemplate([])
        ifTemplate.addKVPsToBack([("c", "3")])
        self.assertTrue(ifTemplate[len(ifTemplate) - 1].contains("c"))
