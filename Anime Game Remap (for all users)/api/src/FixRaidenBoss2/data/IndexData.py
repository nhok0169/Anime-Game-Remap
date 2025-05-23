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
from ..constants.ModTypeNames import ModTypeNames
##### EndLocalImports

##### Script
IndexData = {4.0 : {ModTypeNames.Amber.value: {"head": "0", "body": "5670"},
        ModTypeNames.AmberCN.value: {"head": "0", "body": "5670"},
        ModTypeNames.Ayaka.value: {"head": "0", "body": "11565", "dress": "58209"},
        ModTypeNames.AyakaSpringbloom.value: {"head": "0", "body": "56223", "dress": "69603"},
        ModTypeNames.Barbara.value: {"head": "0", "body": "12015", "dress": "46248"},
        ModTypeNames.BarbaraSummertime.value: {"head": "0", "body": "11943", "dress": "45333"},
        ModTypeNames.Diluc.value: {"head": "0", "body": "10896"},
        ModTypeNames.DilucFlamme.value: {"head": "0", "body": "38061", "dress": "56010"},
        ModTypeNames.Fischl.value: {"head": "0", "body": "11535", "dress": "42471"},
        ModTypeNames.FischlHighness.value: {"head": "0", "body": "23091"},
        ModTypeNames.Ganyu.value: {"head": "0", "body": "12822", "dress": "47160"},
        ModTypeNames.HuTao.value: {"head": "0", "body": "16509"},
        ModTypeNames.Jean.value: {"head": "0", "body": "7779"},
        ModTypeNames.JeanCN.value: {"head": "0", "body": "7779"},
        ModTypeNames.JeanSea.value: {"head": "0", "body": "7662", "dress": "52542"},
        ModTypeNames.Kaeya.value: {"head": "0", "body": "7596", "dress": "47349", "extra": "47727"}, # there seem to be 378 extra triangular faces not included in the original assets repo
        ModTypeNames.KaeyaSailwind.value: {"head": "0", "body": "23109", "dress": "76839"},
        ModTypeNames.Keqing.value: {"head": "0", "body": "10824", "dress": "48216"},
        ModTypeNames.KeqingOpulent.value: {"head": "0", "body": "19623"},
        ModTypeNames.Kirara.value: {"head": "0", "body": "37128", "dress": "75234"},
        ModTypeNames.Klee.value: {"head": "0", "body": "8436"},
        ModTypeNames.KleeBlossomingStarlight.value: {"head": "0", "body": "32553", "dress": "82101"},
        ModTypeNames.Lisa.value: {"head": "0", "body": "16815", "dress": "45873"},
        ModTypeNames.LisaStudent.value: {"head": "0", "body": "29730"},
        ModTypeNames.Mona.value: {"head": "0", "body": "17688"},
        ModTypeNames.MonaCN.value: {"head": "0", "body": "17688"},
        ModTypeNames.Nilou.value: {"head": "0", "body": "44844", "dress": "64080"},
        ModTypeNames.Ningguang.value: {"head": "0", "body": "12384", "dress": "47157"},
        ModTypeNames.NingguangOrchid.value: {"head": "0", "body": "43539", "dress": "56124"},
        ModTypeNames.Rosaria.value: {"head": "0", "body": "11139", "dress": "44088", "extra": "45990"},
        ModTypeNames.RosariaCN.value: {"head": "0", "body": "11025", "dress": "46539", "extra": "48441"},
        ModTypeNames.Shenhe.value: {"head": "0", "body": "14385", "dress": "48753"},
        ModTypeNames.Xiangling.value: {"head": "0", "body": "11964", "dress": "48120"},
        ModTypeNames.Xingqiu.value: {"head": "0", "body": "6132"}},
        4.4: {ModTypeNames.ShenheFrostFlower.value: {"head": "0", "body": "31326", "dress": "66588", "extra": "70068"},
              ModTypeNames.GanyuTwilight.value: {"head": "0", "body": "50817", "dress": "74235"},
              ModTypeNames.XingqiuBamboo.value: {"head": "0", "body": "32508", "dress": "62103"}},
        4.6: {ModTypeNames.Arlecchino.value: {"head": "0", "body": "40179", "dress": "74412"},
              ModTypeNames.ArlecchinoBoss.value: {"head": "0", "body": "40179", "dress": "74412"}},
        4.8: {ModTypeNames.NilouBreeze.value: {"head": "0", "body": "44538", "dress": "73644"},
              ModTypeNames.KiraraBoots.value: {"head": "0", "body": "36804", "dress": "80295"}},
        5.3: {ModTypeNames.CherryHuTao.value: {"head": "0", "body": "43968", "dress": "77301", "extra": "86808"},
              ModTypeNames.XianglingCheer.value: {"head": "0", "body": "46374"}}}
##### EndScript