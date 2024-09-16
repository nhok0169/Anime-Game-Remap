from enum import Enum
from typing import Set


# StrEnum: Enum where values are strings
class StrEnum(Enum):
    def __str__(self) -> str:
        return self.value

    # getAll(): Retrieves the all the enum values
    @classmethod
    def getAll(cls) -> Set[str]:
        result = set()
        for strEnum in cls:
            result.add(strEnum.value)
        return result
    
    # match(name): Searches for an exact match for a particular enum value
    @classmethod
    def match(cls, name: str):
        result = None
        for strEnum in cls:
            if (strEnum.value == name):
                result = strEnum
                break
        
        return result
    
    # find(txt): Searches whether a particular enum value is contained in 'txt'
    @classmethod
    def find(cls, txt: str):
        for strEnum in cls:
            if (txt.find(strEnum.value) > -1):
                return strEnum
        return None