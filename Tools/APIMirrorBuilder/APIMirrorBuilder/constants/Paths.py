import os
import sys

UtilitiesPath = os.path.join("..", "Utilities", "src", "AGRemapUtils")

sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import ProjectMainFolder, APIFolder, APISrcFolder, MirrorSrcFolder

RelPathToProject = os.path.join("..", "..")
ProjectPath = os.path.join(RelPathToProject, ProjectMainFolder)
APIPath = os.path.join(RelPathToProject, APIFolder)
APIFullPath = os.path.join(RelPathToProject, APISrcFolder)
MirrorFullPath = os.path.join(RelPathToProject, MirrorSrcFolder)