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
from enum import Enum
##### EndExtImports


##### Script
class ModTypeNames(Enum):
    """
    The names of the different types of mods this fix will fix from or fix to
    """
    
    Amber = "Amber"
    """
    Amber from GI
    """

    AmberCN = "AmberCN"
    """
    Amber Chinese version from GI
    """

    Ayaka = "Ayaka"
    """
    Ayaka from GI
    """

    AyakaSpringbloom = "AyakaSpringBloom"
    """
    Ayaka Fontaine skin from GI
    """

    Arlecchino = "Arlecchino"
    """
    Arlecchino from GI
    """

    ArlecchinoBoss = "ArlecchinoBoss"
    """
    The first phase of the Arlecchino boss from GI
    """

    Barbara = "Barbara"
    """
    Barabara from GI
    """

    BarbaraSummertime = "BarbaraSummertime"
    """
    Barbara summer skin from GI
    """

    CherryHuTao = "CherryHuTao"
    """
    Hu Tao Lantern Rite skin from GI
    """

    Diluc = "Diluc"
    """
    Diluc from GI
    """

    DilucFlamme = "DilucFlamme"
    """
    Diluc Red Dead of the Night skin from GI
    """

    Fischl = "Fischl"
    """
    Fischl from GI
    """

    FischlHighness = "FischlHighness"
    """
    Fischl summer skin from GI
    """

    Ganyu = "Ganyu"
    """
    Ganyu from GI
    """

    GanyuTwilight = "GanyuTwilight"
    """
    Ganyu Lantern Rite skin from GI
    """

    HuTao = "HuTao"
    """
    HuTao from GI
    """

    Jean = "Jean"
    """
    Jean from GI
    """

    JeanCN = "JeanCN"
    """
    Jean Chinese version from GI
    """

    JeanSea = "JeanSea"
    """
    Jean summer skin from GI
    """

    Kaeya = "Kaeya"
    """
    Kaeya from GI
    """

    KaeyaSailwind = "KaeyaSailwind"
    """
    KaeyaSailwind from GI
    """

    Keqing = "Keqing"
    """
    Keqing from GI
    """

    KeqingOpulent = "KeqingOpulent"
    """
    Keqing Lantern Rite skin from GI
    """

    Kirara = "Kirara"
    """
    Kirara from GI
    """

    KiraraBoots = "KiraraBoots"
    """
    Kirara summer skin from GI
    """

    Klee = "Klee"
    """
    Klee from GI
    """

    KleeBlossomingStarlight = "KleeBlossomingStarlight"
    """
    Klee summer skin from GI
    """

    Lisa = "Lisa"
    """
    Lisa from GI
    """

    LisaStudent = "LisaStudent"
    """
    Lisa Sumeru skin from GI
    """

    Mona = "Mona"
    """
    Mona from GI
    """

    MonaCN = "MonaCN"
    """
    Mona Chinese version from GI
    """

    Nilou = "Nilou"
    """
    Nilou from GI
    """

    NilouBreeze = "NilouBreeze"
    """
    Nilou summer skin from GI
    """

    Ningguang = "Ningguang"
    """
    Ningguang from GI
    """

    NingguangOrchid = "NingguangOrchid"
    """
    Ningguang Lantern Rite from GI
    """

    Raiden = "Raiden"
    """
    Ei from GI
    """

    RaidenBoss = "RaidenBoss"
    """
    The first phase of the Raiden Shogun boss from GI
    """

    Rosaria = "Rosaria"
    """
    Rosaria from GI
    """

    RosariaCN = "RosariaCN"
    """
    Rosaria Chinese version from GI
    """

    Shenhe = "Shenhe"
    """
    Shenhe from GI
    """

    ShenheFrostFlower = "ShenheFrostFlower"
    """
    Shenhe Lantern Rite skin from GI
    """

    Xiangling = "Xiangling"
    """
    Xiangling from GI
    """

    XianglingCheer = "XianglingCheer"
    """
    Xiangling Lantern Rite skin from GI
    """

    Xingqiu = "Xingqiu"
    """
    Xingqiu from GI
    """

    XingqiuBamboo = "XingqiuBamboo"
    """
    Xingqiu Lantern Rite skin from GI
    """
##### EndScripts