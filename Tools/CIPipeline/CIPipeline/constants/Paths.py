import os
import sys

UtilitiesPath = os.path.join("..", "Utilities", "src", "AGRemapUtils")

sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import ProjectMainFolder

ScriptBuilderPath = os.path.join("..", "ScriptBuilder")
ToolStatsUpdaterPath = os.path.join("..", "ToolStatsUpdater")
APIMirrorBuilderPath = os.path.join("..", "APIMirrorBuilder")
ProjectPath = os.path.join("..", "..", ProjectMainFolder)
APIPath = os.path.join(ProjectPath, "api")

