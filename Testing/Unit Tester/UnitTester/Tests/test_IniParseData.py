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

    def test_rolesOfResource_throughItsFile(self):
        data = FRB.IniParseData()
        data.setTextureResource("ResourceTexture7", "a.dds")
        data.setTextureResource("ResourceTexture8", "b.dds")
        data.setRoleFile("upperDiffuse", ("", "component4"), FRB.IniParseData.RoleFile("a.dds", "how"))
        data.setRoleFile("jacketDiffuse", ("", "component5"), FRB.IniParseData.RoleFile("a.dds", "how"))
        data.reject("b.dds", "upperMask", "is flat")

        roles = [(r.role, r.modObj) for r in data.rolesOfResource("resourcetexture7")]
        self.assertEqual(roles, [("jacketDiffuse", ("", "component5")), ("upperDiffuse", ("", "component4"))])
        self.assertEqual(data.rolesOfResource("ResourceTexture8"), [])
        self.assertEqual(data.rolesOfResource("ResourceNone"), [])
        self.assertEqual(data.refusedRoleOf("ResourceTexture8"), "upperMask")
        self.assertIsNone(data.refusedRoleOf("ResourceTexture7"))

        data.clear()
        self.assertEqual(data.rolesOfResource("ResourceTexture7"), [])

    def test_roleOfResource_ownFirst_thenAny_thenRefused(self):
        data = FRB.IniParseData()
        data.setTextureResource("ResourceTexture7", "a.dds")
        data.setTextureResource("ResourceTexture8", "b.dds")
        data.setRoleFile("upperDiffuse", ("", "component4"), FRB.IniParseData.RoleFile("a.dds", "how"))
        data.setRoleFile("jacketDiffuse", ("", "component5"), FRB.IniParseData.RoleFile("a.dds", "how"))
        data.reject("b.dds", "upperMask", "is flat")
        every = lambda chosen: True

        # the role on the line's own component wins; else the first another one takes
        self.assertEqual(data.roleOfResource("ResourceTexture7", ("", "component4"), every), "upperDiffuse")
        self.assertEqual(data.roleOfResource("ResourceTexture7", ("", "component5"), every), "jacketDiffuse")
        self.assertEqual(data.roleOfResource("ResourceTexture7", ("", "component9"), every), "jacketDiffuse")

        # only the pairs 'accepts' takes count
        notJacket = lambda chosen: chosen.role != "jacketDiffuse"
        self.assertEqual(data.roleOfResource("ResourceTexture7", ("", "component5"), notJacket), "upperDiffuse")

        # a file chosen for nothing it accepts: the role it was refused for, else none
        self.assertEqual(data.roleOfResource("ResourceTexture8", ("", "component4"), every), "upperMask")
        self.assertIsNone(data.roleOfResource("ResourceTexture7", ("", "component4"), lambda chosen: False))

    def test_libraries_perObject(self):
        data = FRB.IniParseData()
        obj = ("", "component3")
        self.assertEqual(data.libraries(obj), set())
        data.addLibrary(obj, "RabbitFX")
        data.addLibrary(obj, "RabbitFX")
        self.assertEqual(data.libraries(obj), {"RabbitFX"})
        self.assertEqual(data.libraries(("", "component4")), set())
        data.clear()
        self.assertEqual(data.libraries(obj), set())

    def test_blendVersion(self):
        data = FRB.IniParseData()
        self.assertIsNone(data.blendVersion())
        data.setBlendVersion("3.6")
        self.assertEqual(data.blendVersion(), "3.6")
        data.clear()
        self.assertIsNone(data.blendVersion())

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

    def test_rolesOfResource_aVariantHasItsSiblingsRole(self):
        data = FRB.IniParseData()
        data.setRoleFile("bodyDiffuse", ("", "component0"), FRB.IniParseData.RoleFile("mod/a.dds"))
        chosen = data.roleFile("bodyDiffuse", ("", "component0"))
        chosen.variants = ["mod/a.dds", "mod/b.dds"]
        data.setRoleFile("bodyDiffuse", ("", "component0"), chosen)
        data.setTextureResource("ResourceB", "mod/b.dds")

        self.assertEqual([(r.role, r.modObj) for r in data.rolesOfResource("ResourceB")], [("bodyDiffuse", ("", "component0"))])
