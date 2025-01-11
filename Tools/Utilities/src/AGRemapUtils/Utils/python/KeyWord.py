from ..constants.script.KeyWordTypes import KeyWordTypes


# KeyWord: Class for the keyword of a section in a python module
class KeyWord():
    def __init__(self, type: KeyWordTypes, lineInd: int, isStart: bool = True):
        self.type = type
        self.isStart = isStart
        self.lineInd = lineInd