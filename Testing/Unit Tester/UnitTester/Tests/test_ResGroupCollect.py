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
        self._fixer.graphGroupEdits = [FRB.ResGroupCollect({(0, "", "remapBlend"): {(0, "head", ""): "headVb1", (0, "body", ""): "bodyVb1", (0, "", "blend"): "vb1"},
                                                            (0, "", "remapNormalTex"): {(0, "head", ""): "headPs0"}},
                                                           {(0, "", "remapBlend"): FRB.RemapBlendReplace((0, "", "remapBlend")),
                                                            (0, "", "remapNormalTex"): FRB.TexCreate((0, "", "remapNormalTex"), "NormalMap", FRB.TexCreator(512, 512))},
                                                            FRB.IniGroupedResBuilder(FRB.RemapIniGroupedResource, args = ["testResGroup"]))]
        
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
\trun = Resource1RikaRemapBlend_RG0_0_0
endif

[TextureOverrideRaidenHead]
if 1
\trun = ResourceRikaNormalMapRemapTex_RG0_1_0
endif

[Resource1RikaRemapBlend_RG0_0_0]
filename = someRikaRemapBlend_CQD.buf

[ResourceRikaNormalMapRemapTex_RG0_1_0]
filename = RikaNormalMapRemapTex_Mhp.dds

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
\trun = Resource1RikaRemapBlend_RG0_0_1
endif
if 1
\trun = Resource1RikaRemapBlend_RG0_0_0
endif
if 1
\trun = Resource3RikaRemapBlend_RG0_0_1
endif
if 1
\trun = Resource3RikaRemapBlend_RG0_0_0
endif

[TextureOverrideRaidenHead]
if 1
\trun = ResourceRikaNormalMapRemapTex_RG0_1_1
endif
if 1
\trun = ResourceRikaNormalMapRemapTex_RG0_1_0
endif
if 1
\trun = ResourceRikaNormalMap1RemapTex_RG0_1_1
endif
if 1
\trun = ResourceRikaNormalMap1RemapTex_RG0_1_0
endif

[Resource1RikaRemapBlend_RG0_0_0]
key = 1
filename = someRikaRemapBlend_CQD.buf

[Resource3RikaRemapBlend_RG0_0_0]
key = 3
filename = someRikaRemapBlend2_CQD.buf

[Resource1RikaRemapBlend_RG0_0_1]
key = 1
filename = someRikaRemapBlend_GlL.buf

[Resource3RikaRemapBlend_RG0_0_1]
key = 3
filename = someRikaRemapBlend2_GlL.buf

[ResourceRikaNormalMapRemapTex_RG0_1_0]
filename = RikaNormalMapRemapTex_Mhp.dds

[ResourceRikaNormalMap1RemapTex_RG0_1_0]
filename = RikaNormalMap1RemapTex_Mhp.dds

[ResourceRikaNormalMapRemapTex_RG0_1_1]
filename = RikaNormalMapRemapTex_OKt.dds

[ResourceRikaNormalMap1RemapTex_RG0_1_1]
filename = RikaNormalMap1RemapTex_OKt.dds

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
\t\trun = Resource1RikaRemapBlend_RG0_0_0
\tendif
else if $x >= 5
\tif ($x >= 5) && ($x <= 6)
\t\trun = Resource3RikaRemapBlend_RG0_0_2
\tendif
\tif $x == 8
\t\trun = Resource3RikaRemapBlend_RG0_0_1
\tendif
\tif $y == 8 && ($x >= 5) && ($x <= 10) && $x != 8
\t\trun = Resource3RikaRemapBlend_RG0_0_0
\tendif
else
\tif $x < 5
\t\trun = Resource5RikaRemapBlend_RG0_0_1
\tendif
\tif $y == 8 && ($x < 5) && $x != 8
\t\trun = Resource5RikaRemapBlend_RG0_0_0
\tendif
endif

[TextureOverrideRaidenHead]
if $x <= 6
\tif ($x >= 5) && ($x <= 6)
\t\trun = ResourceRikaNormalMapRemapTex_RG0_1_1
\tendif
\tif $x < 5
\t\trun = ResourceRikaNormalMapRemapTex_RG0_1_0
\tendif
endif

[TextureOverrideRaiden2Head]
if $x == 8
\tif $x == 8
\t\trun = ResourceRikaNormalMap1RemapTex_RG0_1_0
\tendif
else
\trun = TextureOverrideRaiden3Head
endif

[TextureOverrideRaiden3Head]
if $y == 8
\tif $y == 8 && ($x > 10) && $x != 8
\t\trun = ResourceRikaNormalMap2RemapTex_RG0_1_2
\tendif
\tif $y == 8 && ($x >= 5) && ($x <= 10) && $x != 8
\t\trun = ResourceRikaNormalMap2RemapTex_RG0_1_1
\tendif
\tif $y == 8 && ($x < 5) && $x != 8
\t\trun = ResourceRikaNormalMap2RemapTex_RG0_1_0
\tendif
endif

[Resource1RikaRemapBlend_RG0_0_0]
key = 1
filename = someRikaRemapBlend_CQD.buf

[Resource3RikaRemapBlend_RG0_0_0]
key = 3
filename = someRikaRemapBlend2_CQD.buf

[Resource5RikaRemapBlend_RG0_0_0]
key = 5
filename = someRikaRemapBlend3_CQD.buf

[Resource3RikaRemapBlend_RG0_0_1]
key = 3
filename = someRikaRemapBlend2_GlL.buf

[Resource5RikaRemapBlend_RG0_0_1]
key = 5
filename = someRikaRemapBlend3_GlL.buf

[Resource3RikaRemapBlend_RG0_0_2]
key = 3
filename = someRikaRemapBlend2_Lw4.buf

[ResourceRikaNormalMapRemapTex_RG0_1_0]
filename = RikaNormalMapRemapTex_Mhp.dds

[ResourceRikaNormalMap1RemapTex_RG0_1_0]
filename = RikaNormalMap1RemapTex_Mhp.dds

[ResourceRikaNormalMap2RemapTex_RG0_1_0]
filename = RikaNormalMap2RemapTex_Mhp.dds

[ResourceRikaNormalMapRemapTex_RG0_1_1]
filename = RikaNormalMapRemapTex_OKt.dds

[ResourceRikaNormalMap2RemapTex_RG0_1_1]
filename = RikaNormalMap2RemapTex_OKt.dds

[ResourceRikaNormalMap2RemapTex_RG0_1_2]
filename = RikaNormalMap2RemapTex_cn.dds

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