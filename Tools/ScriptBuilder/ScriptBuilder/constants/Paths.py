import os
import sys

from .UtilitiesPath import UtilitiesPath
sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import ProjectMainFolder, APIFolder, APISrcFolder, ScriptSrcFolder


RelPathToProject = os.path.join("..", "..")
ProjectPath = os.path.join(RelPathToProject, ProjectMainFolder)
APIPath = os.path.join(RelPathToProject, APIFolder)
APIFullPath = os.path.join(RelPathToProject, APISrcFolder)

ScriptFolderPath = os.path.join(RelPathToProject, ScriptSrcFolder)