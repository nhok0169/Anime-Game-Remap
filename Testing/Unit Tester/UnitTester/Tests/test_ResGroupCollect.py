import sys
from ordered_set import OrderedSet

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ResGroupCollectTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIParser(self._iniFile, modObjs = OrderedSet([("", "blend"), ("head", ""), ("body", "")]), trackKeys = False)
        
    def createFixer(self):
        self._fixer = FRB.GIMIFixer(self._parser,
                                    modsToFix = ["rika"])                                                                       

    def create(self):
        self.createIniFile()
        self.createParser()
        self.createFixer()
        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer

    # ====================== edit =======================================

    def test_editDifferentInis_resGroupCollected(self):
        self.create()
        self._fixer.graphGroupEdits = [FRB.ResGroupCollect(["OG"],
                                                           {(0, "", "remapBlend"): {(0, "head", ""): "headVb1", (0, "body", ""): "bodyVb1", (0, "", "blend"): "vb1"},
                                                            (0, "", "remapNormalTex"): {(0, "head", ""): "headPs0"}},
                                                           {(0, "", "remapBlend"): {"OG": FRB.RemapBlendReplace((0, "", "remapBlend"))},
                                                            (0, "", "remapNormalTex"): {"OG": FRB.TexCreate((0, "", "remapNormalTex"), "NormalMap", FRB.TexCreator(512, 512))}},
                                                            {"OG": FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["testResGroup"])},
                                                            id = 0)]
        
        tests = [
["""
[TextureOverrideRaidenHead]
ps-t0 = 1
ps-t2 = 2""", 0, ["""
[TextureOverrideRaidenHead]
ps-t0 = 1
ps-t2 = 2

; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHead]
ps-t0 = 1
ps-t2 = 2

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenHead]
headVb1 = Resource1

[Resource1]
filename = someBlend.buf
""", 0, ["""
[TextureOverrideRaidenHead]
headVb1 = Resource1

[Resource1]
filename = someBlend.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHead]
headVb1 = Resource1

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
vb1 = Resource1

[TextureOverrideRaidenHead]
headPs0 = Resource2

[Resource1]
filename = someBlend.buf

[Resource2]
filename = somefile.buf
""", 1, ["""
[TextureOverrideRaidenBlend]
vb1 = Resource1

[TextureOverrideRaidenHead]
headPs0 = Resource2

[Resource1]
filename = someBlend.buf

[Resource2]
filename = somefile.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenBlend]
if 1
\tvb1 = Resource1RikaRemapBlend0_0_0_0
endif

[TextureOverrideRaidenHead]
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_0_1_0
endif

[Resource1RikaRemapBlend0_0_0_0]
filename = someRikaRemapBlend_B8g.buf

[ResourceRikaNormalMapRemapTex0_0_1_0]
filename = RikaNormalMapRemapTex_Fz6.dds

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
vb1 = Resource1
vb1 = Resource3

[TextureOverrideRaidenHead]
headPs0 = Resource2
headPs0 = Resource4

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf
""", 4, ["""
[TextureOverrideRaidenBlend]
vb1 = Resource1
vb1 = Resource3

[TextureOverrideRaidenHead]
headPs0 = Resource2
headPs0 = Resource4

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenBlend]
if 1
\tvb1 = Resource1RikaRemapBlend0_0_0_0
endif
if 1
\tvb1 = Resource1RikaRemapBlend0_0_0_1
endif
if 1
\tvb1 = Resource3RikaRemapBlend0_0_0_0
endif
if 1
\tvb1 = Resource3RikaRemapBlend0_0_0_1
endif

[TextureOverrideRaidenHead]
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_0_1_0
endif
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_0_1_1
endif
if 1
\theadPs0 = ResourceRikaNormalMap1RemapTex0_0_1_0
endif
if 1
\theadPs0 = ResourceRikaNormalMap1RemapTex0_0_1_1
endif

[Resource1RikaRemapBlend0_0_0_0]
key = 1
filename = someRikaRemapBlend_B8g.buf

[Resource3RikaRemapBlend0_0_0_0]
key = 3
filename = someRikaRemapBlend2_B8g.buf

[Resource1RikaRemapBlend0_0_0_1]
key = 1
filename = someRikaRemapBlend_HIj.buf

[Resource3RikaRemapBlend0_0_0_1]
key = 3
filename = someRikaRemapBlend2_HIj.buf

[ResourceRikaNormalMapRemapTex0_0_1_0]
filename = RikaNormalMapRemapTex_Fz6.dds

[ResourceRikaNormalMap1RemapTex0_0_1_0]
filename = RikaNormalMap1RemapTex_Fz6.dds

[ResourceRikaNormalMapRemapTex0_0_1_1]
filename = RikaNormalMapRemapTex_HHn.dds

[ResourceRikaNormalMap1RemapTex0_0_1_1]
filename = RikaNormalMap1RemapTex_HHn.dds

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
if $x > 10
    vb1 = Resource1
else if $x >= 5
    vb1 = Resource3
else
    vb1 = Resource5
endif

[TextureOverrideRaidenHead]
if $x <= 6
    headPs0 = Resource2
endif

[TextureOverrideRaiden2Head]
if $x == 8
    headPs0 = Resource4
else
    run = TextureOverrideRaiden3Head
endif

[TextureOverrideRaiden3Head]
if $y == 8
    headPs0 = Resource6
endif

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf

[Resource5]
key = 5
filename = someBlend3.buf

[Resource6]
key = 6
filename = somefile3.buf
""", 6, ["""
[TextureOverrideRaidenBlend]
if $x > 10
    vb1 = Resource1
else if $x >= 5
    vb1 = Resource3
else
    vb1 = Resource5
endif

[TextureOverrideRaidenHead]
if $x <= 6
    headPs0 = Resource2
endif

[TextureOverrideRaiden2Head]
if $x == 8
    headPs0 = Resource4
else
    run = TextureOverrideRaiden3Head
endif

[TextureOverrideRaiden3Head]
if $y == 8
    headPs0 = Resource6
endif

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf

[Resource5]
key = 5
filename = someBlend3.buf

[Resource6]
key = 6
filename = somefile3.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenBlend]
if $x > 10
\tif $y == 8 && ($x > 10) && $x != 8
\t\tvb1 = Resource1RikaRemapBlend0_0_0_0
\tendif
else if $x >= 5
\tif ($x >= 5) && ($x <= 6)
\t\tvb1 = Resource3RikaRemapBlend0_0_0_0
\tendif
\tif $x == 8
\t\tvb1 = Resource3RikaRemapBlend0_0_0_1
\tendif
\tif $y == 8 && ($x >= 5) && ($x <= 10) && $x != 8
\t\tvb1 = Resource3RikaRemapBlend0_0_0_2
\tendif
else
\tif $x < 5
\t\tvb1 = Resource5RikaRemapBlend0_0_0_0
\tendif
\tif $y == 8 && ($x < 5) && $x != 8
\t\tvb1 = Resource5RikaRemapBlend0_0_0_1
\tendif
endif

[TextureOverrideRaidenHead]
if $x <= 6
\tif ($x >= 5) && ($x <= 6)
\t\theadPs0 = ResourceRikaNormalMapRemapTex0_0_1_0
\tendif
\tif $x < 5
\t\theadPs0 = ResourceRikaNormalMapRemapTex0_0_1_1
\tendif
endif

[TextureOverrideRaiden2Head]
if $x == 8
\tif $x == 8
\t\theadPs0 = ResourceRikaNormalMap1RemapTex0_0_1_0
\tendif
else
\trun = TextureOverrideRaiden3Head
endif

[TextureOverrideRaiden3Head]
if $y == 8
\tif $y == 8 && ($x > 10) && $x != 8
\t\theadPs0 = ResourceRikaNormalMap2RemapTex0_0_1_0
\tendif
\tif $y == 8 && ($x >= 5) && ($x <= 10) && $x != 8
\t\theadPs0 = ResourceRikaNormalMap2RemapTex0_0_1_1
\tendif
\tif $y == 8 && ($x < 5) && $x != 8
\t\theadPs0 = ResourceRikaNormalMap2RemapTex0_0_1_2
\tendif
endif

[Resource1RikaRemapBlend0_0_0_0]
key = 1
filename = someRikaRemapBlend_B8g.buf

[Resource3RikaRemapBlend0_0_0_0]
key = 3
filename = someRikaRemapBlend2_B8g.buf

[Resource5RikaRemapBlend0_0_0_0]
key = 5
filename = someRikaRemapBlend3_B8g.buf

[Resource3RikaRemapBlend0_0_0_1]
key = 3
filename = someRikaRemapBlend2_HIj.buf

[Resource5RikaRemapBlend0_0_0_1]
key = 5
filename = someRikaRemapBlend3_HIj.buf

[Resource3RikaRemapBlend0_0_0_2]
key = 3
filename = someRikaRemapBlend2_NHd.buf

[ResourceRikaNormalMapRemapTex0_0_1_0]
filename = RikaNormalMapRemapTex_Fz6.dds

[ResourceRikaNormalMap1RemapTex0_0_1_0]
filename = RikaNormalMap1RemapTex_Fz6.dds

[ResourceRikaNormalMap2RemapTex0_0_1_0]
filename = RikaNormalMap2RemapTex_Fz6.dds

[ResourceRikaNormalMapRemapTex0_0_1_1]
filename = RikaNormalMapRemapTex_HHn.dds

[ResourceRikaNormalMap2RemapTex0_0_1_1]
filename = RikaNormalMap2RemapTex_HHn.dds

[ResourceRikaNormalMap2RemapTex0_0_1_2]
filename = RikaNormalMap2RemapTex_If0.dds

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
if $x > 10
    vb1 = Resource1
else if $x >= 5
    vb1 = Resource3
endif

[TextureOverrideRaidenHead]
if $x <= 0
    headPs0 = Resource2
endif

[TextureOverrideRaiden2Head]
if 0
    headPs0 = Resource4
endif

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf
""", 0, ["""
[TextureOverrideRaidenBlend]
if $x > 10
    vb1 = Resource1
else if $x >= 5
    vb1 = Resource3
endif

[TextureOverrideRaidenHead]
if $x <= 0
    headPs0 = Resource2
endif

[TextureOverrideRaiden2Head]
if 0
    headPs0 = Resource4
endif

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenBlend]
if $x > 10
\tvb1 = Resource1
else if $x >= 5
\tvb1 = Resource3
endif

[TextureOverrideRaidenHead]
if $x <= 0
\theadPs0 = Resource2
endif

[TextureOverrideRaiden2Head]
if 0
\theadPs0 = Resource4
endif

; --------------------------------------------"""]]
]
        
        for test in tests:
            self.clearHashStates()
            iniTxt = test[0]
            expectedResourceCount = test[1]
            expectedIniTxt = test[2]

            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer

            self._iniFile.fileTxt = iniTxt

            self._iniFile.parse()
            resultFix = self._iniFile.fix()
            resultResources = self._iniFile.resources

            self.assertEqual(len(resultResources), expectedResourceCount)

            fixLen = len(resultFix)
            self.assertEqual(fixLen, len(expectedIniTxt))

            resultFix = list(resultFix.values())
            for i in range(fixLen):
                currentResultFix = resultFix[i]
                currentExpectedFix = expectedIniTxt[i]

                self.assertEqual(currentResultFix, currentExpectedFix)

    def test_editDifferentInisWithRemaps_resGroupCollected(self):
        self.create()
        self._fixer.graphGroupEdits = [FRB.ResGroupCollect(["OG", "Semi", "Full"],
                                                           {(0, "", "remapBlend"): {(0, "head", ""): "headVb1", (0, "body", ""): "bodyVb1", (0, "", "blend"): "vb1"},
                                                            (0, "", "remapNormalTex"): {(0, "head", ""): "headPs0"}},
                                                           {(0, "", "remapBlend"): {"OG": FRB.RemapBlendReplace((0, "OG", "remapBlend")),
                                                                                    "Semi": FRB.RemapBlendReplace((0, "Semi", "remapBlend")),
                                                                                    "Full": FRB.RemapBlendReplace((0, "Full", "remapBlend"))},
                                                            (0, "", "remapNormalTex"): {"OG": FRB.TexCreate((0, "OG", "remapNormalTex"), "NormalMap", FRB.TexCreator(512, 512)),
                                                                                        "Semi": FRB.TexCreate((0, "Semi", "remapNormalTex"), "NormalMap", FRB.TexCreator(512, 512)),
                                                                                        "Full": FRB.TexCreate((0, "Full", "remapNormalTex"), "NormalMap", FRB.TexCreator(512, 512))}},
                                                            {"OG": FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["OGtestResGroup"]),
                                                             "Semi": FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["SemitestResGroup"]),
                                                             "Full": FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["FulltestResGroup"])},
                                                             trackKeys = True,
                                                             keysToTrack = {"OG": {"hash"},
                                                                            "Semi": {"hash"},
                                                                            "Full": {"hash"}},
                                                             remaps = {(0, "head", ""): {"Semi": (0, "Semi", "headGone", lambda name: name + "Semi"),
                                                                                         "Full": (0, "Full", "headGone", lambda name: name + "Full")},
                                                                       (0, "body", ""): {"Full": (0, "Full", "bodyGone", lambda name: name + "Full")},
                                                                       (0, "", "blend"): {"Full": (0, "Full", "blendGone", lambda name: name + "Full")}},
                                                             partPredicates = {(0, "", "remapBlend"): {(0, "head", ""): lambda iterData: iterData.colouring.getRanges(keysExists = {"hash": True}, keyFilters = {"hash": lambda ind, val: val == "head"}), 
                                                                                                       (0, "body", ""): lambda iterData: iterData.colouring.getRanges(keysExists = {"hash": True}, keyFilters = {"hash": lambda ind, val: val == "body"}), 
                                                                                                       (0, "", "blend"): lambda iterData: iterData.colouring.getRanges(keysExists = {"hash": True}, keyFilters = {"hash": lambda ind, val: val == "blend"})},
                                                                               (0, "", "remapNormalTex"): {(0, "head", ""): lambda iterData: iterData.colouring.getRanges(keysExists = {"hash": True}, keyFilters = {"hash": lambda ind, val: val == "head"})}},
                                                             resGroupTypesSameTopology = True,
                                                             id = 0)]
        
        tests = [
["""
[TextureOverrideRaidenHead]
ps-t0 = 1
ps-t2 = 2""", 0, ["""
[TextureOverrideRaidenHead]
ps-t0 = 1
ps-t2 = 2

; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHeadSemi]
ps-t0 = 1
ps-t2 = 2

[TextureOverrideRaidenHeadFull]
ps-t0 = 1
ps-t2 = 2

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenHead]
headVb1 = Resource1

[Resource1]
filename = someBlend.buf
""", 0, ["""
[TextureOverrideRaidenHead]
headVb1 = Resource1

[Resource1]
filename = someBlend.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHeadSemi]
headVb1 = Resource1

[TextureOverrideRaidenHeadFull]
headVb1 = Resource1

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
vb1 = Resource1

[TextureOverrideRaidenHead]
headPs0 = Resource2

[Resource1]
filename = someBlend.buf

[Resource2]
filename = somefile.buf
""", 0, ["""
[TextureOverrideRaidenBlend]
vb1 = Resource1

[TextureOverrideRaidenHead]
headPs0 = Resource2

[Resource1]
filename = someBlend.buf

[Resource2]
filename = somefile.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHeadSemi]
headPs0 = Resource2

[TextureOverrideRaidenHeadFull]
headPs0 = Resource2

[TextureOverrideRaidenBlendFull]
vb1 = Resource1

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
hash = blend
vb1 = Resource1

[TextureOverrideRaidenHead]
hash = head
headPs0 = Resource2

[Resource1]
filename = someBlend.buf

[Resource2]
filename = somefile.buf
""", 3, ["""
[TextureOverrideRaidenBlend]
hash = blend
vb1 = Resource1

[TextureOverrideRaidenHead]
hash = head
headPs0 = Resource2

[Resource1]
filename = someBlend.buf

[Resource2]
filename = somefile.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHeadSemi]
hash = head
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_1_1_0
endif

[TextureOverrideRaidenHeadFull]
hash = head
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_2_1_0
endif

[TextureOverrideRaidenBlendFull]
hash = blend
if 1
\tvb1 = Resource1RikaRemapBlend0_2_0_0
endif

[Resource1RikaRemapBlend0_0_0_0]
filename = someRikaRemapBlend_B8g.buf

[ResourceRikaNormalMapRemapTex0_0_1_0]
filename = RikaNormalMapRemapTex_Fz6.dds

[Resource1RikaRemapBlend0_1_0_0]
filename = someRikaRemapBlend_MQ5.buf

[ResourceRikaNormalMapRemapTex0_1_1_0]
filename = RikaNormalMapRemapTex_OOZ.dds

[Resource1RikaRemapBlend0_2_0_0]
filename = someRikaRemapBlend_LRT.buf

[ResourceRikaNormalMapRemapTex0_2_1_0]
filename = RikaNormalMapRemapTex_Mnb.dds

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
hash = blend
vb1 = Resource1
vb1 = Resource3

[TextureOverrideRaidenHead]
hash = head
headPs0 = Resource2
headPs0 = Resource4

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf
""", 12, ["""
[TextureOverrideRaidenBlend]
hash = blend
vb1 = Resource1
vb1 = Resource3

[TextureOverrideRaidenHead]
hash = head
headPs0 = Resource2
headPs0 = Resource4

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHeadSemi]
hash = head
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_1_1_0
endif
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_1_1_1
endif
if 1
\theadPs0 = ResourceRikaNormalMap1RemapTex0_1_1_0
endif
if 1
\theadPs0 = ResourceRikaNormalMap1RemapTex0_1_1_1
endif

[TextureOverrideRaidenHeadFull]
hash = head
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_2_1_0
endif
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_2_1_1
endif
if 1
\theadPs0 = ResourceRikaNormalMap1RemapTex0_2_1_0
endif
if 1
\theadPs0 = ResourceRikaNormalMap1RemapTex0_2_1_1
endif

[TextureOverrideRaidenBlendFull]
hash = blend
if 1
\tvb1 = Resource1RikaRemapBlend0_2_0_0
endif
if 1
\tvb1 = Resource1RikaRemapBlend0_2_0_1
endif
if 1
\tvb1 = Resource3RikaRemapBlend0_2_0_0
endif
if 1
\tvb1 = Resource3RikaRemapBlend0_2_0_1
endif

[Resource1RikaRemapBlend0_0_0_0]
key = 1
filename = someRikaRemapBlend_B8g.buf

[Resource3RikaRemapBlend0_0_0_0]
key = 3
filename = someRikaRemapBlend2_B8g.buf

[Resource1RikaRemapBlend0_0_0_1]
key = 1
filename = someRikaRemapBlend_HIj.buf

[Resource3RikaRemapBlend0_0_0_1]
key = 3
filename = someRikaRemapBlend2_HIj.buf

[ResourceRikaNormalMapRemapTex0_0_1_0]
filename = RikaNormalMapRemapTex_Fz6.dds

[ResourceRikaNormalMap1RemapTex0_0_1_0]
filename = RikaNormalMap1RemapTex_Fz6.dds

[ResourceRikaNormalMapRemapTex0_0_1_1]
filename = RikaNormalMapRemapTex_HHn.dds

[ResourceRikaNormalMap1RemapTex0_0_1_1]
filename = RikaNormalMap1RemapTex_HHn.dds

[Resource1RikaRemapBlend0_1_0_0]
key = 1
filename = someRikaRemapBlend_MQ5.buf

[Resource3RikaRemapBlend0_1_0_0]
key = 3
filename = someRikaRemapBlend2_MQ5.buf

[Resource1RikaRemapBlend0_1_0_1]
key = 1
filename = someRikaRemapBlend_JmC.buf

[Resource3RikaRemapBlend0_1_0_1]
key = 3
filename = someRikaRemapBlend2_JmC.buf

[ResourceRikaNormalMapRemapTex0_1_1_0]
filename = RikaNormalMapRemapTex_OOZ.dds

[ResourceRikaNormalMap1RemapTex0_1_1_0]
filename = RikaNormalMap1RemapTex_OOZ.dds

[ResourceRikaNormalMapRemapTex0_1_1_1]
filename = RikaNormalMapRemapTex_FbM.dds

[ResourceRikaNormalMap1RemapTex0_1_1_1]
filename = RikaNormalMap1RemapTex_FbM.dds

[Resource1RikaRemapBlend0_2_0_0]
key = 1
filename = someRikaRemapBlend_LRT.buf

[Resource3RikaRemapBlend0_2_0_0]
key = 3
filename = someRikaRemapBlend2_LRT.buf

[Resource1RikaRemapBlend0_2_0_1]
key = 1
filename = someRikaRemapBlend_HWh.buf

[Resource3RikaRemapBlend0_2_0_1]
key = 3
filename = someRikaRemapBlend2_HWh.buf

[ResourceRikaNormalMapRemapTex0_2_1_0]
filename = RikaNormalMapRemapTex_Mnb.dds

[ResourceRikaNormalMap1RemapTex0_2_1_0]
filename = RikaNormalMap1RemapTex_Mnb.dds

[ResourceRikaNormalMapRemapTex0_2_1_1]
filename = RikaNormalMapRemapTex_N2i.dds

[ResourceRikaNormalMap1RemapTex0_2_1_1]
filename = RikaNormalMap1RemapTex_N2i.dds

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
hash = blend
vb1 = Resource1
hash = poopoo
vb1 = Resource3

[TextureOverrideRaidenHead]
hash = peepee
hash = head
headPs0 = Resource2
headPs0 = Resource4

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf
""", 6, ["""
[TextureOverrideRaidenBlend]
hash = blend
vb1 = Resource1
hash = poopoo
vb1 = Resource3

[TextureOverrideRaidenHead]
hash = peepee
hash = head
headPs0 = Resource2
headPs0 = Resource4

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHeadSemi]
hash = peepee
hash = head
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_1_1_0
endif
if 1
\theadPs0 = ResourceRikaNormalMap1RemapTex0_1_1_0
endif

[TextureOverrideRaidenHeadFull]
hash = peepee
hash = head
if 1
\theadPs0 = ResourceRikaNormalMapRemapTex0_2_1_0
endif
if 1
\theadPs0 = ResourceRikaNormalMap1RemapTex0_2_1_0
endif

[TextureOverrideRaidenBlendFull]
hash = blend
if 1
\tvb1 = Resource1RikaRemapBlend0_2_0_0
endif
if 1
\tvb1 = Resource1RikaRemapBlend0_2_0_1
endif
hash = poopoo
vb1 = Resource3

[Resource1RikaRemapBlend0_0_0_0]
key = 1
filename = someRikaRemapBlend_B8g.buf

[Resource1RikaRemapBlend0_0_0_1]
key = 1
filename = someRikaRemapBlend_HIj.buf

[ResourceRikaNormalMapRemapTex0_0_1_0]
filename = RikaNormalMapRemapTex_Fz6.dds

[ResourceRikaNormalMap1RemapTex0_0_1_0]
filename = RikaNormalMap1RemapTex_Fz6.dds

[Resource1RikaRemapBlend0_1_0_0]
key = 1
filename = someRikaRemapBlend_MQ5.buf

[Resource1RikaRemapBlend0_1_0_1]
key = 1
filename = someRikaRemapBlend_JmC.buf

[ResourceRikaNormalMapRemapTex0_1_1_0]
filename = RikaNormalMapRemapTex_OOZ.dds

[ResourceRikaNormalMap1RemapTex0_1_1_0]
filename = RikaNormalMap1RemapTex_OOZ.dds

[Resource1RikaRemapBlend0_2_0_0]
key = 1
filename = someRikaRemapBlend_LRT.buf

[Resource1RikaRemapBlend0_2_0_1]
key = 1
filename = someRikaRemapBlend_HWh.buf

[ResourceRikaNormalMapRemapTex0_2_1_0]
filename = RikaNormalMapRemapTex_Mnb.dds

[ResourceRikaNormalMap1RemapTex0_2_1_0]
filename = RikaNormalMap1RemapTex_Mnb.dds

; --------------------------------------------"""]],

["""
[TextureOverrideRaidenBlend]
hash = blend
if $x > 10
    vb1 = Resource1
else if $x >= 5
    hash = peepee
    vb1 = Resource3
else
    vb1 = Resource5
endif
hash = bang

[TextureOverrideRaidenHead]
if $x <= 6
    headPs0 = Resource2
endif

[TextureOverrideRaiden2Head]
if $x == 8
    hash = boo
    headPs0 = Resource4
else
    hash = head
    run = TextureOverrideRaiden3Head
endif

[TextureOverrideRaiden3Head]
if $y == 8
    headPs0 = Resource6
endif

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf

[Resource5]
key = 5
filename = someBlend3.buf

[Resource6]
key = 6
filename = somefile3.buf
""", 6, ["""
[TextureOverrideRaidenBlend]
hash = blend
if $x > 10
    vb1 = Resource1
else if $x >= 5
    hash = peepee
    vb1 = Resource3
else
    vb1 = Resource5
endif
hash = bang

[TextureOverrideRaidenHead]
if $x <= 6
    headPs0 = Resource2
endif

[TextureOverrideRaiden2Head]
if $x == 8
    hash = boo
    headPs0 = Resource4
else
    hash = head
    run = TextureOverrideRaiden3Head
endif

[TextureOverrideRaiden3Head]
if $y == 8
    headPs0 = Resource6
endif

[Resource1]
key = 1
filename = someBlend.buf

[Resource2]
key = 2
filename = somefile.buf

[Resource3]
key = 3
filename = someBlend2.buf

[Resource4]
key = 4
filename = somefile2.buf

[Resource5]
key = 5
filename = someBlend3.buf

[Resource6]
key = 6
filename = somefile3.buf


; --------------- Raiden Remap ---------------
; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

[TextureOverrideRaidenHeadSemi]
if $x <= 6
\theadPs0 = Resource2
endif

[TextureOverrideRaiden2HeadSemi]
if $x == 8
\thash = boo
\theadPs0 = Resource4
else
\thash = head
\trun = TextureOverrideRaiden3HeadSemi
endif

[TextureOverrideRaiden3HeadSemi]
if $y == 8
\tif $y == 8 && ($x > 10) && $x != 8
\t\theadPs0 = ResourceRikaNormalMapRemapTex0_1_1_0
\tendif
\tif $y == 8 && ($x < 5) && $x != 8
\t\theadPs0 = ResourceRikaNormalMapRemapTex0_1_1_1
\tendif
endif

[TextureOverrideRaidenHeadFull]
if $x <= 6
\theadPs0 = Resource2
endif

[TextureOverrideRaiden2HeadFull]
if $x == 8
\thash = boo
\theadPs0 = Resource4
else
\thash = head
\trun = TextureOverrideRaiden3HeadFull
endif

[TextureOverrideRaiden3HeadFull]
if $y == 8
\tif $y == 8 && ($x > 10) && $x != 8
\t\theadPs0 = ResourceRikaNormalMapRemapTex0_2_1_0
\tendif
\tif $y == 8 && ($x < 5) && $x != 8
\t\theadPs0 = ResourceRikaNormalMapRemapTex0_2_1_1
\tendif
endif

[TextureOverrideRaidenBlendFull]
hash = blend
if $x > 10
\tif $y == 8 && ($x > 10) && $x != 8
\t\tvb1 = Resource1RikaRemapBlend0_2_0_0
\tendif
else if $x >= 5
\thash = peepee
\tvb1 = Resource3
else
\tif $y == 8 && ($x < 5) && $x != 8
\t\tvb1 = Resource5RikaRemapBlend0_2_0_0
\tendif
endif
hash = bang

[Resource1RikaRemapBlend0_0_0_0]
key = 1
filename = someRikaRemapBlend_B8g.buf

[Resource5RikaRemapBlend0_0_0_0]
key = 5
filename = someRikaRemapBlend3_B8g.buf

[ResourceRikaNormalMapRemapTex0_0_1_0]
filename = RikaNormalMapRemapTex_Fz6.dds

[ResourceRikaNormalMapRemapTex0_0_1_1]
filename = RikaNormalMapRemapTex_HHn.dds

[Resource1RikaRemapBlend0_1_0_0]
key = 1
filename = someRikaRemapBlend_MQ5.buf

[Resource5RikaRemapBlend0_1_0_0]
key = 5
filename = someRikaRemapBlend3_MQ5.buf

[ResourceRikaNormalMapRemapTex0_1_1_0]
filename = RikaNormalMapRemapTex_OOZ.dds

[ResourceRikaNormalMapRemapTex0_1_1_1]
filename = RikaNormalMapRemapTex_FbM.dds

[Resource1RikaRemapBlend0_2_0_0]
key = 1
filename = someRikaRemapBlend_LRT.buf

[Resource5RikaRemapBlend0_2_0_0]
key = 5
filename = someRikaRemapBlend3_LRT.buf

[ResourceRikaNormalMapRemapTex0_2_1_0]
filename = RikaNormalMapRemapTex_Mnb.dds

[ResourceRikaNormalMapRemapTex0_2_1_1]
filename = RikaNormalMapRemapTex_N2i.dds

; --------------------------------------------"""]]
]
        sameTopology = [False, True]

        for fixSameTopology in sameTopology:
            for test in tests:
                self.clearHashStates()
                self._fixer.graphGroupEdits[0].resGroupTypesSameTopology = fixSameTopology

                iniTxt = test[0]
                expectedResourceCount = test[1]
                expectedIniTxt = test[2]

                self._iniFile.clear()
                self._iniFile._iniParser = self._parser
                self._iniFile._iniFixer = self._fixer

                self._iniFile.fileTxt = iniTxt

                self._iniFile.parse()
                resultFix = self._iniFile.fix()
                resultResources = self._iniFile.resources

                self.assertEqual(len(resultResources), expectedResourceCount)

                fixLen = len(resultFix)
                self.assertEqual(fixLen, len(expectedIniTxt))

                resultFix = list(resultFix.values())
                for i in range(fixLen):
                    currentResultFix = resultFix[i]
                    currentExpectedFix = expectedIniTxt[i]

                    self.assertEqual(currentResultFix, currentExpectedFix)