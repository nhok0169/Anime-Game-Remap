import sys

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RemapIniRemoverTest(BaseIniFileTest):
    """
    Tests for :class:`RemapIniRemover`

    .. note::
        This is the **C++** ``RemapIniRemover`` (``FixRaidenBoss2.core.RemapIniRemover``), which
        replaced the pure-Python ``IniRemover`` -- since deleted, and since renamed. The two find a fix by genuinely
        different rules, so these are black-box tests of the new behaviour rather than a port of the
        old file's expectations:

        * a `section`_ inside the fix boilerplate is this software's own output on the strength of
          that marker alone, whatever it is named and whether or not it carries a ``hash``
        * a ``Remap``-named `section`_ **outside** the boilerplate needs a ``hash`` belonging to one
          of the .ini file's mod types before it counts -- there is no marker out there to go on
        * whatever is found drags along everything it references and everything that references it
        * ...unless the caller hands in an :class:`IniRemovalContext` with ``ignoreModType``, which
          drops the hash question and takes every candidate -- what the pure-Python original always
          did, and what :class:`IniFile` asks for

        The old file drove ``_removeScriptFix``/``_removeFixSections`` directly; those are internals
        of a class that no longer exists, so everything here goes through :meth:`RemapIniRemover.remove`
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._remover = None

    def createRemover(self):
        self._remover = FRB.RemapIniRemover(self._iniFile)

    def create(self):
        self.createIniFile()
        self.createRemover()
        self._iniFile._iniRemover = self._remover

    def removeFrom(self, iniTxt: str, context = None) -> str:
        """
        Runs a removal over 'iniTxt' and hands back the .ini file's new content

        .. note::
            ``getIfTemplates`` first, because that is the real caller's order (:meth:`Mod.removeFix`
            parses before removing) and because the resource collection needs the parsed `sections`_.
            The `section`_ removal itself does not -- being inside the boilerplate makes a `section`_
            a target whether or not it was parsed -- which is why the other cases here would pass
            either way
        """

        self._iniFile.fileTxt = iniTxt
        self._iniFile.getIfTemplates()
        return self._remover.remove(writeBack = False, context = context)

    # ====================== remove ======================================

    def test_noFix_textUnchanged(self):
        self.create()
        self.assertEqual(self.removeFrom("Hello"), "Hello")

    def test_closedBoilerPlate_wholeRegionRemoved(self):
        self.create()

        iniTxt = "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -----------------------------------------------"
        self.assertEqual(self.removeFrom(iniTxt), "")

    def test_boilerPlateCloseTooShort_regionKept(self):
        # The pure-Python original's pattern needed BOTH halves to match, and this keeps that: a
        # heading whose closing rule is shorter than the title demands (37 sideChars here, where
        # "Raiden Boss Fix" requires 45) never opens a region at all, so the block survives rather
        # than the removal eating the rest of the file.
        self.create()

        iniTxt = "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -------------------------------------"
        self.assertEqual(self.removeFrom(iniTxt), iniTxt)

    def test_sectionsInBoilerPlate_removedWithoutAnyHash(self):
        # No `hash` KVP anywhere -- which is normal for GIMI, where a TextureOverride can be matched
        # by its name alone. The boilerplate marker is what makes these removable.
        self.create()

        iniTxt = ("[TextureOverrideFooBlend]\n"
                  "vb1 = ResourceFooBlend\n"
                  "\n"
                  "[ResourceFooBlend]\n"
                  "filename = FooBlend.buf\n"
                  "\n"
                  "; --------------- Raiden Remap ---------------\n"
                  "\n"
                  "[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "\n"
                  "[ResourceFooRemapBlend]\n"
                  "filename = FooRemapBlend.buf\n"
                  "\n"
                  "; --------------------------------------------")

        result = self.removeFrom(iniTxt)

        self.compareSet(set(self._remover.getRemovedSectionNames()),
                        {"TextureOverrideFooRemapBlend", "ResourceFooRemapBlend"})
        self.assertNotIn("Remap", result)
        self.assertIn("[TextureOverrideFooBlend]", result)
        self.assertIn("[ResourceFooBlend]", result)

    def test_resourceReachedOnlyByVb1_removedAndCollected(self):
        # IniSectionGraph's edges come from `run =` alone, but a real fix points at its Resource
        # sections with `vb1 =`/`ib =`/`ps-t0 =`. The removal closes over the wider reference
        # relation, so those sections go too -- and their files are collected.
        self.create()

        iniTxt = ("; --------------- Raiden Remap ---------------\n"
                  "\n"
                  "[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "ps-t0 = ResourceFooRemapTex\n"
                  "\n"
                  "[ResourceFooRemapBlend]\n"
                  "filename = FooRemapBlend.buf\n"
                  "\n"
                  "[ResourceFooRemapTex]\n"
                  "filename = FooRemapTex.dds\n"
                  "\n"
                  "; --------------------------------------------")

        self.removeFrom(iniTxt)

        self.compareSet(set(self._remover.getRemovedSectionNames()),
                        {"TextureOverrideFooRemapBlend", "ResourceFooRemapBlend", "ResourceFooRemapTex"})

        removedResources = self._remover.getRemovedResources()
        self.compareSet(set(removedResources), {"blend", "texEdit"})
        self.assertEqual(len(removedResources["blend"]), 1)
        self.assertTrue(removedResources["blend"][0].srcPath.endswith("FooRemapBlend.buf"))
        self.assertEqual(len(removedResources["texEdit"]), 1)
        self.assertTrue(removedResources["texEdit"][0].srcPath.endswith("FooRemapTex.dds"))

    def test_remapNamedSectionOutsideBoilerPlateWithNoHash_kept(self):
        # The documented divergence from the pure-Python original, pinned on purpose. That one
        # matched `.*Remap(Blend|Position|Fix|Tex).*` by name wherever it appeared; this one needs
        # either the boilerplate marker or a mod type's hash, and an unclassified .ini file with no
        # boilerplate left offers neither.
        self.create()

        iniTxt = ("[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "\n"
                  "[ResourceFooRemapBlend]\n"
                  "filename = FooRemapBlend.buf")

        self.assertEqual(self.removeFrom(iniTxt), iniTxt)
        self.assertEqual(self._remover.getRemovedSectionNames(), [])

    def test_ignoreModType_remapNamedSectionOutsideBoilerPlateRemoved(self):
        # The exact input the test above keeps, with the flag on and nothing else changed.
        self.create()

        iniTxt = ("[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "\n"
                  "[ResourceFooRemapBlend]\n"
                  "filename = FooRemapBlend.buf")

        result = self.removeFrom(iniTxt, context = FRB.IniRemovalContext(ignoreModType = True))

        self.assertEqual(result, "")
        self.compareSet(set(self._remover.getRemovedSectionNames()),
                        {"TextureOverrideFooRemapBlend", "ResourceFooRemapBlend"})

        # And the resources still come out, classified the same way.
        removedResources = self._remover.getRemovedResources()
        self.compareSet(set(removedResources), {"blend"})
        self.assertTrue(removedResources["blend"][0].srcPath.endswith("FooRemapBlend.buf"))

    def test_ignoreModType_nonRemapSectionOutsideBoilerPlateStillKept(self):
        # The flag widens which candidates are targets, NOT what a candidate is: a section that is
        # neither inside the boilerplate nor Remap-named was never in the pool to begin with.
        self.create()

        iniTxt = ("[TextureOverrideFooBlend]\n"
                  "vb1 = ResourceFooBlend\n"
                  "\n"
                  "[ResourceFooBlend]\n"
                  "filename = FooBlend.buf")

        result = self.removeFrom(iniTxt, context = FRB.IniRemovalContext(ignoreModType = True))

        self.assertEqual(result, iniTxt)
        self.assertEqual(self._remover.getRemovedSectionNames(), [])

    def test_mutatedContext_laterDefaultedCallUnaffected(self):
        # pybind's version of Python's mutable-default-argument bug: had `context` been bound as
        # `py::arg("context") = IniRemovalContext()`, that one object would be shared by every
        # defaulted call and flipping it here would silently turn the flag on for good.
        self.create()

        iniTxt = ("[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "\n"
                  "[ResourceFooRemapBlend]\n"
                  "filename = FooRemapBlend.buf")

        context = FRB.IniRemovalContext()
        self.assertEqual(self.removeFrom(iniTxt, context = context), iniTxt)

        context.ignoreModType = True
        self.assertEqual(self.removeFrom(iniTxt, context = context), "")

        # ...and the defaulted call is still strict.
        self.assertEqual(self.removeFrom(iniTxt), iniTxt)

    def test_iniFileRemoveFix_ignoresModType(self):
        # IniFile runs exactly one remover, which makes it the LAST one -- and the last one sweeps.
        # This is what keeps IniFile._removeFix doing what the pure-Python original always did.
        self.create()

        iniTxt = ("[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "\n"
                  "[ResourceFooRemapBlend]\n"
                  "filename = FooRemapBlend.buf")

        self._iniFile.fileTxt = iniTxt
        self.assertEqual(self._iniFile._removeFix(writeBack = False), "")

    def test_hideOriginalComments_prefixStripped(self):
        # A fix applied with hideOrig comments the ORIGINAL mod out with this prefix. Removing the
        # fix has to take the prefix with it, or the .ini file is left with neither the fix nor the
        # original switched on.
        self.create()

        hide = FRB.IniKeywords.HideOriginalComment.value
        iniTxt = (f"{hide}[TextureOverrideFooBlend]\n"
                  f"{hide}vb1 = ResourceFooBlend\n"
                  "\n"
                  "; --------------- Raiden Remap ---------------\n"
                  "\n"
                  "[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "\n"
                  "; --------------------------------------------")

        result = self.removeFrom(iniTxt)

        self.assertNotIn(hide, result)
        # Trailing newlines survive: the strip is leading-only, matching what the pure-Python
        # original's two-pass removal actually left behind.
        self.assertEqual(result, "[TextureOverrideFooBlend]\nvb1 = ResourceFooBlend\n\n")

    def test_afterRemoval_iniNoLongerMarkedFixed(self):
        self.create()

        self._iniFile._isFixed = True
        self.removeFrom("; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -----------------------------------------------")
        self.assertEqual(self._iniFile.isFixed, False)

    def test_iniFileAttribute_isTheFileGiven(self):
        self.create()
        self.assertIs(self._remover.iniFile, self._iniFile)
