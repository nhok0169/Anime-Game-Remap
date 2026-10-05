import os
import sys

UtilitiesPath = os.path.join("..", "Utilities", "src", "AGRemapUtils")

sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import ProjectMainFolder

APIBuilderPath = os.path.join("..", "APIBuilder")
ScriptBuilderPath = os.path.join("..", "ScriptBuilder")
ToolStatsUpdaterPath = os.path.join("..", "ToolStatsUpdater")
APIMirrorBuilderPath = os.path.join("..", "APIMirrorBuilder")

# the script's own source, which holds the environments the pipeline builds for
ScriptToolPath = os.path.join("..", "Script")

ProjectPath = os.path.join("..", "..", ProjectMainFolder)
APIPath = os.path.join(ProjectPath, "api")
