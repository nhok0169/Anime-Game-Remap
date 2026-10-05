import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


_Z3CTX = FRB.Z3Context()


class _FakeIni():
    """
    The narrow slice of :class:`IniFile` a resource edit actually reads

    A stand-in rather than the real thing, deliberately: a real :class:`IniFile` needs a parser, a
    fixer and a classifier before it will even construct, none of which a resource edit touches
    """

    def __init__(self, sections):
        self.folder = "C:/mods/raiden"
        self.sectionIfTemplates = sections
        self._z3Ctx = _Z3CTX
        self.resources = []
        self.version = None
        self.toVersion = None


class BaseResEditTest(BaseUnitTest):
    def _section(self, name: str, file: str) -> FRB.IfTemplate:
        return FRB.IfTemplate([FRB.IfContentPart({"filename": [(0, file)]}, 0)], name = name)

    def _ini(self, **sections) -> "_FakeIni":
        return _FakeIni(dict(sections))

    def _groups(self):
        return [FRB.IniGraphGroup({})]

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        resModObj = (0, "", "remapBlend")
        edit = FRB.BaseResEdit("resourceRemapBlend", resModObj)

        self.assertEqual(edit.resType, "resourceRemapBlend")
        self.assertIs(edit.resModObj, resModObj)

    def test_init_graphReplaceModeDefaultsToNone(self):
        self.assertIsNone(FRB.BaseResEdit("t", (0, "", "a")).graphReplaceMode)

    # ================================================
    # ================ fileAddGraphId =================

    def test_fileAddGraphId_insertsBeforeTheExtension(self):
        self.assertEqual(FRB.BaseResEdit.fileAddGraphId("a/b/Blend.buf", "ABC"), "a/b/Blend_ABC.buf")

    def test_fileAddGraphId_noExtension_appendsToTheWholeName(self):
        self.assertEqual(FRB.BaseResEdit.fileAddGraphId("Blend", "ABC"), "Blend_ABC")

    def test_fileAddGraphId_emptyId_stillInsertsTheSeparator(self):
        self.assertEqual(FRB.BaseResEdit.fileAddGraphId("Blend.buf"), "Blend_.buf")

    # ================================================
    # =================== getFileId ===================

    def test_getFileId_isDeterministic(self):
        part = FRB.IfContentPart({"filename": [(0, "x.buf")]}, 0)
        modObj = (0, "", "remapBlend")

        self.assertEqual(FRB.BaseResEdit.getFileId(modObj, "S", part, 0, "x.buf"),
                         FRB.BaseResEdit.getFileId(modObj, "S", part, 0, "x.buf"))

    def test_getFileId_differsByEveryComponent(self):
        part = FRB.IfContentPart({"filename": [(0, "x.buf")]}, 0)
        modObj = (0, "", "remapBlend")
        base = FRB.BaseResEdit.getFileId(modObj, "S", part, 0, "x.buf")

        self.assertNotEqual(base, FRB.BaseResEdit.getFileId((1, "", "remapBlend"), "S", part, 0, "x.buf"))
        self.assertNotEqual(base, FRB.BaseResEdit.getFileId(modObj, "T", part, 0, "x.buf"))
        self.assertNotEqual(base, FRB.BaseResEdit.getFileId(modObj, "S", part, 1, "x.buf"))
        self.assertNotEqual(base, FRB.BaseResEdit.getFileId(modObj, "S", part, 0, "y.buf"))

    # ================================================
    # ================ naming defaults ================

    def test_getFixResourceName_isTheRemapFixName(self):
        edit = FRB.BaseResEdit("t", (0, "", "a"))

        self.assertEqual(edit.getFixResourceName("ResourceRaidenBlend", None, "rika"),
                         FRB.IniNamingTools.getRemapFixName("ResourceRaidenBlend", modName = "rika"))

    def test_getFixFile_isTheFixedFileName(self):
        edit = FRB.BaseResEdit("t", (0, "", "a"))

        self.assertEqual(edit.getFixFile("RaidenBlend.buf", None, "rika"),
                         FRB.IniNamingTools.getFixedFile("RaidenBlend.buf", modName = "rika"))

    def test_collectResourceName_returnsBothNamesUnchanged(self):
        self.assertEqual(FRB.BaseResEdit("t", (0, "", "a")).collectResourceName("old", "new"), ("old", "new"))

    def test_renameUncollectedSection_usesGetFixResourceName(self):
        edit = FRB.BaseResEdit("t", (0, "", "a"))

        self.assertEqual(edit.renameUncollectedSection("Other", None, "rika"),
                         edit.getFixResourceName("Other", None, "rika"))

    def test_renameUncollectedSection_pythonOverrideIsHonoured(self):
        class MyEdit(FRB.BaseResEdit):
            def getFixResourceName(self, resource, modType, modName = ""):
                return "CUSTOM_" + resource

        self.assertEqual(MyEdit("t", (0, "", "a")).renameUncollectedSection("Other", None, "rika"), "CUSTOM_Other")

    # ================================================
    # ================= buildResources ================

    def test_buildResources_buildsTheGraphAndTheModels(self):
        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))
        graphGroups = self._groups()

        edit = FRB.BaseResEdit("resourceRemapBlend", (0, "", "remapBlend"))
        result = edit.buildResources({"ResourceRaidenBlend": "Fixed"}, None, ini, graphGroups, "rika")

        self.assertIs(result, graphGroups)
        self.assertIn(("", "remapBlend"), graphGroups[0].graphs)
        self.compareList(sorted(graphGroups[0].graphs[("", "remapBlend")].sections.keys()), ["Fixed"])
        self.assertEqual(len(ini.resources), 1)
        self.assertEqual(ini.resources[0].type, "resourceRemapBlend")

    def test_buildResources_doesNotRenameTheReferencedFile(self):
        # Unlike ResReplace, the base edit leaves the file path alone
        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))
        graphGroups = self._groups()

        FRB.BaseResEdit("t", (0, "", "remapBlend")).buildResources({"ResourceRaidenBlend": "Fixed"}, None, ini,
                                                                    graphGroups, "rika")

        graph = graphGroups[0].graphs[("", "remapBlend")]
        self.compareList(graph.getSection("Fixed").parts[0].entries(), [("filename", "RaidenBlend.buf")])

    def test_buildResources_iniIndexOutOfBounds_noOp(self):
        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))
        graphGroups = self._groups()

        FRB.BaseResEdit("t", (5, "", "remapBlend")).buildResources({"ResourceRaidenBlend": "Fixed"}, None, ini,
                                                                    graphGroups, "rika")

        self.assertEqual(len(graphGroups[0].graphs), 0)
        self.assertEqual(len(ini.resources), 0)

    def test_buildResources_uncollectedSectionsRenamedThroughGetFixResourceName(self):
        ini = self._ini(Collected = self._section("Collected", "a.buf"),
                        Other = self._section("Other", "b.buf"))
        graphGroups = self._groups()

        FRB.BaseResEdit("t", (0, "", "remapBlend")).buildResources({"Collected": "CollectedFixed"}, None, ini,
                                                                    graphGroups, "rika")

        # 'Other' is unreachable from the collected section, so it never enters the graph at all
        graph = graphGroups[0].graphs[("", "remapBlend")]
        self.compareList(sorted(graph.sections.keys()), ["CollectedFixed"])

    # ================================================
    # ================ buildResModels =================

    def test_buildResModels_collectsIntoTheGivenDictInsteadOfTheIni(self):
        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))
        graphGroups = self._groups()
        edit = FRB.BaseResEdit("t", (0, "", "remapBlend"))

        graph = edit.getResGraph({"ResourceRaidenBlend": "Fixed"}, None, ini, graphGroups, "rika")
        collected = {}
        edit.buildResModels(graph, ini, None, resources = collected, modName = "rika")

        self.assertEqual(len(collected), 1)
        self.assertEqual(len(ini.resources), 0)

    def test_buildResModels_resourceFilterRejects_theKvpIsRemoved(self):
        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))
        graphGroups = self._groups()
        edit = FRB.BaseResEdit("t", (0, "", "remapBlend"))

        graph = edit.getResGraph({"ResourceRaidenBlend": "Fixed"}, None, ini, graphGroups, "rika")
        edit.buildResModels(graph, ini, None, resourceFilter = lambda file, fileKey: False, modName = "rika")

        self.assertEqual(len(ini.resources), 0)
        self.compareList(graph.getSection("Fixed").parts[0].entries(), [])

    def test_buildResModels_graphId_isFoldedIntoTheFileName(self):
        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))
        graphGroups = self._groups()
        edit = FRB.BaseResEdit("t", (0, "", "remapBlend"))

        graph = edit.getResGraph({"ResourceRaidenBlend": "Fixed"}, None, ini, graphGroups, "rika")
        edit.buildResModels(graph, ini, None, modName = "rika", graphId = "someGraphId")

        entries = graph.getSection("Fixed").parts[0].entries()
        self.assertEqual(len(entries), 1)
        self.assertNotEqual(entries[0][1], "RaidenBlend.buf")
        self.assertTrue(entries[0][1].endswith(".buf"))

    # ================================================
    # ================ overridability =================

    def test_buildResModel_pythonOverrideIsUsed(self):
        built = []

        class MyEdit(FRB.BaseResEdit):
            def buildResModel(self, resType, ini, srcPath, *args, **kwargs):
                built.append(srcPath)
                return None

        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))
        MyEdit("t", (0, "", "remapBlend")).buildResources({"ResourceRaidenBlend": "Fixed"}, None, ini, self._groups(),
                                                           "rika")

        self.compareList(built, ["RaidenBlend.buf"])
        self.assertEqual(len(ini.resources), 0)
