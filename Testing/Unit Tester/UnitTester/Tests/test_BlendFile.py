import struct
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makeBlendLine(weights, indices):
    return struct.pack("<4f", *weights) + struct.pack("<4i", *indices)


class BlendFileTest(BaseUnitTest):
    """
    Tests for :class:`BlendFile` -- the thin pure-Python subclass of :class:`CppBlendFile` (see
    test_CppBlendFile.py for tests of the inherited behaviour itself)
    """

    def test_isSubclassOfCppBlendFile(self):
        self.assertTrue(issubclass(FRB.BlendFile, FRB.CppBlendFile))
        self.assertTrue(issubclass(FRB.BlendFile, FRB.CppBufFile))

    def test_defaultElements_correctLayout(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line)

        self.assertEqual(blend.bytesPerLine, 32)
        self.assertEqual(blend.fileType, "Blend.buf")
        self.assertEqual([element.name for element in blend.elements], ["BLENDWEIGHT", "BLENDINDICES"])

    def test_constructionWithNoElementsArg_matchesModBlendCorrectionCallShape(self):
        # Mod.blendCorrection calls BlendFile(blendFile) with no 'elements' argument at all --
        # confirm that call shape actually works (CppBlendFile's own 'elements' defaults to None)
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line)
        self.assertEqual(blend.bytesPerLine, 32)

    def test_inheritedRemap_works(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line)

        result = blend.remap(FRB.VGRemap({5: 42}))
        indices = struct.unpack("<4i", bytes(result)[16:])
        self.assertEqual(indices[0], 42)

    def test_inheritedGetMissingIndicesRemap_works(self):
        result = FRB.BlendFile.getMissingIndicesRemap({"BLENDWEIGHT": [1.0], "BLENDINDICES": [99]}, FRB.VGRemap({5: 0}))
        self.assertEqual(result, {99: -100})

    def test_inheritedRemapIndices_works(self):
        result = FRB.BlendFile.remapIndices({"BLENDWEIGHT": [1.0], "BLENDINDICES": [5]}, FRB.VGRemap({5: 42}))
        self.assertEqual(result["BLENDINDICES"], [42])
