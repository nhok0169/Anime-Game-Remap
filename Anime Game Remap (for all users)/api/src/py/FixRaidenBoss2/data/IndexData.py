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
IndexData = {4.0 : {ModTypeIdTools.getName(ModTypeId.Amber): {"": {"head": "0", "body": "5670"}},
        ModTypeIdTools.getName(ModTypeId.AmberCN): {"": {"head": "0", "body": "5670"}},
        ModTypeIdTools.getName(ModTypeId.Ayaka): {"": {"head": "0", "body": "11565", "dress": "58209"}},
        ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): {"": {"head": "0", "body": "56223", "dress": "69603"}},
        ModTypeIdTools.getName(ModTypeId.Barbara): {"": {"head": "0", "body": "12015", "dress": "46248"}},
        ModTypeIdTools.getName(ModTypeId.BarbaraSummertime): {"": {"head": "0", "body": "11943", "dress": "45333"}},
        ModTypeIdTools.getName(ModTypeId.Diluc): {"": {"head": "0", "body": "10896"}},
        ModTypeIdTools.getName(ModTypeId.DilucFlamme): {"": {"head": "0", "body": "38061", "dress": "56010"}},
        ModTypeIdTools.getName(ModTypeId.Fischl): {"": {"head": "0", "body": "11535", "dress": "42471"}},
        ModTypeIdTools.getName(ModTypeId.FischlHighness): {"": {"head": "0", "body": "23091"}},
        ModTypeIdTools.getName(ModTypeId.Ganyu): {"": {"head": "0", "body": "12822", "dress": "47160"}},
        ModTypeIdTools.getName(ModTypeId.HuTao): {"": {"head": "0", "body": "16509"}},
        ModTypeIdTools.getName(ModTypeId.Jean): {"": {"head": "0", "body": "7779"}},
        ModTypeIdTools.getName(ModTypeId.JeanCN): {"": {"head": "0", "body": "7779"}},
        ModTypeIdTools.getName(ModTypeId.JeanSea): {"": {"head": "0", "body": "7662", "dress": "52542"}},
        ModTypeIdTools.getName(ModTypeId.Kaeya): {"": {"head": "0", "body": "7596", "dress": "47349", "extra": "47727"}}, # there seem to be 378 extra triangular faces not included in the original assets repo
        ModTypeIdTools.getName(ModTypeId.KaeyaSailwind): {"": {"head": "0", "body": "23109", "dress": "76839"}},
        ModTypeIdTools.getName(ModTypeId.Keqing): {"": {"head": "0", "body": "10824", "dress": "48216"}},
        ModTypeIdTools.getName(ModTypeId.KeqingOpulent): {"": {"head": "0", "body": "19623"}},
        ModTypeIdTools.getName(ModTypeId.Kirara): {"": {"head": "0", "body": "37128", "dress": "75234"}},
        ModTypeIdTools.getName(ModTypeId.Klee): {"": {"head": "0", "body": "8436"}},
        ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight): {"": {"head": "0", "body": "32553", "dress": "82101"}},
        ModTypeIdTools.getName(ModTypeId.Lisa): {"": {"head": "0", "body": "16815", "dress": "45873"}},
        ModTypeIdTools.getName(ModTypeId.LisaStudent): {"": {"head": "0", "body": "29730"}},
        ModTypeIdTools.getName(ModTypeId.Mona): {"": {"head": "0", "body": "17688"}},
        ModTypeIdTools.getName(ModTypeId.MonaCN): {"": {"head": "0", "body": "17688"}},
        ModTypeIdTools.getName(ModTypeId.Nilou): {"": {"head": "0", "body": "44844", "dress": "64080"}},
        ModTypeIdTools.getName(ModTypeId.Ningguang): {"": {"head": "0", "body": "12384", "dress": "47157"}},
        ModTypeIdTools.getName(ModTypeId.NingguangOrchid): {"": {"head": "0", "body": "43539", "dress": "56124"}},
        ModTypeIdTools.getName(ModTypeId.Rosaria): {"": {"head": "0", "body": "11139", "dress": "44088", "extra": "45990"}},
        ModTypeIdTools.getName(ModTypeId.RosariaCN): {"": {"head": "0", "body": "11025", "dress": "46539", "extra": "48441"}},
        ModTypeIdTools.getName(ModTypeId.Shenhe): {"": {"head": "0", "body": "14385", "dress": "48753"}},
        ModTypeIdTools.getName(ModTypeId.Xiangling): {"": {"head": "0", "body": "11964", "dress": "48120"}},
        ModTypeIdTools.getName(ModTypeId.Xingqiu): {"": {"head": "0", "body": "6132"}}},
        4.4: {ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower): {"": {"head": "0", "body": "31326", "dress": "66588", "extra": "70068"}},
              ModTypeIdTools.getName(ModTypeId.GanyuTwilight): {"": {"head": "0", "body": "50817", "dress": "74235"}},
              ModTypeIdTools.getName(ModTypeId.XingqiuBamboo): {"": {"head": "0", "body": "32508", "dress": "62103"}}},
        4.6: {ModTypeIdTools.getName(ModTypeId.Arlecchino): {"": {"head": "0", "body": "40179", "dress": "74412"}},
              ModTypeIdTools.getName(ModTypeId.ArlecchinoBoss): {"": {"head": "0", "body": "40179", "dress": "74412"}}},
        4.8: {ModTypeIdTools.getName(ModTypeId.NilouBreeze): {"": {"head": "0", "body": "44538", "dress": "73644"}},
              ModTypeIdTools.getName(ModTypeId.KiraraBoots): {"": {"head": "0", "body": "36804", "dress": "80295"}}},
        5.3: {ModTypeIdTools.getName(ModTypeId.CherryHuTao): {"": {"head": "0", "body": "43968", "dress": "77301", "extra": "86808"}},
              ModTypeIdTools.getName(ModTypeId.XianglingCheer): {"": {"head": "0", "body": "46374"}}}}
##### EndScript