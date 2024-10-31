import re
import os
from typing import Optional

from .BaseUpdater import BaseUpdater
from ..softwareStats.SoftwareMetadata import SoftwareMetadata
from ..FileTools import FileTools


VersionReplacePattern = re.compile(r"(?<=version)\s*=.*")
NameReplacePattern = re.compile("(?<=name)\s*=\s(\"|').*(\"|')(?=\n)")


# TomlUpdater: Updates the software metadata for a .toml file
class TomlUpdater(BaseUpdater):
    def __init__(self, file: str, softwareMetadata: SoftwareMetadata):
        super().__init__(file, softwareMetadata)
        self.fileTxt = ""

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


    # update(): Updates the version on a .toml file
    def update(self):
        fullSrcPath = os.path.abspath(self.src)
        print(f"Updating .toml file at: {fullSrcPath}")

        fileTxt = self.read()
        fileTxt = re.sub(VersionReplacePattern, f' = "{self.softwareMetadata.version}"', fileTxt)
        fileTxt = re.sub(NameReplacePattern, f' = "{self.softwareMetadata.name}"', fileTxt)
        self.write(fileTxt)
