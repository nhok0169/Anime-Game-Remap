import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class TextureRolesTest(BaseUnitTest):
    def test_add_keepsEachRoleOncePerFile(self):
        roles = FRB.TextureRoles()
        roles.add("a.dds", "bodyDiffuse", FRB.TextureRoles.DeclaredHash + "abc")
        roles.add("a.dds", "bodyDiffuse", "again")
        roles.add("a.dds", "dressDiffuse", "bound at ps-t0 of component1")

        result = {file: [(role.role, role.how) for role in found] for file, found in roles.rolesOf().items()}
        self.assertEqual(result, {"a.dds": [("bodyDiffuse", "its own hash abc"), ("dressDiffuse", "bound at ps-t0 of component1")]})

    def test_add_emptyFileOrRole_isIgnored(self):
        roles = FRB.TextureRoles()
        roles.add("", "bodyDiffuse", "how")
        roles.add("a.dds", "", "how")
        self.assertEqual(roles.rolesOf(), {})

    def test_resourcesOf_perIni(self):
        roles = FRB.TextureRoles()
        roles.addResource("x.ini", "ResourceA", "a.dds")
        roles.addResource("y.ini", "ResourceB", "b.dds")

        self.assertEqual(roles.resourcesOf("x.ini"), [("ResourceA", "a.dds")])
        self.assertEqual(roles.resourcesOf("z.ini"), [])


class IniParseDataTest(BaseUnitTest):
    def test_defaults_sayEverythingIsThere(self):
        data = FRB.IniParseData()
        self.assertTrue(data.hasRemappableContent())
        self.assertEqual(data.undrawnObjs(), set())
        self.assertFalse(data.isUndrawn(("Body", "C")))
        self.assertIsNone(data.textureRoles())

    def test_undrawn_recordedPerModObj(self):
        data = FRB.IniParseData()
        data.addUndrawn(("Body", "C"))

        self.assertTrue(data.isUndrawn(("Body", "C")))
        self.assertFalse(data.isUndrawn(("Body", "D")))
        self.assertEqual(data.undrawnObjs(), {("Body", "C")})

        data.clearUndrawn()
        self.assertEqual(data.undrawnObjs(), set())

    def test_defer_runsOnceOnTheFirstAsk(self):
        calls = []

        def fill(data):
            calls.append(1)
            roles = FRB.TextureRoles()
            roles.add("a.dds", "bodyDiffuse", "how")
            data.setTextureRoles(roles)
            data.reject("b.dds", "bodyMask", "is a flat bodyMask, which marks no regions")

        data = FRB.IniParseData()
        data.defer(fill)
        self.assertEqual(len(calls), 0)

        self.assertEqual([(r.file, r.role) for r in data.rejections()], [("b.dds", "bodyMask")])
        self.assertIn("a.dds", data.textureRoles().rolesOf())
        self.assertIsNone(data.roleFile("bodyDiffuse", ("", "component0")))
        self.assertEqual(len(calls), 1)

    def test_textureRoles_setOutright(self):
        roles = FRB.TextureRoles()
        roles.add("a.dds", "bodyDiffuse", "how")

        data = FRB.IniParseData()
        data.setTextureRoles(roles)
        self.assertIn("a.dds", data.textureRoles().rolesOf())

    def test_roleFile_perModObj(self):
        data = FRB.IniParseData()
        data.setRoleFile("bodyDiffuse", ("", "component0"), FRB.IniParseData.RoleFile("a.dds", "how"))
        data.setRoleCandidates("bodyDiffuse", [("a.dds", "how")])

        self.assertEqual(data.roleFile("bodyDiffuse", ("", "component0")).file, "a.dds")
        self.assertIsNone(data.roleFile("bodyDiffuse", ("", "component1")))
        self.assertEqual(data.roleCandidates("bodyDiffuse"), [("a.dds", "how")])
        self.assertEqual(data.roleCandidates("faceDiffuse"), [])

    def test_toggles_perResource(self):
        data = FRB.IniParseData()
        data.setThisToggle("resourcetexture7", FRB.IniParseData.TextureToggle("TextureOverrideTexture7", ["ResourceTexture7", "ResourceTexture7a"]))
        data.setRegisterToggle("resourcetexture3_3", FRB.IniParseData.TextureToggle("TextureOverrideComponent3 ps-t3", ["ResourceTexture3_3", "ResourceTexture3_3_0"]))
        data.setThisToggle("resourcehairblock", FRB.IniParseData.TextureToggle("TextureOverrideHair", ["ResourceHairBlock"], True))

        toggle = data.thisToggleOf("resourcetexture7")
        self.assertEqual((toggle.section, toggle.variants, toggle.single), ("TextureOverrideTexture7", ["ResourceTexture7", "ResourceTexture7a"], False))
        self.assertTrue(data.thisToggleOf("resourcehairblock").single)
        self.assertEqual(data.registerToggleOf("resourcetexture3_3").section, "TextureOverrideComponent3 ps-t3")
        self.assertIsNone(data.thisToggleOf("resourcetexture3_3"))
        self.assertIsNone(data.registerToggleOf("resourcetexture7"))

        data.clear()
        self.assertIsNone(data.thisToggleOf("resourcetexture7"))

    def test_toggle_graph(self):
        toggle = FRB.IniParseData.TextureToggle("TextureOverrideTexture7", ["ResourceTexture7"], True,
                                                (FRB.IniGraphModObjKeywords.Texture, "bodyDiffuse"))
        self.assertEqual(toggle.graph, ("texture", "bodyDiffuse"))
        self.assertEqual(FRB.IniParseData.TextureToggle("a", ["b"]).graph, ("", ""))

    def test_anatomy_perObject(self):
        data = FRB.IniParseData()
        obj = ("", "component3")
        data.addDrawRange(obj, (1200, 0))
        data.addDrawRange(obj, (300, 1200))
        data.addDrawPart(obj, 2)
        data.setLateBindings(obj, ["ps-t0", "ps-t1"])
        data.setResourceFile("ResourceTexcoordBuffer", r".\Meshes\a.buf")

        self.assertEqual(data.drawRanges(obj), [(1200, 0), (300, 1200)])
        self.assertEqual(data.drawRanges(("", "component4")), [])
        self.assertEqual(data.drawParts(obj), {2})
        self.assertEqual(data.lateBindings(obj), ["ps-t0", "ps-t1"])
        self.assertFalse(data.replacesTexturesByHash())
        self.assertEqual(data.resourceFile("ResourceTexCoordBuffer"), r".\Meshes\a.buf")
        self.assertIsNone(data.resourceFile("ResourceIndexBuffer"))
        self.assertEqual(data.resourceFiles(), {"resourcetexcoordbuffer": r".\Meshes\a.buf"})

        data.clear()
        self.assertEqual(data.drawRanges(obj), [])
        self.assertEqual(data.resourceFiles(), {})

    def test_clear_resetsTheDefaults(self):
        data = FRB.IniParseData()
        data.setRemappableContent(False)
        data.addUndrawn(("", "component1"))
        data.setTextureRoles(FRB.TextureRoles())
        data.reject("a.dds", "bodyMask", "why")

        data.clear()
        self.assertTrue(data.hasRemappableContent())
        self.assertEqual(data.undrawnObjs(), set())
        self.assertIsNone(data.textureRoles())
        self.assertEqual(data.rejections(), [])

    def test_parser_ownsOne_andItsClearResetsIt(self):
        parser = FRB.BaseIniParser()
        self.assertIsInstance(parser.parseData, FRB.IniParseData)

        parser.parseData.addUndrawn(("", "component1"))
        self.assertIs(parser.parseData.isUndrawn(("", "component1")), True)

        parser.clear()
        self.assertEqual(parser.parseData.undrawnObjs(), set())


class TextureRoleChooserTest(BaseUnitTest):
    def _chooser(self, flat = ()):
        chooser = FRB.TextureRoleChooser()
        chooser.folder = "mod"
        chooser.isFlat = lambda file: file in flat
        return chooser

    def test_choose_bindingOnTheObjectWins(self):
        chooser = self._chooser()
        chooser.candidates = {"bodyDiffuse": [("mod/a.dds", "by hash"), ("mod/b.dds", "by hash")]}
        chooser.bindings = {"mod/b.dds": {("", "component1"): {"bodyDiffuse"}}}
        data = FRB.IniParseData()
        chooser.choose(data, [("", "component0"), ("", "component1")])

        self.assertEqual(data.roleFile("bodyDiffuse", ("", "component1")).file, "mod/b.dds")
        # bound only elsewhere ranks last, so component 0 takes the file nothing binds
        self.assertEqual(data.roleFile("bodyDiffuse", ("", "component0")).file, "mod/a.dds")

    def test_choose_declaredAsAnotherRoleComesLast(self):
        chooser = self._chooser()
        chooser.candidates = {"hairMask": [("mod/a.dds", "bound"), ("mod/b.dds", "bound")]}
        chooser.declared = {"mod/a.dds": {"hairNormal"}}
        data = FRB.IniParseData()
        chooser.choose(data, [("", "component0")])

        self.assertEqual(data.roleFile("hairMask", ("", "component0")).file, "mod/b.dds")

    def test_choose_flatRegionRoleIsRefused(self):
        chooser = self._chooser(flat = {"mod/a.dds"})
        chooser.regionRoles = {"bodyMask"}
        chooser.candidates = {"bodyMask": [("mod/a.dds", "by hash")], "bodyDiffuse": [("mod/a.dds", "by hash")]}
        data = FRB.IniParseData()
        chooser.choose(data, [("", "component0")])

        self.assertIsNone(data.roleFile("bodyMask", ("", "component0")))
        self.assertEqual(data.roleCandidates("bodyMask"), [])
        self.assertEqual([(r.file, r.role) for r in data.rejections()], [("mod/a.dds", "bodyMask")])
        # only a region role is refused: the same flat file still serves the diffuse
        self.assertEqual(data.roleFile("bodyDiffuse", ("", "component0")).file, "mod/a.dds")

    def test_choose_maskAliasedToTheSlotsNormalIsRefused(self):
        chooser = self._chooser()
        chooser.regionRoles = {"hairMask"}
        chooser.candidates = {"hairMask": [("mod/n.dds", "bound")], "hairNormal": [("mod/n.dds", "bound")]}
        chooser.bindings = {"mod/n.dds": {("", "component1"): {"hairMask", "hairNormal"}}}
        data = FRB.IniParseData()
        chooser.choose(data, [("", "component1")])

        self.assertIsNone(data.roleFile("hairMask", ("", "component1")))
        self.assertEqual(data.roleFile("hairNormal", ("", "component1")).file, "mod/n.dds")

    def test_choose_sameFileTwiceKeepsTheFirstWay(self):
        chooser = self._chooser()
        chooser.candidates = {"bodyDiffuse": [("mod/a.dds", "first"), ("mod/a.dds", "second")]}
        data = FRB.IniParseData()
        chooser.choose(data, [("", "component0")])

        chosen = data.roleFile("bodyDiffuse", ("", "component0"))
        self.assertEqual((chosen.file, chosen.how, chosen.tiedFile), ("mod/a.dds", "first", ""))

    def test_choose_equalFilesAreATie(self):
        chooser = self._chooser()
        chooser.candidates = {"bodyDiffuse": [("mod/ab.dds", "one"), ("mod/cd.dds", "two")]}
        data = FRB.IniParseData()
        chooser.choose(data, [("", "component0")])

        chosen = data.roleFile("bodyDiffuse", ("", "component0"))
        self.assertEqual((chosen.file, chosen.tiedFile), ("mod/ab.dds", "mod/cd.dds"))
