import sys
import os
import shutil
import re
import copy
from ordered_set import OrderedSet

from .constants.Paths import UtilitiesPath
from .tools.ProjectPathTools import ProjectPathTools

sys.path.insert(1, UtilitiesPath)
from Utils.python.PyFile import PyFile
from Utils.python.FromImport import FromImport
from Utils.PyPathTools import PyPathTools
from Utils.ModulePathTools import ModulePathTools
from Utils.enums.ScriptPartNames import ScriptPartNames
from Utils.constants.toolStats import APIStats, APIMirrorStats
from Utils.FileTools import FileTools
from Utils.toolStatsUpdater.TomlUpdater import TomlUpdater
from Utils.constants.BoilerPlate import Credits, MirrorPreamble, APIMirrorPreambleStats
from Utils.constants.toolStats import APIMirrorBuildStats, APIMirrorBuilderBuildStats
from Utils.constants.StrReplacements import VersionReplace, RanDateTimeReplace, BuildHashReplace, RanHashReplace, BuiltDateTimeReplace


TomlDependencyPattern = re.compile("dependencies\s*=([^\]]|\n)*\]\n")
TomlProjectSectionPattern = re.compile("\[project\]((?!(\n\n))(.|\n))*")
TomlDependencyListPattern = re.compile("(?<=\[)(.|\n)*(?=\])")


class APIMirrorBuilder():
    def __init__(self, apiFolder: str, mirrorFolder: str, rootModule: str):
        self._module = ModulePathTools.dirname(rootModule)

        self._apiFolder = FileTools.parseOSPath(apiFolder)
        self._apiInitPath = PyPathTools.getInitPath(apiFolder)
        self._apiMainPath = PyPathTools.getMainPath(apiFolder)
        self._apiProjectFolder = os.path.dirname(os.path.dirname(self._apiFolder))

        self._mirrorFolder = FileTools.parseOSPath(mirrorFolder)
        self._mirrorInitPath = PyPathTools.getInitPath(mirrorFolder)
        self._mirrorMainPath = PyPathTools.getMainPath(mirrorFolder)
        self._mirrorProjectFolder = os.path.dirname(os.path.dirname(self._mirrorFolder))

        self._mirrorBuildStats = copy.deepcopy(APIMirrorBuildStats)
        self._mirrorBuilderBuildStats = copy.deepcopy(APIMirrorBuilderBuildStats)
        self._mirrorBuildStats.refresh()
        self._mirrorBuilderBuildStats.refresh()

        self.preamble = self.getPreamble()


    # getPreamble(): Retrieves the preamble text for the files generated by this builder
    def getPreamble(self) -> str:
        generationNote = MirrorPreamble.replace(VersionReplace, self._mirrorBuilderBuildStats.version)
        generationNote = generationNote.replace(RanDateTimeReplace, self._mirrorBuilderBuildStats.getFormattedDatetime())
        generationNote = generationNote.replace(RanHashReplace, self._mirrorBuilderBuildStats.buildHash)

        mirrorStats = APIMirrorPreambleStats.replace(VersionReplace, self._mirrorBuildStats.version)
        mirrorStats = mirrorStats.replace(BuiltDateTimeReplace, self._mirrorBuildStats.getFormattedDatetime())
        mirrorStats = mirrorStats.replace(BuildHashReplace, self._mirrorBuildStats.buildHash)

        result = f"{generationNote}{Credits}"[:-1]
        result += mirrorStats
        return result


    def buildMirrorInit(self):
        file = PyFile(self._apiInitPath, self._module)
        file.read()

        allObjects = file.getLocalObjects()
        fromImport = FromImport(APIStats.name, objects = allObjects)
        
        # get the __all__ text for the init file
        allModulesTxt = "__all__ = ["
        allObjects = map(lambda ob: f'"{ob}"', allObjects)
        allModulesTxt += ", ".join(allObjects)
        allModulesTxt += "]"

        print(f"Creating __init__.py")
        initTxt = f"{self.preamble}{fromImport.toStr()}\n\n{allModulesTxt}"
        mirrorInitFile = PyFile(self._mirrorInitPath, self._module)
        mirrorInitFile.write(initTxt)

    def buildMirrorMain(self):
        file = PyFile(self._apiMainPath, self._module)
        file.read()

        fromImport = FromImport(APIStats.name, objects = OrderedSet([ScriptPartNames.MainFunc.value]))

        print(f"Creating __main__.py")
        mainTxt = f"{self.preamble}{fromImport.toStr()}\n\n{file.getScriptStr()}"
        scriptMainFile = PyFile(self._mirrorMainPath, self._module)
        scriptMainFile.write(mainTxt)

    def buildMirrorConfig(self):
        apiConfig = ProjectPathTools.getPythonTomlConfigPath(self._apiProjectFolder)
        mirrorConfig = ProjectPathTools.getPythonTomlConfigPath(self._mirrorProjectFolder)

        print(f"Creating pyproject.toml")
        shutil.copy2(apiConfig, mirrorConfig)

        configUpdater = TomlUpdater(mirrorConfig, APIMirrorStats)
        configUpdater.update()

        matchResult = re.search(TomlDependencyPattern, configUpdater.fileTxt)
        dependencies = []

        # add the API Mirror's dependency to the API
        if (matchResult is not None):
            configUpdater.fileTxt = configUpdater.fileTxt[:matchResult.start()] + configUpdater.fileTxt[matchResult.end():]
            dependencies = re.search(TomlDependencyListPattern, matchResult.group())
            dependencies = dependencies.group().split(",")
            dependencies = list(map(lambda dependency: dependency.strip(), dependencies))
            dependencies = list(filter(lambda dependency: dependency != "", dependencies))

        dependencies.append(f'"{APIStats.name}=={APIStats.version}"')
        dependencies = "\t" + "\n\t".join(dependencies)
        dependencies = f"dependencies = [\n{dependencies}\n]"

        # write back the dependency to the end of the 'Project' section of the .toml file
        projectMatch = re.search(TomlProjectSectionPattern, configUpdater.fileTxt)
        projectSectionEndInd = projectMatch.span()[1]

        configUpdater.fileTxt = f"{configUpdater.fileTxt[:projectSectionEndInd]}\n{dependencies}{configUpdater.fileTxt[projectSectionEndInd:]}"
        configUpdater.write()
    
    def build(self):
        self.buildMirrorInit()
        self.buildMirrorMain()

        # copy the license and README
        apiReadMePath = ProjectPathTools.getReadMePath(self._apiProjectFolder)
        apiLicensePath = ProjectPathTools.getLicensePath(self._apiProjectFolder)
        mirrorReadMePath = ProjectPathTools.getReadMePath(self._mirrorProjectFolder)
        mirrorLicensePath = ProjectPathTools.getLicensePath(self._mirrorProjectFolder)

        print(f"Copying README")
        shutil.copy2(apiReadMePath, mirrorReadMePath)

        print(f"Copying LICENSE")
        shutil.copy2(apiLicensePath, mirrorLicensePath)

        self.buildMirrorConfig()