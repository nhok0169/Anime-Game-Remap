from ordered_set import OrderedSet
from typing import List, Dict, Optional, Set, Union

from .FromImport import FromImport


# FromImportSet: A set of imports of the form: from ... import ...
class FromImportSet():
    def __init__(self, fromImports: Optional[List[FromImport]] = None):
        if (fromImports is None):
            fromImports = []

        self._fromImports: Dict[str, FromImport] = {}
        self.addMany(fromImports)


    def _checkType(self, fromImport):
        selfType = type(self)
        if (not isinstance(fromImport, selfType) and not isinstance(fromImport, FromImport)):
            raise TypeError(f"Object to add must be type {selfType} or {FromImport}, instead of type {type(fromImport)}")
        

    def __iter__(self):
        for importLoc in self._fromImports:
            yield self._fromImports[importLoc]


    def __iadd__(self, fromImport):
        self._checkType(fromImport)
        
        if (isinstance(fromImport, FromImport)):
            self.add(fromImport)
        else:
            self.addMany(list(fromImport._fromImports.values()))
        return self
    

    def __bool__(self) -> bool:
        return bool(self._fromImports)
    

    def clear(self):
        self._fromImports.clear()


    def addFromStr(self, loc: str, objects: Union[Set[str], OrderedSet[str]]):
        try:
            self._fromImports[loc]
        except KeyError:
            self._fromImports[loc] = FromImport(loc, objects = objects)
        else:
            self._fromImports[loc].addObjs(objects)


    def add(self, fromImport: FromImport):
        try:
            self._fromImports[fromImport.loc]
        except KeyError:
            self._fromImports[fromImport.loc] = fromImport
        else:
            self._fromImports[fromImport.loc] += fromImport


    def addMany(self, fromImports: List[FromImport]):
        for fromImport in fromImports:
            self.add(fromImport)


    def toStr(self):
        fromImportList = []
        for loc, fromImport in self._fromImports.items():
            fromImportList.append(fromImport.toStr())
        
        return "\n".join(fromImportList)
    

    def getObjects(self, loc: str):
        try:
            self._fromImports[loc]
        except KeyError:
            return OrderedSet()

        return self._fromImports[loc].objects


    def getAllObjects(self) -> Union[Set[str], OrderedSet[str]]:
        result = OrderedSet()
        for loc in self._fromImports:
            result = result.union(self.getObjects(loc))
        
        return result
        
