from typing import Optional, List

from .SoftwareContributor import SoftwareContributor


# SoftwareMetadata: Class to keep track of certain metadata about tools and software
class SoftwareMetadata():
    def __init__(self, name: str = "", title: str = "", shortTitle: str = "", version: Optional[str] = None,
                 authors: Optional[List[SoftwareContributor]] = None):
        self.title = title
        self.shortTitle = shortTitle
        self.name = name
        self.version = version

        if (authors is None):
            authors = []
        self.authors = authors


    def getOldDiscNames(self) -> str:
        result = []
        for author in self.authors:
            discName = author.name
            if (author.oldDiscName is not None):
                discName = author.oldDiscName
            elif (author.discName is not None):
                discName = author.discName
            
            result.append(discName)

        return ", ".join(result)