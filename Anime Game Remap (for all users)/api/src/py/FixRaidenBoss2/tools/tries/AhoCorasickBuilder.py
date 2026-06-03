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
from typing import Dict, Optional, Optional, List, Any, Type
##### EndExtImports

##### LocalImports
from .BaseAhoCorasickDFA import BaseAhoCorasickDFA
from .AhoCorasicDFA import AhoCorasickDFA
from .FastAhoCorasickDFA import FastAhoCorasickDFA
from ..Builder import Builder
##### EndLocalImports


##### Script
class AhoCorasickBuilder(Builder[BaseAhoCorasickDFA]):
    """
    This class inherits from :class:`Builder`

    A class to build some implementation of the `Aho-Corasick`_ algorithm

    Parameters
    ----------
    buildCls: Optional[Type[:class:`BaseAhoCorasickDFA`]]
        The class to construct a :class:`BaseAhoCorasickDFA`  :raw-html:`<br />` :raw-html:`<br />`

        If this parameters is ``None``, the class will be a :class:`FastAhoCorasickDFA` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    args: Optional[List[Any]]
        The constant arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    kwargs: Optional[Dict[str, Any]]
        The constant keyword arguments used to build the object :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, buildCls: Optional[Type[BaseAhoCorasickDFA]] = None, args: Optional[List[Any]] = None, kwargs: Optional[Dict[str, Any]] = None):
        if (buildCls is None):
            buildCls = FastAhoCorasickDFA

        super().__init__(buildCls, args, kwargs)

    
    def build(self, *args, **kwargs):
        """
        Builds the `DFA`_

        .. warning::
            If failed to construct the `DFA`_ for the class given, will fallback to constructing a :class:`AhoCorasickDFA`

        Parameters
        ----------
        *args
            arguments to build the object

        **kwargs
            keyword arguments to build the object

        Returns
        -------
        :class:`BaseAhoCorasickDFA`
            The built `DFA`_
        """

        try:
            return super().build(*args, **kwargs)
        except ModuleNotFoundError as e:
            return AhoCorasickDFA(*args, *self._args, **kwargs, **self._kwargs)
##### EndScript