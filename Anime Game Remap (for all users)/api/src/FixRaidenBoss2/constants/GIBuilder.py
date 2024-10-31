##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
import re
##### EndExtImports

##### LocalImports
from ..constants.IniConsts import IniComments
from .ModTypeBuilder import ModTypeBuilder
from ..model.strategies.ModType import ModType
from ..model.strategies.iniParsers.IniParseBuilder import IniParseBuilder
from ..model.strategies.iniParsers.GIMIObjParser import GIMIObjParser
from ..model.strategies.iniFixers.IniFixBuilder import IniFixBuilder
from ..model.strategies.iniFixers.GIMIFixer import GIMIFixer
from ..model.strategies.iniFixers.GIMIObjSplitFixer import GIMIObjSplitFixer
from ..model.strategies.iniFixers.GIMIObjMergeFixer import GIMIObjMergeFixer
from ..model.strategies.iniFixers.GIMIObjRegEditFixer import GIMIObjRegEditFixer
from ..model.strategies.iniFixers.MultiModFixer import MultiModFixer
from ..model.assets.Hashes import Hashes
from ..model.assets.Indices import Indices
from ..model.assets.VGRemaps import VGRemaps
##### EndLocalImports


##### Script
class GIBuilder(ModTypeBuilder):
    """
    This Class inherits from :class:`ModTypeBuilder`

    Creates new :class:`ModType` objects for some anime game
    """

    @classmethod
    def amber(cls) -> ModType:
        """
        Creates the :class:`ModType` for Amber

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Amber", re.compile(r"^\s*\[\s*TextureOverride.*(Amber)((?!(RemapBlend|CN)).)*Blend.*\s*\]"), 
                    Hashes(map = {"Amber": {"AmberCN"}}),Indices(map = {"Amber": {"AmberCN"}}),
                    aliases = ["BaronBunny", "ColleisBestie"],
                    vgRemaps = VGRemaps(map = {"Amber": {"AmberCN"}}))

    @classmethod
    def amberCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for AmberCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("AmberCN", re.compile(r"^\s*\[\s*TextureOverride.*(AmberCN)((?!RemapBlend).)*Blend.*\s*\]"), 
                    Hashes(map = {"AmberCN": {"Amber"}}),Indices(map = {"AmberCN": {"Amber"}}),
                    aliases = ["BaronBunnyCN", "ColleisBestieCN"],
                    vgRemaps = VGRemaps(map = {"AmberCN": {"Amber"}}))

    @classmethod
    def arlecchino(cls) -> ModType:
        """
        Creates the :class:`ModType` for Arlecchino

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Arlecchino", re.compile(r"^\s*\[\s*TextureOverride.*(Arlecchino)((?!RemapBlend).)*Blend.*\s*\]"), 
                    Hashes(map = {"Arlecchino": {"ArlecchinoBoss"}}), Indices(map = {"Arlecchino": {"ArlecchinoBoss"}}),
                    aliases = ["Father", "Knave", "Perrie", "Peruere", "Harlequin"],
                    vgRemaps = VGRemaps(map = {"Arlecchino": {"ArlecchinoBoss"}}))
    
    @classmethod
    def barbara(cls) -> ModType:
        """
        Creates the :class:`ModType` for Barbara

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Barbara", re.compile(r"^\s*\[\s*TextureOverride.*(Barbara)((?!RemapBlend|Summertime).)*Blend.*\s*\]"), 
                    Hashes(map = {"Barbara": {"BarbaraSummertime"}}),Indices(map = {"Barbara": {"BarbaraSummertime"}}),
                    aliases = ["Idol", "Healer"],
                    vgRemaps = VGRemaps(map = {"Barbara": {"BarbaraSummertime"}}))
    
    @classmethod
    def barbaraSummerTime(cls) -> ModType:
        """
        Creates the :class:`ModType` for BarbaraSummerTime

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("BarbaraSummertime", re.compile(r"^\s*\[\s*TextureOverride.*(BarbaraSummertime)((?!RemapBlend).)*Blend.*\s*\]"), 
                    Hashes(map = {"BarbaraSummertime": {"Barbara"}}),Indices(map = {"BarbaraSummertime": {"Barbara"}}),
                    aliases = ["IdolSummertime", "HealerSummertime", "BarbaraBikini"],
                    vgRemaps = VGRemaps(map = {"BarbaraSummertime": {"Barbara"}}))
    
    @classmethod
    def ganyu(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ganyu

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Ganyu", re.compile(r"^\s*\[\s*TextureOverride.*(Ganyu)((?!(RemapBlend|Twilight)).)*Blend.*\s*\]"), 
                    Hashes(map = {"Ganyu": {"GanyuTwilight"}}),Indices(map = {"Ganyu": {"GanyuTwilight"}}),
                    aliases = ["Cocogoat"],
                    vgRemaps = VGRemaps(map = {"Ganyu": {"GanyuTwilight"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head"}]),
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"regRemap": {"head": {"ps-t0": ["ps-t1"], "ps-t1": ["ps-t0", "ps-t2"]}}}))
    
    @classmethod
    def ganyuTwilight(cls) -> ModType:
        """
        Creates the :class:`ModType` for GanyuTwilight

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("GanyuTwilight", re.compile(r"^\s*\[\s*TextureOverride.*(GanyuTwilight)((?!(RemapBlend)).)*Blend.*\s*\]"), 
                    Hashes(map = {"GanyuTwilight": {"Ganyu"}}),Indices(map = {"GanyuTwilight": {"Ganyu"}}),
                    aliases = ["GanyuLanternRite", "LanternRiteGanyu", "CocogoatTwilight", "CocogoatLanternRite", "LanternRiteCocogoat"],
                    vgRemaps = VGRemaps(map = {"GanyuTwilight": {"Ganyu"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head"}]),
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"regRemove": {"head": {"ps-t0"}}, "regRemap": {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]}}}))

    @classmethod
    def jean(cls) -> ModType:
        """
        Creates the :class:`ModType` for Jean

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Jean", re.compile(r"^\s*\[\s*TextureOverride.*(Jean)((?!(RemapBlend|CN|Sea)).)*Blend.*\s*\]"), 
                   Hashes(map = {"Jean": {"JeanCN", "JeanSea"}}), Indices(map = {"Jean": {"JeanCN", "JeanSea"}}),
                   aliases = ["ActingGrandMaster", "KleesBabySitter"],
                   vgRemaps = VGRemaps(map = {"Jean": {"JeanCN", "JeanSea"}}),
                   iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body"}]),
                   iniFixBuilder = IniFixBuilder(MultiModFixer, args = [{"JeanCN": IniFixBuilder(GIMIFixer), "JeanSea": IniFixBuilder(GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}])}]))
    
    @classmethod
    def jeanCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for JeanCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("JeanCN", re.compile(r"^\s*\[\s*TextureOverride.*(JeanCN)((?!RemapBlend|Sea).)*Blend.*\s*\]"), 
                   Hashes(map = {"JeanCN": {"Jean", "JeanSea"}}), Indices(map = {"JeanCN": {"Jean", "JeanSea"}}),
                   aliases = ["ActingGrandMasterCN", "KleesBabySitterCN"],
                   vgRemaps = VGRemaps(map = {"JeanCN": {"Jean", "JeanSea"}}),
                   iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body"}]),
                   iniFixBuilder = IniFixBuilder(MultiModFixer, args = [{"Jean": IniFixBuilder(GIMIFixer), "JeanSea": IniFixBuilder(GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}])}]))
    
    @classmethod
    def jeanSea(cls) -> ModType:
        """
        Creates the :class:`ModType` for JeanSea

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("JeanSea", re.compile(r"^\s*\[\s*TextureOverride.*(JeanSea)((?!RemapBlend|CN).)*Blend.*\s*\]"), 
                   Hashes(map = {"JeanSea": {"Jean", "JeanCN"}}), Indices(map = {"JeanSea": {"Jean", "JeanCN"}}),
                   aliases = ["ActingGrandMasterSea", "KleesBabySitterSea"],
                   vgRemaps = VGRemaps(map = {"JeanSea": {"Jean", "JeanCN"}}),
                   iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body", "dress"}]),
                   iniFixBuilder = IniFixBuilder(GIMIObjMergeFixer, args = [{"body": ["body", "dress"]}], kwargs = {"copyPreamble": IniComments.GIMIObjMergerPreamble.value}))
    
    @classmethod
    def keqing(cls) -> ModType:
        """
        Creates the :class:`ModType` for Keqing

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Keqing", re.compile(r"^\s*\[\s*TextureOverride.*(Keqing)((?!(RemapBlend|Opulent)).)*Blend.*\s*\]"), 
                   Hashes(map = {"Keqing": {"KeqingOpulent"}}),Indices(map = {"Keqing": {"KeqingOpulent"}}),
                   aliases = ["Kequeen", "ZhongliSimp", "MoraxSimp"],
                   vgRemaps = VGRemaps(map = {"Keqing": {"KeqingOpulent"}}),
                   iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body", "dress"}]),
                   iniFixBuilder = IniFixBuilder(GIMIObjMergeFixer, args = [{"body": ["body", "dress"]}], kwargs = {"copyPreamble": IniComments.GIMIObjMergerPreamble.value}))
    
    @classmethod
    def keqingOpulent(cls) -> ModType:
        """
        Creates the :class:`ModType` for KeqingOpulent

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("KeqingOpulent", re.compile(r"^\s*\[\s*TextureOverride.*(KeqingOpulent)((?!RemapBlend).)*Blend.*\s*\]"), 
            Hashes(map = {"KeqingOpulent": {"Keqing"}}),Indices(map = {"KeqingOpulent": {"Keqing"}}),
            aliases = ["LanternRiteKeqing", "KeqingLaternRite", "CuterKequeen", "LanternRiteKequeen", "KequeenLanternRite", "KequeenOpulent", "CuterKeqing", 
                       "ZhongliSimpOpulent", "MoraxSimpOpulent", "ZhongliSimpLaternRite", "MoraxSimpLaternRite", "LaternRiteZhongliSimp", "LaternRiteMoraxSimp"],
            vgRemaps = VGRemaps(map = {"KeqingOpulent": {"Keqing"}}), 
            iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body"}]),
            iniFixBuilder = IniFixBuilder(GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}]))
    
    @classmethod
    def mona(cls) -> ModType:
        """
        Creates the :class:`ModType` for Mona

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Mona", re.compile(r"^\s*\[\s*TextureOverride.*(Mona)((?!(RemapBlend|CN)).)*Blend.*\s*\]"), 
                   Hashes(map = {"Mona": {"MonaCN"}}),Indices(map = {"Mona": {"MonaCN"}}),
                   aliases = ["NoMora", "BigHat"],
                   vgRemaps = VGRemaps(map = {"Mona": {"MonaCN"}}))
    
    @classmethod
    def monaCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for MonaCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("MonaCN", re.compile(r"^\s*\[\s*TextureOverride.*(MonaCN)((?!RemapBlend).)*Blend.*\s*\]"), 
                   Hashes(map = {"MonaCN": {"Mona"}}),Indices(map = {"MonaCN": {"Mona"}}),
                   aliases = ["NoMoraCN", "BigHatCN"],
                   vgRemaps = VGRemaps(map = {"MonaCN": {"Mona"}}))
    
    @classmethod
    def ningguang(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ningguang

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Ningguang", re.compile(r"^\s*\[\s*TextureOverride.*(Ningguang)((?!(RemapBlend|Orchid)).)*Blend.*\s*\]"), 
                   Hashes(map = {"Ningguang": {"NingguangOrchid"}}),Indices(map = {"Ningguang": {"NingguangOrchid"}}),
                   aliases = ["GeoMommy", "SugarMommy"],
                   vgRemaps = VGRemaps(map = {"Ningguang": {"NingguangOrchid"}}))
    
    @classmethod
    def ningguangOrchid(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ningguang

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("NingguangOrchid", re.compile(r"^\s*\[\s*TextureOverride.*(NingguangOrchid)((?!RemapBlend).)*Blend.*\s*\]"), 
                    Hashes(map = {"NingguangOrchid": {"Ningguang"}}),Indices(map = {"NingguangOrchid": {"Ningguang"}}),
                    aliases = ["NingguangLanternRite", "LanternRiteNingguang", "GeoMommyOrchid", "SugarMommyOrchid", "GeoMommyLaternRite", "SugarMommyLanternRite",
                               "LaternRiteGeoMommy", "LanternRiteSugarMommy"],
                    vgRemaps = VGRemaps(map = {"NingguangOrchid": {"Ningguang"}}))
    
    @classmethod
    def raiden(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ei

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Raiden", re.compile(r"^\s*\[\s*TextureOverride.*(Raiden|Shogun)((?!RemapBlend).)*Blend.*\s*\]"), 
                     hashes = Hashes(map = {"Raiden": {"RaidenBoss"}}), indices = Indices(),
                     aliases = ["Ei", "RaidenEi", "Shogun", "RaidenShogun", "RaidenShotgun", "Shotgun", "CrydenShogun", "Cryden", "SmolEi"], 
                     vgRemaps = VGRemaps(map = {"Raiden": {"RaidenBoss"}}))
    
    @classmethod
    def rosaria(cls) -> ModType:
        """
        Creates the :class:`ModType` for Rosaria

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Rosaria", re.compile(r"^\s*\[\s*TextureOverride.*(Rosaria)((?!(RemapBlend|CN)).)*Blend.*\s*\]"), 
                      Hashes(map = {"Rosaria": {"RosariaCN"}}), Indices(map = {"Rosaria": {"RosariaCN"}}),
                      aliases = ["GothGirl"],
                      vgRemaps = VGRemaps(map = {"Rosaria": {"RosariaCN"}}))
    
    @classmethod
    def rosariaCN(cls) -> ModType:
        """
        Creates the :class:`ModType` for RosariaCN

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("RosariaCN", re.compile(r"^\s*\[\s*TextureOverride.*(RosariaCN)((?!RemapBlend).)*Blend.*\s*\]"), 
                      Hashes(map = {"RosariaCN": {"Rosaria"}}), Indices(map = {"RosariaCN": {"Rosaria"}}),
                      aliases = ["GothGirlCN"],
                      vgRemaps = VGRemaps(map = {"RosariaCN": {"Rosaria"}}))
    
    @classmethod
    def shenhe(cls) -> ModType:
        """
        Creates the :class:`ModType` for Shenhe

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Shenhe", re.compile(r"^\s*\[\s*TextureOverride.*(Shenhe)((?!RemapBlend|FrostFlower).)*Blend.*\s*\]"), 
                     Hashes(map = {"Shenhe": {"ShenheFrostFlower"}}), Indices(map = {"Shenhe": {"ShenheFrostFlower"}}),
                     aliases = ["YelansBestie", "RedRopes"],
                     vgRemaps = VGRemaps(map = {"Shenhe": {"ShenheFrostFlower"}}),
                     iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"dress"}]),
                     iniFixBuilder = IniFixBuilder(GIMIObjSplitFixer, args = [{"dress": ["dress", "extra"]}], 
                                                   kwargs = {"regRemove": {"dress": ["ps-t2"]}, "regRemap": {"dress": {"ps-t3": ["ps-t2"]}}}))
    
    @classmethod
    def shenheFrostFlower(cls) -> ModType:
        """
        Creates the :class:`ModType` for ShenheFrostFlower

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("ShenheFrostFlower", re.compile(r"^\s*\[\s*TextureOverride.*(ShenheFrostFlower)((?!RemapBlend).)*Blend.*\s*\]"), 
                     Hashes(map = {"ShenheFrostFlower": {"Shenhe"}}), Indices(map = {"ShenheFrostFlower": {"Shenhe"}}),
                     aliases = ["ShenheLanternRite", "LanternRiteShenhe", "YelansBestieFrostFlower", "YelansBestieLanternRite", "LanternRiteYelansBestie",
                                "RedRopesFrostFlower", "RedRopesLanternRite", "LanternRiteRedRopes"],
                     vgRemaps = VGRemaps(map = {"ShenheFrostFlower": {"Shenhe"}}),
                     iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"dress", "extra"}]),
                     iniFixBuilder = IniFixBuilder(GIMIObjMergeFixer, args = [{"dress": ["dress", "extra"]}], kwargs = {"copyPreamble": IniComments.GIMIObjMergerPreamble.value}))
##### EndScript