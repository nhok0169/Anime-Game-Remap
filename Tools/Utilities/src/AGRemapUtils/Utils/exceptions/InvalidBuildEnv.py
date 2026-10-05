from typing import Iterable

from .Error import Error


# InvalidBuildEnv: Exception when the name of some build environment is not found
class InvalidBuildEnv(Error):
    def __init__(self, searchedEnv: str, availableEnvs: Iterable[str]):
        available = ", ".join(map(lambda env: f"'{env}'", availableEnvs))
        super().__init__(f"Unable to find the build environment by the name, '{searchedEnv}'. The available environments are: {available}")
