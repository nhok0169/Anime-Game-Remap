from typing import Optional


class SoftwareContributor():
    def __init__(self, name: str, discName: Optional[str] = None, oldDisName: Optional[str] = None):
        self.name = name
        self.discName = discName
        self.oldDiscName = oldDisName