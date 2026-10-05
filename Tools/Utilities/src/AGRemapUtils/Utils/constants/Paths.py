import os

from ..path.ModulePathTools import ModulePathTools

ProjectMainFolder = r"Anime Game Remap (for all users)"
APIFolder = os.path.join(ProjectMainFolder, "api")
ScriptFolder = os.path.join(ProjectMainFolder, "script build")
MirrorFolder = os.path.join(ProjectMainFolder, "apiMirror")

ModulePath = ModulePathTools.join("src", "py", "FixRaidenBoss2")

# the script is its own source project now, rather than the API flattened into one file
ScriptModulePath = "Script"
ScriptToolFolder = os.path.join("Tools", "Script")
ModuleRelFolder = ModulePathTools.toFilePath(ModulePath)
APISrcFolder = os.path.join(APIFolder, "src")
APIExternFolder = os.path.join(APIFolder, "extern")

APIPyFolder = os.path.join(APISrcFolder, "py")
APICyFolder = os.path.join(APISrcFolder, "cy")
APICppFolder = os.path.join(APISrcFolder, "cpp")
APIPyBindFolder = os.path.join(APICppFolder, "py")
APICoreFolder = os.path.join(APICppFolder, "core")

APIPySrcFolder = os.path.join(APIFolder, ModuleRelFolder)
APICySrcFolder = os.path.join(APICyFolder, "src")
APIPyBindSrcFolder = os.path.join(APIPyBindFolder, "src")
APICoreSrcFolder = os.path.join(APICoreFolder, "src")
APICoreIncludeFolder = os.path.join(APICoreFolder, "include")
APICoreTestsFolder = os.path.join(APICoreFolder, "tests")

# note: the script build's package is its own folder, NOT the API's module path. It used to be
#   derived from ModuleRelFolder, which silently moved the script build to 'src/py/FixRaidenBoss2'
#   when the API's package moved down a level for its C++ and Cython layers.
ScriptSrcFolder = os.path.join(ScriptFolder, "src", "FixRaidenBoss2")
MirrorSrcFolder = os.path.join(MirrorFolder, "src", "AnimeGameRemap")