import sys

from .Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


class BuildEnv(StrEnum):
    Dev = "dev"
    CIBuildWheel = "cibuildwheel"
    Core = "core"


# CmakeBuildEnv: the value of the API's BUILD_MODE CMake option for each environment
#   (see the BUILD_MODE checks in the api's CMakeLists.txt)
CmakeBuildEnv = {
    BuildEnv.Dev: "python_dev",
    BuildEnv.CIBuildWheel: "cibuildwheel",
    BuildEnv.Core: "core_sdk"
}