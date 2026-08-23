import sys
import copy

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IfPredPartTest(BaseUnitTest):
    """
    Black-box tests against the Z3-based, C++-backed IfPredPart's public API -- this is a
    standalone new class (not a like-for-like port of the deprecated, sympy-typed pure-Python
    original -- see IfPredPart.h's own doc comment), so there's no "convert every old test" story
    here; these tests exist to confirm the pybind11
    wrapping layer itself works correctly (argument marshalling, the IfPredPartType translation,
    exception-to-None propagation, __copy__/__deepcopy__). The underlying Z3 logic itself (query
    correctness, precedence, round-tripping) is already exhaustively covered with real
    z3::solver-based equivalence checks in the C++ core's own core/tests/IfPredPart_test.cpp --
    this file isn't trying to duplicate that from Python.
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._z3Ctx = FRB.Z3Context()

    # ==================== __init__ ====================

    def test_isIfTemplatePart(self):
        part = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        self.assertIsInstance(part, FRB.IfTemplatePart)

    def test_type_isTranslatedToExistingPythonEnum(self):
        # IfPredPart.type must keep returning the *pre-existing* pure-Python IfPredPartType enum
        # (not some new, separately pybind11-bound one) -- every other real caller across this
        # codebase (IfTemplateTree.py, IniSectionGraph.py, BaseIniFixerOld.py, ...) compares
        # '.type' against that exact enum, with no reason to know this class exists at all.
        part = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        self.assertIs(part.type, FRB.IfPredPartType.If)
        self.assertIsInstance(part.type, FRB.IfPredPartType)

    def test_if_queryIsPopulated(self):
        part = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        self.assertIsInstance(part.query, FRB.Z3Predicate)

        expected = FRB.IfPredPart.getLogicQuery(FRB.ParseContext("$x == 5"), self._z3Ctx)
        self.compareZ3Query(part.query, expected)

    def test_elif_bareSpelling_queryIsPopulated(self):
        # The pure-Python original has a live bug for a bare "elif ..." spelling (as opposed to
        # "else if ..."): its getTestStr() falls into a branch referencing a Python local that was
        # never assigned on that path, raising UnboundLocalError. This class implements the
        # clearly-intended behavior instead -- confirmed indirectly here via a real, successfully-
        # parsed query (an UnboundLocalError would surface as this whole call raising instead).
        part = FRB.IfPredPart("elif $y != 3", FRB.IfPredPartType.Elif, self._z3Ctx)
        self.assertIsInstance(part.query, FRB.Z3Predicate)

        expected = FRB.IfPredPart.getLogicQuery(FRB.ParseContext("$y != 3"), self._z3Ctx)
        self.compareZ3Query(part.query, expected)

    def test_elseIf_spelling_queryIsPopulated(self):
        part = FRB.IfPredPart("else if $y != 3", FRB.IfPredPartType.Elif, self._z3Ctx)
        expected = FRB.IfPredPart.getLogicQuery(FRB.ParseContext("$y != 3"), self._z3Ctx)
        self.compareZ3Query(part.query, expected)

    def test_else_queryIsLiteralTrue(self):
        part = FRB.IfPredPart("else", FRB.IfPredPartType.Else, self._z3Ctx)
        self.compareZ3Query(part.query, FRB.Z3Predicate.trueValue(self._z3Ctx))

    def test_endif_queryIsNone(self):
        part = FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, self._z3Ctx)
        self.assertIsNone(part.query)

    def test_malformedPredicate_queryIsNoneNoExceptionRaised(self):
        # A leading PLUS can never start a 'pred' -- guaranteed SyntaxErr during parse (same
        # fixture the C++ core's own tests rely on) -- getLogicQuery swallows it and IfPredPart's
        # constructor should too, not propagate it.
        part = FRB.IfPredPart("if + then", FRB.IfPredPartType.If, self._z3Ctx)
        self.assertIsNone(part.query)

    def test_queryGivenDirectly_usedAsIs(self):
        prebuilt = FRB.IfPredPart.getLogicQuery(FRB.ParseContext("$x == 5"), self._z3Ctx)
        part = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx, query = prebuilt)
        self.compareZ3Query(part.query, prebuilt)

    def test_suppliedParseContext_mutatedInPlace(self):
        parseCtx = FRB.ParseContext("placeholder", file = "myFile.ini", startLineNo = 7)
        FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx, ctx = parseCtx)

        self.assertEqual(parseCtx.lines, [" $x == 5 "])
        self.assertEqual(parseCtx.file, "myFile.ini")

    def test_sharedZ3Context_sameNamedVariableInterns(self):
        # Two independently-parsed IfPredParts, same Z3Context -- the same '$x' variable should
        # intern to the same underlying Z3 constant, so a predicate built from one part's query
        # and the other's should combine/compare consistently (not raise a "different context"
        # AssertionError from compareZ3Query's own sameContext check).
        part1 = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        part2 = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        self.compareZ3Query(part1.query, part2.query)

    # ==================== clone ====================

    def test_clone_sameId_idAndQueryPreserved(self):
        original = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        cloned = original.clone(False)

        self.assertEqual(cloned.id, original.id)
        self.assertEqual(cloned.src, original.src)
        self.assertIs(cloned.type, original.type)
        self.compareZ3Query(cloned.query, original.query)

    def test_clone_newId_freshIdGenerated(self):
        original = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        cloned = original.clone(True)
        self.assertNotEqual(cloned.id, original.id)

    def test_copy_equivalentToClone(self):
        original = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        copied = copy.copy(original)

        self.assertEqual(copied.id, original.id)
        self.compareZ3Query(copied.query, original.query)

    def test_deepcopy_equivalentToClone(self):
        original = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        copied = copy.deepcopy(original)

        self.assertEqual(copied.id, original.id)
        self.compareZ3Query(copied.query, original.query)

    # ==================== getTestStr ====================

    def test_getTestStr_if_keywordsStripped(self):
        part = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        self.assertEqual(part.getTestStr(), " $x == 5 ")

    def test_getTestStr_bareElif_keywordStripped(self):
        part = FRB.IfPredPart("elif $x == 5", FRB.IfPredPartType.Elif, self._z3Ctx)
        self.assertEqual(part.getTestStr(), " $x == 5")

    def test_getTestStr_elseIf_bothKeywordsStripped(self):
        part = FRB.IfPredPart("else if $x == 5", FRB.IfPredPartType.Elif, self._z3Ctx)
        self.assertEqual(part.getTestStr(), "  $x == 5")

    # ==================== getIfPredStr ====================

    def test_getIfPredStr_representablePredicate_stringReturned(self):
        predicate = FRB.IfPredPart.getLogicQuery(FRB.ParseContext("$x == 5"), self._z3Ctx)
        result = FRB.IfPredPart.getIfPredStr(predicate)

        self.assertIsInstance(result, str)
        self.assertIn("$x", result)

        # round-trip: re-parsing the generated text should be logically equivalent to the original
        reparsed = FRB.IfPredPart.getLogicQuery(FRB.ParseContext(result), self._z3Ctx)
        self.compareZ3Query(reparsed, predicate)

    # ==================== toStr ====================

    def test_toStr_noPrefix_trailingNewlineStripped(self):
        part = FRB.IfPredPart("   if $x == 5 then\n", FRB.IfPredPartType.If, self._z3Ctx)
        self.assertEqual(part.toStr(), "   if $x == 5 then")

    def test_toStr_withPrefix_leftSpacingStrippedPrefixPrepended(self):
        part = FRB.IfPredPart("   if $x == 5 then\n", FRB.IfPredPartType.If, self._z3Ctx)
        self.assertEqual(part.toStr(linePrefix = "::"), "::if $x == 5 then")

    # ==================== id (inherited from IfTemplatePart) ====================

    def test_id_autoGenerated_whenNotSupplied(self):
        part1 = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        part2 = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        self.assertNotEqual(part1.id, part2.id)

    def test_id_suppliedExplicitly_used(self):
        part = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx, id = 12345)
        self.assertEqual(part.id, 12345)

    def test_refreshId_generatesNewId(self):
        part = FRB.IfPredPart("if $x == 5 then", FRB.IfPredPartType.If, self._z3Ctx)
        oldId = part.id
        newId = part.refreshId()

        self.assertEqual(part.id, newId)
        self.assertNotEqual(newId, oldId)
