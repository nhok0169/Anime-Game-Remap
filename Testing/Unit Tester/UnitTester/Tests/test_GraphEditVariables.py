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

    # ====================== VarSectionsTake ====================================

    def test_take_copiesTheNamedSections_underTheComment_andRemovesTheGraph(self):
        variables = FRB.GraphEditVariables()
        groups = [FRB.IniGraphGroup({("", "src"): self.graph("A", "B")})]

        FRB.VarSectionsTake(variables, "v", ("", "src"), ["B", "Missing"], "taken").edit(groups, None)

        self.assertNotIn(("", "src"), groups[0].graphs)
        self.assertEqual(variables.sections("v").names(), ["B"])

        self.create(variables, lambda group: ["v"]).edit(groups, None)
        self.assertIn("; taken", groups[0].graphs[self.Out].toStr())

    def test_take_inPlace_keepsThePlaceHeldWhenMade(self):
        def run(inPlace: bool):
            variables = FRB.GraphEditVariables()
            groups = [FRB.IniGraphGroup({("", "src"): self.graph("Taken")})]
            variables.sections("v").add(self.section("Before"))
            take = FRB.VarSectionsTake(variables, "v", ("", "src"), ["Taken"], inPlace = inPlace)
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

        for edit in [FRB.VarSectionsTake(variables, "v", ("", "src"), ["Taken"]), self.create(variables, lambda group: ["v"])]:
            edit.editFromIni(groups, None, None)

        self.assertEqual(self.names(groups[0]), ["Own", "Taken"])
