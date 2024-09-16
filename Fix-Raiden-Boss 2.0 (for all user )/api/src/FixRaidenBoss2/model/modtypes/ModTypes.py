##### ExtImports
import re
from enum import Enum
from typing import Set
##### EndExtImports

##### LocalImports
from .ModType import ModType
from ..assets.Hashes import Hashes
from ..assets.Indices import Indices
from ..assets.VGRemaps import VGRemaps
from ...tools.Heading import Heading
##### EndLocalImports


##### Script
class ModTypes(Enum):
    """
    The supported types of mods that can be fixed

    Attributes
    ----------
    Raiden: :class:`ModType`
        **Raiden mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``[TextureOverride.*(Raiden|Shogun).*Blend]``
    """

    Raiden = ModType("Raiden", re.compile(r"^\s*\[\s*TextureOverride.*(Raiden|Shogun)((?!RemapBlend).)*Blend.*\s*\]"),
                     hashes = Hashes(map = {"Raiden": {"RaidenBoss"}}), indices = Indices(),
                     aliases = ["Ei", "RaidenEi", "Shogun", "RaidenShogun", "RaidenShotgun", "Shotgun", "CrydenShogun", "Cryden", "SmolEi"], 
                     vgRemaps = VGRemaps(map = {"Raiden": {"RaidenBoss"}}))
    
    Jean = ModType("Jean", re.compile(r"^\s*\[\s*TextureOverride.*(Jean)((?!(RemapBlend|CN)).)*Blend.*\s*\]"),
                   Hashes(map = {"Jean": {"JeanCN"}}), Indices(map = {"Jean": {"JeanCN"}}),
                   aliases = ["ActingGrandMaster", "KleesBabySitter"],
                   vgRemaps = VGRemaps(map = {"Jean": {"JeanCN"}}))
    
    JeanCN = ModType("JeanCN", re.compile(r"^\s*\[\s*TextureOverride.*(JeanCN)((?!RemapBlend).)*Blend.*\s*\]"),
                   Hashes(map = {"JeanCN": {"Jean"}}), Indices(map = {"JeanCN": {"Jean"}}),
                   aliases = ["ActingGrandMasterCN", "KleesBabySitterCN"],
                   vgRemaps = VGRemaps(map = {"JeanCN": {"Jean"}}))
    
    Amber = ModType("Amber", re.compile(r"^\s*\[\s*TextureOverride.*(Amber)((?!(RemapBlend|CN)).)*Blend.*\s*\]"),
                    Hashes(map = {"Amber": {"AmberCN"}}),Indices(map = {"Amber": {"AmberCN"}}),
                    aliases = ["BaronBunny", "ColleisBestie"],
                    vgRemaps = VGRemaps(map = {"Amber": {"AmberCN"}}))

    AmberCN = ModType("AmberCN", re.compile(r"^\s*\[\s*TextureOverride.*(AmberCN)((?!RemapBlend).)*Blend.*\s*\]"),
                    Hashes(map = {"AmberCN": {"Amber"}}),Indices(map = {"AmberCN": {"Amber"}}),
                    aliases = ["BaronBunnyCN", "ColleisBestieCN"],
                    vgRemaps = VGRemaps(map = {"AmberCN": {"Amber"}}))
    
    Mona = ModType("Mona", re.compile(r"^\s*\[\s*TextureOverride.*(Mona)((?!(RemapBlend|CN)).)*Blend.*\s*\]"),
                   Hashes(map = {"Mona": {"MonaCN"}}),Indices(map = {"Mona": {"MonaCN"}}),
                   aliases = ["NoMora", "BigHat"],
                   vgRemaps = VGRemaps(map = {"Mona": {"MonaCN"}}))
    
    MonaCN = ModType("MonaCN", re.compile(r"^\s*\[\s*TextureOverride.*(MonaCN)((?!RemapBlend).)*Blend.*\s*\]"),
                   Hashes(map = {"MonaCN": {"Mona"}}),Indices(map = {"MonaCN": {"Mona"}}),
                   aliases = ["NoMoraCN", "BigHatCN"],
                   vgRemaps = VGRemaps(map = {"MonaCN": {"Mona"}}))
    
    Rosaria = ModType("Rosaria", re.compile(r"^\s*\[\s*TextureOverride.*(Rosaria)((?!(RemapBlend|CN)).)*Blend.*\s*\]"),
                      Hashes(map = {"Rosaria": {"RosariaCN"}}), Indices(map = {"Rosaria": {"RosariaCN"}}),
                      aliases = ["GothGirl"],
                      vgRemaps = VGRemaps(map = {"Rosaria": {"RosariaCN"}}))
    
    RosariaCN = ModType("RosariaCN", re.compile(r"^\s*\[\s*TextureOverride.*(RosariaCN)((?!RemapBlend).)*Blend.*\s*\]"),
                      Hashes(map = {"RosariaCN": {"Rosaria"}}), Indices(map = {"RosariaCN": {"Rosaria"}}),
                      aliases = ["GothGirlCN"],
                      vgRemaps = VGRemaps(map = {"RosariaCN": {"Rosaria"}}))
    
    Keqing = ModType("Keqing", re.compile(r"^\s*\[\s*TextureOverride.*(Keqing)((?!(RemapBlend|Opulent)).)*Blend.*\s*\]"),
                   Hashes(map = {"Keqing": {"KeqingOpulent"}}),Indices(map = {"Keqing": {"KeqingOpulent"}}),
                   aliases = ["Kequeen"],
                   vgRemaps = VGRemaps(map = {"Keqing": {"KeqingOpulent"}}))

    KeqingOpulent = ModType("KeqingOpulent", re.compile(r"^\s*\[\s*TextureOverride.*(KeqingOpulent)((?!RemapBlend).)*Blend.*\s*\]"),
            Hashes(map = {"KeqingOpulent": {"Keqing"}}),Indices(map = {"KeqingOpulent": {"Keqing"}}),
            aliases = ["LanternRiteKeqing", "KeqingLaternRite", "CuterKequeen", "LanternRiteKequeen", "KequeenLanternRite", "KequeenOpulent", "CuterKeqing"],
            vgRemaps = VGRemaps(map = {"KeqingOpulent": {"Keqing"}}))
    
    Ningguang = ModType("Ningguang", re.compile(r"^\s*\[\s*TextureOverride.*(Ningguang)((?!(RemapBlend|Orchid)).)*Blend.*\s*\]"),
                   Hashes(map = {"Ningguang": {"NingguangOrchid"}}),Indices(map = {"Ningguang": {"NingguangOrchid"}}),
                   aliases = ["GeoMommy"],
                   vgRemaps = VGRemaps(map = {"Ningguang": {"NingguangOrchid"}}))
    
    NingguangOrchid = ModType("NingguangOrchid", re.compile(r"^\s*\[\s*TextureOverride.*(NingguangOrchid)((?!RemapBlend).)*Blend.*\s*\]"),
                    Hashes(map = {"NingguangOrchid": {"Ningguang"}}),Indices(map = {"NingguangOrchid": {"Ningguang"}}),
                    aliases = ["GeoMommyOrchid"],
                    vgRemaps = VGRemaps(map = {"NingguangOrchid": {"Ningguang"}}))
    
    
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
    def getHelpStr(cls) -> str:
        result = ""
        helpHeading = Heading("supported types of mods", 15)
        result += f"{helpHeading.open()}\n\nThe names/aliases for the mod types are not case sensitive\n\n"

        modTypeHelpTxt = []
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            currentHelpStr = modType.getHelpStr()
            modTypeHelpTxt.append(currentHelpStr)

        modTypeHelpTxt = "\n".join(modTypeHelpTxt)
        result += f"{modTypeHelpTxt}\n\n{helpHeading.close()}"
        return result
##### EndScript