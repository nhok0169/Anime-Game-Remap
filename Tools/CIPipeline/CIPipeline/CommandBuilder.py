import sys

from .constants.Paths import UtilitiesPath, ScriptToolPath
from .constants.CommandOpts import CommandOpts, ShortCommandOpts

sys.path.insert(1, UtilitiesPath)
from Utils.commands.BuildEnvCommandBuilder import BuildEnvCommandBuilder

sys.path.insert(1, ScriptToolPath)
from Script.constants.BuildEnvs import BuildEnv


# CommandBuilder: Handles the options for running the pipeline
#
# note: the same environments as the ScriptBuilder's, from the same enum. The pipeline's environment
#   is the one it hands down to whichever of its stages takes one.
#
# note: this defaults to Prod where the ScriptBuilder defaults to Dev, and the difference is
#   deliberate. The pipeline writes the deliverables that get committed and shipped -- a 'dev' script
#   looks for the API at a path on the machine that compiled it, so shipping one is broken for every
#   user. Running the ScriptBuilder on its own is the case where you are working ON the script and
#   want it pointed at the API in this repo.
class CommandBuilder(BuildEnvCommandBuilder):
    Envs = BuildEnv
    DefaultEnv = BuildEnv.Prod
    EnvSubject = "the deliverables"
    EnvOpt = CommandOpts.Env.value
    ShortEnvOpt = ShortCommandOpts.Env.value

    EnvDescriptions = {
        BuildEnv.Dev.value: "The script reaches the API by a path on this machine, the way the tools in this repo reach each other",
        BuildEnv.Prod.value: "The script downloads the API from pypi when it runs, and gains the options for controlling that download"
    }
