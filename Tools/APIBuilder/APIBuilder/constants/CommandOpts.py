import sys

from .Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


class CommandOpts(StrEnum):
    Env = "--env"
    PrebuildRemove = "--prebuildRemove"
    BuildRemove = "--buildRemove"
    PreInstallRemove = "--preinstallRemove"
    InstallKeep = "--installKeep"
    SkipBuild = "--skipBuild"
    AddDocs = "--addDocs"
    InstallFolder = "--installFolder"
    MakePreBuild = "--makePreBuild"
    MakePreInstall = "--makePreInstall"
    BuildSuffix = "--buildSuffix"
    PrebuildSuffix = "--prebuildSuffix"
    PreinstallSuffix = "--preinstallSuffix"


class ShortCommandOpts(StrEnum):
    Env = "-e"
    PrebuildRemove = "-p"
    BuildRemove = "-b"
    PreInstallRemove = "-pir"
    InstallKeep = "-i"
    SkipBuild = "-s"
    AddDocs = "-d"
    InstallFolder = "-f"
    MakePreBuild = "-pb"
    MakePreInstall = "-pi"
    BuildSuffix = "-bs"
    PrebuildSuffix = "-ps"
    PreinstallSuffix = "-pis"