import os
import shutil
import struct
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


# A YelanTranquil mod of the Body component's slot C only: the real hashes (the compiled parser
# matches by hash, so a made-up one classifies as nothing and every assertion here would pass
# vacuously), tiny stand-in buffers and 4x4 uncompressed textures so the fix runs in milliseconds.
_INI = """; YelanTranquil

[TextureOverrideYelanTranquilBodyPosition]
hash = 02c325ef
vb0 = ResourceYelanTranquilBodyPosition

[TextureOverrideYelanTranquilBodyBlend]
hash = 244a4b2f
vb1 = ResourceYelanTranquilBodyBlend
handling = skip
draw = 4,0

[TextureOverrideYelanTranquilBodyTexcoord]
hash = c772811d
vb1 = ResourceYelanTranquilBodyTexcoord

[TextureOverrideYelanTranquilBodyIB]
hash = 611d6168
handling = skip
drawindexed = auto

[TextureOverrideYelanTranquilBodyC]
hash = 611d6168
match_first_index = 67374
ib = ResourceYelanTranquilBodyCIB
ps-t0 = ResourceYelanTranquilBodyCDiffuse
ps-t1 = ResourceYelanTranquilBodyCLightMap

[ResourceYelanTranquilBodyPosition]
type = Buffer
stride = 40
filename = YelanTranquilBodyPosition.buf

[ResourceYelanTranquilBodyBlend]
type = Buffer
stride = 32
filename = YelanTranquilBodyBlend.buf

[ResourceYelanTranquilBodyTexcoord]
type = Buffer
stride = 20
filename = YelanTranquilBodyTexcoord.buf

[ResourceYelanTranquilBodyCIB]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = YelanTranquilBodyC.ib

[ResourceYelanTranquilBodyCDiffuse]
filename = YelanTranquilBodyCDiffuse.dds

[ResourceYelanTranquilBodyCLightMap]
filename = YelanTranquilBodyCLightMap.dds
"""

_SIZE = 4
_VERTICES = 4
_ORIGINAL = bytes((1, 2, 3, 4))
_EDITED = bytes((12, 34, 56, 78))


class GIMIComponentBuildersTest(BaseUnitTest):
    """
    The Python face of :class:`GIMIMergeFixerConfig` -- the config a prototype hands to
    :func:`makeGIMIMergeFixer`.

    ``lightMapEdit`` was once a ``def_readwrite`` over a ``std::function``, and pybind11's own
    conversion hands the filter it returns a COPY of the texture: a light map edit written in Python
    ran, raised nothing, and the fix saved the unedited light map. Only reading the written texture
    can see that, so that is what the first test does.
    """

    def setUp(self):
        super().setUp()
        FRB.CppStrategyOverrides.clear()
        self.addCleanup(FRB.CppStrategyOverrides.clear)

        self._folder = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self._folder, True)

    # -------------------------------------------------------------------------------------
    # helpers
    # -------------------------------------------------------------------------------------
    def _path(self, name):
        return os.path.join(self._folder, name)

    def _writeTexture(self, name, rgba):
        texFile = FRB.CppTextureFile(self._path(name))
        texFile.setPixels(rgba * (_SIZE * _SIZE), _SIZE, _SIZE)
        texFile.save(compress = False)

    def _writeMod(self):
        with open(self._path("YelanTranquil.ini"), "w", encoding = "utf-8") as f:
            f.write(_INI)

        for name, stride in (("Position", 40), ("Blend", 32), ("Texcoord", 20)):
            with open(self._path(f"YelanTranquilBody{name}.buf"), "wb") as f:
                f.write(bytes(stride * _VERTICES))

        with open(self._path("YelanTranquilBodyC.ib"), "wb") as f:
            f.write(struct.pack("<6I", 0, 1, 2, 1, 2, 3))

        self._writeTexture("YelanTranquilBodyCDiffuse.dds", bytes((200, 100, 50, 255)))
        self._writeTexture("YelanTranquilBodyCLightMap.dds", _ORIGINAL)

    def _config(self, lightMapEdit):
        C = FRB.GIMIMergeFixerConfig

        slots = []
        for name, index, to, normalMap in (("A", "0", "body", True), ("B", "53631", "extra", True), ("C", "67374", "dress", False)):
            slot = C.Slot()
            slot.name = name
            slot.index = index
            slot.to = to
            slot.normalMap = normalMap
            slots.append(slot)

        body = C.Component()
        body.name = "Body"
        body.slots = slots
        body.vertexCount = _VERTICES

        config = C()
        config.components = [body]
        config.targetObjs = ["head", "body", "dress", "extra"]
        config.lightMapEdit = lightMapEdit
        config.compressTextures = False
        config.mipmaps = False
        return config

    # Fixes the mod with 'lightMapEdit' and returns the slot C light map resources the fix built,
    # after running them -- the texture edit itself happens in C++, inside fix().
    def _fixLightMap(self, lightMapEdit):
        self._writeMod()

        FRB.CppGlobalModTypes.registerAll()
        FRB.CppStrategyOverrides.setFixer("YelanTranquil", "Yelan", FRB.makeGIMIMergeFixer(self._config(lightMapEdit)))

        iniFile = FRB.IniFile(self._path("YelanTranquil.ini"), filteredToModTypeIds = {int(FRB.ModTypeId.Yelan)})
        iniFile.parse()
        iniFile.fix(keepBackup = False)

        resources = [resource for resource in iniFile.getResources()
                     if isinstance(resource, FRB.RemapTexEditResource) and os.path.basename(resource.srcPath) == "YelanTranquilBodyCLightMap.dds"]
        for resource in resources:
            self.assertTrue(resource.fix())

        return resources

    def _firstPixel(self, path):
        texFile = FRB.CppTextureFile(path)
        texFile.open()
        self.assertTrue(texFile.hasImage)
        return bytes(texFile.getPixels()[:4])

    # -------------------------------------------------------------------------------------
    # lightMapEdit
    # -------------------------------------------------------------------------------------
    def test_lightMapEdit_editReachesTheSavedTexture(self):
        calls = []

        def lightMapEdit(diffusePath):
            calls.append(os.path.basename(diffusePath))

            def edit(texFile):
                calls.append(type(texFile).__name__)
                texFile.setPixels(_EDITED * (_SIZE * _SIZE), _SIZE, _SIZE)

            return edit

        resources = self._fixLightMap(lightMapEdit)

        self.assertEqual(len(resources), 1)
        # Both levels really ran -- without this, an edit that never fired would fail the pixel
        # check for the wrong reason
        self.assertIn("YelanTranquilBodyCDiffuse.dds", calls)
        self.assertIn("CppTextureFile", calls)
        self.assertEqual(self._firstPixel(resources[0].fixedPath), _EDITED)

    def test_lightMapEdit_noOpEdit_writesTheOriginalPixels(self):
        # The control for the test above: the pixel it checks is the ORIGINAL unless the edit lands
        resources = self._fixLightMap(lambda diffusePath: (lambda texFile: None))

        self.assertEqual(len(resources), 1)
        self.assertEqual(self._firstPixel(resources[0].fixedPath), _ORIGINAL)

    def test_lightMapEdit_returningNone_editsNothing(self):
        self.assertEqual(self._fixLightMap(lambda diffusePath: None), [])

    def test_lightMapEdit_readsBackTheSameCallable(self):
        def lightMapEdit(diffusePath):
            return None

        config = FRB.GIMIMergeFixerConfig()
        self.assertIsNone(config.lightMapEdit)

        config.lightMapEdit = lightMapEdit
        self.assertIs(config.lightMapEdit, lightMapEdit)

        config.lightMapEdit = None
        self.assertIsNone(config.lightMapEdit)

    def test_lightMapEdit_rejectsANonCallable(self):
        config = FRB.GIMIMergeFixerConfig()
        with self.assertRaises(TypeError):
            config.lightMapEdit = "notCallable"
