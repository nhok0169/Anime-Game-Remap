from enum import Enum


# KeywordsType: enum for the keywords used to transform a python module to a script
class KeyWordTypes(Enum):
    ExtImports = 0
    LocalImports = 1
    Script = 2
    Credits = 3