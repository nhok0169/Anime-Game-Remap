from typing import Optional

from .KeyWordTypes import KeyWordTypes
from ...enums.StrEnum import StrEnum


# StartKeyWords: enum for the starting keywords to transform python modules to a script
class StartKeyWords(StrEnum):
    ExtImports = "# ExtImports"
    LocalImports = "# LocalImports"
    Script = "# Script"
    Credits = "# Credits"

    @classmethod
    def getType(cls, line: str) -> Optional[KeyWordTypes]:
        keyWord = cls.find(line)
        if (keyWord is None):
            return None
        
        if (keyWord == cls.ExtImports):
            return KeyWordTypes.ExtImports
        elif (keyWord == cls.LocalImports):
            return KeyWordTypes.LocalImports
        elif (keyWord == cls.Script):
            return KeyWordTypes.Script
        elif (keyWord == cls.Credits):
            return KeyWordTypes.Credits
        

# EndKeyWords: enum for the ending keywords to transform python modules to a script
class EndKeyWords(StrEnum):
    ExtImports = "# EndExtImports"
    LocalImports = "# EndLocalImports"
    Script = "# EndScript"
    Credits = "# EndCredits"

    @classmethod
    def getType(cls, line: str) -> Optional[KeyWordTypes]:
        keyWord = cls.find(line)
        if (keyWord is None):
            return None
        
        if (keyWord == cls.ExtImports):
            return KeyWordTypes.ExtImports
        elif (keyWord == cls.LocalImports):
            return KeyWordTypes.LocalImports
        elif (keyWord == cls.Script):
            return KeyWordTypes.Script
        elif (keyWord == cls.Credits):
            return KeyWordTypes.Credits


# KeyWordPrefix: The extra prefix added onto the keywords, as they are actually written within
#   the source files of this project
#
# note: the keywords themselves are only ever matched as a substring of a line, such that the same
#   keywords also work for the source files of the languages that write their single line comments
#   with a different prefix (eg. '// ##### Credits' for C++)
KeyWordPrefix = "####"

CreditsStartKeyWord = f"{KeyWordPrefix}{StartKeyWords.Credits.value}"
CreditsEndKeyWord = f"{KeyWordPrefix}{EndKeyWords.Credits.value}"
