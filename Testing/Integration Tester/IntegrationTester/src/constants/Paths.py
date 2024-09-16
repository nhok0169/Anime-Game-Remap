import os, sys

UtilitiesPath = os.path.join("..", "..", "Tools", "Utilities")
APIPath = os.path.join("..", "..", "Fix-Raiden-Boss 2.0 (for all user )", "api")
ScriptPath = os.path.join("..", "..", "Fix-Raiden-Boss 2.0 (for all user )", "script build")


sys.path.insert(1, UtilitiesPath)
from Utils.enums.SysEnum import SysEnum


SysPaths = {SysEnum.API: APIPath,
            SysEnum.Script: ScriptPath}