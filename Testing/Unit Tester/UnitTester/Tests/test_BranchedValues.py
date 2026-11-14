import sys
import unittest

from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


# A merged master: the override binds nothing itself and runs a list that binds one ib per $swapvar branch
MergedMaster = """
[TextureOverrideBody]
hash = 00000000
ib = ResourceRoot
run = CommandListBody

[CommandListBody]
if $swapvar == 0
    ib = ResourceBody0
else if $swapvar == 1
    ib = ResourceBody1
endif

[CommandListBlend]
if $swapvar == 0 || $swapvar == 1
    vb1 = ResourceBlend
endif
"""


class BranchedValuesTest(unittest.TestCase):
    """What a merged master binds per branch, read through the API: :meth:`IniFile.sectionGraph`,
    :meth:`GraphCollect.rowsOf` and the branch choices on its rows"""

    def setUp(self):
        self._ini = FRB.IniFile(txt = MergedMaster)
        self._ini.getIfTemplates()

    def _rows(self, root, key):
        return FRB.GraphCollect.rowsOf(self._ini.sectionGraph(root), key)

    # ====================== rowsOf =====================================

    def test_rowsOf_rootFirst_thenWhatItRuns(self):
        rows = self._rows("TextureOverrideBody", "ib")
        self.assertEqual([row.value for row in rows], ["ResourceRoot", "ResourceBody0", "ResourceBody1"])
        self.assertEqual([row.section for row in rows], ["TextureOverrideBody", "CommandListBody", "CommandListBody"])
        self.assertTrue(all(row.rootSection == "TextureOverrideBody" for row in rows))

    def test_rowsOf_eachBranchCarriesItsCondition(self):
        rows = self._rows("TextureOverrideBody", "ib")
        self.assertIsNotNone(rows[1].condition)
        self.assertIsNotNone(rows[2].condition)
        self.assertEqual(rows[1].guards, ["$swapvar == 0"])
        self.assertEqual(rows[2].guards, ["$swapvar == 1"])

    def test_sectionGraph_missingRoot_isEmpty(self):
        self.assertEqual(self._rows("TextureOverrideNothing", "ib"), [])

    # ====================== valueUnder / onlyUnder / anyUnder ==========

    def test_valueUnder_picksTheBranchOfTheQuery(self):
        rows = self._rows("TextureOverrideBody", "ib")[1:]
        z3 = None
        self.assertEqual(FRB.GraphCollect.valueUnder(rows, "", rows[1].condition, z3), "ResourceBody1")
        self.assertEqual(FRB.GraphCollect.valueUnder(rows, "", None, z3), "ResourceBody0")
        self.assertEqual(FRB.GraphCollect.valueUnder([], "fallback", None, z3), "fallback")

    def test_onlyUnder_isTheOneCompatibleRow(self):
        rows = self._rows("TextureOverrideBody", "ib")[1:]
        self.assertEqual(FRB.GraphCollect.onlyUnder(rows, rows[0].condition, None), 0)
        self.assertEqual(FRB.GraphCollect.onlyUnder(rows, rows[1].condition, None), 1)
        self.assertEqual(FRB.GraphCollect.onlyUnder(rows, None, None), -1)
        self.assertEqual(FRB.GraphCollect.onlyUnder(rows[:1], None, None), 0)

    def test_anyUnder_aRowWithNoConditionAlwaysCan(self):
        rows = self._rows("TextureOverrideBody", "ib")
        self.assertTrue(FRB.GraphCollect.anyUnder(rows[:1], rows[1].condition, None))
        self.assertFalse(FRB.GraphCollect.anyUnder([], None, None))

    # ====================== states =====================================

    def test_states_oneSharedBlend_twoIbs_isTwoStates(self):
        ibs = self._rows("TextureOverrideBody", "ib")[1:]
        blends = FRB.GraphCollect.rowsOf(self._ini.sectionGraph("CommandListBlend"), "vb1")
        self.assertEqual(len(blends), 1)
        self.assertEqual(FRB.GraphCollect.states([blends], None), [None])
        states = FRB.GraphCollect.states([blends, ibs], None)
        self.assertEqual(len(states), 2)
        self.assertEqual([FRB.GraphCollect.valueUnder(ibs, "", state, None) for state in states], ["ResourceBody0", "ResourceBody1"])

    # ====================== Row.conditioned ============================

    def test_conditioned_keepsTheValueAndCondition(self):
        row = self._rows("TextureOverrideBody", "ib")[2]
        derived = FRB.GraphCollect.Row.conditioned("C:/mod/Body1.ib", row.condition)
        self.assertEqual(derived.value, "C:/mod/Body1.ib")
        self.assertEqual(FRB.GraphCollect.onlyUnder(self._rows("TextureOverrideBody", "ib")[1:], derived.condition, None), 1)

    # ====================== RegBranchAdd per branch ====================

    def test_perBranchReplacements_writesEachBranchItsOwnValue(self):
        rows = self._rows("TextureOverrideBody", "ib")[1:]
        edit = FRB.RegBranchAdd.perBranchReplacements(FRB.Z3Context(), rows, "blend",
                                                      lambda index, query: [("draw", "%d,0" % (index + 10))])
        self.assertIsInstance(edit, FRB.RegBranchAdd)

        graph = self._ini.sectionGraph("TextureOverrideBody")
        edit.edit(graph, None)
        drawn = [(row.value, row.guards) for row in FRB.GraphCollect.rowsOf(graph, "draw")]
        self.assertEqual(drawn, [("10,0", ["$swapvar == 0"]), ("11,0", ["$swapvar == 1"])])

    def test_perBranchRemovals_removesOnlyInTheBranchesAsked(self):
        rows = self._rows("TextureOverrideBody", "ib")[1:]
        edit = FRB.RegBranchAdd.perBranchRemovals(FRB.Z3Context(), rows, "ibs",
                                                  lambda index, query: ["ib"] if index == 1 else [])
        graph = self._ini.sectionGraph("TextureOverrideBody")
        edit.edit(graph, None)
        self.assertEqual([row.value for row in FRB.GraphCollect.rowsOf(graph, "ib")], ["ResourceRoot", "ResourceBody0"])


class ElseConditionTest(unittest.TestCase):
    """An ``else`` branch's condition is the earlier branches negated, with nothing appended for the ``else``
    itself: :class:`Z3Predicate`'s ``&`` / ``|`` leave out an operand that cannot change the result"""

    def test_elseCondition_rendersWithoutTrue(self):
        ini = FRB.IniFile(txt = "[TextureOverrideX]\nhash = 00000000\nif $c == 1\n    $y = 1\nelse\n    this = ResourceA\nendif\n")
        ini.getIfTemplates()
        rows = FRB.GraphCollect.rowsOf(ini.sectionGraph("TextureOverrideX"), "this")
        self.assertEqual(FRB.IfPredPart.getIfPredStr(rows[0].condition), "!($c == 1)")

    def test_identityOperands_areLeftOut(self):
        z3 = FRB.Z3Context()
        cond = FRB.IfPredPart.getLogicQuery(FRB.ParseContext("$c == 1"), z3)
        true = FRB.Z3Predicate.trueValue(z3)
        false = FRB.Z3Predicate.falseValue(z3)
        self.assertEqual((cond & true).toString(), cond.toString())
        self.assertEqual((true & cond).toString(), cond.toString())
        self.assertEqual((cond | false).toString(), cond.toString())
        self.assertEqual((cond & false).toString(), false.toString())
        self.assertEqual((cond | true).toString(), true.toString())


class TemplateValuesTest(unittest.TestCase):
    """A section's first value, and keys as a mod's author spells them"""

    def setUp(self):
        self._ini = FRB.IniFile(txt = "[ResourceX]\nfilename = a.dds  \nFileName = b.dds\n[TextureOverrideY]\nResource\\RabbitFX\\Lightmap = ref ResourceX\n")
        self._templates = self._ini.getIfTemplates()

    def test_firstVal_isTheFirstValueStripped(self):
        self.assertEqual(self._templates["ResourceX"].firstVal("filename"), "a.dds")
        self.assertIsNone(self._templates["ResourceX"].firstVal("format"))

    def test_firstValMatching_takesAKeyAsItsAuthorSpellsIt(self):
        section = self._templates["TextureOverrideY"]
        found = section.firstValMatching(lambda key: FRB.IniNamingTools.sameKey(key, "resource/rabbitfx/lightmap"))
        self.assertEqual(found, "ref ResourceX")

    def test_sameKey_ignoresCaseAndSlashDirection(self):
        self.assertTrue(FRB.IniNamingTools.sameKey("Resource\\RabbitFX\\Lightmap", "resource/rabbitfx/LIGHTMAP"))
        self.assertTrue(FRB.IniNamingTools.sameKey("PS-T0", "ps-t0"))
        self.assertFalse(FRB.IniNamingTools.sameKey("ps-t0", "ps-t1"))

    def test_getKeys_everyKeyAtTheResolvedVersion(self):
        hashes = FRB.ModTypes.Yelan.value.hashes
        keys = hashes.getKeys("df127976", None, ["Yelan", None])
        self.assertEqual(sorted(keys), [("Yelan", "tex_body_diffuse"), ("Yelan", "tex_dress_diffuse")])
        self.assertIn(hashes.getKey("df127976", None, ["Yelan", None]), keys)
        self.assertEqual(hashes.getKeys("df127976", None, ["Amber", None]), [])
        self.assertEqual(hashes.getKeys("notahash"), [])

    def test_claims_sharedTexturesCountEveryCharacter(self):
        self.assertEqual(FRB.GlobalIniClassifiers.claims("1d064079"), 2)
        self.assertEqual(FRB.GlobalIniClassifiers.claims("df127976"), 1)
        self.assertEqual(FRB.GlobalIniClassifiers.claims("notahash"), 0)
        self.assertIsInstance(FRB.GlobalIniClassifiers.classifier(), FRB.IniClassifier)

    def test_boundResource_noneAndNullBindNothing(self):
        self.assertEqual(FRB.IniNamingTools.boundResource(None), "")
        self.assertEqual(FRB.IniNamingTools.boundResource("null"), "")
        self.assertEqual(FRB.IniNamingTools.boundResource("NULL"), "")
        self.assertEqual(FRB.IniNamingTools.boundResource("ResourceX"), "ResourceX")
