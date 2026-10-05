import os
import sys

from CIPipeline.constants.Paths import APIBuilderPath, ToolStatsUpdaterPath, ScriptBuilderPath, APIMirrorBuilderPath, UtilitiesPath
from CIPipeline.constants.CommandOpts import ShortCommandOpts
from CIPipeline.CommandBuilder import CommandBuilder

sys.path.insert(1, UtilitiesPath)
from Utils.pipeline.Pipeline import Pipeline
from Utils.pipeline.Stage import Stage


# APIBuilderDocsOpt: Builds the documentation related files alongside the binaries
#
# note: the APIBuilder has an environment option of its own, but its environments are a different
#   set from this pipeline's ('dev'/'core'/'cibuildwheel' against 'dev'/'prod'), and the production
#   wheels are built by cibuildwheel in the publish workflow rather than through the APIBuilder. So
#   the pipeline's environment is not handed down to it.
APIBuilderDocsOpt = "-d"


if __name__ == "__main__":
    command = CommandBuilder()
    args = command.parse()

    # the environment is only handed to the stages that actually take one. The mirror and the
    #   software metadata come out the same either way, so passing it to them would be noise.
    envArgs = [ShortCommandOpts.Env.value, args.env.value]

    stages = [
        Stage("Building API and Docs", os.path.join(APIBuilderPath, "main.py"), argv = [APIBuilderDocsOpt]),
        Stage("Compiling Script", os.path.join(ScriptBuilderPath, "main.py"), argv = envArgs),
        Stage("Compiling API Mirror", os.path.join(APIMirrorBuilderPath, "main.py")),
        Stage("Updating Software Metadata", os.path.join(ToolStatsUpdaterPath, "main.py"))
    ]

    print(f"Running the pipeline for the '{args.env.value}' environment\n")

    pipeline = Pipeline(stages)
    pipeline.run()
