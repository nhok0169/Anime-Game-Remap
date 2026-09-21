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
from typing import Set, TYPE_CHECKING, Optional, Type
##### EndExtImports

##### LocalImports
from .BaseModTypeBuilder import BaseModTypeBuilder
from ..core import GIBuilder, WWMIBuilder
from ..tools.Heading import Heading
from ..tools.Builder import Builder
from .GlobalClassifiers import GlobalClassifiers
from ..tools.tries.AhoCorasickSingleton import AhoCorasickSingleton
from ..tools.enums.DeferredEnum import DeferredEnum
from ..tools.enums.StrEnum import StrEnum

if (TYPE_CHECKING):
    from ..model.strategies.ModType import ModType
##### EndLocalImports


##### Script
class ModTypes(StrEnum, DeferredEnum):
    r"""
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

    Bennett: :class:`ModType`
        **Bennett mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(bennett)((?!adventure).)*\]``

    BennettAdventure: :class:`ModType`
        **Bennett Summertime Adventure mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(bennettadventure).*\]``

    CherryHuTao: :class:`ModType`
        **Hu Tao Lantern Rite mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(cherryhutao|hutaocherry).*\]``

    Citlali: :class:`ModType`
        **Citlali mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(citlali)((?!whisperofstars).)*\]``

    CitlaliWhisperofStars: :class:`ModType`
        **Citlali Whisper of Stars mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(citlaliwhisperofstars).*\]``

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

    Sanhua: :class:`ModType`
        **Sanhua mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the character's vertex buffer hash, eg. ``hash = 33e4890f`` -- a WWMI .ini names its sections after the draw slot (``[TextureOverrideComponent0]``) rather than after the character

    SanhuaExorcist: :class:`ModType`
        **Sanhua Moon Chasing skin mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the character's vertex buffer hash, eg. ``hash = b101dcf3`` -- a WWMI .ini names its sections after the draw slot (``[TextureOverrideComponent0]``) rather than after the character

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

    Yelan: :class:`ModType`
        **Yelan mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(yelan)((?!tranquil).)*\]``

    YelanTranquil: :class:`ModType`
        **Yelan Tranquil Banquet mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``^\s*\[\s*textureoverride.*(yelantranquil).*\]``
    """

    Amber = (GIBuilder.amber, )
    AmberCN = (GIBuilder.amberCN, )
    Ayaka = (GIBuilder.ayaka, )
    AyakaSpringBloom = (GIBuilder.ayakaSpringBloom, )
    Arlecchino = (GIBuilder.arlecchino, )
    Barbara = (GIBuilder.barbara, )
    BarbaraSummertime = (GIBuilder.barbaraSummerTime, )
    Bennett = (GIBuilder.bennett, )
    BennettAdventure = (GIBuilder.bennettAdventure, )
    CherryHuTao = (GIBuilder.cherryHutao, )
    Citlali = (GIBuilder.citlali, )
    CitlaliWhisperofStars = (GIBuilder.citlaliWhisperofStars, )
    Diluc = (GIBuilder.diluc, )
    DilucFlamme = (GIBuilder.dilucFlamme, )
    Fischl = (GIBuilder.fischl, )
    FischlHighness = (GIBuilder.fischlHighness, )
    Ganyu = (GIBuilder.ganyu, )
    GanyuTwilight = (GIBuilder.ganyuTwilight, )
    HuTao = (GIBuilder.huTao, )
    Jean = (GIBuilder.jean, )
    JeanCN = (GIBuilder.jeanCN, )
    JeanSea = (GIBuilder.jeanSea, )
    Kaeya = (GIBuilder.kaeya, )
    KaeyaSailwind = (GIBuilder.kaeyaSailwind, )
    Keqing = (GIBuilder.keqing, )
    KeqingOpulent = (GIBuilder.keqingOpulent, )
    Kirara = (GIBuilder.kirara, )
    KiraraBoots = (GIBuilder.kiraraBoots, )
    Klee = (GIBuilder.klee, )
    KleeBlossomingStarlight = (GIBuilder.kleeBlossomingStarlight, )
    Lisa = (GIBuilder.lisa, )
    LisaStudent = (GIBuilder.lisaStudent, )
    Mona = (GIBuilder.mona, )
    MonaCN = (GIBuilder.monaCN, )
    Nilou = (GIBuilder.nilou, )
    NilouBreeze = (GIBuilder.nilouBreeze, )
    Ningguang = (GIBuilder.ningguang, )
    NingguangOrchid = (GIBuilder.ningguangOrchid, )
    Raiden = (GIBuilder.raiden, )
    Rosaria = (GIBuilder.rosaria, )
    RosariaCN = (GIBuilder.rosariaCN, )
    Sanhua = (WWMIBuilder.sanhua, )
    SanhuaExorcist = (WWMIBuilder.sanhuaExorcist, )
    Shenhe = (GIBuilder.shenhe, )
    ShenheFrostFlower = (GIBuilder.shenheFrostFlower, )
    Xiangling = (GIBuilder.xiangling, )
    XianglingCheer = (GIBuilder.xianglingCheer, )
    Xingqiu = (GIBuilder.xingqiu, )
    XingqiuBamboo = (GIBuilder.xingqiuBamboo, )
    Yelan = (GIBuilder.yelan, )
    YelanTranquil = (GIBuilder.yelanTranquil, )
    
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
    def _buildAhocorasickDFA(cls) -> AhoCorasickSingleton:
        data = {}
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            data[modType.name.lower()] = modType

            for nickname in modType.aliases:
                data[nickname.lower()] = modType

        dfa = GlobalClassifiers.ModTypes.value
        dfa.setup(data)
        return dfa
    
    @classmethod
    def search(cls, txt: str) -> Optional["ModTypes"]:
        return super().search(txt.lower().strip())
    
    @classmethod
    def getHelpStr(cls, showFullMods: bool = False) -> str:
        """
        Retrieves the help text for the supported mod types, for the CLI's ``--help`` epilog

        Parameters
        ----------
        showFullMods: :class:`bool`
            Whether every mod type is listed, each with its aliases and how it is identified, rather
            than the reader being sent to the documentation :raw-html:`<br />` :raw-html:`<br />`

            Defaults to ``False``, which prints **no list at all**, only the link. The number of
            supported mods grows with every remap, and a ``--help`` that scrolls a page of character
            names before reaching the options is worse than useless -- the documentation page below
            is the same data, carries the aliases with it, and is not capped by a terminal
            :raw-html:`<br />` :raw-html:`<br />`

            Compare :meth:`GameTypes.getHelpStr`, which defaults to ``True``: there is a handful of
            games and nowhere else worth sending the reader

        Returns
        -------
        :class:`str`
            The help text for the supported mod types
        """

        result = ""
        helpHeading = Heading("supported types of mods", 15)
        result += f"{helpHeading.open()}\n\nThe names/aliases for the mod types are not case sensitive\n\n"

        if (not showFullMods):
            result += "For the list of all the supported mods, their aliases and how each one is identified, please visit:\nhttps://anime-game-remap.readthedocs.io/en/latest/commandOpts.html#mod-types\n\n"
            result += f"{helpHeading.close()}"
            return result

        modTypeHelpTxt = []
        for modTypeEnum in cls:
            modTypeHelpTxt.append(modTypeEnum.value.getHelpStr())

        modTypeHelpTxt = "\n".join(modTypeHelpTxt)

        result += f"{modTypeHelpTxt}\n\n{helpHeading.close()}"
        return result
    

class ModTypeBuilder(BaseModTypeBuilder, Builder):
    """
    Class to dynamically build a new :class:`ModType`
    """

    def __init__(self, name: str, buildCls: Type["ModType"], args = None, kwargs = None):
        self.name = name
        super().__init__(buildCls, args, kwargs)
##### EndScript