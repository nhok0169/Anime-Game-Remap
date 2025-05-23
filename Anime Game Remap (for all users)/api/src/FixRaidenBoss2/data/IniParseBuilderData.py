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
from ..constants.ModTypeNames import ModTypeNames
from ..constants.TexConsts import TexMetadataNames
from ..constants.ColourConsts import ColourConsts
from ..constants.IniConsts import IniKeywords
from ..model.strategies.iniParsers.BaseIniParser import BaseIniParser
from ..model.strategies.iniParsers.GIMIParser import GIMIParser
from ..model.strategies.iniParsers.GIMIObjParser import GIMIObjParser
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
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Amber.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Amber.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Amber.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Amber.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Amber.value]["body"]},})
    
    @classmethod
    def amberCN4_0(cls):
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.AmberCN.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.AmberCN.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.AmberCN.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.AmberCN.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.AmberCN.value]["body"]}})

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
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Ayaka.value][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Ayaka.value][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Ayaka.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Ayaka.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Ayaka.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Ayaka.value]["dress"]}})

    @classmethod
    def ayakaSpringbloom4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head", "body", "dress"}], {})
    
    @classmethod
    def arlecchino5_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"texEdits": {
                    "head": {"ps-t0": {"YellowHeadDiffuse": TexEditor(filters = [ColourReplaceFilter(Colours.NormalMapYellow.value, coloursToReplace = {ColourRanges.NormalMapPurple1.value})])}},
                    "body": {"ps-t0": {"YellowBodyDiffuse": TexEditor(filters = [ColourReplaceFilter(Colours.NormalMapYellow.value)])}},
                }})
    
    @classmethod
    def barbara4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Barbara.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Barbara.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Barbara.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Barbara.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Barbara.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Barbara.value]["dress"]}})
    
    @classmethod
    def barbaraSummertime4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.BarbaraSummertime.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.BarbaraSummertime.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.BarbaraSummertime.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.BarbaraSummertime.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.BarbaraSummertime.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.BarbaraSummertime.value]["dress"]}})
    
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
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[5.3][ModTypeNames.CherryHuTao.value][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[5.3][ModTypeNames.CherryHuTao.value][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[5.3][ModTypeNames.CherryHuTao.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[5.3][ModTypeNames.CherryHuTao.value]["head"],
                                     "body": FileDownloadData[5.3][ModTypeNames.CherryHuTao.value]["body"],
                                     "dress": FileDownloadData[5.3][ModTypeNames.CherryHuTao.value]["dress"],
                                     "extra": FileDownloadData[5.3][ModTypeNames.CherryHuTao.value]["extra"]}})
    
    @classmethod
    def diluc4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Diluc.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Diluc.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Diluc.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Diluc.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Diluc.value]["body"]}})
    
    @classmethod
    def dilucFlamme4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}],
                {"texEdits": {"body": {"ps-t0": {"TransparentBodyDiffuse": TexEditor(filters = [InvertAlphaFilter(),
                                                                                                ColourReplaceFilter(Colour(0, 0, 0, 177), 
                                                                                                                    coloursToReplace = {ColourRange(Colour(0, 0, 0, 125), Colour(0, 0, 0, 130))})])}},
                              "dress": {"ps-t0": {"TransparentDressDiffuse": TexEditor(filters = [InvertAlphaFilter()])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.DilucFlamme.value][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.DilucFlamme.value][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.DilucFlamme.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.DilucFlamme.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.DilucFlamme.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.DilucFlamme.value]["dress"]}})
    
    @classmethod
    def fischl4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Fischl.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Fischl.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Fischl.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Fischl.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Fischl.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Fischl.value]["dress"]}})
    
    @classmethod
    def fischlHighness4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.FischlHighness.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.FischlHighness.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.FischlHighness.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.FischlHighness.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.FischlHighness.value]["body"]}})
    
    @classmethod
    def _ganyuEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 0)
    
    @classmethod
    def ganyu4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._ganyuEditHeadDiffuse,
                                                                                    TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Ganyu.value][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Ganyu.value][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Ganyu.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Ganyu.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Ganyu.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Ganyu.value]["dress"]}})
    
    @classmethod
    def ganyuTwilight4_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.4][ModTypeNames.GanyuTwilight.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.4][ModTypeNames.GanyuTwilight.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.4][ModTypeNames.GanyuTwilight.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.4][ModTypeNames.GanyuTwilight.value]["head"],
                                     "body": FileDownloadData[4.4][ModTypeNames.GanyuTwilight.value]["body"],
                                     "dress": FileDownloadData[4.4][ModTypeNames.GanyuTwilight.value]["dress"]}})
    
    @classmethod
    def _hutaoEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 1)
    
    @classmethod
    def hutao4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}],
                {"texEdits": {"head": {"ps-t0": {"TransparentHeadDiffuse": TexEditor(filters = [cls._hutaoEditHeadDiffuse])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.HuTao.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.HuTao.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.HuTao.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.HuTao.value]["head"],
                                      "body": FileDownloadData[4.0][ModTypeNames.HuTao.value]["body"]}})
    
    @classmethod
    def jean4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Jean.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Jean.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Jean.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Jean.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Jean.value]["body"]}})
    
    @classmethod
    def jeanCN4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.JeanCN.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.JeanCN.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.JeanCN.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.JeanCN.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.JeanCN.value]["body"]}})
    
    @classmethod
    def jeanSea4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.JeanSea.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.JeanSea.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.JeanSea.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.JeanSea.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.JeanSea.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.JeanSea.value]["dress"]}})
    
    @classmethod
    def _jeanEditBodyLightMap5_5(cls, texFile: TextureFile):
        alphaImg = texFile.img.getchannel('A')
        alphaImg = alphaImg.point(lambda alphaPixel: Colour.boundColourChannel(alphaPixel + 77) if (alphaPixel <= 77) else alphaPixel)
        texFile.img.putalpha(alphaImg)
    
    @classmethod
    def jean5_5(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"texEdits": {"body": {"ps-t1": {"ShadeLightMap": TexEditor(filters = [cls._jeanEditBodyLightMap5_5])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Jean.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Jean.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Jean.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Jean.value]["head"],
                                      "body": FileDownloadData[4.0][ModTypeNames.Jean.value]["body"]}})
    
    @classmethod
    def jeanCN5_5(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser,
                [{"head", "body"}], 
                {"texEdits": {"body": {"ps-t1": {"ShadeLightMap": TexEditor(filters = [cls._jeanEditBodyLightMap5_5])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.JeanCN.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.JeanCN.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.JeanCN.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.JeanCN.value]["head"],
                                      "body": FileDownloadData[4.0][ModTypeNames.JeanCN.value]["body"]}})
    
    @classmethod
    def kaeya4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser,
                [{"head", "body", "dress"}],
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Kaeya.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Kaeya.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Kaeya.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Kaeya.value]["head"],
                                      "body": FileDownloadData[4.0][ModTypeNames.Kaeya.value]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeNames.Kaeya.value]["dress"]}})
    
    @classmethod
    def kaeyaSailwind4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser,
                [{"head", "body", "dress"}],
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.KaeyaSailwind.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.KaeyaSailwind.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.KaeyaSailwind.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.KaeyaSailwind.value]["head"],
                                      "body": FileDownloadData[4.0][ModTypeNames.KaeyaSailwind.value]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeNames.KaeyaSailwind.value]["dress"]}})
    
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
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Keqing.value][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Keqing.value][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Keqing.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Keqing.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Keqing.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Keqing.value]["dress"]}})
    
    @classmethod
    def keqingOpulent4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head", "body"}], {})
    
    @classmethod
    def kirara4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"dress": {"ps-t2": {"WhitenLightMap": TexEditor(filters = [ColourReplaceFilter(Colours.White.value, coloursToReplace = {ColourRanges.LightMapGreen.value}, replaceAlpha = False)])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Kirara.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Kirara.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Kirara.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Kirara.value]["head"],
                                      "body": FileDownloadData[4.0][ModTypeNames.Kirara.value]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeNames.Kirara.value]["dress"]}})
    
    @classmethod
    def kiraraBoots4_8(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.8][ModTypeNames.KiraraBoots.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.8][ModTypeNames.KiraraBoots.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.8][ModTypeNames.KiraraBoots.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.8][ModTypeNames.KiraraBoots.value]["head"],
                                      "body": FileDownloadData[4.8][ModTypeNames.KiraraBoots.value]["body"],
                                      "dress": FileDownloadData[4.8][ModTypeNames.KiraraBoots.value]["dress"]}})
    
    @classmethod
    def klee4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"texEdits": {"body": {"ps-t1": {"GreenLightMap": TexEditor(filters = [ColourReplaceFilter(Colour(0, 128, 0, 177), 
                                                                                                            coloursToReplace = {ColourRange(Colour(0, 0, 0, 250), Colour(0, 0, 0, 255)),
                                                                                                                                ColourRange(Colour(0, 0, 0, 125), Colour(0 ,0 ,0, 130))}, replaceAlpha = True)])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Klee.value][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Klee.value][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Klee.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Klee.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Klee.value]["body"]}})

    @classmethod
    def kleeBlossomingStarlight4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.KleeBlossomingStarlight.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.KleeBlossomingStarlight.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.KleeBlossomingStarlight.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.KleeBlossomingStarlight.value]["head"],
                                      "body": FileDownloadData[4.0][ModTypeNames.KleeBlossomingStarlight.value]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeNames.KleeBlossomingStarlight.value]["dress"]}})
    
    @classmethod
    def lisa4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Lisa.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Lisa.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Lisa.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Lisa.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Lisa.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Lisa.value]["dress"]}})
    
    @classmethod
    def lisaStudent4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.LisaStudent.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.LisaStudent.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.LisaStudent.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.LisaStudent.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.LisaStudent.value]["body"]}})
    
    @classmethod
    def mona4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Mona.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Mona.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Mona.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Mona.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Mona.value]["body"]}})
    
    @classmethod
    def monaCN4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.MonaCN.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.MonaCN.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.MonaCN.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.MonaCN.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.MonaCN.value]["body"]}})
    
    @classmethod
    def nilou4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Nilou.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Nilou.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Nilou.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Nilou.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Nilou.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Nilou.value]["dress"]}})
    
    @classmethod
    def nilouBreeze4_8(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.8][ModTypeNames.NilouBreeze.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.8][ModTypeNames.NilouBreeze.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.8][ModTypeNames.NilouBreeze.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.8][ModTypeNames.NilouBreeze.value]["head"],
                                     "body": FileDownloadData[4.8][ModTypeNames.NilouBreeze.value]["body"],
                                     "dress": FileDownloadData[4.8][ModTypeNames.NilouBreeze.value]["dress"]}})
    
    @classmethod
    def _ningguangEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 0)
    
    @classmethod
    def ningguang4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._ningguangEditHeadDiffuse,
                                                                                    TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}},
                "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Ningguang.value][IniKeywords.Blend.value],
                                 IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Ningguang.value][IniKeywords.Position.value],
                                 IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Ningguang.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Ningguang.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Ningguang.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Ningguang.value]["dress"]}})

    @classmethod
    def ningguangOrchid4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.NingguangOrchid.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.NingguangOrchid.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.NingguangOrchid.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.NingguangOrchid.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.NingguangOrchid.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.NingguangOrchid.value]["dress"]}})
    
    @classmethod
    def rosaria4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress", "extra"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Rosaria.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Rosaria.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Rosaria.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Rosaria.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Rosaria.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Rosaria.value]["dress"],
                                     "extra": FileDownloadData[4.0][ModTypeNames.Rosaria.value]["extra"]}})
    
    @classmethod
    def rosariaCN4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress", "extra"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.RosariaCN.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.RosariaCN.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.RosariaCN.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.RosariaCN.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.RosariaCN.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.RosariaCN.value]["dress"],
                                     "extra": FileDownloadData[4.0][ModTypeNames.RosariaCN.value]["extra"]}})

    @classmethod
    def shenhe4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Shenhe.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Shenhe.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Shenhe.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Shenhe.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Shenhe.value]["body"],
                                     "dress": FileDownloadData[4.0][ModTypeNames.Shenhe.value]["dress"]}})
    
    @classmethod
    def shenheFrostFlower4_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress", "extra"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.4][ModTypeNames.ShenheFrostFlower.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.4][ModTypeNames.ShenheFrostFlower.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.4][ModTypeNames.ShenheFrostFlower.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.4][ModTypeNames.ShenheFrostFlower.value]["head"],
                                     "body": FileDownloadData[4.4][ModTypeNames.ShenheFrostFlower.value]["body"],
                                     "dress": FileDownloadData[4.4][ModTypeNames.ShenheFrostFlower.value]["dress"],
                                     "extra": FileDownloadData[4.4][ModTypeNames.ShenheFrostFlower.value]["extra"]}})
    
    @classmethod
    def _xianlingEditHeadDiffuse_4_0(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 1)
    
    @classmethod
    def xiangling4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._xianlingEditHeadDiffuse_4_0])}}},
                 "bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Xiangling.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Xiangling.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Xiangling.value][IniKeywords.Texcoord.value]},
                 "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Xiangling.value]["head"],
                                      "body": FileDownloadData[4.0][ModTypeNames.Xiangling.value]["body"],
                                      "dress": FileDownloadData[4.0][ModTypeNames.Xiangling.value]["dress"]}})
    
    @classmethod
    def xianglingCheer5_3(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
            [{"head", "body"}], 
            {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[5.3][ModTypeNames.XianglingCheer.value][IniKeywords.Blend.value],
                              IniKeywords.Position.value: FileDownloadData[5.3][ModTypeNames.XianglingCheer.value][IniKeywords.Position.value],
                              IniKeywords.Texcoord.value: FileDownloadData[5.3][ModTypeNames.XianglingCheer.value][IniKeywords.Texcoord.value]},
            "objFileDownloads": {"head": FileDownloadData[5.3][ModTypeNames.XianglingCheer.value]["head"],
                                 "body": FileDownloadData[5.3][ModTypeNames.XianglingCheer.value]["body"]}})
    
    @classmethod
    def xingqiu4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.0][ModTypeNames.Xingqiu.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.0][ModTypeNames.Xingqiu.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.0][ModTypeNames.Xingqiu.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.0][ModTypeNames.Xingqiu.value]["head"],
                                     "body": FileDownloadData[4.0][ModTypeNames.Xingqiu.value]["body"]}})
    
    @classmethod
    def xingqiuBamboo4_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"bufDownloads": {IniKeywords.Blend.value: FileDownloadData[4.4][ModTypeNames.XingqiuBamboo.value][IniKeywords.Blend.value],
                                  IniKeywords.Position.value: FileDownloadData[4.4][ModTypeNames.XingqiuBamboo.value][IniKeywords.Position.value],
                                  IniKeywords.Texcoord.value: FileDownloadData[4.4][ModTypeNames.XingqiuBamboo.value][IniKeywords.Texcoord.value]},
                "objFileDownloads": {"head": FileDownloadData[4.4][ModTypeNames.XingqiuBamboo.value]["head"],
                                     "body": FileDownloadData[4.4][ModTypeNames.XingqiuBamboo.value]["body"],
                                     "dress": FileDownloadData[4.4][ModTypeNames.XingqiuBamboo.value]["dress"]}})


IniParseBuilderData = {
    4.0: {ModTypeNames.Amber.value: IniParseBuilderFuncs.amber4_0,
          ModTypeNames.AmberCN.value: IniParseBuilderFuncs.amberCN4_0,
          ModTypeNames.Ayaka.value: IniParseBuilderFuncs.ayaka4_0,
          ModTypeNames.AyakaSpringbloom.value: IniParseBuilderFuncs.ayakaSpringbloom4_0,
          ModTypeNames.Barbara.value: IniParseBuilderFuncs.barbara4_0,
          ModTypeNames.BarbaraSummertime.value: IniParseBuilderFuncs.barbaraSummertime4_0,
          ModTypeNames.Diluc.value: IniParseBuilderFuncs.diluc4_0,
          ModTypeNames.DilucFlamme.value: IniParseBuilderFuncs.dilucFlamme4_0,
          ModTypeNames.Fischl.value: IniParseBuilderFuncs.fischl4_0,
          ModTypeNames.FischlHighness.value: IniParseBuilderFuncs.fischlHighness4_0,
          ModTypeNames.Ganyu.value: IniParseBuilderFuncs.ganyu4_0,
          ModTypeNames.HuTao.value: IniParseBuilderFuncs.hutao4_0,
          ModTypeNames.Jean.value: IniParseBuilderFuncs.jean4_0,
          ModTypeNames.JeanCN.value: IniParseBuilderFuncs.jeanCN4_0,
          ModTypeNames.JeanSea.value: IniParseBuilderFuncs.jeanSea4_0,
          ModTypeNames.Kaeya.value: IniParseBuilderFuncs.kaeya4_0,
          ModTypeNames.KaeyaSailwind.value: IniParseBuilderFuncs.kaeyaSailwind4_0,
          ModTypeNames.Keqing.value: IniParseBuilderFuncs.keqing4_0,
          ModTypeNames.KeqingOpulent.value: IniParseBuilderFuncs.keqingOpulent4_0,
          ModTypeNames.Kirara.value: IniParseBuilderFuncs.kirara4_0,
          ModTypeNames.Klee.value: IniParseBuilderFuncs.klee4_0,
          ModTypeNames.KleeBlossomingStarlight.value:  IniParseBuilderFuncs.kleeBlossomingStarlight4_0,
          ModTypeNames.Lisa.value: IniParseBuilderFuncs.lisa4_0,
          ModTypeNames.LisaStudent.value: IniParseBuilderFuncs.lisaStudent4_0,
          ModTypeNames.Mona.value: IniParseBuilderFuncs.mona4_0,
          ModTypeNames.MonaCN.value: IniParseBuilderFuncs.monaCN4_0,
          ModTypeNames.Nilou.value: IniParseBuilderFuncs.nilou4_0,
          ModTypeNames.Ningguang.value: IniParseBuilderFuncs.ningguang4_0,
          ModTypeNames.NingguangOrchid.value: IniParseBuilderFuncs.ningguangOrchid4_0,
          ModTypeNames.Raiden.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.Rosaria.value: IniParseBuilderFuncs.rosaria4_0,
          ModTypeNames.RosariaCN.value: IniParseBuilderFuncs.rosariaCN4_0,
          ModTypeNames.Shenhe.value: IniParseBuilderFuncs.shenhe4_0,
          ModTypeNames.Xiangling.value: IniParseBuilderFuncs.xiangling4_0,
          ModTypeNames.Xingqiu.value: IniParseBuilderFuncs.xingqiu4_0},

    4.4: {ModTypeNames.GanyuTwilight.value: IniParseBuilderFuncs.ganyuTwilight4_4,
          ModTypeNames.ShenheFrostFlower.value: IniParseBuilderFuncs.shenheFrostFlower4_4,
          ModTypeNames.XingqiuBamboo.value: IniParseBuilderFuncs.xingqiuBamboo4_4},

    4.6: {ModTypeNames.Arlecchino.value: IniParseBuilderFuncs.giDefault},

    4.8: {ModTypeNames.KiraraBoots.value: IniParseBuilderFuncs.kiraraBoots4_8,
          ModTypeNames.NilouBreeze.value: IniParseBuilderFuncs.nilouBreeze4_8},

    5.3: {ModTypeNames.CherryHuTao.value: IniParseBuilderFuncs.cherryHutao5_3,
          ModTypeNames.XianglingCheer.value: IniParseBuilderFuncs.xianglingCheer5_3},

    5.4: {ModTypeNames.Arlecchino.value: IniParseBuilderFuncs.arlecchino5_4},

    5.5: {ModTypeNames.Jean.value: IniParseBuilderFuncs.jean5_5,
          ModTypeNames.JeanCN.value: IniParseBuilderFuncs.jeanCN5_5}
}
##### EndScript