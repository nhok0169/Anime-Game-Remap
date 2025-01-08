import sys

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class MultiModFixersTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._modTypes.add(FRB.ModTypes.Jean.value)

        cls._defaultIniTxt = r"""
[Constants]
global persist $swapvar = 0
global persist $th = 0
global persist $dd = 1
global persist $bl = 0
global $active

[KeySwapTH]
condition = $active == 1
key = h
type = cycle
$th = 0,1

[KeySwapDD]
condition = $active == 1
key = y
type = cycle
$dd = 0,1

[KeySwapBL]
condition = $active == 1
key = n
type = cycle
$bl = 0,1

[Present]
post $active = 0

if $th == 0 && $dd == 1
	$swapvar = 0
else if $th == 1 && $dd == 1
	$swapvar = 1
else if $th == 0 && $dd == 0
	$swapvar = 2
else if $th == 1 && $dd == 0
	$swapvar = 3
endif

; Shader ------------------------------

; Overrides ---------------------------

[TextureOverrideJeanPosition]
hash = 191af650
run = CommandListJeanPosition
$active = 1

[TextureOverrideJeanBlend]
hash = 3cb8153c
run = CommandListJeanBlend

[TextureOverrideJeanTexcoord]
hash = 1722136c
run = CommandListJeanTexcoord

[TextureOverrideJeanVertexLimitRaise]
hash = 6fe07e12

[TextureOverrideJeanIB]
hash = 115737ff
run = CommandListJeanIB

[TextureOverrideJeanHead]
hash = 115737ff
match_first_index = 0
run = CommandListJeanHead

[TextureOverrideJeanBody]
hash = 115737ff
match_first_index = 7779
run = CommandListJeanBody

[TextureOverrideJeanFaceHeadDiffuse]
hash = c2d1a57e
run = CommandListJeanFaceHeadDiffuse

; CommandList -------------------------

[CommandListJeanPosition]
if $swapvar == 0
	vb0 = ResourceJeanPosition.0
else if $swapvar == 1
	vb0 = ResourceJeanPosition.1
else if $swapvar == 2
	vb0 = ResourceJeanPosition.2
else if $swapvar == 3
	vb0 = ResourceJeanPosition.3
endif

[CommandListJeanBlend]
if $swapvar == 0
	vb1 = ResourceJeanBlend.0
	handling = skip
	draw = 11232,0
else if $swapvar == 1
	vb1 = ResourceJeanBlend.1
	handling = skip
	draw = 11232,0
else if $swapvar == 2
	vb1 = ResourceJeanBlend.2
	handling = skip
	draw = 10320,0
else if $swapvar == 3
	vb1 = ResourceJeanBlend.3
	handling = skip
	draw = 10320,0
endif

[CommandListJeanTexcoord]
if $swapvar == 0
	vb1 = ResourceJeanTexcoord.0
else if $swapvar == 1
	vb1 = ResourceJeanTexcoord.1
else if $swapvar == 2
	vb1 = ResourceJeanTexcoord.2
else if $swapvar == 3
	vb1 = ResourceJeanTexcoord.3
endif

[CommandListJeanIB]
if $swapvar == 0
	handling = skip
	drawindexed = auto
else if $swapvar == 1
	handling = skip
	drawindexed = auto
else if $swapvar == 2
	handling = skip
	drawindexed = auto
else if $swapvar == 3
	handling = skip
	drawindexed = auto
endif

[CommandListJeanHead]
if $swapvar == 0
	ib = ResourceJeanHeadIB.0
	ps-t0 = ResourceJeanHeadDiffuse.0
	ps-t1 = ResourceJeanHeadLightMap.0
else if $swapvar == 1
	ib = ResourceJeanHeadIB.1
	ps-t0 = ResourceJeanHeadDiffuse.1
	ps-t1 = ResourceJeanHeadLightMap.1
else if $swapvar == 2
	ib = ResourceJeanHeadIB.2
	ps-t0 = ResourceJeanHeadDiffuse.2
	ps-t1 = ResourceJeanHeadLightMap.2
else if $swapvar == 3
	ib = ResourceJeanHeadIB.3
	ps-t0 = ResourceJeanHeadDiffuse.3
	ps-t1 = ResourceJeanHeadLightMap.3
endif

[CommandListJeanBody]
if $swapvar == 0 && $bl == 0
	ib = ResourceJeanBodyIB.0
	ps-t0 = ResourceJeanBodyDiffuse.0
	ps-t1 = ResourceJeanBodyLightMap.0
else if $swapvar == 0 && $bl == 1
	ib = ResourceJeanBodyIB.0
	ps-t0 = ResourceJeanBodyDiffuse.01
	ps-t1 = ResourceJeanBodyLightMap.01
else if $swapvar == 1 && $bl == 0
	ib = ResourceJeanBodyIB.1
	ps-t0 = ResourceJeanBodyDiffuse.1
	ps-t1 = ResourceJeanBodyLightMap.1
else if $swapvar == 1 && $bl == 1
	ib = ResourceJeanBodyIB.1
	ps-t0 = ResourceJeanBodyDiffuse.11
	ps-t1 = ResourceJeanBodyLightMap.11
else if $swapvar == 2 && $bl == 0
	ib = ResourceJeanBodyIB.2
	ps-t0 = ResourceJeanBodyDiffuse.2
	ps-t1 = ResourceJeanBodyLightMap.2
else if $swapvar == 2 && $bl == 1
	ib = ResourceJeanBodyIB.2
	ps-t0 = ResourceJeanBodyDiffuse.21
	ps-t1 = ResourceJeanBodyLightMap.21
else if $swapvar == 3 && $bl == 0
	ib = ResourceJeanBodyIB.3
	ps-t0 = ResourceJeanBodyDiffuse.3
	ps-t1 = ResourceJeanBodyLightMap.3
else if $swapvar == 3 && $bl == 1
	ib = ResourceJeanBodyIB.3
	ps-t0 = ResourceJeanBodyDiffuse.31
	ps-t1 = ResourceJeanBodyLightMap.31
endif

[CommandListJeanFaceHeadDiffuse]
if $swapvar == 0
	ps-t0 = ResourceJeanFaceHeadDiffuse.0
else if $swapvar == 1
	ps-t0 = ResourceJeanFaceHeadDiffuse.1
else if $swapvar == 2
	ps-t0 = ResourceJeanFaceHeadDiffuse.2
else if $swapvar == 3
	ps-t0 = ResourceJeanFaceHeadDiffuse.3
endif

; Resources ---------------------------

[ResourceJeanPosition.0]
type = Buffer
stride = 40
filename = 0 - Jean DUDU\JeanPosition.buf

[ResourceJeanBlend.0]
type = Buffer
stride = 32
filename = 0 - Jean DUDU\JeanBlend.buf

[ResourceJeanTexcoord.0]
type = Buffer
stride = 12
filename = 0 - Jean DUDU\JeanTexcoord.buf

[ResourceJeanHeadIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - Jean DUDU\JeanHead.ib

[ResourceJeanBodyIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - Jean DUDU\JeanBody.ib

[ResourceJeanHeadDiffuse.0]
filename = 0 - Jean DUDU\JeanHeadDiffuse.dds

[ResourceJeanHeadLightMap.0]
filename = 0 - Jean DUDU\JeanHeadLightMap.dds

[ResourceJeanBodyDiffuse.0]
filename = 0 - Jean DUDU\JeanBodyDiffuse.dds

[ResourceJeanBodyDiffuse.01]
filename = 0 - Jean DUDU\JeanBodyDiffuseNTH.dds

[ResourceJeanBodyLightMap.0]
filename = 0 - Jean DUDU\JeanBodyLightMap.dds

[ResourceJeanBodyLightMap.01]
filename = 0 - Jean DUDU\JeanBodyLightMapNTH.dds

[ResourceJeanFaceHeadDiffuse.0]
filename = 0 - Jean DUDU\JeanFaceHeadDiffuse.dds

[ResourceJeanPosition.1]
type = Buffer
stride = 40
filename = 1 - Jean DUDU TH\JeanPosition.buf

[ResourceJeanBlend.1]
type = Buffer
stride = 32
filename = 1 - Jean DUDU TH\JeanBlend.buf

[ResourceJeanTexcoord.1]
type = Buffer
stride = 12
filename = 1 - Jean DUDU TH\JeanTexcoord.buf

[ResourceJeanHeadIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - Jean DUDU TH\JeanHead.ib

[ResourceJeanBodyIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - Jean DUDU TH\JeanBody.ib

[ResourceJeanHeadDiffuse.1]
filename = 1 - Jean DUDU TH\JeanHeadDiffuse.dds

[ResourceJeanHeadLightMap.1]
filename = 1 - Jean DUDU TH\JeanHeadLightMap.dds

[ResourceJeanBodyDiffuse.1]
filename = 1 - Jean DUDU TH\JeanBodyDiffuse.dds

[ResourceJeanBodyDiffuse.11]
filename = 1 - Jean DUDU TH\JeanBodyDiffuseNTH.dds

[ResourceJeanBodyLightMap.1]
filename = 1 - Jean DUDU TH\JeanBodyLightMap.dds

[ResourceJeanBodyLightMap.11]
filename = 1 - Jean DUDU TH\JeanBodyLightMapNTH.dds

[ResourceJeanFaceHeadDiffuse.1]
filename = 1 - Jean DUDU TH\JeanFaceHeadDiffuse.dds

[ResourceJeanPosition.2]
type = Buffer
stride = 40
filename = 3 - Jean\JeanPosition.buf

[ResourceJeanBlend.2]
type = Buffer
stride = 32
filename = 3 - Jean\JeanBlend.buf

[ResourceJeanTexcoord.2]
type = Buffer
stride = 12
filename = 3 - Jean\JeanTexcoord.buf

[ResourceJeanHeadIB.2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - Jean\JeanHead.ib

[ResourceJeanBodyIB.2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - Jean\JeanBody.ib

[ResourceJeanHeadDiffuse.2]
filename = 3 - Jean\JeanHeadDiffuse.dds

[ResourceJeanHeadLightMap.2]
filename = 3 - Jean\JeanHeadLightMap.dds

[ResourceJeanBodyDiffuse.2]
filename = 3 - Jean\JeanBodyDiffuse.dds

[ResourceJeanBodyDiffuse.21]
filename = 3 - Jean\JeanBodyDiffuseNTH.dds

[ResourceJeanBodyLightMap.2]
filename = 3 - Jean\JeanBodyLightMap.dds

[ResourceJeanBodyLightMap.21]
filename = 3 - Jean\JeanBodyLightMapNTH.dds

[ResourceJeanFaceHeadDiffuse.2]
filename = 3 - Jean\JeanFaceHeadDiffuse.dds

[ResourceJeanPosition.3]
type = Buffer
stride = 40
filename = 4 - Jean TH\JeanPosition.buf

[ResourceJeanBlend.3]
type = Buffer
stride = 32
filename = 4 - Jean TH\JeanBlend.buf

[ResourceJeanTexcoord.3]
type = Buffer
stride = 12
filename = 4 - Jean TH\JeanTexcoord.buf

[ResourceJeanHeadIB.3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 4 - Jean TH\JeanHead.ib

[ResourceJeanBodyIB.3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 4 - Jean TH\JeanBody.ib

[ResourceJeanHeadDiffuse.3]
filename = 4 - Jean TH\JeanHeadDiffuse.dds

[ResourceJeanHeadLightMap.3]
filename = 4 - Jean TH\JeanHeadLightMap.dds

[ResourceJeanBodyDiffuse.3]
filename = 4 - Jean TH\JeanBodyDiffuse.dds

[ResourceJeanBodyDiffuse.31]
filename = 4 - Jean TH\JeanBodyDiffuseNTH.dds

[ResourceJeanBodyLightMap.3]
filename = 4 - Jean TH\JeanBodyLightMap.dds

[ResourceJeanBodyLightMap.31]
filename = 4 - Jean TH\JeanBodyLightMapNTH.dds

[ResourceJeanFaceHeadDiffuse.3]
filename = 4 - Jean TH\JeanFaceHeadDiffuse.dds"""
        
        cls._iniTxtLines = []
        cls.setupIniTxt(cls._defaultIniTxt)

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIObjParser(self._iniFile, {"body"})

    def createFixer(self):
        self._fixer = FRB.MultiModFixer(self._parser, {"JeanCN": FRB.IniFixBuilder(FRB.GIMIFixer), "JeanSea": FRB.IniFixBuilder(FRB.GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}])})

    def create(self):
        self.createIniFile()
        self.createParser()
        self.createFixer()
        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer


    # ========================= fix ======================================

    def test_JeanIniTxt_IniFixedForJeanCNAndJeanSea(self):
        self.create()
        tests = [[self._defaultIniTxt, ["""
[Constants]
global persist $swapvar = 0
global persist $th = 0
global persist $dd = 1
global persist $bl = 0
global $active

[KeySwapTH]
condition = $active == 1
key = h
type = cycle
$th = 0,1

[KeySwapDD]
condition = $active == 1
key = y
type = cycle
$dd = 0,1

[KeySwapBL]
condition = $active == 1
key = n
type = cycle
$bl = 0,1

[Present]
post $active = 0

if $th == 0 && $dd == 1
\t$swapvar = 0
else if $th == 1 && $dd == 1
\t$swapvar = 1
else if $th == 0 && $dd == 0
\t$swapvar = 2
else if $th == 1 && $dd == 0
\t$swapvar = 3
endif

; Shader ------------------------------

; Overrides ---------------------------

[TextureOverrideJeanPosition]
hash = 191af650
run = CommandListJeanPosition
$active = 1

[TextureOverrideJeanBlend]
hash = 3cb8153c
run = CommandListJeanBlend

[TextureOverrideJeanTexcoord]
hash = 1722136c
run = CommandListJeanTexcoord

[TextureOverrideJeanVertexLimitRaise]
hash = 6fe07e12

[TextureOverrideJeanIB]
hash = 115737ff
run = CommandListJeanIB

[TextureOverrideJeanHead]
hash = 115737ff
match_first_index = 0
run = CommandListJeanHead

[TextureOverrideJeanBody]
hash = 115737ff
match_first_index = 7779
run = CommandListJeanBody

[TextureOverrideJeanFaceHeadDiffuse]
hash = c2d1a57e
run = CommandListJeanFaceHeadDiffuse

; CommandList -------------------------

[CommandListJeanPosition]
if $swapvar == 0
\tvb0 = ResourceJeanPosition.0
else if $swapvar == 1
\tvb0 = ResourceJeanPosition.1
else if $swapvar == 2
\tvb0 = ResourceJeanPosition.2
else if $swapvar == 3
\tvb0 = ResourceJeanPosition.3
endif

[CommandListJeanBlend]
if $swapvar == 0
\tvb1 = ResourceJeanBlend.0
\thandling = skip
\tdraw = 11232,0
else if $swapvar == 1
\tvb1 = ResourceJeanBlend.1
\thandling = skip
\tdraw = 11232,0
else if $swapvar == 2
\tvb1 = ResourceJeanBlend.2
\thandling = skip
\tdraw = 10320,0
else if $swapvar == 3
\tvb1 = ResourceJeanBlend.3
\thandling = skip
\tdraw = 10320,0
endif

[CommandListJeanTexcoord]
if $swapvar == 0
\tvb1 = ResourceJeanTexcoord.0
else if $swapvar == 1
\tvb1 = ResourceJeanTexcoord.1
else if $swapvar == 2
\tvb1 = ResourceJeanTexcoord.2
else if $swapvar == 3
\tvb1 = ResourceJeanTexcoord.3
endif

[CommandListJeanIB]
if $swapvar == 0
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 1
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 2
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 3
\thandling = skip
\tdrawindexed = auto
endif

[CommandListJeanHead]
if $swapvar == 0
\tib = ResourceJeanHeadIB.0
\tps-t0 = ResourceJeanHeadDiffuse.0
\tps-t1 = ResourceJeanHeadLightMap.0
else if $swapvar == 1
\tib = ResourceJeanHeadIB.1
\tps-t0 = ResourceJeanHeadDiffuse.1
\tps-t1 = ResourceJeanHeadLightMap.1
else if $swapvar == 2
\tib = ResourceJeanHeadIB.2
\tps-t0 = ResourceJeanHeadDiffuse.2
\tps-t1 = ResourceJeanHeadLightMap.2
else if $swapvar == 3
\tib = ResourceJeanHeadIB.3
\tps-t0 = ResourceJeanHeadDiffuse.3
\tps-t1 = ResourceJeanHeadLightMap.3
endif

[CommandListJeanBody]
if $swapvar == 0 && $bl == 0
\tib = ResourceJeanBodyIB.0
\tps-t0 = ResourceJeanBodyDiffuse.0
\tps-t1 = ResourceJeanBodyLightMap.0
else if $swapvar == 0 && $bl == 1
\tib = ResourceJeanBodyIB.0
\tps-t0 = ResourceJeanBodyDiffuse.01
\tps-t1 = ResourceJeanBodyLightMap.01
else if $swapvar == 1 && $bl == 0
\tib = ResourceJeanBodyIB.1
\tps-t0 = ResourceJeanBodyDiffuse.1
\tps-t1 = ResourceJeanBodyLightMap.1
else if $swapvar == 1 && $bl == 1
\tib = ResourceJeanBodyIB.1
\tps-t0 = ResourceJeanBodyDiffuse.11
\tps-t1 = ResourceJeanBodyLightMap.11
else if $swapvar == 2 && $bl == 0
\tib = ResourceJeanBodyIB.2
\tps-t0 = ResourceJeanBodyDiffuse.2
\tps-t1 = ResourceJeanBodyLightMap.2
else if $swapvar == 2 && $bl == 1
\tib = ResourceJeanBodyIB.2
\tps-t0 = ResourceJeanBodyDiffuse.21
\tps-t1 = ResourceJeanBodyLightMap.21
else if $swapvar == 3 && $bl == 0
\tib = ResourceJeanBodyIB.3
\tps-t0 = ResourceJeanBodyDiffuse.3
\tps-t1 = ResourceJeanBodyLightMap.3
else if $swapvar == 3 && $bl == 1
\tib = ResourceJeanBodyIB.3
\tps-t0 = ResourceJeanBodyDiffuse.31
\tps-t1 = ResourceJeanBodyLightMap.31
endif

[CommandListJeanFaceHeadDiffuse]
if $swapvar == 0
\tps-t0 = ResourceJeanFaceHeadDiffuse.0
else if $swapvar == 1
\tps-t0 = ResourceJeanFaceHeadDiffuse.1
else if $swapvar == 2
\tps-t0 = ResourceJeanFaceHeadDiffuse.2
else if $swapvar == 3
\tps-t0 = ResourceJeanFaceHeadDiffuse.3
endif

; Resources ---------------------------

[ResourceJeanPosition.0]
type = Buffer
stride = 40
filename = 0 - Jean DUDU\JeanPosition.buf

[ResourceJeanBlend.0]
type = Buffer
stride = 32
filename = 0 - Jean DUDU\JeanBlend.buf

[ResourceJeanTexcoord.0]
type = Buffer
stride = 12
filename = 0 - Jean DUDU\JeanTexcoord.buf

[ResourceJeanHeadIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - Jean DUDU\JeanHead.ib

[ResourceJeanBodyIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 0 - Jean DUDU\JeanBody.ib

[ResourceJeanHeadDiffuse.0]
filename = 0 - Jean DUDU\JeanHeadDiffuse.dds

[ResourceJeanHeadLightMap.0]
filename = 0 - Jean DUDU\JeanHeadLightMap.dds

[ResourceJeanBodyDiffuse.0]
filename = 0 - Jean DUDU\JeanBodyDiffuse.dds

[ResourceJeanBodyDiffuse.01]
filename = 0 - Jean DUDU\JeanBodyDiffuseNTH.dds

[ResourceJeanBodyLightMap.0]
filename = 0 - Jean DUDU\JeanBodyLightMap.dds

[ResourceJeanBodyLightMap.01]
filename = 0 - Jean DUDU\JeanBodyLightMapNTH.dds

[ResourceJeanFaceHeadDiffuse.0]
filename = 0 - Jean DUDU\JeanFaceHeadDiffuse.dds

[ResourceJeanPosition.1]
type = Buffer
stride = 40
filename = 1 - Jean DUDU TH\JeanPosition.buf

[ResourceJeanBlend.1]
type = Buffer
stride = 32
filename = 1 - Jean DUDU TH\JeanBlend.buf

[ResourceJeanTexcoord.1]
type = Buffer
stride = 12
filename = 1 - Jean DUDU TH\JeanTexcoord.buf

[ResourceJeanHeadIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - Jean DUDU TH\JeanHead.ib

[ResourceJeanBodyIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 1 - Jean DUDU TH\JeanBody.ib

[ResourceJeanHeadDiffuse.1]
filename = 1 - Jean DUDU TH\JeanHeadDiffuse.dds

[ResourceJeanHeadLightMap.1]
filename = 1 - Jean DUDU TH\JeanHeadLightMap.dds

[ResourceJeanBodyDiffuse.1]
filename = 1 - Jean DUDU TH\JeanBodyDiffuse.dds

[ResourceJeanBodyDiffuse.11]
filename = 1 - Jean DUDU TH\JeanBodyDiffuseNTH.dds

[ResourceJeanBodyLightMap.1]
filename = 1 - Jean DUDU TH\JeanBodyLightMap.dds

[ResourceJeanBodyLightMap.11]
filename = 1 - Jean DUDU TH\JeanBodyLightMapNTH.dds

[ResourceJeanFaceHeadDiffuse.1]
filename = 1 - Jean DUDU TH\JeanFaceHeadDiffuse.dds

[ResourceJeanPosition.2]
type = Buffer
stride = 40
filename = 3 - Jean\JeanPosition.buf

[ResourceJeanBlend.2]
type = Buffer
stride = 32
filename = 3 - Jean\JeanBlend.buf

[ResourceJeanTexcoord.2]
type = Buffer
stride = 12
filename = 3 - Jean\JeanTexcoord.buf

[ResourceJeanHeadIB.2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - Jean\JeanHead.ib

[ResourceJeanBodyIB.2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 3 - Jean\JeanBody.ib

[ResourceJeanHeadDiffuse.2]
filename = 3 - Jean\JeanHeadDiffuse.dds

[ResourceJeanHeadLightMap.2]
filename = 3 - Jean\JeanHeadLightMap.dds

[ResourceJeanBodyDiffuse.2]
filename = 3 - Jean\JeanBodyDiffuse.dds

[ResourceJeanBodyDiffuse.21]
filename = 3 - Jean\JeanBodyDiffuseNTH.dds

[ResourceJeanBodyLightMap.2]
filename = 3 - Jean\JeanBodyLightMap.dds

[ResourceJeanBodyLightMap.21]
filename = 3 - Jean\JeanBodyLightMapNTH.dds

[ResourceJeanFaceHeadDiffuse.2]
filename = 3 - Jean\JeanFaceHeadDiffuse.dds

[ResourceJeanPosition.3]
type = Buffer
stride = 40
filename = 4 - Jean TH\JeanPosition.buf

[ResourceJeanBlend.3]
type = Buffer
stride = 32
filename = 4 - Jean TH\JeanBlend.buf

[ResourceJeanTexcoord.3]
type = Buffer
stride = 12
filename = 4 - Jean TH\JeanTexcoord.buf

[ResourceJeanHeadIB.3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 4 - Jean TH\JeanHead.ib

[ResourceJeanBodyIB.3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = 4 - Jean TH\JeanBody.ib

[ResourceJeanHeadDiffuse.3]
filename = 4 - Jean TH\JeanHeadDiffuse.dds

[ResourceJeanHeadLightMap.3]
filename = 4 - Jean TH\JeanHeadLightMap.dds

[ResourceJeanBodyDiffuse.3]
filename = 4 - Jean TH\JeanBodyDiffuse.dds

[ResourceJeanBodyDiffuse.31]
filename = 4 - Jean TH\JeanBodyDiffuseNTH.dds

[ResourceJeanBodyLightMap.3]
filename = 4 - Jean TH\JeanBodyLightMap.dds

[ResourceJeanBodyLightMap.31]
filename = 4 - Jean TH\JeanBodyLightMapNTH.dds

[ResourceJeanFaceHeadDiffuse.3]
filename = 4 - Jean TH\JeanFaceHeadDiffuse.dds


; --------------- Jean Remap ---------------
; Jean remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Jean mods pls give credit for "Albert Gold#2696" and "Nhok0169"
; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

; ***** JeanCN *****
[TextureOverrideJeanJeanCNRemapBlend]
hash = d159bf31
run = CommandListJeanJeanCNRemapBlend

[CommandListJeanJeanCNRemapBlend]
if $swapvar == 0
\tvb1 = ResourceJeanJeanCNRemapBlend.0
\thandling = skip
\tdraw = 11232,0
else if $swapvar == 1
\tvb1 = ResourceJeanJeanCNRemapBlend.1
\thandling = skip
\tdraw = 11232,0
else if $swapvar == 2
\tvb1 = ResourceJeanJeanCNRemapBlend.2
\thandling = skip
\tdraw = 10320,0
else if $swapvar == 3
\tvb1 = ResourceJeanJeanCNRemapBlend.3
\thandling = skip
\tdraw = 10320,0
endif


[TextureOverrideJeanPositionJeanCNRemapFix]
hash = 93bb2522
run = CommandListJeanPositionJeanCNRemapFix
$active = 1

[CommandListJeanPositionJeanCNRemapFix]
if $swapvar == 0
\tvb0 = ResourceJeanPosition.0
else if $swapvar == 1
\tvb0 = ResourceJeanPosition.1
else if $swapvar == 2
\tvb0 = ResourceJeanPosition.2
else if $swapvar == 3
\tvb0 = ResourceJeanPosition.3
endif

[TextureOverrideJeanTexcoordJeanCNRemapFix]
hash = 0ffefb98
run = CommandListJeanTexcoordJeanCNRemapFix

[CommandListJeanTexcoordJeanCNRemapFix]
if $swapvar == 0
\tvb1 = ResourceJeanTexcoord.0
else if $swapvar == 1
\tvb1 = ResourceJeanTexcoord.1
else if $swapvar == 2
\tvb1 = ResourceJeanTexcoord.2
else if $swapvar == 3
\tvb1 = ResourceJeanTexcoord.3
endif

[TextureOverrideJeanVertexLimitRaiseJeanCNRemapFix]
hash = a3cccc14

[TextureOverrideJeanIBJeanCNRemapFix]
hash = aad861e0
run = CommandListJeanIBJeanCNRemapFix

[CommandListJeanIBJeanCNRemapFix]
if $swapvar == 0
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 1
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 2
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 3
\thandling = skip
\tdrawindexed = auto
endif

[TextureOverrideJeanHeadJeanCNRemapFix]
hash = aad861e0
match_first_index = 0
run = CommandListJeanHeadJeanCNRemapFix

[CommandListJeanHeadJeanCNRemapFix]
if $swapvar == 0
\tib = ResourceJeanHeadIB.0
\tps-t0 = ResourceJeanHeadDiffuse.0
\tps-t1 = ResourceJeanHeadLightMap.0
else if $swapvar == 1
\tib = ResourceJeanHeadIB.1
\tps-t0 = ResourceJeanHeadDiffuse.1
\tps-t1 = ResourceJeanHeadLightMap.1
else if $swapvar == 2
\tib = ResourceJeanHeadIB.2
\tps-t0 = ResourceJeanHeadDiffuse.2
\tps-t1 = ResourceJeanHeadLightMap.2
else if $swapvar == 3
\tib = ResourceJeanHeadIB.3
\tps-t0 = ResourceJeanHeadDiffuse.3
\tps-t1 = ResourceJeanHeadLightMap.3
endif

[TextureOverrideJeanBodyJeanCNRemapFix]
hash = aad861e0
match_first_index = 7779
run = CommandListJeanBodyJeanCNRemapFix

[CommandListJeanBodyJeanCNRemapFix]
if $swapvar == 0 && $bl == 0
\tib = ResourceJeanBodyIB.0
\tps-t0 = ResourceJeanBodyDiffuse.0
\tps-t1 = ResourceJeanBodyLightMap.0
else if $swapvar == 0 && $bl == 1
\tib = ResourceJeanBodyIB.0
\tps-t0 = ResourceJeanBodyDiffuse.01
\tps-t1 = ResourceJeanBodyLightMap.01
else if $swapvar == 1 && $bl == 0
\tib = ResourceJeanBodyIB.1
\tps-t0 = ResourceJeanBodyDiffuse.1
\tps-t1 = ResourceJeanBodyLightMap.1
else if $swapvar == 1 && $bl == 1
\tib = ResourceJeanBodyIB.1
\tps-t0 = ResourceJeanBodyDiffuse.11
\tps-t1 = ResourceJeanBodyLightMap.11
else if $swapvar == 2 && $bl == 0
\tib = ResourceJeanBodyIB.2
\tps-t0 = ResourceJeanBodyDiffuse.2
\tps-t1 = ResourceJeanBodyLightMap.2
else if $swapvar == 2 && $bl == 1
\tib = ResourceJeanBodyIB.2
\tps-t0 = ResourceJeanBodyDiffuse.21
\tps-t1 = ResourceJeanBodyLightMap.21
else if $swapvar == 3 && $bl == 0
\tib = ResourceJeanBodyIB.3
\tps-t0 = ResourceJeanBodyDiffuse.3
\tps-t1 = ResourceJeanBodyLightMap.3
else if $swapvar == 3 && $bl == 1
\tib = ResourceJeanBodyIB.3
\tps-t0 = ResourceJeanBodyDiffuse.31
\tps-t1 = ResourceJeanBodyLightMap.31
endif

[TextureOverrideJeanFaceHeadDiffuseJeanCNRemapFix]
hash = c2d1a57e
run = CommandListJeanFaceHeadDiffuseJeanCNRemapFix

[CommandListJeanFaceHeadDiffuseJeanCNRemapFix]
if $swapvar == 0
\tps-t0 = ResourceJeanFaceHeadDiffuse.0
else if $swapvar == 1
\tps-t0 = ResourceJeanFaceHeadDiffuse.1
else if $swapvar == 2
\tps-t0 = ResourceJeanFaceHeadDiffuse.2
else if $swapvar == 3
\tps-t0 = ResourceJeanFaceHeadDiffuse.3
endif


[ResourceJeanJeanCNRemapBlend.0]
type = Buffer
stride = 32
filename = 0 - Jean DUDU/JeanJeanCNRemapBlend.buf

[ResourceJeanJeanCNRemapBlend.1]
type = Buffer
stride = 32
filename = 1 - Jean DUDU TH/JeanJeanCNRemapBlend.buf

[ResourceJeanJeanCNRemapBlend.2]
type = Buffer
stride = 32
filename = 3 - Jean/JeanJeanCNRemapBlend.buf

[ResourceJeanJeanCNRemapBlend.3]
type = Buffer
stride = 32
filename = 4 - Jean TH/JeanJeanCNRemapBlend.buf

; ******************

; ***** JeanSea *****
[TextureOverrideJeanJeanSeaRemapBlend]
hash = ac801371
run = CommandListJeanJeanSeaRemapBlend

[CommandListJeanJeanSeaRemapBlend]
if $swapvar == 0
\tvb1 = ResourceJeanJeanSeaRemapBlend.0
\thandling = skip
\tdraw = 11232,0
else if $swapvar == 1
\tvb1 = ResourceJeanJeanSeaRemapBlend.1
\thandling = skip
\tdraw = 11232,0
else if $swapvar == 2
\tvb1 = ResourceJeanJeanSeaRemapBlend.2
\thandling = skip
\tdraw = 10320,0
else if $swapvar == 3
\tvb1 = ResourceJeanJeanSeaRemapBlend.3
\thandling = skip
\tdraw = 10320,0
endif


[TextureOverrideJeanPositionJeanSeaRemapFix]
hash = 16fef1eb
run = CommandListJeanPositionJeanSeaRemapFix
$active = 1

[CommandListJeanPositionJeanSeaRemapFix]
if $swapvar == 0
\tvb0 = ResourceJeanPosition.0
else if $swapvar == 1
\tvb0 = ResourceJeanPosition.1
else if $swapvar == 2
\tvb0 = ResourceJeanPosition.2
else if $swapvar == 3
\tvb0 = ResourceJeanPosition.3
endif

[TextureOverrideJeanTexcoordJeanSeaRemapFix]
hash = 3ffb0363
run = CommandListJeanTexcoordJeanSeaRemapFix

[CommandListJeanTexcoordJeanSeaRemapFix]
if $swapvar == 0
\tvb1 = ResourceJeanTexcoord.0
else if $swapvar == 1
\tvb1 = ResourceJeanTexcoord.1
else if $swapvar == 2
\tvb1 = ResourceJeanTexcoord.2
else if $swapvar == 3
\tvb1 = ResourceJeanTexcoord.3
endif

[TextureOverrideJeanVertexLimitRaiseJeanSeaRemapFix]
hash = 1ec879c9

[TextureOverrideJeanIBJeanSeaRemapFix]
hash = 69c0c24e
run = CommandListJeanIBJeanSeaRemapFix

[CommandListJeanIBJeanSeaRemapFix]
if $swapvar == 0
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 1
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 2
\thandling = skip
\tdrawindexed = auto
else if $swapvar == 3
\thandling = skip
\tdrawindexed = auto
endif

[TextureOverrideJeanHeadJeanSeaRemapFix]
hash = 69c0c24e
match_first_index = 0
run = CommandListJeanHeadJeanSeaRemapFix

[CommandListJeanHeadJeanSeaRemapFix]
if $swapvar == 0
\tib = ResourceJeanHeadIB.0
\tps-t0 = ResourceJeanHeadDiffuse.0
\tps-t1 = ResourceJeanHeadLightMap.0
else if $swapvar == 1
\tib = ResourceJeanHeadIB.1
\tps-t0 = ResourceJeanHeadDiffuse.1
\tps-t1 = ResourceJeanHeadLightMap.1
else if $swapvar == 2
\tib = ResourceJeanHeadIB.2
\tps-t0 = ResourceJeanHeadDiffuse.2
\tps-t1 = ResourceJeanHeadLightMap.2
else if $swapvar == 3
\tib = ResourceJeanHeadIB.3
\tps-t0 = ResourceJeanHeadDiffuse.3
\tps-t1 = ResourceJeanHeadLightMap.3
endif

[TextureOverrideJeanFaceHeadDiffuseJeanSeaRemapFix]
hash = c2d1a57e
run = CommandListJeanFaceHeadDiffuseJeanSeaRemapFix

[CommandListJeanFaceHeadDiffuseJeanSeaRemapFix]
if $swapvar == 0
\tps-t0 = ResourceJeanFaceHeadDiffuse.0
else if $swapvar == 1
\tps-t0 = ResourceJeanFaceHeadDiffuse.1
else if $swapvar == 2
\tps-t0 = ResourceJeanFaceHeadDiffuse.2
else if $swapvar == 3
\tps-t0 = ResourceJeanFaceHeadDiffuse.3
endif

[TextureOverrideJeanBodyJeanSeaRemapFix]
hash = 69c0c24e
match_first_index = 7662
run = CommandListJeanBodyJeanSeaRemapFix

[TextureOverrideJeanDressJeanSeaRemapFix]
hash = 69c0c24e
match_first_index = 52542
run = CommandListJeanDressJeanSeaRemapFix

[CommandListJeanBodyJeanSeaRemapFix]
if $swapvar == 0 && $bl == 0
\tib = ResourceJeanBodyIB.0
\tps-t0 = ResourceJeanBodyDiffuse.0
\tps-t1 = ResourceJeanBodyLightMap.0
else if $swapvar == 0 && $bl == 1
\tib = ResourceJeanBodyIB.0
\tps-t0 = ResourceJeanBodyDiffuse.01
\tps-t1 = ResourceJeanBodyLightMap.01
else if $swapvar == 1 && $bl == 0
\tib = ResourceJeanBodyIB.1
\tps-t0 = ResourceJeanBodyDiffuse.1
\tps-t1 = ResourceJeanBodyLightMap.1
else if $swapvar == 1 && $bl == 1
\tib = ResourceJeanBodyIB.1
\tps-t0 = ResourceJeanBodyDiffuse.11
\tps-t1 = ResourceJeanBodyLightMap.11
else if $swapvar == 2 && $bl == 0
\tib = ResourceJeanBodyIB.2
\tps-t0 = ResourceJeanBodyDiffuse.2
\tps-t1 = ResourceJeanBodyLightMap.2
else if $swapvar == 2 && $bl == 1
\tib = ResourceJeanBodyIB.2
\tps-t0 = ResourceJeanBodyDiffuse.21
\tps-t1 = ResourceJeanBodyLightMap.21
else if $swapvar == 3 && $bl == 0
\tib = ResourceJeanBodyIB.3
\tps-t0 = ResourceJeanBodyDiffuse.3
\tps-t1 = ResourceJeanBodyLightMap.3
else if $swapvar == 3 && $bl == 1
\tib = ResourceJeanBodyIB.3
\tps-t0 = ResourceJeanBodyDiffuse.31
\tps-t1 = ResourceJeanBodyLightMap.31
endif

[CommandListJeanDressJeanSeaRemapFix]
if $swapvar == 0 && $bl == 0
\tib = ResourceJeanBodyIB.0
\tps-t0 = ResourceJeanBodyDiffuse.0
\tps-t1 = ResourceJeanBodyLightMap.0
else if $swapvar == 0 && $bl == 1
\tib = ResourceJeanBodyIB.0
\tps-t0 = ResourceJeanBodyDiffuse.01
\tps-t1 = ResourceJeanBodyLightMap.01
else if $swapvar == 1 && $bl == 0
\tib = ResourceJeanBodyIB.1
\tps-t0 = ResourceJeanBodyDiffuse.1
\tps-t1 = ResourceJeanBodyLightMap.1
else if $swapvar == 1 && $bl == 1
\tib = ResourceJeanBodyIB.1
\tps-t0 = ResourceJeanBodyDiffuse.11
\tps-t1 = ResourceJeanBodyLightMap.11
else if $swapvar == 2 && $bl == 0
\tib = ResourceJeanBodyIB.2
\tps-t0 = ResourceJeanBodyDiffuse.2
\tps-t1 = ResourceJeanBodyLightMap.2
else if $swapvar == 2 && $bl == 1
\tib = ResourceJeanBodyIB.2
\tps-t0 = ResourceJeanBodyDiffuse.21
\tps-t1 = ResourceJeanBodyLightMap.21
else if $swapvar == 3 && $bl == 0
\tib = ResourceJeanBodyIB.3
\tps-t0 = ResourceJeanBodyDiffuse.3
\tps-t1 = ResourceJeanBodyLightMap.3
else if $swapvar == 3 && $bl == 1
\tib = ResourceJeanBodyIB.3
\tps-t0 = ResourceJeanBodyDiffuse.31
\tps-t1 = ResourceJeanBodyLightMap.31
endif


[ResourceJeanJeanSeaRemapBlend.0]
type = Buffer
stride = 32
filename = 0 - Jean DUDU/JeanJeanSeaRemapBlend.buf

[ResourceJeanJeanSeaRemapBlend.1]
type = Buffer
stride = 32
filename = 1 - Jean DUDU TH/JeanJeanSeaRemapBlend.buf

[ResourceJeanJeanSeaRemapBlend.2]
type = Buffer
stride = 32
filename = 3 - Jean/JeanJeanSeaRemapBlend.buf

[ResourceJeanJeanSeaRemapBlend.3]
type = Buffer
stride = 32
filename = 4 - Jean TH/JeanJeanSeaRemapBlend.buf

; *******************

; ------------------------------------------"""]]]

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
