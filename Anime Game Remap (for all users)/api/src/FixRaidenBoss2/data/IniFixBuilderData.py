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
    def _regIsTexFxWrapper(val: Tuple[int, str]) -> bool:
        return val[1].lower().find(IniKeywords.TexFxFolder.value.lower()) > -1
    
    def _regValIsOrFixWrapper(val: Tuple[int, str]) -> bool:
        return val[1] == IniKeywords.ORFixPath.value
    
    @classmethod
    def _regIsTex(cls, val: Tuple[int, str]) -> bool:
        return cls._regIsTexFxWrapper(val)
    
    @classmethod
    def _regValIsOrFix(cls, val: Tuple[int, str]) -> bool:
        return cls._regValIsOrFixWrapper(val)

    TexFxRemove = {("run", _regIsTexFxWrapper)}
    TexFxTempReg = "tempTexFx"
    TexFxTempRegRemap = {"ps-t69": ["ps-t69", TexFxTempReg], "ps-t70": ["ps-t70", TexFxTempReg]}
    TexFXTempToRun = {TexFxTempReg: ["run"]}
    TexFXToNormalValRename4_0 = {TexFxTempReg: IniKeywords.TexFxShortTransparency1.value}
    TexFXToNormalValRename5_0 = {TexFxTempReg: IniKeywords.TexFxShortTransparency1Natlan.value}
    TexFxNoNormalValRename4_0 = {TexFxTempReg: IniKeywords.TexFxShortTransparency0.value}
    TexFxNoNormalValRename5_0 = {TexFxTempReg: IniKeywords.TexFxShortTransparency0Natlan.value}
    
    ReflectionHeadRemove = {"ResourceRefHeadDiffuse", "ResourceRefHeadLightMap", "$CharacterIB", ("run", _regValIsOrFixWrapper)}
    ReflectionBodyRemove = {"ResourceRefBodyDiffuse", "ResourceRefBodyLightMap", "$CharacterIB", ("run", _regValIsOrFixWrapper)}
    ReflectionDressRemove = {"ResourceRefDressDiffuse", "ResourceRefDressLightMap", "$CharacterIB", ("run", _regValIsOrFixWrapper)}
    ReflectionExtraRemove = {"ResourceRefExtraDiffuse", "ResourceRefExtraLightMap", "$CharacterIB", ("run", _regValIsOrFixWrapper)}

    ORFixTempReg = "tempORFix"
    ORFixValRename = {ORFixTempReg: IniKeywords.ORFixPath.value}
    ORFixTempToRun = {ORFixTempReg: ["run"]}
    
    @classmethod
    def _isNormalMap(cls, val: str) -> bool:
        return val.lower().find("normalmap") != -1
    
    @classmethod
    def _isDiffuse(cls, val: str) -> bool:
        return val.lower().find("diffuse") != -1
    
    @classmethod
    def _isLightMap(cls, val: str) -> bool:
        return val.lower().find("lightmap") != -1
    
    @classmethod
    def _isMetalMap(cls, val: str) -> bool:
        return val.lower().find("metalmap") != -1
    
    @classmethod
    def _isShadow(cls, val: str) -> bool:
        return val.lower().find("shadow") != -1
    
    @classmethod
    def _remapIsDiffuse(cls, key: str, val: str) -> bool:
        return cls._isDiffuse(val)
    
    @classmethod
    def _remapIsLightMap(cls, key: str, val: str) -> bool:
        return cls._isLightMap(val)
    
    @classmethod
    def _remapIsMetalMap(cls, key: str, val: str) -> bool:
        return cls._isMetalMap(val)
    
    @classmethod
    def _remapIsShadow(cls, key: str, val: str) -> bool:
        return cls._isShadow(val)
    
    @classmethod
    def _removeIsNormalMap(cls, val: Tuple[int, str]) -> bool:
        return cls._isNormalMap(val[1])
    
    # =======================================================

    @classmethod
    def giDefault(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIFixer, [], {})
    
    @classmethod
    def amber4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def amber5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "head"], "body": ["body", "body"]}],
                {"copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
    @classmethod
    def amberCN4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def amberCN5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "head"], "body": ["body", "body"]}],
                {"copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
    @classmethod
    def ayaka4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                       RegRemove(remove = {"head": {"ps-t2", *cls.TexFxRemove},
                                           "body": {"ps-t3", *cls.TexFxRemove}}),
                       RegTexEdit({"BrightLightMap": ["ps-t1"], "OpaqueDiffuse": ["ps-t0"], "TransparentDiffuse": ["ps-t0"]}),
                       RegRemap(remap = {"head": {"ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t0": ["ps-t0", "ps-t1"], **cls.TexFxTempRegRemap},
                                         "body": {"ps-t2": ["ps-t3"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t0": ["ps-t0", "ps-t1"], **cls.TexFxTempRegRemap}}),
                       RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))},
                                             "body": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}, mustAdd = False),
                       RegNewVals({"head": {**cls.ORFixValRename, **cls.TexFXToNormalValRename4_0},
                                   "body": {**cls.ORFixValRename, **cls.TexFXToNormalValRename4_0}}),
                       RegRemap(remap = {"head": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                         "body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def ayaka5_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                       RegRemove(remove = {"head": {"ps-t2", *cls.TexFxRemove},
                                           "body": {"ps-t3", *cls.TexFxRemove}}),
                       RegTexEdit({"BrightLightMap": ["ps-t1"], "OpaqueDiffuse": ["ps-t0"], "TransparentDiffuse": ["ps-t0"]}),
                       RegRemap(remap = {"head": {"ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t0": ["ps-t0", "ps-t1"], **cls.TexFxTempRegRemap},
                                         "body": {"ps-t2": ["ps-t3"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t0": ["ps-t0", "ps-t1"], **cls.TexFxTempRegRemap}}),
                       RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value))},
                                             "body": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value))}}, mustAdd = False),
                       RegNewVals({"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0},
                                   "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0}}),
                       RegRemap(remap = {"head": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                         "body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def ayaka5_6(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "head"], "body": ["body", "body"], "dress": ["dress", "dress"]}], 
                {
                 "preRegEditFilters": [
                       RegRemove(remove = {"head": {"ps-t2", *cls.TexFxRemove},
                                           "body": {"ps-t3", *cls.TexFxRemove}}),
                       RegTexEdit({"BrightLightMap": ["ps-t1"], "OpaqueDiffuse": ["ps-t0"], "TransparentDiffuse": ["ps-t0"]}),
                       RegRemap(remap = {"head": {"ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t0": ["ps-t0", "ps-t1"], **cls.TexFxTempRegRemap},
                                         "body": {"ps-t2": ["ps-t3"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t0": ["ps-t0", "ps-t1"], **cls.TexFxTempRegRemap}}),
                       RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value))},
                                             "body": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value))}}, mustAdd = False),
                       RegNewVals({"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0},
                                   "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0}}),
                       RegRemap(remap = {"head": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                         "body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
    @classmethod
    def ayaka5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "head"], "body": ["body", "body"], "dress": ["dress", "dress"]}], 
                {
                 "preRegEditFilters": [
                       RegRemove(remove = {"head": {"ps-t2"},
                                           "body": {"ps-t3"}}),
                       RegTexEdit({"BrightLightMap": ["ps-t1"], "OpaqueDiffuse": ["ps-t0"], "TransparentDiffuse": ["ps-t0"]}),
                       RegRemap(remap = {"head": {"ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t0": ["ps-t0", "ps-t1"]},
                                         "body": {"ps-t2": ["ps-t3"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t0": ["ps-t0", "ps-t1"]}}),
                       RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value))},
                                             "body": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value))}}, mustAdd = False),
                       RegNewVals({"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value},
                                   "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value}}),
                       RegRemap(remap = {"head": {**cls.ORFixTempToRun},
                                         "body": {**cls.ORFixTempToRun}})
                ],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
    @classmethod
    def ayakaSpringbloom4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", "ps-t3", *cls.ReflectionHeadRemove, *cls.TexFxRemove},
                                        "body": {"ps-t0", "ResourceRefBodyDiffuse", *cls.ReflectionBodyRemove, *cls.TexFxRemove},
                                        "dress": {"ps-t3", "ResourceRefDressDiffuse", *cls.ReflectionDressRemove, *cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], **cls.TexFxTempRegRemap},
                                      "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {**cls.TexFxNoNormalValRename4_0},
                                       "body": {**cls.TexFxNoNormalValRename4_0}}),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun},
                                      "body": {**cls.TexFXTempToRun}})
                ]})

    @classmethod
    def ayakaSpringbloom5_6(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["body", "body", "body"], "body": ["body", "head", "head"], "dress": ["dress", "dress", "dress"]}], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", "ps-t3", *cls.ReflectionHeadRemove, *cls.TexFxRemove},
                                        "body": {"ps-t0", *cls.ReflectionBodyRemove, *cls.TexFxRemove},
                                        "dress": {"ps-t3", *cls.ReflectionDressRemove}}),
                    RegTexEdit(textures = {"HeadShadeLightMap": ["ps-t2"]}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], **cls.TexFxTempRegRemap},
                                      "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {**cls.TexFxNoNormalValRename5_0},
                                       "body": {**cls.TexFxNoNormalValRename5_0}}),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun},
                                      "body": {**cls.TexFXTempToRun}})
                ],
                "postRegEditFilters": [RegNewVals({"head": {"ib": "null"}})],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], [], []]})
    
    @classmethod
    def ayakaSpringbloom5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["body", "body", "body"], "body": ["body", "head", "head"], "dress": ["dress", "dress", "dress"]}], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t3", *cls.ReflectionHeadRemove},
                                        "body": {*cls.ReflectionBodyRemove},
                                        "dress": {"ps-t3", *cls.ReflectionDressRemove}}),
                    RegRemap(remap = {"head": {"ps-t2": ["ps-t2", (cls.ORFixTempReg, cls._remapIsShadow)]},
                                      "body": {"ps-t2": ["ps-t2", (cls.ORFixTempReg, cls._remapIsShadow)]}}, keepKeysWithoutRemap = True),
                    RegTexEdit(textures = {"HeadShadeLightMap": [("ps-t2", cls._remapIsShadow)], 
                                           "HeadAltShadeLightMap": [("ps-t1", cls._remapIsShadow)],
                                           "BodyTransparentDiffuse": [("ps-t1", cls._remapIsLightMap)],
                                           "BodyAltTransparentDiffuse": [("ps-t0", cls._remapIsLightMap)],
                                           "BodyOpaqueGreenLightMap": [("ps-t2", cls._remapIsShadow)],  
                                           "BodyAltOpaqueGreenLightMap": [("ps-t1", cls._remapIsShadow)]}),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value},
                                       "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value}}),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun},
                                      "body": {**cls.ORFixTempToRun}})
                ],
                "postRegEditFilters": [RegNewVals({"head": {"ib": "null"}})],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], [], []]})

    
    @classmethod
    def arlecchino5_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {"preRegEditFilters": [RegTexEdit({"YellowHeadNormal": ["ps-t0"], "YellowBodyNormal": ["ps-t0"]})]})
    
    @classmethod
    def arlecchino5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer,
                [{"head": ["head", "head"], "body": ["body", "body"]}],
                {"preRegEditFilters": [RegTexEdit({"YellowHeadNormal": ["ps-t0"], "YellowBodyNormal": ["ps-t0"]})],
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
    @classmethod
    def barbara4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def barbara5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer,
                [{"head": ["head", "head"], "body": ["body", "body"], "dress": ["dress", "dress"]}],
                {"copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
    @classmethod
    def barbaraSummertime4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, [], {})
    
    @classmethod
    def barbaraSummertime5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer,
                [{"head": ["head", "head"], "body": ["body", "body"], "dress": ["dress", "dress"]}],
                {"copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})

    @classmethod
    def cherryHuTao5_3(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "extra"], "body": ["body", "dress"]}], 
                {
                 "preRegEditFilters": [
                         RegRemove(remove = {"head": {*cls.ReflectionHeadRemove, *cls.TexFxRemove},
                                             "body": {*cls.ReflectionBodyRemove},
                                             "dress": {*cls.ReflectionDressRemove, *cls.TexFxRemove},
                                             "extra": {*cls.ReflectionExtraRemove}}),
                         RegTexEdit(textures = {"TransparentBodyDiffuse": ["ps-t0"],
                                                "TransparentyDressDiffuse": ["ps-t1"],
                                                "OpaqueBodyLightMap": ["ps-t1"]}),
                         RegRemove(remove = {"head": {"ps-t0"},
                                             "dress": {"ps-t0"}}),
                         RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], **cls.TexFxTempRegRemap},
                                           "dress": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], **cls.TexFxTempRegRemap}}),
                         RegNewVals(vals = {"head": {**cls.TexFxNoNormalValRename5_0},
                                            "dress": {**cls.TexFxNoNormalValRename5_0}}),
                         RegRemap(remap = {"head": {**cls.TexFXTempToRun},
                                           "dress": {**cls.TexFXTempToRun}})
                ],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value})
    
    @classmethod
    def diluc4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer, 
                [{"body": ["body", "dress"]}], 
                {})
    
    @classmethod
    def diluc5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer,
                [{"head": ["head", "head"], "body": ["body", "body"], "dress": ["body", "body"]}],
                {"copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
    @classmethod
    def dilucFlamme4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"body": ["body", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                    RegTexEdit({"TransparentBodyDiffuse": ["ps-t0"], "TransparentDressDiffuse": ["ps-t0"]})
                ]})
    
    @classmethod
    def dilucFlamme5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "head", "head"], "body": ["body", "body", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                    RegTexEdit({"TransparentBodyDiffuse": ["ps-t0"], "TransparentDressDiffuse": ["ps-t0"]})
                ],
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], [], []]})
    
    @classmethod
    def fischl4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"body": ["body", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value})
    
    @classmethod
    def fischl5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "head", "head"], "body": ["body", "body", "dress"]}], 
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                 "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], [], []]})

    
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
    def fischlHighness5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head":  ["head", "head"], "body": ["body", "body"], "dress": ["body", "body"]}], 
                {
                 "postRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"}}),
                    RegRemap(remap = {"head": {"ps-t3": ["ps-t2"]}}),
                    RegNewVals({"dress": {"ib": "null"}})
                ],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})

    
    @classmethod
    def ganyu4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [],
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {*cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], **cls.TexFxTempRegRemap}}),
                    RegTexEdit(textures = {"DarkDiffuse": ["ps-t1"]}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}),
                    RegNewVals(vals = {"head": {**cls.TexFXToNormalValRename4_0}}),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def ganyu5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer,
                [{"head": ["head", "head"], "body": ["body", "body"], "dress": ["dress", "dress"]}],
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {*cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], **cls.TexFxTempRegRemap}}),
                    RegTexEdit(textures = {"DarkDiffuse": ["ps-t1"]}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormalMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0}}),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
    @classmethod
    def ganyuTwilight4_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", *cls.ReflectionHeadRemove, *cls.TexFxRemove},
                                        "body": {*cls.ReflectionBodyRemove},
                                        "dress": {*cls.ReflectionDressRemove}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {**cls.TexFxNoNormalValRename4_0}}),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def ganyuTwilight5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer, 
                [{"head": ["head", "head"], "body": ["body", "body"], "dress": ["dress", "dress"]}], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", *cls.ReflectionHeadRemove, *cls.TexFxRemove},
                                        "body": {*cls.ReflectionBodyRemove},
                                        "dress": {*cls.ReflectionDressRemove}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {**cls.TexFxNoNormalValRename5_0}}),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun}})
                ],
                "copyPreamble": IniComments.GIMIObjMergerPreamble.value,
                "iniPostModelRegEditFilters": [[RegNewVals(vals = {IniKeywords.Ib.value: {"hash": "null"}})], []]})
    
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
                    RegRemove(remove = {"head": {*cls.TexFxRemove},
                                        "dress": {*cls.TexFxRemove},
                                        "extra": {"ps-t0", "ps-t1"}}),
                    RegNewVals(vals = {"extra": {IniKeywords.Ib.value: "null"}, 
                                       "dress": {IniKeywords.Ib.value: "null"}}),
                    RegTexEdit(textures = {"TransparentHeadDiffuse": ["ps-t0"]}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], **cls.TexFxTempRegRemap},
                                      "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {"ps-t0": "null", **cls.TexFXToNormalValRename4_0},
                                       "dress": {**cls.TexFXToNormalValRename4_0}}),
                    RegTexAdd(textures = {"dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapBlue.value))}}, mustAdd = False),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun},
                                      "dress": {**cls.TexFXTempToRun}})
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
                    RegRemove(remove = {"head": {*cls.TexFxRemove},
                                        "dress": {*cls.TexFxRemove},
                                        "extra": {"ps-t0", "ps-t1"}}),
                    RegNewVals(vals = {"extra": {IniKeywords.Ib.value: "null"}, 
                                       "dress": {IniKeywords.Ib.value: "null"}}),
                    RegTexEdit(textures = {"TransparentHeadDiffuse": ["ps-t0"]}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], **cls.TexFxTempRegRemap},
                                      "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {"ps-t0": "null", **cls.TexFXToNormalValRename5_0},
                                       "dress": {**cls.TexFXToNormalValRename5_0}}),
                    RegTexAdd(textures = {"dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapBlue.value))}}, mustAdd = False),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun},
                                      "dress": {**cls.TexFXTempToRun}})
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
                    RegRemove(remove = {"body": {*cls.TexFxRemove}}),
                    RegRemap(remap = {"body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap}}),
                    RegTexAdd(textures = {"body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}, mustAdd = False),
                    RegNewVals(vals = {"body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename4_0}}),
                    RegRemap(remap = {"body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def kaeya5_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer,
                [],
                {"postRegEditFilters": [
                    RegRemove(remove = {"body": {*cls.TexFxRemove}}),
                    RegRemap(remap = {"body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap}}),
                    RegTexAdd(textures = {"body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}, mustAdd = False),
                    RegNewVals(vals = {"body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0}}),
                    RegRemap(remap = {"body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def kaeyaSailwind4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer,
                [{"head": ["head"], "body": ["body"], "dress": ["dress", "extra"]}],
                {"preRegEditFilters": [
                    RegRemove(remove = {"head": {*cls.ReflectionHeadRemove},
                                        "body": {"ps-t0", *cls.ReflectionBodyRemove, *cls.TexFxRemove},
                                        "dress": {*cls.ReflectionDressRemove}}),
                    RegRemap(remap = {"body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"body": {**cls.TexFxNoNormalValRename4_0}}),
                    RegRemap(remap = {"body": {**cls.TexFXTempToRun}})
                ]})

    @classmethod
    def kaeyaSailwind5_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer,
                [{"head": ["head"], "body": ["body"], "dress": ["dress", "extra"]}],
                {"preRegEditFilters": [
                    RegRemove(remove = {"head": {*cls.ReflectionHeadRemove},
                                        "body": {*cls.ReflectionBodyRemove, *cls.TexFxRemove},
                                        "dress": {*cls.ReflectionDressRemove}}),
                    RegRemap(remap = {"body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], "ps-t3": ["ps-t2"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"body": {**cls.TexFxNoNormalValRename5_0}}),
                    RegRemap(remap = {"body": {**cls.TexFXTempToRun}})
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
        return (GIMIObjSplitFixer, 
                [{"head": ["head"], "body": ["body", "dress"]}], 
                {"preRegEditFilters": [
                    RegTexEdit(textures = {"NonReflectiveLightMap": ["ps-t1"]})
                ]})
    
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
    def kirara5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [],
                {
                    "preRegEditFilters": [
                    RegRemove(remove = {
                        "head": {*cls.ReflectionHeadRemove, *cls.TexFxRemove}, 
                        "body": {*cls.ReflectionBodyRemove, *cls.TexFxRemove}, 
                        "dress": {("ps-t0", cls._removeIsNormalMap), *cls.ReflectionDressRemove, *cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t2": ["ps-t2", cls.ORFixTempReg], **cls.TexFxTempRegRemap},
                                      "body": {"ps-t2": [("ps-t2", cls._remapIsLightMap), (cls.ORFixTempReg, cls._remapIsLightMap)], **cls.TexFxTempRegRemap},
                                      "dress": {"ps-t1": [("ps-t0", cls._remapIsDiffuse)], "ps-t2": [("ps-t1", cls._remapIsLightMap)], **cls.TexFxTempRegRemap}}, keepKeysWithoutRemap = True),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFxNoNormalValRename5_0},
                                       "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFxNoNormalValRename5_0},
                                       "dress": {**cls.TexFxNoNormalValRename5_0}}),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                      "body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                      "dress": {**cls.TexFXTempToRun}})
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
    def kiraraBoots5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {*cls.ReflectionHeadRemove, *cls.TexFxRemove},
                                        "body": {*cls.ReflectionBodyRemove, *cls.TexFxRemove},
                                        "dress": {("run", cls._regValIsOrFix), "ps-t2"}}),
                    RegRemap(remap = {"head": {"ps-t0": [("tempNorm", cls._remapIsDiffuse), ("ps-t1", cls._remapIsDiffuse)], 
                                               "ps-t1": [("ps-t2", cls._remapIsLightMap), (cls.ORFixTempReg, cls._remapIsLightMap)], 
                                               "ps-t2": [("ps-t2", cls._remapIsLightMap), (cls.ORFixTempReg, cls._remapIsLightMap)],
                                               **cls.TexFxTempRegRemap},
                                      "body": {"ps-t2": [("ps-t2", cls._remapIsLightMap), (cls.ORFixTempReg, cls._remapIsLightMap)],
                                               **cls.TexFxTempRegRemap}}, keepKeysWithoutRemap = True),
                    RegTexAdd(textures = {"head": {"tempNorm": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value))}}, mustAdd = False),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFxNoNormalValRename5_0},
                                       "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFxNoNormalValRename5_0}}),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun, "tempNorm": ["ps-t0"], **cls.TexFXTempToRun},
                                      "body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
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
                    RegTexEdit(textures = {"TransparentDiffuse": ["ps-t0"]}),
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
                    RegRemove(remove = {"head": {*cls.TexFxRemove},
                                        "body": {*cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], **cls.TexFxTempRegRemap},
                                      "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)},
                                          "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)}}, mustAdd = False),
                    RegNewVals(vals = {"head": {**cls.TexFXToNormalValRename4_0},
                                       "body": {**cls.TexFXToNormalValRename4_0}}),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun},
                                      "body": {**cls.TexFXTempToRun}})
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
                    RegRemove(remove = {"head": {*cls.TexFxRemove},
                                        "body": {*cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], **cls.TexFxTempRegRemap},
                                      "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2"], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value), False)},
                                          "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple1.value), False)}}, mustAdd = False),
                    RegNewVals(vals = {"head": {**cls.TexFXToNormalValRename5_0},
                                       "body": {**cls.TexFXToNormalValRename5_0}}),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun},
                                      "body": {**cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def lisa5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjMergeFixer,
                [{"head": ["head"], "body": ["body", "dress"]}],
                {
                 "copyPreamble": IniComments.GIMIObjMergerPreamble.value, "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t2"},
                                        "body": {"ps-t3"},
                                        "dress": {"ps-t2"}})
                ]})
    
    @classmethod
    def lisaStudent4_0(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer,
                [{"body": ["body", "dress"]}],
                {
                 "preRegEditOldObj": True,
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", "ps-t3", *cls.TexFxRemove}, 
                                        "body": {"ps-t0", "ps-t3", *cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], **cls.TexFxTempRegRemap},
                                      "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {**cls.TexFxNoNormalValRename4_0},
                                       "body": {**cls.TexFxNoNormalValRename4_0}}),
                    RegRemap(remap = {"head": {**cls.TexFXTempToRun},
                                      "body": {**cls.TexFXTempToRun}})
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
                    RegRemove(remove = {"head": {"ps-t0", *cls.ReflectionHeadRemove, *cls.TexFxRemove}, 
                                        "body": {"ps-t0", *cls.ReflectionBodyRemove, *cls.TexFxRemove}, 
                                        "dress": {"ps-t0", *cls.ReflectionDressRemove, *cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1", cls.ORFixTempReg], "ps-t3": ["ps-t2"], **cls.TexFxTempRegRemap},
                                        "body": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1", cls.ORFixTempReg], "ps-t3": ["ps-t2"], **cls.TexFxTempRegRemap},
                                        "dress": {"ps-t1": ["ps-t0"], "ps-t2": ["ps-t1", cls.ORFixTempReg], "ps-t3": ["ps-t2"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFxNoNormalValRename4_0},
                                       "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFxNoNormalValRename4_0},
                                       "dress": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFxNoNormalValRename4_0}}),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                      "body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                      "dress": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def nilou5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {("ps-t0", cls._removeIsNormalMap), cls.ReflectionHeadRemove}, 
                                        "body": {("ps-t0", cls._removeIsNormalMap), cls.ReflectionDressRemove}, 
                                        "dress": {("ps-t0", cls._removeIsNormalMap), cls.ReflectionDressRemoves}}),
                    RegRemap(remap = {"head": {"ps-t2": [("ps-t2", cls._remapIsLightMap), (cls.ORFixTempReg, cls._remapIsLightMap)]},
                                        "body": {"ps-t2": [("ps-t2", cls._remapIsLightMap), (cls.ORFixTempReg, cls._remapIsLightMap)]},
                                        "dress": {"ps-t2": [("ps-t2", cls._remapIsLightMap), (cls.ORFixTempReg, cls._remapIsLightMap)]}}, keepKeysWithoutRemap = True),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value},
                                       "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value},
                                       "dress": {cls.ORFixTempReg: IniKeywords.ORFixPath.value}}),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun},
                                      "body": {**cls.ORFixTempToRun},
                                      "dress": {**cls.ORFixTempToRun}})
                ]})
    
    @classmethod
    def nilouBreeze4_8(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t3", *cls.TexFxRemove},
                                        "dress": {"ps-t3", *cls.TexFxRemove},
                                        "body": {"ps-t3", *cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap},
                                        "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap},
                                        "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename4_0},
                                        "dress": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename4_0},
                                        "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename4_0}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)},
                                            "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)},
                                            "dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapYellow.value), False)}}, mustAdd = False),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                        "dress": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                        "body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def nilouBreeze5_4(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t3", *cls.TexFxRemove},
                                        "dress": {"ps-t3", *cls.TexFxRemove},
                                        "body": {"ps-t3", *cls.TexFxRemove}}),
                    RegRemap(remap = {"head": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap},
                                        "dress": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap},
                                        "body": {"ps-t0": ["ps-t0", "ps-t1"], "ps-t1": ["ps-t2", cls.ORFixTempReg], "ps-t2": ["ps-t3"], **cls.TexFxTempRegRemap}}),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0},
                                        "dress": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0},
                                        "body": {cls.ORFixTempReg: IniKeywords.ORFixPath.value, **cls.TexFXToNormalValRename5_0}}),
                    RegTexAdd(textures = {"head": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple2.value), False)},
                                            "body": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple2.value), False)},
                                            "dress": {"ps-t0": ("NormMap", TexCreator(1024, 1024, colour = Colours.NormalMapPurple2.value), False)}}, mustAdd = False),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                        "dress": {**cls.ORFixTempToRun, **cls.TexFXTempToRun},
                                        "body": {**cls.ORFixTempToRun, **cls.TexFXTempToRun}})
                ]})
    
    @classmethod
    def nilouBreeze5_7(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjRegEditFixer, 
                [], 
                {
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t3"},
                                        "dress": {"ps-t3"},
                                        "body": {"ps-t3"}})
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
                [{"body": ["body", "extra"]}], 
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
                    RegRemap(remap = {"head": {"ps-t2": ["ps-t2", cls.ORFixTempReg]}}),
                    RegNewVals(vals = {"head": {cls.ORFixTempReg: IniKeywords.ORFixPath.value}}),
                    RegRemap(remap = {"head": {**cls.ORFixTempToRun}})
                ]})
    
    @classmethod
    def xianglingCheer5_3(cls) -> Tuple[BaseIniFixer, List[Any], Dict[str, Any]]:
        return (GIMIObjSplitFixer,
                [{"head": ["head", "dress"], "body": ["body"]}], 
                {
                 "preRegEditOldObj": True,
                 "preRegEditFilters": [
                    RegRemove(remove = {"head": {"ps-t0", *cls.ReflectionHeadRemove}, 
                                        "body": {"ps-t0", *cls.ReflectionBodyRemove}}),
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

    5.0: {
        ModTypeNames.Kaeya.value: IniFixBuilderFuncs.kaeya5_0,
        ModTypeNames.KaeyaSailwind.value: IniFixBuilderFuncs.kaeyaSailwind5_0
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
        ModTypeNames.HuTao.value: IniFixBuilderFuncs.hutao5_6,
        ModTypeNames.Ayaka.value: IniFixBuilderFuncs.ayaka5_6,
        ModTypeNames.AyakaSpringbloom.value: IniFixBuilderFuncs.ayakaSpringbloom5_6
    },

    5.7: {
        ModTypeNames.Amber.value: IniFixBuilderFuncs.amber5_7,
        ModTypeNames.AmberCN.value: IniFixBuilderFuncs.amberCN5_7,
        ModTypeNames.Ayaka.value: IniFixBuilderFuncs.ayaka5_7,
        ModTypeNames.AyakaSpringbloom.value: IniFixBuilderFuncs.ayakaSpringbloom5_7,
        ModTypeNames.Arlecchino.value: IniFixBuilderFuncs.arlecchino5_7,
        ModTypeNames.Barbara.value: IniFixBuilderFuncs.barbara5_7,
        ModTypeNames.BarbaraSummertime.value: IniFixBuilderFuncs.barbaraSummertime5_7,
        ModTypeNames.Diluc.value: IniFixBuilderFuncs.diluc5_7,
        ModTypeNames.DilucFlamme.value: IniFixBuilderFuncs.dilucFlamme5_7,
        ModTypeNames.Fischl.value: IniFixBuilderFuncs.fischl5_7,
        ModTypeNames.FischlHighness.value: IniFixBuilderFuncs.fischlHighness5_7,
        ModTypeNames.Ganyu.value: IniFixBuilderFuncs.ganyu5_7,
        ModTypeNames.GanyuTwilight.value: IniFixBuilderFuncs.ganyuTwilight5_7,
        ModTypeNames.Kirara.value: IniFixBuilderFuncs.kirara5_7,
        ModTypeNames.KiraraBoots.value: IniFixBuilderFuncs.kiraraBoots5_7,
        ModTypeNames.Lisa.value: IniFixBuilderFuncs.lisa5_7,
        ModTypeNames.Nilou.value: IniFixBuilderFuncs.nilou5_7,
        ModTypeNames.NilouBreeze.value: IniFixBuilderFuncs.nilouBreeze5_7,
    }
}
##### EndScript