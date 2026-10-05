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


class SectionIterDataTest(BaseUnitTest):
    def makeParentChildGraph(self):
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"], z3Ctx = FRB.Z3Context())
        return graph, sections

    # ========== SectionIterData (iterByContentPart) ====================================

    def test_iterByContentPart_noColour_fieldsPopulatedColouringIsNone(self):
        graph, sections = self.makeParentChildGraph()
        results = list(graph.iterByContentPart())

        self.assertEqual(len(results), 2)
        parentData, childData = results

        self.assertEqual(parentData.sectionName, "parent")
        self.assertIs(parentData.section, sections["parent"])
        self.assertIs(parentData.part, sections["parent"].parts[0])
        self.assertIsNone(parentData.colouring)

        self.assertEqual(childData.sectionName, "child")
        self.assertIs(childData.section, sections["child"])
        self.assertIs(childData.part, sections["child"].parts[0])

    def test_iterByContentPart_colourTrue_colouringIsPopulated(self):
        graph, sections = self.makeParentChildGraph()
        results = list(graph.iterByContentPart(colour = True))

        for data in results:
            self.assertIsInstance(data.colouring, FRB.IfContentPartColouring)

    def test_iterByContentPart_state_isAnInt(self):
        graph, sections = self.makeParentChildGraph()
        data = next(iter(graph.iterByContentPart()))
        self.assertIsInstance(data.state, int)

    # ========== SectionIterQueryData (iterByQuery) ====================================

    def test_iterByQuery_noBranching_queryIsTheTruePredicate(self):
        graph, sections = self.makeParentChildGraph()
        results = list(graph.iterByQuery())

        self.assertEqual(len(results), 2)
        parentData, childData = results

        self.assertIsInstance(parentData.query, FRB.Z3Predicate)
        self.assertTrue(parentData.query.isSatisfiable())

        self.assertEqual(parentData.sectionName, "parent")
        self.assertIs(parentData.section, sections["parent"])
        self.assertEqual(parentData.rootSectionName, "parent")
        self.assertIs(parentData.rootSection, sections["parent"])
        self.assertIs(parentData.part, sections["parent"].parts[0])

        # "child" is only reachable via "parent"'s own 'run =' call -- its root section is still
        # "parent", not "child"
        self.assertEqual(childData.sectionName, "child")
        self.assertEqual(childData.rootSectionName, "parent")
        self.assertIs(childData.rootSection, sections["parent"])

    def test_iterByQuery_colourTrue_colouringIsPopulated(self):
        graph, sections = self.makeParentChildGraph()
        results = list(graph.iterByQuery(colour = True))
        for data in results:
            self.assertIsInstance(data.colouring, FRB.IfContentPartColouring)
