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


class IniSectionGraphTest(BaseUnitTest):
    # "parent" calls "child" via 'run ='; "child" makes no call of its own
    def makeParentChildSections(self):
        return {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)]),
        }

    # ========= construct / basic properties ====================================

    def test_construct_inlineSections_graphBuiltFromRoots(self):
        # sections constructed entirely inline, with no separate Python reference held to them --
        # the pattern this port's own keepAlive_ fix (see PyIniSectionGraph.h) exists to protect
        graph = FRB.IniSectionGraph({
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)]),
        }, ["parent"])

        self.assertEqual(set(graph.sections.keys()), {"parent", "child"})
        self.assertEqual(graph.roots, ["parent"])
        self.assertEqual(graph.targetSectionNames, ["parent"])
        self.assertFalse(graph.isEmpty())

    def test_construct_unreachableSection_notIncludedInGraph(self):
        sections = self.makeParentChildSections()
        sections["orphan"] = FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)])
        graph = FRB.IniSectionGraph(sections, ["parent"])

        self.assertEqual(set(graph.sections.keys()), {"parent", "child"})

    def test_construct_noTargetSections_isEmpty(self):
        graph = FRB.IniSectionGraph({}, [])
        self.assertTrue(graph.isEmpty())

    def test_neighbours_parentHasChildAsNeighbour(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        # a section with no out-neighbours of its own (eg. "child" here) simply has no key at all,
        # same convention as CallGraph.forwardEdges
        self.assertEqual(graph.neighbours, {"parent": ["child"]})

    # ========================================================
    # ========= getSection / getRootSections =================

    def test_getSection_existingSection_returnsIt(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        self.assertIs(graph.getSection("parent"), sections["parent"])

    def test_getSection_missingSection_raiseExceptionFalse_returnsNone(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        self.assertIsNone(graph.getSection("nonexistent", raiseException = False))

    def test_getSection_missingSection_raiseExceptionTrue_raises(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        with self.assertRaises(Exception):
            graph.getSection("nonexistent")

    def test_getRootSections_returnsOnlyTheRoots(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        roots = graph.getRootSections()
        self.assertEqual(len(roots), 1)
        self.assertIs(roots[0], sections["parent"])

    # ========================================================
    # ========= getNeighbourNames / getNeighbours ============

    def test_getNeighbourNames_parent_returnsChild(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        self.assertEqual(graph.getNeighbourNames("parent"), ["child"])

    def test_getNeighbours_parent_returnsChildSection(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        neighbours = graph.getNeighbours("parent")
        self.assertEqual(set(neighbours.keys()), {"child"})
        self.assertIs(neighbours["child"], sections["child"])

    # ========================================================
    # ========= getChildren ==================================

    def test_getChildren_getNeighbourChildrenTrue_includesChild(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        result = graph.getChildren(["parent"], True)
        self.assertIn("child", result)

    def test_getChildren_getNeighbourChildrenFalse_excludesNeighbourOnlyChild(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])
        result = graph.getChildren(["parent"], False)
        self.assertNotIn("child", result)

    # ========================================================
    # ========= combine ======================================

    def test_combine_twoIndependentGraphs_sectionsMerged(self):
        graph1 = FRB.IniSectionGraph({"a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)])}, ["a"])
        graph2 = FRB.IniSectionGraph({"b": FRB.IfTemplate([FRB.IfContentPart({"y": [(0, "2")]}, 0)])}, ["b"])

        graph1.combine([graph2])
        self.assertEqual(set(graph1.sections.keys()), {"a", "b"})

    # ========================================================
    # ========= rename ========================================

    def test_rename_sectionsRenamedAndRunCallsUpdated(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])

        graph.rename(lambda name: name.upper())

        self.assertEqual(set(graph.sections.keys()), {"PARENT", "CHILD"})
        parentSection = graph.getSection("PARENT")
        runVals = parentSection.parts[0].getVals("run")
        self.assertEqual(runVals, ["CHILD"])

    # ========================================================
    # ========= deepcopy ======================================

    def test_deepcopy_resultIsIndependentCopy(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])

        copied = graph.deepcopy()
        self.assertIsNot(copied, graph)
        self.assertEqual(set(copied.sections.keys()), {"parent", "child"})
        self.assertIsNot(copied.getSection("parent"), graph.getSection("parent"))

        # mutating the copy must not affect the original
        copied.getSection("parent").parts[0].addKVP("newKey", "newVal")
        self.assertFalse(graph.getSection("parent").parts[0].contains("newKey"))

    def test_copy_moduleLevel_matchesDeepcopy(self):
        import copy
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])

        copied = copy.deepcopy(graph)
        self.assertIsNot(copied, graph)
        self.assertEqual(set(copied.sections.keys()), {"parent", "child"})

    # ========================================================
    # ========= isKeyFullyCover / rootsAreFullyCovered / getKeyMissingParts
    # NOTE: these are keyed by each IfTemplate's own .name attribute (not by the key used in the
    # 'sections' dict passed to the constructor) -- every section below is built with an explicit
    # name = "a" to match.

    def test_isKeyFullyCover_keyInEveryBranch_true(self):
        sections = {"a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)], name = "a")}
        graph = FRB.IniSectionGraph(sections, ["a"])
        self.assertEqual(graph.isKeyFullyCover("x"), {"a": True})

    def test_isKeyFullyCover_keyMissingFromSomeBranch_false(self):
        sections = {"a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)], name = "a")}
        graph = FRB.IniSectionGraph(sections, ["a"])
        self.assertEqual(graph.isKeyFullyCover("z"), {"a": False})

    def test_rootsAreFullyCovered_matchesIsKeyFullyCoverForRootSections(self):
        sections = {"a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)], name = "a")}
        graph = FRB.IniSectionGraph(sections, ["a"])
        self.assertEqual(graph.rootsAreFullyCovered("x"), {"a": True})
        self.assertEqual(graph.rootsAreFullyCovered("z"), {"a": False})

    def test_getKeyMissingParts_keyMissingFromSection_partReturnedForThatSection(self):
        sections = {"a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)], name = "a")}
        graph = FRB.IniSectionGraph(sections, ["a"])
        result = graph.getKeyMissingParts("z")
        self.assertIn("a", result)
        self.assertEqual(len(result["a"]), 1)

    def test_getKeyMissingParts_keyPresentEverywhere_emptyResult(self):
        sections = {"a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)], name = "a")}
        graph = FRB.IniSectionGraph(sections, ["a"])
        result = graph.getKeyMissingParts("x")
        self.assertEqual(result.get("a", set()), set())

    def test_getKeyMissingParts_allCalledSubCommandsFullyMissing_consolidatesToTheCallingPart(self):
        # "main" calls both "subA" and "subB" (two separate 'run =' targets on the same content
        # part); the key is missing everywhere -- in "main" itself and in both subcommands. Since
        # *everything* reachable is missing, the whole subtree collapses to just "main"'s own
        # missing location, rather than separately reporting "subA"'s and "subB"'s own parts too.
        # Regression test for a real bug found by inspection (not by a failing test): every one of
        # getKeyMissingPartsNode's three loops (branch children / already-visited subcommands /
        # not-yet-visited subcommands) was incrementing the *same* shared counter instead of each
        # loop's own -- harmless on its own, since the three are only ever summed together -- but
        # the not-yet-visited-subcommands loop compounded it by never refreshing its own
        # "all branches missing" flag per subcommand, silently reusing a stale value left over from
        # the branch-children loop for every iteration. With two subcommands both fully missing,
        # that undercounts (0 of 2, instead of 2 of 2), so the buggy version wrongly falls through
        # to reporting the deeper {subA's part, subB's part} pair instead of consolidating to
        # {main's part}.
        sections = {
            "main": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "subA"), (2, "subB")]}, 0)], name = "main"),
            "subA": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "subA"),
            "subB": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "3")]}, 0)], name = "subB"),
        }
        graph = FRB.IniSectionGraph(sections, ["main"])

        result = graph.getKeyMissingParts("target")

        self.assertEqual(len(result["main"]), 1)
        mainMissingPart = next(iter(result["main"]))
        self.assertIs(mainMissingPart, sections["main"].parts[0])

    def test_getKeyMissingParts_onlySomeCalledSubCommandsMissing_reportsOnlyThoseSubCommands(self):
        # Mirror of the above, but "subB" *has* the key -- so not everything reachable is missing,
        # and the result should stay at the deeper, specific "subA" location rather than
        # consolidating up to "main" (which, unlike the all-missing case, isn't itself a valid
        # insertion point here).
        sections = {
            "main": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "subA"), (2, "subB")]}, 0)], name = "main"),
            "subA": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)], name = "subA"),
            "subB": FRB.IfTemplate([FRB.IfContentPart({"target": [(0, "3")]}, 0)], name = "subB"),
        }
        graph = FRB.IniSectionGraph(sections, ["main"])

        result = graph.getKeyMissingParts("target")

        self.assertEqual(len(result["main"]), 1)
        mainMissingPart = next(iter(result["main"]))
        self.assertIs(mainMissingPart, sections["subA"].parts[0])
        self.assertEqual(result["subB"], set())

    # ========================================================
    # ========= normalize ======================================

    def test_normalize_ifWithNoElse_syntheticElseInsertedIntoSection(self):
        sections = {"a": FRB.IfTemplate([
            FRB.IfContentPart({"x": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, FRB.Z3Context()),
            FRB.IfContentPart({"y": [(0, "2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, FRB.Z3Context()),
        ])}
        graph = FRB.IniSectionGraph(sections, ["a"])

        graph.normalize()
        self.assertEqual(len(graph.getSection("a")), 6)

    # ========================================================
    # ========= __iter__ (iterSections) ======================

    def test_iterSections_dfsOrder_parentBeforeChild(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])

        result = list(graph)
        names = [name for name, section in result]
        self.assertEqual(names, ["parent", "child"])
        self.assertIs(result[0][1], sections["parent"])
        self.assertIs(result[1][1], sections["child"])

    # ========================================================
    # ========= computeSectionPredecessors (static) ==========

    # NOTE: 'section.parts' is fetched *once* and held in a local variable before calling
    # computeSectionPredecessors -- like buildCallGraph()/buildPartPredecessorGraph() (see
    # PyIniSectionGraph.h's own top-level note), this static method only guarantees id(part)
    # consistency for parts that already have a live wrapper *before* it runs; a second, separate
    # '.parts' access afterward is not guaranteed to reuse the same wrapper.
    def test_computeSectionPredecessors_noBranching_eachPartDependsOnPrevious(self):
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfContentPart({"b": [(0, "2")]}, 0),
        ])
        part0, part1 = section.parts
        result = FRB.IniSectionGraph.computeSectionPredecessors(section)

        self.assertEqual(result[id(part0)], [])
        self.assertEqual(result[id(part1)], [id(part0)])

    def test_computeSectionPredecessors_ifElse_eachBranchDependsOnlyOnWhatPrecededTheIf(self):
        z3Ctx = FRB.Z3Context()
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, z3Ctx),
            FRB.IfContentPart({"b": [(0, "2")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, z3Ctx),
            FRB.IfContentPart({"c": [(0, "3")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, z3Ctx),
        ])
        contentParts = [p for p in section.parts if isinstance(p, FRB.IfContentPart)]
        headPart, ifBranchPart, elseBranchPart = contentParts

        result = FRB.IniSectionGraph.computeSectionPredecessors(section)

        self.assertEqual(result[id(headPart)], [])
        self.assertEqual(result[id(ifBranchPart)], [id(headPart)])
        self.assertEqual(result[id(elseBranchPart)], [id(headPart)])

    def test_computeSectionPredecessors_afterEndIfWithElse_dependsOnEveryBranchOnly(self):
        # a part right after "endIf" depends on *every* branch's own ending part -- and, since this
        # if/else has an "else" covering the "no branch taken" case, NOT on whatever preceded the
        # "if" (that path is unreachable without going through one of the branches first)
        z3Ctx = FRB.Z3Context()
        section = FRB.IfTemplate([
            FRB.IfContentPart({"pre": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, z3Ctx),
            FRB.IfContentPart({"ifb": [(0, "1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, z3Ctx),
            FRB.IfContentPart({"elseb": [(0, "1")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, z3Ctx),
            FRB.IfContentPart({"post": [(0, "1")]}, 0),
        ])
        prePart, ifPart, elsePart, postPart = [p for p in section.parts if isinstance(p, FRB.IfContentPart)]

        result = FRB.IniSectionGraph.computeSectionPredecessors(section)

        self.assertEqual(set(result[id(postPart)]), {id(ifPart), id(elsePart)})
        self.assertNotIn(id(prePart), result[id(postPart)])

    def test_computeSectionPredecessors_afterEndIfWithoutElse_alsoDependsOnWhatPrecededTheIf(self):
        # mirror of the above, but with no "else" -- the "if" might not have been taken at all, so
        # a part right after "endIf" must ALSO depend on whatever preceded the "if"
        z3Ctx = FRB.Z3Context()
        section = FRB.IfTemplate([
            FRB.IfContentPart({"pre": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, z3Ctx),
            FRB.IfContentPart({"ifb": [(0, "1")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, z3Ctx),
            FRB.IfContentPart({"post": [(0, "1")]}, 0),
        ])
        prePart, ifPart, postPart = [p for p in section.parts if isinstance(p, FRB.IfContentPart)]

        result = FRB.IniSectionGraph.computeSectionPredecessors(section)

        self.assertEqual(set(result[id(postPart)]), {id(ifPart), id(prePart)})

    def test_computeSectionPredecessors_elif_eachBranchStillOnlyDependsOnWhatPrecededTheIf(self):
        # if/elif/else -- every branch (not just the first "if") only depends on whatever preceded
        # the whole if/elif/else chain, never on a sibling branch
        z3Ctx = FRB.Z3Context()
        section = FRB.IfTemplate([
            FRB.IfContentPart({"pre": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, z3Ctx),
            FRB.IfContentPart({"ifb": [(0, "1")]}, 1),
            FRB.IfPredPart("elif $i == 1", FRB.IfPredPartType.Elif, z3Ctx),
            FRB.IfContentPart({"elifb": [(0, "1")]}, 1),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, z3Ctx),
            FRB.IfContentPart({"elseb": [(0, "1")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, z3Ctx),
        ])
        prePart, ifPart, elifPart, elsePart = [p for p in section.parts if isinstance(p, FRB.IfContentPart)]

        result = FRB.IniSectionGraph.computeSectionPredecessors(section)

        self.assertEqual(result[id(ifPart)], [id(prePart)])
        self.assertEqual(result[id(elifPart)], [id(prePart)])
        self.assertEqual(result[id(elsePart)], [id(prePart)])

    # A stray 'endif'/'else'/'elif' with no open 'if' shows up in real mods (a lone 'endif' at the
    # end of a Resource section was found in a production mod folder). IfTemplateTree.construct
    # skips such a part; computeSectionPredecessors used to index an empty frame stack there --
    # undefined behaviour that crashed outright on that mod, and silently corrupted the heap on
    # others. These pin the "treated as a pass-through" behaviour.
    def test_computeSectionPredecessors_strayEndIfWithNoIf_isPassThrough(self):
        z3Ctx = FRB.Z3Context()
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, z3Ctx),
            FRB.IfContentPart({"b": [(0, "2")]}, 0),
        ])
        partA, partB = [p for p in section.parts if isinstance(p, FRB.IfContentPart)]

        result = FRB.IniSectionGraph.computeSectionPredecessors(section)

        self.assertEqual(result[id(partA)], [])
        self.assertEqual(result[id(partB)], [id(partA)])

    def test_computeSectionPredecessors_strayElseAndElifWithNoIf_arePassThrough(self):
        z3Ctx = FRB.Z3Context()
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("else", FRB.IfPredPartType.Else, z3Ctx),
            FRB.IfContentPart({"b": [(0, "2")]}, 0),
            FRB.IfPredPart("elif $i == 1", FRB.IfPredPartType.Elif, z3Ctx),
            FRB.IfContentPart({"c": [(0, "3")]}, 0),
        ])
        partA, partB, partC = [p for p in section.parts if isinstance(p, FRB.IfContentPart)]

        result = FRB.IniSectionGraph.computeSectionPredecessors(section)

        self.assertEqual(result[id(partA)], [])
        self.assertEqual(result[id(partB)], [id(partA)])
        self.assertEqual(result[id(partC)], [id(partB)])

    def test_computeSectionPredecessors_strayEndIfAfterAClosedIf_isPassThrough(self):
        # the frame stack has held (and popped) a frame before the stray 'endif' arrives -- the
        # case where the old code read a moved-from frame and underflowed the stack instead of
        # faulting immediately
        z3Ctx = FRB.Z3Context()
        section = FRB.IfTemplate([
            FRB.IfContentPart({"a": [(0, "1")]}, 0),
            FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, z3Ctx),
            FRB.IfContentPart({"b": [(0, "2")]}, 1),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, z3Ctx),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, z3Ctx),
            FRB.IfContentPart({"c": [(0, "3")]}, 0),
        ])
        partA, partB, partC = [p for p in section.parts if isinstance(p, FRB.IfContentPart)]

        result = FRB.IniSectionGraph.computeSectionPredecessors(section)

        self.assertEqual(result[id(partB)], [id(partA)])
        # after the real endIf (no else): depends on the if-branch's end AND on what preceded the if
        self.compareSet(set(result[id(partC)]), {id(partB), id(partA)})

    def test_buildCallGraph_strayEndIfWithNoIf_doesNotCrash(self):
        z3Ctx = FRB.Z3Context()
        sections = {"res": FRB.IfTemplate([
            FRB.IfContentPart({"filename": [(0, "x.dds")]}, 0),
            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, z3Ctx),
        ])}
        graph = FRB.IniSectionGraph(sections, ["res"])

        self.assertEqual(len(graph.buildPartPredecessorGraph()), 1)
        self.assertEqual(len(graph.buildCallGraph().partsById), 1)

    # ========================================================
    # ========= buildPartPredecessorGraph ====================

    def test_buildPartPredecessorGraph_runCall_calleeEntryDependsOnCallingPart(self):
        sections = self.makeParentChildSections()
        graph = FRB.IniSectionGraph(sections, ["parent"])

        predecessors = graph.buildPartPredecessorGraph()
        parentPart = sections["parent"].parts[0]
        childPart = sections["child"].parts[0]

        self.assertEqual(predecessors[id(parentPart)], [])
        self.assertEqual(predecessors[id(childPart)], [id(parentPart)])

    def test_buildPartPredecessorGraph_selfReferencingRunCall_doesNotHang(self):
        # a section whose only 'run =' target is itself -- this must terminate (not infinite-loop)
        # and the part ends up listed as its own predecessor, which is harmless for the dedup
        # purposes this graph exists for (see IniGraphEditing/CLAUDE.md's own note on this)
        sections = {"a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")], "run": [(1, "a")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["a"])
        part = sections["a"].parts[0]

        predecessors = graph.buildPartPredecessorGraph()

        self.assertEqual(predecessors[id(part)], [id(part)])

    def test_buildPartPredecessorGraph_mutualRunCalls_bothDirectionsRecorded(self):
        # "a" calls "b" and "b" calls "a" -- both directions must be recorded independently, not
        # just one side winning
        sections = {
            "a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")], "run": [(1, "b")]}, 0)]),
            "b": FRB.IfTemplate([FRB.IfContentPart({"y": [(0, "2")], "run": [(1, "a")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["a"])
        partA = sections["a"].parts[0]
        partB = sections["b"].parts[0]

        predecessors = graph.buildPartPredecessorGraph()

        self.assertEqual(predecessors[id(partA)], [id(partB)])
        self.assertEqual(predecessors[id(partB)], [id(partA)])
