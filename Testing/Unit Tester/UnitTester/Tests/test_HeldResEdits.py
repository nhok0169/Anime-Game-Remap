import os
import sys
import tempfile
import shutil
import unittest
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class HeldResEditsTest(BaseIniFileTest):
    """The resource edits the C++ templates use, driven from Python through a :class:`ResRegCollect` over a real .ini"""

    _heldIniTxt = """
[TextureOverrideRaidenShogunBlend]
vb1 = ResourceRaidenShogunBlend
ps-cb4 = ResourceShaderCB4
ps-t0 = ResourceNormalMap
ps-t1 = ResourceOtherMap
this = ResourceRaidenDiffuseRemapDL
handling = skip
draw = 21916,0

[ResourceRaidenShogunBlend]
type = Buffer
stride = 32
filename = RaidenShogunBlend.buf"""

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None
        cls._fixer = None

    def create(self):
        self.createIniFile()
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("head", ""), ("body", "")]), trackKeys = False)
        self._fixer = FRB.GIMIFixer(self._parser, modsToFix = ["rika"])
        self.useStrategies(parser = self._parser, fixer = self._fixer)

    def _fix(self, *edits):
        self.create()
        self._fixer.graphGroupEdits = list(edits)
        self.writeIniTxt(self._heldIniTxt)
        self._iniFile.clear()
        self._iniFile.parse()
        return list(self._iniFile.fix().values())

    @classmethod
    def _fixBlock(cls, txt):
        return txt[txt.find("; --------------- Raiden Remap"):]

    # ====================== FileAddCreate ==============================

    def test_fileAddCreate_buildsTheNamedSection_andTheFile(self):
        file = FRB.FileAddCreate.File([("type", "Buffer"), ("stride", "4")], "ShaderCB4.buf", b"\x01\x02\x03\x04")
        create = FRB.FileAddCreate((0, "", "buffer"), {"ResourceShaderCB4": file})
        result = self._fix(FRB.ResRegCollect({(0, "", "blend"): "ps-cb4"}, {"buffer": create}))

        block = self._fixBlock(result[0])
        self.assertIn("ps-cb4 = ResourceShaderCB4\n", block)
        self.assertIn("[ResourceShaderCB4]\ntype = Buffer\nstride = 4\nfilename = ShaderCB4.buf\n", block)

        resources = self._iniFile.getResources()
        self.assertEqual(len(resources), 1)
        self.assertIsInstance(resources[0], FRB.RemapFileAddResource)

    def test_fileAddCreate_file_keepsWhatItIsGiven(self):
        made = lambda: b"\x09"
        file = FRB.FileAddCreate.File([("type", "Buffer")], "A.buf", b"\x01\x02", makeBytes = made)

        self.assertEqual(file.keys, [("type", "Buffer")])
        self.assertEqual(file.file, "A.buf")
        self.assertEqual(file.bytes, b"\x01\x02")
        self.assertEqual(file.makeBytes(), b"\x09")

        create = FRB.FileAddCreate((0, "", "buffer"), {"ResourceA": file})
        self.assertEqual(create.files["ResourceA"].file, "A.buf")
        self.assertIsNone(create.getFixResourceName("ResourceA"))

    # ====================== FileRefAdd =================================

    def test_fileRefAdd_buildsAResourceForEachListedFile_andNoSection(self):
        add = FRB.FileRefAdd({"RaidenShogunBlend.buf": b"\x05\x06"})
        result = self._fix(FRB.ResRegCollect({(0, "", "blend"): "ps-cb4"}, {"shader": add}),
                           FRB.ResRegCollect({(0, "", "blend"): "vb1"}, {"other": FRB.FileRefAdd({"Unlisted.buf": b"\x00"})}))

        self.assertEqual(add.files, {"RaidenShogunBlend.buf": b"\x05\x06"})
        self.assertNotIn("Unlisted", result[0])

    # ====================== FileDownloadCreate =========================

    def test_fileDownloadCreate_buildsTheNamedSection_andADownload(self):
        file = FRB.FileDownloadCreate.File([("type", "Texture2D")], "RaidenDiffuseRemapDL.dds", "https://example.invalid/a.dds")
        download = FRB.FileDownloadCreate((0, "", "download"), {"ResourceRaidenDiffuseRemapDL": file})
        result = self._fix(FRB.ResRegCollect({(0, "", "blend"): "this"}, {"download": download}))

        block = self._fixBlock(result[0])
        self.assertIn("[ResourceRaidenDiffuseRemapDL]\ntype = Texture2D\nfilename = RaidenDiffuseRemapDL.dds\n", block)

        resources = self._iniFile.getResources()
        self.assertEqual(len(resources), 1)
        self.assertIsInstance(resources[0], FRB.RemapIniDownload)

    def test_resRegCollect_extraResources_buildsWhatNoLineBinds(self):
        # a file the fix needs that nothing binds (a game texture a texture edit reads): without extraResources the
        # collect finds no line naming it and builds nothing
        file = FRB.FileDownloadCreate.File([], "RaidenLightMapGameRemapDL.dds", "https://example.invalid/b.dds")
        unbound = FRB.FileDownloadCreate((0, "", "download"), {"ResourceRaidenLightMapGameRemapDL": file})
        result = self._fix(FRB.ResRegCollect({(0, "", "blend"): "ps-t9"}, {"download": unbound}))
        self.assertNotIn("ResourceRaidenLightMapGameRemapDL", result[0])
        self.assertEqual(len(self._iniFile.getResources()), 0)

        download = FRB.FileDownloadCreate((0, "", "download"), {"ResourceRaidenLightMapGameRemapDL": file})
        collect = FRB.ResRegCollect({(0, "", "blend"): "ps-t9"}, {"download": download},
                                    extraResources = ["ResourceRaidenLightMapGameRemapDL"])
        result = self._fix(collect)

        self.assertEqual(collect.extraResources, ["ResourceRaidenLightMapGameRemapDL"])
        self.assertIn("[ResourceRaidenLightMapGameRemapDL]\nfilename = RaidenLightMapGameRemapDL.dds\n", self._fixBlock(result[0]))
        self.assertNotIn("= ResourceRaidenLightMapGameRemapDL", result[0])      # no line was rewritten to bind it
        resources = self._iniFile.getResources()
        self.assertEqual([type(resource).__name__ for resource in resources], ["RemapIniDownload"])

    def test_fileDownloadCreate_aTexEdit_editsTheDownload(self):
        editor = FRB.TexEditor([lambda tex: None])
        file = FRB.FileDownloadCreate.File([], "Edited.dds", "https://example.invalid/a.dds", texEdit = editor, rawFile = "RawRemapDL.dds")
        download = FRB.FileDownloadCreate((0, "", "download"), {"ResourceRaidenDiffuseRemapDL": file})
        self._fix(FRB.ResRegCollect({(0, "", "blend"): "this"}, {"download": download}))

        resources = self._iniFile.getResources()
        self.assertEqual([type(resource).__name__ for resource in resources], ["RemapIniDownload", "RemapTexEditResource"])

        # the Python editor itself, not a C++ copy of its base
        self.assertIs(resources[1].texEditor, editor)

    # ====================== TexCreate.files ============================

    def test_texCreate_files_keepTheRegistersName_andNameTheFile(self):
        create = FRB.TexCreate((0, "", "normal"), "NormalMap", FRB.TexCreator(4, 4))
        create.files = {"ResourceNormalMap": "Normal.dds"}
        result = self._fix(FRB.ResRegCollect({(0, "", "blend"): "ps-t0"}, {"normal": create}),
                           FRB.ResRegCollect({(0, "", "blend"): "ps-t1"}, {"other": create}))

        block = self._fixBlock(result[0])
        self.assertIn("ps-t0 = ResourceNormalMap\n", block)
        self.assertIn("[ResourceNormalMap]\nfilename = Normal.dds\n", block)
        self.assertNotIn("[ResourceOtherMap]", block)

    def test_texCreate_noFiles_renamesAsBefore(self):
        create = FRB.TexCreate((0, "", "normal"), "NormalMap", FRB.TexCreator(4, 4))
        result = self._fix(FRB.ResRegCollect({(0, "", "blend"): "ps-t0"}, {"normal": create}))

        block = self._fixBlock(result[0])
        self.assertNotIn("ps-t0 = ResourceNormalMap\n", block)
        self.assertIn("NormalMap", block)

    # ====================== VGRemapBlendReplace ========================

    def test_vgRemapBlendReplace_buildsARemapBlendResource_withTheRowsRemap(self):
        # the row looked up under 'rowModName' (Raiden -> RaidenBoss has one), each anchored chain on its root's target
        replace = FRB.VGRemapBlendReplace((0, "", "remapBlend"), self._raidenModType)
        replace.rowModName = "RaidenBoss"
        replace.anchorChains = {3: [4, 5]}
        self._fix(FRB.ResRegCollect({(0, "", "blend"): "vb1"}, {"blendType": replace}))

        resources = self._iniFile.getResources()
        self.assertEqual(len(resources), 1)
        self.assertIsInstance(resources[0], FRB.RemapBlendResource)
        self.assertEqual(resources[0].type, "blend")

        remap = resources[0].vgRemap.remap
        self.assertEqual(remap[4], remap[3])
        self.assertEqual(remap[5], remap[3])

    def test_vgRemapBlendReplace_noRow_writesAPlainCopy(self):
        replace = FRB.VGRemapBlendReplace((0, "", "remapBlend"), self._raidenModType)
        self._fix(FRB.ResRegCollect({(0, "", "blend"): "vb1"}, {"blendType": replace}))

        resources = self._iniFile.getResources()
        self.assertEqual(len(resources), 1)
        self.assertNotIsInstance(resources[0], FRB.RemapBlendResource)

    def test_vgRemapBlendReplace_keepsWhatItIsGiven(self):
        replace = FRB.VGRemapBlendReplace((0, "", "remapBlend"), self._raidenModType, toComp = "Body")
        fixFunc = lambda resource: True
        elements = lambda srcPath: FRB.WWMIBlend.blendElements(4)
        pairedIds = lambda srcPath: [FRB.VGRemapBlendReplace.PairedIds("ids.buf", 2)]

        replace.fixFunc = fixFunc
        replace.elements = elements
        replace.pairedIds = pairedIds
        replace.influences = 8
        replace.rowModName = "Skin"
        replace.anchorChains = {1: [2]}

        self.assertIs(replace.modType, self._raidenModType)
        self.assertIs(replace.fixFunc, fixFunc)
        self.assertIs(replace.elements, elements)
        self.assertIs(replace.pairedIds, pairedIds)
        self.assertEqual(replace.influences, 8)
        self.assertEqual(replace.rowModName, "Skin")
        self.assertEqual(replace.toComp, "Body")
        self.assertEqual(replace.anchorChains, {1: [2]})


class HeldTexEditorsTest(unittest.TestCase):
    """A texture editor is HELD by a resource, so a Python subclass's ``fix`` is the one that runs"""

    def setUp(self):
        self._folder = tempfile.mkdtemp(prefix = "AGRemapHeldEditors")
        self._src = os.path.join(self._folder, "src.dds")
        FRB.CppTexCreator(8, 8, compress = False).fix(FRB.CppTextureFile(self._src), self._src)

    def tearDown(self):
        shutil.rmtree(self._folder, ignore_errors = True)

    def test_aPythonTexEditor_isTheOneThatRuns(self):
        calls = []
        editor = FRB.TexEditor([lambda tex: calls.append(tex)], compress = False)
        fixed = os.path.join(self._folder, "fixed.dds")
        resource = FRB.RemapTexEditResource(self._folder, self._src, fixed, editor)

        self.assertIs(resource.texEditor, editor)
        resource.fix()

        self.assertEqual(len(calls), 1)
        self.assertTrue(os.path.isfile(fixed))

    def test_aPythonTexCreator_isTheOneThatRuns(self):
        calls = []

        class Recording(FRB.CppTexCreator):
            def fix(self, texFile, fixedTexFile):
                calls.append(fixedTexFile)

        made = os.path.join(self._folder, "made.dds")
        resource = FRB.RemapTexAddResource(self._folder, made, Recording(4, 4))
        resource.fix()

        self.assertEqual(calls, [made])

    def test_compressNotAllowed_leavesTheSharedEditorAsItWas(self):
        editor = FRB.TexEditor([lambda tex: None], compress = True)
        fixed = os.path.join(self._folder, "fixed.dds")
        resource = FRB.RemapTexEditResource(self._folder, self._src, fixed, editor)
        resource.compressAllowed = False

        seen = []
        editor.filters = [lambda tex: seen.append(editor.compress)]
        resource.fix()

        self.assertEqual(seen, [False])
        self.assertTrue(editor.compress)
