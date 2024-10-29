from ordered_set import OrderedSet
from typing import Set, Optional, Set, Union


# Import: Class to store data about importing an object
class Import():
    def __init__(self, objects: Optional[OrderedSet[str]] = None):
        if (objects is None):
            objects = OrderedSet()

        self._objects = OrderedSet()
        self.addObjs(objects)


    def _checkType(self, importObj):
        selfType = type(self)
        if (not isinstance(importObj, selfType)):
            raise TypeError(f"Object to add must be type, {selfType}, instead of type {type(importObj)}")


    def __iadd__(self, importObj):
        self._checkType(importObj)
        self.addObjs(importObj.objects, clean = False)
        return self


    def __add__(self, importObj):
        self._checkType(importObj)
        return Import(self.objects.union(importObj.objects))


    @property
    def objects(self):
        return self._objects
    

    @objects.setter
    def objects(self, newObjects: Union[Set[str], OrderedSet[str]]):
        self.addObjs(newObjects)


    def clear(self):
        self.objects.clear()


    def addObj(self, newObject: str, clean: bool = True):
        toAdd = newObject.strip() if (clean) else newObject
        self._objects.add(toAdd)


    def addObjs(self, newObjects: Union[Set[str], OrderedSet[str]], clean: bool = True):
        for ob in newObjects:
            self.addObj(ob, clean = clean)


    def toStr(self) -> str:
        objectStr = ", ".join(self._objects)
        if (not self._objects):
            objectStr = "*"

        return f"import {objectStr}"