import re
import os
from typing import Optional, List, Dict, Tuple

from .BaseUpdater import BaseUpdater
from ..softwareStats.SoftwareMetadata import SoftwareMetadata
from ..toml.TomlFile import TomlFile


ProjectSection = "project"
NameKey = "name"
VersionKey = "version"
DependenciesKey = "dependencies"

TomlDependencyVersionSpecfierPattern = re.compile(r"(===|==|~=|!=|<=|>=|<|>)")


# TomlUpdater: Updates the software metadata for a .toml file
#
# note: only the keys within the [project] section are touched, and each one is replaced in place.
#   Scoping matters -- the keys are not unique across a whole .toml file, and the API's own one has a
#   'cmake.version' under [tool.scikit-build] that a file-wide search happily overwrites with the
#   software's version.
class TomlUpdater(BaseUpdater):
    def __init__(self, file: str, softwareMetadata: SoftwareMetadata, dependencies: Optional[List[Tuple[Optional[str], SoftwareMetadata]]] = None):
        super().__init__(file, softwareMetadata)
        self.toml = TomlFile(file)
        self.dependencies = [] if (dependencies is None) else dependencies


    # read(): Reads a .toml file
    def read(self) -> str:
        return self.toml.read()


    # write(txt): Writes to the .toml file
    def write(self, txt: Optional[str] = None):
        self.toml.write(txt)


    # parseDependencies(dependencyLines): Retrieves all the dependencies for the .toml file
    def parseDependencies(self, dependencyLines: List[str]) -> Dict[str, Optional[Tuple[str, str]]]:
        dependencyStr = "\n".join(dependencyLines)
        dependencyStr = dependencyStr[dependencyStr.find("[") + 1: dependencyStr.rfind("]")]

        dependencies = map(lambda dependency: dependency.strip().strip("\"'"), dependencyStr.split(","))
        dependencies = filter(lambda dependency: dependency != "", dependencies)

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


    # getDependencyLines(dependencies): Retrieves the lines for the 'dependencies' assignment
    def getDependencyLines(self, dependencies: Dict[str, Optional[Tuple[str, str]]]) -> List[str]:
        dependencyStrs = []
        for dependencyName in dependencies:
            dependencyVersionSpec = dependencies[dependencyName]
            if (dependencyVersionSpec is None):
                dependencyStrs.append(f'"{dependencyName}"')
            else:
                dependencyStrs.append(f'"{dependencyName}{dependencyVersionSpec[0]}{dependencyVersionSpec[1]}"')

        result = [f"{DependenciesKey} = ["]
        lastInd = len(dependencyStrs) - 1

        for i in range(len(dependencyStrs)):
            separator = "" if (i == lastInd) else ","
            result.append(f"\t{dependencyStrs[i]}{separator}")

        result.append("]")
        return result


    # updateDependencies(): Updates the required dependencies
    def updateDependencies(self):
        dependencyLines = self.toml.getKey(ProjectSection, DependenciesKey)
        dependencies = {} if (dependencyLines is None) else self.parseDependencies(dependencyLines)

        for targetDependencies in self.dependencies:
            specifier = targetDependencies[0]
            metadata = targetDependencies[1]

            if (specifier is not None and metadata.version is not None):
                dependencies[metadata.name] = (specifier, metadata.version)
            else:
                dependencies[metadata.name] = None

        self.toml.setKey(ProjectSection, DependenciesKey, self.getDependencyLines(dependencies))


    # update(): Updates the software metadata on a .toml file
    def update(self):
        fullSrcPath = os.path.abspath(self.src)
        print(f"Updating .toml file at: {fullSrcPath}")

        self.read()

        if (self.softwareMetadata.name):
            self.toml.setKey(ProjectSection, NameKey, [f'{NameKey} = "{self.softwareMetadata.name}"'])

        if (self.softwareMetadata.version is not None):
            self.toml.setKey(ProjectSection, VersionKey, [f'{VersionKey} = "{self.softwareMetadata.version}"'])

        if (self.dependencies):
            self.updateDependencies()

        self.write()
