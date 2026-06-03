import os

from ..path.ModulePathTools import ModulePathTools

ProjectMainFolder = r"Anime Game Remap (for all users)"
APIFolder = os.path.join(ProjectMainFolder, "api")
ScriptFolder = os.path.join(ProjectMainFolder, "script build")
MirrorFolder = os.path.join(ProjectMainFolder, "apiMirror")

ModulePath = ModulePathTools.join("src", "py", "FixRaidenBoss2")
ModuleRelFolder = ModulePathTools.toFilePath(ModulePath)
APISrcFolder = os.path.join(APIFolder, "src")

APIPyFolder = os.path.join(APISrcFolder, "py")
APICyFolder = os.path.join(APISrcFolder, "cy")
APICppFolder = os.path.join(APISrcFolder, "cpp")
APIPyBindFolder = os.path.join(APICppFolder, "py")
APICoreFolder = os.path.join(APICppFolder, "core")

APIPySrcFolder = os.path.join(APIFolder, ModuleRelFolder)
APICySrcFolder = os.path.join(APICyFolder, "src")
APIPyBindSrcFolder = os.path.join(APIPyBindFolder, "src")
APICoreSrcFolder = os.path.join(APICoreFolder, "src")

ScriptSrcFolder = os.path.join(ScriptFolder, ModuleRelFolder)
MirrorSrcFolder = os.path.join(MirrorFolder, "src", "AnimeGameRemap")