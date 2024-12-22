import sys

from .baseIniObjTest import BaseIniObjTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class GIMIObjRegEditFixerTest(BaseIniObjTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._defaultIniTxt = r"""
[Constants]
global persist $swapvar = 0
global $active
global $creditinfo = 0

[KeySwap]
condition = $active == 1
key = p
type = cycle
$swapvar = 0,1,2,3
$creditinfo = 0

[Present]
post $active = 0

; Shader ------------------------------

; Overrides ---------------------------

[TextureOverrideGanyuPosition]
hash = a5169f1d
run = CommandListGanyuPosition
$active = 1

[TextureOverrideGanyuBlend]
hash = 6f47a39d
run = CommandListGanyuBlend

[TextureOverrideGanyuTexcoord]
hash = cf27251f
run = CommandListGanyuTexcoord

[TextureOverrideGanyuVertexLimitRaise]
hash = 721ca964

[TextureOverrideGanyuIB]
hash = 1575ec63
;hash = 2da186bc
run = CommandListGanyuIB

[TextureOverrideGanyuHead]
hash = 1575ec63
;hash = 2da186bc
match_first_index = 0
run = CommandListGanyuHead

[TextureOverrideGanyuBody]
hash = 1575ec63
;hash = 2da186bc
match_first_index = 12822
run = CommandListGanyuBody

[TextureOverrideGanyuDress]
hash = 1575ec63
;hash = 2da186bc
match_first_index = 47160
run = CommandListGanyuDress

[TextureOverrideGanyuFaceHeadDiffuse]
hash = b2657593
run = CommandListGanyuFaceHeadDiffuse

; CommandList -------------------------

[CommandListGanyuPosition]
if $swapvar == 0
	vb0 = ResourceGanyuPosition.0
else if $swapvar == 1
	vb0 = ResourceGanyuPosition.1
else if $swapvar == 2
	vb0 = ResourceGanyuPosition.2
else if $swapvar == 3
	vb0 = ResourceGanyuPosition.3
endif

[CommandListGanyuBlend]
if $swapvar == 0
	vb1 = ResourceGanyuBlend.0
	handling = skip
	draw = 22548,0
else if $swapvar == 1
	vb1 = ResourceGanyuBlend.1
	handling = skip
	draw = 18988,0
else if $swapvar == 2
	vb1 = ResourceGanyuBlend.2
	handling = skip
	draw = 22555,0
else if $swapvar == 3
	vb1 = ResourceGanyuBlend.3
	handling = skip
	draw = 18995,0
endif

[CommandListGanyuTexcoord]
if $swapvar == 0
	vb1 = ResourceGanyuTexcoord.0
else if $swapvar == 1
	vb1 = ResourceGanyuTexcoord.1
else if $swapvar == 2
	vb1 = ResourceGanyuTexcoord.2
else if $swapvar == 3
	vb1 = ResourceGanyuTexcoord.3
endif

[CommandListGanyuIB]
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

[CommandListGanyuHead]
if $swapvar == 0
	ib = ResourceGanyuHeadIB.0
	ps-t0 = ResourceGanyuHeadDiffuse.0
	ps-t1 = ResourceGanyuHeadLightMap.0
else if $swapvar == 1
	ib = ResourceGanyuHeadIB.1
	ps-t0 = ResourceGanyuHeadDiffuse.1
	ps-t1 = ResourceGanyuHeadLightMap.1
else if $swapvar == 2
	ib = ResourceGanyuHeadIB.2
	ps-t0 = ResourceGanyuHeadDiffuse.2
	ps-t1 = ResourceGanyuHeadLightMap.2
else if $swapvar == 3
	ib = ResourceGanyuHeadIB.3
	ps-t0 = ResourceGanyuHeadDiffuse.3
	ps-t1 = ResourceGanyuHeadLightMap.3
endif

[CommandListGanyuBody]
if $swapvar == 0
	ib = ResourceGanyuBodyIB.0
	ps-t0 = ResourceGanyuBodyDiffuse.0
	ps-t1 = ResourceGanyuBodyLightMap.0
else if $swapvar == 1
	ib = ResourceGanyuBodyIB.1
	ps-t0 = ResourceGanyuBodyDiffuse.1
	ps-t1 = ResourceGanyuBodyLightMap.1
else if $swapvar == 2
	ib = ResourceGanyuBodyIB.2
	ps-t0 = ResourceGanyuBodyDiffuse.2
	ps-t1 = ResourceGanyuBodyLightMap.2
else if $swapvar == 3
	ib = ResourceGanyuBodyIB.3
	ps-t0 = ResourceGanyuBodyDiffuse.3
	ps-t1 = ResourceGanyuBodyLightMap.3
endif

[CommandListGanyuDress]
if $swapvar == 0
	ib = ResourceGanyuDressIB.0
	ps-t0 = ResourceGanyuDressDiffuse.0
	ps-t1 = ResourceGanyuDressLightMap.0
else if $swapvar == 1
	ib = ResourceGanyuDressIB.1
	ps-t0 = ResourceGanyuDressDiffuse.1
	ps-t1 = ResourceGanyuDressLightMap.1
else if $swapvar == 2
	ib = ResourceGanyuDressIB.2
	ps-t0 = ResourceGanyuDressDiffuse.2
	ps-t1 = ResourceGanyuDressLightMap.2
else if $swapvar == 3
	ib = ResourceGanyuDressIB.3
	ps-t0 = ResourceGanyuDressDiffuse.3
	ps-t1 = ResourceGanyuDressLightMap.3
endif

[CommandListGanyuFaceHeadDiffuse]
if $swapvar == 0
	ps-t0 = ResourceGanyuFaceHeadDiffuse.0
else if $swapvar == 1
	ps-t0 = ResourceGanyuFaceHeadDiffuse.1
else if $swapvar == 2
	ps-t0 = ResourceGanyuFaceHeadDiffuse.2
else if $swapvar == 3
	ps-t0 = ResourceGanyuFaceHeadDiffuse.3
endif

; Resources ---------------------------

[ResourceGanyuPosition.0]
type = Buffer
stride = 40
filename = .\GanyuSummer1CanonBody\GanyuPosition.buf

[ResourceGanyuBlend.0]
type = Buffer
stride = 32
filename = .\GanyuSummer1CanonBody\GanyuBlend.buf

[ResourceGanyuTexcoord.0]
type = Buffer
stride = 20
filename = .\GanyuSummer1CanonBody\GanyuTexcoord.buf

[ResourceGanyuHeadIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer1CanonBody\GanyuHead.ib

[ResourceGanyuBodyIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer1CanonBody\GanyuBody.ib

[ResourceGanyuDressIB.0]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer1CanonBody\GanyuDress.ib

[ResourceGanyuHeadDiffuse.0]
filename = .\GanyuSummer1CanonBody\GanyuHeadDiffuseCopy.dds

[ResourceGanyuHeadLightMap.0]
filename = .\GanyuSummer1CanonBody\GanyuHeadLightMap.dds

[ResourceGanyuBodyDiffuse.0]
filename = .\GanyuSummer1CanonBody\GanyuBodyDiffuse.dds

[ResourceGanyuBodyLightMap.0]
filename = .\GanyuSummer1CanonBody\GanyuBodyLightMap.dds

[ResourceGanyuDressDiffuse.0]
filename = .\GanyuSummer1CanonBody\GanyuDressDiffuse.dds

[ResourceGanyuDressLightMap.0]
filename = .\GanyuSummer1CanonBody\GanyuDressLightMap.dds

[ResourceGanyuFaceHeadDiffuse.0]
filename = .\GanyuSummer1CanonBody\GanyuFaceHeadDiffuse.dds

[ResourceGanyuPosition.1]
type = Buffer
stride = 40
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuPosition.buf

[ResourceGanyuBlend.1]
type = Buffer
stride = 32
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuBlend.buf

[ResourceGanyuTexcoord.1]
type = Buffer
stride = 20
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuTexcoord.buf

[ResourceGanyuHeadIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuHead.ib

[ResourceGanyuBodyIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuBody.ib

[ResourceGanyuDressIB.1]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuDress.ib

[ResourceGanyuHeadDiffuse.1]
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuHeadDiffuse.dds

[ResourceGanyuHeadLightMap.1]
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuHeadLightMap.dds

[ResourceGanyuBodyDiffuse.1]
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuBodyDiffuse.dds

[ResourceGanyuBodyLightMap.1]
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuBodyLightMap.dds

[ResourceGanyuDressDiffuse.1]
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuDressDiffuse.dds

[ResourceGanyuDressLightMap.1]
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuDressLightMap.dds

[ResourceGanyuFaceHeadDiffuse.1]
filename = .\GanyuSummer2CanonBodyNoSkirt\GanyuFaceHeadDiffuse.dds

[ResourceGanyuPosition.2]
type = Buffer
stride = 40
filename = .\GanyuSummer3AlternateBody\GanyuPosition.buf

[ResourceGanyuBlend.2]
type = Buffer
stride = 32
filename = .\GanyuSummer3AlternateBody\GanyuBlend.buf

[ResourceGanyuTexcoord.2]
type = Buffer
stride = 20
filename = .\GanyuSummer3AlternateBody\GanyuTexcoord.buf

[ResourceGanyuHeadIB.2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer3AlternateBody\GanyuHead.ib

[ResourceGanyuBodyIB.2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer3AlternateBody\GanyuBody.ib

[ResourceGanyuDressIB.2]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer3AlternateBody\GanyuDress.ib

[ResourceGanyuHeadDiffuse.2]
filename = .\GanyuSummer3AlternateBody\GanyuHeadDiffuse.dds

[ResourceGanyuHeadLightMap.2]
filename = .\GanyuSummer3AlternateBody\GanyuHeadLightMap.dds

[ResourceGanyuBodyDiffuse.2]
filename = .\GanyuSummer3AlternateBody\GanyuBodyDiffuse.dds

[ResourceGanyuBodyLightMap.2]
filename = .\GanyuSummer3AlternateBody\GanyuBodyLightMap.dds

[ResourceGanyuDressDiffuse.2]
filename = .\GanyuSummer3AlternateBody\GanyuDressDiffuse.dds

[ResourceGanyuDressLightMap.2]
filename = .\GanyuSummer3AlternateBody\GanyuDressLightMap.dds

[ResourceGanyuFaceHeadDiffuse.2]
filename = .\GanyuSummer3AlternateBody\GanyuFaceHeadDiffuse.dds

[ResourceGanyuPosition.3]
type = Buffer
stride = 40
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuPosition.buf

[ResourceGanyuBlend.3]
type = Buffer
stride = 32
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuBlend.buf

[ResourceGanyuTexcoord.3]
type = Buffer
stride = 20
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuTexcoord.buf

[ResourceGanyuHeadIB.3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuHead.ib

[ResourceGanyuBodyIB.3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuBody.ib

[ResourceGanyuDressIB.3]
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuDress.ib

[ResourceGanyuHeadDiffuse.3]
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuHeadDiffuse.dds

[ResourceGanyuHeadLightMap.3]
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuHeadLightMap.dds

[ResourceGanyuBodyDiffuse.3]
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuBodyDiffuse.dds

[ResourceGanyuBodyLightMap.3]
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuBodyLightMap.dds

[ResourceGanyuDressDiffuse.3]
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuDressDiffuse.dds

[ResourceGanyuDressLightMap.3]
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuDressLightMap.dds

[ResourceGanyuFaceHeadDiffuse.3]
filename = .\GanyuSummer4AlternateBodyNoSkirt\GanyuFaceHeadDiffuse.dds"""

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIObjParser(self._iniFile, {"head"})

    def createFixer(self):
        self._fixer = FRB.GIMIObjRegEditFixer(self._parser, regEditFilters = [
            FRB.RegRemap(remap = {"head": {"ps-t0": ["ps-t1"], "ps-t1": ["ps-t0", "ps-t2"]}})
		])

    def create(self):
        self.createIniFile()
        self.createParser()
        self.createFixer()
        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer

    # ======================= getFix =====================================

    def test_DifferentIniText_IniFixedWithHeadRegsRemapped(self):
        self.create()
        tests = [[self._defaultIniTxt, """

PREFIX:


; ***** kyrie *****
[TextureOverrideGanyukyrieRemapBlend]
hash = HashNotFound
run = CommandListGanyukyrieRemapBlend

[CommandListGanyukyrieRemapBlend]
if $swapvar == 0
\tvb1 = ResourceGanyukyrieRemapBlend.0
\thandling = skip
\tdraw = 22548,0
else if $swapvar == 1
\tvb1 = ResourceGanyukyrieRemapBlend.1
\thandling = skip
\tdraw = 18988,0
else if $swapvar == 2
\tvb1 = ResourceGanyukyrieRemapBlend.2
\thandling = skip
\tdraw = 22555,0
else if $swapvar == 3
\tvb1 = ResourceGanyukyrieRemapBlend.3
\thandling = skip
\tdraw = 18995,0
endif

[TextureOverrideGanyuHeadkyrieRemapFix]
hash = HashNotFound
match_first_index = missa tota
run = CommandListGanyuHeadkyrieRemapFix

[CommandListGanyuHeadkyrieRemapFix]
if $swapvar == 0
\tib = ResourceGanyuHeadIB.0
\tps-t1 = ResourceGanyuHeadDiffuse.0
\tps-t0 = ResourceGanyuHeadLightMap.0
\tps-t2 = ResourceGanyuHeadLightMap.0
else if $swapvar == 1
\tib = ResourceGanyuHeadIB.1
\tps-t1 = ResourceGanyuHeadDiffuse.1
\tps-t0 = ResourceGanyuHeadLightMap.1
\tps-t2 = ResourceGanyuHeadLightMap.1
else if $swapvar == 2
\tib = ResourceGanyuHeadIB.2
\tps-t1 = ResourceGanyuHeadDiffuse.2
\tps-t0 = ResourceGanyuHeadLightMap.2
\tps-t2 = ResourceGanyuHeadLightMap.2
else if $swapvar == 3
\tib = ResourceGanyuHeadIB.3
\tps-t1 = ResourceGanyuHeadDiffuse.3
\tps-t0 = ResourceGanyuHeadLightMap.3
\tps-t2 = ResourceGanyuHeadLightMap.3
endif


[ResourceGanyukyrieRemapBlend.0]
type = Buffer
stride = 32
filename = GanyuSummer1CanonBody/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.1]
type = Buffer
stride = 32
filename = GanyuSummer2CanonBodyNoSkirt/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.2]
type = Buffer
stride = 32
filename = GanyuSummer3AlternateBody/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.3]
type = Buffer
stride = 32
filename = GanyuSummer4AlternateBodyNoSkirt/GanyukyrieRemapBlend.buf

; *****************"""]]

        prefixStr = "\n\nPREFIX:\n"

        for test in tests:
            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer
            self._iniFile.fileTxt = test[0]
            self._iniFile.parse()

            result = self._fixer.getFix(fixStr = prefixStr)
            self.assertEqual(result, test[1])

    def test_DifferentIniText_IniFixedWithRegAndTexChanged(self):
        self.createIniFile()
        self._parser = FRB.GIMIObjParser(self._iniFile, {"head", "body"}, texEdits = {"head": {"ps-t0": {"ConcentratedDiffuse": FRB.BaseTexEditor()}}})
        self._fixer = FRB.GIMIObjRegEditFixer(self._parser, regEditFilters = [
            FRB.RegRemove(remove = {"body": {"ps-t0"}}),
            FRB.RegTexAdd(textures = {"head": {"ps-t1": ("DilutedDiffuse", FRB.TexCreator(1024, 1024), False)}}, mustAdd = False),
            FRB.RegTexAdd(textures = {"head": {"cd-1": ("SaturatedDiffuse", FRB.TexCreator(2048, 1024), True),
                                               "cd-2": ("OversaturatedDiffuse", FRB.TexCreator(0, 0), True),
                                               "cd-3": ("UnsaturatedDiffuse", FRB.TexCreator(100, 100), True)}}),
            FRB.RegTexEdit(textures = {"ConcentratedDiffuse": ["cd-1", "cd-1-2", "cd-1-1", "ps-t0"]}),
            FRB.RegRemap(remap = {"head": {"ps-t0": ["ps-t1"], "ps-t1": ["ps-t0", "ps-t2"], "cd-1": ["cd-1"], "cd-2": [], "cd-3": ["cd-3", "cd-3-1", "cd-3-2", "cd-3-3"]}}),
            FRB.RegNewVals(vals = {"head": {"cd-3-2": "Overwritten", "cd-3-3": "Newwy"}})
            ])

        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer
        
        tests = [[self._defaultIniTxt, """

PREFIX:


; ***** kyrie *****
[TextureOverrideGanyukyrieRemapBlend]
hash = HashNotFound
run = CommandListGanyukyrieRemapBlend

[CommandListGanyukyrieRemapBlend]
if $swapvar == 0
\tvb1 = ResourceGanyukyrieRemapBlend.0
\thandling = skip
\tdraw = 22548,0
else if $swapvar == 1
\tvb1 = ResourceGanyukyrieRemapBlend.1
\thandling = skip
\tdraw = 18988,0
else if $swapvar == 2
\tvb1 = ResourceGanyukyrieRemapBlend.2
\thandling = skip
\tdraw = 22555,0
else if $swapvar == 3
\tvb1 = ResourceGanyukyrieRemapBlend.3
\thandling = skip
\tdraw = 18995,0
endif

[TextureOverrideGanyuBodykyrieRemapFix]
hash = HashNotFound
match_first_index = IndexNotFound
run = CommandListGanyuBodykyrieRemapFix

[CommandListGanyuBodykyrieRemapFix]
if $swapvar == 0
\tib = ResourceGanyuBodyIB.0
\tps-t1 = ResourceGanyuBodyLightMap.0
else if $swapvar == 1
\tib = ResourceGanyuBodyIB.1
\tps-t1 = ResourceGanyuBodyLightMap.1
else if $swapvar == 2
\tib = ResourceGanyuBodyIB.2
\tps-t1 = ResourceGanyuBodyLightMap.2
else if $swapvar == 3
\tib = ResourceGanyuBodyIB.3
\tps-t1 = ResourceGanyuBodyLightMap.3
endif

[TextureOverrideGanyuHeadkyrieRemapFix]
hash = HashNotFound
match_first_index = missa tota
run = CommandListGanyuHeadkyrieRemapFix
cd-1 = ResourceKyrieHeadSaturatedDiffusekyrieRemapTex
cd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
cd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
cd-3-2 = Overwritten
cd-3-3 = Newwy

[CommandListGanyuHeadkyrieRemapFix]
if $swapvar == 0
\tib = ResourceGanyuHeadIB.0
\tps-t1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0
\tps-t0 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tps-t2 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tcd-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0
\tcd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-2 = Overwritten
\tcd-3-3 = Newwy
\tcd-1-2 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0
\tcd-1-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0
else if $swapvar == 1
\tib = ResourceGanyuHeadIB.1
\tps-t1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1
\tps-t0 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tps-t2 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tcd-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1
\tcd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-2 = Overwritten
\tcd-3-3 = Newwy
\tcd-1-2 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1
\tcd-1-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1
else if $swapvar == 2
\tib = ResourceGanyuHeadIB.2
\tps-t1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2
\tps-t0 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tps-t2 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tcd-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2
\tcd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-2 = Overwritten
\tcd-3-3 = Newwy
\tcd-1-2 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2
\tcd-1-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2
else if $swapvar == 3
\tib = ResourceGanyuHeadIB.3
\tps-t1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3
\tps-t0 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tps-t2 = ResourceKyrieHeadDilutedDiffusekyrieRemapTex
\tcd-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3
\tcd-3 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-1 = ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex
\tcd-3-2 = Overwritten
\tcd-3-3 = Newwy
\tcd-1-2 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3
\tcd-1-1 = ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3
endif


[ResourceGanyukyrieRemapBlend.0]
type = Buffer
stride = 32
filename = GanyuSummer1CanonBody/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.1]
type = Buffer
stride = 32
filename = GanyuSummer2CanonBodyNoSkirt/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.2]
type = Buffer
stride = 32
filename = GanyuSummer3AlternateBody/GanyukyrieRemapBlend.buf

[ResourceGanyukyrieRemapBlend.3]
type = Buffer
stride = 32
filename = GanyuSummer4AlternateBodyNoSkirt/GanyukyrieRemapBlend.buf

[ResourceKyrieHeadSaturatedDiffusekyrieRemapTex]
filename = KyrieHeadSaturatedDiffusekyrieRemapTex.dds

[ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex]
filename = KyrieHeadUnsaturatedDiffusekyrieRemapTex.dds

[ResourceKyrieHeadDilutedDiffusekyrieRemapTex]
filename = KyrieHeadDilutedDiffusekyrieRemapTex.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0]
filename = GanyuSummer1CanonBody/GanyuHeadDiffuseCopykyrieRemapTex0.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1]
filename = GanyuSummer2CanonBodyNoSkirt/GanyuHeadDiffusekyrieRemapTex0.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2]
filename = GanyuSummer3AlternateBody/GanyuHeadDiffusekyrieRemapTex0.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3]
filename = GanyuSummer4AlternateBodyNoSkirt/GanyuHeadDiffusekyrieRemapTex0.dds

; *****************"""]]
        
        prefixStr = "\n\nPREFIX:\n"

        for test in tests:
            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer
            self._iniFile.fileTxt = test[0]
            self._iniFile.parse()
            
            result = self._fixer.getFix(fixStr = prefixStr)
            self.assertEqual(result, test[1])