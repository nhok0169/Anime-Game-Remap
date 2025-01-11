import os

from ..path.ModulePathTools import ModulePathTools

ProjectMainFolder = r"Anime Game Remap (for all users)"
APIFolder = os.path.join(ProjectMainFolder, "api")
ScriptFolder = os.path.join(ProjectMainFolder, "script build")
MirrorFolder = os.path.join(ProjectMainFolder, "apiMirror")

ModulePath = ModulePathTools.join("src", "FixRaidenBoss2")
ModuleRelFolder = ModulePathTools.toFilePath(ModulePath)
APISrcFolder = os.path.join(APIFolder, ModuleRelFolder)
ScriptSrcFolder = os.path.join(ScriptFolder, ModuleRelFolder)
MirrorSrcFolder = os.path.join(MirrorFolder, "src", "AnimeGameRemap")