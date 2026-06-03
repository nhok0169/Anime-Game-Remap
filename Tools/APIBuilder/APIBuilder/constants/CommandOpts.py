import sys

from .Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


class CommandOpts(StrEnum):
    Env = "--env"
    BuildKeep = "--buildKeep"
    InstallKeep = "--installKeep"
    SkipBuild = "--skipBuild"
    AddDocs = "--addDocs"
    InstallFolder = "--installFolder"


class ShortCommandOpts(StrEnum):
    Env = "-e"
    BuildKeep = "-b"
    InstallKeep = "-i"
    SkipBuild = "-s"
    AddDocs = "-d"
    InstallFolder = "-f"