from typing import List

from .updaters.BaseUpdater import BaseUpdater


# ToolStatsUpdater: Class to help update all software metadata
#   accross the repo
class ToolStatsUpdater():
    def __init__(self, updaters: List[BaseUpdater]):
        self.updaters = updaters

    # update(): Updates all the software metadata needed
    def update(self):
        for updater in self.updaters:
            updater.update()