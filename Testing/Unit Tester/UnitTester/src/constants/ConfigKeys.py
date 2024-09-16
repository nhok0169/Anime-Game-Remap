import sys

from ..constants.Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


# ConfigKeys: Enums for the names of the configurations available
class ConfigKeys(StrEnum):
    System = "system"
    SysPath = "systemPath"