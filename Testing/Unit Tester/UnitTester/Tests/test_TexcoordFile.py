import math
import struct
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def _uvs(*pairs):
    """Texcoord bytes: one (U, V) pair of 16-bit halves a vertex"""
    return b"".join(struct.pack("<2e", u, v) for u, v in pairs)


class TexcoordFileTest(BaseUnitTest):
    """
    Tests for :class:`TexcoordFile` -- a ``Texcoord.buf``: its U values, the triangles straddling a U tile, and the
    clean that clears values that are not a number and folds U back into [0, 1)
    """

    def file(self, data):
        return FRB.TexcoordFile(data, FRB.TexcoordFile.halfElements(4))

    def test_us_readANaNAsZero(self):
        self.assertEqual(self.file(_uvs((0.5, 0.25), (math.nan, 0.0), (1.5, 1.0))).us(), [0.5, 0.0, 1.5])

    def test_halfElements_oneElementOfStrideOverTwoHalves(self):
        elements = FRB.TexcoordFile.halfElements(8)
        self.assertEqual(len(elements), 1)
        self.assertEqual(self.file(_uvs((0.5, 0.25))).uvElement(), FRB.TexcoordFile.UVKey)

    def test_straddling_marksTheCornersOfATriangleAcrossTwoTiles(self):
        texcoord = self.file(_uvs((0.9, 0.0), (1.1, 0.0), (1.2, 0.0), (1.3, 0.0), (1.4, 0.0), (1.5, 0.0)))
        # the first triangle crosses U = 1, the second stays in [1, 2), the third names a vertex that does not exist
        self.assertEqual(texcoord.straddling([0, 1, 2, 3, 4, 5, 0, 1, 9]), [True, True, True, False, False, False])

    def test_clean_clearsNaNs_foldsUAtOrAboveOne_leavesNegativeU(self):
        texcoord = self.file(_uvs((1.25, 0.5), (math.nan, math.nan), (-0.05, 0.5), (0.5, 0.5)))

        stats = texcoord.clean()

        self.assertEqual((stats.cleared, stats.folded), (2, 1))
        self.assertEqual(texcoord.us(), [0.25, 0.0, struct.unpack("<e", struct.pack("<e", -0.05))[0], 0.5])

    def test_clean_leavesAKeptVertexAlone(self):
        texcoord = self.file(_uvs((1.25, 0.5), (1.5, 0.5)))

        stats = texcoord.clean([True, False])

        self.assertEqual(stats.folded, 1)
        self.assertEqual(texcoord.us(), [1.25, 0.5])

    def test_clean_nothingToDo_changesNothing(self):
        data = _uvs((0.25, 0.5), (0.75, 0.125))
        texcoord = self.file(data)

        stats = texcoord.clean()

        self.assertEqual((stats.cleared, stats.folded), (0, 0))
        self.assertEqual(bytes(texcoord.data), data)
