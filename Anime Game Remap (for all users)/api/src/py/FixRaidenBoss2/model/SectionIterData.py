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

##### CppLocalImports
from ..core import IfContentPartColouring
##### EndCppLocalImports

##### LocalImports
from ..constants.GenericTypes import SympBooleanType
from .iftemplate.IfContentPart import IfContentPart
from .iftemplate.IfTemplate import IfTemplate
from typing import Optional
##### EndLocalImports


##### Script
class SectionIterData():
    """
    A class that contains the needed data for each iteration after calling :meth:`IniSectionGraph.iterSectsByContentPart`

    Parameters
    ----------
    sectionName: :class:`str`
        The name of the `section`_

    section: :class:`IfTemplate`
        The corresponding `section`_ the part resides in

    part: :class:`IfContentPart`
        The corresponding part

    state: :class:`int`
        The current state of the `section`_

    colouring: Optional[:class:`IfContentPartColouring`]
        The current `KVP`_ states of the :class:`IfContentPart`

    Attributes
    ----------
    sectionName: :class:`str`
        The name of the `section`_

    section: :class:`IfTemplate`
        The corresponding `section`_ the part resides in

    part: :class:`IfContentPart`
        The corresponding part

    state: :class:`int`
        The current state of the `section`_

    colouring: Optional[:class:`IfContentPartColouring`]
        The current `KVP`_ states of the :class:`IfContentPart`
    """

    def __init__(self, sectionName: str, section: IfTemplate, part: IfContentPart, state: int, colouring: Optional[IfContentPartColouring] = None):
        self.sectionName = sectionName
        self.section = section
        self.part = part
        self.state = state
        self.colouring = colouring


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

    colouring: Optional[:class:`IfContentPartColouring`]
        The current `KVP`_ states of the :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
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

    colouring: Optional[:class:`IfContentPartColouring`]
        The current `KVP`_ states of the :class:`IfContentPart`
    """

    def __init__(self, part: IfContentPart, query: Union[bool, SympBooleanType], sectionName: str, section: IfTemplate, rootSectionName: str, 
                 rootSection: IfTemplate, state: int, colouring: Optional[IfContentPartColouring] = None):
        self.part = part
        self.query = query
        self.sectionName = sectionName
        self.section = section
        self.rootSectionName = rootSectionName
        self.rootSection = rootSection
        self.state = state
        self.colouring = colouring
##### EndScript