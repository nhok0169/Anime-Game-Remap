import sys

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GlobalRemapIniRemoverTest(BaseIniFileTest):
    """
    Tests for :class:`GlobalRemapIniRemover`

    .. note::
        The general use remover: a :class:`RemapIniRemover` that always behaves as though it were
        handed an :class:`IniRemovalContext` with ``ignoreModType`` set. It exists for the .ini file
        that belongs to a mod but was not attributed to any type of mod -- there is no mod type to
        ask whose a leftover `section`_ is, so the stricter rule could only ever recognize the fix
        boilerplate and would leave every ``Remap``-named leftover standing forever

        Everything about *how* the fix is found is :class:`RemapIniRemover`'s, and is covered by
        ``test_RemapIniRemover.py``. What is tested here is only the difference: that the sweep
        happens, that the caller cannot turn it off, and that widening which candidates are targets
        did not widen what a candidate **is**
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._remover = None

    def createRemover(self):
        self._remover = FRB.GlobalRemapIniRemover(self._iniFile)

    def create(self):
        self.createIniFile()
        self.createRemover()
        self._iniFile._iniRemover = self._remover

    def removeFrom(self, iniTxt: str, context = None) -> str:
        """
        Runs a removal over 'iniTxt' and hands back the .ini file's new content

        .. note::
            ``getIfTemplates`` first, for the same reason ``test_RemapIniRemover.py`` does it -- see
            that file's own note
        """

        self._iniFile.fileTxt = iniTxt
        self._iniFile.getIfTemplates()
        return self._remover.remove(writeBack = False, context = context)

    # ====================== remove ======================================

    def test_isARemapIniRemover(self):
        self.create()
        self.assertIsInstance(self._remover, FRB.RemapIniRemover)
        self.assertIsInstance(self._remover, FRB.BaseIniRemover)

    def test_iniFileAttribute_isTheFileGiven(self):
        self.create()
        self.assertIs(self._remover.iniFile, self._iniFile)

    def test_noFix_textUnchanged(self):
        self.create()
        self.assertEqual(self.removeFrom("Hello"), "Hello")

    def test_remapNamedSectionOutsideBoilerPlateWithNoHash_removed(self):
        # The exact input test_RemapIniRemover.test_remapNamedSectionOutsideBoilerPlateWithNoHash_kept
        # KEEPS: no boilerplate marker, no hash, no mod type -- nothing the strict rule can attribute.
        # This class is what stops that debris living forever.
        self.create()

        iniTxt = ("[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "\n"
                  "[ResourceFooRemapBlend]\n"
                  "filename = FooRemapBlend.buf")

        self.assertEqual(self.removeFrom(iniTxt), "")
        self.compareSet(set(self._remover.getRemovedSectionNames()),
                        {"TextureOverrideFooRemapBlend", "ResourceFooRemapBlend"})

        # ...and the resources still come out, classified exactly the way RemapIniRemover does it.
        removedResources = self._remover.getRemovedResources()
        self.compareSet(set(removedResources), {"blend"})
        self.assertTrue(removedResources["blend"][0].srcPath.endswith("FooRemapBlend.buf"))

    def test_explicitStrictContext_sweepStillHappens(self):
        # ignoreModType is this class's own decision, not the caller's -- an explicit False is
        # ignored rather than honoured.
        self.create()

        iniTxt = ("[TextureOverrideFooRemapBlend]\n"
                  "vb1 = ResourceFooRemapBlend\n"
                  "\n"
                  "[ResourceFooRemapBlend]\n"
                  "filename = FooRemapBlend.buf")

        context = FRB.IniRemovalContext(ignoreModType = False)
        self.assertEqual(self.removeFrom(iniTxt, context = context), "")

        # The context is taken by value all the way down, so the caller's own object is untouched.
        self.assertEqual(context.ignoreModType, False)

    def test_nonRemapSectionOutsideBoilerPlate_kept(self):
        # The sweep widens which CANDIDATES are targets, not what a candidate is: a section that is
        # neither inside the boilerplate nor Remap-named was never in the pool to begin with.
        self.create()

        iniTxt = ("[TextureOverrideFooBlend]\n"
                  "vb1 = ResourceFooBlend\n"
                  "\n"
                  "[ResourceFooBlend]\n"
                  "filename = FooBlend.buf")

        self.assertEqual(self.removeFrom(iniTxt), iniTxt)
        self.assertEqual(self._remover.getRemovedSectionNames(), [])

    def test_boilerPlateRegion_removedWithTheOriginalKept(self):
        # Unchanged from RemapIniRemover: the marker alone makes everything it surrounds a target,
        # and the region goes with it.
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

        self.assertNotIn("Remap", result)
        self.assertIn("[TextureOverrideFooBlend]", result)
        self.assertIn("[ResourceFooBlend]", result)

    def test_afterRemoval_iniNoLongerMarkedFixed(self):
        self.create()

        self._iniFile._isFixed = True
        self.removeFrom("; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -----------------------------------------------")
        self.assertEqual(self._iniFile.isFixed, False)
