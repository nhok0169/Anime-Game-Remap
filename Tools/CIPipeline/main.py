import os

from CIPipeline.constants.Paths import ToolStatsUpdaterPath, ScriptBuilderPath
from CIPipeline.Pipeline import Pipeline
from CIPipeline.Stage import Stage

if __name__ == "__main__":
    stages = [
        Stage("Compiling Script", os.path.join(ScriptBuilderPath, "main.py")),
        Stage("Updating Software Metadata", os.path.join(ToolStatsUpdaterPath, "main.py"))
    ]

    pipeline = Pipeline(stages)
    pipeline.run()