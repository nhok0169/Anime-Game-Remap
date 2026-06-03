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
from typing import Union
##### EndExtImports

##### LocalImports
from ..constants.GenericTypes import SympBooleanType
from .iftemplate.IfContentPart import IfContentPart
from .iftemplate.IfTemplate import IfTemplate
from typing import List, Optional, Dict, Tuple
##### EndLocalImports


##### Script
class SectionIterQueryData():
    """
    A class that contains the needed data for each iteration after calling :meth:`IniSectionGraph.iterByQuery`

    Parameters
    ----------
    part: :class:`IfContentPart`
        The part retrieved

    query: Union[:class:`bool`, `sympy.Boolean`_]
        The corresponding logical query that the part resides in

    sectionName: :class:`str`
        The name of the `section`_ the part resides in

    section: :class:`IfTemplate`
        The corresponding `section`_ the part resides in

    rootSectionName: :class:`str`
        The name of the root `section`_ the part resides in

    rootSection: :class:`IfTemplate`
        The corresponding root `section`_ the part resides in

    state: :class:`int`
        The current state the `section`_ is in

    kvps: Optional[Dict[:class:`str`, List[Tuple[:class:`int`, :class:`str`]]]]
        The current state of a subset of the `KVPs`_ for the part :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the keys to track that have been found in the part
        * The values are the different instances where the particular key is found and each tuple contains:

            #. The index where that `KVP`_ is located
            #. The value of the `KVP`_

    Arguments
    ---------
    part: :class:`IfContentPart`
        The part retrieved

    query: Union[:class:`bool`, `sympy.Boolean`_]
        The corresponding logical query that the part resides in

    sectionName: :class:`str`
        The name of the `section`_ the part resides in

    section: :class:`IfTemplate`
        The corresponding `section`_ the part resides in

    rootSectionName: :class:`str`
        The name of the root `section`_ the part resides in

    rootSection: :class:`IfTemplate`
        The corresponding root `section`_ the part resides in

    state: :class:`int`
        The current state the `section`_ is in

    kvps: Dict[:class:`str`, List[Tuple[:class:`int`, :class:`str`]]]
        The current state of a subset of the `KVPs`_ for the part :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the keys to track that have been found in the part
        * The values are the different instances where the particular key is found and each tuple contains:

            #. The index where that `KVP`_ is located
            #. The value of the `KVP`_
    """

    def __init__(self, part: IfContentPart, query: Union[bool, SympBooleanType], sectionName: str, section: IfTemplate, rootSectionName: str, rootSection: IfTemplate, state: int, kvps: Optional[Dict[str, List[Tuple[int, str]]]] = None):
        self.part = part
        self.query = query
        self.sectionName = sectionName
        self.section = section
        self.rootSectionName = rootSectionName
        self.rootSection = rootSection
        self.state = state
        self.kvps = kvps if (kvps is not None) else {}
##### EndScript