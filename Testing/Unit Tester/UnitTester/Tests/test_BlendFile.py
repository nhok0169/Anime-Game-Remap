import os
import struct
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _makeBlendLine(weights, indices):
    return struct.pack("<4f", *weights) + struct.pack("<4i", *indices)


class BlendFileTest(BaseUnitTest):
    """
    Tests for :class:`BlendFile` -- used for handling ``Blend.buf`` files. Bound directly as a
    real pybind11 class (previously a thin pure-Python subclass of a separate ``CppBlendFile`` --
    the two were merged into this one class, per the "outcome 1 collapsing into outcome 2" case in
    Architecture/CLAUDE.md, since the pure-Python side added no behaviour of its own)
    """

    # ================================================
    # ================= elements =====================

    def test_defaultElements_correctLayout(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line)

        self.assertEqual(blend.bytesPerLine, 32)
        self.assertEqual(blend.fileType, "Blend.buf")

        elementNames = [element.name for element in blend.elements]
        self.assertEqual(elementNames, ["BLENDWEIGHT", "BLENDINDICES"])

    def test_defaultElements_emptyListAlsoUsesDefault(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line, [])
        self.assertEqual(blend.bytesPerLine, 32)

    def test_customElements_overridesDefault(self):
        line = struct.pack("<3f", 1.0, 2.0, 3.0)
        blend = FRB.BlendFile(line, [FRB.BufElementType("POSITION", "R32G32B32_FLOAT", [FRB.BufFloat(), FRB.BufFloat(), FRB.BufFloat()])])
        self.assertEqual(blend.bytesPerLine, 12)

    def test_constructionWithNoElementsArg_matchesModBlendCorrectionCallShape(self):
        # Mod.blendCorrection calls BlendFile(blendFile) with no 'elements' argument at all --
        # confirm that call shape actually works (the constructor's own 'elements' defaults to None)
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line)
        self.assertEqual(blend.bytesPerLine, 32)

    def test_isInstanceOfBufFileAndBinaryFile(self):
        blend = FRB.BlendFile(_makeBlendLine([0.0, 0.0, 0.0, 0.0], [0, 0, 0, 0]))
        self.assertIsInstance(blend, FRB.CppBufFile)
        self.assertIsInstance(blend, FRB.BinaryFile)

    # ================================================
    # ========== getMissingIndicesRemap ==============

    def test_getMissingIndicesRemap_onlyMissingIndicesIncluded(self):
        src = {"BLENDWEIGHT": [1.0, 1.0, 1.0, 1.0], "BLENDINDICES": [5, 99, 5, 100]}
        vgRemap = FRB.VGRemap({5: 42})

        result = FRB.BlendFile.getMissingIndicesRemap(src, vgRemap)
        self.compareDict(result, {99: -100, 100: -101})

    def test_getMissingIndicesRemap_allIndicesKnown_emptyResult(self):
        src = {"BLENDWEIGHT": [1.0, 1.0], "BLENDINDICES": [5, 6]}
        vgRemap = FRB.VGRemap({5: 0, 6: 0})

        result = FRB.BlendFile.getMissingIndicesRemap(src, vgRemap)
        self.compareDict(result, {})

    # ================================================
    # =============== remapIndices ===================

    def test_remapIndices_remapsKnownIndicesWithNonZeroWeight(self):
        src = {"BLENDWEIGHT": [1.0, 0.5, 0.0, 0.0], "BLENDINDICES": [5, 7, 0, 0]}
        vgRemap = FRB.VGRemap({5: 42, 7: 43})

        result = FRB.BlendFile.remapIndices(src, vgRemap)
        self.assertEqual(result["BLENDINDICES"], [42, 43, 0, 0])

    def test_remapIndices_zeroWeightIndicesUnchanged(self):
        src = {"BLENDWEIGHT": [0.0, 1.0], "BLENDINDICES": [5, 5]}
        vgRemap = FRB.VGRemap({5: 42})

        result = FRB.BlendFile.remapIndices(src, vgRemap)
        self.assertEqual(result["BLENDINDICES"], [5, 42])

    def test_remapIndices_missingIndexDeactivated(self):
        src = {"BLENDWEIGHT": [1.0], "BLENDINDICES": [99]}
        vgRemap = FRB.VGRemap({5: 42})

        result = FRB.BlendFile.remapIndices(src, vgRemap, remapMissingIndices = True)
        self.assertEqual(result["BLENDINDICES"], [-100])

    def test_remapIndices_missingIndexNotDeactivatedWhenDisabled(self):
        src = {"BLENDWEIGHT": [1.0], "BLENDINDICES": [99]}
        vgRemap = FRB.VGRemap({5: 42})

        result = FRB.BlendFile.remapIndices(src, vgRemap, remapMissingIndices = False)
        self.assertEqual(result["BLENDINDICES"], [99])

    # ================================================
    # =================== remap ======================

    def test_remap_emptyRemap_bytesSrc_returnsUnchangedCopy(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line)

        result = blend.remap(FRB.VGRemap({}))
        self.assertIsInstance(result, bytearray)
        self.assertEqual(bytes(result), line)

    def test_remap_emptyRemap_fileSrc_returnsNone(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        with tempfile.NamedTemporaryFile(delete = False) as f:
            f.write(line)
            path = f.name

        try:
            blend = FRB.BlendFile(path)
            self.assertIsNone(blend.remap(FRB.VGRemap({})))
        finally:
            os.remove(path)

    def test_remap_nonEmptyRemap_remapsIndices(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line)

        result = blend.remap(FRB.VGRemap({5: 42}))
        self.assertIsInstance(result, bytearray)

        weights = struct.unpack("<4f", bytes(result)[:16])
        indices = struct.unpack("<4i", bytes(result)[16:])
        self.assertEqual(indices[0], 42)
        self.assertEqual(weights, (1.0, 0.0, 0.0, 0.0))

    def test_remap_withFixedBlendFile_writesFileAndReturnsPath(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [5, 0, 0, 0])
        blend = FRB.BlendFile(line)

        outPath = os.path.join(tempfile.gettempdir(), "test_BlendFile_fixOutput.buf")
        try:
            result = blend.remap(FRB.VGRemap({5: 42}), fixedBlendFile = outPath)
            self.assertEqual(result, outPath)

            with open(outPath, "rb") as f:
                written = f.read()
            self.assertEqual(struct.unpack("<4i", written[16:])[0], 42)
        finally:
            if (os.path.isfile(outPath)):
                os.remove(outPath)

    def test_remap_missingIndexDeactivated(self):
        line = _makeBlendLine([1.0, 0.0, 0.0, 0.0], [99, 0, 0, 0])
        blend = FRB.BlendFile(line)

        result = blend.remap(FRB.VGRemap({5: 42}))
        indices = struct.unpack("<4i", bytes(result)[16:])
        self.assertEqual(indices[0], -100)
