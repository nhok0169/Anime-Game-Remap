import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppColourReplaceTest(BaseUnitTest):
    """
    Tests for :class:`CppColourReplace` and its pass-through pure-Python subclass :class:`ColourReplace`
    """

    def test_noColoursToReplace_alwaysReplaces(self):
        pixel = FRB.Colour(1, 2, 3, 4)
        FRB.CppColourReplace(FRB.Colour(9, 9, 9, 9)).transform(pixel, 0, 0)
        self.assertEqual(pixel.getTuple(), (9, 9, 9, 9))

    def test_replaceAlphaFalse_preservesOriginalAlpha(self):
        pixel = FRB.Colour(1, 2, 3, 4)
        FRB.CppColourReplace(FRB.Colour(9, 9, 9, 9), replaceAlpha = False).transform(pixel, 0, 0)
        self.assertEqual(pixel.getTuple(), (9, 9, 9, 4))

    def test_coloursToReplace_exactColourMatch(self):
        pixel = FRB.Colour(5, 5, 5, 5)
        f = FRB.CppColourReplace(FRB.Colour(9, 9, 9, 9), coloursToReplace = {FRB.Colour(5, 5, 5, 5)})
        f.transform(pixel, 0, 0)
        self.assertEqual(pixel.getTuple(), (9, 9, 9, 9))

    def test_coloursToReplace_noMatch_leavesPixelUnchanged(self):
        pixel = FRB.Colour(5, 5, 5, 5)
        f = FRB.CppColourReplace(FRB.Colour(9, 9, 9, 9), coloursToReplace = {FRB.Colour(1, 1, 1, 1)})
        f.transform(pixel, 0, 0)
        self.assertEqual(pixel.getTuple(), (5, 5, 5, 5))

    def test_coloursToReplace_rangeMatch(self):
        pixel = FRB.Colour(50, 50, 50, 255)
        rng = FRB.ColourRange(FRB.Colour(0, 0, 0, 0), FRB.Colour(100, 100, 100, 255))
        f = FRB.CppColourReplace(FRB.Colour(9, 9, 9, 9), coloursToReplace = {rng})
        f.transform(pixel, 0, 0)
        self.assertEqual(pixel.getTuple(), (9, 9, 9, 9))

    def test_coloursToReplace_propertyRoundTrips(self):
        # ColourRange/Colour only define __hash__ (matching the pure-Python originals), not
        # __eq__, so compare the round-tripped set's single element by value (getId()) rather
        # than via set equality against the original object's identity.
        rng = FRB.ColourRange(FRB.Colour(0, 0, 0, 0), FRB.Colour(1, 1, 1, 1))
        f = FRB.CppColourReplace(FRB.Colour(), coloursToReplace = {rng})
        roundTripped = f.coloursToReplace
        self.assertEqual(len(roundTripped), 1)
        self.assertEqual(next(iter(roundTripped)).getId(), rng.getId())

        f.coloursToReplace = None
        self.assertIsNone(f.coloursToReplace)

    def test_bareSubclass_inheritsCleanly(self):
        self.assertTrue(issubclass(FRB.ColourReplace, FRB.CppColourReplace))
        pixel = FRB.Colour(1, 2, 3, 4)
        FRB.ColourReplace(FRB.Colour(9, 9, 9, 9)).transform(pixel, 0, 0)
        self.assertEqual(pixel.getTuple(), (9, 9, 9, 9))
