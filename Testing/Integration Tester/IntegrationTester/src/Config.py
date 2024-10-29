import sys

from .constants.Commands import Commands
from .constants.Paths import UtilitiesPath, SysPaths
from .constants.ConfigKeys import ConfigKeys

sys.path.insert(1, UtilitiesPath)
from Utils.enums.SysEnum import SysEnum


# Configurations for the program are used as a global variable to
#   allow passing of configuration to the integration tests
Config = {ConfigKeys.Command: Commands.RunSuite,
          ConfigKeys.System: SysEnum.API,
          ConfigKeys.SysPath: SysPaths[SysEnum.API]}
