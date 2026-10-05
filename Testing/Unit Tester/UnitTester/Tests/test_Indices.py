import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IndicesTest(BaseUnitTest):
    """
    Tests the real, production ``Indices`` class (built from the real ``IndexData``, now backed
    by the C++ core's ``ModDictAssets``/``ModMappedAssets``) -- see test_Hashes.py's own
    docstring for why this is deliberately a thin wrapper-contract check, not a re-test of the
    core algorithm itself.
    """

    def test_fromAssets_isProperty(self):
        indices = FRB.Indices()
        fromAssets = indices.fromAssets
        self.assertIsInstance(fromAssets, list)
        self.assertGreater(len(fromAssets), 5)

    def test_fixFromFixTo_alwaysEmpty(self):
        indices = FRB.Indices()
        self.compareSet(indices.fixFrom, set())
        self.compareSet(indices.fixTo, set())

    def test_convertNonVersionVals_allInputShapes(self):
        indices = FRB.Indices()

        # Indices has 3 non-version indices: name, component, type.
        self.assertEqual(indices._convertNonVersionVals(None), [None, None, None])
        self.assertEqual(indices._convertNonVersionVals(["Amber"]), ["Amber", None, None])
        self.assertEqual(indices._convertNonVersionVals({"component": "body"}), [None, "body", None])
