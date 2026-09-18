import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppTexCreatorTest(BaseUnitTest):
    """
    Tests for :class:`CppTexCreator`.

    .. note::
        The Python-visible :class:`TexCreator` overrides ``fix`` entirely in Python (so the new
        texture goes through :class:`TextureFile`'s own Pillow-facing ``save``) -- see
        test_TexCreator.py for the real, exercised behaviour. This just confirms the C++ class's
        own constructor/properties
    """

    def test_defaultColour_isOpaqueWhite(self):
        creator = FRB.CppTexCreator(4, 8)
        self.assertEqual(creator.colour.getTuple(), (255, 255, 255, 255))

    def test_widthHeightColour_stored(self):
        creator = FRB.CppTexCreator(4, 8, FRB.Colour(1, 2, 3, 4))
        self.assertEqual((creator.width, creator.height), (4, 8))
        self.assertEqual(creator.colour.getTuple(), (1, 2, 3, 4))

    def test_widthHeightColour_settable(self):
        creator = FRB.CppTexCreator(4, 8)
        creator.width = 100
        creator.height = 200
        creator.colour = FRB.Colour(9, 9, 9, 9)
        self.assertEqual((creator.width, creator.height), (100, 200))
        self.assertEqual(creator.colour.getTuple(), (9, 9, 9, 9))

    def test_isSubclassOfBaseTexEditor(self):
        self.assertTrue(issubclass(FRB.CppTexCreator, FRB.CppBaseTexEditor))
