import sys
import json

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class SomeObj:
    def __init__(self, a, b):
        self.a = a
        self.b = b
        self._hidden = "should not be serialized"


class HashToolsTest(BaseUnitTest):

    def setUp(self):
        super().setUp()
        FRB.HashTools.clear()

    # =========== inherited from CppHashTools =========

    def test_isSubclassOfCppHashTools(self):
        self.assertTrue(issubclass(FRB.HashTools, FRB.CppHashTools))

    def test_getDeterministicHash_strAndBytes_matchCppHashTools(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            self.assertEqual(FRB.HashTools.getDeterministicHash(text), FRB.CppHashTools.getDeterministicHash(text))
            self.assertEqual(FRB.HashTools.getDeterministicHash(text.encode("utf-8")), FRB.CppHashTools.getDeterministicHash(text.encode("utf-8")))

    def test_getDeterministicHashStr_strAndBytes_matchCppHashTools(self):
        tests = ["", "hello world", "Raiden Shogun"]
        for text in tests:
            self.assertEqual(FRB.HashTools.getDeterministicHashStr(text), FRB.CppHashTools.getDeterministicHashStr(text))

    def test_getShortDeterministicHashStr_strAndBytes_matchCppHashTools(self):
        # HashTools/CppHashTools share the same underlying collision-frequency state, so clear()
        # between the two calls -- otherwise the first call bumps the frequency the second call
        # then sees, and they'd never match
        text = "Raiden Shogun"

        FRB.HashTools.clear()
        result1 = FRB.HashTools.getShortDeterministicHashStr(text)

        FRB.HashTools.clear()
        result2 = FRB.CppHashTools.getShortDeterministicHashStr(text)

        self.assertEqual(result1, result2)

    def test_clear_isInheritedFromCppHashTools(self):
        text = "Raiden Shogun"
        first = FRB.HashTools.getShortDeterministicHashStr(text)
        second = FRB.HashTools.getShortDeterministicHashStr(text)
        self.assertNotEqual(first, second)

        FRB.HashTools.clear()
        third = FRB.HashTools.getShortDeterministicHashStr(text)
        self.assertEqual(third, first)

    # ================================================
    # ============= hashLibSerialize ==================

    def test_hashLibSerialize_primitives_passThrough(self):
        tests = [1, 1.5, "abc", True, False, None]
        for value in tests:
            self.assertEqual(json.loads(FRB.HashTools.hashLibSerialize(value)), value)

    def test_hashLibSerialize_listAndTuple_elementWise(self):
        self.assertEqual(json.loads(FRB.HashTools.hashLibSerialize([1, "a", None])), [1, "a", None])
        self.assertEqual(json.loads(FRB.HashTools.hashLibSerialize((1, "a", None))), [1, "a", None])

    def test_hashLibSerialize_dict_sortedByKey(self):
        result = FRB.HashTools.hashLibSerialize({"z": 1, "a": 2})
        self.assertEqual(result, FRB.HashTools.hashLibSerialize({"a": 2, "z": 1}))
        self.assertEqual(json.loads(result), {"a": 2, "z": 1})

    def test_hashLibSerialize_customObj_usesDictExcludingPrivateAttrs(self):
        result = json.loads(FRB.HashTools.hashLibSerialize(SomeObj(1, [1, 2])))
        self.compareDict(result, {"a": 1, "b": [1, 2]})

    def test_hashLibSerialize_isDeterministic(self):
        obj = SomeObj(1, "x")
        self.assertEqual(FRB.HashTools.hashLibSerialize(obj), FRB.HashTools.hashLibSerialize(SomeObj(1, "x")))

    # ================================================
    # ===== getDeterministicHash(obj)/etc wrapper ======

    def test_getDeterministicHash_arbitraryObj_matchesSerializedBytes(self):
        obj = SomeObj(1, [1, 2, 3])
        expected = FRB.CppHashTools.getDeterministicHash(FRB.HashTools.hashLibSerialize(obj))
        self.assertEqual(FRB.HashTools.getDeterministicHash(obj), expected)

    def test_getDeterministicHash_sameShapeDifferentInstances_sameHash(self):
        self.assertEqual(FRB.HashTools.getDeterministicHash(SomeObj(1, "x")), FRB.HashTools.getDeterministicHash(SomeObj(1, "x")))

    def test_getDeterministicHash_dictKeyOrderIndependent(self):
        self.assertEqual(FRB.HashTools.getDeterministicHash({"z": 1, "a": 2}), FRB.HashTools.getDeterministicHash({"a": 2, "z": 1}))

    def test_getDeterministicHashStr_arbitraryObj_matchesToBase64(self):
        obj = {"a": 1, "b": 2}
        self.assertEqual(FRB.HashTools.getDeterministicHashStr(obj), FRB.HashTools.getDeterministicHash(obj).toBase64())

    def test_getShortDeterministicHashStr_arbitraryObj_collisionSuffixBehaviour(self):
        obj = {"a": 1, "b": 2}
        first = FRB.HashTools.getShortDeterministicHashStr(obj)
        second = FRB.HashTools.getShortDeterministicHashStr(obj)
        self.assertEqual(second, f"{first}_{FRB.IntTools.toBase64(1)}")

    # ================================================
