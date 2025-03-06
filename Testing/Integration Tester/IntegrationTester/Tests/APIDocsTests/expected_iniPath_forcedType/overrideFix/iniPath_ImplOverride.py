import sys
import os
import copy

from IntegrationTester.src.constants.ConfigKeys import ConfigKeys
from IntegrationTester.src.Config import Config

sys.path.insert(1, Config[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


iniRunPath = FRB.FileService.parseOSPath(os.path.dirname(os.path.abspath(__file__)))
kiraraModType = FRB.ModTypes.Kirara.value

oldParseBuilder = copy.deepcopy(kiraraModType.iniParseBuilder)
oldFixBuilder = copy.deepcopy(kiraraModType.iniFixBuilder)

kiraraModType.iniParseBuilder = FRB.IniParseBuilder(FRB.GIMIObjParser, args = [{"head", "body"}], kwargs = {"texEdits": {
    "body": {"ps-t1": {"DarkenDiffuse": FRB.TexEditor(filters = [FRB.TexMetadataFilter(edits = {"gamma": FRB.ColourConsts.SRGBGamma.value})])}}
}})

kiraraModType.iniFixBuilder = FRB.IniFixBuilder(FRB.GIMIObjMergeFixer, args = [{"head": ["head", "body"], "body": ["body"]}], 
                                                kwargs = {
                                                    "preRegEditFilters": [
                                                        FRB.RegTexEdit({"DarkenDiffuse": ["ps-t1"]})
                                                    ],
                                                    "postRegEditFilters": [
                                                        FRB.RegNewVals({"body": {"ib": "null"}})
                                                    ]
                                                })

remapService = FRB.RemapService(path = iniRunPath, verbose = False, keepBackups = False)
remapService.fix()

kiraraModType.iniParseBuilder = oldParseBuilder
kiraraModType.iniFixBuilder = oldFixBuilder