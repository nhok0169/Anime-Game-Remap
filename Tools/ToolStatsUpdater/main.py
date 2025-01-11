import os
import sys

from ToolStatsUpdater.constants.Paths import APIPath, UtilitiesPath, UtilitiesProjectPath
from ToolStatsUpdater.ToolStatsUpdater import ToolStatsUpdater

sys.path.insert(1, UtilitiesPath)
from Utils.constants.toolStats import APIStats, UtilityStats
from Utils.toolStatsUpdater.TomlUpdater import TomlUpdater


if __name__ == "__main__":
    updaters = [
        TomlUpdater(os.path.join(APIPath, "pyproject.toml"), APIStats),
        TomlUpdater(os.path.join(UtilitiesProjectPath, "pyproject.toml"), UtilityStats)
    ]

    toolStatUpdater = ToolStatsUpdater(updaters)
    toolStatUpdater.update()