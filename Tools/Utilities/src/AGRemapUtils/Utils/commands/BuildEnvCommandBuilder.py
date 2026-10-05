from typing import Any, Dict, Optional

from .BaseCommandBuilder import BaseCommandBuilder
from ..exceptions.InvalidBuildEnv import InvalidBuildEnv


# BuildEnvCommandBuilder: Base class for the command of a tool that builds for a particular environment
#
# note: the configuration below lives on the CLASS rather than on the instance, because
#   BaseCommandBuilder adds the arguments from within its own __init__ -- anything a subclass sets
#   after calling super().__init__() would arrive too late to be read.
class BuildEnvCommandBuilder(BaseCommandBuilder):
    # Envs: The enum of the available environments
    Envs = None

    # DefaultEnv: The environment used when the option is not given
    DefaultEnv = None

    # EnvSubject: What is being built, for the option's help text
    EnvSubject = ""

    # EnvDescriptions: What each environment means, keyed by the environment's value
    EnvDescriptions: Dict[str, str] = {}

    EnvOpt = "--env"
    ShortEnvOpt = "-e"

    # getEnvHelp(): Retrieves the help text for the environment option
    @classmethod
    def getEnvHelp(cls) -> str:
        result = f"The environment to build {cls.EnvSubject} for. Below are the available environments:\n\n"

        for env in cls.Envs:
            description = cls.EnvDescriptions.get(env.value, "")
            result += f"{env.value}: {description}\n"

        result += f"\nBy default, will use the {cls.DefaultEnv.value} environment"
        return result

    def _addArguments(self):
        self._argParser.add_argument(self.ShortEnvOpt, self.EnvOpt, action = "store", type = str, help = self.getEnvHelp())

    def _parseEnv(self):
        env = self._args.env

        if (env is None):
            self._args.env = self.DefaultEnv
            return

        foundEnv = self.Envs.match(env)
        if (foundEnv is None):
            raise InvalidBuildEnv(env, map(lambda buildEnv: buildEnv.value, self.Envs))

        self._args.env = foundEnv

    def parse(self) -> Any:
        self.parseArgs()
        self._parseEnv()
        return self._args
