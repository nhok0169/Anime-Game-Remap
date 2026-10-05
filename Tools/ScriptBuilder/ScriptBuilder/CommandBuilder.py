import sys

from .constants.UtilitiesPath import UtilitiesPath
from .constants.CommandOpts import CommandOpts, ShortCommandOpts
from .constants.Paths import ScriptToolPath

sys.path.insert(1, UtilitiesPath)
from Utils.commands.BuildEnvCommandBuilder import BuildEnvCommandBuilder

sys.path.insert(1, ScriptToolPath)
from Script.constants.BuildEnvs import BuildEnv


# CommandBuilder: Handles the options for building the script
#
# note: BuildEnv comes from the script's own source rather than from here. It is what a compiled
#   script reads to know how to reach the API, so the script has to carry it -- and having the
#   builder import the same enum keeps the two from drifting apart.
class CommandBuilder(BuildEnvCommandBuilder):
    Envs = BuildEnv
    DefaultEnv = BuildEnv.Dev
    EnvSubject = "the script"
    EnvOpt = CommandOpts.Env.value
    ShortEnvOpt = ShortCommandOpts.Env.value

    EnvDescriptions = {
        BuildEnv.Dev.value: "The script reaches the API by a path on this machine, the way the tools in this repo reach each other",
        BuildEnv.Prod.value: "The script downloads the API from pypi when it runs, and gains the options for controlling that download"
    }
