import sys

from ..constants.Paths import UtilitiesPath
from ..constants.KeyWordTypes import KeyWordTypes

sys.path.insert(1, UtilitiesPath)
from Utils.exceptions.Error import Error


# InvalidKeyWordType: Exception when both the opening and closing keywords do not match for
#    a section in a python module
class InvalidKeyWordType(Error):
    def __init__(self, openingType: KeyWordTypes, closingType: KeyWordTypes):
        super().__init__(f"Opening Keyword of type, {openingType.name}, does not match closing keyword of type, {closingType.name}")