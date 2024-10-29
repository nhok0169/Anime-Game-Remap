import sys

from .baseIniObjTest import BaseIniObjTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class GIMIObjMergeFixerTest(BaseIniObjTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._defaultIniTxt = r"""
[Constants]
global persist $swapvar = 0
global persist $swapscarf = 0
global $active
global $creditinfo = 0

[KeySwap]
condition = $active == 1
key = h
type = cycle
$swapvar = 0,1
$creditinfo = 0

[KeySwapScarf]
condition = $active == 1
key = y
type = cycle
$swapscarf = 0,1
$creditinfo = 0


[Present]
post $active = 0

; Shader ------------------------------

; Overrides ---------------------------

[TextureOverrideKeqingPosition]
hash = 3aaf3e94
run = CommandListKeqingPosition
$active = 1

[TextureOverrideKeqingBlend]
hash = 0bf8e621
run = CommandListKeqingBlend

[TextureOverrideKeqingTexcoord]
hash = 723848fe
run = CommandListKeqingTexcoord

[TextureOverrideKeqingVertexLimitRaise]
hash = 4526145e

[TextureOverrideKeqingIB]
hash = cbf1894b
run = CommandListKeqingIB

[TextureOverrideKeqingHead]
hash = cbf1894b
match_first_index = 0
run = CommandListKeqingHead

[TextureOverrideKeqingBody]
hash = cbf1894b
match_first_index = 10824
run = CommandListKeqingBody

[TextureOverrideKeqingDress]
hash = cbf1894b
match_first_index = 48216
run = CommandListKeqingDress

[TextureOverrideKeqingFaceHeadDiffuse]
hash = d8c9c399
run = CommandListKeqingFaceHeadDiffuse

[TextureOverride41FixVertexLimitRaise]
hash = ccc33b79

; CommandList -------------------------

[CommandListKeqingPosition]
if $swapvar == 0 && $swapscarf == 0
    vb0 = ResourceKeqingPosition0
else if $swapvar == 1 && $swapscarf == 0
    vb0 = ResourceKeqingPosition1
else if $swapvar == 0 && $swapscarf == 1
    vb0 = ResourceKeqingPosition2
else if $swapvar == 1 && $swapscarf == 1
    vb0 = ResourceKeqingPosition3
endif

[CommandListKeqingBlend]
if $swapvar == 0 && $swapscarf == 0
    vb1 = ResourceKeqingBlend0
    handling = skip
    draw = 24194,0
else if $swapvar == 1 && $swapscarf == 0
    vb1 = ResourceKeqingBlend1
    handling = skip
    draw = 24963,0
else if $swapvar == 0 && $swapscarf == 1
    vb1 = ResourceKeqingBlend2
    handling = skip
    draw = 30036,0
else if $swapvar == 1 && $swapscarf == 1
    vb1 = ResourceKeqingBlend3
    handling = skip
    draw = 31265,0
endif

[CommandListKeqingTexcoord]
if $swapvar == 0 && $swapscarf == 0
    vb1 = ResourceKeqingTexcoord0
else if $swapvar == 1 && $swapscarf == 0
    vb1 = ResourceKeqingTexcoord1
else if $swapvar == 0 && $swapscarf == 1
    vb1 = ResourceKeqingTexcoord2
else if $swapvar == 1 && $swapscarf == 1
    vb1 = ResourceKeqingTexcoord3
endif

[CommandListKeqingIB]
if $swapvar == 0 && $swapscarf == 0
    handling = skip
    drawindexed = auto
else if $swapvar == 1 && $swapscarf == 0
    handling = skip
    drawindexed = auto
else if $swapvar == 0 && $swapscarf == 1
    handling = skip
    drawindexed = auto
else if $swapvar == 1 && $swapscarf == 1
    handling = skip
    drawindexed = auto
endif

[CommandListKeqingHead]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingHeadIB0
    ps-t0 = ResourceKeqingHeadDiffuse0
    ps-t1 = ResourceKeqingHeadLightMap0
    ps-t2 = ResourceKeqingHeadMetalMap0
    ps-t3 = ResourceKeqingHeadShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingHeadIB1
    ps-t0 = ResourceKeqingHeadDiffuse1
    ps-t1 = ResourceKeqingHeadLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingHeadIB2
    ps-t0 = ResourceKeqingHeadDiffuse2
    ps-t1 = ResourceKeqingHeadLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingHeadIB3
    ps-t0 = ResourceKeqingHeadDiffuse3
    ps-t1 = ResourceKeqingHeadLightMap3
endif

[CommandListKeqingBody]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingBodyIB0
    ps-t0 = ResourceKeqingBodyDiffuse0
    ps-t1 = ResourceKeqingBodyLightMap0
    ps-t2 = ResourceKeqingBodyMetalMap0
    ps-t3 = ResourceKeqingBodyShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingBodyIB1
    ps-t0 = ResourceKeqingBodyDiffuse1
    ps-t1 = ResourceKeqingBodyLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingBodyIB2
    ps-t0 = ResourceKeqingBodyDiffuse2
    ps-t1 = ResourceKeqingBodyLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingBodyIB3
    ps-t0 = ResourceKeqingBodyDiffuse3
    ps-t1 = ResourceKeqingBodyLightMap3
endif

[CommandListKeqingDress]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingDressIB0
    ps-t0 = ResourceKeqingDressDiffuse0
    ps-t1 = ResourceKeqingDressLightMap0
    ps-t2 = ResourceKeqingDressMetalMap0
    ps-t3 = ResourceKeqingDressShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingDressIB1
    ps-t0 = ResourceKeqingDressDiffuse1
    ps-t1 = ResourceKeqingDressLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingDressIB2
    ps-t0 = ResourceKeqingDressDiffuse2
    ps-t1 = ResourceKeqingDressLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingDressIB3
    ps-t0 = ResourceKeqingDressDiffuse3
    ps-t1 = ResourceKeqingDressLightMap3
endif

[CommandListKeqingFaceHeadDiffuse]
if $swapvar == 0 && $swapscarf == 0
    ps-t0 = ResourceKeqingFaceHeadDiffuse0
else if $swapvar == 1 && $swapscarf == 0
    ps-t0 = ResourceKeqingFaceHeadDiffuse1
else if $swapvar == 0 && $swapscarf == 1
    ps-t0 = ResourceKeqingFaceHeadDiffuse2
else if $swapvar == 1 && $swapscarf == 1
    ps-t0 = ResourceKeqingFaceHeadDiffuse3
endif

; Resources ---------------------------

[ResourceKeqingPosition0]
type = Buffer
stride = 40
filename = 0 - keqing_firstlanternrite\KeqingPosition.buf

[ResourceKeqingBlend0]
type = Buffer
stride = 32
filename = 0 - keqing_firstlanternrite\KeqingBlend.buf

[ResourceKeqingTexcoord0]
type = Buffer
stride = 20
filename = 0 - keqing_firstlanternrite\KeqingTexcoord.buf

[ResourceKeqingHeadIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingHead.ib

[ResourceKeqingBodyIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingBody.ib

[ResourceKeqingDressIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingDress.ib

[ResourceKeqingHeadDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition1]
type = Buffer
stride = 40
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingPosition.buf

[ResourceKeqingBlend1]
type = Buffer
stride = 32
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBlend.buf

[ResourceKeqingTexcoord1]
type = Buffer
stride = 20
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingTexcoord.buf

[ResourceKeqingHeadIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHead.ib

[ResourceKeqingBodyIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBody.ib

[ResourceKeqingDressIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDress.ib

[ResourceKeqingHeadDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition2]
type = Buffer
stride = 40
filename = 2 - keqing_firstlanternrite_scarf\KeqingPosition.buf

[ResourceKeqingBlend2]
type = Buffer
stride = 32
filename = 2 - keqing_firstlanternrite_scarf\KeqingBlend.buf

[ResourceKeqingTexcoord2]
type = Buffer
stride = 20
filename = 2 - keqing_firstlanternrite_scarf\KeqingTexcoord.buf

[ResourceKeqingHeadIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingHead.ib

[ResourceKeqingBodyIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingBody.ib

[ResourceKeqingDressIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingDress.ib

[ResourceKeqingHeadDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition3]
type = Buffer
stride = 40
filename = 3 - keqingfirstlanternrite-shorts\KeqingPosition.buf

[ResourceKeqingBlend3]
type = Buffer
stride = 32
filename = 3 - keqingfirstlanternrite-shorts\KeqingBlend.buf

[ResourceKeqingTexcoord3]
type = Buffer
stride = 20
filename = 3 - keqingfirstlanternrite-shorts\KeqingTexcoord.buf

[ResourceKeqingHeadIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingHead.ib

[ResourceKeqingBodyIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingBody.ib

[ResourceKeqingDressIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingDress.ib

[ResourceKeqingHeadDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingFaceHeadDiffuse.dds
"""
        
        cls._iniTxtLines = []
        cls.setupIniTxt(cls._defaultIniTxt)

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIObjParser(self._iniFile, {"body", "dress"})

    def createFixer(self):
        self._fixer = FRB.GIMIObjMergeFixer(self._parser, {"body": ["body", "dress"]})

    def create(self):
        self.createIniFile()
        self.createParser()
        self.createFixer()
        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer

    # ========================= fix ======================================

    def test_KeqingIniText_IniFixedWithBodyDressMerged(self):
        self.create()
        tests = [[self._defaultIniTxt, [
"""
[Constants]
global persist $swapvar = 0
global persist $swapscarf = 0
global $active
global $creditinfo = 0

[KeySwap]
condition = $active == 1
key = h
type = cycle
$swapvar = 0,1
$creditinfo = 0

[KeySwapScarf]
condition = $active == 1
key = y
type = cycle
$swapscarf = 0,1
$creditinfo = 0


[Present]
post $active = 0

; Shader ------------------------------

; Overrides ---------------------------

[TextureOverrideKeqingPosition]
hash = 3aaf3e94
run = CommandListKeqingPosition
$active = 1

[TextureOverrideKeqingBlend]
hash = 0bf8e621
run = CommandListKeqingBlend

[TextureOverrideKeqingTexcoord]
hash = 723848fe
run = CommandListKeqingTexcoord

[TextureOverrideKeqingVertexLimitRaise]
hash = 4526145e

[TextureOverrideKeqingIB]
hash = cbf1894b
run = CommandListKeqingIB

[TextureOverrideKeqingHead]
hash = cbf1894b
match_first_index = 0
run = CommandListKeqingHead

[TextureOverrideKeqingBody]
hash = cbf1894b
match_first_index = 10824
run = CommandListKeqingBody

[TextureOverrideKeqingDress]
hash = cbf1894b
match_first_index = 48216
run = CommandListKeqingDress

[TextureOverrideKeqingFaceHeadDiffuse]
hash = d8c9c399
run = CommandListKeqingFaceHeadDiffuse

[TextureOverride41FixVertexLimitRaise]
hash = ccc33b79

; CommandList -------------------------

[CommandListKeqingPosition]
if $swapvar == 0 && $swapscarf == 0
    vb0 = ResourceKeqingPosition0
else if $swapvar == 1 && $swapscarf == 0
    vb0 = ResourceKeqingPosition1
else if $swapvar == 0 && $swapscarf == 1
    vb0 = ResourceKeqingPosition2
else if $swapvar == 1 && $swapscarf == 1
    vb0 = ResourceKeqingPosition3
endif

[CommandListKeqingBlend]
if $swapvar == 0 && $swapscarf == 0
    vb1 = ResourceKeqingBlend0
    handling = skip
    draw = 24194,0
else if $swapvar == 1 && $swapscarf == 0
    vb1 = ResourceKeqingBlend1
    handling = skip
    draw = 24963,0
else if $swapvar == 0 && $swapscarf == 1
    vb1 = ResourceKeqingBlend2
    handling = skip
    draw = 30036,0
else if $swapvar == 1 && $swapscarf == 1
    vb1 = ResourceKeqingBlend3
    handling = skip
    draw = 31265,0
endif

[CommandListKeqingTexcoord]
if $swapvar == 0 && $swapscarf == 0
    vb1 = ResourceKeqingTexcoord0
else if $swapvar == 1 && $swapscarf == 0
    vb1 = ResourceKeqingTexcoord1
else if $swapvar == 0 && $swapscarf == 1
    vb1 = ResourceKeqingTexcoord2
else if $swapvar == 1 && $swapscarf == 1
    vb1 = ResourceKeqingTexcoord3
endif

[CommandListKeqingIB]
if $swapvar == 0 && $swapscarf == 0
    handling = skip
    drawindexed = auto
else if $swapvar == 1 && $swapscarf == 0
    handling = skip
    drawindexed = auto
else if $swapvar == 0 && $swapscarf == 1
    handling = skip
    drawindexed = auto
else if $swapvar == 1 && $swapscarf == 1
    handling = skip
    drawindexed = auto
endif

[CommandListKeqingHead]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingHeadIB0
    ps-t0 = ResourceKeqingHeadDiffuse0
    ps-t1 = ResourceKeqingHeadLightMap0
    ps-t2 = ResourceKeqingHeadMetalMap0
    ps-t3 = ResourceKeqingHeadShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingHeadIB1
    ps-t0 = ResourceKeqingHeadDiffuse1
    ps-t1 = ResourceKeqingHeadLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingHeadIB2
    ps-t0 = ResourceKeqingHeadDiffuse2
    ps-t1 = ResourceKeqingHeadLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingHeadIB3
    ps-t0 = ResourceKeqingHeadDiffuse3
    ps-t1 = ResourceKeqingHeadLightMap3
endif

[CommandListKeqingBody]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingBodyIB0
    ps-t0 = ResourceKeqingBodyDiffuse0
    ps-t1 = ResourceKeqingBodyLightMap0
    ps-t2 = ResourceKeqingBodyMetalMap0
    ps-t3 = ResourceKeqingBodyShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingBodyIB1
    ps-t0 = ResourceKeqingBodyDiffuse1
    ps-t1 = ResourceKeqingBodyLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingBodyIB2
    ps-t0 = ResourceKeqingBodyDiffuse2
    ps-t1 = ResourceKeqingBodyLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingBodyIB3
    ps-t0 = ResourceKeqingBodyDiffuse3
    ps-t1 = ResourceKeqingBodyLightMap3
endif

[CommandListKeqingDress]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingDressIB0
    ps-t0 = ResourceKeqingDressDiffuse0
    ps-t1 = ResourceKeqingDressLightMap0
    ps-t2 = ResourceKeqingDressMetalMap0
    ps-t3 = ResourceKeqingDressShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingDressIB1
    ps-t0 = ResourceKeqingDressDiffuse1
    ps-t1 = ResourceKeqingDressLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingDressIB2
    ps-t0 = ResourceKeqingDressDiffuse2
    ps-t1 = ResourceKeqingDressLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingDressIB3
    ps-t0 = ResourceKeqingDressDiffuse3
    ps-t1 = ResourceKeqingDressLightMap3
endif

[CommandListKeqingFaceHeadDiffuse]
if $swapvar == 0 && $swapscarf == 0
    ps-t0 = ResourceKeqingFaceHeadDiffuse0
else if $swapvar == 1 && $swapscarf == 0
    ps-t0 = ResourceKeqingFaceHeadDiffuse1
else if $swapvar == 0 && $swapscarf == 1
    ps-t0 = ResourceKeqingFaceHeadDiffuse2
else if $swapvar == 1 && $swapscarf == 1
    ps-t0 = ResourceKeqingFaceHeadDiffuse3
endif

; Resources ---------------------------

[ResourceKeqingPosition0]
type = Buffer
stride = 40
filename = 0 - keqing_firstlanternrite\KeqingPosition.buf

[ResourceKeqingBlend0]
type = Buffer
stride = 32
filename = 0 - keqing_firstlanternrite\KeqingBlend.buf

[ResourceKeqingTexcoord0]
type = Buffer
stride = 20
filename = 0 - keqing_firstlanternrite\KeqingTexcoord.buf

[ResourceKeqingHeadIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingHead.ib

[ResourceKeqingBodyIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingBody.ib

[ResourceKeqingDressIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingDress.ib

[ResourceKeqingHeadDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition1]
type = Buffer
stride = 40
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingPosition.buf

[ResourceKeqingBlend1]
type = Buffer
stride = 32
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBlend.buf

[ResourceKeqingTexcoord1]
type = Buffer
stride = 20
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingTexcoord.buf

[ResourceKeqingHeadIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHead.ib

[ResourceKeqingBodyIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBody.ib

[ResourceKeqingDressIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDress.ib

[ResourceKeqingHeadDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition2]
type = Buffer
stride = 40
filename = 2 - keqing_firstlanternrite_scarf\KeqingPosition.buf

[ResourceKeqingBlend2]
type = Buffer
stride = 32
filename = 2 - keqing_firstlanternrite_scarf\KeqingBlend.buf

[ResourceKeqingTexcoord2]
type = Buffer
stride = 20
filename = 2 - keqing_firstlanternrite_scarf\KeqingTexcoord.buf

[ResourceKeqingHeadIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingHead.ib

[ResourceKeqingBodyIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingBody.ib

[ResourceKeqingDressIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingDress.ib

[ResourceKeqingHeadDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition3]
type = Buffer
stride = 40
filename = 3 - keqingfirstlanternrite-shorts\KeqingPosition.buf

[ResourceKeqingBlend3]
type = Buffer
stride = 32
filename = 3 - keqingfirstlanternrite-shorts\KeqingBlend.buf

[ResourceKeqingTexcoord3]
type = Buffer
stride = 20
filename = 3 - keqingfirstlanternrite-shorts\KeqingTexcoord.buf

[ResourceKeqingHeadIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingHead.ib

[ResourceKeqingBodyIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingBody.ib

[ResourceKeqingDressIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingDress.ib

[ResourceKeqingHeadDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingFaceHeadDiffuse.dds



; --------------- Keqing Remap ---------------
; Keqing remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Keqing mods pls give credit for "Nhok0169" and "Albert Gold#2696"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

; ***** KeqingOpulent *****
[TextureOverrideKeqingKeqingOpulentRemapBlend]
hash = 6f010b58
run = CommandListKeqingKeqingOpulentRemapBlend

[CommandListKeqingKeqingOpulentRemapBlend]
if $swapvar == 0 && $swapscarf == 0
\tvb1 = ResourceKeqingKeqingOpulentRemapBlend0
\thandling = skip
\tdraw = 24194,0
else if $swapvar == 1 && $swapscarf == 0
\tvb1 = ResourceKeqingKeqingOpulentRemapBlend1
\thandling = skip
\tdraw = 24963,0
else if $swapvar == 0 && $swapscarf == 1
\tvb1 = ResourceKeqingKeqingOpulentRemapBlend2
\thandling = skip
\tdraw = 30036,0
else if $swapvar == 1 && $swapscarf == 1
\tvb1 = ResourceKeqingKeqingOpulentRemapBlend3
\thandling = skip
\tdraw = 31265,0
endif


[TextureOverrideKeqingPositionKeqingOpulentRemapFix]
hash = 0d7e3cc5
run = CommandListKeqingPositionKeqingOpulentRemapFix
$active = 1

[CommandListKeqingPositionKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tvb0 = ResourceKeqingPosition0
else if $swapvar == 1 && $swapscarf == 0
\tvb0 = ResourceKeqingPosition1
else if $swapvar == 0 && $swapscarf == 1
\tvb0 = ResourceKeqingPosition2
else if $swapvar == 1 && $swapscarf == 1
\tvb0 = ResourceKeqingPosition3
endif

[TextureOverrideKeqingTexcoordKeqingOpulentRemapFix]
hash = 52f78cb7
run = CommandListKeqingTexcoordKeqingOpulentRemapFix

[CommandListKeqingTexcoordKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tvb1 = ResourceKeqingTexcoord0
else if $swapvar == 1 && $swapscarf == 0
\tvb1 = ResourceKeqingTexcoord1
else if $swapvar == 0 && $swapscarf == 1
\tvb1 = ResourceKeqingTexcoord2
else if $swapvar == 1 && $swapscarf == 1
\tvb1 = ResourceKeqingTexcoord3
endif

[TextureOverrideKeqingVertexLimitRaiseKeqingOpulentRemapFix]
hash = 6629a84e

[TextureOverrideKeqingIBKeqingOpulentRemapFix]
hash = 7c6fc8c3
run = CommandListKeqingIBKeqingOpulentRemapFix

[CommandListKeqingIBKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 1 && $swapscarf == 0
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 0 && $swapscarf == 1
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 1 && $swapscarf == 1
\thandling = skip
\tdrawindexed = auto
endif

[TextureOverrideKeqingHeadKeqingOpulentRemapFix]
hash = 7c6fc8c3
match_first_index = 0
run = CommandListKeqingHeadKeqingOpulentRemapFix

[CommandListKeqingHeadKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tib = ResourceKeqingHeadIB0
\tps-t0 = ResourceKeqingHeadDiffuse0
\tps-t1 = ResourceKeqingHeadLightMap0
\tps-t2 = ResourceKeqingHeadMetalMap0
\tps-t3 = ResourceKeqingHeadShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
\tib = ResourceKeqingHeadIB1
\tps-t0 = ResourceKeqingHeadDiffuse1
\tps-t1 = ResourceKeqingHeadLightMap1
else if $swapvar == 0 && $swapscarf == 1
\tib = ResourceKeqingHeadIB2
\tps-t0 = ResourceKeqingHeadDiffuse2
\tps-t1 = ResourceKeqingHeadLightMap2
else if $swapvar == 1 && $swapscarf == 1
\tib = ResourceKeqingHeadIB3
\tps-t0 = ResourceKeqingHeadDiffuse3
\tps-t1 = ResourceKeqingHeadLightMap3
endif

[TextureOverrideKeqingFaceHeadDiffuseKeqingOpulentRemapFix]
hash = c2b17f84
run = CommandListKeqingFaceHeadDiffuseKeqingOpulentRemapFix

[CommandListKeqingFaceHeadDiffuseKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tps-t0 = ResourceKeqingFaceHeadDiffuse0
else if $swapvar == 1 && $swapscarf == 0
\tps-t0 = ResourceKeqingFaceHeadDiffuse1
else if $swapvar == 0 && $swapscarf == 1
\tps-t0 = ResourceKeqingFaceHeadDiffuse2
else if $swapvar == 1 && $swapscarf == 1
\tps-t0 = ResourceKeqingFaceHeadDiffuse3
endif

[TextureOverride41FixVertexLimitRaiseKeqingOpulentRemapFix]
hash = 6629a84e

[TextureOverrideKeqingBodyKeqingOpulentRemapFix]
hash = 7c6fc8c3
match_first_index = 19623
run = CommandListKeqingBodyKeqingOpulentRemapFix

[CommandListKeqingBodyKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tib = ResourceKeqingBodyIB0
\tps-t0 = ResourceKeqingBodyDiffuse0
\tps-t1 = ResourceKeqingBodyLightMap0
\tps-t2 = ResourceKeqingBodyMetalMap0
\tps-t3 = ResourceKeqingBodyShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
\tib = ResourceKeqingBodyIB1
\tps-t0 = ResourceKeqingBodyDiffuse1
\tps-t1 = ResourceKeqingBodyLightMap1
else if $swapvar == 0 && $swapscarf == 1
\tib = ResourceKeqingBodyIB2
\tps-t0 = ResourceKeqingBodyDiffuse2
\tps-t1 = ResourceKeqingBodyLightMap2
else if $swapvar == 1 && $swapscarf == 1
\tib = ResourceKeqingBodyIB3
\tps-t0 = ResourceKeqingBodyDiffuse3
\tps-t1 = ResourceKeqingBodyLightMap3
endif


[ResourceKeqingKeqingOpulentRemapBlend0]
type = Buffer
stride = 32
filename = 0 - keqing_firstlanternrite/KeqingKeqingOpulentRemapBlend.buf

[ResourceKeqingKeqingOpulentRemapBlend1]
type = Buffer
stride = 32
filename = 1 - keqingfirstlanternrite-shorts-noscarf/KeqingKeqingOpulentRemapBlend.buf

[ResourceKeqingKeqingOpulentRemapBlend2]
type = Buffer
stride = 32
filename = 2 - keqing_firstlanternrite_scarf/KeqingKeqingOpulentRemapBlend.buf

[ResourceKeqingKeqingOpulentRemapBlend3]
type = Buffer
stride = 32
filename = 3 - keqingfirstlanternrite-shorts/KeqingKeqingOpulentRemapBlend.buf

; *************************

; --------------------------------------------""", 

"""
[Constants]
global persist $swapvar = 0
global persist $swapscarf = 0
global $active
global $creditinfo = 0

[KeySwap]
condition = $active == 1
key = h
type = cycle
$swapvar = 0,1
$creditinfo = 0

[KeySwapScarf]
condition = $active == 1
key = y
type = cycle
$swapscarf = 0,1
$creditinfo = 0


[Present]
post $active = 0

; Shader ------------------------------

; Overrides ---------------------------

[TextureOverrideKeqingPosition]
hash = 3aaf3e94
run = CommandListKeqingPosition
$active = 1

[TextureOverrideKeqingBlend]
hash = 0bf8e621
run = CommandListKeqingBlend

[TextureOverrideKeqingTexcoord]
hash = 723848fe
run = CommandListKeqingTexcoord

[TextureOverrideKeqingVertexLimitRaise]
hash = 4526145e

[TextureOverrideKeqingIB]
hash = cbf1894b
run = CommandListKeqingIB

[TextureOverrideKeqingHead]
hash = cbf1894b
match_first_index = 0
run = CommandListKeqingHead

[TextureOverrideKeqingBody]
hash = cbf1894b
match_first_index = 10824
run = CommandListKeqingBody

[TextureOverrideKeqingDress]
hash = cbf1894b
match_first_index = 48216
run = CommandListKeqingDress

[TextureOverrideKeqingFaceHeadDiffuse]
hash = d8c9c399
run = CommandListKeqingFaceHeadDiffuse

[TextureOverride41FixVertexLimitRaise]
hash = ccc33b79

; CommandList -------------------------

[CommandListKeqingPosition]
if $swapvar == 0 && $swapscarf == 0
    vb0 = ResourceKeqingPosition0
else if $swapvar == 1 && $swapscarf == 0
    vb0 = ResourceKeqingPosition1
else if $swapvar == 0 && $swapscarf == 1
    vb0 = ResourceKeqingPosition2
else if $swapvar == 1 && $swapscarf == 1
    vb0 = ResourceKeqingPosition3
endif

[CommandListKeqingBlend]
if $swapvar == 0 && $swapscarf == 0
    vb1 = ResourceKeqingBlend0
    handling = skip
    draw = 24194,0
else if $swapvar == 1 && $swapscarf == 0
    vb1 = ResourceKeqingBlend1
    handling = skip
    draw = 24963,0
else if $swapvar == 0 && $swapscarf == 1
    vb1 = ResourceKeqingBlend2
    handling = skip
    draw = 30036,0
else if $swapvar == 1 && $swapscarf == 1
    vb1 = ResourceKeqingBlend3
    handling = skip
    draw = 31265,0
endif

[CommandListKeqingTexcoord]
if $swapvar == 0 && $swapscarf == 0
    vb1 = ResourceKeqingTexcoord0
else if $swapvar == 1 && $swapscarf == 0
    vb1 = ResourceKeqingTexcoord1
else if $swapvar == 0 && $swapscarf == 1
    vb1 = ResourceKeqingTexcoord2
else if $swapvar == 1 && $swapscarf == 1
    vb1 = ResourceKeqingTexcoord3
endif

[CommandListKeqingIB]
if $swapvar == 0 && $swapscarf == 0
    handling = skip
    drawindexed = auto
else if $swapvar == 1 && $swapscarf == 0
    handling = skip
    drawindexed = auto
else if $swapvar == 0 && $swapscarf == 1
    handling = skip
    drawindexed = auto
else if $swapvar == 1 && $swapscarf == 1
    handling = skip
    drawindexed = auto
endif

[CommandListKeqingHead]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingHeadIB0
    ps-t0 = ResourceKeqingHeadDiffuse0
    ps-t1 = ResourceKeqingHeadLightMap0
    ps-t2 = ResourceKeqingHeadMetalMap0
    ps-t3 = ResourceKeqingHeadShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingHeadIB1
    ps-t0 = ResourceKeqingHeadDiffuse1
    ps-t1 = ResourceKeqingHeadLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingHeadIB2
    ps-t0 = ResourceKeqingHeadDiffuse2
    ps-t1 = ResourceKeqingHeadLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingHeadIB3
    ps-t0 = ResourceKeqingHeadDiffuse3
    ps-t1 = ResourceKeqingHeadLightMap3
endif

[CommandListKeqingBody]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingBodyIB0
    ps-t0 = ResourceKeqingBodyDiffuse0
    ps-t1 = ResourceKeqingBodyLightMap0
    ps-t2 = ResourceKeqingBodyMetalMap0
    ps-t3 = ResourceKeqingBodyShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingBodyIB1
    ps-t0 = ResourceKeqingBodyDiffuse1
    ps-t1 = ResourceKeqingBodyLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingBodyIB2
    ps-t0 = ResourceKeqingBodyDiffuse2
    ps-t1 = ResourceKeqingBodyLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingBodyIB3
    ps-t0 = ResourceKeqingBodyDiffuse3
    ps-t1 = ResourceKeqingBodyLightMap3
endif

[CommandListKeqingDress]
if $swapvar == 0 && $swapscarf == 0
    ib = ResourceKeqingDressIB0
    ps-t0 = ResourceKeqingDressDiffuse0
    ps-t1 = ResourceKeqingDressLightMap0
    ps-t2 = ResourceKeqingDressMetalMap0
    ps-t3 = ResourceKeqingDressShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
    ib = ResourceKeqingDressIB1
    ps-t0 = ResourceKeqingDressDiffuse1
    ps-t1 = ResourceKeqingDressLightMap1
else if $swapvar == 0 && $swapscarf == 1
    ib = ResourceKeqingDressIB2
    ps-t0 = ResourceKeqingDressDiffuse2
    ps-t1 = ResourceKeqingDressLightMap2
else if $swapvar == 1 && $swapscarf == 1
    ib = ResourceKeqingDressIB3
    ps-t0 = ResourceKeqingDressDiffuse3
    ps-t1 = ResourceKeqingDressLightMap3
endif

[CommandListKeqingFaceHeadDiffuse]
if $swapvar == 0 && $swapscarf == 0
    ps-t0 = ResourceKeqingFaceHeadDiffuse0
else if $swapvar == 1 && $swapscarf == 0
    ps-t0 = ResourceKeqingFaceHeadDiffuse1
else if $swapvar == 0 && $swapscarf == 1
    ps-t0 = ResourceKeqingFaceHeadDiffuse2
else if $swapvar == 1 && $swapscarf == 1
    ps-t0 = ResourceKeqingFaceHeadDiffuse3
endif

; Resources ---------------------------

[ResourceKeqingPosition0]
type = Buffer
stride = 40
filename = 0 - keqing_firstlanternrite\KeqingPosition.buf

[ResourceKeqingBlend0]
type = Buffer
stride = 32
filename = 0 - keqing_firstlanternrite\KeqingBlend.buf

[ResourceKeqingTexcoord0]
type = Buffer
stride = 20
filename = 0 - keqing_firstlanternrite\KeqingTexcoord.buf

[ResourceKeqingHeadIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingHead.ib

[ResourceKeqingBodyIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingBody.ib

[ResourceKeqingDressIB0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - keqing_firstlanternrite\KeqingDress.ib

[ResourceKeqingHeadDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap0]
filename = 0 - keqing_firstlanternrite\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap0]
filename = 0 - keqing_firstlanternrite\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp0]
filename = 0 - keqing_firstlanternrite\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse0]
filename = 0 - keqing_firstlanternrite\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition1]
type = Buffer
stride = 40
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingPosition.buf

[ResourceKeqingBlend1]
type = Buffer
stride = 32
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBlend.buf

[ResourceKeqingTexcoord1]
type = Buffer
stride = 20
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingTexcoord.buf

[ResourceKeqingHeadIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHead.ib

[ResourceKeqingBodyIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBody.ib

[ResourceKeqingDressIB1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDress.ib

[ResourceKeqingHeadDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse1]
filename = 1 - keqingfirstlanternrite-shorts-noscarf\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition2]
type = Buffer
stride = 40
filename = 2 - keqing_firstlanternrite_scarf\KeqingPosition.buf

[ResourceKeqingBlend2]
type = Buffer
stride = 32
filename = 2 - keqing_firstlanternrite_scarf\KeqingBlend.buf

[ResourceKeqingTexcoord2]
type = Buffer
stride = 20
filename = 2 - keqing_firstlanternrite_scarf\KeqingTexcoord.buf

[ResourceKeqingHeadIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingHead.ib

[ResourceKeqingBodyIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingBody.ib

[ResourceKeqingDressIB2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 2 - keqing_firstlanternrite_scarf\KeqingDress.ib

[ResourceKeqingHeadDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse2]
filename = 2 - keqing_firstlanternrite_scarf\KeqingFaceHeadDiffuse.dds

[ResourceKeqingPosition3]
type = Buffer
stride = 40
filename = 3 - keqingfirstlanternrite-shorts\KeqingPosition.buf

[ResourceKeqingBlend3]
type = Buffer
stride = 32
filename = 3 - keqingfirstlanternrite-shorts\KeqingBlend.buf

[ResourceKeqingTexcoord3]
type = Buffer
stride = 20
filename = 3 - keqingfirstlanternrite-shorts\KeqingTexcoord.buf

[ResourceKeqingHeadIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingHead.ib

[ResourceKeqingBodyIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingBody.ib

[ResourceKeqingDressIB3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - keqingfirstlanternrite-shorts\KeqingDress.ib

[ResourceKeqingHeadDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadDiffuse.dds

[ResourceKeqingHeadLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadLightMap.dds

[ResourceKeqingHeadMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadMetalMap.dds

[ResourceKeqingHeadShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingHeadShadowRamp.jpg

[ResourceKeqingBodyDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyDiffuse.dds

[ResourceKeqingBodyLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyLightMap.dds

[ResourceKeqingBodyMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyMetalMap.dds

[ResourceKeqingBodyShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingBodyShadowRamp.jpg

[ResourceKeqingDressDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressDiffuse.dds

[ResourceKeqingDressLightMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressLightMap.dds

[ResourceKeqingDressMetalMap3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressMetalMap.dds

[ResourceKeqingDressShadowRamp3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingDressShadowRamp.jpg

[ResourceKeqingFaceHeadDiffuse3]
filename = 3 - keqingfirstlanternrite-shorts\KeqingFaceHeadDiffuse.dds



; --------------- Keqing Remap ---------------
; Keqing remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Keqing mods pls give credit for "Nhok0169" and "Albert Gold#2696"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

; ***** KeqingOpulent *****
[TextureOverrideKeqingKeqingOpulentRemapBlend]
hash = 6f010b58
run = CommandListKeqingKeqingOpulentRemapBlend

[CommandListKeqingKeqingOpulentRemapBlend]
if $swapvar == 0 && $swapscarf == 0
\tvb1 = ResourceKeqingKeqingOpulentRemapBlend0
\thandling = skip
\tdraw = 24194,0
else if $swapvar == 1 && $swapscarf == 0
\tvb1 = ResourceKeqingKeqingOpulentRemapBlend1
\thandling = skip
\tdraw = 24963,0
else if $swapvar == 0 && $swapscarf == 1
\tvb1 = ResourceKeqingKeqingOpulentRemapBlend2
\thandling = skip
\tdraw = 30036,0
else if $swapvar == 1 && $swapscarf == 1
\tvb1 = ResourceKeqingKeqingOpulentRemapBlend3
\thandling = skip
\tdraw = 31265,0
endif


[TextureOverrideKeqingPositionKeqingOpulentRemapFix]
hash = 0d7e3cc5
run = CommandListKeqingPositionKeqingOpulentRemapFix
$active = 1

[CommandListKeqingPositionKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tvb0 = ResourceKeqingPosition0
else if $swapvar == 1 && $swapscarf == 0
\tvb0 = ResourceKeqingPosition1
else if $swapvar == 0 && $swapscarf == 1
\tvb0 = ResourceKeqingPosition2
else if $swapvar == 1 && $swapscarf == 1
\tvb0 = ResourceKeqingPosition3
endif

[TextureOverrideKeqingTexcoordKeqingOpulentRemapFix]
hash = 52f78cb7
run = CommandListKeqingTexcoordKeqingOpulentRemapFix

[CommandListKeqingTexcoordKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tvb1 = ResourceKeqingTexcoord0
else if $swapvar == 1 && $swapscarf == 0
\tvb1 = ResourceKeqingTexcoord1
else if $swapvar == 0 && $swapscarf == 1
\tvb1 = ResourceKeqingTexcoord2
else if $swapvar == 1 && $swapscarf == 1
\tvb1 = ResourceKeqingTexcoord3
endif

[TextureOverrideKeqingVertexLimitRaiseKeqingOpulentRemapFix]
hash = 6629a84e

[TextureOverrideKeqingIBKeqingOpulentRemapFix]
hash = 7c6fc8c3
run = CommandListKeqingIBKeqingOpulentRemapFix

[CommandListKeqingIBKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 1 && $swapscarf == 0
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 0 && $swapscarf == 1
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 1 && $swapscarf == 1
\thandling = skip
\tdrawindexed = auto
endif

[TextureOverrideKeqingHeadKeqingOpulentRemapFix]
hash = 7c6fc8c3
match_first_index = 0
run = CommandListKeqingHeadKeqingOpulentRemapFix

[CommandListKeqingHeadKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tib = ResourceKeqingHeadIB0
\tps-t0 = ResourceKeqingHeadDiffuse0
\tps-t1 = ResourceKeqingHeadLightMap0
\tps-t2 = ResourceKeqingHeadMetalMap0
\tps-t3 = ResourceKeqingHeadShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
\tib = ResourceKeqingHeadIB1
\tps-t0 = ResourceKeqingHeadDiffuse1
\tps-t1 = ResourceKeqingHeadLightMap1
else if $swapvar == 0 && $swapscarf == 1
\tib = ResourceKeqingHeadIB2
\tps-t0 = ResourceKeqingHeadDiffuse2
\tps-t1 = ResourceKeqingHeadLightMap2
else if $swapvar == 1 && $swapscarf == 1
\tib = ResourceKeqingHeadIB3
\tps-t0 = ResourceKeqingHeadDiffuse3
\tps-t1 = ResourceKeqingHeadLightMap3
endif

[TextureOverrideKeqingFaceHeadDiffuseKeqingOpulentRemapFix]
hash = c2b17f84
run = CommandListKeqingFaceHeadDiffuseKeqingOpulentRemapFix

[CommandListKeqingFaceHeadDiffuseKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tps-t0 = ResourceKeqingFaceHeadDiffuse0
else if $swapvar == 1 && $swapscarf == 0
\tps-t0 = ResourceKeqingFaceHeadDiffuse1
else if $swapvar == 0 && $swapscarf == 1
\tps-t0 = ResourceKeqingFaceHeadDiffuse2
else if $swapvar == 1 && $swapscarf == 1
\tps-t0 = ResourceKeqingFaceHeadDiffuse3
endif

[TextureOverride41FixVertexLimitRaiseKeqingOpulentRemapFix]
hash = 6629a84e

[TextureOverrideKeqingBodyKeqingOpulentRemapFix]
hash = 7c6fc8c3
match_first_index = 19623
run = CommandListKeqingBodyKeqingOpulentRemapFix

[CommandListKeqingBodyKeqingOpulentRemapFix]
if $swapvar == 0 && $swapscarf == 0
\tib = ResourceKeqingDressIB0
\tps-t0 = ResourceKeqingDressDiffuse0
\tps-t1 = ResourceKeqingDressLightMap0
\tps-t2 = ResourceKeqingDressMetalMap0
\tps-t3 = ResourceKeqingDressShadowRamp0
else if $swapvar == 1 && $swapscarf == 0
\tib = ResourceKeqingDressIB1
\tps-t0 = ResourceKeqingDressDiffuse1
\tps-t1 = ResourceKeqingDressLightMap1
else if $swapvar == 0 && $swapscarf == 1
\tib = ResourceKeqingDressIB2
\tps-t0 = ResourceKeqingDressDiffuse2
\tps-t1 = ResourceKeqingDressLightMap2
else if $swapvar == 1 && $swapscarf == 1
\tib = ResourceKeqingDressIB3
\tps-t0 = ResourceKeqingDressDiffuse3
\tps-t1 = ResourceKeqingDressLightMap3
endif


[ResourceKeqingKeqingOpulentRemapBlend0]
type = Buffer
stride = 32
filename = 0 - keqing_firstlanternrite/KeqingKeqingOpulentRemapBlend.buf

[ResourceKeqingKeqingOpulentRemapBlend1]
type = Buffer
stride = 32
filename = 1 - keqingfirstlanternrite-shorts-noscarf/KeqingKeqingOpulentRemapBlend.buf

[ResourceKeqingKeqingOpulentRemapBlend2]
type = Buffer
stride = 32
filename = 2 - keqing_firstlanternrite_scarf/KeqingKeqingOpulentRemapBlend.buf

[ResourceKeqingKeqingOpulentRemapBlend3]
type = Buffer
stride = 32
filename = 3 - keqingfirstlanternrite-shorts/KeqingKeqingOpulentRemapBlend.buf

; *************************

; --------------------------------------------"""]]]

        for test in tests:
            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer
            self._iniFile.fileTxt = test[0]
            self._iniFile.parse()

            result = self._fixer.fix()
            expected = test[1]
            expectedIniLen = len(expected)

            if (expectedIniLen == 1):
                self.assertIsInstance(result, str)
                result = [result]
            else:
                self.assertIsInstance(result, list)

            self.assertEqual(len(result), expectedIniLen)
            for i in range(expectedIniLen):
                self.assertEqual(result[i], expected[i])

    # ====================================================================
