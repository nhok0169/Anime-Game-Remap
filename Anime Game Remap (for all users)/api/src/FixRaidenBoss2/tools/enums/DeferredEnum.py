##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
from enum import Enum
from typing import Optional, List, Dict, Any, Callable
##### EndExtImports


##### Script
class DeferredEnum(Enum):
    """
    This class inherits from: `Enum`_

    An enumeration that defers its initialization only once its :attr:`value` is called :raw-html:`<br />` :raw-html:`<br />`

    .. important::
        Please initialize the enum such that the value is a tuple

        eg. :raw-html:`<br />`

        .. code-block:: python
            :caption: exampleEnum.py
            :linenos:

            import AnimeGameRemap as AGR

            class Directions(AGR.DeferredEnum):
                Up = (lambda: "0: up", )
                Down = (lambda id, name: f"{id}: {name}", [1, "down"])
                Left = (lambda id, name = "unknown direction": f"{id}: {name}", [2], {"name": "left"})

    Parameters
    ----------
    func: Callable[[...], Any]
        The function to generate the desired value

    funcArgs: Optional[List[Any]]
        The arguments to supply into the generation function :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    funcKwargs: Optional[Dict[:class:`str`, Any]]
        The keyword arguments to supply into the generation function :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    __id__ = 0
    
    def __new__(cls, func: Callable[[Any], Any], funcArgs: Optional[List[Any]] = None, funcKwargs: Optional[Dict[str, Any]] = None):
        obj = object.__new__(cls)
        obj._value_ = cls.__id__
        obj._generate = func
        obj._generateArgs = [] if (funcArgs is None) else funcArgs
        obj._generateKwargs = {} if (funcKwargs is None) else funcKwargs
        obj._initialized = False
        cls.__id__ += 1
        return obj
    
    @property
    def value(self) -> Any:
        if (self._initialized):
            return self._value_
        
        self._value_ = self._generate(*self._generateArgs, **self._generateKwargs)
        self._initialized = True

        del self._generate
        del self._generateArgs
        del self._generateKwargs

        return self._value_
##### EndScript