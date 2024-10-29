import sys
import uuid
import datetime
from typing import Optional

from ..constants.Paths import UtilitiesPath

sys.path.insert(1, UtilitiesPath)
from Utils.SoftwareMetadata import SoftwareMetadata


# BuildMetadata: Metadata about the build/compiling of a software
class BuildMetadata():
    def __init__(self, version: Optional[int] = None):
        self.version = version
        self.buildDateTime = datetime.datetime.now(datetime.timezone.utc)
        self.buildHash = str(uuid.uuid4())

    # fromSoftwareMetadta(softwareMetadata): Transform a software metadata
    #   into metadta used for building a softwares
    @classmethod
    def fromSoftwareMetadata(cls, softwareMetadata: SoftwareMetadata):
        return cls(version = softwareMetadata.version)