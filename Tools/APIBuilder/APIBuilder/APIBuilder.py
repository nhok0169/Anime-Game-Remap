import shutil
import subprocess
import os
import sys
from pathlib import Path
from typing import List

from .constants.Paths import APIPyFolderPath, APICyBuildFolderPath, APIPyBindBuildFolderPath, APICoreBuildFolderPath, APITopBuildFolderPath, APIPath, BuildFolder, APICoreXMLFolderPath, APICoreFolderPath
from .constants.BuildEnv import BuildEnv


class APIBuilder():
    _PackageName = "FixRaidenBoss2"

    def __init__(self, env: BuildEnv = BuildEnv.Dev, installPath: str = APIPyFolderPath, cleanBuild: bool = True, cleanInstall: bool = True, makeBuild: bool = True, addDocs: bool = False):
        self.env = env
        self.installPath = installPath
        self.cleanBuild = cleanBuild
        self.cleanInstall = cleanInstall
        self.makeBuild = makeBuild
        self.addDocs = addDocs

    def __call__(self):
        self.run()

    def run(self):
        if (self.cleanBuild):
            shutil.rmtree(APITopBuildFolderPath, ignore_errors=True)
            shutil.rmtree(APICyBuildFolderPath, ignore_errors=True)
            shutil.rmtree(APIPyBindBuildFolderPath, ignore_errors=True)
            shutil.rmtree(APICoreBuildFolderPath, ignore_errors=True)

        if (self.cleanInstall):
            self.cleanInstalls()

        if (self.makeBuild):
            self.buildAPI()

        if (self.addDocs):
            self.buildDocs()

    def cleanInstalls(self):
        basePath = Path(APIPath).resolve()
        excludeSubDirs = [APITopBuildFolderPath, APICyBuildFolderPath, APIPyBindBuildFolderPath, APICoreBuildFolderPath]
        excludePaths = [Path(subdir).resolve() for subdir in excludeSubDirs]
        
        targetExtensions = {'.so', '.pyd'}

        for filePath in basePath.rglob('*'):
            if filePath.is_file() and filePath.suffix in targetExtensions:

                isExcluded = any(
                    exPath in filePath.parents or filePath.parent == exPath 
                    for exPath in excludePaths
                    )
                
                if isExcluded:
                    continue
                    
                filePath.unlink()

    def buildAPI(self):
        os.chdir(APIPath)

        cmakeBuildType = "Release"
        subprocess.run(["cmake", "-B", BuildFolder, "-DCMAKE_BUILD_TYPE=Release"], check=True)
        subprocess.run(["cmake", "--build", BuildFolder, "--config", cmakeBuildType], check=True)
        subprocess.run(["cmake", "--install", BuildFolder,  "--prefix", f'{self.installPath}', "--config", cmakeBuildType], check=True)

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

