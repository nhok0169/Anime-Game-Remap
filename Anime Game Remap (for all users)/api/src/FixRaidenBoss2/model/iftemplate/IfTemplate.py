##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### ExtImports
from typing import List, Union, Dict, Any, Optional, Set, Callable, Tuple
##### EndExtImports

##### LocalImports
from ...constants.IniConsts import IniKeywords
from ...constants.IfPredPartType import IfPredPartType
from ..assets.Hashes import Hashes
from ..assets.Indices import Indices
from .IfTemplatePart import IfTemplatePart
from .IfPredPart import IfPredPart
from .IfContentPart import IfContentPart
##### EndLocalImports


##### Script
# IfTemplate: Data class for the if..else template of the .ini file
class IfTemplate():
    """
    Data for storing information about a `section`_ in a .ini file

    :raw-html:`<br />`

    .. note::
        Assuming every `if/else` clause must be on its own line, we have that an :class:`IfTemplate` have a form looking similar to this:

        .. code-block:: ini
            :linenos:
            :emphasize-lines: 1,2,5,7,12,16,17

            ...(does stuff)...
            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                endif
            endif
            ...(does stuff)...
            ...(does stuff)...

        We split the above structure into parts (:class:`IfTemplatePart`) where each part is either:

        #. **An If Predicate Part (:class:`IfPredPart`)**: a single line containing the keywords "if", "else" or "endif" :raw-html:`<br />` **OR** :raw-html:`<br />`
        #. **A Content Part (:class:`IfContentPart`)**: a group of lines that *"does stuff"*

        **Note that:** an :class:`ifTemplate` does not need to contain any parts containing the keywords "if", "else" or "endif". This case covers the scenario
        when the user does not use if..else statements for a particular `section`_
        
        Based on the above assumptions, we can assume that every ``[section]`` in a .ini file contains this :class:`IfTemplate`

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: for element in x

            Iterates over all the parts of the :class:`IfTemplate`, ``x``

        .. describe:: x[num]

            Retrieves the part from the :class:`IfTemplate`, ``x``, at index ``num``

        .. describe:: x[num] = newPart

            Sets the part at index ``num`` of the :class:`IfTemplate`, ``x``, to have the value of ``newPart``

    :raw-html:`<br />`

    Parameters
    ----------
    parts: List[:class:`IfTemplatePart`]
        The individual parts of how we divided an :class:`IfTemplate` described above

    name: :class:`str`
        The name of the `section`_ for this :class:`IfTemplate`

        **Default**: ``""``

    Attributes
    ----------
    parts: List[:class:`IfTemplatePart`]
        The individual parts of how we divided an :class:`IfTemplate` described above

    calledSubCommands: Dict[:class:`int`, List[:class:`str`]]
        Any other sections that this :class:`IfTemplate` references :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the indices to the :class:`IfContentPart` in the :class:`IfTemplate` that the section is called
        * The values are the referenced sections within the :class:`IfContentPart`

    hashes: Set[:class:`str`]
        The hashes this :class:`IfTemplate` references

    indices: Set[:class:`str`]
        The indices this :class:`IfTemplate` references
    """

    def __init__(self, parts: List[IfTemplatePart], name: str = ""):
        self.name = name
        self.parts = parts

        self.calledSubCommands = {}
        self.hashes = set()
        self.indices = set()

        self.find(pred = self._hasNeededAtts, postProcessor = self._setupIfTemplateAtts)

    def _hasNeededAtts(self, ifTemplate, partIndex: int, part: IfTemplatePart) -> bool:
        return isinstance(part, IfContentPart) and (IniKeywords.Run.value in part or IniKeywords.Hash.value in part or IniKeywords.MatchFirstIndex.value in part)
    
    def _setupIfTemplateAtts(self, ifTemplate, partIndex: int, part: IfContentPart):
        if (IniKeywords.Run.value in part):
            ifTemplate.calledSubCommands[partIndex] = part[IniKeywords.Run.value]
        
        if (IniKeywords.Hash.value in part):
            ifTemplate.hashes.update(set(map(lambda valData: valData[1], part[IniKeywords.Hash.value])))

        if (IniKeywords.MatchFirstIndex.value in part):
            ifTemplate.indices.update(set(map(lambda valData: valData[1], part[IniKeywords.MatchFirstIndex.value])))

    @classmethod
    def build(cls, rawParts: List[Union[str, Dict[str, List[Tuple[int, str]]]]], name: str = ""):
        parts = []
        rawPartsLen = len(rawParts)
        depth = 0

        for i in range(rawPartsLen):
            rawPart = rawParts[i]
            part = None

            if (isinstance(rawPart, str)):
                predType = IfPredPartType.getType(rawPart)
                if (predType is None):
                    continue
                elif (predType == IfPredPartType.If):
                    depth += 1
                elif (predType == IfPredPartType.EndIf):
                    depth -= 1

                part = IfPredPart(rawPart, predType)

            elif (isinstance(rawPart, dict)):
                part = IfContentPart(rawPart, depth)

            if (part is not None):
                parts.append(part)

        return cls(parts, name = name)


    def __iter__(self):
        return self.parts.__iter__()
    
    def __getitem__(self, key: int) -> Union[str, Dict[str, Any]]:
        return self.parts[key]
    
    def __setitem__(self, key: int, value: Union[str, Dict[str, Any]]):
        self.parts[key] = value

    def add(self, part: Union[str, Dict[str, Any]]):
        """
        Adds a part to the :class:`ifTemplate`

        Parameters
        ----------
        part: Union[:class:`str`, Dict[:class:`str`, Any]]
            The part to add to the :class:`IfTemplate`
        """
        self.parts.append(part)

    # find(pred, postProcessor): Searches each part in the if template based on 'pred'
    def find(self, pred: Optional[Callable[[int, IfTemplatePart], bool]] = None, postProcessor: Optional[Callable[[int, IfTemplatePart], Any]] = None) -> Dict[int, Any]:
        """
        Searches the :class:`IfTemplate` for parts that meet a certain condition

        Parameters
        ----------
        pred: Optional[Callable[[:class:`IfTemplate`, :class:`int`, :class:`IfTemplatePart`], :class:`bool`]]
            The predicate used to filter the parts :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then this function will return all the parts :raw-html:`<br />` :raw-html:`<br />`

            The order of arguments passed into the predicate will be:

            #. The :class:`IfTemplate` that this method is calling from
            #. The index for the part in the :class:`IfTemplate`
            #. The current part of the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        postProcessor: Optional[Callable[[:class:`IfTemplate`, :class:`int`, :class:`IfTemplatePart`], Any]]
            A function that performs any post-processing on the found part that meets the required condition :raw-html:`<br />` :raw-html:`<br />`

            The order of arguments passed into the post-processor will be:

            #. The :class:`IfTemplate` that this method is calling from
            #. The index for the part in the :class:`IfTemplate`
            #. The current part of the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``

        Returns
        -------
        Dict[:class:`int`, Any]
            The filtered parts that meet the search condition :raw-html:`<br />` :raw-html:`<br />`

            The keys are the index locations of the parts and the values are the found parts
        """

        result = {}
        if (pred is None):
            pred = lambda ifTemplate, partInd, part: True

        if (postProcessor is None):
            postProcessor = lambda ifTemplate, partInd, part: part

        partsLen = len(self.parts)
        for i in range(partsLen):
            part = self.parts[i]
            if (pred(self, i, part)):
                result[i] = (postProcessor(self, i, part))

        return result
    
    def getMods(self, hashRepo: Hashes, indexRepo: Indices, version: Optional[float] = None) -> Set[str]:
        """
        Retrieves the corresponding mods the :class:`IfTemplate` will fix to

        Parameters
        ----------
        hashRepo: :class:`Hashes`
            The data source for the hashes

        indexRepo: :class:`Indices`
            The data source for the indices

        version: Optional[:class:`float`]
            What version we want to fix :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, will assume we want to fix to the latest version :raw-html:`<br />` :raw-html:`<br />`
            
             **Default**: ``None``

        Returns
        -------
        Set[:class:`str`]
            Names of all the types of mods the :class:`IfTemplate` will fix to
        """

        result = set()

        for hash in self.hashes:
            replacments = hashRepo.replace(hash, version = version)
            result = result.union(set(replacments.keys()))

        for index in self.indices:
            replacments = indexRepo.replace(index, version = version)
            result = result.union(set(replacments.keys()))

        return result
##### EndScript