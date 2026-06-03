import sys

from .Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


class BuildEnv(StrEnum):
    Dev = "dev"
    CIBuildWheel = "cibuildwheel"
    Core = "core"


CmakeBuildEnv = {
    BuildEnv.Dev: "python_dev",
    BuildEnv.CIBuildWheel: "cibuildwheel",
    BuildEnv.Core: ""
}