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
from enum import Enum
from typing import Set
##### EndExtImports

##### LocalImports
from ..model.strategies.ModType import ModType
from .GIBuilder import GIBuilder
from ..tools.Heading import Heading
##### EndLocalImports


##### Script
class ModTypes(Enum):
    """
    The supported types of mods that can be fixed

    Attributes
    ----------
    Amber: :class:`ModType`
        **Amber mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Amber)((?!(RemapBlend|CN)).)*Blend.*\s*\]``

    AmberCN: :class:`ModType`
        **Amber Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(AmberCN)((?!RemapBlend).)*Blend.*\s*\]``

    Arlecchino: :class:`ModType`
        **Arlecchino mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Arlecchino)((?!RemapBlend).)*Blend.*\s*\]``

    Barbara: :class:`ModType`
        **Barabara mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Barbara)((?!RemapBlend|Summertime).)*Blend.*\s*\]``

    BarbaraSummertime: :class:`ModType`
        **Barabara Summertime mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(BarbaraSummertime)((?!RemapBlend).)*Blend.*\s*\]``

    Ganyu: :class:`ModType`
        **Ganyu mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Ganyu)((?!(RemapBlend|Twilight)).)*Blend.*\s*\]``

    GanyuTwilight: :class:`ModType`
        **Ganyu Latern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(GanyuTwilight)((?!(RemapBlend)).)*Blend.*\s*\]``

    Jean: :class:`ModType`
        **Jean mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Jean)((?!(RemapBlend|CN|Sea)).)*Blend.*\s*\]``

    JeanCN: :class:`ModType`
        **Jean Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(JeanCN)((?!RemapBlend|Sea).)*Blend.*\s*\]``

    JeanSea: :class:`ModType`
        **Jean Summertime mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(JeanSea)((?!RemapBlend|CN).)*Blend.*\s*\]``

    Keqing: :class:`ModType`
        **Keqing mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Keqing)((?!(RemapBlend|Opulent)).)*Blend.*\s*\]``

    KeqingOpulent: :class:`ModType`
        **Keqing Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(KeqingOpulent)((?!RemapBlend).)*Blend.*\s*\]``

    Mona: :class:`ModType`
        **Mona mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Mona)((?!(RemapBlend|CN)).)*Blend.*\s*\]``

    MonaCN: :class:`ModType`
        **Mona Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(MonaCN)((?!RemapBlend).)*Blend.*\s*\]``

    Ningguang: :class:`ModType`
        **Ningguang Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Ningguang)((?!(RemapBlend|Orchid)).)*Blend.*\s*\]``

    NingguangOrchid: :class:`ModType`
        **Ningguang Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(NingguangOrchid)((?!RemapBlend).)*Blend.*\s*\]``

    Raiden: :class:`ModType`
        **Raiden mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Raiden|Shogun)((?!RemapBlend).)*Blend.*\s*\]``

    Rosaria: :class:`ModType`
        **Rosaria mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Rosaria)((?!(RemapBlend|CN)).)*Blend.*\s*\]``

    RosariaCN: :class:`ModType`
        **Rosaria Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(RosariaCN)((?!RemapBlend).)*Blend.*\s*\]``

    Shenhe: :class:`ModType`
        **Shenhe mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(Shenhe)((?!RemapBlend|FrostFlower).)*Blend.*\s*\]``

    ShenheFrostFlower: :class:`ModType`
        **Shenhe Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*TextureOverride.*(ShenheFrostFlower)((?!RemapBlend).)*Blend.*\s*\]``
    """

    Amber = GIBuilder.amber()
    AmberCN = GIBuilder.amberCN()
    Arlecchino = GIBuilder.arlecchino()
    Barbara = GIBuilder.barbara()
    BarbaraSummertime = GIBuilder.barbaraSummerTime()
    Ganyu = GIBuilder.ganyu()
    GanyuTwilight = GIBuilder.ganyuTwilight()
    Jean = GIBuilder.jean()
    JeanCN = GIBuilder.jeanCN()
    JeanSea = GIBuilder.jeanSea()
    Keqing = GIBuilder.keqing()
    KeqingOpulent = GIBuilder.keqingOpulent()
    Mona = GIBuilder.mona()
    MonaCN = GIBuilder.monaCN()
    Ningguang = GIBuilder.ningguang()
    NingguangOrchid = GIBuilder.ningguangOrchid()
    Raiden = GIBuilder.raiden()
    Rosaria = GIBuilder.rosaria()
    RosariaCN = GIBuilder.rosariaCN()
    Shenhe = GIBuilder.shenhe()
    ShenheFrostFlower = GIBuilder.shenheFrostFlower()
    
    @classmethod
    def getAll(cls) -> Set[ModType]:
        """
        Retrieves a set of all the mod types available

        Returns
        -------
        Set[:class:`ModType`]
            All the available mod types
        """

        result = set()
        for modTypeEnum in cls:
            result.add(modTypeEnum.value)
        return result
    
    @classmethod
    def search(cls, name: str):
        """
        Searches a mod type based off the provided name

        Parameters
        ----------
        name: :class:`str`
            The name of the mod to search for

        Returns
        -------
        Optional[:class:`ModType`]
            The found mod type based off the provided name
        """

        result = None
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            if (modType.isName(name)):
                result = modType
                break
        
        return result
    
    @classmethod
    def getHelpStr(cls, showFullMods: bool = False) -> str:
        result = ""
        helpHeading = Heading("supported types of mods", 15)
        result += f"{helpHeading.open()}\n\nThe names/aliases for the mod types are not case sensitive\n\n"

        if (not showFullMods):
            result += "Below contains a condensed list of all the supported mods, for more details, please visit:\nhttps://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api#mod-types\n\n"

        modTypeHelpTxt = []
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            
            if (showFullMods):
                currentHelpStr = modType.getHelpStr()
            else:
                currentHelpStr = f"- {modType.name}"

            modTypeHelpTxt.append(currentHelpStr)

        modTypeHelpTxt = "\n".join(modTypeHelpTxt)
        
        result += f"{modTypeHelpTxt}\n\n{helpHeading.close()}"
        return result
##### EndScript