from typing import List
import sys

from .Stage import Stage
from ..Heading import Heading


class Pipeline():
    def __init__(self, stages: List[Stage]):
        self.stages = stages

    # _print(txt): Prints out text that is guaranteed to land before whatever a stage prints next
    #
    # note: a stage's output comes from a subprocess writing straight to the same handle, while this
    #   process' own output sits in a buffer until it fills. Without the flush, every heading arrives
    #   after the stage it introduces, and a log reader cannot tell which stage produced what.
    def _print(self, txt: str):
        print(txt)
        sys.stdout.flush()

    def run(self):
        stageHeading = Heading(sideLen = 5)
        stagesLen = len(self.stages)

        for i in range(stagesLen):
            stage = self.stages[i]

            stageHeading.title = f"Stage {i + 1}: {stage.name}"
            self._print(f"{stageHeading.open()}\n")

            stage.run()

            self._print(f"\n{stageHeading.close()}")
