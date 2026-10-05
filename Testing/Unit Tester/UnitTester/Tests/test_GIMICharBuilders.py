import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GIMICharBuildersTest(BaseUnitTest):
    """
    The Python face of :class:`GIMICharFixerConfig` -- the config a prototype hands to
    :func:`makeGIMICharFixer`.

    ``objRegRemovals`` and ``objRegRemaps`` hold C++ ``RegRef`` / ``RegRemapRule`` values, which were
    once unbound: the fields read fine and every assignment raised ``TypeError``, so no Python
    config could strip or rename a register at all.
    """

    def setUp(self):
        self._config = FRB.GIMICharFixerConfig()
        self._C = FRB.GIMICharFixerConfig

    # ================== RegRef ==================

    def test_objRegRemovals_acceptsPlainRegisterNames(self):
        self._config.objRegRemovals = [("head", ["ps-t0", "$CharacterIB"])]

        obj, refs = self._config.objRegRemovals[0]
        self.assertEqual(obj, "head")
        self.compareList([ref.reg for ref in refs], ["ps-t0", "$CharacterIB"])

    def test_objRegRemovals_acceptsConditionalRegRefs(self):
        self._config.objRegRemovals = [("dress", ["ps-t3", self._C.RegRef("ps-t0", self._C.RegValChecks.isNormalMap)])]

        _, refs = self._config.objRegRemovals[0]
        self.compareList([ref.reg for ref in refs], ["ps-t3", "ps-t0"])

    def test_RegRef_noneCheckMeansUnconditional(self):
        ref = self._C.RegRef("ps-t1", None)
        self.assertEqual(ref.reg, "ps-t1")

    # ================== RegRemapRule ==================

    def test_objRegRemaps_acceptsTuples(self):
        self._config.objRegRemaps = [("head", [("ps-t1", ["ps-t0"]), ("ps-t2", ["ps-t1", "ps-t2"], True)])]

        obj, rules = self._config.objRegRemaps[0]
        self.assertEqual(obj, "head")
        self.compareList([(rule.from_, [ref.reg for ref in rule.to], rule.keepIfNoneMatch) for rule in rules],
                         [("ps-t1", ["ps-t0"], False), ("ps-t2", ["ps-t1", "ps-t2"], True)])

    def test_objRegRemaps_acceptsRuleObjects(self):
        rule = self._C.RegRemapRule("ps-t1", [self._C.RegRef("ps-t0", self._C.RegValChecks.isDiffuse)], keepIfNoneMatch = True)
        self._config.srcObjRegRemaps = [("body", [rule])]

        _, rules = self._config.srcObjRegRemaps[0]
        self.assertEqual(rules[0].from_, "ps-t1")
        self.assertTrue(rules[0].keepIfNoneMatch)

    def test_objRegRemaps_rejectsAMalformedTuple(self):
        with self.assertRaises(TypeError):
            self._config.objRegRemaps = [("head", [("ps-t1",)])]

    # ================== RegValChecks ==================

    def test_RegValChecks_readTheResourceName(self):
        checks = self._C.RegValChecks
        self.assertTrue(checks.isDiffuse("ResourceKiraraBodyDiffuse"))
        self.assertFalse(checks.isDiffuse("ResourceKiraraBodyLightMap"))
        self.assertTrue(checks.isLightMap("ResourceKiraraBodyLightMap"))
        self.assertTrue(checks.isNormalMap("ResourceKiraraBodyNormalMap"))

    # ================== TexEdit ==================

    def test_TexEdit_carriesTheMergeFields(self):
        edit = self._C.TexEdit("head", "ps-t1", "DarkenDiffuse", lambda texFile: None, srcObj = "body", toReg = "ps-t2",
                               check = self._C.RegValChecks.isDiffuse)
        self._config.texEdits = [edit]

        stored = self._config.texEdits[0]
        self.assertEqual((stored.obj, stored.reg, stored.name, stored.srcObj, stored.toReg),
                         ("head", "ps-t1", "DarkenDiffuse", "body", "ps-t2"))

    def test_TexEdit_mergeFieldsDefaultToEmpty(self):
        edit = self._C.TexEdit("body", "ps-t0", "Edit", lambda texFile: None)
        self.assertEqual((edit.srcObj, edit.toReg), ("", ""))

    def test_TexEdit_rejectsANonCallableFilter(self):
        with self.assertRaises(TypeError):
            self._C.TexEdit("body", "ps-t0", "Edit", "notCallable")

    # ================== makeGIMICharFixer ==================

    def test_makeGIMICharFixer_buildsFromAFullConfig(self):
        self._config.drawnObjs = ["head", "body"]
        self._config.objRegRemovals = [("head", ["ps-t0"])]
        self._config.objRegRemaps = [("body", [("ps-t1", ["ps-t0"])])]
        self.assertEqual(type(FRB.makeGIMICharFixer(self._config)).__name__, "CppIniFixFactory")
