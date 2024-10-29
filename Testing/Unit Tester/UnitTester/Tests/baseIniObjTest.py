import sys
import re
import unittest.mock as mock
from typing import List, Dict, Union

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class BaseIniObjTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        
        cls._modTypes.add(FRB.ModTypes.KeqingOpulent.value)
        cls._modTypes.add(FRB.ModTypes.Keqing.value)

        cls._defaultIniTxt = r"""
                    ; Constants ---------------------------

                    [Constants]
                    global persist $swapvar = 0
                    global $active
                    global $creditinfo = 0

                    [KeySwap]
                    condition = $active == 1
                    key = h
                    type = cycle
                    $swapvar = 0,1
                    $creditinfo = 0

                    [Present]
                    post $active = 0
                    run = CommandListCreditInfo

                    ; Shader ------------------------------

                    ; Overrides ---------------------------

                    [TextureOverrideKeqingOpulentPosition]
                    hash = 0d7e3cc5
                    run = CommandListKeqingOpulentPosition
                    $active = 1

                    [TextureOverrideKeqingOpulentBlend]
                    hash = 6f010b58
                    run = CommandListKeqingOpulentBlend

                    [TextureOverrideKeqingOpulentTexcoord]
                    hash = 52f78cb7
                    run = CommandListKeqingOpulentTexcoord

                    [TextureOverrideKeqingOpulentVertexLimitRaise]
                    hash = efcc8769

                    [TextureOverrideKeqingOpulentIB]
                    hash = 7c6fc8c3
                    run = CommandListKeqingOpulentIB

                    [TextureOverrideKeqingOpulentHead]
                    hash = 7c6fc8c3
                    match_first_index = 0
                    run = CommandListKeqingOpulentHead

                    [TextureOverrideKeqingOpulentBody]
                    hash = 7c6fc8c3
                    match_first_index = 19623
                    run = CommandListKeqingOpulentBody

                    [TextureOverrideKeqingOpulentFaceHeadDiffuse]
                    hash = c2b17f84
                    run = CommandListKeqingOpulentFaceHeadDiffuse

                    [TextureOverride41FixVertexLimitRaise]
                    hash = 6629a84e

                    ; CommandList -------------------------

                    [CommandListKeqingOpulentPosition]
                    if $swapvar == 0
                        vb0 = ResourceKeqingOpulentPosition.0
                        $ActiveCharacter = 1
                    else if $swapvar == 1
                        vb0 = ResourceKeqingOpulentPosition.1
                        $ActiveCharacter = 1
                    endif

                    [CommandListKeqingOpulentBlend]
                    if $swapvar == 0
                        vb1 = ResourceKeqingOpulentBlend.0
                        handling = skip
                        draw = 125644,0
                    else if $swapvar == 1
                        vb1 = ResourceKeqingOpulentBlend.1
                        handling = skip
                        draw = 129460,0
                    endif

                    [CommandListKeqingOpulentTexcoord]
                    if $swapvar == 0
                        vb1 = ResourceKeqingOpulentTexcoord.0
                    else if $swapvar == 1
                        vb1 = ResourceKeqingOpulentTexcoord.1
                    endif

                    [CommandListKeqingOpulentIB]
                    if $swapvar == 0
                        handling = skip
                        drawindexed = auto
                    else if $swapvar == 1
                        handling = skip
                        drawindexed = auto
                    endif

                    [CommandListKeqingOpulentHead]
                    if $swapvar == 0
                        ib = ResourceKeqingOpulentHeadIB.0
                        ps-t0 = ResourceKeqingOpulentHeadDiffuse.0
                        ps-t1 = ResourceKeqingOpulentHeadLightMap.0
                    else if $swapvar == 1
                        ib = ResourceKeqingOpulentHeadIB.1
                        ps-t0 = ResourceKeqingOpulentHeadDiffuse.1
                        ps-t1 = ResourceKeqingOpulentHeadLightMap.1
                    endif

                    [CommandListKeqingOpulentBody]
                    if $swapvar == 0
                        ib = ResourceKeqingOpulentBodyIB.0
                        ps-t0 = ResourceKeqingOpulentBodyDiffuse.0
                        ps-t1 = ResourceKeqingOpulentBodyLightMap.0
                    else if $swapvar == 1
                        ib = ResourceKeqingOpulentBodyIB.1
                        ps-t0 = ResourceKeqingOpulentBodyDiffuse.1
                        ps-t1 = ResourceKeqingOpulentBodyLightMap.1
                    endif

                    [CommandListKeqingOpulentFaceHeadDiffuse]
                    if $swapvar == 0
                        ps-t0 = ResourceKeqingOpulentFaceHeadDiffuse.0
                    else if $swapvar == 1
                        ps-t0 = ResourceKeqingOpulentFaceHeadDiffuse.1
                    endif

                    [CommandListCreditInfo]
                    if $swapvar == 0
                        if $creditinfo == 0 && $ActiveCharacter == 1
                            pre Resource\ShaderFixes\help.ini\Notification = ResourceCreditInfo.0
                            pre run = CustomShader\ShaderFixes\help.ini\FormatText
                            pre $\ShaderFixes\help.ini\notification_timeout = time + 10.0
                            $creditinfo = 1
                        endif
                    else if $swapvar == 1
                        if $creditinfo == 0 && $ActiveCharacter == 1
                            pre Resource\ShaderFixes\help.ini\Notification = ResourceCreditInfo.1
                            pre run = CustomShader\ShaderFixes\help.ini\FormatText
                            pre $\ShaderFixes\help.ini\notification_timeout = time + 10.0
                            $creditinfo = 1
                        endif
                    endif

                    ; Resources ---------------------------

                    [ResourceKeqingOpulentPosition.0]
                    type = Buffer
                    stride = 40
                    filename = .\Keqing 0\KeqingOpulentPosition.buf

                    [ResourceKeqingOpulentBlend.0]
                    type = Buffer
                    stride = 32
                    filename = .\Keqing 0\KeqingOpulentBlend.buf

                    [ResourceKeqingOpulentTexcoord.0]
                    type = Buffer
                    stride = 20
                    filename = .\Keqing 0\KeqingOpulentTexcoord.buf

                    [ResourceKeqingOpulentHeadIB.0]
                    type = Buffer
                    format = DXGI_FORMAT_R32_UINT
                    filename = .\Keqing 0\KeqingOpulentHead.ib

                    [ResourceKeqingOpulentBodyIB.0]
                    type = Buffer
                    format = DXGI_FORMAT_R32_UINT
                    filename = .\Keqing 0\KeqingOpulentBody.ib

                    [ResourceKeqingOpulentHeadDiffuse.0]
                    filename = .\Keqing 0\KeqingOpulentHeadDiffuse.dds

                    [ResourceKeqingOpulentHeadLightMap.0]
                    filename = .\Keqing 0\KeqingOpulentHeadLightMap.dds

                    [ResourceKeqingOpulentBodyDiffuse.0]
                    filename = .\Keqing 0\KeqingOpulentBodyDiffuse.dds

                    [ResourceKeqingOpulentBodyLightMap.0]
                    filename = .\Keqing 0\KeqingOpulentBodyLightMap.dds

                    [ResourceKeqingOpulentFaceHeadDiffuse.0]
                    filename = .\Keqing 0\KeqingOpulentFaceHeadDiffuse.dds

                    [ResourceKeqingOpulentPosition.1]
                    type = Buffer
                    stride = 40
                    filename = .\Keqing 1\KeqingOpulentPosition.buf

                    [ResourceKeqingOpulentBlend.1]
                    type = Buffer
                    stride = 32
                    filename = .\Keqing 1\KeqingOpulentBlend.buf

                    [ResourceKeqingOpulentTexcoord.1]
                    type = Buffer
                    stride = 20
                    filename = .\Keqing 1\KeqingOpulentTexcoord.buf

                    [ResourceKeqingOpulentHeadIB.1]
                    type = Buffer
                    format = DXGI_FORMAT_R32_UINT
                    filename = .\Keqing 1\KeqingOpulentHead.ib

                    [ResourceKeqingOpulentBodyIB.1]
                    type = Buffer
                    format = DXGI_FORMAT_R32_UINT
                    filename = .\Keqing 1\KeqingOpulentBody.ib

                    [ResourceKeqingOpulentHeadDiffuse.1]
                    filename = .\Keqing 1\KeqingOpulentHeadDiffuse.dds

                    [ResourceKeqingOpulentHeadLightMap.1]
                    filename = .\Keqing 1\KeqingOpulentHeadLightMap.dds

                    [ResourceKeqingOpulentBodyDiffuse.1]
                    filename = .\Keqing 1\KeqingOpulentBodyDiffuse.dds

                    [ResourceKeqingOpulentBodyLightMap.1]
                    filename = .\Keqing 1\KeqingOpulentBodyLightMap.ddss

                    [ResourceKeqingOpulentFaceHeadDiffuse.1]
                    filename = .\Keqing 1\KeqingOpulentFaceHeadDiffuse.dds"""
        
        cls._iniTxtLines = []
        cls.setupIniTxt(cls._defaultIniTxt)