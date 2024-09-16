import sys

from .Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


class ConfigKeys(StrEnum):
    Command = "command"
    System = "system"
    SysPath = "systemPath"
    