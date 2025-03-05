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
import re
##### EndExtImports

##### LocalImports
from ..constants.IniConsts import IniKeywords
from ..constants.ModTypeNames import ModTypeNames
from .ModTypeBuilder import ModTypeBuilder
from ..model.strategies.ModType import ModType
from ..model.strategies.iniParsers.IniParseBuilder import IniParseBuilder
from ..model.strategies.iniFixers.IniFixBuilder import IniFixBuilder
from ..model.assets.Hashes import Hashes
from ..model.assets.Indices import Indices
from ..model.assets.VGRemaps import VGRemaps
from ..model.assets.IniFixBuilderArgs import IniFixBuilderArgs
from ..model.assets.IniParseBuilderArgs import IniParseBuilderArgs
from ..model.assets.PositionEditors import PositionEditors
##### EndLocalImports


##### Script
class GIBuilder(ModTypeBuilder):
    """
    This Class inherits from :class:`ModTypeBuilder`

    Creates new :class:`ModType` objects for some anime game
    """

    @classmethod
    def _regValIsOrFix(cls, val: str) -> bool:
        return val[1] == IniKeywords.ORFixPath.value

    @classmethod
    def amber(cls) -> ModType:
        """
        Creates the :class:`ModType` for Amber

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Amber.value, 
                    Hashes(map = {ModTypeNames.Amber.value: {ModTypeNames.AmberCN.value}}),Indices(map = {ModTypeNames.Amber.value: {ModTypeNames.AmberCN.value}}),
                    aliases = ["BaronBunny", "ColleisBestie"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Amber.value: {ModTypeNames.AmberCN.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Amber.value: {ModTypeNames.AmberCN.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))

    @classmethod
    def amberCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for AmberCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.AmberCN.value, 
                    Hashes(map = {ModTypeNames.AmberCN.value: {ModTypeNames.Amber.value}}),Indices(map = {ModTypeNames.AmberCN.value: {ModTypeNames.Amber.value}}),
                    aliases = ["BaronBunnyCN", "ColleisBestieCN"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.AmberCN.value: {ModTypeNames.Amber.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.AmberCN.value: {ModTypeNames.Amber.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))

    @classmethod
    def ayaka(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ayaka

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Ayaka.value,
                    Hashes(map = {ModTypeNames.Ayaka.value: {ModTypeNames.AyakaSpringbloom.value}}),Indices(map = {ModTypeNames.Ayaka.value: {ModTypeNames.AyakaSpringbloom.value}}),
                    aliases = ["Ayaya", "Yandere", "NewArchonOfEternity"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Ayaka.value: {ModTypeNames.AyakaSpringbloom.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Ayaka.value: {ModTypeNames.AyakaSpringbloom.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def ayakaSpringBloom(cls) -> ModType:
        """
        Creates the :class:`ModType` for AyakaSpringBloom

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.AyakaSpringbloom.value,
                    Hashes(map = {ModTypeNames.AyakaSpringbloom.value: {ModTypeNames.Ayaka.value}}),Indices(map = {ModTypeNames.AyakaSpringbloom.value: {ModTypeNames.Ayaka.value}}),
                    aliases = ["AyayaFontaine", "YandereFontaine", "NewArchonOfEternityFontaine",
                               "FontaineAyaya", "FontaineYandere", "NewFontaineArchonOfEternity",
                               "MusketeerAyaka", "AyakaMusketeer", "AyayaMusketeer"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.AyakaSpringbloom.value: {ModTypeNames.Ayaka.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.AyakaSpringbloom.value: {ModTypeNames.Ayaka.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))

    @classmethod
    def arlecchino(cls) -> ModType:
        """
        Creates the :class:`ModType` for Arlecchino

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Arlecchino.value,
                    Hashes(map = {ModTypeNames.Arlecchino.value: {ModTypeNames.ArlecchinoBoss.value}}), Indices(map = {ModTypeNames.Arlecchino.value: {ModTypeNames.ArlecchinoBoss.value}}),
                    aliases = ["Father", "Knave", "Perrie", "Peruere", "Harlequin"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Arlecchino.value: {ModTypeNames.ArlecchinoBoss.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Arlecchino.value: {ModTypeNames.ArlecchinoBoss.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def barbara(cls) -> ModType:
        """
        Creates the :class:`ModType` for Barbara

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Barbara.value,
                    Hashes(map = {ModTypeNames.Barbara.value: {ModTypeNames.BarbaraSummertime.value}}),Indices(map = {ModTypeNames.Barbara.value: {ModTypeNames.BarbaraSummertime.value}}),
                    aliases = ["Idol", "Healer"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Barbara.value: {ModTypeNames.BarbaraSummertime.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Barbara.value: {ModTypeNames.BarbaraSummertime.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def barbaraSummerTime(cls) -> ModType:
        """
        Creates the :class:`ModType` for BarbaraSummerTime

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.BarbaraSummertime.value, 
                    Hashes(map = {ModTypeNames.BarbaraSummertime.value: {ModTypeNames.Barbara.value}}),Indices(map = {ModTypeNames.BarbaraSummertime.value: {ModTypeNames.Barbara.value}}),
                    aliases = ["IdolSummertime", "HealerSummertime", "BarbaraBikini"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.BarbaraSummertime.value: {ModTypeNames.Barbara.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.BarbaraSummertime.value: {ModTypeNames.Barbara.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def cherryHutao(cls) -> ModType:
        """
        Creates the :class:`ModType` for CherryHuTao

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.CherryHuTao.value, 
                     Hashes(map = {ModTypeNames.CherryHuTao.value: {ModTypeNames.HuTao.value}}), Indices(map = {ModTypeNames.CherryHuTao.value: {ModTypeNames.HuTao.value}}),
                     aliases = ["HutaoCherry", "HutaoSnowLaden", "SnowLadenHutao",
                                "LanternRiteHutao", "HutaoLanternRite",
                                "Cherry77thDirectoroftheWangshengFuneralParlor", "CherryQiqiKidnapper",
                                "77thDirectoroftheWangshengFuneralParlorCherry", "QiqiKidnapperCherry",
                                "LanternRite77thDirectoroftheWangshengFuneralParlor", "LanternRiteQiqiKidnapper",
                                "77thDirectoroftheWangshengFuneralParlorLanternRite", "QiqiKidnapperLanternRite",],
                     vgRemaps = VGRemaps(map = {ModTypeNames.CherryHuTao.value: {ModTypeNames.HuTao.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.CherryHuTao.value: {ModTypeNames.HuTao.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def diluc(cls) -> ModType:
        """
        Creates the :class:`ModType` for Diluc

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Diluc.value,
                    Hashes(map = {ModTypeNames.Diluc.value: {ModTypeNames.DilucFlamme.value}}),Indices(map = {ModTypeNames.Diluc.value: {ModTypeNames.DilucFlamme.value}}),
                    aliases = ["KaeyasBrother", "DawnWineryMaster", "AngelShareOwner", "DarkNightBlaze"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Diluc.value: {ModTypeNames.DilucFlamme.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Diluc.value: {ModTypeNames.DilucFlamme.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def dilucFlamme(cls) -> ModType:
        """
        Creates the :class:`ModType` for DilucFlamme

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.DilucFlamme.value,
                    Hashes(map = {ModTypeNames.DilucFlamme.value: {ModTypeNames.Diluc.value}}),Indices(map = {ModTypeNames.DilucFlamme.value: {ModTypeNames.Diluc.value}}),
                    aliases = ["RedDeadOfTheNight", "DarkNightHero"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.DilucFlamme.value: {ModTypeNames.Diluc.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.DilucFlamme.value: {ModTypeNames.Diluc.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def fischl(cls) -> ModType:
        """
        Creates the :class:`ModType` for Fischl

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Fischl.value,
                    Hashes(map = {ModTypeNames.Fischl.value: {ModTypeNames.FischlHighness.value}}),Indices(map = {ModTypeNames.Fischl.value: {ModTypeNames.FischlHighness.value}}),
                    aliases = ["Amy", "Chunibyo", "8thGraderSyndrome", "Delusional", "PrinzessinderVerurteilung", "MeinFraulein", " FischlvonLuftschlossNarfidort", "PrincessofCondemnation", "TheCondemedPrincess", "OzsMiss"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Fischl.value: {ModTypeNames.FischlHighness.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Fischl.value: {ModTypeNames.FischlHighness.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def fischlHighness(cls) -> ModType:
        """
        Creates the :class:`ModType` for FischlHighness

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.FischlHighness.value,
                    Hashes(map = {ModTypeNames.FischlHighness.value: {ModTypeNames.Fischl.value}}),Indices(map = {ModTypeNames.FischlHighness.value: {ModTypeNames.Fischl.value}}),
                    aliases = ["PrincessAmy", "RealPrinzessinderVerurteilung", "Prinzessin", "PrincessFischlvonLuftschlossNarfidort", "PrinzessinFischlvonLuftschlossNarfidort", "ImmernachtreichPrincess", 
                               "PrinzessinderImmernachtreich", "PrincessoftheEverlastingNight", "OzsPrincess"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.FischlHighness.value: {ModTypeNames.Fischl.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.FischlHighness.value: {ModTypeNames.Fischl.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def ganyu(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ganyu

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """

        return ModType(ModTypeNames.Ganyu.value,
                    Hashes(map = {ModTypeNames.Ganyu.value: {ModTypeNames.GanyuTwilight.value}}),Indices(map = {ModTypeNames.Ganyu.value: {ModTypeNames.GanyuTwilight.value}}),
                    aliases = ["Cocogoat"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Ganyu.value: {ModTypeNames.GanyuTwilight.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Ganyu.value: {ModTypeNames.GanyuTwilight.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def ganyuTwilight(cls) -> ModType:
        """
        Creates the :class:`ModType` for GanyuTwilight

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.GanyuTwilight.value,
                    Hashes(map = {ModTypeNames.GanyuTwilight.value: {ModTypeNames.Ganyu.value}}),Indices(map = {ModTypeNames.GanyuTwilight.value: {ModTypeNames.Ganyu.value}}),
                    aliases = ["GanyuLanternRite", "LanternRiteGanyu", "CocogoatTwilight", "CocogoatLanternRite", "LanternRiteCocogoat"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.GanyuTwilight.value: {ModTypeNames.Ganyu.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.GanyuTwilight.value: {ModTypeNames.Ganyu.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def huTao(cls) -> ModType:
        """
        Creates the :class:`ModType` for HuTao

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.HuTao.value, 
                     Hashes(map = {ModTypeNames.HuTao.value: {ModTypeNames.CherryHuTao.value}}), Indices(map = {ModTypeNames.HuTao.value: {ModTypeNames.CherryHuTao.value}}),
                     aliases = ["77thDirectoroftheWangshengFuneralParlor", "QiqiKidnapper"],
                     vgRemaps = VGRemaps(map = {ModTypeNames.HuTao.value: {ModTypeNames.CherryHuTao.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.HuTao.value: {ModTypeNames.CherryHuTao.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))

    @classmethod
    def jean(cls) -> ModType:
        """
        Creates the :class:`ModType` for Jean

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Jean.value,
                   Hashes(map = {ModTypeNames.Jean.value: {ModTypeNames.JeanCN.value, ModTypeNames.JeanSea.value}}), Indices(map = {ModTypeNames.Jean.value: {ModTypeNames.JeanCN.value, ModTypeNames.JeanSea.value}}),
                   aliases = ["ActingGrandMaster", "KleesBabySitter"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.Jean.value: {ModTypeNames.JeanCN.value, ModTypeNames.JeanSea.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.Jean.value: {ModTypeNames.JeanCN.value, ModTypeNames.JeanSea.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def jeanCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for JeanCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.JeanCN.value,
                   Hashes(map = {ModTypeNames.JeanCN.value: {ModTypeNames.Jean.value, ModTypeNames.JeanSea.value}}), Indices(map = {ModTypeNames.JeanCN.value: {ModTypeNames.Jean.value, ModTypeNames.JeanSea.value}}),
                   aliases = ["ActingGrandMasterCN", "KleesBabySitterCN"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.JeanCN.value: {ModTypeNames.Jean.value, ModTypeNames.JeanSea.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.JeanCN.value: {ModTypeNames.Jean.value, ModTypeNames.JeanSea.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def jeanSea(cls) -> ModType:
        """
        Creates the :class:`ModType` for JeanSea

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.JeanSea.value,
                   Hashes(map = {ModTypeNames.JeanSea.value: {ModTypeNames.Jean.value, ModTypeNames.JeanCN.value}}), Indices(map = {ModTypeNames.JeanSea.value: {ModTypeNames.Jean.value, ModTypeNames.JeanCN.value}}),
                   aliases = ["ActingGrandMasterSea", "KleesBabySitterSea"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.JeanSea.value: {ModTypeNames.Jean.value, ModTypeNames.JeanCN.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.JeanSea.value: {ModTypeNames.Jean.value, ModTypeNames.JeanCN.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def keqing(cls) -> ModType:
        """
        Creates the :class:`ModType` for Keqing

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Keqing.value,
                   Hashes(map = {ModTypeNames.Keqing.value: {ModTypeNames.KeqingOpulent.value}}),Indices(map = {ModTypeNames.Keqing.value: {ModTypeNames.KeqingOpulent.value}}),
                   aliases = ["Kequeen", "ZhongliSimp", "MoraxSimp"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.Keqing.value: {ModTypeNames.KeqingOpulent.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.Keqing.value: {ModTypeNames.KeqingOpulent.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def keqingOpulent(cls) -> ModType:
        """
        Creates the :class:`ModType` for KeqingOpulent

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.KeqingOpulent.value,
            Hashes(map = {ModTypeNames.KeqingOpulent.value: {ModTypeNames.Keqing.value}}),Indices(map = {ModTypeNames.KeqingOpulent.value: {ModTypeNames.Keqing.value}}),
            aliases = ["LanternRiteKeqing", "KeqingLaternRite", "CuterKequeen", "LanternRiteKequeen", "KequeenLanternRite", "KequeenOpulent", "CuterKeqing", 
                       "ZhongliSimpOpulent", "MoraxSimpOpulent", "ZhongliSimpLaternRite", "MoraxSimpLaternRite", "LaternRiteZhongliSimp", "LaternRiteMoraxSimp"],
            vgRemaps = VGRemaps(map = {ModTypeNames.KeqingOpulent.value: {ModTypeNames.Keqing.value}}), 
            positionEditors = PositionEditors(map = {ModTypeNames.KeqingOpulent.value: {ModTypeNames.Keqing.value}}),
            iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
            iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def kirara(cls) -> ModType:
        """
        Creates the :class:`ModType` for Kirara

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Kirara.value,
                    Hashes(map = {ModTypeNames.Kirara.value: {ModTypeNames.KiraraBoots.value}}),Indices(map = {ModTypeNames.Kirara.value: {ModTypeNames.KiraraBoots.value}}),
                    aliases = ["Nekomata", "KonomiyaExpress", "CatBox"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Kirara.value: {ModTypeNames.KiraraBoots.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Kirara.value: {ModTypeNames.KiraraBoots.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def kiraraBoots(cls) -> ModType:
        """
        Creates the :class:`ModType` for KiraraBoots

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.KiraraBoots.value,
                    Hashes(map = {ModTypeNames.KiraraBoots.value: {ModTypeNames.Kirara.value}}),Indices(map = {ModTypeNames.KiraraBoots.value: {ModTypeNames.Kirara.value}}),
                    aliases = ["NekomataInBoots", "KonomiyaExpressInBoots", "CatBoxWithBoots", "PussInBoots"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.KiraraBoots.value: {ModTypeNames.Kirara.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.KiraraBoots.value: {ModTypeNames.Kirara.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def klee(cls) -> ModType:
        """
        Creates the :class:`ModType` for Klee

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Klee.value,
                    Hashes(map = {ModTypeNames.Klee.value: {ModTypeNames.KleeBlossomingStarlight.value}}),Indices(map = {ModTypeNames.Klee.value: {ModTypeNames.KleeBlossomingStarlight.value}}),
                    aliases = ["SparkKnight", "DodocoBuddy", "DestroyerofWorlds"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.Klee.value: {ModTypeNames.KleeBlossomingStarlight.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.Klee.value: {ModTypeNames.KleeBlossomingStarlight.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))

    @classmethod
    def kleeBlossomingStarlight(cls) -> ModType:
        """
        Creates the :class:`ModType` for KleeBlossomingStarlight

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.KleeBlossomingStarlight.value,
                    Hashes(map = {ModTypeNames.KleeBlossomingStarlight.value: {ModTypeNames.Klee.value}}),Indices(map = {ModTypeNames.KleeBlossomingStarlight.value: {ModTypeNames.Klee.value}}),
                    aliases = ["RedVelvetMage", "DodocoLittleWitchBuddy", "MagicDestroyerofWorlds", "FlandreScarlet", "ScarletFlandre"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.KleeBlossomingStarlight.value: {ModTypeNames.Klee.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.KleeBlossomingStarlight.value: {ModTypeNames.Klee.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def mona(cls) -> ModType:
        """
        Creates the :class:`ModType` for Mona

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Mona.value,
                   Hashes(map = {ModTypeNames.Mona.value: {ModTypeNames.MonaCN.value}}),Indices(map = {ModTypeNames.Mona.value: {ModTypeNames.MonaCN.value}}),
                   aliases = ["NoMora", "BigHat"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.Mona.value: {ModTypeNames.MonaCN.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.Mona.value: {ModTypeNames.MonaCN.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def monaCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for MonaCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.MonaCN.value,
                   Hashes(map = {ModTypeNames.MonaCN.value: {ModTypeNames.Mona.value}}),Indices(map = {ModTypeNames.MonaCN.value: {ModTypeNames.Mona.value}}),
                   aliases = ["NoMoraCN", "BigHatCN"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.MonaCN.value: {ModTypeNames.Mona.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.MonaCN.value: {ModTypeNames.Mona.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def nilou(cls) -> ModType:
        """
        Creates the :class:`ModType` for Nilou

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Nilou.value,
                   Hashes(map = {ModTypeNames.Nilou.value: {ModTypeNames.NilouBreeze.value}}),Indices(map = {ModTypeNames.Nilou.value: {ModTypeNames.NilouBreeze.value}}),
                   aliases = ["Dancer", "Morgiana", "BloomGirl"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.Nilou.value: {ModTypeNames.NilouBreeze.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.Nilou.value: {ModTypeNames.NilouBreeze.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))

    @classmethod
    def nilouBreeze(cls) -> ModType:
        """
        Creates the :class:`ModType` for NilouBreeze

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """ 
        return ModType(ModTypeNames.NilouBreeze.value, 
                   Hashes(map = {ModTypeNames.NilouBreeze.value: {ModTypeNames.Nilou.value}}),Indices(map = {ModTypeNames.NilouBreeze.value: {ModTypeNames.Nilou.value}}),
                   aliases = ["ForestFairy", "NilouFairy", "DancerBreeze", "MorgianaBreeze", "BloomGirlBreeze",
                              "DancerFairy", "MorgianaFairy", "BloomGirlFairy", "FairyNilou", "FairyDancer", "FairyMorgiana", "FairyBloomGirl"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.NilouBreeze.value: {ModTypeNames.Nilou.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.NilouBreeze.value: {ModTypeNames.Nilou.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))

    @classmethod
    def ningguang(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ningguang

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """

        return ModType(ModTypeNames.Ningguang.value,
                   Hashes(map = {ModTypeNames.Ningguang.value: {ModTypeNames.NingguangOrchid.value}}),Indices(map = {ModTypeNames.Ningguang.value: {ModTypeNames.NingguangOrchid.value}}),
                   aliases = ["GeoMommy", "SugarMommy"],
                   vgRemaps = VGRemaps(map = {ModTypeNames.Ningguang.value: {ModTypeNames.NingguangOrchid.value}}),
                   positionEditors = PositionEditors(map = {ModTypeNames.Ningguang.value: {ModTypeNames.NingguangOrchid.value}}),
                   iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                   iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def ningguangOrchid(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ningguang

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.NingguangOrchid.value,
                    Hashes(map = {ModTypeNames.NingguangOrchid.value: {ModTypeNames.Ningguang.value}}),Indices(map = {ModTypeNames.NingguangOrchid.value: {ModTypeNames.Ningguang.value}}),
                    aliases = ["NingguangLanternRite", "LanternRiteNingguang", "GeoMommyOrchid", "SugarMommyOrchid", "GeoMommyLaternRite", "SugarMommyLanternRite",
                               "LaternRiteGeoMommy", "LanternRiteSugarMommy"],
                    vgRemaps = VGRemaps(map = {ModTypeNames.NingguangOrchid.value: {ModTypeNames.Ningguang.value}}),
                    positionEditors = PositionEditors(map = {ModTypeNames.NingguangOrchid.value: {ModTypeNames.Ningguang.value}}),
                    iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                    iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def raiden(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ei

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Raiden.value,
                     hashes = Hashes(map = {ModTypeNames.Raiden.value: {ModTypeNames.RaidenBoss.value}}), indices = Indices(),
                     aliases = ["Ei", "RaidenEi", "Shogun", "RaidenShogun", "RaidenShotgun", "Shotgun", "CrydenShogun", "Cryden", "SmolEi"], 
                     vgRemaps = VGRemaps(map = {ModTypeNames.Raiden.value: {ModTypeNames.RaidenBoss.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.Raiden.value: {ModTypeNames.RaidenBoss.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def rosaria(cls) -> ModType:
        """
        Creates the :class:`ModType` for Rosaria

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Rosaria.value,
                      Hashes(map = {ModTypeNames.Rosaria.value: {ModTypeNames.RosariaCN.value}}), Indices(map = {ModTypeNames.Rosaria.value: {ModTypeNames.RosariaCN.value}}),
                      aliases = ["GothGirl"],
                      vgRemaps = VGRemaps(map = {ModTypeNames.Rosaria.value: {ModTypeNames.RosariaCN.value}}),
                      positionEditors = PositionEditors(map = {ModTypeNames.Rosaria.value: {ModTypeNames.RosariaCN.value}}),
                      iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                      iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def rosariaCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for RosariaCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.RosariaCN.value,
                      Hashes(map = {ModTypeNames.RosariaCN.value: {ModTypeNames.Rosaria.value}}), Indices(map = {ModTypeNames.RosariaCN.value: {ModTypeNames.Rosaria.value}}),
                      aliases = ["GothGirlCN"],
                      vgRemaps = VGRemaps(map = {ModTypeNames.RosariaCN.value: {ModTypeNames.Rosaria.value}}),
                      positionEditors = PositionEditors(map = {ModTypeNames.RosariaCN.value: {ModTypeNames.Rosaria.value}}),
                      iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                      iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def shenhe(cls) -> ModType:
        """
        Creates the :class:`ModType` for Shenhe

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Shenhe.value,
                     Hashes(map = {ModTypeNames.Shenhe.value: {ModTypeNames.ShenheFrostFlower.value}}), Indices(map = {ModTypeNames.Shenhe.value: {ModTypeNames.ShenheFrostFlower.value}}),
                     aliases = ["YelansBestie", "RedRopes"],
                     vgRemaps = VGRemaps(map = {ModTypeNames.Shenhe.value: {ModTypeNames.ShenheFrostFlower.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.Shenhe.value: {ModTypeNames.ShenheFrostFlower.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def shenheFrostFlower(cls) -> ModType:
        """
        Creates the :class:`ModType` for ShenheFrostFlower

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.ShenheFrostFlower.value,
                     Hashes(map = {ModTypeNames.ShenheFrostFlower.value: {ModTypeNames.Shenhe.value}}), Indices(map = {ModTypeNames.ShenheFrostFlower.value: {ModTypeNames.Shenhe.value}}),
                     aliases = ["ShenheLanternRite", "LanternRiteShenhe", "YelansBestieFrostFlower", "YelansBestieLanternRite", "LanternRiteYelansBestie",
                                "RedRopesFrostFlower", "RedRopesLanternRite", "LanternRiteRedRopes"],
                     vgRemaps = VGRemaps(map = {ModTypeNames.ShenheFrostFlower.value: {ModTypeNames.Shenhe.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.ShenheFrostFlower.value: {ModTypeNames.Shenhe.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def xiangling(cls) -> ModType:
        """
        Creates the :class:`ModType` for Xiangling

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Xiangling.value,
                     Hashes(map = {ModTypeNames.Xiangling.value: {ModTypeNames.XianglingCheer.value}}), Indices(map = {ModTypeNames.Xiangling.value: {ModTypeNames.XianglingCheer.value}}),
                     aliases = ["CookingFanatic", "HeadChefoftheWanminRestaurant", "ChefMaosDaughter", "GuobasBuddy"],
                     vgRemaps = VGRemaps(map = {ModTypeNames.Xiangling.value: {ModTypeNames.XianglingCheer.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.Xiangling.value: {ModTypeNames.XianglingCheer.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def xianglingCheer(cls) -> ModType:
        """
        Creates the :class:`ModType` for XianglingCheer

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """

        return ModType(ModTypeNames.XianglingCheer.value,
                     Hashes(map = {ModTypeNames.XianglingCheer.value: {ModTypeNames.Xiangling.value}}), Indices(map = {ModTypeNames.XianglingCheer.value: {ModTypeNames.Xiangling.value}}),
                     aliases = ["XianglingLanternRite", "LanternRiteXiangling", 
                                "CookingFanaticLanternRite", "HeadChefoftheWanminRestaurantLanternRite", "ChefMaosDaughterLanternRite", "GuobasBuddyLanternRite",
                                "LanternRiteCookingFanatic", "LanternRiteHeadChefoftheWanminRestaurant", "LanternRiteChefMaosDaughter", "LanternRiteGuobasBuddy"],
                     vgRemaps = VGRemaps(map = {ModTypeNames.XianglingCheer.value: {ModTypeNames.Xiangling.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.XianglingCheer.value: {ModTypeNames.Xiangling.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))

    
    @classmethod
    def xingqiu(cls) -> ModType:
        """
        Creates the :class:`ModType` for Xingqiu

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.Xingqiu.value,
                     Hashes(map = {ModTypeNames.Xingqiu.value: {ModTypeNames.XingqiuBamboo.value}}), Indices(map = {ModTypeNames.Xingqiu.value: {ModTypeNames.XingqiuBamboo.value}}),
                     aliases = ["GuhuaGeek", "Bookworm", "SecondSonofTheFeiyunCommerceGuild", "ChongyunsBestie"],
                     vgRemaps = VGRemaps(map = {ModTypeNames.Xingqiu.value: {ModTypeNames.XingqiuBamboo.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.Xingqiu.value: {ModTypeNames.XingqiuBamboo.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
    
    @classmethod
    def xingqiuBamboo(cls) -> ModType:
        """
        Creates the :class:`ModType` for XingqiuBamboo

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType(ModTypeNames.XingqiuBamboo.value,
                     Hashes(map = {ModTypeNames.XingqiuBamboo.value: {ModTypeNames.Xingqiu.value}}), Indices(map = {ModTypeNames.XingqiuBamboo.value: {ModTypeNames.Xingqiu.value}}),
                     aliases = ["XingqiuLanternRite", "GuhuaGeekLanternRite", "BookwormLanternRite", "SecondSonofTheFeiyunCommerceGuildLanternRite", "ChongyunsBestieLanternRite",
                                "LanternRiteXingqiu", "LanternRiteGuhuaGeek", "LanternRiteBookworm", "LanternRiteSecondSonofTheFeiyunCommerceGuild", "LanternRiteChongyunsBestie",
                                "GuhuaGeekBamboo", "BookwormBamboo", "SecondSonofTheFeiyunCommerceGuildBamboo", "ChongyunsBestieBamboo"],
                     vgRemaps = VGRemaps(map = {ModTypeNames.XingqiuBamboo.value: {ModTypeNames.Xingqiu.value}}),
                     positionEditors = PositionEditors(map = {ModTypeNames.XingqiuBamboo.value: {ModTypeNames.Xingqiu.value}}),
                     iniParseBuilder = IniParseBuilder(IniParseBuilderArgs()),
                     iniFixBuilder = IniFixBuilder(IniFixBuilderArgs()))
##### EndScript