from typing import Optional

from ...constants.script.KeyWordTypes import KeyWordTypes
from ..Error import Error


# InvalidKeyWordType: Exception when both the opening and closing keywords do not match for
#    a section in a source file
class InvalidKeyWordType(Error):
    def __init__(self, openingType: KeyWordTypes, closingType: KeyWordTypes, file: Optional[str] = None, lineInd: Optional[int] = None):
        location = ""
        if (file is not None):
            location = f" at '{file}'"

            if (lineInd is not None):
                location += f", line {lineInd + 1}"

        super().__init__(f"Opening Keyword of type, {openingType.name}, does not match closing keyword of type, {closingType.name}{location}")