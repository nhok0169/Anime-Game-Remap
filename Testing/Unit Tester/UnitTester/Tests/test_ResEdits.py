import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


_Z3CTX = FRB.Z3Context()


class _FakeIni():
    """
    The narrow slice of :class:`IniFile` a resource edit actually reads -- see
    ``test_BaseResEdit.py``'s own copy for why a stand-in is used rather than the real thing
    """

    def __init__(self, sections):
        self.folder = "C:/mods/raiden"
        self.sectionIfTemplates = sections
        self._z3Ctx = _Z3CTX
        self.resources = []
        self.version = None
        self.toVersion = None


class ResEditsTest(BaseUnitTest):
    """
    Covers the three concrete resource edits that share :class:`BaseResEdit`'s surface --
    :class:`ResIdentity`, :class:`ResReplace` and :class:`ResCreate`
    """

    def _section(self, name: str, file: str) -> FRB.IfTemplate:
        return FRB.IfTemplate([FRB.IfContentPart({"filename": [(0, file)]}, 0)], name = name)

    def _ini(self, **sections) -> "_FakeIni":
        return _FakeIni(dict(sections))

    def _groups(self):
        return [FRB.IniGraphGroup({})]

    # ================================================
    # ================== ResIdentity ==================

    def test_resIdentity_getFixResourceName_isAlwaysNone(self):
        self.assertIsNone(FRB.ResIdentity((0, "", "a")).getFixResourceName("anything", None, "rika"))

    def test_resIdentity_buildsTheGraphAndTheModels(self):
        ini = self._ini(ResourceBlend = self._section("ResourceBlend", "Blend.buf"))
        graphGroups = self._groups()

        FRB.ResIdentity((0, "", "remapBlend")).buildResources({"ResourceBlend": "ResourceBlend"}, None, ini,
                                                               graphGroups, "rika")

        self.assertIn(("", "remapBlend"), graphGroups[0].graphs)
        self.assertEqual(len(ini.resources), 1)

    def test_resIdentity_createResModelFalse_buildsTheGraphButNoModels(self):
        ini = self._ini(ResourceBlend = self._section("ResourceBlend", "Blend.buf"))
        graphGroups = self._groups()

        edit = FRB.ResIdentity((0, "", "remapBlend"), createResModel = False)
        self.assertFalse(edit.createResModel)
        edit.buildResources({"ResourceBlend": "ResourceBlend"}, None, ini, graphGroups, "rika")

        self.assertIn(("", "remapBlend"), graphGroups[0].graphs)
        self.assertEqual(len(ini.resources), 0)

    def test_resIdentity_isABaseResEdit(self):
        self.assertIsInstance(FRB.ResIdentity((0, "", "a")), FRB.BaseResEdit)

    # ================================================
    # =================== ResReplace ==================

    def test_resReplace_renamesTheFileAndBuildsAFixResource(self):
        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))
        graphGroups = self._groups()

        FRB.ResReplace("resourceRemapBlend", (0, "", "remapBlend")).buildResources(
            {"ResourceRaidenBlend": "Fixed"}, None, ini, graphGroups, "rika")

        graph = graphGroups[0].graphs[("", "remapBlend")]
        entries = graph.getSection("Fixed").parts[0].entries()

        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0][1], FRB.IniNamingTools.getFixedFile("RaidenBlend.buf", modName = "rika"))
        self.assertEqual(len(ini.resources), 1)
        self.assertIsInstance(ini.resources[0], FRB.IniFixResource)

    def test_resReplace_resourceCarriesBothPaths(self):
        ini = self._ini(ResourceRaidenBlend = self._section("ResourceRaidenBlend", "RaidenBlend.buf"))

        FRB.ResReplace("resourceRemapBlend", (0, "", "remapBlend")).buildResources(
            {"ResourceRaidenBlend": "Fixed"}, None, ini, self._groups(), "rika")

        resource = ini.resources[0]
        self.assertTrue(resource.srcPath.endswith("RaidenBlend.buf"))
        self.assertNotEqual(resource.srcPath, resource.fixedPath)

    def test_resReplace_isABaseResEdit(self):
        self.assertIsInstance(FRB.ResReplace("t", (0, "", "a")), FRB.BaseResEdit)

    # ================================================
    # =================== ResCreate ===================

    def test_resCreate_collectResourceName_usesTheNewNameForBothHalves(self):
        self.assertEqual(FRB.ResCreate("t", (0, "", "a")).collectResourceName("old", "new"), ("new", "new"))

    def test_resCreate_buildSection_isANoOpByDefault(self):
        self.assertIsNone(FRB.ResCreate("t", (0, "", "a")).buildSection("Whatever", None, "rika"))

    def test_resCreate_buildsItsSectionsFromScratch(self):
        class MyCreate(FRB.ResCreate):
            def buildSection(self, sectionName, modType, modName = ""):
                return FRB.IfTemplate([FRB.IfContentPart({"filename": [(0, sectionName + ".dds")]}, 0)],
                                      name = sectionName)

        # No sections in the .ini file at all -- the graph is built entirely out of buildSection
        ini = self._ini()
        graphGroups = self._groups()

        MyCreate("resourceRemapTexAdd", (0, "", "remapTex")).buildResources({"Made": "Made"}, None, ini, graphGroups,
                                                                             "rika")

        graph = graphGroups[0].graphs[("", "remapTex")]
        self.compareList(sorted(graph.sections.keys()), ["Made"])
        self.compareList(graph.getSection("Made").parts[0].entries(), [("filename", "Made.dds")])
        self.assertEqual(len(ini.resources), 1)

    def test_resCreate_buildSectionReturningNone_skipsThatSection(self):
        class MyCreate(FRB.ResCreate):
            def buildSection(self, sectionName, modType, modName = ""):
                return None

        graphGroups = self._groups()
        MyCreate("t", (0, "", "remapTex")).buildResources({"Made": "Made"}, None, self._ini(), graphGroups, "rika")

        graph = graphGroups[0].graphs[("", "remapTex")]
        self.assertEqual(len(graph.sections), 0)

    def test_resCreate_isABaseResEdit(self):
        self.assertIsInstance(FRB.ResCreate("t", (0, "", "a")), FRB.BaseResEdit)

    # ================================================
    # ============== RemapBlendReplace ================

    def test_remapBlendReplace_defaults(self):
        edit = FRB.RemapBlendReplace((0, "", "remapBlend"))

        self.assertEqual(edit.resType, "resourceRemapBlend")
        self.assertIsNone(edit.fixFunc)
        self.assertIsNone(edit.resSubType)
        self.assertIsNone(edit.fromComp)
        self.assertIsNone(edit.toComp)

    def test_remapBlendReplace_keepsTheExactFixFunc(self):
        fixFunc = lambda resource: True
        self.assertIs(FRB.RemapBlendReplace((0, "", "remapBlend"), fixFunc = fixFunc).fixFunc, fixFunc)

    def test_remapBlendReplace_namesUseTheBlendConventions(self):
        edit = FRB.RemapBlendReplace((0, "", "remapBlend"))

        self.assertEqual(edit.getFixResourceName("ResourceRaidenBlend", None, "rika"),
                         FRB.IniNamingTools.getRemapBlendResourceName("ResourceRaidenBlend", modName = "Rika"))
        self.assertEqual(edit.getFixFile("RaidenBlend.buf", None, "rika"),
                         FRB.IniNamingTools.getFixedBlendFile("RaidenBlend.buf", modName = "Rika"))

    def test_remapBlendReplace_resSubType_isFoldedIntoEveryName(self):
        edit = FRB.RemapBlendReplace((0, "", "remapBlend"), resSubType = "Semi")

        self.assertEqual(edit.getFixResourceName("ResourceRaidenBlend", None, "rika"),
                         FRB.IniNamingTools.getRemapBlendResourceName("ResourceRaidenBlend", modName = "RikaSemi"))
        self.assertEqual(edit.getFixFile("RaidenBlend.buf", None, "rika"),
                         FRB.IniNamingTools.getFixedBlendFile("RaidenBlend.buf", modName = "RikaSemi"))

    def test_remapBlendReplace_graphId_isAppendedToTheFixedFile(self):
        edit = FRB.RemapBlendReplace((0, "", "remapBlend"))
        withoutId = edit.getFixFile("RaidenBlend.buf", None, "rika")

        self.assertEqual(edit.getFixFile("RaidenBlend.buf", None, "rika", "GID"),
                         FRB.BaseResEdit.fileAddGraphId(withoutId, "GID"))

    def test_remapBlendReplace_isABaseResEdit(self):
        self.assertIsInstance(FRB.RemapBlendReplace((0, "", "a")), FRB.BaseResEdit)

    # ================================================
    # =================== TexCreate ===================

    def test_texCreate_defaults(self):
        edit = FRB.TexCreate((0, "", "remapTex"), "NormalMap", FRB.TexCreator(512, 512))

        self.assertEqual(edit.resType, "resourceRemapTexAdd")
        self.assertEqual(edit.texName, "NormalMap")
        self.assertIsNone(edit.fixFunc)

    def test_texCreate_keepsTheExactTexCreatorAndFixFunc(self):
        texCreator = FRB.TexCreator(512, 512)
        fixFunc = lambda resource: True
        edit = FRB.TexCreate((0, "", "remapTex"), "NormalMap", texCreator, fixFunc = fixFunc)

        self.assertIs(edit.texCreator, texCreator)
        self.assertIs(edit.fixFunc, fixFunc)

    def test_texCreate_numbersSuccessiveTexturesApart(self):
        edit = FRB.TexCreate((0, "", "remapTex"), "NormalMap", FRB.TexCreator(512, 512))

        first = edit.getFixResourceName("ignored", None, "rika")
        second = edit.getFixResourceName("ignored", None, "rika")
        third = edit.getFixResourceName("ignored", None, "rika")

        self.assertNotEqual(first, second)
        self.assertNotEqual(second, third)
        self.assertEqual(first, FRB.IniNamingTools.getRemapTexResourceName("RikaNormalMap"))
        self.assertEqual(second, FRB.IniNamingTools.getRemapTexResourceName("RikaNormalMap1"))

    def test_texCreate_clearResetsTheCounter(self):
        edit = FRB.TexCreate((0, "", "remapTex"), "NormalMap", FRB.TexCreator(512, 512))

        first = edit.getFixResourceName("ignored", None, "rika")
        edit.getFixResourceName("ignored", None, "rika")
        edit.clear()

        self.assertEqual(edit.getFixResourceName("ignored", None, "rika"), first)

    def test_texCreate_buildSection_referencesTheDdsFile(self):
        edit = FRB.TexCreate((0, "", "remapTex"), "NormalMap", FRB.TexCreator(512, 512))
        section = edit.buildSection("ResourceRikaNormalMapRemapTex", None, "rika")

        self.assertIsInstance(section, FRB.IfTemplate)
        entries = section.parts[0].entries()
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0][0], "filename")
        self.assertTrue(entries[0][1].endswith(".dds"))

    def test_texCreate_buildsTheGraphAndATexAddResource(self):
        ini = self._ini()
        graphGroups = self._groups()

        FRB.TexCreate((0, "", "remapTex"), "NormalMap", FRB.TexCreator(512, 512)).buildResources(
            {"ResourceRikaNormalMapRemapTex": "ResourceRikaNormalMapRemapTex"}, None, ini, graphGroups, "rika")

        graph = graphGroups[0].graphs[("", "remapTex")]
        self.compareList(sorted(graph.sections.keys()), ["ResourceRikaNormalMapRemapTex"])
        self.assertEqual(len(ini.resources), 1)
        self.assertIsInstance(ini.resources[0], FRB.RemapTexAddResource)

    def test_texCreate_isABaseResEdit(self):
        self.assertIsInstance(FRB.TexCreate((0, "", "a"), "N", FRB.TexCreator(512, 512)), FRB.BaseResEdit)
