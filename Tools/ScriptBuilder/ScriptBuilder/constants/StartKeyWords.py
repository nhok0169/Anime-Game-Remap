import sys
from typing import Optional

from ..constants.Paths import UtilitiesPath
from .KeyWordTypes import KeyWordTypes

sys.path.insert(1, UtilitiesPath)
from Utils.enums.StrEnum import StrEnum


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