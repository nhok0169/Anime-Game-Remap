##### ExtImports
from typing import Union, Optional, Callable, List, Set
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import Pattern
from ..assets.Hashes import Hashes
from ..assets.Indices import Indices
from ..assets.VGRemaps import VGRemaps
from ..VGRemap import VGRemap
from ...tools.ListTools import ListTools
##### EndLocalImports


##### Script
class ModType():
    """
    Class for defining a generic type of mod

    Parameters
    ----------
    name: :class:`str`
        The default name for the type of mod

    check: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
        The specific check used to identify the .ini file belongs to the specific type of mod when checking arbitrary line in a .ini file :raw-html:`<br />` :raw-html:`<br />`

        #. If this argument is a string, then will check if a line in the .ini file equals to this argument
        #. If this argument is a regex pattern, then will check if a line in the .ini file matches this regex pattern
        #. If this argument is a function, then will check if a line in the .ini file will make the function for this argument return `True`

    hashes: Optional[:class:`Hashes`]
        The hashes related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will create a new, empty :class:`Hashes` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
        

    indices Optional[:class:`Indices`]
        The indices related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

        If this ``None``, then will create a new , emtpy :class:`Indices` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    vgRemaps: Optional[:class:`VGRemaps`]
        Maps the blend indices from the vertex group of one mod to another mod :raw-html:`<br />`

        If this value is ``None``, then will create a new, empty :class:`VGRemaps` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    name: :class:`str`
        The default name for the type of mod

    check: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
        The specific check used to identify the .ini file belongs to the specific type of mod when checking arbitrary line in a .ini file

    hashes: :class:`Hashes`
        The hashes related to the mod and its fix

    indices :class:`Indices`
        The indices related to the mod and its fix

    vgRemaps: :class:`VGRemaps`
        The repository that stores the mapping for remapping vertex group blend indices of the mod to the vertex group blend indices of another mod

    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod
    """

    def __init__(self, name: str, check: Union[str, Pattern, Callable[[str], bool]], hashes: Optional[Hashes], indices: Optional[Indices] = None, aliases: Optional[List[str]] = None, vgRemaps: Optional[VGRemaps] = None):
        self.name = name
        if (hashes is None):
            hashes = Hashes()

        if (indices is None):
            indices = Indices()

        self.hashes = hashes
        self.indices = indices

        self.check = check
        if (isinstance(check, str)):
            self._check = lambda line: line == check
        elif (callable(check)):
            self._check = check
        else:
            self._check = lambda line: bool(check.search(line))
        
        if (aliases is None):
            aliases = []
        self.aliases = ListTools.getDistinct(aliases)
        
        self._maxVgIndex = None
        if (vgRemaps is None):
            vgRemaps = VGRemaps()
        self.vgRemaps = vgRemaps

    def isName(self, name: str) -> bool:
        """
        Determines whether a certain name matches with the names defined for this type of mod

        Parameters
        ----------
        name: :class:`str`
            The name being searched

        Returns
        -------
        :class:`bool`
            Whether the searched name matches with the names for this type of mod
        """

        name = name.lower()
        if (self.name.lower() == name):
            return True
        
        for alias in self.aliases:
            if (alias.lower() == name):
                return True

        return False
    
    def isType(self, iniLine: str) -> bool:
        """
        Determines whether a line in the .ini file correponds with this mod type

        Parameters
        ----------
        iniLine: :class:`str`
            An arbitrary line in a .ini file

        Returns
        -------
        :class:`bool`
            Whether the line in the .ini file corresponds with this type of mod
        """

        return self._check(iniLine)
    

    def getModsToFix(self) -> Set[str]:
        """
        Retrieves the names of the mods this mod type will fix to

        Returns
        -------
        Set[:class:`str`]
            The names of the mods to fix to
        """

        result = set()
        result = result.union(self.hashes.fixTo)
        result = result.union(self.indices.fixTo)
        result = result.union(self.vgRemaps.fixTo)
        return result
    
    def getVGRemap(self, modName: str, version: Optional[float] = None) -> VGRemap:
        """
        Retrieves the corresponding Vertex Group Remap

        .. warning::
            This function assumes that the specified map :attr:`ModType.vgRemaps` (:attr:`VGRemaps.map`) contains :attr:`ModType.name` (the name of this mod type) as a mod to map from

        Parameters
        ----------
        modName: :class:`str`
            The name of the mod to map to

        version: Optional[:class:`float`]
            The specific game version we want for the remap :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will get the latest version of the remap :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns 
        -------
        :class:`VGRemap`
            The corresponding remap
        """

        return self.vgRemaps.get(self.name, modName, version = version)

    def getHelpStr(self) -> str:
        modTypeHeading = Heading(self.name, 8, "-")

        currentHelpStr = f"{modTypeHeading.open()}"
        currentHelpStr += f"\n\nname: {self.name}"
        
        if (self.aliases):
            aliasStr = ", ".join(self.aliases)
            currentHelpStr += f"\naliases: {aliasStr}"

        if (isinstance(self.check, str)):
            currentHelpStr += f"\ndescription: check if the .ini file contains the section named, '{self.check}'"
        elif (not callable(self.check)):
            currentHelpStr += f"\ndescription: check if the .ini file contains a section matching the regex, {self.check.pattern}"

        currentHelpStr += f"\n\n{modTypeHeading.close()}"
        return currentHelpStr
##### EndScript