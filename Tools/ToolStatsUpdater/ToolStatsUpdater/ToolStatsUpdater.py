import sys
from typing import List

from .constants.Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.toolStatsUpdater.BaseUpdater import BaseUpdater


# ToolStatsUpdater: Class to help update all software metadata
#   accross the repo
class ToolStatsUpdater():
    def __init__(self, updaters: List[BaseUpdater]):
        self.updaters = updaters

    # update(): Updates all the software metadata needed
    def update(self):
        for updater in self.updaters:
            updater.update()