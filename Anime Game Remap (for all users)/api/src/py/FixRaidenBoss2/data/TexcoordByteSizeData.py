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

##### LocalImports
from ..core import ModTypeId, ModTypeIdTools
##### EndLocalImports

##### Script
TexcoordByteSizeData = {4.0 : {ModTypeIdTools.getName(ModTypeId.Amber): 12,
        ModTypeIdTools.getName(ModTypeId.AmberCN): 12,
        ModTypeIdTools.getName(ModTypeId.Ayaka): 20,
        ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): 20,
        ModTypeIdTools.getName(ModTypeId.Barbara): 20,
        ModTypeIdTools.getName(ModTypeId.BarbaraSummertime): 20,
        ModTypeIdTools.getName(ModTypeId.Diluc): 12,
        ModTypeIdTools.getName(ModTypeId.DilucFlamme): 20,
        ModTypeIdTools.getName(ModTypeId.Fischl): 20,
        ModTypeIdTools.getName(ModTypeId.FischlHighness): 12,
        ModTypeIdTools.getName(ModTypeId.Ganyu): 20,
        ModTypeIdTools.getName(ModTypeId.HuTao): 12,
        ModTypeIdTools.getName(ModTypeId.Jean): 12,
        ModTypeIdTools.getName(ModTypeId.JeanCN): 12,
        ModTypeIdTools.getName(ModTypeId.JeanSea): 20,
        ModTypeIdTools.getName(ModTypeId.Kaeya): 20,
        ModTypeIdTools.getName(ModTypeId.KaeyaSailwind): 20,
        ModTypeIdTools.getName(ModTypeId.Keqing): 20,
        ModTypeIdTools.getName(ModTypeId.KeqingOpulent): 20,
        ModTypeIdTools.getName(ModTypeId.Kirara): 20,
        ModTypeIdTools.getName(ModTypeId.Klee): 12,
        ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight): 20,
        ModTypeIdTools.getName(ModTypeId.Lisa): 20,
        ModTypeIdTools.getName(ModTypeId.LisaStudent): 20,
        ModTypeIdTools.getName(ModTypeId.Mona): 12,
        ModTypeIdTools.getName(ModTypeId.MonaCN): 12,
        ModTypeIdTools.getName(ModTypeId.Nilou): 20,
        ModTypeIdTools.getName(ModTypeId.Ningguang): 20,
        ModTypeIdTools.getName(ModTypeId.NingguangOrchid): 20,
        ModTypeIdTools.getName(ModTypeId.Raiden): 20,
        ModTypeIdTools.getName(ModTypeId.Rosaria): 20,
        ModTypeIdTools.getName(ModTypeId.RosariaCN): 20,
        ModTypeIdTools.getName(ModTypeId.Shenhe): 20,
        ModTypeIdTools.getName(ModTypeId.Xiangling): 20,
        ModTypeIdTools.getName(ModTypeId.Xingqiu): 12},
        4.4: {ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower): 20,
              ModTypeIdTools.getName(ModTypeId.GanyuTwilight): 20,
              ModTypeIdTools.getName(ModTypeId.XingqiuBamboo): 20},
        4.6: {ModTypeIdTools.getName(ModTypeId.Arlecchino): 20},
        4.8: {ModTypeIdTools.getName(ModTypeId.NilouBreeze): 20,
              ModTypeIdTools.getName(ModTypeId.KiraraBoots): 20},
        5.3: {ModTypeIdTools.getName(ModTypeId.CherryHuTao): 28,
              ModTypeIdTools.getName(ModTypeId.XianglingCheer): 12}}
##### EndScript