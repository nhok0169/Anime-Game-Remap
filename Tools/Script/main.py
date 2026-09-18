import os
import sys

ScriptToolPath = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(1, ScriptToolPath)

from Script.apiRefs.PathApiRef import PathApiRef
from Script.main import remapMain


# where the API's package sits, relative to this tool
APIPath = os.path.join(ScriptToolPath, "..", "..", "Anime Game Remap (for all users)", "api", "src", "py")


if __name__ == "__main__":
    # a compiled script reads the API's location out of the values the ScriptBuilder filled in.
    #   Running from source there is nothing filled in yet, so the path is handed over directly.
    remapMain(apiRef = PathApiRef("FixRaidenBoss2", apiPath = os.path.abspath(APIPath)))
