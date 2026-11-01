import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class WWMIRuntimeTest(BaseUnitTest):
    """
    Tests for :class:`WWMIRuntime` -- the sections a fix of a Wuthering Waves mod writes to drive WWMI itself
    """

    def skeleton(self, remapped = False):
        skeleton = FRB.WWMIRuntime.Skeleton()
        skeleton.name = lambda name: name + "Fix"
        skeleton.cb4Hash = "abcd1234"
        skeleton.boneDataFilter = "3000"
        skeleton.mergedSkeletonSlots = 256
        skeleton.slotWindows = [("0", "10"), ("10", "5")]
        skeleton.remapped = remapped
        skeleton.blendRemapForward = "ResourceForwardFix"
        return skeleton

    def names(self, sections):
        return [section.name for section in sections]

    def test_skeletonSections_theMergedSkeletonTheMarkerAndOneMergeListASlot(self):
        names = self.names(FRB.WWMIRuntime.skeletonSections(FRB.Z3Context(), self.skeleton()))
        self.assertEqual(names, ["ResourceMergedSkeletonFix", "ResourceExtraMergedSkeletonFix", "ResourceMergedSkeletonRWFix",
                                 "ResourceExtraMergedSkeletonRWFix", "TextureOverrideMarkBoneDataCBFix",
                                 "CommandListMergeSlot0Fix", "CommandListMergeSlot1Fix"])

    def test_skeletonSections_remapped_addsTheRemappedSkeletonAndItsLists(self):
        names = self.names(FRB.WWMIRuntime.skeletonSections(FRB.Z3Context(), self.skeleton(remapped = True)))
        self.assertIn("ResourceRemappedSkeletonFix", names)
        self.assertIn("CommandListRemapMergedSkeletonFix", names)
        self.assertIn("CommandListMergeWindowFix", names)
        self.assertEqual(names[-2:], ["CommandListMergeSlot0Fix", "CommandListMergeSlot1Fix"])

    def test_mergeListName_isTheFixsName(self):
        self.assertEqual(FRB.WWMIRuntime.mergeListName(self.skeleton(), 3), "CommandListMergeSlot3Fix")

    def test_blendRemapSections_theResourcesThenTheInitList(self):
        blendRemap = FRB.WWMIRuntime.BlendRemap()
        blendRemap.name = lambda name: name + "Fix"
        blendRemap.blendFile = "Meshes/RemapBlend.buf"
        blendRemap.remapFile = lambda which: which + ".buf"
        blendRemap.blendStride = 8
        blendRemap.influences = 4
        blendRemap.blendReg = "vb4"

        names = self.names(FRB.WWMIRuntime.blendRemapSections(FRB.Z3Context(), blendRemap))
        self.assertEqual(names[-1], FRB.WWMIRuntime.blendRemapInitList(blendRemap))
        self.assertEqual(len(names), 8)

    def test_remappedSkeletonRemovals_theMergeListItsRoutesAndTheSkeletonBindings(self):
        removals = FRB.WWMIRuntime.remappedSkeletonRemovals()
        self.assertIn(("run", "commandlistmergeskeleton"), removals)
        self.assertIn(("CheckTextureOverride", "vs-cb4"), removals)
        self.assertIn(("vs-cb3", "resourcemergedskeleton"), removals)
        self.assertEqual(len(removals), 6)

    def hide(self, **fields):
        hide = FRB.WWMIRuntime.Hide()
        hide.name = "TextureOverrideHide"
        hide.vb0Hash = "abcd1234"
        hide.indexOffset, hide.indexCount, hide.vgOffset, hide.vgCount = "0", "300", "10", "20"
        hide.slot = "3"
        hide.mergeSkeletonList = "CommandListMergeSkeleton"
        for name, value in fields.items():
            setattr(hide, name, value)
        return FRB.WWMIRuntime.hideSection(FRB.Z3Context(), hide)

    def test_hideSection_isNamedAndCarriesTheSlotsMatchKeys(self):
        section = self.hide()
        self.assertEqual(section.name, "TextureOverrideHide")
