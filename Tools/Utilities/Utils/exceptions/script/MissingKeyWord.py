from ...constants.script.KeyWordTypes import KeyWordTypes
from ..Error import Error


# MissingKeyWord: Exception when there is a missing opening keyword for a section
#   in a python module
class MissingKeyWord(Error):
    def __init__(self, keyWordType: KeyWordTypes, isStart: bool = True):
        keyWordOpeningType = "closing"
        if (isStart):
            keyWordOpeningType = "opening"

        super().__init__(f"Missing {keyWordOpeningType} keyword for type: {keyWordType.name}")