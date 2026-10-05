import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppMultiModFixerTest(BaseUnitTest):
    """
    Tests for :class:`MultiModFixer` -- the fixer that delegates to one child per mod type.

    The delegation/merging/ordering behaviour is pinned C++-side in
    ``core/tests/MultiModFixer_test.cpp``, where a recording child fixer can observe the
    :class:`IniFixingContext` each one is handed. What these cover is the *binding*: that the
    children dict crosses the boundary in both directions without losing object identity, that it
    nests, and that the filter is read through Python attribute lookup rather than through a C++
    ``IniFile`` pointer a Python-side fixer does not have.
    """

    # ================================================
    # ================== children ====================

    def test_noChildren_byDefault(self):
        self.compareDict(FRB.MultiModFixer().children, {})

    def test_children_roundTrip(self):
        first = FRB.BaseIniFixer()
        second = FRB.BaseIniFixer()

        fixer = FRB.MultiModFixer({10: first, 20: second})

        self.compareList(sorted(fixer.children.keys()), [10, 20])

    def test_children_preserveObjectIdentity(self):
        # The dict crosses as shared_ptr<BaseIniFixer>, which is NOT a registered pybind type --
        # only PyBaseIniFixer is -- so both directions convert entry by entry. Losing identity here
        # would mean a caller could never recognise its own child again.
        child = FRB.BaseIniFixer()
        fixer = FRB.MultiModFixer({7: child})

        self.assertIs(fixer.children[7], child)

    def test_children_settable(self):
        fixer = FRB.MultiModFixer()
        child = FRB.BaseIniFixer()

        fixer.children = {3: child}

        self.compareList(sorted(fixer.children.keys()), [3])
        self.assertIs(fixer.children[3], child)

    def test_defaultChildren_areNotShared(self):
        # py::arg("children") = <dict literal> would be pybind11's version of Python's
        # mutable-default-argument bug; the binding builds a fresh dict per call instead.
        first = FRB.MultiModFixer()
        second = FRB.MultiModFixer()

        first.children = {99: FRB.BaseIniFixer()}

        self.compareDict(second.children, {})

    def test_children_preserveInsertionOrder(self):
        # Children is a tsl::ordered_map, and a Python dict preserves insertion order too, so the
        # order the caller wrote survives the round-trip. It is not cosmetic: it decides which child
        # takes the .ini file's backup and which hides the original mod.
        ids = [30, 10, 20, 5]
        fixer = FRB.MultiModFixer({modTypeId: FRB.BaseIniFixer() for modTypeId in ids})

        self.compareList(list(fixer.children.keys()), ids)

    def test_children_setterPreservesInsertionOrder(self):
        fixer = FRB.MultiModFixer()
        ids = [7, 3, 9]

        fixer.children = {modTypeId: FRB.BaseIniFixer() for modTypeId in ids}

        self.compareList(list(fixer.children.keys()), ids)

    def test_noneChild_isKept(self):
        fixer = FRB.MultiModFixer({1: None})

        self.assertIsNone(fixer.children[1])

    # ================================================
    # ================== the type ====================

    def test_isABaseIniFixer(self):
        # What makes nesting possible at all.
        self.assertIsInstance(FRB.MultiModFixer(), FRB.BaseIniFixer)

    def test_nests(self):
        leaf = FRB.BaseIniFixer()
        inner = FRB.MultiModFixer({10: leaf})
        outer = FRB.MultiModFixer({1: inner, 2: FRB.BaseIniFixer()})

        self.compareList(sorted(outer.children.keys()), [1, 2])
        self.assertIs(outer.children[1], inner)
        self.assertIs(outer.children[1].children[10], leaf)

    def test_aGIMIFixerCanBeAChild(self):
        # Any BaseIniFixer subclass, not just another MultiModFixer.
        gimiFixer = FRB.GIMIFixer(None)
        fixer = FRB.MultiModFixer({5: gimiFixer})

        self.assertIs(fixer.children[5], gimiFixer)
