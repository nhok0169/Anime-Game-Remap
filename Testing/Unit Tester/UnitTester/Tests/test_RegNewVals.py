import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RegNewValsTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        vals = {"a": "1"}
        edit = FRB.RegNewVals(vals, addNewKVPs = True)
        self.assertIs(edit.vals, vals)
        self.assertTrue(edit.addNewKVPs)

    def test_init_addNewKVPsDefaultsFalse(self):
        edit = FRB.RegNewVals({"a": "1"})
        self.assertFalse(edit.addNewKVPs)

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSamePartInstance(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"a": "2"})

        result = edit.edit(part, "root", None)
        self.assertIs(result, part)

    def test_edit_existingKey_valueReplaced(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit = FRB.RegNewVals({"a": "new"})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "new"), ("b", "2")])

    def test_edit_everyOccurenceOfTheKeyReplaced(self):
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2")], "b": [(2, "3")]}, 0)
        edit = FRB.RegNewVals({"a": "new"})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "new"), ("a", "new"), ("b", "3")])

    def test_edit_multipleKeys_eachReplacedIndependently(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)
        edit = FRB.RegNewVals({"a": "x", "c": "z"})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "x"), ("b", "2"), ("c", "z")])

    def test_edit_unmentionedKey_leftUnchanged(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "z": [(1, "9")]}, 0)
        edit = FRB.RegNewVals({"a": "new"})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "new"), ("z", "9")])

    def test_edit_missingKeyAddNewKVPsFalse_keySkipped(self):
        # 'addNewKVPs' defaults to False here (unlike IfContentPart.replaceVals' own 'addNew',
        # which defaults to True) -- a key that doesn't exist yet is silently skipped, no error
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"missing": "9"})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "1")])

    def test_edit_missingKeyAddNewKVPsTrue_kvpAppended(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"missing": "9"}, addNewKVPs = True)

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "1"), ("missing", "9")])

    def test_edit_replaceListSpec_valuesAssignedPositionally(self):
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2"), (2, "3")]}, 0)
        edit = FRB.RegNewVals({"a": FRB.ReplaceList(["x", "y"])})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "x"), ("a", "y"), ("a", "3")])

    def test_edit_replaceIfSpec_onlyMatchingValuesReplaced(self):
        part = FRB.IfContentPart({"a": [(0, "no"), (1, "yes")]}, 0)
        edit = FRB.RegNewVals({"a": FRB.ReplaceIf("new", lambda old, modType: old == "yes")})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "no"), ("a", "new")])

    def test_edit_replaceIfSpec_predicateReceivesTheModType(self):
        # unlike every replaceVals, this class calls a ReplaceIf predicate with (oldValue, modType)
        seen = []
        modType = object()
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2")]}, 0)
        edit = FRB.RegNewVals({"a": FRB.ReplaceIf("new", lambda old, mt: seen.append((old, mt)) or True)})

        edit.edit(part, "root", modType)

        self.compareList([old for old, _ in seen], ["1", "2"])
        for _, mt in seen:
            self.assertIs(mt, modType)
        self.compareList(part.entries(), [("a", "new"), ("a", "new")])

    def test_edit_replaceIfSpec_predicateCanDecideOffTheModType(self):
        class FakeModType:
            def __init__(self, name):
                self.name = name

        edit = FRB.RegNewVals({"ps-t1": FRB.ReplaceIf("null", lambda old, modType: modType.name == "Amber")})

        matching = FRB.IfContentPart({"ps-t1": [(0, "resource")]}, 0)
        edit.edit(matching, "root", FakeModType("Amber"))
        self.compareList(matching.entries(), [("ps-t1", "null")])

        nonMatching = FRB.IfContentPart({"ps-t1": [(0, "resource")]}, 0)
        edit.edit(nonMatching, "root", FakeModType("Klee"))
        self.compareList(nonMatching.entries(), [("ps-t1", "resource")])

    def test_edit_callableVal_calledWithTheModTypeAndItsResultWritten(self):
        # a callable sitting where a new value goes is a value producer, called as newVal(modType)
        seen = []
        modType = object()
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit = FRB.RegNewVals({"a": lambda mt: seen.append(mt) or "new"})

        edit.edit(part, "root", modType)

        self.assertEqual(len(seen), 1)
        self.assertIs(seen[0], modType)
        self.compareList(part.entries(), [("a", "new"), ("b", "2")])

    def test_edit_callableVal_canDecideOffTheModType(self):
        class FakeModType:
            def __init__(self, name):
                self.name = name

        edit = FRB.RegNewVals({"ps-t1": lambda modType: f"{modType.name}Texture"})

        amber = FRB.IfContentPart({"ps-t1": [(0, "resource")]}, 0)
        edit.edit(amber, "root", FakeModType("Amber"))
        self.compareList(amber.entries(), [("ps-t1", "AmberTexture")])

        klee = FRB.IfContentPart({"ps-t1": [(0, "resource")]}, 0)
        edit.edit(klee, "root", FakeModType("Klee"))
        self.compareList(klee.entries(), [("ps-t1", "KleeTexture")])

    def test_edit_callableVal_calledOncePerEditNotPerOccurence(self):
        calls = []
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2")]}, 0)
        edit = FRB.RegNewVals({"a": lambda mt: calls.append(mt) or "new"})

        edit.edit(part, "root", None)

        self.assertEqual(len(calls), 1)
        self.compareList(part.entries(), [("a", "new"), ("a", "new")])

    def test_edit_callableValWithNoModType_receivesNone(self):
        seen = []
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"a": lambda mt: seen.append(mt) or "new"})

        edit.edit(part, "root", None)

        self.compareList(seen, [None])
        self.compareList(part.entries(), [("a", "new")])

    def test_edit_callableVal_nonStrResultStringified(self):
        # same stringification every other value slot gets
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"a": lambda mt: 12})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "12")])

    def test_edit_callableValWithAddNewKVPs_kvpAppended(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"missing": lambda mt: "9"}, addNewKVPs = True)

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "1"), ("missing", "9")])

    def test_edit_callableValInsideAReplaceList_resolvedPositionally(self):
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2"), (2, "3")]}, 0)
        edit = FRB.RegNewVals({"a": FRB.ReplaceList(["x", lambda mt: f"{mt}y"])})

        edit.edit(part, "root", "M")
        self.compareList(part.entries(), [("a", "x"), ("a", "My"), ("a", "3")])

    def test_edit_callableValAsAReplaceIfValue_resolvedForMatchingOccurences(self):
        part = FRB.IfContentPart({"a": [(0, "no"), (1, "yes")]}, 0)
        edit = FRB.RegNewVals({"a": FRB.ReplaceIf(lambda mt: f"{mt}New", lambda old, mt: old == "yes")})

        edit.edit(part, "root", "M")
        self.compareList(part.entries(), [("a", "no"), ("a", "MNew")])

    def test_edit_callableValMutatedInPlaceAfterInit_mutationHonoured(self):
        # 'vals' is re-read on every edit, so swapping in a producer later takes effect
        vals = {"a": "new"}
        edit = FRB.RegNewVals(vals)
        vals["a"] = lambda mt: "fromProducer"

        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "fromProducer")])

    def test_edit_callableValRaising_propagates(self):
        def boom(modType):
            raise ValueError("nope")

        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"a": boom})

        with self.assertRaises(ValueError):
            edit.edit(part, "root", None)

    def test_edit_callableValWithAZeroArgCallable_raisesTypeError(self):
        # a producer is always called with the modType, so a 0-arg callable is a TypeError
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"a": lambda: "new"})

        with self.assertRaises(TypeError):
            edit.edit(part, "root", None)

    def test_edit_replaceIfSpecWithASingleArgPredicate_raisesTypeError(self):
        # a 1-arg predicate is what every replaceVals wants, and is deliberately NOT accepted here
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"a": FRB.ReplaceIf("new", lambda old: True)})

        with self.assertRaises(TypeError):
            edit.edit(part, "root", None)

    def test_edit_replaceIfSpec_stillOneArgForAPlainReplaceVals(self):
        # the same marker class keeps its single-argument contract everywhere else
        part = FRB.IfContentPart({"a": [(0, "no"), (1, "yes")]}, 0)
        part.replaceVals({"a": FRB.ReplaceIf("new", lambda old: old == "yes")})

        self.compareList(part.entries(), [("a", "no"), ("a", "new")])

    def test_edit_emptyVals_partUnchanged(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "1")])

    def test_edit_partRanges_onlyRestrictedOccurencesReplaced(self):
        # an occurrence outside 'partRanges' is left alone, even though its key is unconditionally
        # mentioned in 'vals'
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2"), (2, "3")]}, 0)
        edit = FRB.RegNewVals({"a": "new"})

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(1, 2)]))
        self.compareList(part.entries(), [("a", "1"), ("a", "new"), ("a", "3")])

    def test_edit_emptyPartRanges_partUnchanged(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"a": "new"})

        edit.edit(part, "root", None, partRanges = FRB.Ranges.createEmpty())
        self.compareList(part.entries(), [("a", "1")])

    def test_edit_valsMutatedInPlaceAfterInit_mutationHonoured(self):
        # 'vals' stays the exact Python dict that was passed in, so mutating it later changes what
        # the edit does -- same as the pure-Python original this replaced
        vals = {"a": "new"}
        edit = FRB.RegNewVals(vals)
        vals["b"] = "alsoNew"

        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "new"), ("b", "alsoNew")])

    # ================================================
    # ================== editFromIni ==================

    def test_editFromIni_ignoresIniAndForwardsToEdit(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegNewVals({"a": "new"})

        result = edit.editFromIni(part, "root", None, None)
        self.assertIs(result, part)
        self.compareList(part.entries(), [("a", "new")])
