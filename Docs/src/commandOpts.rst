.. role:: raw-html(raw)
    :format: html

Command Options
===============


Options
-------
.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Option
     - Description
   * - -h, -\-help   
     - show this help message and exit
   * - -s str, -\-src str
     - | The path to the Raiden mod folder. If this option is not specified, then will
       | use the current directory as the mod folder.
   * - -v str, -\-version str
     - | The game version we want the fix to be compatible with. If this option is not specified,
       | then will use the latest game version
   * - -d, -\-deleteBackup
     - deletes backup copies of the original .ini files
   * - -f, -\-fixOnly
     - only fixes the mod without cleaning any previous runs of the script
   * - -u, -\-undo
     - Undo the previous runs of the script
   * - -ho, -\-hideOriginal
     - Show only the mod on the remapped character and do not show the mod on the original character
   * - -l str, -\-log str
     - | The folder location to log the printed out text into a seperate .txt file.
       | If this option is not specified, then will not log the printed out text.
   * - -a, -\-all
     - | Parses all \*.ini files that the program encounters. 
       | This option supersedes the `-\-types` option.
       |
       | For \*.ini file where a mod cannot be identified, 
       | usually, you would also need to specify what particular mod 
       | the \*.ini file defaults to using the `-\-defaultType` option.
       | 
       | Otherwise, you will be defaulted to fixing 'raiden' mods.
   * - -dt str, -\-defaultType str
     - | The default mod type to use if the \*.ini file belongs to some unknown mod.
       |
       | - If `-\-forceType` is set to True, this option has not effect 
       | - If the `-\-all` is set to True and no values are specified for this option, the default argument for this option is set to 'raiden'
       | - Otherwise, this option has not effect and any unknown mods will be skipped
       | 
       | See below for the different names/aliases of the supported types of mods.
   * - -ft str, -\-forceType str
     - | Forcibly assumes the mod type for all \*.ini file parsed.
       |
       | This option supersedes the `-\-types` option and the `-\-all` option.
       |
       | See below for the different names/aliases of the supported types of mods.
   * - -t str, -\-types str
     - | Parses \*.ini files that the program encounters for only specific types of mods.
       | If the `-\-all` option has been specified, this option has no effect.
       | By default, if this option is not specified, 
       | will parse the \*.ini files for all the supported types of mods.
       |
       | Please specify the types of mods using the the mod type's name or alias, 
       | then seperate each name/alias with a comma(,)
       | *eg. raiden,arlecchino,ayaya*
       |
       | See below for the different names/aliases of the supported types of mods.
   * - -rt str, -\-remappedTypes str
     - | From all the mods to fix, specified by the -\-types option, 
       | will specifically remap those mods to the mods specified by this option.
       |
       | For a mod specified by the -\-types option, if none of its corresponding 
       | remapped mods are specified by this option, then the mod specified by the -\-types option will be remapped to all its corresponding mods.
       |
       | -------------------
       | eg.
       | If this program was ran with the following options:
       | --types kequeen,jean
       | --remappedTypes jeanSea
       | 
       | the program will do the following remap:
       | keqing --> keqingOpulent
       | Jean --> JeanSea
       | 
       | Note that Jean will not remap to JeanCN
       | -------------------
       |
       | By default, if this option is not specified, will remap 
       | all the mods specified in --types to their corresponding remapped mods.
       |
       | Please specify the types of mods using the the mod type's name or alias, 
       | then seperate each name/alias with a comma(,)
       | *eg. raiden,arlecchino,ayaya*
       |
       | See below for the different names/aliases of the supported types of mods.
   * - -dl str, --download str
     - | The download mode to handle file downloads need. The below are the available download modes:
       | 
       | **Disabled** :  Will not perform any downloads
       | **Always** : Will always try to perform downloads, if there is room available to inject a download in the .ini file.
       | **Normal** : Will perform downloads based off the following heuristics
       |    1. Download textures if there is a branch in the texture sections that does not reference a texture
       |    2. Download model binary files if either texture downloads needed to be performed or there is branch 
       |       in the vertex buffer sections that does not reference a resource to some vertex buffer metadata
   * - -p str, --proxy str
     - | The link to the proxy server for those whose internet access must go through a proxy. 
       | The software will make all internet network requests through this proxy

:raw-html:`<br />`
:raw-html:`<br />`

Mod Types
---------

Below are the supported types of mods

:raw-html:`<br />`

.. note::
    Before the regex checks below are parsed, the text is first normalized to
    be all lowercase

.. list-table::
   :widths: 25 25 50
   :header-rows: 1

   * - Name
     - Aliases
     - Description
   * - **Amber**
     - | ColleisBestie, 
       | BaronBunny
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(amber)((?!cn).)*\]``
   * - **AmberCN**
     - | ColleisBestieCN, 
       | BaronBunnyCN
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(ambercn).*\]``
   * - **Ayaka**
     - | Ayaya, 
       | NewArchonOfEternity, 
       | Yandere
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(ayaka)((?!(springbloom)).)*\]``
   * - **AyakaSpringBloom**
     - | AyakaMusketeer, 
       | AyayaFontaine, 
       | AyayaMusketeer, 
       | FontaineAyaya, 
       | FontaineYandere, 
       | MusketeerAyaka, 
       | NewArchonOfEternityFontaine, 
       | NewFontaineArchonOfEternity, 
       | YandereFontaine
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(ayakaspringbloom).*\]``
   * - **Arlecchino**
     - | Father, Knave,
       | Perrie, Peruere,
       | Harlequin
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(arlecchino).*\]``
   * - **Barbara**
     - | Idol, Healer
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(barbara)((?!summertime).)*\]``
   * - **BarbaraSummertime**
     - | IdolSummertime,
       | HealerSummertime,
       | BarbaraBikini
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(barbarasummertime).*\]``
   * - **CherryHuTao**
     - | 77thDirectoroftheWangshengFuneralParlorCherry, 
       | 77thDirectoroftheWangshengFuneralParlorLanternRite, 
       | Cherry77thDirectoroftheWangshengFuneralParlor, 
       | CherryQiqiKidnapper, 
       | HutaoCherry, 
       | HutaoLanternRite, 
       | HutaoSnowLaden, 
       | LanternRite77thDirectoroftheWangshengFuneralParlor, 
       | LanternRiteHutao, 
       | LanternRiteQiqiKidnapper, 
       | QiqiKidnapperCherry, 
       | QiqiKidnapperLanternRite, 
       | SnowLadenHutao
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(cherryhutao|hutaocherry).*\]``
   * - **Diluc**
     - | AngelShareOwner, 
       | DarkNightBlaze, 
       | DawnWineryMaster, 
       | KaeyasBrother
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(diluc)((?!flamme).)*\]``
   * - **DilucFlamme**
     - | DarkNightHero, 
       | RedDeadOfTheNight
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(dilucflamme).*\]``
   * - **Fischl**
     - | FischlvonLuftschlossNarfidort, 
       | 8thGraderSyndrome, Amy, 
       | Chunibyo, 
       | Delusional, 
       | MeinFraulein, 
       | OzsMiss, 
       | PrincessofCondemnation, 
       | PrinzessinderVerurteilung, 
       | TheCondemedPrincess
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(fischl)((?!highness).)*\]``
   * - **FischlHighness**
     - | ImmernachtreichPrincess, 
       | OzsPrincess, 
       | PrincessAmy, 
       | PrincessFischlvonLuftschlossNarfidort, 
       | PrincessoftheEverlastingNight, 
       | Prinzessin, 
       | PrinzessinFischlvonLuftschlossNarfidort, 
       | PrinzessinderImmernachtreich, 
       | RealPrinzessinderVerurteilung
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(fischlhighness).*\]``
   * - **Ganyu**
     - | Cocogoat
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*textureoverride.*(ganyu)((?!(twilight)).)*\]
   * - **GanyuTwilight**
     - | GanyuLanternRite,
       | LanternRiteGanyu,
       | CocogoatTwilight,
       | CocogoatLanternRite,
       | LanternRiteCocogoat
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(ganyutwilight).*\]``
   * - **HuTao**
     - | 77thDirectoroftheWangshengFuneralParlor, 
       | QiqiKidnapper
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride((?!cherry).)*(hutao)((?!cherry).)*\]``
   * - **Jean**
     - | KleesBabySitter, 
       | ActingGrandMaster
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(jean)((?!(cn|sea)).)*\]``
   * - **JeanCN**
     - | KleesBabySitterCN, 
       | ActingGrandMasterCN
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(jeancn)((?!sea).)*\]``
   * - **JeanSea**
     - | ActingGrandMasterSea,
       | KleesBabySitterSea
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(jeansea)((?!cn).)*\]``
   * - **Keqing**
     - | Kequeen,
       | ZhongliSimp
       | MoraxSimp
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(keqing)((?!(opulent)).)*\]``
   * - **KeqingOpulent**
     - | LanternRiteKeqing,
       | KeqingLaternRite,
       | CuterKequeen,
       | LanternRiteKequeen,
       | KequeenLanternRite,
       | KequeenOpulent,
       | CuterKeqing,
       | ZhongliSimpOpulent,
       | MoraxSimpOpulent,
       | ZhongliSimpLaternRite,
       | MoraxSimpLaternRite,
       | LaternRiteZhongliSimp,
       | LaternRiteMoraxSimp
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(keqingopulent).*\]``
   * - **Kirara**
     - | CatBox, KonomiyaExpress, 
       | Nekomata
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(kirara)((?!boots).)*\]``
   * - **KiraraBoots**
     - | CatBoxWithBoots, 
       | KonomiyaExpressInBoots, 
       | NekomataInBoots, 
       | PussInBoots
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(kiraraboots).*\]``
   * - **Klee**
     - | DestroyerofWorlds, 
       | DodocoBuddy, 
       | SparkKnight
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(klee)((?!blossomingstarlight).)*\]``
   * - **KleeBlossomingStarlight**
     - | DodocoLittleWitchBuddy, 
       | FlandreScarlet, 
       | MagicDestroyerofWorlds, 
       | RedVelvetMage
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(kleeblossomingstarlight).*\]``
   * - **Lisa**
     - | CutieLibrarian
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(lisa)((?!student).)*\]``
   * - **LisaStudent**
     - | LisaSumeru, 
       | SumeruLisa, 
       | AkademiyaLisa, 
       | LisaAkademiya
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(lisastudent).*\]``
   * - **Mona**
     - | BigHat, NoMora
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(mona)((?!(cn)).)*\]``
   * - **MonaCN**
     - | NoMoraCN, BigHatCN
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(monacn).*\]``
   * - **Nilou**
     - | BloomGirl, Dancer, Morgiana
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(nilou)((?!(breeze)).)*\]``
   * - **NilouBreeze**
     - | BloomGirlBreeze, 
       | BloomGirlFairy, 
       | DancerBreeze, 
       | DancerFairy, 
       | FairyBloomGirl, 
       | FairyDancer, 
       | FairyMorgiana, 
       | FairyNilou, 
       | ForestFairy, 
       | MorgianaBreeze, 
       | MorgianaFairy, 
       | NilouFairy
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(niloubreeze).*\]``
   * - **Ningguang**
     - | GeoMommy,
       | SugarMommy
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(ningguang)((?!(orchid)).)*\]``
   * - **NingguangOrchid**
     - | NingguangLanternRite,
       | LanternRiteNingguang,
       | GeoMommyOrchid,
       | SugarMommyOrchid,
       | GeoMommyLaternRite,
       | SugarMommyLanternRite,
       | LaternRiteGeoMommy,
       | LanternRiteSugarMommy
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(ningguangorchid).*\]``
   * - **Raiden**
     - | Ei, CrydenShogun, SmolEi, 
       | RaidenEi, Shogun, Shotgun, 
       | RaidenShotgun,
       | Cryden, RaidenShogun
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(raiden|shogun).*\]``
   * - **Rosaria**
     - | GothGirl
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(rosaria)((?!(cn)).)*\]``
   * - **RosariaCN**
     - | GothGirlCN
     - |  check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(rosariacn).*\]``
   * - **Shenhe**
     - | YelansBestie,
       | RedRopes
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(shenhe)((?!frostflower).)*\]``
   * - **ShenheFrostFlower**
     - | ShenheLanternRite,
       | LanternRiteShenhe,
       | YelansBestieFrostFlower,
       | YelansBestieLanternRite,
       | LanternRiteYelansBestie,
       | RedRopesFrostFlower,
       | RedRopesLanternRite,
       | LanternRiteRedRopes
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(shenhefrostflower).*\]``
   * - **Xiangling**
     - | CookingFanatic,
       | HeadChefoftheWanminRestaurant,
       | ChefMaosDaughter,
       | GuobasBuddy
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(xiangling)((?!cheer|newyear).)*\]``
   * - **XianglingCheer**
     - | XianglingLanternRite,
       | LanternRiteXiangling,
       | CookingFanaticLanternRite,
       | HeadChefoftheWanminRestaurantLanternRite,
       | ChefMaosDaughterLanternRite,
       | GuobasBuddyLanternRite,
       | LanternRiteCookingFanatic,
       | LanternRiteHeadChefoftheWanminRestaurant,
       | LanternRiteChefMaosDaughter,
       | LanternRiteGuobasBuddy
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(xiangling(cheer|newyear)).*\]``
   * - **Xingqiu**
     - | Bookworm, ChongyunsBestie, 
       | GuhuaGeek, 
       | SecondSonofTheFeiyunCommerceGuild
     - | check if the .ini file contains a section matching the regex,
       | ``^\s*\[\s*textureoverride.*(xingqiu)((?!bamboo).)*\]``
   * - **XingqiuBamboo**
     - | BookwormBamboo, 
       | BookwormLanternRite, 
       | ChongyunsBestieBamboo, 
       | ChongyunsBestieLanternRite, 
       | GuhuaGeekBamboo, 
       | GuhuaGeekLanternRite, 
       | LanternRiteBookworm, 
       | LanternRiteChongyunsBestie, 
       | LanternRiteGuhuaGeek, 
       | LanternRiteSecondSonofTheFeiyunCommerceGuild, 
       | LanternRiteXingqiu, 
       | SecondSonofTheFeiyunCommerceGuildBamboo, 
       | SecondSonofTheFeiyunCommerceGuildLanternRite, 
       | XingqiuLanternRite
     - | check if the .ini file contains a section matching the regex, 
       | ``^\s*\[\s*textureoverride.*(xingqiubamboo).*\]``


:raw-html:`<br />`
:raw-html:`<br />`

Download Modes
--------------

Below are the differents download modes supported by the software.

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Mode Name
     - Description
   * - **Disabled**
     - Will not perform any file downloads for any mods
   * - **Always**
     - Will always perform file downloads for every mod
   * - **Normal**
     - | Will perform file downloads based off the following heuristics:
       |
       | 1. Download textures or .ib files if there is a branch in the texture sections that does not reference the files
       | 2. Download model binary files if either texture/.ib downloads needed to be performed or there is branch in the vertex buffer sections that does not reference a resource to some vertex buffer metadata
       |
       | **WARNING**
       |    The following heuristics may not download any files for certain cases that require file downloads 
       | 
       |    In such cases, you may need to switch using the **Always** download mode