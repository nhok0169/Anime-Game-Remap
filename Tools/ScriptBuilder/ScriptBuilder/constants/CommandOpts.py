import sys

from .UtilitiesPath import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


class CommandOpts(StrEnum):
    Env = "--env"


class ShortCommandOpts(StrEnum):
    Env = "-e"
