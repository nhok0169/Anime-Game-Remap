from .resources import BaseResource
from .tools import FileTools
from typing import Dict, Optional, List


LinkStartKeyWord = "# LINK"
LinkEndKeyWord = "# ENDLINK"
LinkStartKeyWordLen = len(LinkStartKeyWord)


# Linker: Links external resources into some file
class Linker():
    def __init__(self, resources: Optional[Dict[str, BaseResource]] = None):
        self.resources = resources
        self.fileLines = []


    # readFileLines(file): Reads the lines for the target file 
    def readFileLines(self, file: str):
        self.fileLines = FileTools.readFile(file, lambda filePtr: filePtr.readlines())


    # writeFileLines(file): Writes back the file with the linked resource
    def writeFileLines(self, file: str):
        txt = "".join(self.fileLines)
        FileTools.writeFile(file, lambda filePtr: filePtr.write(txt))

    
    # _getLinkName(linkSuffix): Retrieves the name for a resource link
    def _getLinkName(self, linkSuffix: str) -> str:
        result = linkSuffix.strip().split(" ", 1)
        if (len(result) > 1):
            return result[0]
        return ""
    

    # _getResourceLines(varName, resource): Retrieves the lines for a particular resource
    def _getResourceLines(self, varName: str, resource: BaseResource) -> List[str]:
        resourceLines = resource.get(varName)
        resourceLines = resourceLines.split("\n")
        resourceLinesLen = len(resourceLines)

        for i in range(resourceLinesLen):
            resourceLines[i] += "\n"

        return resourceLines


    # _link(): Links all the resources to the read file lines
    def _link(self):
        fileLinesLen = len(self.fileLines)
        linkName = None
        linkStartLine = -1

        i = 0
        while (i < fileLinesLen):
            fileLine = self.fileLines[i]

            # retrieve the location for a needed resource
            if (linkName is None):
                linkStartInd = fileLine.find(LinkStartKeyWord)
                
                if (linkStartInd > -1):
                    linkSuffix = fileLine[linkStartInd + LinkStartKeyWordLen:]
                    linkName = self._getLinkName(linkSuffix)
                    linkStartLine = i

            # link the corresponding resource
            elif (fileLine.find(LinkEndKeyWord) > -1):
                try:
                    resource = self.resources[linkName]
                except KeyError:
                    pass
                else:
                    resourceLines = self._getResourceLines(linkName, resource)
                    resourceLinesLen = len(resourceLines)
                    
                    self.fileLines = self.fileLines[:linkStartLine + 1] + resourceLines + self.fileLines[i:]

                    linesAdded = resourceLinesLen - (i - linkStartLine - 1)
                    i += linesAdded
                    fileLinesLen += linesAdded

                    print(f"Linked {linkName}")

                linkName = None

            i += 1


    # link(file): Links the needed resources to 'file'
    def link(self, file: str):
        print(f"Linking dependencies to {file}")
        self.readFileLines(file)
        self._link()
        self.writeFileLines(file)