import sys
import re
import os

from ..constants.Paths import UtilitiesPath
from .BaseUpdater import BaseUpdater

sys.path.insert(1, UtilitiesPath)
from Utils.SoftwareMetadata import SoftwareMetadata
from Utils.FileTools import FileTools


VersionReplacePattern = re.compile(r"(?<=version)\s*=.*")


# TomlUpdater: Updates the software metadata for a .toml file
class TomlUpdater(BaseUpdater):
    def __init__(self, file: str, softwareMetadata: SoftwareMetadata):
        super().__init__(file, softwareMetadata)

    # update(): Updates the version on a .toml file
    def update(self):
        fullSrcPath = os.path.abspath(self.src)
        print(f"Updating .toml file at: {fullSrcPath}")

        fileTxt = FileTools.readFile(self.src, lambda filePtr: filePtr.read())
        fileTxt = re.sub(VersionReplacePattern, f' = "{self.softwareMetadata.version}"', fileTxt)
        FileTools.writeFile(self.src, lambda filePtr: filePtr.write(fileTxt))
