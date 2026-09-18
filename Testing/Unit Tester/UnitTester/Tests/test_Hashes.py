import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class HashesTest(BaseUnitTest):
    """
    Tests the real, production ``Hashes`` class (built from the real ``HashData``, now backed by
    the C++ core's ``ModDictAssets``/``ModMappedAssets`` -- see model/assets/Hashes.py). The core
    algorithm itself already has extensive standalone C++ regression coverage
    (core/tests/ModDictAssets_test.cpp, core/tests/ModMappedAssets_test.cpp); this file is about
    the Python-side wrapper's own contract (property access, _convertNonVersionVals, and the real
    data actually round-tripping through the pybind boundary correctly) -- deliberately avoids
    hardcoding assumptions about which specific character owns which hash at which version (real
    data shifts over time); test values are derived from the live Hashes instance itself instead.
    """

    # =========================== fromAssets =====================================

    def test_fromAssets_isNonEmptyListNotMethod(self):
        hashes = FRB.Hashes()

        # A real historical gap caught during development: this must be a *property*
        # (IniFile.py does `type.hashes.fromAssets`, not `.fromAssets()`).
        fromAssets = hashes.fromAssets
        self.assertIsInstance(fromAssets, list)
        self.assertGreater(len(fromAssets), 100)

    # =========================== fixFrom / fixTo =====================================

    def test_fixFromFixTo_alwaysEmpty(self):
        # The pure-Python original declares these but never populates either anywhere --
        # matching that (seemingly unfinished, but real) contract, not adding new behavior.
        hashes = FRB.Hashes()
        self.compareSet(hashes.fixFrom, set())
        self.compareSet(hashes.fixTo, set())

    # =========================== hasFrom / getKey =====================================

    def test_hasFrom_and_getKey_onSharedHash(self):
        hashes = FRB.Hashes()

        # "b0e08915" is a real hash shared across dozens of characters in HashData.py (e.g. as
        # tex_body_metalmap/tex_head_metalmap) -- the exact shape that corrupted the reverse
        # index in the live pure-Python ModMappedAssets during development.
        self.assertTrue(hashes.hasFrom("b0e08915"))

        # Don't hardcode which character owns it at the latest version (that shifts as real data
        # changes) -- just confirm a real, well-shaped key comes back.
        key = hashes.getKey("b0e08915", None, None)
        self.assertIsInstance(key, tuple)
        self.assertEqual(len(key), 2)  # Hashes' non-version indices: (name, type)

        # Now confirm that SAME name resolves consistently when used as an explicit filter.
        ownerName = key[0]
        filteredKey = hashes.getKey("b0e08915", None, [ownerName, None])
        self.assertEqual(filteredKey, key)

        self.assertFalse(hashes.hasFrom("this-hash-does-not-exist-anywhere"))

    # =========================== replace =====================================

    def test_replace_realCharacterMap(self):
        # Amber -> AmberCN is a real mapping this project ships (GIBuilder.py). Pin an explicit
        # version (rather than None/"latest") for both the lookup and the getKey/replace calls --
        # "latest version this exact hash VALUE occurs at, across every character" (what
        # fromVersion=None means to getKey) isn't the same "latest" as "Amber's own latest entry"
        # (what version=None means to a plain get()) whenever some other character's data extends
        # further than Amber's -- pinning one shared, known-real version keeps both queries
        # unambiguously about the same thing.
        hashes = FRB.Hashes(map = {"Amber": ["AmberCN"]})
        amberHash = hashes.get(["Amber", "tex_body_metalmap"], version = "4.0")
        self.assertIsNotNone(amberHash)

        replaced = hashes.replace(amberHash, fromVersion = "4.0", fromNonVersionVals = ["Amber", None], toVersion = "4.0", toAssetName = "AmberCN")
        self.assertIsNotNone(replaced)

        amberCNHash = hashes.get(["AmberCN", "tex_body_metalmap"], version = "4.0")
        self.assertEqual(replaced, amberCNHash)

    # =========================== _convertNonVersionVals =====================================

    def test_convertNonVersionVals_allInputShapes(self):
        hashes = FRB.Hashes()

        self.assertEqual(hashes._convertNonVersionVals(None), [None, None])
        self.assertEqual(hashes._convertNonVersionVals(["Amber"]), ["Amber", None])
        self.assertEqual(hashes._convertNonVersionVals(["Amber", "tex_body_metalmap"]), ["Amber", "tex_body_metalmap"])
        self.assertEqual(hashes._convertNonVersionVals({"type": "tex_body_metalmap"}), [None, "tex_body_metalmap"])
        self.assertEqual(hashes._convertNonVersionVals("Amber"), ["Amber", None])
