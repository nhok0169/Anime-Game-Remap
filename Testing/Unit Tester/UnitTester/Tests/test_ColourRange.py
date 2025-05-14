import unittest
import sys
from .baseUnitTest import BaseUnitTest
from unittest.mock import MagicMock
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


##########
# Note:
#    This unit test is created using AI (Github Copilot).
##########
class ColourRangeTest(BaseUnitTest):
    def setUp(self):
        # Mock Colour objects for min and max
        self.min_colour = MagicMock(spec=FRB.Colour)
        self.min_colour.red = 0
        self.min_colour.green = 0
        self.min_colour.blue = 0
        self.min_colour.alpha = 0
        self.min_colour.getId.return_value = "0000"

        self.max_colour = MagicMock(spec=FRB.Colour)
        self.max_colour.red = 255
        self.max_colour.green = 255
        self.max_colour.blue = 255
        self.max_colour.alpha = 255
        self.max_colour.getId.return_value = "255255255255"

        self.colour_range = FRB.ColourRange(self.min_colour, self.max_colour)

    # ============== __hash__ ========================

    def test_hash(self):
        # Test the hash method
        expected_hash = hash("0000255255255255")
        self.assertEqual(hash(self.colour_range), expected_hash)

    # ================================================
    # ============== getId ==========================

    def test_getId(self):
        # Test the getId method
        expected_id = "0000255255255255"
        self.assertEqual(self.colour_range.getId(), expected_id)

    # ================================================
    # ============== match ===========================

    def test_match_within_range(self):
        # Test match method for a colour within the range
        test_colour = MagicMock(spec=FRB.Colour)
        test_colour.red = 128
        test_colour.green = 128
        test_colour.blue = 128
        test_colour.alpha = 128
        self.assertTrue(self.colour_range.match(test_colour))

    def test_match_outside_range(self):
        # Test match method for a colour outside the range
        test_colour = MagicMock(spec=FRB.Colour)
        test_colour.red = 300
        test_colour.green = 128
        test_colour.blue = 128
        test_colour.alpha = 128
        self.assertFalse(self.colour_range.match(test_colour))

    def test_match_on_boundary(self):
        # Test match method for a colour on the boundary
        test_colour = MagicMock(spec=FRB.Colour)
        test_colour.red = 255
        test_colour.green = 255
        test_colour.blue = 255
        test_colour.alpha = 255
        self.assertTrue(self.colour_range.match(test_colour))

    def test_match_below_min_boundary(self):
        # Test match method for a colour below the minimum boundary
        test_colour = MagicMock(spec=FRB.Colour)
        test_colour.red = -1
        test_colour.green = 0
        test_colour.blue = 0
        test_colour.alpha = 0
        self.assertFalse(self.colour_range.match(test_colour))

    def test_match_above_max_boundary(self):
        # Test match method for a colour above the maximum boundary
        test_colour = MagicMock(spec=FRB.Colour)
        test_colour.red = 256
        test_colour.green = 255
        test_colour.blue = 255
        test_colour.alpha = 255
        self.assertFalse(self.colour_range.match(test_colour))

    # ================================================