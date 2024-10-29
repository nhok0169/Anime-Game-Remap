from typing import Optional


# SoftwareMetadata: Class to keep track of certain metadata about tools and software
class SoftwareMetadata():
    def __init__(self, version: Optional[int] = None):
        self.version = version