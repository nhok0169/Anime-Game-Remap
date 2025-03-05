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
from ..model.strategies.iniParsers.BaseIniParser import BaseIniParser
from ..model.strategies.iniParsers.IniParseBuilder import IniParseBuilder
from ..model.strategies.iniParsers.GIMIParser import GIMIParser
from ..model.strategies.iniParsers.GIMIObjParser import GIMIObjParser
from ..model.strategies.texEditors.TexEditor import TexEditor
from ..model.strategies.texEditors.texFilters.PixelFilter import PixelFilter
from ..model.strategies.texEditors.texFilters.InvertAlphaFilter import InvertAlphaFilter
from ..model.strategies.texEditors.pixelTransforms.Transparency import Transparency
from ..model.strategies.texEditors.pixelTransforms.ColourReplace import ColourReplace
from ..model.strategies.texEditors.texFilters.TexMetadataFilter import TexMetadataFilter
from ..model.files.TextureFile import TextureFile
from ..model.textures.Colour import Colour
from ..model.textures.ColourRange import ColourRange
##### EndLocalImports


##### Script
# IniParseBuilderFunc: Class to define how the IniParseBuilder arguments for some
#   mod is built for a particular game version
class IniParseBuilderFuncs():
    @classmethod
    def giDefault(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIParser, [], {})

    @classmethod
    def _ayakaEditDressDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 177)

    @classmethod
    def _ayakaEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 1)

    @classmethod
    def ayaka4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}],
                {"texEdits": {"head": {"ps-t0": {"TransparentDiffuse": TexEditor(filters = [TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value}),
                                                                                            cls._ayakaEditHeadDiffuse])}},
                              "body": {"ps-t1": {"BrightLightMap": TexEditor(filters = [PixelFilter(transforms = [Transparency(-78)])])}},
                              "dress": {"ps-t0": {"OpaqueDiffuse": TexEditor(filters = [cls._ayakaEditDressDiffuse,
                                                                                        TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}}})

    @classmethod
    def ayakaSpringbloom4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head", "body", "dress"}], {})
    
    @classmethod
    def arlecchino5_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"texEdits": {
                    "head": {"ps-t0": {"YellowHeadDiffuse": TexEditor(filters = [PixelFilter(transforms = [ColourReplace(Colours.NormalMapYellow.value, coloursToReplace = {ColourRanges.NormalMapPurple1.value})])])}},
                    "body": {"ps-t0": {"YellowBodyDiffuse": TexEditor(filters = [PixelFilter(transforms = [ColourReplace(Colours.NormalMapYellow.value)])])}},
                }})
    
    @classmethod
    def cherryHutao5_3(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress", "extra"}],
                {"texEdits": {"body": {"ps-t0": {"TransparentBodyDiffuse": TexEditor(filters = [InvertAlphaFilter()])},
                                       "ps-t1": {"OpaqueBodyDiffuse": TexEditor(filters = [TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1}),
                                                                                           PixelFilter(transforms = [ColourReplace(Colours.LightMapGreen.value, 
                                                                                                                                   coloursToReplace = {ColourRange(Colour(0, 120, 110, 65), Colour(255, 140, 255, 75)),
                                                                                                                                                       ColourRange(Colour(0, 120, 0, 65), Colour(255, 140, 200, 75)),
                                                                                                                                                       ColourRange(Colour(0, 0, 200, 65), Colour(30, 30, 255, 75))})])])}},
                              "dress": {"ps-t1": {"TransparentyDressDiffuse": TexEditor(filters = [InvertAlphaFilter()])}}}})
    
    @classmethod
    def diluc4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"body", "dress"}], {})
    
    @classmethod
    def dilucFlamme4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"body", "dress"}],
                {"texEdits": {"body": {"ps-t0": {"TransparentBodyDiffuse": TexEditor(filters = [InvertAlphaFilter(),
                                                                                                PixelFilter(transforms = [ColourReplace(Colour(0, 0, 0, 177), 
                                                                                                                          coloursToReplace = {ColourRange(Colour(0, 0, 0, 125), Colour(0, 0, 0, 130))})])])}},
                              "dress": {"ps-t0": {"TransparentDressDiffuse": TexEditor(filters = [InvertAlphaFilter()])}}}})
    
    @classmethod
    def fischl4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"body", "dress"}], {})
    
    @classmethod
    def fischlHighness4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"body", "head"}], {})
    
    @classmethod
    def _ganyuEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 0)
    
    @classmethod
    def ganyu4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._ganyuEditHeadDiffuse,
                                                                                    TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}}})
    
    @classmethod
    def ganyuTwilight4_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head"}], {})
    
    @classmethod
    def _hutaoEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 1)
    
    @classmethod
    def hutao4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}],
                {"texEdits": {"head": {"ps-t0": {"TransparentHeadDiffuse": TexEditor(filters = [cls._hutaoEditHeadDiffuse])}}}})
    
    @classmethod
    def jean4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"body"}], {})
    
    @classmethod
    def jeanCN4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"body"}], {})
    
    @classmethod
    def jeanSea4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"body", "dress"}], {})
    
    @classmethod
    def _keqingEditDressDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 255)

    @classmethod
    def _keqingEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 255)
    
    @classmethod
    def keqing4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "dress"}], 
                {"texEdits": {"dress": {"ps-t0": {"OpaqueDressDiffuse": TexEditor(filters = [cls._keqingEditDressDiffuse])}},
                              "head": {"ps-t0": {"OpaqueHeadDiffuse": TexEditor(filters = [cls._keqingEditHeadDiffuse])}}}})
    
    @classmethod
    def keqingOpulent4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"body"}], {})
    
    @classmethod
    def kirara4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"dress"}], 
                {"texEdits": {"dress": {"ps-t2": {"WhitenLightMap": TexEditor(filters = [PixelFilter(transforms = [ColourReplace(Colours.White.value, coloursToReplace = {ColourRanges.LightMapGreen.value}, replaceAlpha = False)])])}}}})
    
    @classmethod
    def kiraraBoots4_8(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"dress"}], {})
    
    @classmethod
    def klee4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body"}], 
                {"texEdits": {"body": {"ps-t1": {"GreenLightMap": TexEditor(filters = [PixelFilter(transforms = [ColourReplace(Colour(0, 128, 0, 177), 
                                                                                                                               coloursToReplace = {ColourRange(Colour(0, 0, 0, 250), Colour(0, 0, 0, 255)),
                                                                                                                                                   ColourRange(Colour(0, 0, 0, 125), Colour(0 ,0 ,0, 130))}, replaceAlpha = True)])])}}}})

    @classmethod
    def kleeBlossomingStarlight4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head", "body", "dress"}], {})
    
    @classmethod
    def nilou4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head", "body", "dress"}], {})
    
    @classmethod
    def nilouBreeze4_8(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head", "dress", "body"}], {})
    
    @classmethod
    def _ningguangEditHeadDiffuse(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 0)
    
    @classmethod
    def ningguang4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._ningguangEditHeadDiffuse,
                                                                                    TexMetadataFilter(edits = {TexMetadataNames.Gamma.value: 1 / ColourConsts.StandardGamma.value})])}}}})
    
    @classmethod
    def shenhe4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"dress"}], {})
    
    @classmethod
    def shenheFrostFlower4_4(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"dress", "extra"}], {})
    
    @classmethod
    def _xianlingEditHeadDiffuse_4_0(cls, texFile: TextureFile):
        TexEditor.setTransparency(texFile, 1)
    
    @classmethod
    def xiangling4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
                [{"head", "body", "dress"}], 
                {"texEdits": {"head": {"ps-t0": {"DarkDiffuse": TexEditor(filters = [cls._xianlingEditHeadDiffuse_4_0])}}}})
    
    @classmethod
    def xianglingCheer5_3(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, 
            [{"head", "body"}], 
            {})
    
    @classmethod
    def xingqiu4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head"}], {})
    
    @classmethod
    def xingqiuBamboo4_0(cls) -> Tuple[BaseIniParser, List[Any], Dict[str, Any]]:
        return (GIMIObjParser, [{"head", "dress"}], {})


IniParseBuilderData = {
    4.0: {ModTypeNames.Amber.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.AmberCN.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.Ayaka.value: IniParseBuilderFuncs.ayaka4_0,
          ModTypeNames.AyakaSpringbloom.value: IniParseBuilderFuncs.ayakaSpringbloom4_0,
          ModTypeNames.Barbara.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.BarbaraSummertime.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.Diluc.value: IniParseBuilderFuncs.diluc4_0,
          ModTypeNames.DilucFlamme.value: IniParseBuilderFuncs.dilucFlamme4_0,
          ModTypeNames.Fischl.value: IniParseBuilderFuncs.fischl4_0,
          ModTypeNames.FischlHighness.value: IniParseBuilderFuncs.fischlHighness4_0,
          ModTypeNames.Ganyu.value: IniParseBuilderFuncs.ganyu4_0,
          ModTypeNames.HuTao.value: IniParseBuilderFuncs.hutao4_0,
          ModTypeNames.Jean.value: IniParseBuilderFuncs.jean4_0,
          ModTypeNames.JeanCN.value: IniParseBuilderFuncs.jeanCN4_0,
          ModTypeNames.JeanSea.value: IniParseBuilderFuncs.jeanSea4_0,
          ModTypeNames.Keqing.value: IniParseBuilderFuncs.keqing4_0,
          ModTypeNames.KeqingOpulent.value: IniParseBuilderFuncs.keqingOpulent4_0,
          ModTypeNames.Kirara.value: IniParseBuilderFuncs.kirara4_0,
          ModTypeNames.Klee.value: IniParseBuilderFuncs.klee4_0,
          ModTypeNames.KleeBlossomingStarlight.value:  IniParseBuilderFuncs.kleeBlossomingStarlight4_0,
          ModTypeNames.Mona.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.MonaCN.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.Nilou.value: IniParseBuilderFuncs.nilou4_0,
          ModTypeNames.Ningguang.value: IniParseBuilderFuncs.ningguang4_0,
          ModTypeNames.NingguangOrchid.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.Raiden.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.Rosaria.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.RosariaCN.value: IniParseBuilderFuncs.giDefault,
          ModTypeNames.Shenhe.value: IniParseBuilderFuncs.shenhe4_0,
          ModTypeNames.Xiangling.value: IniParseBuilderFuncs.xiangling4_0,
          ModTypeNames.Xingqiu.value: IniParseBuilderFuncs.xingqiu4_0,
          ModTypeNames.XingqiuBamboo.value: IniParseBuilderFuncs.xingqiuBamboo4_0},

    4.4: {ModTypeNames.GanyuTwilight.value: IniParseBuilderFuncs.ganyuTwilight4_4,
          ModTypeNames.ShenheFrostFlower.value: IniParseBuilderFuncs.shenheFrostFlower4_4},

    4.6: {ModTypeNames.Arlecchino.value: IniParseBuilderFuncs.giDefault},

    4.8: {ModTypeNames.KiraraBoots.value: IniParseBuilderFuncs.kiraraBoots4_8,
          ModTypeNames.NilouBreeze.value: IniParseBuilderFuncs.nilouBreeze4_8},

    5.3: {ModTypeNames.CherryHuTao.value: IniParseBuilderFuncs.cherryHutao5_3,
          ModTypeNames.XianglingCheer.value: IniParseBuilderFuncs.xianglingCheer5_3},

    5.4: {ModTypeNames.Arlecchino.value: IniParseBuilderFuncs.arlecchino5_4}
}
##### EndScript