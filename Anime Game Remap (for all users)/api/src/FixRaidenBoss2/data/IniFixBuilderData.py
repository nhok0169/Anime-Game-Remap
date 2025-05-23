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
from ..constants.Colours import Colours
from ..constants.IniConsts import IniKeywords, IniComments
from ..constants.ModTypeNames import ModTypeNames
from ..model.strategies.iniFixers.BaseIniFixer import BaseIniFixer
from ..model.strategies.iniFixers.GIMIFixer import GIMIFixer
from ..model.strategies.iniFixers.GIMIObjRegEditFixer import GIMIObjRegEditFixer
from ..model.strategies.iniFixers.GIMIObjMergeFixer import GIMIObjMergeFixer
from ..model.strategies.iniFixers.GIMIObjSplitFixer import GIMIObjSplitFixer
from ..model.strategies.iniFixers.MultiModFixer import MultiModFixer
from ..model.strategies.iniFixers.IniFixBuilder import IniFixBuilder
from ..model.strategies.iniFixers.regEditFilters.RegRemove import RegRemove
from ..model.strategies.iniFixers.regEditFilters.RegTexEdit import RegTexEdit
from ..model.strategies.iniFixers.regEditFilters.RegRemap import RegRemap
from ..model.strategies.iniFixers.regEditFilters.RegTexAdd import RegTexAdd
from ..model.strategies.iniFixers.regEditFilters.RegNewVals import RegNewVals
from ..model.strategies.texEditors.TexCreator import TexCreator
from .FileDownloadData import FileDownloadData
##### EndLocalImports


##### Script
# IniFixBuilderFunc: Class to define how the IniFixBuilder arguments for some
#   mod are built for a particular game version
class IniFixBuilderFuncs():
    @classmethod
    def _regValIsOrFix(cls, val: str) -> bool:
        return val[1] == IniKeywords.ORFixPath.value

    @classmethod
    def giDefault(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIFixer, [], {})
    
    @classmethod
    def amber4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def amberCN4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def ayaka4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
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
                ]})
    
    @classmethod
    def ayaka5_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                       RegRemove(remove = {"head": {"ps-t2"},
                                           "body": {"ps-t3"}}),
                       RegTexEdit({"BrightLightMap": ["ps-t1"], "OpaqueDiffuse": ["ps-t0"], "TransparentDiffuse": ["ps-t0"]}),
                       RegRemap(remap = {"head": {"ps-t1": ["ps-t2", "temp"], "ps-t0": ["ps-t0", "ps-t1"]},
                                         "body": {"ps-t2": ["ps-t3"], "ps-t1": ["ps-t2", "temp"], "ps-t0": ["ps-t0", "ps-t1"]}}),
                       RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value))},
                                             "body": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value))}}, mustAdd = False),
                       RegNewVals({"head": {"temp": IniKeywords.ORFixPath.value},
                                   "body": {"temp": IniKeywords.ORFixPath.value}}),
                       RegRemap(remap = {"head": {"temp": ["run"]},
                                         "body": {"temp": ["run"]}})
                ]})
    
    @classmethod
    def ayakaSpringbloom4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", "ps-t3", "ResourceRefHeadDiffuse", "ResourceRefHeadLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)},
                                        "body": {"ps-t0", "ResourceRefBodyDiffuse", "ResourceRefBodyLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)},
                                        "dress": {"ps-t3", "ResourceRefDressDiffuse", "ResourceRefDressLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]},
                                        "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"]}})
                ]})
    
    @classmethod
    def arlecchino5_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {"preRegEditFilters": [RegTexEdit({"YellowHeadDiffuse": ["ps-t0"], "YellowBodyDiffuse": ["ps-t0"]})]})
    
    @classmethod
    def barbara4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def barbaraSummertime4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})

    @classmethod
    def cherryHuTao5_3(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "extra"], "body": ["body", "dress"]}], 
                {
                 "preRegEditFilters": [
                         RegRemove(remove = {"head": {"ResourceRefHeadDiffuse", "ResourceRefHeadLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)},
                                             "body": {"ResourceRefBodyDiffuse", "ResourceRefBodyLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)},
                                             "dress": {"ResourceRefDressDiffuse", "ResourceRefDressLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)},
                                             "extra": {"ResourceRefExtraDiffuse", "ResourceRefExtraLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)}}),
                         RegTexEdit(textures = {"TransparentBodyDiffuse": ["ps-t0"],
                                                "TransparentyDressDiffuse": ["ps-t1"],
                                                "OpaqueBodyLightMap": ["ps-t1"]}),
                         RegRemove(remove = {"head": {"ps-t0"},
                                             "dress": {"ps-t0"}}),
                         RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]},
                                           "dress": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]}})
                ]})
    
    @classmethod
    def diluc4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer, 
                [{"body": ["body", "dress"]}], 
                {})
    
    @classmethod
    def dilucFlamme4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"body": ["body", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                    RegTexEdit({"TransparentBodyDiffuse": ["ps-t0"], "TransparentDressDiffuse": ["ps-t0"]})
                ]})
    
    @classmethod
    def fischl4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"body": ["body", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value})
    
    @classmethod
    def fischlHighness4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer, 
                [{"body": ["body", "dress"]}], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"}}),
                    RegRemap(remap = {"head": {"ps-t3": ["ps-t2"]}})
                ]})
    
    @classmethod
    def ganyu4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [],
                {
                 "preRegEditFilters": [
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]}}),
                    RegTexEdit(textures = {"DarkDiffuse": ["ps-t1"]}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}})
                ]})
    
    @classmethod
    def ganyuTwilight4_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0"}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]}})
                ]})
    
    @classmethod
    def hutao4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer, 
                [{"head": ["head", "extra"], "body": ["body", "dress"]}], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"},
                                        "body": {"ps-t2", "ps-t3"}})
                ],
                "postRegEditFilters": [
                    RegRemove(remove = {"extra": {"ps-t0", "ps-t1"}}),
                    RegNewVals(vals = {"extra": {IniKeywords.Ib.value: "null"}, 
                                       "dress": {IniKeywords.Ib.value: "null"}}),
                    RegTexEdit(textures = {"TransparentHeadDiffuse": ["ps-t0"]}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]},
                                        "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]}}),
                    RegNewVals(vals = {"head": {"ps-t0": "null"}}),
                    RegTexAdd(textures = {"dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapBlue.value))}}, mustAdd = False)
                ]})
    
    @classmethod
    def hutao5_6(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "head"], "body": ["body", "body"], "dress": ["body", "body"], "extra": ["head", "head"]}],
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"},
                                        "body": {"ps-t2", "ps-t3"}})
                ],
                "postRegEditFilters": [
                    RegRemove(remove = {"extra": {"ps-t0", "ps-t1"}}),
                    RegNewVals(vals = {"extra": {IniKeywords.Ib.value: "null"}, 
                                       "dress": {IniKeywords.Ib.value: "null"}}),
                    RegTexEdit(textures = {"TransparentHeadDiffuse": ["ps-t0"]}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]},
                                        "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]}}),
                    RegNewVals(vals = {"head": {"ps-t0": "null"}}),
                    RegTexAdd(textures = {"dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapBlue.value))}}, mustAdd = False)
                ],
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value})
    
    @classmethod
    def jean4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (MultiModFixer, 
                [{ModTypeNames.JeanCN.value: IniFixBuilder(GIMIObjRegEditFixer), 
                  ModTypeNames.JeanSea.value: IniFixBuilder(GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}])}],
                {})
    
    @classmethod
    def jeanCN4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (MultiModFixer, 
                [{ModTypeNames.Jean.value: IniFixBuilder(GIMIObjRegEditFixer), 
                  ModTypeNames.JeanSea.value: IniFixBuilder(GIMIObjSplitFixer, args = [{"body": ["body", "dress"]}])}],
                {})
    
    @classmethod
    def jean5_5(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (MultiModFixer, 
                [{ModTypeNames.JeanCN.value: IniFixBuilder(GIMIObjRegEditFixer, kwargs = {}), 
                  ModTypeNames.JeanSea.value: IniFixBuilder(GIMIObjSplitFixer, 
                                                            args = [{"body": ["body", "dress"]}],
                                                            kwargs = {
                                                                
                                                                "postRegEditFilters": [
                                                                    RegNewVals(vals = {"dress": {"ib": "null"}}),
                                                                    RegTexEdit(textures = {"ShadeLightMap": ["ps-t1"]})
                                                                ]
                                                            })}],
                {})
    
    @classmethod
    def jeanCN5_5(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (MultiModFixer, 
                [{ModTypeNames.Jean.value: IniFixBuilder(GIMIObjRegEditFixer, kwargs = {}), 
                  ModTypeNames.JeanSea.value: IniFixBuilder(GIMIObjSplitFixer, 
                                                            args = [{"body": ["body", "dress"]}],
                                                            kwargs = {
                                                                
                                                                "postRegEditFilters": [
                                                                    RegNewVals(vals = {"dress": {"ib": "null"}}),
                                                                    RegTexEdit(textures = {"ShadeLightMap": ["ps-t1"]})
                                                                ]
                                                            })}],
                {})
    
    @classmethod
    def jeanSea4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"body": ["body", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value})
    
    @classmethod
    def kaeya4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer,
                [],
                {"postRegEditFilters": [
                    RegRemap(remap = {"body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"], "ps-t2": ["ps-t3"]}}),
                    RegTexAdd(textures = {"body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}, mustAdd = False),
                    RegNewVals(vals = {"body": {"temp": IniKeywords.ORFixPath.value}}),
                    RegRemap(remap = {"body": {"temp": ["run"]}})
                ]})
    
    @classmethod
    def kaeyaSailwind4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer,
                [{"head": ["head"], "body": ["body"], "dress": ["dress", "extra"]}],
                {"preRegEditFilters": [
                    RegRemove(remove = {"head": {"ResourceRefHeadDiffuse", "ResourceRefHeadLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)},
                                        "body": {"ps-t0", "ResourceRefBodyDiffuse", "ResourceRefBodyLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)},
                                        "dress": {"ResourceRefDressDiffuse", "ResourceRefDressLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)}}),
                    RegRemap(remap = {"body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"]}}),
                ]})
    
    @classmethod
    def keqing4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["dress", "head"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                    RegTexEdit({"OpaqueDressDiffuse": ["ps-t0"], "OpaqueHeadDiffuse": ["ps-t0"]})
                ]})
    
    @classmethod
    def keqingOpulent4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer, [{"body": ["body", "dress"]}], {})
    
    @classmethod
    def kirara4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [],
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"dress": {"ps-t0"}}),
                    RegRemap(remap = {"dress": {"ps-t1": ["ps-t0", "ps-t1"]}}),
                    RegTexEdit(textures = {"WhitenLightMap": ["ps-t2"]})
                ]})
    
    @classmethod
    def kiraraBoots4_8(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemap(remap = {"dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]}}),
                    RegTexAdd(textures = {"dress": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}, mustAdd = False)
                ]})
    
    @classmethod
    def klee4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer, 
                [{"body": ["body", "dress"]}], 
                {
                 "preRegEditFilters": [
                    RegTexEdit(textures = {"GreenLightMap": ["ps-t1"]}),
                    RegRemap(remap = {"head": {"ps-t2": ["ps-t3"]}})
                ]})
    
    @classmethod
    def kleeBlossomingStarlight4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"body": ["body", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"}}),
                    RegRemap(remap = {"head": {"ps-t3": ["ps-t2"]}})
                ]})
    
    @classmethod
    def lisa4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer,
                [{"head": ["head"], "body": ["body", "dress"]}],
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"},
                                        "body": {"ps-t3"},
                                        "dress": {"ps-t2"}})
                ],
                "postRegEditFilters": [
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]},
                                      "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], "ps-t2": ["ps-t3"]}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)},
                                          "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)}}, mustAdd = False)
                ]})
    
    @classmethod
    def lisa5_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer,
                [{"head": ["head"], "body": ["body", "dress"]}],
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"},
                                        "body": {"ps-t3"},
                                        "dress": {"ps-t2"}})
                ],
                "postRegEditFilters": [
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"]},
                                      "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], "ps-t2": ["ps-t3"]}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value), False)},
                                          "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value), False)}}, mustAdd = False)
                ]})
    
    @classmethod
    def lisaStudent4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer,
                [{"body": ["body", "dress"]}],
                {
                 "preRegEditOldObj": True,
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", "ps-t3"}, "body": {"ps-t0", "ps-t3"}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]},
                                      "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]}})
                ],
                "postRegEditFilters": [
                    RegRemap(remap = {"body": {"ps-t3": ["ps-t2"]}})
                ]})
    
    @classmethod
    def mona4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def monaCN4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def nilou4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
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
                ]})
    
    @classmethod
    def nilouBreeze4_8(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t3"},
                                        "dress": {"ps-t3"},
                                        "body": {"ps-t3"}}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"], "ps-t2": ["ps-t3"]},
                                        "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"], "ps-t2": ["ps-t3"]},
                                        "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"], "ps-t2": ["ps-t3"]}}),
                    RegNewVals(vals = {"head": {"temp": IniKeywords.ORFixPath.value},
                                        "dress": {"temp": IniKeywords.ORFixPath.value},
                                        "body": {"temp": IniKeywords.ORFixPath.value}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)},
                                            "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)},
                                            "dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)}}, mustAdd = False),
                    RegRemap(remap = {"head": {"temp": ["run"]},
                                        "dress": {"temp": ["run"]},
                                        "body": {"temp": ["run"]}})
                ]})
    
    @classmethod
    def nilouBreeze5_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t3"},
                                        "dress": {"ps-t3"},
                                        "body": {"ps-t3"}}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"], "ps-t2": ["ps-t3"]},
                                        "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"], "ps-t2": ["ps-t3"]},
                                        "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", "temp"], "ps-t2": ["ps-t3"]}}),
                    RegNewVals(vals = {"head": {"temp": IniKeywords.ORFixPath.value},
                                        "dress": {"temp": IniKeywords.ORFixPath.value},
                                        "body": {"temp": IniKeywords.ORFixPath.value}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple2.value), False)},
                                            "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple2.value), False)},
                                            "dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple2.value), False)}}, mustAdd = False),
                    RegRemap(remap = {"head": {"temp": ["run"]},
                                        "dress": {"temp": ["run"]},
                                        "body": {"temp": ["run"]}})
                ]})

    
    @classmethod
    def ningguang4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [],
                {
                 "preRegEditFilters": [
                    RegTexEdit({"DarkDiffuse": ["ps-t0"]})
                ]})
    
    @classmethod
    def ningguangOrchid4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def rosaria4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def rosariaCN4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def shenhe4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer, 
                [{"dress": ["dress", "extra"]}], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"dress": ["ps-t2"]}),
                    RegRemap(remap = {"dress": {"ps-t3": ["ps-t2"]}})
                ]})
    
    @classmethod
    def shenheFrostFlower4_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"dress": ["dress", "extra"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value})
    
    @classmethod
    def xiangling4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "body", "dress"], "body": ["body"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "preRegEditFilters": [
                    RegTexEdit({"DarkDiffuse": ["ps-t0"]}),
                    RegRemove(remove = {"head": {"ps-t2"},
                                        "body": {"ps-t2", "ps-t3"},
                                        "dress": {"ps-t2"}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t2"], "ps-t0": ["ps-t0", "ps-t1"]},
                                      "body": {"ps-t1": ["ps-t2"], "ps-t0": ["ps-t0", "ps-t1"]},
                                      "dress": {"ps-t1": ["ps-t2"], "ps-t0": ["ps-t0", "ps-t1"]}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapBlue.value))},
                                          "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapBlue.value))},
                                          "dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapBlue.value))}}, mustAdd = False),
                ],
                "postRegEditFilters": [
                    RegNewVals(vals = {"body": {IniKeywords.Ib.value: "null"}}),
                    RegRemap(remap = {"head": {"ps-t2": ["ps-t2", "temp"]}}),
                    RegNewVals(vals = {"head": {"temp": IniKeywords.ORFixPath.value}}),
                    RegRemap(remap = {"head": {"temp": ["run"]}})
                ]})
    
    @classmethod
    def xianglingCheer5_3(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer,
                [{"head": ["head", "dress"], "body": ["body"]}], 
                {
                 "preRegEditOldObj": True,
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", "ResourceRefHeadDiffuse", "ResourceRefHeadLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)}, 
                                        "body": {"ps-t0", "ResourceRefBodyDiffuse", "ResourceRefBodyLightMap", "$CharacterIB", ("run", cls._regValIsOrFix)}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]},
                                      "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"]}})
                ],
                "postRegEditFilters": [
                    RegNewVals(vals = {"head": {IniKeywords.Ib.value: "null"}})
                ]})
    
    @classmethod
    def xingqiu4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer, 
                [{"head": ["head", "dress"]}], 
                {
                 "postRegEditFilters": [
                    RegRemap(remap = {"head": {"ps-t2": ["ps-t3"]}})
                ]})
    
    @classmethod
    def xingqiuBamboo4_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"}}),
                    RegRemap(remap = {"head": {"ps-t3": ["ps-t2"]}})
                ]})


IniFixBuilderData = {
    4.0: {
        ModTypeNames.Amber.value: IniFixBuilderFuncs.amber4_0,
        ModTypeNames.AmberCN.value: IniFixBuilderFuncs.amberCN4_0,
        ModTypeNames.Ayaka.value: IniFixBuilderFuncs.ayaka4_0,
        ModTypeNames.AyakaSpringbloom.value: IniFixBuilderFuncs.ayakaSpringbloom4_0,
        ModTypeNames.Barbara.value: IniFixBuilderFuncs.barbara4_0,
        ModTypeNames.BarbaraSummertime.value: IniFixBuilderFuncs.barbaraSummertime4_0,
        ModTypeNames.Diluc.value: IniFixBuilderFuncs.diluc4_0,
        ModTypeNames.DilucFlamme.value: IniFixBuilderFuncs.dilucFlamme4_0,
        ModTypeNames.Fischl.value: IniFixBuilderFuncs.fischl4_0,
        ModTypeNames.FischlHighness.value: IniFixBuilderFuncs.fischlHighness4_0,
        ModTypeNames.Ganyu.value: IniFixBuilderFuncs.ganyu4_0,
        ModTypeNames.HuTao.value: IniFixBuilderFuncs.hutao4_0,
        ModTypeNames.Jean.value: IniFixBuilderFuncs.jean4_0,
        ModTypeNames.JeanCN.value: IniFixBuilderFuncs.jeanCN4_0,
        ModTypeNames.JeanSea.value: IniFixBuilderFuncs.jeanSea4_0,
        ModTypeNames.Kaeya.value: IniFixBuilderFuncs.kaeya4_0,
        ModTypeNames.KaeyaSailwind.value: IniFixBuilderFuncs.kaeyaSailwind4_0,
        ModTypeNames.Keqing.value: IniFixBuilderFuncs.keqing4_0,
        ModTypeNames.KeqingOpulent.value: IniFixBuilderFuncs.keqingOpulent4_0,
        ModTypeNames.Kirara.value: IniFixBuilderFuncs.kirara4_0,
        ModTypeNames.Klee.value: IniFixBuilderFuncs.klee4_0,
        ModTypeNames.KleeBlossomingStarlight.value: IniFixBuilderFuncs.kleeBlossomingStarlight4_0,
        ModTypeNames.Lisa.value: IniFixBuilderFuncs.lisa4_0,
        ModTypeNames.LisaStudent.value: IniFixBuilderFuncs.lisaStudent4_0,
        ModTypeNames.Mona.value: IniFixBuilderFuncs.mona4_0,
        ModTypeNames.MonaCN.value: IniFixBuilderFuncs.monaCN4_0,
        ModTypeNames.Nilou.value: IniFixBuilderFuncs.nilou4_0,
        ModTypeNames.Ningguang.value: IniFixBuilderFuncs.ningguang4_0,
        ModTypeNames.NingguangOrchid.value: IniFixBuilderFuncs.ningguangOrchid4_0,
        ModTypeNames.Raiden.value: IniFixBuilderFuncs.giDefault,
        ModTypeNames.Rosaria.value: IniFixBuilderFuncs.rosaria4_0,
        ModTypeNames.RosariaCN.value: IniFixBuilderFuncs.rosariaCN4_0,
        ModTypeNames.Shenhe.value: IniFixBuilderFuncs.shenhe4_0,
        ModTypeNames.Xiangling.value: IniFixBuilderFuncs.xiangling4_0,
        ModTypeNames.Xingqiu.value: IniFixBuilderFuncs.xingqiu4_0
    },

    4.4: {
        ModTypeNames.GanyuTwilight.value: IniFixBuilderFuncs.ganyuTwilight4_4,
        ModTypeNames.ShenheFrostFlower.value: IniFixBuilderFuncs.shenheFrostFlower4_4,
        ModTypeNames.XingqiuBamboo.value: IniFixBuilderFuncs.xingqiuBamboo4_4
    },

    4.6: {ModTypeNames.Arlecchino.value: IniFixBuilderFuncs.giDefault},

    4.8: {
        ModTypeNames.KiraraBoots.value: IniFixBuilderFuncs.kiraraBoots4_8,
        ModTypeNames.NilouBreeze.value: IniFixBuilderFuncs.nilouBreeze4_8
    },

    5.3: {
        ModTypeNames.CherryHuTao.value: IniFixBuilderFuncs.cherryHuTao5_3,
        ModTypeNames.XianglingCheer.value: IniFixBuilderFuncs.xianglingCheer5_3
    },

    5.4: {
        ModTypeNames.Ayaka.value: IniFixBuilderFuncs.ayaka5_4,
        ModTypeNames.Arlecchino.value: IniFixBuilderFuncs.arlecchino5_4,
        ModTypeNames.NilouBreeze.value: IniFixBuilderFuncs.nilouBreeze5_4,
        ModTypeNames.Lisa.value: IniFixBuilderFuncs.lisa5_4,
    },
    
    5.5: {
        ModTypeNames.Jean.value: IniFixBuilderFuncs.jean5_5,
        ModTypeNames.JeanCN.value: IniFixBuilderFuncs.jeanCN5_5
    },
    5.6: {
        ModTypeNames.HuTao.value: IniFixBuilderFuncs.hutao5_6
    }
}
##### EndScript