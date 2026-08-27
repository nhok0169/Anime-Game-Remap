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
from typing import List, TYPE_CHECKING, Tuple, Optional, Callable
##### EndExtImports

##### CppLocalImports
from .....core import Ranges
from .....core import IniGraphGroup
from .....core import RegAdd
##### EndCppLocalImports

##### LocalImports
from .BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from .....core import SectionIterData

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class GraphInherit(BaseIniGraphGroupEdit):
    """
    This class inherits from :class:`BaseIniGraphGroupEdit`

    Merges the graph at 'dst' into the graph at 'src', by inserting consecutive `KVPs`_ into 'src' that reference
    every root `section`_ of the graph at 'dst' :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        This only inserts the reference `KVPs`_ into 'src' -- the `sections`_ of 'dst' themselves are left
        untouched (and still need to be reachable/present elsewhere for the reference to resolve, the same way
        a plain ``run =`` reference to another `section`_ works)

    .. note::
        If either the graph at 'src' or the graph at 'dst' cannot be found, nothing is inserted and the original
        ``graphGroups`` is returned as-is -- no exception is raised

    Parameters
    ----------
    src: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The id of the source :class:`IniSectionGraph` to insert the reference `KVPs`_ into. The tuple contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    dst: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The id of the :class:`IniSectionGraph` to merge into 'src'. Same tuple format as 'src'

    reg: :class:`str`
        The name of the register used to reference the root `sections`_ of the graph at 'dst'

    latest: :class:`bool`
        Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the front :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    partFilter: Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]
        The filter used to indicate which areas of some :class:`IfContentPart` within the graph at 'src' are valid
        to insert the `KVPs`_ :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then the `KVPs`_ are instead inserted directly at the very front/back (based on
        'latest') of every root `section`_ of the graph at 'src', instead of being filtered through every
        :class:`IfContentPart` of the graph :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    src: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The id of the source :class:`IniSectionGraph` to insert the reference `KVPs`_ into

    dst: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The id of the :class:`IniSectionGraph` to merge into 'src'

    reg: :class:`str`
        The name of the register used to reference the root `sections`_ of the graph at 'dst'

    latest: :class:`bool`
        Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the front

    partFilter: Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]
        The filter used to indicate which areas of some :class:`IfContentPart` within the graph at 'src' are valid
        to insert the `KVPs`_
    """

    def __init__(self, src: Tuple[int, str, str], dst: Tuple[int, str, str], reg: str, latest: bool = True, partFilter: Optional[Callable[[SectionIterData, "ModType", Optional["IniFile"]], Ranges]] = None):
        self.src = src
        self.dst = dst
        self.reg = reg
        self.latest = latest
        self.partFilter = partFilter

    def edit(self, graphGroups: List[IniGraphGroup], modType: "ModType", modName: str = "") -> List[IniGraphGroup]:
        srcGraph = self.getGraph(graphGroups, self.src, errorOnNotFound = False)
        dstGraph = self.getGraph(graphGroups, self.dst, errorOnNotFound = False)

        if (srcGraph is None or dstGraph is None):
            return graphGroups

        kvps = [(self.reg, rootName) for rootName in dstGraph.roots]
        if (not kvps):
            return graphGroups

        # No filter -- insert straight to the very front/back of every root section of 'src'
        if (self.partFilter is None):
            for section in srcGraph.getRootSections():
                if (self.latest):
                    section.addKVPsToBack(kvps)
                else:
                    section.addKVPsToFront(kvps)

                section.rebuild()

            return graphGroups

        # Filter given -- insert at the earliest/latest valid index of every IfContentPart the filter accepts
        regAdd = RegAdd(kvps, latest = self.latest)
        touchedSections = set()

        for iterData in srcGraph.iterByContentPart():
            partRanges = self.partFilter(iterData, modType, None)
            if (partRanges.isEmpty()):
                continue

            regAdd.edit(iterData.part, iterData.sectionName, modType, modName = modName, partRanges = partRanges)
            touchedSections.add(iterData.section)

        for section in touchedSections:
            section.rebuild()

        return graphGroups
##### EndScript