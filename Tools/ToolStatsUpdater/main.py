import os
import sys

from ToolStatsUpdater.constants.Paths import APIPath, UtilitiesPath
from ToolStatsUpdater.ToolStatsUpdater import ToolStatsUpdater

sys.path.insert(1, UtilitiesPath)
from Utils.constants.toolStats import APIStats
from Utils.toolStatsUpdater.TomlUpdater import TomlUpdater


if __name__ == "__main__":
    updaters = [
        TomlUpdater(os.path.join(APIPath, "pyproject.toml"), APIStats)
    ]

    toolStatUpdater = ToolStatsUpdater(updaters)
    toolStatUpdater.update()