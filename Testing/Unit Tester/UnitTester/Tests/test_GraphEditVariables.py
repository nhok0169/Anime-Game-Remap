import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GraphEditVariablesTest(BaseUnitTest):
    Out = ("", "out")

    @classmethod
    def section(cls, name: str, value: str = "1") -> FRB.IfTemplate:
        return FRB.IfTemplate([FRB.IfContentPart({"a": [(0, value)]}, 0)], name = name)

    @classmethod
    def graph(cls, *names: str) -> FRB.IniSectionGraph:
        return FRB.IniSectionGraph({name: cls.section(name) for name in names}, list(names))

    @classmethod
    def names(cls, group: FRB.IniGraphGroup):
        return list(group.graphs[cls.Out].targetSectionNames)

    @classmethod
    def create(cls, variables, variablesOf):
        # a variable written out as a graph in each group: GraphCreate building each group's graph from its sections
        create = FRB.GraphCreate((0, *cls.Out))
        create.sectionsOf = lambda group: [section for name in variablesOf(group) for section in variables.sections(name).list()]
        return create

    # ====================== values of any type =================================

    def test_aValue_crossesAsTheFirstTypeBothKnow(self):
        variables = FRB.GraphEditVariables()
        held = object()
        for name, value in [("text", "3.6"), ("flag", True), ("count", 3), ("real", 0.5), ("held", held), ("none", None)]:
            variables[name] = value

        self.assertEqual(variables["text"], "3.6")
        self.assertIs(variables["flag"], True)
        self.assertEqual(variables["count"], 3)
        self.assertIsInstance(variables["count"], int)
        self.assertEqual(variables["real"], 0.5)
        self.assertIs(variables["held"], held)
        self.assertIsNone(variables["none"])
        self.assertEqual(variables.names(), ["text", "flag", "count", "real", "held", "none"])

    def test_setItem_replaces_delItemRemoves_aMissingOneIsAKeyError(self):
        variables = FRB.GraphEditVariables()
        variables["v"] = 1
        variables["v"] = "two"
        self.assertEqual(variables["v"], "two")
        self.assertIn("v", variables)

        del variables["v"]
        self.assertNotIn("v", variables)
        with self.assertRaises(KeyError):
            variables["v"]
        with self.assertRaises(KeyError):
            del variables["v"]

    def test_sections_onAVariableOfAnotherType_raises(self):
        variables = FRB.GraphEditVariables()
        variables["v"] = 1
        with self.assertRaises(Exception):
            variables.sections("v")

        variables.sections("s").add(self.section("One"))
        self.assertEqual(variables["s"].names(), ["One"])
        self.assertEqual(variables.sectionsNames(), ["s"])

    def test_clear_keepsTheSectionsVariablesEmpty_andDropsTheRest(self):
        variables = FRB.GraphEditVariables()
        variables.sections("s").add(self.section("One"))
        variables["v"] = 1
        variables.clear()

        self.assertEqual(variables.names(), ["s"])
        self.assertTrue(variables.sections("s").empty())

    def test_apply_replacesInto_withTheInputsInOrder(self):
        variables = FRB.GraphEditVariables()
        variables["a"], variables["b"], variables["into"] = 2, 3, "old"
        variables.apply(["b", "a", "missing"], lambda b, a, missing: f"{b}{a}{missing}", "into")
        self.assertEqual(variables["into"], "32None")

    def test_apply_returningNone_leavesInto(self):
        variables = FRB.GraphEditVariables()
        variables["into"] = "old"
        variables.apply([], lambda: None, "into")
        self.assertEqual(variables["into"], "old")

        variables.apply([], lambda: None, "new")
        self.assertNotIn("new", variables)

    def test_varApply_editsNoGraph_andAppliesWhenFiled(self):
        data = FRB.IniParseData()
        data.variables()["a"] = 2
        apply = FRB.GraphVarApply(["a"], lambda a: a * 10, "b")
        group = FRB.IniGraphGroup({("", "src"): self.graph("One")})

        apply.edit([group], None)
        self.assertEqual(list(group.graphs[("", "src")].targetSectionNames), ["One"])
        self.assertNotIn("b", data.variables())

        apply.fileInto(data)
        self.assertEqual(data.variables()["b"], 20)

    def test_varApply_inAFixersList_appliesOverItsVariablesWhenItRuns_inOrder(self):
        variables = FRB.GraphEditVariables()
        variables["a"] = 2
        first = FRB.GraphVarApply(["a"], lambda a: a * 10, "b", variables = variables)
        second = FRB.GraphVarApply(["a", "b"], lambda a, b: {"sum": a + b}, "c", variables = variables)
        group = FRB.IniGraphGroup({("", "src"): self.graph("One")})
        self.assertNotIn("b", variables)

        first.edit([group], None)
        second.edit([group], None)
        self.assertEqual(variables["b"], 20)
        self.assertEqual(variables["c"], {"sum": 22})
        self.assertEqual(list(group.graphs[("", "src")].targetSectionNames), ["One"])

    def test_varApply_fieldsReadAndWrite(self):
        apply = FRB.GraphVarApply(["a"], lambda a: a, "b")
        apply.inputs, apply.into = ["x", "y"], "z"
        self.assertEqual((apply.inputs, apply.into), (["x", "y"], "z"))

        apply.fn = lambda x, y: x + y
        self.assertEqual(apply.fn(1, 2), 3)

    # ====================== the variables ======================================

    def test_sections_madeOnFirstAsk_inTheOrderMade(self):
        variables = FRB.GraphEditVariables()
        variables.sections("b").add(self.section("One"))
        variables.sections("a")

        self.assertEqual(variables.sectionsNames(), ["b", "a"])
        self.assertEqual(variables.sections("b").names(), ["One"])
        self.assertTrue(variables.sections("a").empty())

    def test_add_takesACopy(self):
        variables = FRB.GraphEditVariables()
        section = self.section("One")
        variables.sections("v").add(section)
        section.name = "RenamedAfter"

        self.assertEqual(variables.sections("v").names(), ["One"])

    def test_reserve_aLaterAddAtLandsInTheHeldPlace(self):
        sections = FRB.GraphEditVariables().sections("v")
        sections.add(self.section("Before"))
        place = sections.reserve()
        sections.add(self.section("After"))
        sections.addAt(self.section("Held"), place)

        self.assertEqual(sections.names(), ["Before", "Held", "After"])

    def test_reserve_thenClear_addAtAppends(self):
        sections = FRB.GraphEditVariables().sections("v")
        place = sections.reserve()
        sections.clear()
        sections.add(self.section("After"))
        sections.addAt(self.section("Held"), place)

        self.assertEqual(sections.names(), ["After", "Held"])

    def test_clear_everyVariable(self):
        variables = FRB.GraphEditVariables()
        variables.sections("a").add(self.section("One"))
        variables.sections("b").add(self.section("Two"))

        variables.clear()

        self.assertTrue(variables.sections("a").empty() and variables.sections("b").empty())

    def test_of_eachNameHoldsItsValue_aNoneMakesNoVariable(self):
        variables = FRB.GraphEditVariables.of(["a", "b", "c"], ["text", None, 3])
        self.assertEqual(variables.names(), ["a", "c"])
        self.assertEqual((variables["a"], variables["c"]), ("text", 3))

    def test_aParsersResourceData_crossesAsACopy(self):
        variables = FRB.GraphEditVariables()
        data = FRB.IniParseData.ResourceData()
        data.undrawn = {("", "body")}
        variables["res"] = data
        data.undrawn = set()

        held = variables["res"]
        self.assertIsInstance(held, FRB.IniParseData.ResourceData)
        self.assertEqual(held.undrawn, {("", "body")})
        held.undrawn = set()
        self.assertEqual(variables["res"].undrawn, {("", "body")})

        roles = FRB.IniParseData.RoleFileData()
        roles.fileResources = {"a.dds": "ResourceA"}
        variables["roles"] = roles
        self.assertEqual(variables["roles"].fileResources, {"a.dds": "ResourceA"})

    # ====================== GraphCreate.sectionsOf =============================

    def test_create_eachGroupFromItsVariables_inOrder(self):
        variables = FRB.GraphEditVariables()
        variables.sections("shared").add(self.section("Shared"))
        variables.sections("one").add(self.section("OnlyOne"))
        groups = [FRB.IniGraphGroup(), FRB.IniGraphGroup()]

        self.create(variables, lambda group: ["shared", "one"] if group == 1 else ["shared"]).edit(groups, None)

        self.assertEqual(self.names(groups[0]), ["Shared"])
        self.assertEqual(self.names(groups[1]), ["Shared", "OnlyOne"])

    def test_create_aGroupWithNothing_getsNoGraph(self):
        variables = FRB.GraphEditVariables()
        variables.sections("v").add(self.section("One"))
        groups = [FRB.IniGraphGroup(), FRB.IniGraphGroup()]

        self.create(variables, lambda group: ["v"] if group == 0 else ["missing"]).edit(groups, None)

        self.assertNotIn(self.Out, groups[1].graphs)

    def test_create_aRepeatedName_keepsTheFirst(self):
        variables = FRB.GraphEditVariables()
        variables.sections("a").add(self.section("One", "first"))
        variables.sections("b").add(self.section("One", "second"))
        groups = [FRB.IniGraphGroup()]

        self.create(variables, lambda group: ["a", "b"]).edit(groups, None)

        self.assertEqual(self.names(groups[0]), ["One"])
        self.assertIn("first", groups[0].graphs[self.Out].toStr())
        self.assertNotIn("second", groups[0].graphs[self.Out].toStr())

    def test_create_noGroups_writesGroupZeroIntoAGroupOfItsOwn(self):
        variables = FRB.GraphEditVariables()
        variables.sections("v").add(self.section("One"))
        groups = []

        groups = self.create(variables, lambda group: ["v"]).edit(groups, None)

        self.assertEqual(len(groups), 1)
        self.assertEqual(self.names(groups[0]), ["One"])

    def test_list_isTheVariablesOwnSections(self):
        sections = FRB.GraphEditVariables().sections("v")
        sections.add(self.section("One"))
        sections.add(self.section("Two"))

        self.assertEqual([section.name for section in sections.list()], ["One", "Two"])

    # ====================== GraphVarTake ====================================

    def test_take_copiesTheNamedSections_underTheComment_andRemovesTheGraph(self):
        variables = FRB.GraphEditVariables()
        groups = [FRB.IniGraphGroup({("", "src"): self.graph("A", "B")})]

        FRB.GraphVarTake(variables, "v", ("", "src"), ["B", "Missing"], "taken").edit(groups, None)

        self.assertNotIn(("", "src"), groups[0].graphs)
        self.assertEqual(variables.sections("v").names(), ["B"])

        self.create(variables, lambda group: ["v"]).edit(groups, None)
        self.assertIn("; taken", groups[0].graphs[self.Out].toStr())

    def test_take_inPlace_keepsThePlaceHeldWhenMade(self):
        def run(inPlace: bool):
            variables = FRB.GraphEditVariables()
            groups = [FRB.IniGraphGroup({("", "src"): self.graph("Taken")})]
            variables.sections("v").add(self.section("Before"))
            take = FRB.GraphVarTake(variables, "v", ("", "src"), ["Taken"], inPlace = inPlace)
            variables.sections("v").add(self.section("After"))
            take.edit(groups, None)
            return variables.sections("v").names()

        self.assertEqual(run(True), ["Before", "Taken", "After"])
        self.assertEqual(run(False), ["Before", "After", "Taken"])

    def test_editsInAFixersEditList_runThroughPython(self):
        # what a Python fixer does with them: a list of edits handed to editFromIni one by one
        variables = FRB.GraphEditVariables()
        variables.sections("v").add(self.section("Own"))
        groups = [FRB.IniGraphGroup({("", "src"): self.graph("Taken")})]

        for edit in [FRB.GraphVarTake(variables, "v", ("", "src"), ["Taken"]), self.create(variables, lambda group: ["v"])]:
            edit.editFromIni(groups, None, None)

        self.assertEqual(self.names(groups[0]), ["Own", "Taken"])
