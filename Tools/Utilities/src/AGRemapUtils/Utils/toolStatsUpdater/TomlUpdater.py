import re
import os
from typing import Optional, List, Dict, Tuple

from .BaseUpdater import BaseUpdater
from ..softwareStats.SoftwareMetadata import SoftwareMetadata
from ..FileTools import FileTools


VersionReplacePattern = re.compile(r"(?<=version)\s*=.*")
NameReplacePattern = re.compile("(?<=name)\s*=\s(\"|').*(\"|')(?=\n)")

TomlDependencyPattern = re.compile("dependencies\s*=([^\]]|\n)*\]\n")
TomlProjectSectionPattern = re.compile("\[project\]((?!(\n\n))(.|\n))*")
TomlDependencyListPattern = re.compile("(?<=\[)(.|\n)*(?=\])")
TomlDependencyVersionSpecfierPattern = re.compile(r"(===|==|~=|!=|<=|>=|<|>)")


# TomlUpdater: Updates the software metadata for a .toml file
class TomlUpdater(BaseUpdater):
    def __init__(self, file: str, softwareMetadata: SoftwareMetadata, dependencies: Optional[List[Tuple[Optional[str], SoftwareMetadata]]] = None):
        super().__init__(file, softwareMetadata)
        self.fileTxt = ""
        self.dependencies = [] if (dependencies is None) else dependencies


    # read(): Reads a .toml file
    def read(self) -> str:
        self.fileTxt = FileTools.readFile(self.src, lambda filePtr: filePtr.read())
        return self.fileTxt


    # write(txt, update): Writes to the .toml file
    def write(self, txt: Optional[str] = None, update: bool = True):
        if (txt is None):
            txt = self.fileTxt

        FileTools.writeFile(self.src, lambda filePtr: filePtr.write(txt))

        if (update):
            self.fileTxt = txt

    # parseDependencies(dependencyStr): Retrieves all the dependencies for the .toml file
    def parseDependencies(self, dependencyStr: str) -> Dict[str, Optional[Tuple[str, str]]]:
        dependencies = re.search(TomlDependencyListPattern, dependencyStr)
        dependencies = dependencies.group().split(",")
        dependencies = list(map(lambda dependency: dependency.strip(), dependencies))
        dependencies = list(filter(lambda dependency: dependency != "", dependencies))

        result = {}
        for dependency in dependencies:
            versionSpecifierSearch = re.search(TomlDependencyVersionSpecfierPattern, dependency)
            if (versionSpecifierSearch is None):
                result[dependency] = None
                continue

            dependencyName = dependency[:versionSpecifierSearch.start()].strip()
            dependencyVersion = dependency[versionSpecifierSearch.end():].strip()
            result[dependencyName] = (versionSpecifierSearch.group(), dependencyVersion)

        return result

    # updateDependencies(): Updates the required dependencies
    def updateDependencies(self):
        matchResult = re.search(TomlDependencyPattern, self.fileTxt)
        dependencies = {}

        # parse the existing dependencies
        if (matchResult is not None):
            dependencies = self.parseDependencies(matchResult.group())

        # update the dependencies
        for targetDependencies in self.dependencies:
            specifier = targetDependencies[0]
            metadata = targetDependencies[1]

            if (specifier is not None and metadata.version is not None):
                dependencies[metadata.name] = (specifier, metadata.version)
            else:
                dependencies[metadata.name] = None

        # create the dependency string
        dependencyStr = []
        for dependencyName in dependencies:
            dependencyVersionSpec = dependencies[dependencyName]
            if (dependencyVersionSpec is None):
                dependencyStr.append(dependencyName)
            else:
                dependencyStr.append(f'"{dependencyName}{dependencyVersionSpec[0]}{dependencyVersionSpec[1]}"')

        dependencyStr = "\t" + "\n\t".join(dependencyStr)
        dependencyStr = f"dependencies = [\n{dependencyStr}\n]"

        # write back the dependency to the end of the 'Project' section of the .toml file
        projectMatch = re.search(TomlProjectSectionPattern, self.fileTxt)
        projectSectionEndInd = projectMatch.span()[1]
        self.fileTxt = f"{self.fileTxt[:projectSectionEndInd]}\n{dependencyStr}{self.fileTxt[projectSectionEndInd:]}"

    # update(): Updates the version on a .toml file
    def update(self):
        fullSrcPath = os.path.abspath(self.src)
        print(f"Updating .toml file at: {fullSrcPath}")

        fileTxt = self.read()
        fileTxt = re.sub(VersionReplacePattern, f' = "{self.softwareMetadata.version}"', fileTxt)
        fileTxt = re.sub(NameReplacePattern, f' = "{self.softwareMetadata.name}"', fileTxt)
        self.fileTxt = fileTxt

        if (self.dependencies):
            self.updateDependencies()

        self.write()
