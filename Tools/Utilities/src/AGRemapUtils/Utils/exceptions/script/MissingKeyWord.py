from typing import Optional

from ...constants.script.KeyWordTypes import KeyWordTypes
from ..Error import Error


# MissingKeyWord: Exception when there is a missing opening keyword for a section
#   in a source file
#
# note: the keywords are matched as a SUBSTRING of a line, so an ordinary comment that happens to
#   start with '# Script' opens a section that never closes -- and this repo's own
#   '# Name: description' comment convention makes that very easy to write by accident. That is why
#   the file and the line are worth naming here: without them, this error says only the type.
class MissingKeyWord(Error):
    def __init__(self, keyWordType: KeyWordTypes, isStart: bool = True, file: Optional[str] = None, lineInd: Optional[int] = None):
        keyWordOpeningType = "closing"
        if (isStart):
            keyWordOpeningType = "opening"

        location = ""
        if (file is not None):
            location = f" at '{file}'"

            if (lineInd is not None):
                location += f", line {lineInd + 1}"

        super().__init__(f"Missing {keyWordOpeningType} keyword for type: {keyWordType.name}{location}")