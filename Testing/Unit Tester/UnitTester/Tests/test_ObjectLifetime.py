import os
import shutil
import sys
import tempfile
import unittest
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def makeSection(name):
    return FRB.IfTemplate([FRB.IfContentPart({"handling": [(0, "skip")]}, 0)], name = name)


class ObjectLifetimeTest(unittest.TestCase):
    """A Python object that wraps a section, a part or a graph without owning it does not outlive the object: once the
    library frees the object, the wrapper raises when used, and it is never handed back for a new object -- pybind11
    finds a wrapper by address, and a later object may be allocated at the same address"""

    def test_aSectionFreedUnderItsWrapperRaises(self):
        variables = FRB.GraphEditVariables()
        sections = variables.sections("fix")
        sections.add(makeSection("TextureOverrideMine"))
        borrowed = sections.list()[0]
        self.assertEqual(borrowed.name, "TextureOverrideMine")

        sections.clear()
        with self.assertRaises(Exception):
            borrowed.name

    def test_aPartFreedWithItsSectionRaises(self):
        variables = FRB.GraphEditVariables()
        sections = variables.sections("fix")
        sections.add(makeSection("TextureOverrideMine"))
        part = sections.list()[0].parts[0]
        part.id

        sections.clear()
        with self.assertRaises(Exception):
            part.id

    def test_aSectionOfAFreedGraphRaises(self):
        graph = FRB.IniSectionGraph({"TextureOverrideMine": makeSection("TextureOverrideMine")}, ["TextureOverrideMine"],
                                    copySections = True)
        borrowed = graph.sections["TextureOverrideMine"]
        self.assertEqual(borrowed.name, "TextureOverrideMine")

        del graph
        with self.assertRaises(Exception):
            borrowed.name

    def test_aNewSectionNeverGetsTheWrapperOfAFreedOne(self):
        variables = FRB.GraphEditVariables()
        sections = variables.sections("fix")
        freed = []
        for i in range(50):
            sections.add(makeSection("Old" + str(i)))
            freed.append(sections.list()[0])
            sections.clear()

        sections.add(makeSection("New"))
        fresh = sections.list()[0]
        self.assertFalse(any(fresh is old for old in freed))
        self.assertEqual(fresh.name, "New")


class IniFileLifetimeTest(unittest.TestCase):
    """A file the fix hands a parser is borrowed: once the fix is done with it, its wrapper raises when used, and a new
    file at the same address gets a wrapper of its own -- pybind11 used to hand back the freed file's wrapper there and
    delete the new file"""

    _iniTxt = "[TextureOverrideAmberBody]\nhash = cdb0ac07\nmatch_first_index = 0\n"

    def setUp(self):
        self._folder = tempfile.mkdtemp()
        for i in range(10):
            with open(os.path.join(self._folder, "mod%02d.ini" % i), "w") as file:
                file.write(self._iniTxt)

        self._borrowed = []
        def makeParser(ini, modTypeId):
            self._borrowed.append(ini)
            return FRB.GIMIParser(ini)

        FRB.CppStrategyOverrides.clear()
        FRB.CppStrategyOverrides.setParser("Amber", makeParser)
        try:
            FRB.RemapService(path = self._folder, keepBackups = False, downloadMode = FRB.DownloadMode.Disabled).fix()
        finally:
            FRB.CppStrategyOverrides.clear()

    def tearDown(self):
        shutil.rmtree(self._folder, ignore_errors = True)

    def test_aFreedFileRaises(self):
        self.assertEqual(len(self._borrowed), 10)
        with self.assertRaises(Exception):
            self._borrowed[0].file

    def test_aNewFileNeverGetsTheWrapperOfAFreedOne(self):
        live = FRB.IniFile(file = os.path.join(self._folder, "mod00.ini"))
        for _ in range(500):
            sibling = live.siblingOf(live)
            self.assertFalse(any(sibling is old for old in self._borrowed))
            self.assertEqual(sibling.file, live.file)


class BorrowSection(FRB.BaseIniGraphGroupEdit):
    """Keeps the wrapper of one section of one graph, as a graph or a reader borrowing it does"""

    def __init__(self, obj, section):
        super().__init__()
        self.obj, self.section, self.borrowed = obj, section, None

    def edit(self, graphGroups, modType, modName = ""):
        self.borrowed = graphGroups[0].graphs[self.obj].sections[self.section]
        return graphGroups


class GraphKeeperTest(BaseIniFileTest):
    """A graph an edit takes out of its group lives as long as the parser running the edit -- until the parser is
    cleared -- and not only as long as that one edit: a graph that borrows its sections, or a later edit, may still
    point into it"""

    _iniTxt = r"""
[TextureOverrideBody]
hash = 11111111
match_first_index = 0
drawindexed = 300, 0, 0
"""

    Body = ("", "body")
    Copy = ("", "copy")

    def test_aRemovedGraphLivesUntilItsParserIsCleared(self):
        # a graph of copies the parser's groups own (GraphCreate copies what it builds), borrowed, then taken out
        source = makeSection("TextureOverrideBody")
        create = FRB.GraphCreate((0, *self.Copy))
        create.sectionsOf = lambda group: [source]
        borrow = BorrowSection(self.Copy, "TextureOverrideBody")
        steps = [create, borrow, FRB.GraphRemove([(0, *self.Copy)])]

        self.setupIniTxt(self._iniTxt)
        self.createIniFile()
        parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([self.Body]), trackKeys = False, parseSteps = steps)
        self.useStrategies(parser = parser)
        self._iniFile.parse()

        self.assertNotIn(self.Copy, parser.collectParseResult()[0].graphs)
        self.assertEqual(borrow.borrowed.name, "TextureOverrideBody")

        parser.clear()
        with self.assertRaises(Exception):
            borrow.borrowed.name
