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
from enum import Enum
from typing import Set, TYPE_CHECKING
##### EndExtImports

##### LocalImports
from .GIBuilder import GIBuilder
from ..tools.Heading import Heading
from .GlobalClassifiers import GlobalClassifiers

if (TYPE_CHECKING):
    from ..model.strategies.ModType import ModType
##### EndLocalImports


##### Script
ModTypesSearchDFA = GlobalClassifiers.ModTypes.value


class ModTypes(Enum):
    """
    The supported types of mods that can be fixed :raw-html:`<br />`

    .. caution::
        The different :class:`ModType` objects in this enum are used by the software to help fix specific types of mods.

        Modifying the objects within this enum will also modify the behaviour of how this software fixes a particular mod.
        If this side effect is not your intention, then you can construct a brand new :class:`ModType` object from the :class:`GIBuilder` class

    :raw-html:`<br />`

    .. tip::
        Before parsing the regexes below, the text is normalized by being converted to all lowercase

    :raw-html:`<br />`

    Attributes
    ----------
    Amber: :class:`ModType`
        **Amber mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(amber)((?!cn).)*\]``

    AmberCN: :class:`ModType`
        **Amber Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(ambercn).*\]``

    Ayaka: :class:`ModType`
        **Ayaka mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(ayaka)((?!(springbloom)).)*\]``

    AyakaSpringBloom: :class:`ModType`
        **Ayaka Fontaine mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(ayakaspringbloom).*\]``

    Arlecchino: :class:`ModType`
        **Arlecchino mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(arlecchino).*\]``

    Barbara: :class:`ModType`
        **Barabara mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(barbara)((?!summertime).)*\]``

    BarbaraSummertime: :class:`ModType`
        **Barabara Summer mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(barbarasummertime).*\]``

    CherryHuTao: :class:`ModType`
        **Hu Tao Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(cherryhutao|hutaocherry).*\]``

    Diluc: :class:`ModType`
        **Diluc mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(diluc)((?!flamme).)*\]``

    DilucFlamme: :class:`ModType`
        **Diluc Red Dead of the Night mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(dilucflamme).*\]``

    Fischl: :class:`ModType`
        **Fischl mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(fischl)((?!highness).)*\]``

    FischlHighness: :class:`ModType`
        **Fischl Summer mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(fischlhighness).*\]``

    Ganyu: :class:`ModType`
        **Ganyu mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(ganyu)((?!(twilight)).)*\]``

    GanyuTwilight: :class:`ModType`
        **Ganyu Latern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(ganyutwilight).*\]``

    HuTao: :class:`ModType`
        **Hu Tao mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride((?!cherry).)*(hutao)((?!cherry).)*\]``

    Jean: :class:`ModType`
        **Jean mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(jean)((?!(cn|sea)).)*\]``

    JeanCN: :class:`ModType`
        **Jean Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(jeancn)((?!sea).)*\]``

    JeanSea: :class:`ModType`
        **Jean Summertime mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(jeansea)((?!cn).)*\]``

    Kaeya: :class:`ModType`
        **Kaeya mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(kaeya)((?!(sailwind)).)*\]``

    KaeyaSailwind: :class:`ModType`'
        **Kaeya Summertime mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(kaeyasailwind).*\]``

    Keqing: :class:`ModType`
        **Keqing mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(keqing)((?!(opulent)).)*\]``

    KeqingOpulent: :class:`ModType`
        **Keqing Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(keqingopulent).*\]``

    Kirara: :class:`ModType`
        **Kirara mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(kirara)((?!boots).)*\]``

    KiraraBoots: :class:`ModType`
        **Kirara in Boots mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(kiraraboots).*\]``

    Klee: :class:`ModType`
        **Klee mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(klee)((?!blossomingstarlight).)*\]``

    KleeBlossomingStarlight: :class:`ModType`
        **Klee Summertime mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(kleeblossomingstarlight).*\]``

    Lisa: :class:`ModType`
        **Lisa mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(lisa)((?!student).)*\]``

    LisaStudent: :class:`ModType`
        **Lisa Sumeru mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(lisastudent).*\]``

    Mona: :class:`ModType`
        **Mona mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(mona)((?!(cn)).)*\]``

    MonaCN: :class:`ModType`
        **Mona Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(monacn).*\]``

    Nilou: :class:`ModType`
        **Nilou mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(nilou)((?!(breeze)).)*\]``

    NilouBreeze: :class:`ModType`
        **Nilou Forest Fairy mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(niloubreeze).*\]``

    Ningguang: :class:`ModType`
        **Ningguang Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(ningguang)((?!(orchid)).)*\]``

    NingguangOrchid: :class:`ModType`
        **Ningguang Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(ningguangorchid).*\]``

    Raiden: :class:`ModType`
        **Raiden mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(raiden|shogun).*\]``

    Rosaria: :class:`ModType`
        **Rosaria mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(rosaria)((?!(cn)).)*\]``

    RosariaCN: :class:`ModType`
        **Rosaria Chinese mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(rosariacn).*\]``

    Shenhe: :class:`ModType`
        **Shenhe mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(shenhe)((?!frostflower).)*\]``

    ShenheFrostFlower: :class:`ModType`
        **Shenhe Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(shenhefrostflower).*\]``

    Xiangling: :class:`ModType`
        **Xiangling mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(xiangling)((?!cheer).)*\]``

    XianglingCheer: :class:`ModType`
        **Xiangling Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(xiangling(cheer|newyear)).*\]``

    Xingqiu: :class:`ModType`
        **Xingqiu mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(xingqiu)((?!bamboo).)*\]``

    XingqiuBamboo: :class:`ModType`
        **Xingqiu Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(xingqiubamboo).*\]``
    """

    Amber = GIBuilder.amber()
    AmberCN = GIBuilder.amberCN()
    Ayaka = GIBuilder.ayaka()
    AyakaSpringBloom = GIBuilder.ayakaSpringBloom()
    Arlecchino = GIBuilder.arlecchino()
    Barbara = GIBuilder.barbara()
    BarbaraSummertime = GIBuilder.barbaraSummerTime()
    CherryHuTao = GIBuilder.cherryHutao()
    Diluc = GIBuilder.diluc()
    DilucFlamme = GIBuilder.dilucFlamme()
    Fischl = GIBuilder.fischl()
    FischlHighness = GIBuilder.fischlHighness()
    Ganyu = GIBuilder.ganyu()
    GanyuTwilight = GIBuilder.ganyuTwilight()
    HuTao = GIBuilder.huTao()
    Jean = GIBuilder.jean()
    JeanCN = GIBuilder.jeanCN()
    JeanSea = GIBuilder.jeanSea()
    Kaeya = GIBuilder.kaeya()
    KaeyaSailwind = GIBuilder.kaeyaSailwind()
    Keqing = GIBuilder.keqing()
    KeqingOpulent = GIBuilder.keqingOpulent()
    Kirara = GIBuilder.kirara()
    KiraraBoots = GIBuilder.kiraraBoots()
    Klee = GIBuilder.klee()
    KleeBlossomingStarlight = GIBuilder.kleeBlossomingStarlight()
    Lisa = GIBuilder.lisa()
    LisaStudent = GIBuilder.lisaStudent()
    Mona = GIBuilder.mona()
    MonaCN = GIBuilder.monaCN()
    Nilou = GIBuilder.nilou()
    NilouBreeze = GIBuilder.nilouBreeze()
    Ningguang = GIBuilder.ningguang()
    NingguangOrchid = GIBuilder.ningguangOrchid()
    Raiden = GIBuilder.raiden()
    Rosaria = GIBuilder.rosaria()
    RosariaCN = GIBuilder.rosariaCN()
    Shenhe = GIBuilder.shenhe()
    ShenheFrostFlower = GIBuilder.shenheFrostFlower()
    Xiangling = GIBuilder.xiangling()
    XianglingCheer = GIBuilder.xianglingCheer()
    Xingqiu = GIBuilder.xingqiu()
    XingqiuBamboo = GIBuilder.xingqiuBamboo()
    
    @classmethod
    def getAll(cls) -> Set["ModType"]:
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
    def setupSearch(cls):
        if (ModTypesSearchDFA.isSetup):
            return
        
        data = {}
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            data[modType.name.lower()] = modType

            for nickname in modType.aliases:
                data[nickname.lower()] = modType

        ModTypesSearchDFA.setup(data)
    
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

        cls.setupSearch()
        keyword, modType = ModTypesSearchDFA.dfa.getMaximal(name.lower().strip(), errorOnNotFound = False)
        return modType
    
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