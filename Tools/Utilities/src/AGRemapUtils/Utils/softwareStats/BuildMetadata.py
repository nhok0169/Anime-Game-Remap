import uuid
import datetime
from typing import Optional
from .SoftwareMetadata import SoftwareMetadata


# BuildMetadata: Metadata about the build/compiling of a software
class BuildMetadata():
    def __init__(self, version: Optional[str] = None):
        self.version = version
        self.refresh()

    def refresh(self):
        self.buildDateTime = datetime.datetime.now(datetime.timezone.utc)
        self.buildHash = str(uuid.uuid4())

    # fromSoftwareMetadta(softwareMetadata): Transform a software metadata
    #   into metadta used for building a softwares
    @classmethod
    def fromSoftwareMetadata(cls, softwareMetadata: SoftwareMetadata):
        return cls(version = softwareMetadata.version)
    
    def getFormattedDatetime(self) -> str:
        microseconds = int(self.buildDateTime.microsecond / 1000)
        return self.buildDateTime.strftime(f"%A, %B %d, %Y %I:%M:%S.{microseconds} %p %Z")