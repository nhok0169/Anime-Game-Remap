import os, sys

UtilitiesPath = os.path.join("..", "..", "Tools", "Utilities")
APIPath = os.path.join("..", "..", "Anime Game Remap (for all users)", "api")
ScriptPath = os.path.join("..", "..", "Anime Game Remap (for all users)", "script build")


sys.path.insert(1, UtilitiesPath)
from Utils.enums.SysEnum import SysEnum


SysPaths = {SysEnum.API: APIPath,
            SysEnum.Script: ScriptPath}