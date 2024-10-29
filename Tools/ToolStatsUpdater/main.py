import os
import sys

from ToolStatsUpdater.constants.Paths import APIPath, UtilitiesPath
from ToolStatsUpdater.ToolStatsUpdater import ToolStatsUpdater
from ToolStatsUpdater.updaters.TomlUpdater import TomlUpdater

sys.path.insert(1, UtilitiesPath)
from Utils.constants.toolStats import APIStats


if __name__ == "__main__":
    updaters = [
        TomlUpdater(os.path.join(APIPath, "pyproject.toml"), APIStats)
    ]

    toolStatUpdater = ToolStatsUpdater(updaters)
    toolStatUpdater.update()