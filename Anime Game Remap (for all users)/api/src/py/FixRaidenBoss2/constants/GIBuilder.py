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
from ..constants.IniConsts import IniKeywords
from ..constants.ModTypeNames import ModTypeNames
from ..constants.GlobalPackageManager import GlobalPackageManager
from ..constants.Packages import PackageModules
from .BaseModTypeBuilder import BaseModTypeBuilder
from ..model.strategies.ModType import ModType
from ..model.strategies.iniParsers.IniParseBuilder import IniParseBuilder
from ..model.strategies.iniFixers.IniFixBuilder import IniFixBuilder
from ..model.assets.Hashes import Hashes
from ..model.assets.Indices import Indices
from ..data.ModDataAssets import ModDataAssets
##### EndLocalImports


##### Script
class GIBuilder(BaseModTypeBuilder):
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
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Amber.value, 
                    Hashes(map = {ModTypeNames.Amber.value: OrderedSet([ModTypeNames.AmberCN.value])}),
                    Indices(map = {ModTypeNames.Amber.value: OrderedSet([ModTypeNames.AmberCN.value])}),
                    aliases = ["BaronBunny", "ColleisBestie"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    @classmethod
    def amberCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for AmberCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.AmberCN.value, 
                    Hashes(map = {ModTypeNames.AmberCN.value: OrderedSet([ModTypeNames.Amber.value])}),
                    Indices(map = {ModTypeNames.AmberCN.value: OrderedSet([ModTypeNames.Amber.value])}),
                    aliases = ["BaronBunnyCN", "ColleisBestieCN"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    @classmethod
    def ayaka(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ayaka

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Ayaka.value,
                    Hashes(map = {ModTypeNames.Ayaka.value: OrderedSet([ModTypeNames.AyakaSpringbloom.value])}),
                    Indices(map = {ModTypeNames.Ayaka.value: OrderedSet([ModTypeNames.AyakaSpringbloom.value])}),
                    aliases = ["Ayaya", "Yandere", "NewArchonOfEternity"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def ayakaSpringBloom(cls) -> ModType:
        """
        Creates the :class:`ModType` for AyakaSpringBloom

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.AyakaSpringbloom.value,
                    Hashes(map = {ModTypeNames.AyakaSpringbloom.value: OrderedSet([ModTypeNames.Ayaka.value])}),
                    Indices(map = {ModTypeNames.AyakaSpringbloom.value: OrderedSet([ModTypeNames.Ayaka.value])}),
                    aliases = ["AyayaFontaine", "YandereFontaine", "NewArchonOfEternityFontaine",
                               "FontaineAyaya", "FontaineYandere", "NewFontaineArchonOfEternity",
                               "MusketeerAyaka", "AyakaMusketeer", "AyayaMusketeer"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    @classmethod
    def arlecchino(cls) -> ModType:
        """
        Creates the :class:`ModType` for Arlecchino

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Arlecchino.value,
                    Hashes(map = {ModTypeNames.Arlecchino.value: OrderedSet([ModTypeNames.ArlecchinoBoss.value])}), 
                    Indices(map = {ModTypeNames.Arlecchino.value: OrderedSet([ModTypeNames.ArlecchinoBoss.value])}),
                    aliases = ["Father", "Knave", "Perrie", "Peruere", "Harlequin"],
                    vertexCounts= ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def barbara(cls) -> ModType:
        """
        Creates the :class:`ModType` for Barbara

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Barbara.value,
                    Hashes(map = {ModTypeNames.Barbara.value: OrderedSet([ModTypeNames.BarbaraSummertime.value])}),
                    Indices(map = {ModTypeNames.Barbara.value: OrderedSet([ModTypeNames.BarbaraSummertime.value])}),
                    aliases = ["Idol", "Healer"],
                    vertexCounts= ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def barbaraSummerTime(cls) -> ModType:
        """
        Creates the :class:`ModType` for BarbaraSummerTime

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.BarbaraSummertime.value, 
                    Hashes(map = {ModTypeNames.BarbaraSummertime.value: OrderedSet([ModTypeNames.Barbara.value])}),
                    Indices(map = {ModTypeNames.BarbaraSummertime.value: OrderedSet([ModTypeNames.Barbara.value])}),
                    aliases = ["IdolSummertime", "HealerSummertime", "BarbaraBikini"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def cherryHutao(cls) -> ModType:
        """
        Creates the :class:`ModType` for CherryHuTao

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.CherryHuTao.value, 
                     Hashes(map = {ModTypeNames.CherryHuTao.value: OrderedSet([ModTypeNames.HuTao.value])}), 
                     Indices(map = {ModTypeNames.CherryHuTao.value: OrderedSet([ModTypeNames.HuTao.value])}),
                     aliases = ["HutaoCherry", "HutaoSnowLaden", "SnowLadenHutao",
                                "LanternRiteHutao", "HutaoLanternRite",
                                "Cherry77thDirectoroftheWangshengFuneralParlor", "CherryQiqiKidnapper",
                                "77thDirectoroftheWangshengFuneralParlorCherry", "QiqiKidnapperCherry",
                                "LanternRite77thDirectoroftheWangshengFuneralParlor", "LanternRiteQiqiKidnapper",
                                "77thDirectoroftheWangshengFuneralParlorLanternRite", "QiqiKidnapperLanternRite",],
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def diluc(cls) -> ModType:
        """
        Creates the :class:`ModType` for Diluc

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Diluc.value,
                    Hashes(map = {ModTypeNames.Diluc.value: OrderedSet([ModTypeNames.DilucFlamme.value])}),
                    Indices(map = {ModTypeNames.Diluc.value: OrderedSet([ModTypeNames.DilucFlamme.value])}),
                    aliases = ["KaeyasBrother", "DawnWineryMaster", "AngelShareOwner", "DarkNightBlaze"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def dilucFlamme(cls) -> ModType:
        """
        Creates the :class:`ModType` for DilucFlamme

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.DilucFlamme.value,
                    Hashes(map = {ModTypeNames.DilucFlamme.value: OrderedSet([ModTypeNames.Diluc.value])}),
                    Indices(map = {ModTypeNames.DilucFlamme.value: OrderedSet([ModTypeNames.Diluc.value])}),
                    aliases = ["RedDeadOfTheNight", "DarkNightHero"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def fischl(cls) -> ModType:
        """
        Creates the :class:`ModType` for Fischl

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Fischl.value,
                    Hashes(map = {ModTypeNames.Fischl.value: OrderedSet([ModTypeNames.FischlHighness.value])}),
                    Indices(map = {ModTypeNames.Fischl.value: OrderedSet([ModTypeNames.FischlHighness.value])}),
                    aliases = ["Amy", "Chunibyo", "8thGraderSyndrome", "Delusional", "PrinzessinderVerurteilung", "MeinFraulein", " FischlvonLuftschlossNarfidort", "PrincessofCondemnation", "TheCondemedPrincess", "OzsMiss"],
                    vertexCounts= ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def fischlHighness(cls) -> ModType:
        """
        Creates the :class:`ModType` for FischlHighness

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.FischlHighness.value,
                    Hashes(map = {ModTypeNames.FischlHighness.value: {ModTypeNames.Fischl.value}}),
                    Indices(map = {ModTypeNames.FischlHighness.value: {ModTypeNames.Fischl.value}}),
                    aliases = ["PrincessAmy", "RealPrinzessinderVerurteilung", "Prinzessin", "PrincessFischlvonLuftschlossNarfidort", "PrinzessinFischlvonLuftschlossNarfidort", "ImmernachtreichPrincess", 
                               "PrinzessinderImmernachtreich", "PrincessoftheEverlastingNight", "OzsPrincess"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def ganyu(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ganyu

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Ganyu.value,
                    Hashes(map = {ModTypeNames.Ganyu.value: OrderedSet([ModTypeNames.GanyuTwilight.value])}),
                    Indices(map = {ModTypeNames.Ganyu.value: OrderedSet([ModTypeNames.GanyuTwilight.value])}),
                    aliases = ["Cocogoat"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def ganyuTwilight(cls) -> ModType:
        """
        Creates the :class:`ModType` for GanyuTwilight

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.GanyuTwilight.value,
                    Hashes(map = {ModTypeNames.GanyuTwilight.value: OrderedSet([ModTypeNames.Ganyu.value])}),
                    Indices(map = {ModTypeNames.GanyuTwilight.value: OrderedSet([ModTypeNames.Ganyu.value])}),
                    aliases = ["GanyuLanternRite", "LanternRiteGanyu", "CocogoatTwilight", "CocogoatLanternRite", "LanternRiteCocogoat"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def huTao(cls) -> ModType:
        """
        Creates the :class:`ModType` for HuTao

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.HuTao.value, 
                     Hashes(map = {ModTypeNames.HuTao.value: OrderedSet([ModTypeNames.CherryHuTao.value])}), 
                     Indices(map = {ModTypeNames.HuTao.value: OrderedSet([ModTypeNames.CherryHuTao.value])}),
                     aliases = ["77thDirectoroftheWangshengFuneralParlor", "QiqiKidnapper"],
                     vertexCounts= ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    @classmethod
    def jean(cls) -> ModType:
        """
        Creates the :class:`ModType` for Jean

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Jean.value,
                   Hashes(map = {ModTypeNames.Jean.value: OrderedSet([ModTypeNames.JeanCN.value, ModTypeNames.JeanSea.value])}), 
                   Indices(map = {ModTypeNames.Jean.value: OrderedSet([ModTypeNames.JeanCN.value, ModTypeNames.JeanSea.value])}),
                   aliases = ["ActingGrandMaster", "KleesBabySitter"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def jeanCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for JeanCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.JeanCN.value,
                   Hashes(map = {ModTypeNames.JeanCN.value: OrderedSet([ModTypeNames.Jean.value, ModTypeNames.JeanSea.value])}), 
                   Indices(map = {ModTypeNames.JeanCN.value: OrderedSet([ModTypeNames.Jean.value, ModTypeNames.JeanSea.value])}),
                   aliases = ["ActingGrandMasterCN", "KleesBabySitterCN"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def jeanSea(cls) -> ModType:
        """
        Creates the :class:`ModType` for JeanSea

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.JeanSea.value,
                   Hashes(map = {ModTypeNames.JeanSea.value: OrderedSet([ModTypeNames.Jean.value, ModTypeNames.JeanCN.value])}), 
                   Indices(map = {ModTypeNames.JeanSea.value: OrderedSet([ModTypeNames.Jean.value, ModTypeNames.JeanCN.value])}),
                   aliases = ["ActingGrandMasterSea", "KleesBabySitterSea"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def kaeya(cls) -> ModType:
        """
        Creates the :class:`ModType` for Kaeya

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Kaeya.value,
                   Hashes(map = {ModTypeNames.Kaeya.value: OrderedSet([ModTypeNames.KaeyaSailwind.value])}),
                   Indices(map = {ModTypeNames.Kaeya.value: OrderedSet([ModTypeNames.KaeyaSailwind.value])}),
                   aliases = ["DilucsBrother", "CavalryCaptain"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def kaeyaSailwind(cls) -> ModType:
        """
        Creates the :class:`ModType` for KaeyaSailwind

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.KaeyaSailwind.value,
                   Hashes(map = {ModTypeNames.KaeyaSailwind.value: OrderedSet([ModTypeNames.Kaeya.value])}),
                   Indices(map = {ModTypeNames.KaeyaSailwind.value: OrderedSet([ModTypeNames.Kaeya.value])}),
                   aliases = ["DilucsBrotherSailwind", "CavalryCaptainSailwind", "TheftKaeya", "TheftDilucsBrother", "TheftCavalryCaptain", 
                              "KaeyaTheft", "DilucsBrotherTheft", "CavalryCaptainTheft"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def keqing(cls) -> ModType:
        """
        Creates the :class:`ModType` for Keqing

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Keqing.value,
                   Hashes(map = {ModTypeNames.Keqing.value: OrderedSet([ModTypeNames.KeqingOpulent.value])}),
                   Indices(map = {ModTypeNames.Keqing.value: OrderedSet([ModTypeNames.KeqingOpulent.value])}),
                   aliases = ["Kequeen", "ZhongliSimp", "MoraxSimp"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def keqingOpulent(cls) -> ModType:
        """
        Creates the :class:`ModType` for KeqingOpulent

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.KeqingOpulent.value,
            Hashes(map = {ModTypeNames.KeqingOpulent.value: OrderedSet([ModTypeNames.Keqing.value])}),
            Indices(map = {ModTypeNames.KeqingOpulent.value: OrderedSet([ModTypeNames.Keqing.value])}),
            aliases = ["LanternRiteKeqing", "KeqingLaternRite", "CuterKequeen", "LanternRiteKequeen", "KequeenLanternRite", "KequeenOpulent", "CuterKeqing", 
                       "ZhongliSimpOpulent", "MoraxSimpOpulent", "ZhongliSimpLaternRite", "MoraxSimpLaternRite", "LaternRiteZhongliSimp", "LaternRiteMoraxSimp"],
            vertexCounts = ModDataAssets.VertexCounts.value,
            iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
            iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def kirara(cls) -> ModType:
        """
        Creates the :class:`ModType` for Kirara

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Kirara.value,
                    Hashes(map = {ModTypeNames.Kirara.value: OrderedSet([ModTypeNames.KiraraBoots.value])}),
                    Indices(map = {ModTypeNames.Kirara.value: OrderedSet([ModTypeNames.KiraraBoots.value])}),
                    aliases = ["Nekomata", "KonomiyaExpress", "CatBox"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def kiraraBoots(cls) -> ModType:
        """
        Creates the :class:`ModType` for KiraraBoots

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
        
        return ModType(ModTypeNames.KiraraBoots.value,
                    Hashes(map = {ModTypeNames.KiraraBoots.value: OrderedSet([ModTypeNames.Kirara.value])}),
                    Indices(map = {ModTypeNames.KiraraBoots.value: OrderedSet([ModTypeNames.Kirara.value])}),
                    aliases = ["NekomataInBoots", "KonomiyaExpressInBoots", "CatBoxWithBoots", "PussInBoots"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def klee(cls) -> ModType:
        """
        Creates the :class:`ModType` for Klee

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Klee.value,
                    Hashes(map = {ModTypeNames.Klee.value: OrderedSet([ModTypeNames.KleeBlossomingStarlight.value])}),
                    Indices(map = {ModTypeNames.Klee.value: OrderedSet([ModTypeNames.KleeBlossomingStarlight.value])}),
                    aliases = ["SparkKnight", "DodocoBuddy", "DestroyerofWorlds"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    @classmethod
    def kleeBlossomingStarlight(cls) -> ModType:
        """
        Creates the :class:`ModType` for KleeBlossomingStarlight

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.KleeBlossomingStarlight.value,
                    Hashes(map = {ModTypeNames.KleeBlossomingStarlight.value: OrderedSet([ModTypeNames.Klee.value])}),
                    Indices(map = {ModTypeNames.KleeBlossomingStarlight.value: OrderedSet([ModTypeNames.Klee.value])}),
                    aliases = ["RedVelvetMage", "DodocoLittleWitchBuddy", "MagicDestroyerofWorlds", "FlandreScarlet", "ScarletFlandre"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def lisa(cls) -> ModType:
        """
        Creates the :class:`ModType` for Lisa

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Lisa.value,
                    Hashes(map = {ModTypeNames.Lisa.value: OrderedSet([ModTypeNames.LisaStudent.value])}),
                    Indices(map = {ModTypeNames.Lisa.value: OrderedSet([ModTypeNames.LisaStudent.value])}),
                    aliases = ["CutieLibrarian"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def lisaStudent(cls) -> ModType:
        """
        Creates the :class:`ModType` for LisaStudent

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.LisaStudent.value,
                    Hashes(map = {ModTypeNames.LisaStudent.value: OrderedSet([ModTypeNames.Lisa.value])}),
                    Indices(map = {ModTypeNames.LisaStudent.value: OrderedSet([ModTypeNames.Lisa.value])}),
                    aliases = ["LisaSumeru", "SumeruLisa", "AkademiyaLisa", "LisaAkademiya"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    @classmethod
    def mona(cls) -> ModType:
        """
        Creates the :class:`ModType` for Mona

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Mona.value,
                   Hashes(map = {ModTypeNames.Mona.value: OrderedSet([ModTypeNames.MonaCN.value])}),
                   Indices(map = {ModTypeNames.Mona.value: OrderedSet([ModTypeNames.MonaCN.value])}),
                   aliases = ["NoMora", "BigHat"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def monaCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for MonaCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.MonaCN.value,
                   Hashes(map = {ModTypeNames.MonaCN.value: OrderedSet([ModTypeNames.Mona.value])}),
                   Indices(map = {ModTypeNames.MonaCN.value: OrderedSet([ModTypeNames.Mona.value])}),
                   aliases = ["NoMoraCN", "BigHatCN"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def nilou(cls) -> ModType:
        """
        Creates the :class:`ModType` for Nilou

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet
        
        return ModType(ModTypeNames.Nilou.value,
                   Hashes(map = {ModTypeNames.Nilou.value: OrderedSet([ModTypeNames.NilouBreeze.value])}),
                   Indices(map = {ModTypeNames.Nilou.value: OrderedSet([ModTypeNames.NilouBreeze.value])}),
                   aliases = ["Dancer", "Morgiana", "BloomGirl"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    @classmethod
    def nilouBreeze(cls) -> ModType:
        """
        Creates the :class:`ModType` for NilouBreeze

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.NilouBreeze.value, 
                   Hashes(map = {ModTypeNames.NilouBreeze.value: OrderedSet([ModTypeNames.Nilou.value])}),
                   Indices(map = {ModTypeNames.NilouBreeze.value: OrderedSet([ModTypeNames.Nilou.value])}),
                   aliases = ["ForestFairy", "NilouFairy", "DancerBreeze", "MorgianaBreeze", "BloomGirlBreeze",
                              "DancerFairy", "MorgianaFairy", "BloomGirlFairy", "FairyNilou", "FairyDancer", "FairyMorgiana", "FairyBloomGirl"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    @classmethod
    def ningguang(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ningguang

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Ningguang.value,
                   Hashes(map = {ModTypeNames.Ningguang.value: OrderedSet([ModTypeNames.NingguangOrchid.value])}),
                   Indices(map = {ModTypeNames.Ningguang.value: OrderedSet([ModTypeNames.NingguangOrchid.value])}),
                   aliases = ["GeoMommy", "SugarMommy"],
                   vertexCounts = ModDataAssets.VertexCounts.value,
                   iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                   iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def ningguangOrchid(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ningguang

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.NingguangOrchid.value,
                    Hashes(map = {ModTypeNames.NingguangOrchid.value: OrderedSet([ModTypeNames.Ningguang.value])}),
                    Indices(map = {ModTypeNames.NingguangOrchid.value: OrderedSet([ModTypeNames.Ningguang.value])}),
                    aliases = ["NingguangLanternRite", "LanternRiteNingguang", "GeoMommyOrchid", "SugarMommyOrchid", "GeoMommyLaternRite", "SugarMommyLanternRite",
                               "LaternRiteGeoMommy", "LanternRiteSugarMommy"],
                    vertexCounts = ModDataAssets.VertexCounts.value,
                    iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                    iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def raiden(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ei

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Raiden.value,
                     hashes = Hashes(map = {ModTypeNames.Raiden.value: OrderedSet([ModTypeNames.RaidenBoss.value])}), 
                     indices = Indices(),
                     aliases = ["Ei", "RaidenEi", "Shogun", "RaidenShogun", "RaidenShotgun", "Shotgun", "CrydenShogun", "Cryden", "SmolEi"], 
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def rosaria(cls) -> ModType:
        """
        Creates the :class:`ModType` for Rosaria

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Rosaria.value,
                      Hashes(map = {ModTypeNames.Rosaria.value: OrderedSet([ModTypeNames.RosariaCN.value])}), 
                      Indices(map = {ModTypeNames.Rosaria.value: OrderedSet([ModTypeNames.RosariaCN.value])}),
                      aliases = ["GothGirl"],
                      vertexCounts = ModDataAssets.VertexCounts.value,
                      iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                      iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def rosariaCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for RosariaCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.RosariaCN.value,
                      Hashes(map = {ModTypeNames.RosariaCN.value: OrderedSet([ModTypeNames.Rosaria.value])}), 
                      Indices(map = {ModTypeNames.RosariaCN.value: OrderedSet([ModTypeNames.Rosaria.value])}),
                      aliases = ["GothGirlCN"],
                      vertexCounts = ModDataAssets.VertexCounts.value,
                      iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                      iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def shenhe(cls) -> ModType:
        """
        Creates the :class:`ModType` for Shenhe

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Shenhe.value,
                     Hashes(map = {ModTypeNames.Shenhe.value: OrderedSet([ModTypeNames.ShenheFrostFlower.value])}), 
                     Indices(map = {ModTypeNames.Shenhe.value: OrderedSet([ModTypeNames.ShenheFrostFlower.value])}),
                     aliases = ["YelansBestie", "RedRopes"],
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def shenheFrostFlower(cls) -> ModType:
        """
        Creates the :class:`ModType` for ShenheFrostFlower

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.ShenheFrostFlower.value,
                     Hashes(map = {ModTypeNames.ShenheFrostFlower.value: OrderedSet([ModTypeNames.Shenhe.value])}), 
                     Indices(map = {ModTypeNames.ShenheFrostFlower.value: OrderedSet([ModTypeNames.Shenhe.value])}),
                     aliases = ["ShenheLanternRite", "LanternRiteShenhe", "YelansBestieFrostFlower", "YelansBestieLanternRite", "LanternRiteYelansBestie",
                                "RedRopesFrostFlower", "RedRopesLanternRite", "LanternRiteRedRopes"],
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def xiangling(cls) -> ModType:
        """
        Creates the :class:`ModType` for Xiangling

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Xiangling.value,
                     Hashes(map = {ModTypeNames.Xiangling.value: OrderedSet([ModTypeNames.XianglingCheer.value])}), 
                     Indices(map = {ModTypeNames.Xiangling.value: OrderedSet([ModTypeNames.XianglingCheer.value])}),
                     aliases = ["CookingFanatic", "HeadChefoftheWanminRestaurant", "ChefMaosDaughter", "GuobasBuddy"],
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def xianglingCheer(cls) -> ModType:
        """
        Creates the :class:`ModType` for XianglingCheer

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.XianglingCheer.value,
                     Hashes(map = {ModTypeNames.XianglingCheer.value: OrderedSet([ModTypeNames.Xiangling.value])}), 
                     Indices(map = {ModTypeNames.XianglingCheer.value: OrderedSet([ModTypeNames.Xiangling.value])}),
                     aliases = ["XianglingLanternRite", "LanternRiteXiangling", 
                                "CookingFanaticLanternRite", "HeadChefoftheWanminRestaurantLanternRite", "ChefMaosDaughterLanternRite", "GuobasBuddyLanternRite",
                                "LanternRiteCookingFanatic", "LanternRiteHeadChefoftheWanminRestaurant", "LanternRiteChefMaosDaughter", "LanternRiteGuobasBuddy"],
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))

    
    @classmethod
    def xingqiu(cls) -> ModType:
        """
        Creates the :class:`ModType` for Xingqiu

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.Xingqiu.value,
                     Hashes(map = {ModTypeNames.Xingqiu.value: OrderedSet([ModTypeNames.XingqiuBamboo.value])}), 
                     Indices(map = {ModTypeNames.Xingqiu.value: OrderedSet([ModTypeNames.XingqiuBamboo.value])}),
                     aliases = ["GuhuaGeek", "Bookworm", "SecondSonofTheFeiyunCommerceGuild", "ChongyunsBestie"],
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
    
    @classmethod
    def xingqiuBamboo(cls) -> ModType:
        """
        Creates the :class:`ModType` for XingqiuBamboo

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        return ModType(ModTypeNames.XingqiuBamboo.value,
                     Hashes(map = {ModTypeNames.XingqiuBamboo.value: OrderedSet([ModTypeNames.Xingqiu.value])}), 
                     Indices(map = {ModTypeNames.XingqiuBamboo.value: OrderedSet([ModTypeNames.Xingqiu.value])}),
                     aliases = ["XingqiuLanternRite", "GuhuaGeekLanternRite", "BookwormLanternRite", "SecondSonofTheFeiyunCommerceGuildLanternRite", "ChongyunsBestieLanternRite",
                                "LanternRiteXingqiu", "LanternRiteGuhuaGeek", "LanternRiteBookworm", "LanternRiteSecondSonofTheFeiyunCommerceGuild", "LanternRiteChongyunsBestie",
                                "GuhuaGeekBamboo", "BookwormBamboo", "SecondSonofTheFeiyunCommerceGuildBamboo", "ChongyunsBestieBamboo"],
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
##### EndScript