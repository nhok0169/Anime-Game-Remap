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
        self._fixer = FRB.GIMIObjRegEditFixer(self._parser, preRegEditFilters = [
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

[TextureOverrideGanyukyrieRemapIB]
hash = HashNotFound
run = CommandListGanyukyrieRemapIB

[CommandListGanyukyrieRemapIB]
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
        self._fixer = FRB.GIMIObjRegEditFixer(self._parser, preRegEditFilters = [
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

[TextureOverrideGanyukyrieRemapIB]
hash = HashNotFound
run = CommandListGanyukyrieRemapIB

[CommandListGanyukyrieRemapIB]
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
filename = kyrieHeadSaturatedDiffuseRemapTex.dds

[ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex]
filename = kyrieHeadUnsaturatedDiffuseRemapTex.dds

[ResourceKyrieHeadDilutedDiffusekyrieRemapTex]
filename = kyrieHeadDilutedDiffuseRemapTex.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0]
filename = GanyuSummer1CanonBody/kyrieHeadRemapTexBpy Gl+.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1]
filename = GanyuSummer2CanonBodyNoSkirt/kyrieHeadRemapTexB9e Gl+.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2]
filename = GanyuSummer3AlternateBody/kyrieHeadRemapTexB9e Gl+.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3]
filename = GanyuSummer4AlternateBodyNoSkirt/kyrieHeadRemapTexB9e Gl+.dds

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
            
    def test_DifferentIniTextPreAndPostEdits_IniFixedWithRegAndTexChanged(self):
        self.createIniFile()
        self._parser = FRB.GIMIObjParser(self._iniFile, {"head", "body"}, texEdits = {"head": {"ps-t0": {"ConcentratedDiffuse": FRB.BaseTexEditor()}}})
        self._fixer = FRB.GIMIObjRegEditFixer(self._parser, preRegEditFilters = [
            FRB.RegRemove(remove = {"body": {"ps-t0"}}),
            FRB.RegTexAdd(textures = {"head": {"ps-t1": ("DilutedDiffuse", FRB.TexCreator(1024, 1024), False)}}, mustAdd = False),
            FRB.RegTexAdd(textures = {"head": {"cd-1": ("SaturatedDiffuse", FRB.TexCreator(2048, 1024), True),
                                               "cd-2": ("OversaturatedDiffuse", FRB.TexCreator(0, 0), True),
                                               "cd-3": ("UnsaturatedDiffuse", FRB.TexCreator(100, 100), True)}}),
            FRB.RegTexEdit(textures = {"ConcentratedDiffuse": ["cd-1", "cd-1-2", "cd-1-1", "ps-t0"]}),
            FRB.RegRemap(remap = {"head": {"ps-t0": ["ps-t1"], "ps-t1": ["ps-t0", "ps-t2"], "cd-1": ["cd-1"], "cd-2": [], "cd-3": ["cd-3", "cd-3-1", "cd-3-2", "cd-3-3"]}}),
            FRB.RegNewVals(vals = {"head": {"cd-3-2": "Overwritten", "cd-3-3": "Newwy"}})
            ],
            
			postRegEditFilters = [
                FRB.RegNewVals(vals = {"head": {"hash": ("HashedPotatoes!!!", lambda val: val == "HashNotFound")}, "body": {"hash": ("HashedPotatoes!!!", lambda val: val == "EmptyPlateofHashedUpPotatoes")}})
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

[TextureOverrideGanyukyrieRemapIB]
hash = HashNotFound
run = CommandListGanyukyrieRemapIB

[CommandListGanyukyrieRemapIB]
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
hash = HashedPotatoes!!!
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
filename = kyrieHeadSaturatedDiffuseRemapTex.dds

[ResourceKyrieHeadUnsaturatedDiffusekyrieRemapTex]
filename = kyrieHeadUnsaturatedDiffuseRemapTex.dds

[ResourceKyrieHeadDilutedDiffusekyrieRemapTex]
filename = kyrieHeadDilutedDiffuseRemapTex.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex0]
filename = GanyuSummer1CanonBody/kyrieHeadRemapTexBpy Gl+.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex1]
filename = GanyuSummer2CanonBodyNoSkirt/kyrieHeadRemapTexB9e Gl+.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex2]
filename = GanyuSummer3AlternateBody/kyrieHeadRemapTexB9e Gl+.dds

[ResourceKyrieHeadConcentratedDiffusekyrieRemapTex3]
filename = GanyuSummer4AlternateBodyNoSkirt/kyrieHeadRemapTexB9e Gl+.dds

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

    def test_DifferentIniText_ProperRegRemap(self):
        self.createIniFile()
        self._parser = FRB.GIMIObjParser(self._iniFile, {"head", "body"}, texEdits = {"head": {"ps-t0": {"ConcentratedDiffuse": FRB.BaseTexEditor()}}})
        self._fixer = FRB.GIMIObjRegEditFixer(self._parser, postRegEditFilters = [
                    FRB.RegNewVals(vals = {"body": {FRB.IniKeywords.Ib.value: "null"}}),
                    FRB.RegRemap(remap = {"head": {"ps-t2": ["ps-t2", "temp"]}}),
                    FRB.RegNewVals(vals = {"head": {"temp": FRB.IniKeywords.ORFixPath.value}}),
                    FRB.RegRemap(remap = {"head": {"temp": ["run"]}})
                ])

        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer
        
        tests = [["""
;----------------------------------------------------------
; >>>>>> Start of Constants <<<<<<
[Constants]
global $active0
global persist $swapkey0 = 0

; >>>>>> End of Constants <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of Present <<<<<<
[Present]
post $active0 = 0

; >>>>>> End of Present <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of Key <<<<<<
[KeySwap0]
condition = $active0 == 1
key = 0
type = cycle
$swapkey0 = 1,0

; >>>>>> End of Key <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of TextureOverrideVertexLimitRaise <<<<<<
[TextureOverride_cc7a4851_VertexLimitRaise]
hash = e71f5012
override_byte_stride = 40
override_vertex_count = 159891

; >>>>>> End of TextureOverrideVertexLimitRaise <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of TextureOverrideVB <<<<<<
; cc7a4851 ----------------------------
[TextureOverride_VB_cc7a4851_Position]
hash = 05a65c3f
run = CommandList_VB_cc7a4851_Position

[TextureOverride_VB_cc7a4851_Texcoord]
hash = c679abfe
run = CommandList_VB_cc7a4851_Texcoord

[TextureOverride_VB_cc7a4851_Blend]
hash = bd659168
run = CommandList_VB_cc7a4851_Blend

; >>>>>> End of TextureOverrideVB <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of TextureOverrideIB <<<<<<
[TextureOverride_IB_cc7a4851_Head]
hash = cc7a4851
match_first_index = 0
handling = skip
ib = Resource_cc7a4851_Head
ps-t0 = Resource_cc7a4851-2725cfa6-1-NormalMap
ps-t1 = Resource_cc7a4851-7866ddd9-1-DiffuseMap
ps-t2 = Resource_cc7a4851-16a7176a-1-LightMap
run = CommandList\global\ORFix\ORFix
run = CommandList_IB_cc7a4851_Head

[TextureOverride_IB_cc7a4851_Body]
hash = cc7a4851
match_first_index = 46374
handling = skip
ib = Resource_cc7a4851_Body
ps-t0 = Resource_cc7a4851-25260201-2-NormalMap
ps-t1 = Resource_cc7a4851-a1ef63e6-2-DiffuseMap
ps-t2 = Resource_cc7a4851-17c172d2-2-LightMap
run = CommandList_IB_cc7a4851_Body

[TextureOverrideXianglingfaceDiffuseMap]
hash = 98820b5c
ps-t0 = Resource_98820b5c-1d353f0b-1-XianglingfaceDiffuseMap

; >>>>>> End of TextureOverrideIB <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of CommandList <<<<<<
[CommandList_VB_cc7a4851_Position]
vb1 = Resourcecc7a4851Blend
vb0 = Resourcecc7a4851Position
handling = skip
draw = 159891, 0
$active0 = 1

; >>>>>> End of CommandList <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of CommandList <<<<<<
[CommandList_VB_cc7a4851_Texcoord]
vb1 = Resourcecc7a4851Texcoord

; >>>>>> End of CommandList <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of CommandList <<<<<<
[CommandList_VB_cc7a4851_Blend]

; >>>>>> End of CommandList <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of CommandList <<<<<<
[CommandList_IB_cc7a4851_Head]
; collection name: [default.029] obj name: [cc7a4851-1.011]  (VertexCount:7272)
drawindexed = 29055,0,0

; collection name: [default.029] obj name: [cc7a4851-1.014]  (VertexCount:97255)
drawindexed = 557118,29055,0

; collection name: [default.029] obj name: [cc7a4851-1.015]  (VertexCount:132)
drawindexed = 384,586173,0

if $swapkey0  == 1
; collection name: [default.031] obj name: [cc7a4851-1.013]  (VertexCount:1492)
drawindexed = 6447,586557,0

endif

[CommandList_IB_cc7a4851_Body]
; collection name: [default.030] obj name: [cc7a4851-2.011]  (VertexCount:9786)
drawindexed = 38562,0,0

; collection name: [default.030] obj name: [cc7a4851-2.008]  (VertexCount:40972)
drawindexed = 204312,38562,0

; collection name: [default.030] obj name: [cc7a4851-2.012]  (VertexCount:2982)
drawindexed = 14040,242874,0

; >>>>>> End of CommandList <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of ResourceBuffer <<<<<<
[Resourcecc7a4851Position]
encrypt_acpt_v4 = 1
type = Buffer
stride = 40
filename = Buffer/cc7a4851-Position.buf

[Resourcecc7a4851Texcoord]
encrypt_acpt_v4 = 1
type = Buffer
stride = 12
filename = Buffer/cc7a4851-Texcoord.buf

[Resourcecc7a4851Blend]
encrypt_acpt_v4 = 1
type = Buffer
stride = 32
filename = Buffer/cc7a4851-Blend.buf

[Resource_cc7a4851_Head]
encrypt_acpt_v4 = 1
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = Buffer/cc7a4851-Head.buf

[Resource_cc7a4851_Body]
encrypt_acpt_v4 = 1
type = Buffer
format = DXGI_FORMAT_R32_UINT
filename = Buffer/cc7a4851-Body.buf

; >>>>>> End of ResourceBuffer <<<<<<
;----------------------------------------------------------


;----------------------------------------------------------
; >>>>>> Start of ResourceTexture <<<<<<
[Resource_cc7a4851-2725cfa6-1-NormalMap]
filename = Texture/cc7a4851-2725cfa6-1-NormalMap.dds

[Resource_cc7a4851-7866ddd9-1-DiffuseMap]
filename = Texture/cc7a4851-7866ddd9-1-DiffuseMap.dds

[Resource_cc7a4851-16a7176a-1-LightMap]
filename = Texture/cc7a4851-16a7176a-1-LightMap.dds

[Resource_cc7a4851-25260201-2-NormalMap]
filename = Texture/cc7a4851-25260201-2-NormalMap.dds

[Resource_cc7a4851-a1ef63e6-2-DiffuseMap]
filename = Texture/cc7a4851-a1ef63e6-2-DiffuseMap.dds

[Resource_cc7a4851-17c172d2-2-LightMap]
filename = Texture/cc7a4851-17c172d2-2-LightMap.dds

[Resource_98820b5c-1d353f0b-1-XianglingfaceDiffuseMap]
filename = Texture/98820b5c-1d353f0b-1-XianglingfaceDiffuseMap.dds

; >>>>>> End of ResourceTexture <<<<<<
;----------------------------------------------------------""", 
"""

PREFIX:


; ***** kyrie *****
[TextureOverride_VB_cc7a4851_kyrieRemapBlend]
hash = HashNotFound
run = CommandList_VB_cc7a4851_kyrieRemapBlend

[CommandList_VB_cc7a4851_kyrieRemapBlend]

[TextureOverride_IB_cc7a4851_BodykyrieRemapFix]
hash = HashNotFound
match_first_index = IndexNotFound
handling = skip
ib = null
ps-t0 = Resource_cc7a4851-25260201-2-NormalMap
ps-t1 = Resource_cc7a4851-a1ef63e6-2-DiffuseMap
ps-t2 = Resource_cc7a4851-17c172d2-2-LightMap
run = CommandList_IB_cc7a4851_BodykyrieRemapFix

[CommandList_IB_cc7a4851_BodykyrieRemapFix]
drawindexed = 38562,0,0
drawindexed = 204312,38562,0
drawindexed = 14040,242874,0

[TextureOverride_IB_cc7a4851_HeadkyrieRemapFix]
hash = HashNotFound
match_first_index = missa tota
handling = skip
ib = Resource_cc7a4851_Head
ps-t0 = Resource_cc7a4851-2725cfa6-1-NormalMap
ps-t1 = Resource_cc7a4851-7866ddd9-1-DiffuseMap
ps-t2 = Resource_cc7a4851-16a7176a-1-LightMap
run = CommandList\global\ORFix\ORFix
run = CommandList\global\ORFix\ORFix
run = CommandList_IB_cc7a4851_HeadkyrieRemapFix

[CommandList_IB_cc7a4851_HeadkyrieRemapFix]
drawindexed = 29055,0,0
drawindexed = 557118,29055,0
drawindexed = 384,586173,0
if $swapkey0  == 1
\tdrawindexed = 6447,586557,0
endif

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
            
    # ====================================================================