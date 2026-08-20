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
from ..core import ModTypeId, ModTypeIdTools
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Amber), 
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Amber): OrderedSet([ModTypeIdTools.getName(ModTypeId.AmberCN)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Amber): OrderedSet([ModTypeIdTools.getName(ModTypeId.AmberCN)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.AmberCN), 
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.AmberCN): OrderedSet([ModTypeIdTools.getName(ModTypeId.Amber)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.AmberCN): OrderedSet([ModTypeIdTools.getName(ModTypeId.Amber)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Ayaka),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Ayaka): OrderedSet([ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Ayaka): OrderedSet([ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): OrderedSet([ModTypeIdTools.getName(ModTypeId.Ayaka)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): OrderedSet([ModTypeIdTools.getName(ModTypeId.Ayaka)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Arlecchino),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Arlecchino): OrderedSet([ModTypeIdTools.getName(ModTypeId.ArlecchinoBoss)])}), 
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Arlecchino): OrderedSet([ModTypeIdTools.getName(ModTypeId.ArlecchinoBoss)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Barbara),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Barbara): OrderedSet([ModTypeIdTools.getName(ModTypeId.BarbaraSummertime)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Barbara): OrderedSet([ModTypeIdTools.getName(ModTypeId.BarbaraSummertime)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.BarbaraSummertime), 
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.BarbaraSummertime): OrderedSet([ModTypeIdTools.getName(ModTypeId.Barbara)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.BarbaraSummertime): OrderedSet([ModTypeIdTools.getName(ModTypeId.Barbara)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.CherryHuTao), 
                     Hashes(map = {ModTypeIdTools.getName(ModTypeId.CherryHuTao): OrderedSet([ModTypeIdTools.getName(ModTypeId.HuTao)])}), 
                     Indices(map = {ModTypeIdTools.getName(ModTypeId.CherryHuTao): OrderedSet([ModTypeIdTools.getName(ModTypeId.HuTao)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Diluc),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Diluc): OrderedSet([ModTypeIdTools.getName(ModTypeId.DilucFlamme)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Diluc): OrderedSet([ModTypeIdTools.getName(ModTypeId.DilucFlamme)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.DilucFlamme),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.DilucFlamme): OrderedSet([ModTypeIdTools.getName(ModTypeId.Diluc)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.DilucFlamme): OrderedSet([ModTypeIdTools.getName(ModTypeId.Diluc)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Fischl),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Fischl): OrderedSet([ModTypeIdTools.getName(ModTypeId.FischlHighness)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Fischl): OrderedSet([ModTypeIdTools.getName(ModTypeId.FischlHighness)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.FischlHighness),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.FischlHighness): {ModTypeIdTools.getName(ModTypeId.Fischl)}}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.FischlHighness): {ModTypeIdTools.getName(ModTypeId.Fischl)}}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Ganyu),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Ganyu): OrderedSet([ModTypeIdTools.getName(ModTypeId.GanyuTwilight)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Ganyu): OrderedSet([ModTypeIdTools.getName(ModTypeId.GanyuTwilight)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.GanyuTwilight),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.GanyuTwilight): OrderedSet([ModTypeIdTools.getName(ModTypeId.Ganyu)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.GanyuTwilight): OrderedSet([ModTypeIdTools.getName(ModTypeId.Ganyu)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.HuTao), 
                     Hashes(map = {ModTypeIdTools.getName(ModTypeId.HuTao): OrderedSet([ModTypeIdTools.getName(ModTypeId.CherryHuTao)])}), 
                     Indices(map = {ModTypeIdTools.getName(ModTypeId.HuTao): OrderedSet([ModTypeIdTools.getName(ModTypeId.CherryHuTao)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Jean),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.Jean): OrderedSet([ModTypeIdTools.getName(ModTypeId.JeanCN), ModTypeIdTools.getName(ModTypeId.JeanSea)])}), 
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.Jean): OrderedSet([ModTypeIdTools.getName(ModTypeId.JeanCN), ModTypeIdTools.getName(ModTypeId.JeanSea)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.JeanCN),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.JeanCN): OrderedSet([ModTypeIdTools.getName(ModTypeId.Jean), ModTypeIdTools.getName(ModTypeId.JeanSea)])}), 
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.JeanCN): OrderedSet([ModTypeIdTools.getName(ModTypeId.Jean), ModTypeIdTools.getName(ModTypeId.JeanSea)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.JeanSea),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.JeanSea): OrderedSet([ModTypeIdTools.getName(ModTypeId.Jean), ModTypeIdTools.getName(ModTypeId.JeanCN)])}), 
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.JeanSea): OrderedSet([ModTypeIdTools.getName(ModTypeId.Jean), ModTypeIdTools.getName(ModTypeId.JeanCN)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Kaeya),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.Kaeya): OrderedSet([ModTypeIdTools.getName(ModTypeId.KaeyaSailwind)])}),
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.Kaeya): OrderedSet([ModTypeIdTools.getName(ModTypeId.KaeyaSailwind)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.KaeyaSailwind),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.KaeyaSailwind): OrderedSet([ModTypeIdTools.getName(ModTypeId.Kaeya)])}),
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.KaeyaSailwind): OrderedSet([ModTypeIdTools.getName(ModTypeId.Kaeya)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Keqing),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.Keqing): OrderedSet([ModTypeIdTools.getName(ModTypeId.KeqingOpulent)])}),
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.Keqing): OrderedSet([ModTypeIdTools.getName(ModTypeId.KeqingOpulent)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.KeqingOpulent),
            Hashes(map = {ModTypeIdTools.getName(ModTypeId.KeqingOpulent): OrderedSet([ModTypeIdTools.getName(ModTypeId.Keqing)])}),
            Indices(map = {ModTypeIdTools.getName(ModTypeId.KeqingOpulent): OrderedSet([ModTypeIdTools.getName(ModTypeId.Keqing)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Kirara),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Kirara): OrderedSet([ModTypeIdTools.getName(ModTypeId.KiraraBoots)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Kirara): OrderedSet([ModTypeIdTools.getName(ModTypeId.KiraraBoots)])}),
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
        
        return ModType(ModTypeIdTools.getName(ModTypeId.KiraraBoots),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.KiraraBoots): OrderedSet([ModTypeIdTools.getName(ModTypeId.Kirara)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.KiraraBoots): OrderedSet([ModTypeIdTools.getName(ModTypeId.Kirara)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Klee),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Klee): OrderedSet([ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Klee): OrderedSet([ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight): OrderedSet([ModTypeIdTools.getName(ModTypeId.Klee)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight): OrderedSet([ModTypeIdTools.getName(ModTypeId.Klee)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Lisa),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.Lisa): OrderedSet([ModTypeIdTools.getName(ModTypeId.LisaStudent)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.Lisa): OrderedSet([ModTypeIdTools.getName(ModTypeId.LisaStudent)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.LisaStudent),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.LisaStudent): OrderedSet([ModTypeIdTools.getName(ModTypeId.Lisa)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.LisaStudent): OrderedSet([ModTypeIdTools.getName(ModTypeId.Lisa)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Mona),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.Mona): OrderedSet([ModTypeIdTools.getName(ModTypeId.MonaCN)])}),
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.Mona): OrderedSet([ModTypeIdTools.getName(ModTypeId.MonaCN)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.MonaCN),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.MonaCN): OrderedSet([ModTypeIdTools.getName(ModTypeId.Mona)])}),
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.MonaCN): OrderedSet([ModTypeIdTools.getName(ModTypeId.Mona)])}),
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
        
        return ModType(ModTypeIdTools.getName(ModTypeId.Nilou),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.Nilou): OrderedSet([ModTypeIdTools.getName(ModTypeId.NilouBreeze)])}),
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.Nilou): OrderedSet([ModTypeIdTools.getName(ModTypeId.NilouBreeze)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.NilouBreeze), 
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.NilouBreeze): OrderedSet([ModTypeIdTools.getName(ModTypeId.Nilou)])}),
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.NilouBreeze): OrderedSet([ModTypeIdTools.getName(ModTypeId.Nilou)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Ningguang),
                   Hashes(map = {ModTypeIdTools.getName(ModTypeId.Ningguang): OrderedSet([ModTypeIdTools.getName(ModTypeId.NingguangOrchid)])}),
                   Indices(map = {ModTypeIdTools.getName(ModTypeId.Ningguang): OrderedSet([ModTypeIdTools.getName(ModTypeId.NingguangOrchid)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.NingguangOrchid),
                    Hashes(map = {ModTypeIdTools.getName(ModTypeId.NingguangOrchid): OrderedSet([ModTypeIdTools.getName(ModTypeId.Ningguang)])}),
                    Indices(map = {ModTypeIdTools.getName(ModTypeId.NingguangOrchid): OrderedSet([ModTypeIdTools.getName(ModTypeId.Ningguang)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Raiden),
                     hashes = Hashes(map = {ModTypeIdTools.getName(ModTypeId.Raiden): OrderedSet([ModTypeIdTools.getName(ModTypeId.RaidenBoss)])}), 
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Rosaria),
                      Hashes(map = {ModTypeIdTools.getName(ModTypeId.Rosaria): OrderedSet([ModTypeIdTools.getName(ModTypeId.RosariaCN)])}), 
                      Indices(map = {ModTypeIdTools.getName(ModTypeId.Rosaria): OrderedSet([ModTypeIdTools.getName(ModTypeId.RosariaCN)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.RosariaCN),
                      Hashes(map = {ModTypeIdTools.getName(ModTypeId.RosariaCN): OrderedSet([ModTypeIdTools.getName(ModTypeId.Rosaria)])}), 
                      Indices(map = {ModTypeIdTools.getName(ModTypeId.RosariaCN): OrderedSet([ModTypeIdTools.getName(ModTypeId.Rosaria)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Shenhe),
                     Hashes(map = {ModTypeIdTools.getName(ModTypeId.Shenhe): OrderedSet([ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)])}), 
                     Indices(map = {ModTypeIdTools.getName(ModTypeId.Shenhe): OrderedSet([ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower),
                     Hashes(map = {ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower): OrderedSet([ModTypeIdTools.getName(ModTypeId.Shenhe)])}), 
                     Indices(map = {ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower): OrderedSet([ModTypeIdTools.getName(ModTypeId.Shenhe)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Xiangling),
                     Hashes(map = {ModTypeIdTools.getName(ModTypeId.Xiangling): OrderedSet([ModTypeIdTools.getName(ModTypeId.XianglingCheer)])}), 
                     Indices(map = {ModTypeIdTools.getName(ModTypeId.Xiangling): OrderedSet([ModTypeIdTools.getName(ModTypeId.XianglingCheer)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.XianglingCheer),
                     Hashes(map = {ModTypeIdTools.getName(ModTypeId.XianglingCheer): OrderedSet([ModTypeIdTools.getName(ModTypeId.Xiangling)])}), 
                     Indices(map = {ModTypeIdTools.getName(ModTypeId.XianglingCheer): OrderedSet([ModTypeIdTools.getName(ModTypeId.Xiangling)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.Xingqiu),
                     Hashes(map = {ModTypeIdTools.getName(ModTypeId.Xingqiu): OrderedSet([ModTypeIdTools.getName(ModTypeId.XingqiuBamboo)])}), 
                     Indices(map = {ModTypeIdTools.getName(ModTypeId.Xingqiu): OrderedSet([ModTypeIdTools.getName(ModTypeId.XingqiuBamboo)])}),
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

        return ModType(ModTypeIdTools.getName(ModTypeId.XingqiuBamboo),
                     Hashes(map = {ModTypeIdTools.getName(ModTypeId.XingqiuBamboo): OrderedSet([ModTypeIdTools.getName(ModTypeId.Xingqiu)])}), 
                     Indices(map = {ModTypeIdTools.getName(ModTypeId.XingqiuBamboo): OrderedSet([ModTypeIdTools.getName(ModTypeId.Xingqiu)])}),
                     aliases = ["XingqiuLanternRite", "GuhuaGeekLanternRite", "BookwormLanternRite", "SecondSonofTheFeiyunCommerceGuildLanternRite", "ChongyunsBestieLanternRite",
                                "LanternRiteXingqiu", "LanternRiteGuhuaGeek", "LanternRiteBookworm", "LanternRiteSecondSonofTheFeiyunCommerceGuild", "LanternRiteChongyunsBestie",
                                "GuhuaGeekBamboo", "BookwormBamboo", "SecondSonofTheFeiyunCommerceGuildBamboo", "ChongyunsBestieBamboo"],
                     vertexCounts = ModDataAssets.VertexCounts.value,
                     iniParseBuilder = IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value),
                     iniFixBuilder = IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value))
##### EndScript