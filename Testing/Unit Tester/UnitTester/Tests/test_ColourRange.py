import unittest
import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ColourRangeTest(BaseUnitTest):
    """
    Tests for :class:`ColourRange` (the thin pure-Python subclass of the pybind11-bound
    :class:`CppColourRange`) :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        Previously used ``MagicMock(spec=FRB.Colour)`` in place of real :class:`Colour` instances --
        that stopped working once :class:`Colour`/:class:`ColourRange` became pybind11-bound
        (a Mock has no underlying C++ storage for the constructor to cast into), the same class of
        breakage documented in this repo's Testing help doc for "private" method mocking. Rewritten
        to use real :class:`Colour` instances throughout
    """

    def setUp(self):
        super().setUp()
        self.min_colour = FRB.Colour(0, 0, 0, 0)
        self.max_colour = FRB.Colour(255, 255, 255, 255)
        self.colour_range = FRB.ColourRange(self.min_colour, self.max_colour)

    # ================================================
    # ============== __hash__ ========================

    def test_hash(self):
        expected_hash = hash("0000255255255255")
        self.assertEqual(hash(self.colour_range), expected_hash)

    # ================================================
    # ============== getId ==========================

    def test_getId(self):
        self.assertEqual(self.colour_range.getId(), "0000255255255255")

    # ================================================
    # ============== match ===========================

    def test_match_within_range(self):
        self.assertTrue(self.colour_range.match(FRB.Colour(128, 128, 128, 128)))

    def test_match_on_boundary(self):
        self.assertTrue(self.colour_range.match(FRB.Colour(255, 255, 255, 255)))
        self.assertTrue(self.colour_range.match(FRB.Colour(0, 0, 0, 0)))

    def test_match_outside_range(self):
        # Colour's own constructor bounds every channel to [0, 255], so an "out of range" colour
        # has to be expressed via a range that excludes part of that space, not an out-of-bounds
        # channel value (unlike the old MagicMock-based test, which could set red=300 directly).
        narrowRange = FRB.ColourRange(FRB.Colour(10, 10, 10, 10), FRB.Colour(20, 20, 20, 20))
        self.assertFalse(narrowRange.match(FRB.Colour(128, 15, 15, 15)))
        self.assertFalse(narrowRange.match(FRB.Colour(15, 15, 15, 5)))

    # ================================================
    # ============== min/max ==========================

    def test_minMax_areSettableProperties(self):
        newMin = FRB.Colour(1, 2, 3, 4)
        self.colour_range.min = newMin
        self.assertEqual(self.colour_range.min.getTuple(), (1, 2, 3, 4))

    # ================================================
