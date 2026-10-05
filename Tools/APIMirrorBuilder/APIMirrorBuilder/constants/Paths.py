import os
import sys

UtilitiesPath = os.path.join("..", "Utilities", "src", "AGRemapUtils")

sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import ProjectMainFolder, APIFolder, APIPySrcFolder, MirrorFolder, MirrorSrcFolder

RelPathToProject = os.path.join("..", "..")
ProjectPath = os.path.join(RelPathToProject, ProjectMainFolder)

# the folders holding each project's pyproject.toml, LICENSE and README
APIPath = os.path.join(RelPathToProject, APIFolder)
MirrorPath = os.path.join(RelPathToProject, MirrorFolder)

# the folders holding each project's python package
#
# note: these are NOT the same depth as each other -- the API's package sits under 'src/py' next to
#   its 'src/cpp' and 'src/cy' layers, while the mirror is pure python and keeps its package at 'src'
APIFullPath = os.path.join(RelPathToProject, APIPySrcFolder)
MirrorFullPath = os.path.join(RelPathToProject, MirrorSrcFolder)