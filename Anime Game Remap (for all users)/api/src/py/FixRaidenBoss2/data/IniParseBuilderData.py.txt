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
from typing import Tuple, List, Dict, Any
##### EndExtImports

##### LocalImports
from ..constants.Colours import Colours, ColourRanges
from ..core import ModTypeId, ModTypeIdTools
from ..constants.TexConsts import TexMetadataNames
from ..constants.ColourConsts import ColourConsts
from ..constants.IniConsts import IniKeywords
from ..core import BaseIniParser
from ..core import GIMIParser
from ..model.strategies.iniParsers.GIMIObjParserOld import GIMIObjParser
from ..model.strategies.texEditors.TexEditor import TexEditor
from ..model.strategies.texEditors.texFilters.InvertAlphaFilter import InvertAlphaFilter
from ..model.strategies.texEditors.texFilters.ColourReplaceFilter import ColourReplaceFilter
from ..model.strategies.texEditors.texFilters.TransparencyAdjustFilter import TransparencyAdjustFilter
from ..model.strategies.texEditors.texFilters.TexMetadataFilter import TexMetadataFilter
from ..model.files.TextureFile import TextureFile
from ..model.textures.Colour import Colour
from ..model.textures.ColourRange import ColourRange
from .FileDownloadData import FileDownloadData
##### EndLocalImports


##### Script
# IniParseBuilderFunc: Class to define how the IniParseBuilder arguments for some
#   mod is built for a particular game version
class IniParseBuilderFuncs():
    @classmethod
    def giDefault(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIParser, [], {})
    
    @classmethod
    def amber4_0(cls):
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Amber)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Amber)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Amber)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Amber)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Amber)]["body"]},})
    
    @classmethod
    def amberCN4_0(cls):
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AmberCN)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AmberCN)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AmberCN)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AmberCN)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AmberCN)]["body"]}})

    @classmethod
    def _ayakaEditDressDiffuse(cls, texFile: TextureFile) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        TexEditor.setTransparency(texFile, 177)

    @classmethod
    def _ayakaEditHeadDiffuse(cls, texFile: TextureFile) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        TexEditor.setTransparency(texFile, 1)

    @classmethod
    def ayaka4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}],
                {"texEdits": {"head": {"ps-t0": {"TransparentDiffuse": TexEditor(filters = [TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value}),
                                                                                            cls._ayakaEditHeadDiffuse])}},
                              "body": {"ps-t1": {"BrightLightMap": TexEditor(filters = [TransparencyAdjustFilter(-78)])}},
                              "dress": {"ps-t0": {"OpaqueDiffuse": TexEditor(filters = [cls._ayakaEditDressDiffuse,
                                                                                        TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ayaka)][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ayaka)][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ayaka)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ayaka)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ayaka)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ayaka)]["dress"]}})
    
    @classmethod
    def _ayakaSpringbloomEditLightMap5_6(cls, texFile: TextureFile):
        alphaImg = texFile.img.getchannel('A')
        alphaImg = alphaImg.point(lambda alphaPixel: Colour.boundColourChannel(alphaPixel + 200) if (alphaPixel <= 200) else alphaPixel)
        texFile.img.putalpha(alphaImg)

    @classmethod
    def ayakaSpringbloom4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["dress"]}})
    
    @classmethod
    def ayakaSpringbloom5_6(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t2": {"HeadShadeLightMap": TexEditor(filters = [ColourReplaceFilter(Colour(0, 128, 0, 1), coloursToReplace = {ColourRange(Colour(0, 125, 0, 255), Colour(50, 160, 50, 255))}),
                                                                                           ColourReplaceFilter(Colours.LightMapGreen.value, 
                                                                                                               coloursToReplace = {ColourRange(Colour(0, 125, 0, 100), Colour(50, 160, 50, 254)),
                                                                                                                                   ColourRange(Colour(0, 0, 0, 100), Colour(0, 0, 0, 200))})])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Blend.value],
                                IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Position.value],
                                IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["head"],
                                    "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["body"],
                                    "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["dress"]}})
    
    @classmethod
    def ayakaSpingbloomEditBodyDiffuse5_7(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 1)
    
    @classmethod
    def ayakaSpringbloom5_7(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        headShadeLightMapTexEditor = TexEditor(filters = [ColourReplaceFilter(Colour(0, 128, 0, 1), coloursToReplace = {ColourRange(Colour(0, 125, 0, 255), Colour(50, 160, 50, 255))}),
                                                                                           ColourReplaceFilter(Colours.LightMapGreen.value, 
                                                                                                               coloursToReplace = {ColourRange(Colour(0, 125, 0, 100), Colour(50, 160, 50, 254)),
                                                                                                                                   ColourRange(Colour(0, 0, 0, 100), Colour(0, 0, 0, 200))})])
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t1": {"HeadAltShadeLightMap": headShadeLightMapTexEditor},
                                       "ps-t2": {"HeadShadeLightMap": headShadeLightMapTexEditor}},
                              "body": {"ps-t1": {"BodyTransparentDiffuse": TexEditor(filters = [cls.ayakaSpingbloomEditBodyDiffuse5_7]),
                                                 "BodyAltOpaqueGreenLightMap": TexEditor(filters = [TransparencyAdjustFilter(255, coloursToFilter = {ColourRanges.LightMapGreen.value})])},
                                       "ps-t0": {"BodyAltTransparentDiffuse": TexEditor(filters = [cls.ayakaSpingbloomEditBodyDiffuse5_7])},
                                       "ps-t2": {"BodyOpaqueGreenLightMap": TexEditor(filters = [TransparencyAdjustFilter(255, coloursToFilter = {ColourRanges.LightMapGreen.value})])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Blend.value],
                                IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Position.value],
                                IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["head"],
                                    "body": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["body"],
                                    "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom)]["dress"]}})

    @classmethod
    def arlecchino5_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {
                    "head": {"ps-t0": {"YellowHeadNormal": TexEditor(filters = [ColourReplaceFilter(Colours.NormalMapYellow.value, coloursToReplace = {ColourRanges.NormalMapPurple1.value})])}},
                    "body": {"ps-t0": {"YellowBodyNormal": TexEditor(filters = [ColourReplaceFilter(Colours.NormalMapYellow.value)])}},
                }})
    
    @classmethod
    def barbara4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Barbara)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Barbara)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Barbara)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Barbara)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Barbara)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Barbara)]["dress"]}})
    
    @classmethod
    def barbaraSummertime4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.BarbaraSummertime)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.BarbaraSummertime)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.BarbaraSummertime)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.BarbaraSummertime)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.BarbaraSummertime)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.BarbaraSummertime)]["dress"]}})
    
    @classmethod
    def cherryHutao5_3(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress", "extra"}],
                {"texEdits": {"body": {"ps-t0": {"TransparentBodyDiffuse": TexEditor(filters = [InvertAlphaFilter()])},
                                       "ps-t1": {"OpaqueBodyLightMap": TexEditor(filters = [TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1}),
                                                                                           ColourReplaceFilter(Colours.LightMapGreen.value, 
                                                                                                               coloursToReplace = {ColourRange(Colour(0, 120, 110, 65), Colour(255, 140, 255, 75)),
                                                                                                                                   ColourRange(Colour(0, 120, 0, 65), Colour(255, 140, 200, 75)),
                                                                                                                                   ColourRange(Colour(0, 0, 200, 65), Colour(30, 30, 255, 75))})])}},
                              "dress": {"ps-t1": {"TransparentyDressDiffuse": TexEditor(filters = [InvertAlphaFilter()])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.CherryHuTao)][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.CherryHuTao)][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.CherryHuTao)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.CherryHuTao)]["head"],
                                     "body": FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.CherryHuTao)]["body"],
                                     "dress": FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.CherryHuTao)]["dress"],
                                     "extra": FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.CherryHuTao)]["extra"]}})
    
    @classmethod
    def diluc4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Diluc)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Diluc)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Diluc)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Diluc)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Diluc)]["body"]}})
    
    @classmethod
    def dilucFlamme4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}],
                {"texEdits": {"body": {"ps-t0": {"TransparentBodyDiffuse": TexEditor(filters = [InvertAlphaFilter(),
                                                                                                ColourReplaceFilter(Colour(0, 0, 0, 177), 
                                                                                                                    coloursToReplace = {ColourRange(Colour(0, 0, 0, 125), Colour(0, 0, 0, 130))})])}},
                              "dress": {"ps-t0": {"TransparentDressDiffuse": TexEditor(filters = [InvertAlphaFilter()])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.DilucFlamme)][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.DilucFlamme)][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.DilucFlamme)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.DilucFlamme)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.DilucFlamme)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.DilucFlamme)]["dress"]}})
    
    @classmethod
    def fischl4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Fischl)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Fischl)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Fischl)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Fischl)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Fischl)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Fischl)]["dress"]}})
    
    @classmethod
    def fischlHighness4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.FischlHighness)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.FischlHighness)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.FischlHighness)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.FischlHighness)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.FischlHighness)]["body"]}})
    
    @classmethod
    def _ganyuEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 0)
    
    @classmethod
    def ganyu4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._ganyuEditHeadDiffuse,
                                                                                    TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ganyu)][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ganyu)][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ganyu)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ganyu)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ganyu)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ganyu)]["dress"]}})
    
    @classmethod
    def ganyuTwilight4_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)]["head"],
                                     "body": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)]["body"],
                                     "dress": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)]["dress"]}})
    
    @classmethod
    def ganyuTwilight5_7(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)]["head"],
                                     "body": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)]["body"],
                                     "dress": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.GanyuTwilight)]["dress"]}})
    
    @classmethod
    def _hutaoEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 1)
    
    @classmethod
    def hutao4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}],
                {"texEdits": {"head": {"ps-t0": {"TransparentHeadDiffuse": TexEditor(filters = [cls._hutaoEditHeadDiffuse])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.HuTao)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.HuTao)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.HuTao)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.HuTao)]["head"],
                                      "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.HuTao)]["body"]}})
    
    @classmethod
    def jean4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)]["body"]}})
    
    @classmethod
    def jeanCN4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)]["body"]}})
    
    @classmethod
    def jeanSea4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanSea)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanSea)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanSea)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanSea)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanSea)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanSea)]["dress"]}})
    
    @classmethod
    def _jeanEditBodyLightMap5_5(cls, texFile: TextureFile):
        alphaImg = texFile.img.getchannel('A')
        alphaImg = alphaImg.point(lambda alphaPixel: Colour.boundColourChannel(alphaPixel + 77) if (alphaPixel <= 77) else alphaPixel)
        texFile.img.putalpha(alphaImg)
    
    @classmethod
    def jean5_5(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"texEdits": {"body": {"ps-t1": {"ShadeLightMap": TexEditor(filters = [cls._jeanEditBodyLightMap5_5], readPillowImg = True)}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)]["head"],
                                      "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Jean)]["body"]}})
    
    @classmethod
    def jeanCN5_5(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser,
                [{"head", "body"}], 
                {"texEdits": {"body": {"ps-t1": {"ShadeLightMap": TexEditor(filters = [cls._jeanEditBodyLightMap5_5], readPillowImg = True)}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)]["head"],
                                      "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.JeanCN)]["body"]}})
    
    @classmethod
    def kaeya4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser,
                [{"head", "body", "dress"}],
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kaeya)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kaeya)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kaeya)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kaeya)]["head"],
                                      "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kaeya)]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kaeya)]["dress"]}})
    
    @classmethod
    def kaeyaSailwind4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser,
                [{"head", "body", "dress"}],
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KaeyaSailwind)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KaeyaSailwind)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KaeyaSailwind)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KaeyaSailwind)]["head"],
                                      "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KaeyaSailwind)]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KaeyaSailwind)]["dress"]}})
    
    @classmethod
    def _keqingEditDressDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 255)

    @classmethod
    def _keqingEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 255)
    
    @classmethod
    def keqing4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"dress": {"ps-t0": {"OpaqueDressDiffuse": TexEditor(filters = [cls._keqingEditDressDiffuse])}},
                              "head": {"ps-t0": {"OpaqueHeadDiffuse": TexEditor(filters = [cls._keqingEditHeadDiffuse])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Keqing)][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Keqing)][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Keqing)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Keqing)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Keqing)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Keqing)]["dress"]}})
    
    @classmethod
    def keqingOpulent4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"texEdits": {"head": {"ps-t1": {"NonReflectiveLightMap": TexEditor(filters = [TransparencyAdjustFilter(255, coloursToFilter = {ColourRange(Colour(20, 0, 20, 0), Colour(225, 0, 225, 254)),
                                                                                                                                                ColourRange(Colour(120, 120, 50, 0), Colour(140, 140, 70, 254))})])}}}})
    
    @classmethod
    def kirara4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"dress": {"ps-t2": {"WhitenLightMap": TexEditor(filters = [ColourReplaceFilter(Colours.White.value, coloursToReplace = {ColourRanges.LightMapGreen.value}, replaceAlpha = False)])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)]["head"],
                                      "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)]["dress"]}})
    
    @classmethod
    def kirara5_7(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Kirara)]["head"],
                                      "body": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.Kirara)]["body"],
                                      "dress": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.Kirara)]["dress"]}})

    @classmethod
    def kiraraBoots4_8(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)]["head"],
                                      "body": FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)]["body"],
                                      "dress": FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)]["dress"]}})
    
    @classmethod
    def kiraraBoots5_7(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.KiraraBoots)]["head"],
                                      "body": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.KiraraBoots)]["body"],
                                      "dress": FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.KiraraBoots)]["dress"]}})
    
    @classmethod
    def klee4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"texEdits": {"body": {"ps-t1": {"GreenLightMap": TexEditor(filters = [ColourReplaceFilter(Colour(0, 128, 0, 177), 
                                                                                                            coloursToReplace = {ColourRange(Colour(0, 0, 0, 250), Colour(0, 0, 0, 255)),
                                                                                                                                ColourRange(Colour(0, 0, 0, 125), Colour(0 ,0 ,0, 130))}, replaceAlpha = True)])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Klee)][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Klee)][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Klee)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Klee)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Klee)]["body"]}})

    @classmethod
    def kleeBlossomingStarlight4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"dress": {"ps-t0": {"TransparentDiffuse": TexEditor(filters = [InvertAlphaFilter()])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight)]["head"],
                                      "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight)]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight)]["dress"]}})
    
    @classmethod
    def lisa4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Lisa)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Lisa)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Lisa)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Lisa)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Lisa)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Lisa)]["dress"]}})
    
    @classmethod
    def lisaStudent4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.LisaStudent)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.LisaStudent)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.LisaStudent)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.LisaStudent)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.LisaStudent)]["body"]}})
    
    @classmethod
    def lisaStudent5_7(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.LisaStudent)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.LisaStudent)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.LisaStudent)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.LisaStudent)]["head"],
                                     "body": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.LisaStudent)]["body"]}})
    
    @classmethod
    def mona4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Mona)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Mona)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Mona)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Mona)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Mona)]["body"]}})
    
    @classmethod
    def monaCN4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.MonaCN)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.MonaCN)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.MonaCN)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.MonaCN)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.MonaCN)]["body"]}})
    
    @classmethod
    def nilou4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)]["dress"]}})
    
    @classmethod
    def nilou5_7(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Nilou)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.Nilou)]["head"],
                                     "body": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.Nilou)]["body"],
                                     "dress": FileDownloadData[5.7][ModTypeIdTools.getName(ModTypeId.Nilou)]["dress"]}})
    
    @classmethod
    def nilouBreeze4_8(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.NilouBreeze)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.NilouBreeze)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.NilouBreeze)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.NilouBreeze)]["head"],
                                     "body": FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.NilouBreeze)]["body"],
                                     "dress": FileDownloadData[4.8][ModTypeIdTools.getName(ModTypeId.NilouBreeze)]["dress"]}})
    
    @classmethod
    def _ningguangEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 0)
    
    @classmethod
    def ningguang4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._ningguangEditHeadDiffuse,
                                                                                    TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ningguang)][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ningguang)][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ningguang)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ningguang)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ningguang)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Ningguang)]["dress"]}})

    @classmethod
    def ningguangOrchid4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.NingguangOrchid)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.NingguangOrchid)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.NingguangOrchid)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.NingguangOrchid)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.NingguangOrchid)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.NingguangOrchid)]["dress"]}})
    
    @classmethod
    def rosaria4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress", "extra"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Rosaria)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Rosaria)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Rosaria)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Rosaria)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Rosaria)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Rosaria)]["dress"],
                                     "extra": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Rosaria)]["extra"]}})
    
    @classmethod
    def rosariaCN4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress", "extra"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.RosariaCN)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.RosariaCN)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.RosariaCN)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.RosariaCN)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.RosariaCN)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.RosariaCN)]["dress"],
                                     "extra": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.RosariaCN)]["extra"]}})

    @classmethod
    def shenhe4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Shenhe)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Shenhe)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Shenhe)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Shenhe)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Shenhe)]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Shenhe)]["dress"]}})
    
    @classmethod
    def shenheFrostFlower4_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress", "extra"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)]["head"],
                                     "body": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)]["body"],
                                     "dress": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)]["dress"],
                                     "extra": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower)]["extra"]}})
    
    @classmethod
    def _xianlingEditHeadDiffuse_4_0(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 1)
    
    @classmethod
    def xiangling4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._xianlingEditHeadDiffuse_4_0])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xiangling)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xiangling)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xiangling)][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xiangling)]["head"],
                                      "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xiangling)]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xiangling)]["dress"]}})
    
    @classmethod
    def xianglingCheer5_3(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
            [{"head", "body"}], 
            {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.XianglingCheer)][IniKeywords.Blend.value],
                              IniKeywords.Position.value: FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.XianglingCheer)][IniKeywords.Position.value],
                              IniKeywords.Texcoord.value: FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.XianglingCheer)][IniKeywords.Texcoord.value]},
            "objFileDownloads": {"head": FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.XianglingCheer)]["head"],
                                 "body": FileDownloadData[5.3][ModTypeIdTools.getName(ModTypeId.XianglingCheer)]["body"]}})
    
    @classmethod
    def xingqiu4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xingqiu)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xingqiu)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xingqiu)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xingqiu)]["head"],
                                     "body": FileDownloadData[4.0][ModTypeIdTools.getName(ModTypeId.Xingqiu)]["body"]}})
    
    @classmethod
    def xingqiuBamboo4_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.XingqiuBamboo)][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.XingqiuBamboo)][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.XingqiuBamboo)][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.XingqiuBamboo)]["head"],
                                     "body": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.XingqiuBamboo)]["body"],
                                     "dress": FileDownloadData[4.4][ModTypeIdTools.getName(ModTypeId.XingqiuBamboo)]["dress"]}})


IniParseBuilderData = {
    4.0: {ModTypeIdTools.getName(ModTypeId.Amber): IniParseBuilderFuncs.amber4_0,
          ModTypeIdTools.getName(ModTypeId.AmberCN): IniParseBuilderFuncs.amberCN4_0,
          ModTypeIdTools.getName(ModTypeId.Ayaka): IniParseBuilderFuncs.ayaka4_0,
          ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): IniParseBuilderFuncs.ayakaSpringbloom4_0,
          ModTypeIdTools.getName(ModTypeId.Barbara): IniParseBuilderFuncs.barbara4_0,
          ModTypeIdTools.getName(ModTypeId.BarbaraSummertime): IniParseBuilderFuncs.barbaraSummertime4_0,
          ModTypeIdTools.getName(ModTypeId.Diluc): IniParseBuilderFuncs.diluc4_0,
          ModTypeIdTools.getName(ModTypeId.DilucFlamme): IniParseBuilderFuncs.dilucFlamme4_0,
          ModTypeIdTools.getName(ModTypeId.Fischl): IniParseBuilderFuncs.fischl4_0,
          ModTypeIdTools.getName(ModTypeId.FischlHighness): IniParseBuilderFuncs.fischlHighness4_0,
          ModTypeIdTools.getName(ModTypeId.Ganyu): IniParseBuilderFuncs.ganyu4_0,
          ModTypeIdTools.getName(ModTypeId.HuTao): IniParseBuilderFuncs.hutao4_0,
          ModTypeIdTools.getName(ModTypeId.Jean): IniParseBuilderFuncs.jean4_0,
          ModTypeIdTools.getName(ModTypeId.JeanCN): IniParseBuilderFuncs.jeanCN4_0,
          ModTypeIdTools.getName(ModTypeId.JeanSea): IniParseBuilderFuncs.jeanSea4_0,
          ModTypeIdTools.getName(ModTypeId.Kaeya): IniParseBuilderFuncs.kaeya4_0,
          ModTypeIdTools.getName(ModTypeId.KaeyaSailwind): IniParseBuilderFuncs.kaeyaSailwind4_0,
          ModTypeIdTools.getName(ModTypeId.Keqing): IniParseBuilderFuncs.keqing4_0,
          ModTypeIdTools.getName(ModTypeId.KeqingOpulent): IniParseBuilderFuncs.keqingOpulent4_0,
          ModTypeIdTools.getName(ModTypeId.Kirara): IniParseBuilderFuncs.kirara4_0,
          ModTypeIdTools.getName(ModTypeId.Klee): IniParseBuilderFuncs.klee4_0,
          ModTypeIdTools.getName(ModTypeId.KleeBlossomingStarlight):  IniParseBuilderFuncs.kleeBlossomingStarlight4_0,
          ModTypeIdTools.getName(ModTypeId.Lisa): IniParseBuilderFuncs.lisa4_0,
          ModTypeIdTools.getName(ModTypeId.LisaStudent): IniParseBuilderFuncs.lisaStudent4_0,
          ModTypeIdTools.getName(ModTypeId.Mona): IniParseBuilderFuncs.mona4_0,
          ModTypeIdTools.getName(ModTypeId.MonaCN): IniParseBuilderFuncs.monaCN4_0,
          ModTypeIdTools.getName(ModTypeId.Nilou): IniParseBuilderFuncs.nilou4_0,
          ModTypeIdTools.getName(ModTypeId.Ningguang): IniParseBuilderFuncs.ningguang4_0,
          ModTypeIdTools.getName(ModTypeId.NingguangOrchid): IniParseBuilderFuncs.ningguangOrchid4_0,
          ModTypeIdTools.getName(ModTypeId.Raiden): IniParseBuilderFuncs.giDefault,
          ModTypeIdTools.getName(ModTypeId.Rosaria): IniParseBuilderFuncs.rosaria4_0,
          ModTypeIdTools.getName(ModTypeId.RosariaCN): IniParseBuilderFuncs.rosariaCN4_0,
          ModTypeIdTools.getName(ModTypeId.Shenhe): IniParseBuilderFuncs.shenhe4_0,
          ModTypeIdTools.getName(ModTypeId.Xiangling): IniParseBuilderFuncs.xiangling4_0,
          ModTypeIdTools.getName(ModTypeId.Xingqiu): IniParseBuilderFuncs.xingqiu4_0},

    4.4: {ModTypeIdTools.getName(ModTypeId.GanyuTwilight): IniParseBuilderFuncs.ganyuTwilight4_4,
          ModTypeIdTools.getName(ModTypeId.ShenheFrostFlower): IniParseBuilderFuncs.shenheFrostFlower4_4,
          ModTypeIdTools.getName(ModTypeId.XingqiuBamboo): IniParseBuilderFuncs.xingqiuBamboo4_4},

    4.6: {ModTypeIdTools.getName(ModTypeId.Arlecchino): IniParseBuilderFuncs.giDefault},

    4.8: {ModTypeIdTools.getName(ModTypeId.KiraraBoots): IniParseBuilderFuncs.kiraraBoots4_8,
          ModTypeIdTools.getName(ModTypeId.NilouBreeze): IniParseBuilderFuncs.nilouBreeze4_8},

    5.3: {ModTypeIdTools.getName(ModTypeId.CherryHuTao): IniParseBuilderFuncs.cherryHutao5_3,
          ModTypeIdTools.getName(ModTypeId.XianglingCheer): IniParseBuilderFuncs.xianglingCheer5_3},

    5.4: {ModTypeIdTools.getName(ModTypeId.Arlecchino): IniParseBuilderFuncs.arlecchino5_4},

    5.5: {ModTypeIdTools.getName(ModTypeId.Jean): IniParseBuilderFuncs.jean5_5,
          ModTypeIdTools.getName(ModTypeId.JeanCN): IniParseBuilderFuncs.jeanCN5_5},

    5.6: {ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): IniParseBuilderFuncs.ayakaSpringbloom5_6},

    5.7: {ModTypeIdTools.getName(ModTypeId.AyakaSpringbloom): IniParseBuilderFuncs.ayakaSpringbloom5_7,
          ModTypeIdTools.getName(ModTypeId.GanyuTwilight): IniParseBuilderFuncs.ganyuTwilight5_7,
          ModTypeIdTools.getName(ModTypeId.Kirara): IniParseBuilderFuncs.kirara5_7,
          ModTypeIdTools.getName(ModTypeId.KiraraBoots): IniParseBuilderFuncs.kiraraBoots5_7,
          ModTypeIdTools.getName(ModTypeId.LisaStudent): IniParseBuilderFuncs.lisaStudent5_7,
          ModTypeIdTools.getName(ModTypeId.Nilou): IniParseBuilderFuncs.nilou5_7}
}
##### EndScript