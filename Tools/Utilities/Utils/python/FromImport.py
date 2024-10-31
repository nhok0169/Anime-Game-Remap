from ordered_set import OrderedSet
from typing import Set, Optional
from .Import import Import


# FromImport: Class to store data about importing an object from some location
class FromImport(Import):
    def __init__(self, loc: str, objects: Optional[OrderedSet[str]] = None):
        super().__init__(objects = objects)
        self.loc = loc


    @property
    def loc(self):
        return self._loc
    
    @loc.setter
    def loc(self, newLoc: str):
        self._loc = newLoc.strip()

    
    def toStr(self) -> str:
        importStr = super().toStr()
        return f"from {self._loc} {importStr}"