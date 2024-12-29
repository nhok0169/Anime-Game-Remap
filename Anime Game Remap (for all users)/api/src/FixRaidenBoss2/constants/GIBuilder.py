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
from ..constants.IniConsts import IniComments, IniKeywords
from ..constants.Colours import Colours, ColourRanges
from ..constants.ColourConsts import ColourConsts
from ..constants.TexConsts import TexMetadataNames
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
from ..model.strategies.texEditors.TexCreator import TexCreator
from ..model.strategies.texEditors.TexEditor import TexEditor
from ..model.strategies.texEditors.pixelTransforms.ColourReplace import ColourReplace
from ..model.strategies.texEditors.pixelTransforms.HighlightShadow import HighlightShadow
from ..model.strategies.texEditors.pixelTransforms.TempControl import TempControl
from ..model.strategies.texEditors.pixelTransforms.Transparency import Transparency
from ..model.strategies.texEditors.texFilters.HueAdjust import HueAdjust
from ..model.strategies.texEditors.texFilters.PixelFilter import PixelFilter
from ..model.strategies.texEditors.texFilters.TexMetadataFilter import TexMetadataFilter
from ..model.strategies.texEditors.texFilters.GammaFilter import GammaFilter
from ..model.strategies.texEditors.texFilters.InvertAlphaFilter import InvertAlphaFilter
from ..model.files.TextureFile import TextureFile
from ..model.textures.Colour import Colour
from ..model.textures.ColourRange import ColourRange
from ..model.assets.Hashes import Hashes
from ..model.assets.Indices import Indices
from ..model.assets.VGRemaps import VGRemaps
from ..model.strategies.iniFixers.regEditFilters.RegRemap import RegRemap
from ..model.strategies.iniFixers.regEditFilters.RegRemove import RegRemove
from ..model.strategies.iniFixers.regEditFilters.RegTexAdd import RegTexAdd
from ..model.strategies.iniFixers.regEditFilters.RegTexEdit import RegTexEdit
from ..model.strategies.iniFixers.regEditFilters.RegNewVals import RegNewVals
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
    def ayaka(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ayaka

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Ayaka", re.compile(r"^\s*\[\s*TextureOverride.*(Ayaka)((?!(RemapBlend|SpringBloom)).)*Blend.*\s*\]"), 
                    Hashes(map = {"Ayaka": {"AyakaSpringBloom"}}),Indices(map = {"Ayaka": {"AyakaSpringBloom"}}),
                    aliases = ["Ayaya", "Yandere", "NewArchonOfEternity"],
                    vgRemaps = VGRemaps(map = {"Ayaka": {"AyakaSpringBloom"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head", "body", "dress"}],
                                                      kwargs = {"texEdits": {"head": {"ps-t0": {"TransparentDiffuse": TexEditor(filters = [TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}},
                                                                             "body": {"ps-t1": {"BrightLightMap": TexEditor(filters = [PixelFilter(transforms = [Transparency(-78)])])}},
                                                                             "dress": {"ps-t0": {"OpaqueDiffuse": TexEditor(filters = [lambda texFile: TexEditor.setTransparency(texFile, 177),
                                                                                                                                       TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}}}),
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                       RegRemove(remove = {"head": {"ps-t2"},
                                           "body": {"ps-t3"}}),
                       RegTexEdit({"BrightLightMap": ["ps-t1"], "OpaqueDiffuse": ["ps-t0"], "TransparentDiffuse": ["ps-t0"]}),
                       RegRemap(remap = {"head": {"ps-t1": ["ps-t2", "temp"], "ps-t0": ["ps-t0", "ps-t1"]},
                                         "body": {"ps-t2": ["ps-t3"], "ps-t1": ["ps-t2", "temp"], "ps-t0": ["ps-t0", "ps-t1"]}}),
                       RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))},
                                             "body": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}, mustAdd = False),
                       RegNewVals({"head": {"temp": IniKeywords.ORFixPath.value},
                                   "body": {"temp": IniKeywords.ORFixPath.value}}),
                       RegRemap(remap = {"head": {"temp": ["run"]},
                                         "body": {"temp": ["run"]}})
                   ]}))
    
    @classmethod
    def ayakaSpringBloom(cls) -> ModType:
        """
        Creates the :class:`ModType` for AyakaSpringBloom

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("AyakaSpringBloom", re.compile(r"^\s*\[\s*TextureOverride.*(AyakaSpringBloom)((?!(RemapBlend)).)*Blend.*\s*\]"), 
                    Hashes(map = {"AyakaSpringBloom": {"Ayaka"}}),Indices(map = {"AyakaSpringBloom": {"Ayaka"}}),
                    aliases = ["AyayaFontaine", "YandereFontaine", "NewArchonOfEternityFontaine",
                               "FontaineAyaya", "FontaineYandere", "NewFontaineArchonOfEternity",
                               "MusketeerAyaka", "AyakaMusketeer", "AyayaMusketeer"],
                    vgRemaps = VGRemaps(map = {"AyakaSpringBloom": {"Ayaka"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head", "body", "dress"}]),
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                       RegRemove(remove = {"head": {"ps-t0", "ps-t3", "ResourceRefHeadDiffuse", "ResourceRefHeadLightMap", ("run", lambda val: val[1] == IniKeywords.ORFixPath.value)},
                                           "body": {"ps-t0", "ResourceRefBodyDiffuse", "ResourceRefBodyLightMap", ("run", lambda val: val[1] == IniKeywords.ORFixPath.value)},
                                           "dress": {"ps-t3", "ResourceRefDressDiffuse", "ResourceRefDressLightMap", ("run", lambda val: val[1] == IniKeywords.ORFixPath.value)}}),
                       RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]},
                                         "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"]}})
                   ]}))

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
    def diluc(cls) -> ModType:
        """
        Creates the :class:`ModType` for Diluc

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Diluc", re.compile(r"^\s*\[\s*TextureOverride.*(Diluc)((?!RemapBlend|Flamme).)*Blend.*\s*\]"), 
                    Hashes(map = {"Diluc": {"DilucFlamme"}}),Indices(map = {"Diluc": {"DilucFlamme"}}),
                    aliases = ["KaeyasBrother", "DawnWineryMaster", "AngelShareOwner", "DarkNightBlaze"],
                    vgRemaps = VGRemaps(map = {"Diluc": {"DilucFlamme"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body", "dress"}]),
                    iniFixBuilder = IniFixBuilder(GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}]))
    
    @classmethod
    def dilucFlamme(cls) -> ModType:
        """
        Creates the :class:`ModType` for DilucFlamme

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("DilucFlamme", re.compile(r"^\s*\[\s*TextureOverride.*(DilucFlamme)((?!RemapBlend).)*Blend.*\s*\]"), 
                    Hashes(map = {"DilucFlamme": {"Diluc"}}),Indices(map = {"DilucFlamme": {"Diluc"}}),
                    aliases = ["RedDeadOfTheNight", "DarkNightHero"],
                    vgRemaps = VGRemaps(map = {"DilucFlamme": {"Diluc"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body", "dress"}],
                                                      kwargs = {"texEdits": {"body": {"ps-t0": {"TransparentBodyDiffuse": TexEditor(filters = [InvertAlphaFilter(),
                                                                                                                                               PixelFilter(transforms = [ColourReplace(Colour(0, 0, 0, 177), 
                                                                                                                                                                                       coloursToReplace = {ColourRange(Colour(0, 0, 0, 125), Colour(0, 0, 0, 130))})])])}},
                                                                             "dress": {"ps-t0": {"TransparentDressDiffuse": TexEditor(filters = [InvertAlphaFilter()])}}}}),
                    iniFixBuilder = IniFixBuilder(GIMIObjMergeFixer, args = [{"body": ["body", "dress"]}], 
                                                  kwargs = {"copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                                                     RegTexEdit({"TransparentBodyDiffuse": ["ps-t0"], "TransparentDressDiffuse": ["ps-t0"]})
                                                 ]}))
    
    @classmethod
    def fischl(cls) -> ModType:
        """
        Creates the :class:`ModType` for Fischl

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Fischl", re.compile(r"^\s*\[\s*TextureOverride.*(Fischl)((?!RemapBlend|Highness).)*Blend.*\s*\]"), 
                    Hashes(map = {"Fischl": {"FischlHighness"}}),Indices(map = {"Fischl": {"FischlHighness"}}),
                    aliases = ["Amy", "Chunibyo", "8thGraderSyndrome", "Delusional", "PrinzessinderVerurteilung", "MeinFraulein", " FischlvonLuftschlossNarfidort", "PrincessofCondemnation", "TheCondemedPrincess", "OzsMiss"],
                    vgRemaps = VGRemaps(map = {"Fischl": {"FischlHighness"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body", "dress"}]),
                    iniFixBuilder = IniFixBuilder(GIMIObjMergeFixer, args = [{"body": ["body", "dress"]}], kwargs = {"copyPreamble": IniComments.GIMIObjMergerPreamble.value}))
    
    @classmethod
    def fischlHighness(cls) -> ModType:
        """
        Creates the :class:`ModType` for FischlHighness

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("FischlHighness", re.compile(r"^\s*\[\s*TextureOverride.*(FischlHighness)((?!RemapBlend).)*Blend.*\s*\]"), 
                    Hashes(map = {"FischlHighness": {"Fischl"}}),Indices(map = {"FischlHighness": {"Fischl"}}),
                    aliases = ["PrincessAmy", "RealPrinzessinderVerurteilung", "Prinzessin", "PrincessFischlvonLuftschlossNarfidort", "PrinzessinFischlvonLuftschlossNarfidort", "ImmernachtreichPrincess", 
                               "PrinzessinderImmernachtreich", "PrincessoftheEverlastingNight", "OzsPrincess"],
                    vgRemaps = VGRemaps(map = {"FischlHighness": {"Fischl"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"body", "head"}]),
                    iniFixBuilder = IniFixBuilder(GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}], kwargs = {"preRegEditFilters": [
                        RegRemove(remove = {"head": {"ps-t2"}}),
                        RegRemap(remap = {"head": {"ps-t3": ["ps-t2"]}})
                    ]}))
    
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
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head"}], 
                                                      kwargs = {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [lambda texFile: TexEditor.setTransparency(texFile, 0),
                                                                                                                                    TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}}}),
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                        RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]}}),
                        RegTexEdit(textures = {"DarkDiffuse": ["ps-t1"]}),
                        RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}})
                    ]}))
    
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
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                        RegRemove(remove = {"head": {"ps-t0"}}),
                        RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]}})
                    ]}))

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
                   iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head", "dress"}], 
                                                     kwargs = {"texEdits": {"dress": {"ps-t0": {"OpaqueDressDiffuse": TexEditor(filters = [lambda texFile: TexEditor.setTransparency(texFile, 255)])}},
                                                                            "head": {"ps-t0": {"OpaqueHeadDiffuse": TexEditor(filters = [lambda texFile: TexEditor.setTransparency(texFile, 255)])}}}}),
                   iniFixBuilder = IniFixBuilder(GIMIObjMergeFixer, args = [{"head": ["dress", "head"]}], 
                                                 kwargs = {"copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                                                     RegTexEdit({"OpaqueDressDiffuse": ["ps-t0"], "OpaqueHeadDiffuse": ["ps-t0"]})
                                                 ]}))
    
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
    def kirara(cls) -> ModType:
        """
        Creates the :class:`ModType` for Kirara

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Kirara", re.compile(r"^\s*\[\s*TextureOverride.*(Kirara)((?!RemapBlend|Boots).)*Blend.*\s*\]"), 
                    Hashes(map = {"Kirara": {"KiraraBoots"}}),Indices(map = {"Kirara": {"KiraraBoots"}}),
                    aliases = ["Nekomata", "KonomiyaExpress", "CatBox"],
                    vgRemaps = VGRemaps(map = {"Kirara": {"KiraraBoots"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"dress"}], 
                                                      kwargs = {"texEdits": {"dress": {"ps-t2": {"WhitenLightMap": TexEditor(filters = [
                                                          PixelFilter(transforms = [ColourReplace(Colours.White.value, coloursToReplace = {ColourRanges.LightMapGreen.value}, replaceAlpha = False)])
                                                          ])}}}}),
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                        RegRemove(remove = {"dress": {"ps-t0"}}),
                        RegRemap(remap = {"dress": {"ps-t1": ["ps-t0", "ps-t1"]}}),
                        RegTexEdit(textures = {"WhitenLightMap": ["ps-t2"]})
                    ]}))
    
    @classmethod
    def kiraraBoots(cls) -> ModType:
        """
        Creates the :class:`ModType` for KiraraBoots

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("KiraraBoots", re.compile(r"^\s*\[\s*TextureOverride.*(KiraraBoots)((?!RemapBlend).)*Blend.*\s*\]"), 
                    Hashes(map = {"KiraraBoots": {"Kirara"}}),Indices(map = {"KiraraBoots": {"Kirara"}}),
                    aliases = ["NekomataInBoots", "KonomiyaExpressInBoots", "CatBoxWithBoots", "PussInBoots"],
                    vgRemaps = VGRemaps(map = {"KiraraBoots": {"Kirara"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"dress"}]),
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                        RegRemap(remap = {"dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]}}),
                        RegTexAdd(textures = {"dress": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}, mustAdd = False)
                    ]}))
    
    @classmethod
    def klee(cls) -> ModType:
        """
        Creates the :class:`ModType` for Klee

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Klee", re.compile(r"^\s*\[\s*TextureOverride.*(Klee)((?!RemapBlend|BlossomingStarlight).)*Blend.*\s*\]"), 
                    Hashes(map = {"Klee": {"KleeBlossomingStarlight"}}),Indices(map = {"Klee": {"KleeBlossomingStarlight"}}),
                    aliases = ["SparkKnight", "DodocoBuddy", "DestroyerofWorlds"],
                    vgRemaps = VGRemaps(map = {"Klee": {"KleeBlossomingStarlight"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head", "body"}], kwargs = {"texEdits": {"body": {"ps-t1": {"GreenLightMap": TexEditor(filters = [
                                                            PixelFilter(transforms = [ColourReplace(Colour(0, 128, 0, 177), coloursToReplace = {ColourRange(Colour(0, 0, 0, 250), Colour(0, 0, 0, 255)),
                                                                                                                                              ColourRange(Colour(0, 0, 0, 125), Colour(0 ,0 ,0, 130))}, replaceAlpha = True)])
                                                        ])}}}}),
                    iniFixBuilder = IniFixBuilder(GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}], kwargs = {"preRegEditFilters": [
                        RegTexEdit(textures = {"GreenLightMap": ["ps-t1"]}),
                        RegRemap(remap = {"head": {"ps-t2": ["ps-t3"]}})
                    ]}))

    @classmethod
    def kleeBlossomingStarlight(cls) -> ModType:
        """
        Creates the :class:`ModType` for KleeBlossomingStarlight

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("KleeBlossomingStarlight", re.compile(r"^\s*\[\s*TextureOverride.*(KleeBlossomingStarlight)((?!RemapBlend).)*Blend.*\s*\]"), 
                    Hashes(map = {"KleeBlossomingStarlight": {"Klee"}}),Indices(map = {"KleeBlossomingStarlight": {"Klee"}}),
                    aliases = ["RedVelvetMage", "DodocoLittleWitchBuddy", "MagicDestroyerofWorlds", "FlandreScarlet", "ScarletFlandre"],
                    vgRemaps = VGRemaps(map = {"KleeBlossomingStarlight": {"Klee"}}),
                    iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head", "body", "dress"}]),
                    iniFixBuilder = IniFixBuilder(GIMIObjMergeFixer, args = [{"body": ["body", "dress"]}], kwargs = {"copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                                                    RegRemove(remove = {"head": {"ps-t2"}}),
                                                    RegRemap(remap = {"head": {"ps-t3": ["ps-t2"]}})
                                                 ]}))
    
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
    def nilou(cls) -> ModType:
        """
        Creates the :class:`ModType` for Nilou

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """
        return ModType("Nilou", re.compile(r"^\s*\[\s*TextureOverride.*(Nilou)((?!(RemapBlend|Breeze)).)*Blend.*\s*\]"), 
                   Hashes(map = {"Nilou": {"NilouBreeze"}}),Indices(map = {"Nilou": {"NilouBreeze"}}),
                   aliases = ["Dancer", "Morgiana", "BloomGirl"],
                   vgRemaps = VGRemaps(map = {"Nilou": {"NilouBreeze"}}),
                   iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head", "body", "dress"}]),
                   iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                       RegRemove(remove = {"head": {"ps-t0"}, "body": {"ps-t0"}, "dress": {"ps-t0"}}),
                       RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"]},
                                         "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"]},
                                         "dress": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"]}}),
                       RegNewVals(vals = {"head": {"ResourceRefHeadDiffuse": "reference ps-t0",
                                                   "ResourceRefHeadLightMap": "reference ps-t1"},
                                          "body": {"ResourceRefBodyDiffuse": "reference ps-t0",
                                                   "ResourceRefBodyDiffuse": "reference ps-t0"},
                                          "dress": {"ResourceRefDressDiffuse": "reference ps-t0",
                                                    "ResourceRefDressLightMap": "reference ps-t1"}})
                   ]}))

    @classmethod
    def nilouBreeze(cls) -> ModType:
        """
        Creates the :class:`ModType` for NilouBreeze

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """ 
        return ModType("NilouBreeze", re.compile(r"^\s*\[\s*TextureOverride.*(NilouBreeze)((?!(RemapBlend)).)*Blend.*\s*\]"), 
                   Hashes(map = {"NilouBreeze": {"Nilou"}}),Indices(map = {"NilouBreeze": {"Nilou"}}),
                   aliases = ["ForestFairy", "NilouFairy", "DancerBreeze", "MorgianaBreeze", "BloomGirlBreeze",
                              "DancerFairy", "MorgianaFairy", "BloomGirlFairy", "FairyNilou", "FairyDancer", "FairyMorgiana", "FairyBloomGirl"],
                   vgRemaps = VGRemaps(map = {"NilouBreeze": {"Nilou"}}),
                   iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head", "dress", "body"}]),
                   iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                       RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"]},
                                         "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"]},
                                         "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"]}}),
                       RegNewVals(vals = {"head": {"temp": IniKeywords.ORFixPath.value},
                                          "dress": {"temp": IniKeywords.ORFixPath.value},
                                          "body": {"temp": IniKeywords.ORFixPath.value}}),
                       RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)},
                                             "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)},
                                             "dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)}}, mustAdd = False),
                       RegRemap(remap = {"head": {"temp": ["run"]},
                                         "dress": {"temp": ["run"]},
                                         "body": {"temp": ["run"]}})
                   ]}))

    @classmethod
    def ningguang(cls) -> ModType:
        """
        Creates the :class:`ModType` for Ningguang

        Returns 
        -------
        :class:`ModType`
            The resultant :class:`ModType`
        """

        hueFilter = HueAdjust(-2)

        return ModType("Ningguang", re.compile(r"^\s*\[\s*TextureOverride.*(Ningguang)((?!(RemapBlend|Orchid)).)*Blend.*\s*\]"), 
                   Hashes(map = {"Ningguang": {"NingguangOrchid"}}),Indices(map = {"Ningguang": {"NingguangOrchid"}}),
                   aliases = ["GeoMommy", "SugarMommy"],
                   vgRemaps = VGRemaps(map = {"Ningguang": {"NingguangOrchid"}}),
                   iniParseBuilder = IniParseBuilder(GIMIObjParser, args = [{"head"}], 
                                                      kwargs = {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [lambda texFile: TexEditor.setTransparency(texFile, 0),
                                                                                                                                    TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}}}), 
                    iniFixBuilder = IniFixBuilder(GIMIObjRegEditFixer, kwargs = {"preRegEditFilters": [
                        RegTexEdit({"DarkDiffuse": ["ps-t0"]})
                    ]}))
    
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
                     iniFixBuilder = IniFixBuilder(GIMIObjSplitFixer, args = [{"dress": ["dress", "extra"]}], kwargs = {"preRegEditFilters": [
                         RegRemove(remove = {"dress": ["ps-t2"]}),
                         RegRemap(remap = {"dress": {"ps-t3": ["ps-t2"]}})
                     ]}))
    
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