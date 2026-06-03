##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from typing import List, Dict, Any
##### EndExtImports

##### LocalImports
from ..constants.BufTypeNames import BufElementNames
from ..constants.ModTypeNames import ModTypeNames
from ..model.strategies.bufEditors.BufEditor import BufEditor
##### EndLocalImports

##### Script
# IniFixBuilderFunc: Class to define how the PositionEditor filters to edit the position.buf
#   for some mod for a particular version
class PositionEditorFuncs():
    @classmethod
    def xiangling_xianglingCheer_5_3(cls, src: Dict[str, List[Any]], startInd: int, lineInd: int, lineSize: int) -> Dict[str, List[Any]]:
        position = src[BufElementNames.Position.value]

        position[1] += 0.7755
        position[2] -= 0.0405
        return src
    
    @classmethod
    def xianglingCheer_xiangling_5_3(cls, src: Dict[str, List[Any]], startInd: int, lineInd: int, lineSize: int) -> Dict[str, List[Any]]:
        position = src[BufElementNames.Position.value]

        position[1] -= 0.7755
        position[2] += 0.0405
        return src


PositionEditorData= {
    4.0: {ModTypeNames.Amber.value: {ModTypeNames.AmberCN.value: None},
          ModTypeNames.AmberCN.value: {ModTypeNames.Amber.value: None},
          ModTypeNames.Ayaka.value: {ModTypeNames.AyakaSpringbloom.value: None},
          ModTypeNames.AyakaSpringbloom.value: {ModTypeNames.Ayaka.value: None},
          ModTypeNames.Barbara.value: {ModTypeNames.BarbaraSummertime.value: None},
          ModTypeNames.BarbaraSummertime.value: {ModTypeNames.Barbara.value: None},
          ModTypeNames.Diluc.value: {ModTypeNames.DilucFlamme.value: None},
          ModTypeNames.DilucFlamme.value: {ModTypeNames.Diluc.value: None},
          ModTypeNames.Fischl.value: {ModTypeNames.FischlHighness.value: None},
          ModTypeNames.FischlHighness.value: {ModTypeNames.Fischl.value: None},
          ModTypeNames.Jean.value: {ModTypeNames.JeanCN.value: None,
                                    ModTypeNames.JeanSea.value: None},
          ModTypeNames.JeanCN.value: {ModTypeNames.Jean.value: None,
                                      ModTypeNames.JeanSea.value: None},
          ModTypeNames.JeanSea.value: {ModTypeNames.Jean.value: None,
                                       ModTypeNames.JeanCN.value: None},
          ModTypeNames.Kaeya.value: {ModTypeNames.KaeyaSailwind.value: None},
          ModTypeNames.KaeyaSailwind.value: {ModTypeNames.Kaeya.value: None},
          ModTypeNames.Keqing.value: {ModTypeNames.KeqingOpulent.value: None},
          ModTypeNames.KeqingOpulent.value: {ModTypeNames.Keqing.value: None},
          ModTypeNames.Klee.value: {ModTypeNames.KleeBlossomingStarlight.value: None},
          ModTypeNames.KleeBlossomingStarlight.value: {ModTypeNames.Klee.value: None},
          ModTypeNames.Lisa.value: {ModTypeNames.LisaStudent.value: None},
          ModTypeNames.LisaStudent.value: {ModTypeNames.Lisa.value: None},
          ModTypeNames.Mona.value: {ModTypeNames.MonaCN.value: None},
          ModTypeNames.MonaCN.value: {ModTypeNames.Mona.value: None},
          ModTypeNames.Ningguang.value: {ModTypeNames.NingguangOrchid.value: None},
          ModTypeNames.NingguangOrchid.value: {ModTypeNames.Ningguang.value: None},
          ModTypeNames.Raiden.value: {ModTypeNames.RaidenBoss.value: None},
          ModTypeNames.RaidenBoss.value: {ModTypeNames.Raiden.value: None},
          ModTypeNames.Rosaria.value: {ModTypeNames.RosariaCN.value: None},
          ModTypeNames.RosariaCN.value: {ModTypeNames.Rosaria.value: None},
          ModTypeNames.Xingqiu.value: {ModTypeNames.XingqiuBamboo.value: None},
          ModTypeNames.XingqiuBamboo.value: {ModTypeNames.Xingqiu.value: None}},

    4.4: {ModTypeNames.Ganyu.value: {ModTypeNames.GanyuTwilight.value: None},
          ModTypeNames.GanyuTwilight.value: {ModTypeNames.Ganyu.value: None},
          ModTypeNames.Shenhe.value: {ModTypeNames.ShenheFrostFlower.value: None},
          ModTypeNames.ShenheFrostFlower.value: {ModTypeNames.Shenhe.value: None}},

    4.6: {ModTypeNames.Arlecchino.value: {ModTypeNames.ArlecchinoBoss.value: None},
          ModTypeNames.ArlecchinoBoss.value: {ModTypeNames.ArlecchinoBoss.value: None}},

    4.8: {ModTypeNames.Kirara.value: {ModTypeNames.KiraraBoots.value: None},
          ModTypeNames.KiraraBoots.value: {ModTypeNames.Kirara.value: None},
          ModTypeNames.Nilou.value: {ModTypeNames.NilouBreeze.value: None},
          ModTypeNames.NilouBreeze.value: {ModTypeNames.Nilou.value: None}},

    5.3: {ModTypeNames.CherryHuTao.value: {ModTypeNames.HuTao.value: None},
          ModTypeNames.HuTao.value: {ModTypeNames.CherryHuTao.value: None},
          ModTypeNames.Xiangling.value: {ModTypeNames.XianglingCheer.value: BufEditor(filters = [PositionEditorFuncs.xiangling_xianglingCheer_5_3])},
          ModTypeNames.XianglingCheer.value: {ModTypeNames.Xiangling.value: BufEditor(filters = [PositionEditorFuncs.xianglingCheer_xiangling_5_3])}}
}
##### EndScript