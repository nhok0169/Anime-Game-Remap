from typing import List

from .Stage import Stage
from ..Heading import Heading


class Pipeline():
    def __init__(self, stages: List[Stage]):
        self.stages = stages

    def run(self):
        stageHeading = Heading(sideLen = 5)
        stagesLen = len(self.stages)

        for i in range(stagesLen):
            stage = self.stages[i]

            stageHeading.title = f"Stage {i + 1}: {stage.name}"
            print(f"{stageHeading.open()}\n")

            stage.run()

            print(f"\n{stageHeading.close()}")
