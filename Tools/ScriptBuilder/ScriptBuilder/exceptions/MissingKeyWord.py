import sys

from ..constants.Paths import UtilitiesPath
from ..constants.KeyWordTypes import KeyWordTypes

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.Error import Error


# MissingKeyWord: Exception when there is a missing opening keyword for a section
#   in a python module
class MissingKeyWord(Error):
    def __init__(self, keyWordType: KeyWordTypes, isStart: bool = True):
        keyWordOpeningType = "closing"
        if (isStart):
            keyWordOpeningType = "opening"

        super().__init__(f"Missing {keyWordOpeningType} keyword for type: {keyWordType.name}")