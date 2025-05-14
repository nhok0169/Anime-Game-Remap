##### ExtImports
from typing import Union, Optional, Callable, List, Set, TYPE_CHECKING
##### EndExtImports

##### LocalImports
from ...constants.GlobalIniRemoveBuilders import GlobalIniRemoveBuilders
from ...constants.GenericTypes import Pattern
from ..assets.Hashes import Hashes
from ..assets.Indices import Indices
from ..assets.VertexCounts import VertexCounts
from ..assets.VGRemaps import VGRemaps
from ..assets.PositionEditors import PositionEditors
from ..VGRemap import VGRemap
from ...tools.ListTools import ListTools
from ...tools.Heading import Heading
from ...model.strategies.iniParsers.IniParseBuilder import IniParseBuilder
from ...model.strategies.iniParsers.GIMIParser import GIMIParser
from ...model.strategies.iniFixers.IniFixBuilder import IniFixBuilder
from ...model.strategies.iniFixers.GIMIFixer import GIMIFixer
from ...model.strategies.iniRemovers.IniRemoveBuilder import IniRemoveBuilder
from ...model.strategies.bufEditors.BaseBufEditor import BaseBufEditor

if (TYPE_CHECKING):
    from ..files.IniFile import IniFile
##### EndLocalImports


##### Script
class ModType():
    """
    Class for defining a generic type of mod

    Parameters
    ----------
    name: :class:`str`
        The default name for the type of mod

    hashes: Optional[:class:`Hashes`]
        The hashes related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will create a new, empty :class:`Hashes` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    indices: Optional[:class:`Indices`]
        The indices related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

        If this ``None``, then will create a new, emtpy :class:`Indices` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    vgRemaps: Optional[:class:`VGRemaps`]
        Maps the blend indices from the vertex group of one mod to another mod :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will create a new, empty :class:`VGRemaps` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    positionEditors: Optional[:class:`PositionEditors`]
        The editors used for fixing position.buf files :raw-html:`<br />` :raw-html:`<br />`

        If this ``None``, then will create a new, emtpy :class:`PositionEditors` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    iniParseBuilder: Optional[:class:`IniParseBuilder`]
        The builder to build the parser used for .ini files :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then by default this attribute will be set to **IniParseBuilder(:class:`GIMIParser`)** :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    iniFixBuilder: Optional[:class:`IniFixBuilder`]
        The builder to build the fixer used for .ini files :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then by default this attribute will be set to **IniFixBuilder(:class:`GIMIFixer`)** :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    iniRemoveBuilder: Optional[:class:`IniRemoveBuilder`]
        The builder to build the remover used for .ini files :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then by default this attribute will be set to **IniRemoveBuilder(:class:`IniRemover`)** :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    name: :class:`str`
        The default name for the type of mod

    hashes: :class:`Hashes`
        The hashes related to the mod and its fix

    indices: :class:`Indices`
        The indices related to the mod and its fix

    vertexCounts: :class:`VertexCounts`
        The vertex counts related to the mod and its fix

    vgRemaps: :class:`VGRemaps`
        The repository that stores the mapping for remapping vertex group blend indices of the mod to the vertex group blend indices of another mod

    positionEditors: :class:`PositionEditors`
        The editors used for fixing position.buf files

    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod

    iniParseBuilder: :class:`IniParseBuilder`
        The builder to build the parser used for .ini files

    iniFixBuilder: :class:`IniFixBuilder`
        the builder to build the fixer used for .ini files

    iniRemoveBuilder: :class:`IniRemoveBuilder`
        the builder to build the remover used for .ini files
    """

    def __init__(self, name: str, hashes: Optional[Hashes] = None, indices: Optional[Indices] = None, vertexCounts: Optional[VertexCounts] = None,
                 aliases: Optional[List[str]] = None, vgRemaps: Optional[VGRemaps] = None, positionEditors: Optional[PositionEditors] = None, 
                 iniParseBuilder: Optional[IniParseBuilder] = None, iniFixBuilder: Optional[IniFixBuilder] = None, iniRemoveBuilder: Optional[IniRemoveBuilder] = None):
        self.name = name
        if (hashes is None):
            hashes = Hashes()

        if (indices is None):
            indices = Indices()

        if (vertexCounts is None):
            vertexCounts = VertexCounts()

        if (positionEditors is None):
            positionEditors = PositionEditors({})

        self.hashes = hashes
        self.indices = indices
        self.vertexCounts = vertexCounts
        
        if (aliases is None):
            aliases = []
        self.aliases = ListTools.getDistinct(aliases)
        
        self._maxVgIndex = None
        if (vgRemaps is None):
            vgRemaps = VGRemaps()

        self.vgRemaps = vgRemaps

        if (iniParseBuilder is None):
            iniParseBuilder = IniParseBuilder(GIMIParser)

        if (iniFixBuilder is None):
            iniFixBuilder = IniFixBuilder(GIMIFixer)

        if (iniRemoveBuilder is None):
            iniRemoveBuilder = GlobalIniRemoveBuilders.RemoveBuilder.value

        self.positionEditors = positionEditors
        self.iniParseBuilder = iniParseBuilder
        self.iniFixBuilder = iniFixBuilder
        self.iniRemoveBuilder = iniRemoveBuilder

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
        result = result.union(self.positionEditors.fixTo)
        return result
    
    def getVertexCount(self, version: Optional[float] = None) -> int:
        """
        Retrieves the number of vertices for a mod

        .. attention::
            This function assumes that the specified dictionary :attr:`vertexCounts` (:attr:`VertexCounts.map`) contains :attr:`name` (the name of this mod type) as a mod to get the vertex count for

        Parameters
        ----------
        version: Optional[:class:`float`]
            The specific game version we want for the vertex count :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will get the latest version of the vertex count :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns 
        -------
        :class:`int`
            The number of vertices for the mod
        """

        return self.vertexCounts.get(self.name, version = version)
    
    def getVGRemap(self, modName: str, version: Optional[float] = None) -> VGRemap:
        """
        Retrieves the corresponding Vertex Group Remap

        .. attention::
            This function assumes that the specified map :attr:`vgRemaps` (:attr:`VGRemaps.map`) contains :attr:`name` (the name of this mod type) as a mod to map from

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
    
    def getPositionEditor(self, modName: str, version: Optional[float] = None) -> Optional[BaseBufEditor]:
        """
        Retrieves the corresponding position editor for editting position.buf files

        .. attention::
            This function assumes that the specified map :attr:`positionEditors` (:attr:`PositionEditors.map`) contains :attr:`name` (the name of this mod type) as a mod to map from

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
        Optional[:class:`BaseBufEditor`]
            The corresponding position editor
        """

        return self.positionEditors.get(self.name, modName, version = version)

    def getHelpStr(self) -> str:
        modTypeHeading = Heading(self.name, 8, "-")

        currentHelpStr = f"{modTypeHeading.open()}"
        currentHelpStr += f"\n\nname: {self.name}"
        
        if (self.aliases):
            sortedAliases = sorted(self.aliases)
            aliasStr = ", ".join(sortedAliases)
            currentHelpStr += f"\naliases: {aliasStr}"

        currentHelpStr += f"\n\n{modTypeHeading.close()}"
        return currentHelpStr
    
    def fixIni(self, iniFile: "IniFile", keepBackup: bool = True, fixOnly: bool = False):
        """
        Fixes the .ini file associated to this type of mod

        Parameters
        ----------
        iniFile: :class:`IniFile`
            The .ini file to fix

        keepBackup: :class:`bool`
            Whether to keep backups for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether to only fix the .ini file without undoing any fixes :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        iniModType = iniFile.availableType
        if (iniModType is not None and iniModType.name == self.name):
            iniFile.fix(keepBackup = keepBackup, fixOnly = fixOnly)
##### EndScript