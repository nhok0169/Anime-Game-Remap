import sys

from .baseIniObjTest import BaseIniObjTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class GIMIObjSplitFixerTest(BaseIniObjTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._parser = None
        cls._fixer = None

    def createParser(self):
        self._parser = FRB.GIMIObjParser(self._iniFile, {"body"})

    def createFixer(self):
        self._fixer = FRB.GIMIObjSplitFixer(self._parser, {"body": ["body", "dress"]})

    def create(self):
        self.createIniFile()
        self.createParser()
        self.createFixer()
        self._iniFile._iniParser = self._parser
        self._iniFile._iniFixer = self._fixer


    # ======================= getFix =====================================

    def test_DifferentIniText_IniFixedWithBodySplit(self):
        self.create()
        tests = [[self._defaultIniTxt, 
"""

PREFIX:


; ***** Keqing *****
[TextureOverrideKeqingOpulentKeqingRemapBlend]
hash = 0bf8e621
run = CommandListKeqingOpulentKeqingRemapBlend

[CommandListKeqingOpulentKeqingRemapBlend]
                    if $swapvar == 0
                    \tvb1 = ResourceKeqingOpulentKeqingRemapBlend.0
                    \thandling = skip
                    \tdraw = 125644,0
                    else if $swapvar == 1
                    \tvb1 = ResourceKeqingOpulentKeqingRemapBlend.1
                    \thandling = skip
                    \tdraw = 129460,0
                    endif


[TextureOverrideKeqingOpulentPositionKeqingRemapFix]
hash = 3aaf3e94
run = CommandListKeqingOpulentPositionKeqingRemapFix
$active = 1

[CommandListKeqingOpulentPositionKeqingRemapFix]
                    if $swapvar == 0
                    \tvb0 = ResourceKeqingOpulentPosition.0
                    \t$ActiveCharacter = 1
                    else if $swapvar == 1
                    \tvb0 = ResourceKeqingOpulentPosition.1
                    \t$ActiveCharacter = 1
                    endif

[TextureOverrideKeqingOpulentTexcoordKeqingRemapFix]
hash = 723848fe
run = CommandListKeqingOpulentTexcoordKeqingRemapFix

[CommandListKeqingOpulentTexcoordKeqingRemapFix]
                    if $swapvar == 0
                    \tvb1 = ResourceKeqingOpulentTexcoord.0
                    else if $swapvar == 1
                    \tvb1 = ResourceKeqingOpulentTexcoord.1
                    endif

[TextureOverrideKeqingOpulentVertexLimitRaiseKeqingRemapFix]
hash = ccc33b79

[TextureOverrideKeqingOpulentIBKeqingRemapFix]
hash = cbf1894b
run = CommandListKeqingOpulentIBKeqingRemapFix

[CommandListKeqingOpulentIBKeqingRemapFix]
                    if $swapvar == 0
                    \thandling = skip
                    \tdrawindexed = auto
                    else if $swapvar == 1
                    \thandling = skip
                    \tdrawindexed = auto
                    endif

[TextureOverrideKeqingOpulentHeadKeqingRemapFix]
hash = cbf1894b
match_first_index = 0
run = CommandListKeqingOpulentHeadKeqingRemapFix

[CommandListKeqingOpulentHeadKeqingRemapFix]
                    if $swapvar == 0
                    \tib = ResourceKeqingOpulentHeadIB.0
                    \tps-t0 = ResourceKeqingOpulentHeadDiffuse.0
                    \tps-t1 = ResourceKeqingOpulentHeadLightMap.0
                    else if $swapvar == 1
                    \tib = ResourceKeqingOpulentHeadIB.1
                    \tps-t0 = ResourceKeqingOpulentHeadDiffuse.1
                    \tps-t1 = ResourceKeqingOpulentHeadLightMap.1
                    endif

[TextureOverrideKeqingOpulentFaceHeadDiffuseKeqingRemapFix]
hash = d8c9c399
run = CommandListKeqingOpulentFaceHeadDiffuseKeqingRemapFix

[CommandListKeqingOpulentFaceHeadDiffuseKeqingRemapFix]
                    if $swapvar == 0
                    \tps-t0 = ResourceKeqingOpulentFaceHeadDiffuse.0
                    else if $swapvar == 1
                    \tps-t0 = ResourceKeqingOpulentFaceHeadDiffuse.1
                    endif

[TextureOverride41FixVertexLimitRaiseKeqingRemapFix]
hash = ccc33b79

[TextureOverrideKeqingOpulentBodyKeqingRemapFix]
hash = cbf1894b
match_first_index = 10824
run = CommandListKeqingOpulentBodyKeqingRemapFix

[TextureOverrideKeqingOpulentDressKeqingRemapFix]
hash = cbf1894b
match_first_index = 48216
run = CommandListKeqingOpulentDressKeqingRemapFix

[CommandListKeqingOpulentBodyKeqingRemapFix]
                    if $swapvar == 0
                    \tib = ResourceKeqingOpulentBodyIB.0
                    \tps-t0 = ResourceKeqingOpulentBodyDiffuse.0
                    \tps-t1 = ResourceKeqingOpulentBodyLightMap.0
                    else if $swapvar == 1
                    \tib = ResourceKeqingOpulentBodyIB.1
                    \tps-t0 = ResourceKeqingOpulentBodyDiffuse.1
                    \tps-t1 = ResourceKeqingOpulentBodyLightMap.1
                    endif

[CommandListKeqingOpulentDressKeqingRemapFix]
                    if $swapvar == 0
                    \tib = ResourceKeqingOpulentBodyIB.0
                    \tps-t0 = ResourceKeqingOpulentBodyDiffuse.0
                    \tps-t1 = ResourceKeqingOpulentBodyLightMap.0
                    else if $swapvar == 1
                    \tib = ResourceKeqingOpulentBodyIB.1
                    \tps-t0 = ResourceKeqingOpulentBodyDiffuse.1
                    \tps-t1 = ResourceKeqingOpulentBodyLightMap.1
                    endif


[ResourceKeqingOpulentKeqingRemapBlend.0]
type = Buffer
stride = 32
filename = Keqing 0/KeqingOpulentKeqingRemapBlend.buf

[ResourceKeqingOpulentKeqingRemapBlend.1]
type = Buffer
stride = 32
filename = Keqing 1/KeqingOpulentKeqingRemapBlend.buf

; ******************"""]]

        prefixStr = "\n\nPREFIX:\n"

        for test in tests:
            self._iniFile.clear()
            self._iniFile._iniParser = self._parser
            self._iniFile._iniFixer = self._fixer
            self._iniFile.fileTxt = test[0]
            self._iniFile.parse()

            result = self._fixer.getFix(fixStr = prefixStr)
            self.assertEqual(result, test[1])


    def test_differentIniTextWithRegEdits_IniFixedWithSplitObjAndRegEditted(self):
        self.create()

        self._fixer.preRegEditFilters = [
            FRB.RegRemove(remove = {"body": {"ps-t0"}, "dress": {"ps-t1", "ps-t1000"}}),
            FRB.RegRemap(remap = {"body": {"ps-t0": ["bad girl"], "ps-t1": ["new-ps-t2", "new-ps-t3"]}, "dress": {"ps-t999": ["my_new_reg"], "ps-t0": []}}),
            FRB.RegNewVals(vals = {"body": {"new-ps-t2": "newValueForPST2"}})
        ]

        tests = [[self._defaultIniTxt, """

PREFIX:


; ***** Keqing *****
[TextureOverrideKeqingOpulentKeqingRemapBlend]
hash = 0bf8e621
run = CommandListKeqingOpulentKeqingRemapBlend

[CommandListKeqingOpulentKeqingRemapBlend]
                    if $swapvar == 0
                    \tvb1 = ResourceKeqingOpulentKeqingRemapBlend.0
                    \thandling = skip
                    \tdraw = 125644,0
                    else if $swapvar == 1
                    \tvb1 = ResourceKeqingOpulentKeqingRemapBlend.1
                    \thandling = skip
                    \tdraw = 129460,0
                    endif


[TextureOverrideKeqingOpulentPositionKeqingRemapFix]
hash = 3aaf3e94
run = CommandListKeqingOpulentPositionKeqingRemapFix
$active = 1

[CommandListKeqingOpulentPositionKeqingRemapFix]
                    if $swapvar == 0
                    \tvb0 = ResourceKeqingOpulentPosition.0
                    \t$ActiveCharacter = 1
                    else if $swapvar == 1
                    \tvb0 = ResourceKeqingOpulentPosition.1
                    \t$ActiveCharacter = 1
                    endif

[TextureOverrideKeqingOpulentTexcoordKeqingRemapFix]
hash = 723848fe
run = CommandListKeqingOpulentTexcoordKeqingRemapFix

[CommandListKeqingOpulentTexcoordKeqingRemapFix]
                    if $swapvar == 0
                    \tvb1 = ResourceKeqingOpulentTexcoord.0
                    else if $swapvar == 1
                    \tvb1 = ResourceKeqingOpulentTexcoord.1
                    endif

[TextureOverrideKeqingOpulentVertexLimitRaiseKeqingRemapFix]
hash = ccc33b79

[TextureOverrideKeqingOpulentIBKeqingRemapFix]
hash = cbf1894b
run = CommandListKeqingOpulentIBKeqingRemapFix

[CommandListKeqingOpulentIBKeqingRemapFix]
                    if $swapvar == 0
                    \thandling = skip
                    \tdrawindexed = auto
                    else if $swapvar == 1
                    \thandling = skip
                    \tdrawindexed = auto
                    endif

[TextureOverrideKeqingOpulentHeadKeqingRemapFix]
hash = cbf1894b
match_first_index = 0
run = CommandListKeqingOpulentHeadKeqingRemapFix

[CommandListKeqingOpulentHeadKeqingRemapFix]
                    if $swapvar == 0
                    \tib = ResourceKeqingOpulentHeadIB.0
                    \tps-t0 = ResourceKeqingOpulentHeadDiffuse.0
                    \tps-t1 = ResourceKeqingOpulentHeadLightMap.0
                    else if $swapvar == 1
                    \tib = ResourceKeqingOpulentHeadIB.1
                    \tps-t0 = ResourceKeqingOpulentHeadDiffuse.1
                    \tps-t1 = ResourceKeqingOpulentHeadLightMap.1
                    endif

[TextureOverrideKeqingOpulentFaceHeadDiffuseKeqingRemapFix]
hash = d8c9c399
run = CommandListKeqingOpulentFaceHeadDiffuseKeqingRemapFix

[CommandListKeqingOpulentFaceHeadDiffuseKeqingRemapFix]
                    if $swapvar == 0
                    \tps-t0 = ResourceKeqingOpulentFaceHeadDiffuse.0
                    else if $swapvar == 1
                    \tps-t0 = ResourceKeqingOpulentFaceHeadDiffuse.1
                    endif

[TextureOverride41FixVertexLimitRaiseKeqingRemapFix]
hash = ccc33b79

[TextureOverrideKeqingOpulentBodyKeqingRemapFix]
hash = cbf1894b
match_first_index = 10824
run = CommandListKeqingOpulentBodyKeqingRemapFix

[TextureOverrideKeqingOpulentDressKeqingRemapFix]
hash = cbf1894b
match_first_index = 48216
run = CommandListKeqingOpulentDressKeqingRemapFix

[CommandListKeqingOpulentBodyKeqingRemapFix]
                    if $swapvar == 0
                    \tib = ResourceKeqingOpulentBodyIB.0
                    \tnew-ps-t2 = newValueForPST2
                    \tnew-ps-t3 = ResourceKeqingOpulentBodyLightMap.0
                    else if $swapvar == 1
                    \tib = ResourceKeqingOpulentBodyIB.1
                    \tnew-ps-t2 = newValueForPST2
                    \tnew-ps-t3 = ResourceKeqingOpulentBodyLightMap.1
                    endif

[CommandListKeqingOpulentDressKeqingRemapFix]
                    if $swapvar == 0
                    \tib = ResourceKeqingOpulentBodyIB.0
                    else if $swapvar == 1
                    \tib = ResourceKeqingOpulentBodyIB.1
                    endif


[ResourceKeqingOpulentKeqingRemapBlend.0]
type = Buffer
stride = 32
filename = Keqing 0/KeqingOpulentKeqingRemapBlend.buf

[ResourceKeqingOpulentKeqingRemapBlend.1]
type = Buffer
stride = 32
filename = Keqing 1/KeqingOpulentKeqingRemapBlend.buf

; ******************"""]]

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