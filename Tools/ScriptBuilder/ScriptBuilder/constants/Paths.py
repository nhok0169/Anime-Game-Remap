import os

from ..tools.PathTools import FilePathTools, ModulePathTools

UtilitiesPath = os.path.join("..", "Utilities")
ProjectPath = os.path.join("..", "..", r"Fix-Raiden-Boss 2.0 (for all user )")

APIPath = FilePathTools.parseOSPath(os.path.join(ProjectPath, "api"))
ModulePath = ModulePathTools.join("src", "FixRaidenBoss2")
ModuleRelFilePath = ModulePathTools.toFilePath(ModulePath)
APIFullPath = os.path.join(APIPath, ModuleRelFilePath)

ScriptFolderPath = os.path.join(ProjectPath, "script build", ModuleRelFilePath)