from typing import Optional, List
import subprocess
import sys


# Stage: A single stage in the pipeline
class Stage():
    def __init__(self, name: str, src: str, argv: Optional[List[str]] = None):
        self.name = name
        self.src = src
        self.argv = argv

    def run(self):
        processName = ""
        if (self.src.endswith(".py")):
            # note: the interpreter running the pipeline, rather than whichever one a bare 'python'
            #   happens to resolve to. Those are not always the same -- on the machine this was
            #   written on, 'python' is 3.9.13 while the interpreter running the pipeline is 3.9.3 --
            #   and on a machine where 'python' is not on PATH at all, the bare name finds nothing.
            processName = sys.executable

        subProcessArgs = [processName, self.src]
        if (self.argv is not None):
            subProcessArgs += self.argv
        subprocess.run(subProcessArgs, check=True)