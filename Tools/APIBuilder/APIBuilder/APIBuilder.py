import shlex
import shutil
import stat
import subprocess
import os
import sys
from pathlib import Path
from typing import List, Optional
from types import SimpleNamespace

from .constants.Paths import UtilitiesPath, APIPyFolderPath, APITopBuildFolderPath, APIPath, BuildFolder, APICoreXMLFolderPath, APICoreFolderPath, BuildFolder, PathToProject, RemoveAllFolder, PreBuildFolder, PreInstallFolder, APITopPreBuildFolderPath, APIExternFolderPath, APITopPreInstallFolderPath, APISrcFolderPaths, CMakeArgsEnvVar
from .constants.BuildEnv import BuildEnv

sys.path.insert(1, UtilitiesPath)
from Utils.credits.CreditsUpdater import CreditsUpdater


class APIBuilder():
    _PackageName = "FixRaidenBoss2"

    def __init__(self, env: BuildEnv = BuildEnv.Dev, installPath: str = APIPyFolderPath, cleanPreBuild: Optional[str] = None, cleanPreInstall: Optional[str] = None, cleanBuild: Optional[str] = None, 
                 cleanInstall: bool = True, makeBuild: bool = True, addDocs: bool = False, addCredits: bool = False, makePreBuild: bool = False, makePreInstall: bool = False, buildSuffix: str = "", preBuildSuffix: str = "", preInstallSuffix: str = "",
                 buildLocation: str = PathToProject):
        self.env = env
        self.installPath = installPath
        self.cleanPreBuild = cleanPreBuild
        self.cleanBuild = cleanBuild
        self.cleanPreInstall = cleanPreInstall
        self.cleanInstall = cleanInstall
        self.makeBuild = makeBuild
        self.addDocs = addDocs
        self.addCredits = addCredits
        self.buildSuffix = buildSuffix
        self.buildLocation = buildLocation
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
        if (self.addCredits):
            self.updateCredits()

        if (self.cleanPreBuild is not None):
            self.removePrefixedFolder(PathToProject, PreBuildFolder, self.cleanPreBuild)

        if (self.cleanPreInstall is not None):
            self.removePrefixedFolder(PathToProject, PreInstallFolder, self.cleanPreInstall)

        if (self.cleanBuild is not None):
            self.removePrefixedFolder(self.buildLocation, BuildFolder, self.cleanBuild)

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

    # updateCredits(): Updates the credits boiler plate within the source files of the API
    def updateCredits(self):
        print("Updating the credits for the API's source files...")
        creditsUpdater = CreditsUpdater(APISrcFolderPaths)
        creditsUpdater.update()

    # _isLink(path): Whether 'path' is a symbolic link or a Windows directory junction
    # note: Path.is_symlink() is False for a junction before Python 3.12, and shutil.rmtree() refuses both
    #   ("Cannot call rmtree on a symbolic link"), so a build folder kept on another drive through a junction
    #   used to abort every --buildRemove run, and one kept there through a symlink was silently skipped
    @classmethod
    def _isLink(cls, path: Path) -> bool:
        if (path.is_symlink()):
            return True

        try:
            attributes = getattr(os.lstat(path), "st_file_attributes", 0)
        except OSError:
            return False

        return bool(attributes & getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0))

    # _removeFolder(folder): Deletes a build folder. A link keeps its place and loses its contents, so a build
    #   tree deliberately kept on another drive stays there
    @classmethod
    def _removeFolder(cls, folder: Path):
        if (cls._isLink(folder)):
            if (not folder.is_dir()):
                return

            print(f"Emptying the linked build folder at {folder} (the link itself is kept)")
            for child in folder.iterdir():
                if (cls._isLink(child)):
                    # a link inside is removed as a link, never followed
                    try:
                        os.unlink(child)
                    except (IsADirectoryError, PermissionError):
                        os.rmdir(child)
                elif (child.is_dir()):
                    shutil.rmtree(child)
                else:
                    child.unlink()
            return

        if (folder.is_dir()):
            shutil.rmtree(folder)
        elif (folder.exists()):
            folder.unlink()

    def removePrefixedFolder(self, srcFolder: str, folderPrefix: str, folderSuffix: str):
        if (folderSuffix == RemoveAllFolder):
            srcFolder = Path(srcFolder)

            for item in srcFolder.glob(f"{folderPrefix}*"):
                self._removeFolder(item)

        else:
            folderSuffix = folderSuffix[:-1]
            targetFolder = Path(srcFolder) / f"{folderPrefix}{folderSuffix}"
            self._removeFolder(targetFolder)

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
            self._buildFolder = os.path.join(self.buildLocation, f"{BuildFolder}{self.buildSuffix}")

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

    # _extraCMakeArgs(): the extra configure options in AGREMAP_CMAKE_ARGS, split like a POSIX shell command
    #   line on every OS -- so quote a value holding spaces, and write a path with forward slashes (CMake reads
    #   them on Windows too, and a backslash would be taken as an escape)
    @classmethod
    def _extraCMakeArgs(cls) -> List[str]:
        value = os.environ.get(CMakeArgsEnvVar, "").strip()
        if (not value):
            return []

        return shlex.split(value)

    def buildAPI(self):
        self._setupBuildFolders()
        os.chdir(APIPath)

        extraArgs = self._extraCMakeArgs()
        if (extraArgs):
            print(f"Adding CMake options from {CMakeArgsEnvVar}: {' '.join(extraArgs)}")

        subprocess.run(["cmake", "-G", "Ninja", "-B", self._buildFolder, "-DCMAKE_BUILD_TYPE=Release", f"-DCMAKE_PREFIX_PATH={self._extInstallFolders.z3}", *extraArgs], check=True)
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

