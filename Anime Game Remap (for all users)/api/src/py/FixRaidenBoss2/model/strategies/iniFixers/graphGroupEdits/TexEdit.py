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
from typing import Optional, Tuple, TYPE_CHECKING, Callable
##### EndExtImports

##### CppLocalImports
from .....core import IfContentPart
##### EndCppLocalImports

##### LocalImports
from .....constants.IniConsts import IniKeywords
from .....constants.FileExt import FileExt
from .....tools.TextTools import TextTools
from ....IniNamingTools import IniNamingTools
from ....iniresources.IniResource import IniResource
from .....core import IfTemplate
from ....iniresources.RemapTexResource import RemapTexAddResource
from ....strategies.texEditors.TexCreator import TexCreator
from .ResEdit import ResCreate

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class TexCreate(ResCreate):
    """
    This class inherits from :class:`ResReplace`

    Class that builds the necessary parts to create some new texture file

    Parameters
    ----------
    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

        The tuple contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``resourceRemapBlend``

    texName: :class:`str`
        The name for the type of texture

    texCreator: :class:`TexCreator`
        The editor for the texture file

    resType: :class:`str`
        The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``resourceRemapTexAdd``

    fixFunc: Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]
        The custom function for creating the texture :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    texName: :class:`str`
        The name for the type of texture

    texCreator: :class:`TexCreator`
        The editor for the texture file

    fixFunc: Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]
        The custom function for creating the texture
    """

    def __init__(self, resModObj: Tuple[int, str, str], texName: str, texCreator: TexCreator, resType: str = "resourceRemapTexAdd", fixFunc: Optional[Callable[[RemapTexAddResource], bool]] = None):
        super().__init__(resType, resModObj)
        self.texCreator = texCreator
        self.fixFunc = fixFunc
        self.texName = texName
        self._texInd = 0

    def clear(self):
        self._texInd = 0

    def getFixResourceName(self, resource: str, modType: "ModType", modName: str = "") -> Optional[str]:
        resource = f"{TextTools.capitalize(modName)}{self.texName}"

        if (self._texInd):
            resource += f"{self._texInd}"

        resource = IniNamingTools.getRemapTexResourceName(resource)

        self._texInd += 1
        return resource
    
    def getFixFile(self, file: str, modType: "ModType", modName: str = "", graphId: str = "") -> str:
        result = IniNamingTools.getFixedTexFile(file)
        if (not graphId):
            return result

        return self.fileAddGraphId(file, graphId = graphId)

    def buildResModel(self, resType: str, ini: "IniFile", srcPath: str, modType: "ModType", *args, modName: str = "", **kwargs) -> IniResource:
        return RemapTexAddResource(ini.folder, srcPath, self.texCreator, type = resType, fixFunc = self.fixFunc)

    def buildSection(self, sectionName: str, modType: "ModType", modName: str = "") -> IfTemplate: 
        fileBaseName = sectionName[len(IniKeywords.Resource.value):] if (sectionName.startswith(IniKeywords.Resource.value)) else sectionName
        file = self.getFixFile(f"{fileBaseName}{FileExt.DDS.value}", modType, modName = modName)
        return IfTemplate([IfContentPart({IniKeywords.Filename.value: [(0, file)]}, 0)], name = sectionName)
##### EndScript