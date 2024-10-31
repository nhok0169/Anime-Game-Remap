from typing import Optional, List
import subprocess


# Stage: A single stage in the pipeline
class Stage():
    def __init__(self, name: str, src: str, argv: Optional[List[str]] = None):
        self.name = name
        self.src = src
        self.argv = argv

    def run(self):
        processName = ""
        if (self.src.endswith(".py")):
            processName = "python"

        subProcessArgs = [processName, self.src]
        if (self.argv is not None):
            subProcessArgs += self.argv
        subprocess.run(subProcessArgs, check=True)