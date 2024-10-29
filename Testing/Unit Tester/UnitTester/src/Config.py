import sys

from .constants.ConfigKeys import ConfigKeys
from .constants.Paths import UtilitiesPath, SysPaths

sys.path.insert(1, UtilitiesPath)
from Utils.enums.SysEnum import SysEnum


# Initial configurations for the tester
Configs = {ConfigKeys.System: SysEnum.API,
           ConfigKeys.SysPath: SysPaths[SysEnum.API]}