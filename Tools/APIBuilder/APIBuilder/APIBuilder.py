import shutil
import subprocess
import os
import sys
from pathlib import Path
from typing import List, Optional
from types import SimpleNamespace

from .constants.Paths import APIPyFolderPath, APITopBuildFolderPath, APIPath, BuildFolder, APICoreXMLFolderPath, APICoreFolderPath, BuildFolder, PathToProject, RemoveAllFolder, PreBuildFolder, PreInstallFolder, APITopPreBuildFolderPath, APIExternFolderPath, APITopPreInstallFolderPath
from .constants.BuildEnv import BuildEnv


class APIBuilder():
    _PackageName = "FixRaidenBoss2"

    def __init__(self, env: BuildEnv = BuildEnv.Dev, installPath: str = APIPyFolderPath, cleanPreBuild: Optional[str] = None, cleanPreInstall: Optional[str] = None, cleanBuild: Optional[str] = None, 
                 cleanInstall: bool = True, makeBuild: bool = True, addDocs: bool = False, makePreBuild: bool = False, makePreInstall: bool = False, buildSuffix: str = "", preBuildSuffix: str = "", preInstallSuffix: str = ""):
        self.env = env
        self.installPath = installPath
        self.cleanPreBuild = cleanPreBuild
        self.cleanBuild = cleanBuild
        self.cleanPreInstall = cleanPreInstall
        self.cleanInstall = cleanInstall
        self.makeBuild = makeBuild
        self.addDocs = addDocs
        self.buildSuffix = buildSuffix
        self.preBuildSuffix = preBuildSuffix
        self.preInstallSuffix = preInstallSuffix
        self.makePreBuild = makePreBuild
        self.makePreInstall = makePreInstall

        self._preBuildFolder = ""
        self._preInstallFolder = ""
        self._buildFolder = ""
        self._extBuildFolders = SimpleNamespace()
        self._extInstallFolders = SimpleNamespace()
        self._buildFoldersIsSet = False

    def __call__(self):
        self.run()

    def run(self):
        if (self.cleanPreBuild is not None):
            self.removePrefixedFolder(PathToProject, PreBuildFolder, self.cleanPreBuild)

        if (self.cleanPreInstall is not None):
            self.removePrefixedFolder(PathToProject, PreInstallFolder, self.cleanPreInstall)

        if (self.cleanBuild is not None):
            self.removePrefixedFolder(PathToProject, BuildFolder, self.cleanBuild)

        if (self.cleanInstall):
            self.cleanInstalls()

        if (self.makePreBuild):
            self.preBuildExterns()

        if (self.makePreInstall):
            self.preInstallExterns()

        if (self.makeBuild):
            self.buildAPI()

        if (self.addDocs):
            self.buildDocs()

    def removePrefixedFolder(self, srcFolder: str, folderPrefix: str, folderSuffix: str):
        if (folderSuffix == RemoveAllFolder):
            srcFolder = Path(srcFolder)

            for item in srcFolder.glob(f"{folderPrefix}*"):
                if item.is_file() or item.is_symlink():
                    item.unlink()
                elif item.is_dir():
                    shutil.rmtree(item)

        else:
            folderSuffix = folderSuffix[:-1]
            targetFolder = Path(srcFolder) / f"{folderPrefix}{folderSuffix}"
    
            if targetFolder.is_dir() and not targetFolder.is_symlink():
                shutil.rmtree(targetFolder)

    def cleanInstalls(self):
        basePath = Path(APIPath).resolve()
        
        targetExtensions = {'.so', '.pyd'}

        for filePath in basePath.rglob('*'):
            # note: is_file() stats the path, and a path the OS refuses to stat raises here rather
            #   than returning False -- which aborts the entire build before anything is compiled.
            #   Seen with a symlink created from WSL inside a checkout shared with Windows: it
            #   lands on the Windows drive as an LX_SYMLINK reparse point that Windows cannot read,
            #   giving "OSError: [WinError 1920] The file cannot be accessed by the system".
            #   Anything unreadable is also, by definition, not something worth deleting here.
            try:
                isFile = filePath.is_file()
            except OSError:
                continue

            if isFile and filePath.suffix in targetExtensions:

                isExcluded = any(
                    parent.name.startswith(BuildFolder)
                    for parent in filePath.parents
                )

                if isExcluded:
                    continue

                filePath.unlink()

    def _setupBuildFolders(self):
        if (not self._buildFoldersIsSet):
            # note: the suffix is concatenated onto the folder name (cbuild + "lin" -> cbuildlin), not
            #   joined as a subfolder -- this matches removePrefixedFolder's delete target, the
            #   --buildSuffix/--prebuildSuffix/--preinstallSuffix help text, and the "cannot contain
            #   slashes" restriction CommandBuilder enforces on suffix names
            self._preBuildFolder = f"{APITopPreBuildFolderPath}{self.preBuildSuffix}"
            self._preInstallFolder = f"{APITopPreInstallFolderPath}{self.preInstallSuffix}"
            self._buildFolder = f"{APITopBuildFolderPath}{self.buildSuffix}"

            self._extBuildFolders.z3 = os.path.join(self._preBuildFolder, "z3")
            self._extInstallFolders.z3 = os.path.join(self._preInstallFolder, "z3")

            self._buildFoldersIsSet = True

    def preBuildExterns(self):
        self._setupBuildFolders()

        if (not os.path.isdir(self._extBuildFolders.z3)):
            z3SrcFolder = os.path.join(APIExternFolderPath, "z3")
            os.chdir(z3SrcFolder)

            subprocess.run(["cmake", "-G", "Ninja", "-B", self._extBuildFolders.z3, "-DCMAKE_BUILD_TYPE=Release", f"-DZ3_DIR={self._extBuildFolders.z3}"], check=True)
            subprocess.run(["cmake", "--build", self._extBuildFolders.z3, "--parallel"], check=True)

    def preInstallExterns(self):
        self._setupBuildFolders()

        if (not os.path.isdir(self._extInstallFolders.z3)):
            subprocess.run(["cmake", "--install", self._extBuildFolders.z3, "--prefix", self._extInstallFolders.z3])

    def buildAPI(self):
        self._setupBuildFolders()
        os.chdir(APIPath)

        subprocess.run(["cmake", "-G", "Ninja", "-B", self._buildFolder, "-DCMAKE_BUILD_TYPE=Release", f"-DCMAKE_PREFIX_PATH={self._extInstallFolders.z3}"], check=True)
        subprocess.run(["cmake", "--build", self._buildFolder, "--parallel"], check=True)
        subprocess.run(["cmake", "--install", self._buildFolder,  "--prefix", f'{self.installPath}'], check=True)

    def getInstallModuleNames(self) -> List[str]:
        result = []
        
        for root, dirs, files in os.walk(APIPyFolderPath):
            for file in files:
                if file.endswith(('.so', '.pyd')):
                    moduleName = file.split('.', 1)[0]
                    result.append(moduleName)
                    
        return result

    def buildDocs(self):
        # Pybind11 documentation setup
        os.chdir(APIPyFolderPath)

        env = os.environ.copy()
        currPythonPath = env.get("PYTHONPATH", "")

        if currPythonPath:
            env["PYTHONPATH"] = f"{APIPyFolderPath}{os.pathsep}{currPythonPath}"
        else:
            env["PYTHONPATH"] = APIPyFolderPath

        moduleNames = self.getInstallModuleNames()
        for moduleName in moduleNames:
            try:
                subprocess.run([sys.executable, "-m", "pybind11_stubgen", f"{self._PackageName}.{moduleName}", "-o", APIPyFolderPath, "--root-suffix", ""], env=env, check=True, capture_output=True, text=True)
            except subprocess.CalledProcessError as e:
                print(f"Error Details:\n{e.stderr}")

        # Doxygen documentation setup
        os.chdir(APICoreFolderPath)
        shutil.rmtree(APICoreXMLFolderPath, ignore_errors=True)

        subprocess.run(["doxygen", "Doxyfile"], check=True)

