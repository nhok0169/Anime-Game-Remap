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
from ..core import ModTypeId, ModTypeIdTools
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
    4.0: {ModTypeIdTools.getName(ModTypeId.Amber): {ModTypeIdTools.getName(ModTypeId.AmberCN): None},
          ModTypeIdTools.getName(ModTypeId.AmberCN): {ModTypeIdTools.getName(ModTypeId.Amber): None},
          ModTypeIdTools.getName(ModTypeId.Ayaka): {ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): None},
          ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): {ModTypeIdTools.getName(ModTypeId.Ayaka): None},
          ModTypeIdTools.getName(ModTypeId.Barbara): {ModTypeIdTools.getName(ModTypeId.BarbaraSummertime): None},
          ModTypeIdTools.getName(ModTypeId.BarbaraSummertime): {ModTypeIdTools.getName(ModTypeId.Barbara): None},
          ModTypeIdTools.getName(ModTypeId.Diluc): {ModTypeIdTools.getName(ModTypeId.DilucFlamme): None},
          ModTypeIdTools.getName(ModTypeId.DilucFlamme): {ModTypeIdTools.getName(ModTypeId.Diluc): None},
          ModTypeIdTools.getName(ModTypeId.Fischl): {ModTypeIdTools.getName(ModTypeId.FischlHighness): None},
          ModTypeIdTools.getName(ModTypeId.FischlHighness): {ModTypeIdTools.getName(ModTypeId.Fischl): None},
          ModTypeIdTools.getName(ModTypeId.Jean): {ModTypeIdTools.getName(ModTypeId.JeanCN): None,
                                    ModTypeIdTools.getName(ModTypeId.JeanSea): None},
          ModTypeIdTools.getName(ModTypeId.JeanCN): {ModTypeIdTools.getName(ModTypeId.Jean): None,
                                      ModTypeIdTools.getName(ModTypeId.JeanSea): None},
          ModTypeIdTools.getName(ModTypeId.JeanSea): {ModTypeIdTools.getName(ModTypeId.Jean): None,
                                       ModTypeIdTools.getName(ModTypeId.JeanCN): None},
          ModTypeIdTools.getName(ModTypeId.Kaeya): {ModTypeIdTools.getName(ModTypeId.KaeyaSailwind): None},
          ModTypeIdTools.getName(ModTypeId.KaeyaSailwind): {ModTypeIdTools.getName(ModTypeId.Kaeya): None},
          ModTypeIdTools.getName(ModTypeId.Keqing): {ModTypeIdTools.getName(ModTypeId.KeqingOpulent): None},
          ModTypeIdTools.getName(ModTypeId.KeqingOpulent): {ModTypeIdTools.getName(ModTypeId.Keqing): None},
          ModTypeIdTools.getName(ModTypeId.Klee): {ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight): None},
          ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight): {ModTypeIdTools.getName(ModTypeId.Klee): None},
          ModTypeIdTools.getName(ModTypeId.Lisa): {ModTypeIdTools.getName(ModTypeId.LisaStudent): None},
          ModTypeIdTools.getName(ModTypeId.LisaStudent): {ModTypeIdTools.getName(ModTypeId.Lisa): None},
          ModTypeIdTools.getName(ModTypeId.Mona): {ModTypeIdTools.getName(ModTypeId.MonaCN): None},
          ModTypeIdTools.getName(ModTypeId.MonaCN): {ModTypeIdTools.getName(ModTypeId.Mona): None},
          ModTypeIdTools.getName(ModTypeId.Ningguang): {ModTypeIdTools.getName(ModTypeId.NingguangOrchid): None},
          ModTypeIdTools.getName(ModTypeId.NingguangOrchid): {ModTypeIdTools.getName(ModTypeId.Ningguang): None},
          ModTypeIdTools.getName(ModTypeId.Raiden): {ModTypeIdTools.getName(ModTypeId.RaidenBoss): None},
          ModTypeIdTools.getName(ModTypeId.RaidenBoss): {ModTypeIdTools.getName(ModTypeId.Raiden): None},
          ModTypeIdTools.getName(ModTypeId.Rosaria): {ModTypeIdTools.getName(ModTypeId.RosariaCN): None},
          ModTypeIdTools.getName(ModTypeId.RosariaCN): {ModTypeIdTools.getName(ModTypeId.Rosaria): None},
          ModTypeIdTools.getName(ModTypeId.Xingqiu): {ModTypeIdTools.getName(ModTypeId.XingqiuBamboo): None},
          ModTypeIdTools.getName(ModTypeId.XingqiuBamboo): {ModTypeIdTools.getName(ModTypeId.Xingqiu): None}},

    4.4: {ModTypeIdTools.getName(ModTypeId.Ganyu): {ModTypeIdTools.getName(ModTypeId.GanyuTwilight): None},
          ModTypeIdTools.getName(ModTypeId.GanyuTwilight): {ModTypeIdTools.getName(ModTypeId.Ganyu): None},
          ModTypeIdTools.getName(ModTypeId.Shenhe): {ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower): None},
          ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower): {ModTypeIdTools.getName(ModTypeId.Shenhe): None}},

    4.6: {ModTypeIdTools.getName(ModTypeId.Arlecchino): {ModTypeIdTools.getName(ModTypeId.ArlecchinoBoss): None},
          ModTypeIdTools.getName(ModTypeId.ArlecchinoBoss): {ModTypeIdTools.getName(ModTypeId.ArlecchinoBoss): None}},

    4.8: {ModTypeIdTools.getName(ModTypeId.Kirara): {ModTypeIdTools.getName(ModTypeId.KiraraBoots): None},
          ModTypeIdTools.getName(ModTypeId.KiraraBoots): {ModTypeIdTools.getName(ModTypeId.Kirara): None},
          ModTypeIdTools.getName(ModTypeId.Nilou): {ModTypeIdTools.getName(ModTypeId.NilouBreeze): None},
          ModTypeIdTools.getName(ModTypeId.NilouBreeze): {ModTypeIdTools.getName(ModTypeId.Nilou): None}},

    5.3: {ModTypeIdTools.getName(ModTypeId.CherryHuTao): {ModTypeIdTools.getName(ModTypeId.HuTao): None},
          ModTypeIdTools.getName(ModTypeId.HuTao): {ModTypeIdTools.getName(ModTypeId.CherryHuTao): None},
          ModTypeIdTools.getName(ModTypeId.Xiangling): {ModTypeIdTools.getName(ModTypeId.XianglingCheer): BufEditor(filters = [PositionEditorFuncs.xiangling_xianglingCheer_5_3])},
          ModTypeIdTools.getName(ModTypeId.XianglingCheer): {ModTypeIdTools.getName(ModTypeId.Xiangling): BufEditor(filters = [PositionEditorFuncs.xianglingCheer_xiangling_5_3])}}
}
##### EndScript