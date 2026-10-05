import os
import sys

from .UtilitiesPath import UtilitiesPath
sys.path.insert(1, UtilitiesPath)
from Utils.constants.Paths import ProjectMainFolder, APIFolder, APIPyFolder, ScriptSrcFolder, ScriptModulePath
from Utils.path.ModulePathTools import ModulePathTools


RelPathToProject = os.path.join("..", "..")
ProjectPath = os.path.join(RelPathToProject, ProjectMainFolder)
APIPath = os.path.join(RelPathToProject, APIFolder)

# the folder CONTAINING the API's package, which is what a 'dev' build of the script puts onto its
#   import path -- the package folder itself would be one level too deep to import from
APIFullPath = os.path.join(RelPathToProject, APIPyFolder)

# the script's own source, which is what actually gets compiled now
ScriptToolPath = os.path.join("..", "Script")
ScriptModuleFolder = os.path.join(ScriptToolPath, ModulePathTools.toFilePath(ScriptModulePath))

# where the compiled script is written
ScriptFolderPath = os.path.join(RelPathToProject, ScriptSrcFolder)
