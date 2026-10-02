.. role:: raw-html(raw)
    :format: html

Command Options
===============


Options
-------
The **Build** column says which build has the option. The Script accepts every option of the API, since it hands them on to the API.

.. list-table::
   :widths: 25 15 60
   :header-rows: 1

   * - Option
     - Build
     - Description
   * - -h, -\-help   
     - API, Script
     - show this help message and exit
   * - -s str, -\-src str
     - API, Script
     - | The starting path to run this fix. If this option is not specified, then will
       | run the fix from the current directory.
   * - -v str, -\-version str
     - API, Script
     - | The game version we want the fix to be compatible with. If this option is not specified,
       | then will use the latest game version
   * - -fv str, -\-fromVersion str
     - API, Script
     - The game version the mods being fixed were made for. This picks how the mods are read (which hashes/indices they are looked up by), where ``--version`` picks the fix that is written. If this option is not specified, then will read the mods as the latest game version.
   * - -d, -\-deleteBackup
     - API, Script
     - deletes backup copies of the original .ini files
   * - -f, -\-fixOnly
     - API, Script
     - only fixes the mod without cleaning any previous runs of the script
   * - -u, -\-undo
     - API, Script
     - Undo the previous runs of the script
   * - -ho, -\-hideOriginal
     - API, Script
     - Show only the mod on the remapped character and do not show the mod on the original character
   * - -l str, -\-log str
     - API, Script
     - | The folder location to log the printed out text into a seperate .txt file.
       | If this option is not specified, then will not log the printed out text.
   * - -a, -\-all
     - API, Script
     - | Parses all \*.ini files that the program encounters. 
       | This option supersedes the ``--types`` option.
       |
       | For \*.ini file where a mod cannot be identified, 
       | usually, you would also need to specify what particular mod 
       | the \*.ini file defaults to using the ``--defaultType`` option.
       | 
       | Otherwise, you will be defaulted to fixing 'raiden' mods.
   * - -dt str, -\-defaultType str
     - API, Script
     - | The default mod type to use if the \*.ini file belongs to some unknown mod.
       |
       | - If ``--forceType`` is set to True, this option has not effect 
       | - If the ``--all`` is set to True and no values are specified for this option, the default argument for this option is set to 'raiden'
       | - Otherwise, this option has not effect and any unknown mods will be skipped
       | 
       | See below for the different names/aliases of the supported types of mods.
   * - -ft str, -\-forceType str
     - API, Script
     - | Forcibly assumes the mod type for all \*.ini file parsed.
       |
       | This option supersedes the ``--types`` option and the ``--all`` option.
       |
       | See below for the different names/aliases of the supported types of mods.
   * - -t str, -\-types str
     - API, Script
     - | Parses \*.ini files that the program encounters for only specific types of mods.
       | If the ``--all`` option has been specified, this option has no effect.
       | By default, if this option is not specified, 
       | will parse the \*.ini files for all the supported types of mods.
       |
       | Please specify the types of mods using the the mod type's name or alias, 
       | then seperate each name/alias with a comma(,)
       | *eg. raiden,arlecchino,ayaya*
       |
       | See below for the different names/aliases of the supported types of mods.
   * - -rt str, -\-remappedTypes str
     - API, Script
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
   * - -g str, -\-game str
     - API, Script
     - | Fixes mods only for the specified games.
       | By default, if this option is not specified, will fix mods for all the supported games.
       |
       | Please specify the types of games using the game type's name or alias,
       | then seperate each name/alias with a comma(,)
       | *eg. GI,WuWa*
       |
       | See :ref:`Game Types <commandOpts:Game Types>` for the different names/aliases of the supported types of games.
   * - -c, -\-compressTextures
     - API, Script
     - | Whether to compress the textures the fix writes.
       |
       | **By default textures are left uncompressed**, which is what the older pure-Python
       | versions always did. Pick your poison: do you want the fix to run faster, but your
       | textures take up more space, OR your fix to run slower, but textures take minimal
       | space.
       |
       | This option only *permits* compression. Specifying it lets each mod type's own
       | texture edits decide for themselves; an edit that deliberately writes an
       | uncompressed texture still does so.
   * - -dl str, -\-download str
     - API, Script
     - | The download mode to handle file downloads need. The below are the available download modes:
       | 
       | See :ref:`Download Modes <commandOpts:Download Modes>` for details on the available download modes.
       |
       | By default, the download mode used is: **Normal**
   * - -p str, -\-proxy str
     - API, Script
     - | The link to the proxy server for those whose internet access must go through a proxy. 
       | The software will make all internet network requests through this proxy
   * - -up, -\-update
     - Script
     - Updates the ``FixRaidenBoss2`` package (the API) to its latest version before running. Without this option, the package is only downloaded when it is not already installed.
   * - -pre, -\-preRelease
     - Script
     - Also considers prereleases of the ``FixRaidenBoss2`` package when downloading it.

:raw-html:`<br />`
:raw-html:`<br />`

Mod Types
---------

Below are the supported types of mods

:raw-html:`<br />`

.. list-table::
   :widths: 20 10 25 45
   :header-rows: 1

   * - Name
     - Game
     - Aliases
     - Description
   * - **Amber**
     - GI
     - | ColleisBestie, 
       | BaronBunny
     - Amber mods
   * - **AmberCN**
     - GI
     - | ColleisBestieCN, 
       | BaronBunnyCN
     - Amber Chinese mods
   * - **Arlecchino**
     - GI
     - | Father, Knave,
       | Perrie, Peruere,
       | Harlequin
     - Arlecchino mods
   * - **Ayaka**
     - GI
     - | Ayaya, 
       | NewArchonOfEternity, 
       | Yandere
     - Ayaka mods
   * - **AyakaSpringBloom**
     - GI
     - | AyakaMusketeer, 
       | AyayaFontaine, 
       | AyayaMusketeer, 
       | FontaineAyaya, 
       | FontaineYandere, 
       | MusketeerAyaka, 
       | NewArchonOfEternityFontaine, 
       | NewFontaineArchonOfEternity, 
       | YandereFontaine
     - Ayaka Fontaine mods
   * - **Barbara**
     - GI
     - | Idol, Healer
     - Barbara mods
   * - **BarbaraSummertime**
     - GI
     - | IdolSummertime,
       | HealerSummertime,
       | BarbaraBikini
     - Barbara Summer mods
   * - **Bennett**
     - GI
     - | Benny
     - Bennett mods
   * - **BennettAdventure**
     - GI
     - | AdventureBennett,
       | AdventureBenny,
       | BennettNatlan,
       | BennettSummer,
       | BennyAdventure,
       | BennyNatlan,
       | BennySummer,
       | NatlanBennett,
       | NatlanBenny,
       | SummerBennett,
       | SummerBenny
     - Bennett Summertime Adventure mods
   * - **Charlotte**
     - GI
     - 
     - Charlotte mods
   * - **CharlotteHurlock**
     - GI
     - HurlockCharlotte
     - Charlotte Hurlock mods
   * - **CherryHuTao**
     - GI
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
     - Hu Tao Lantern Rite mods
   * - **Chisa**
     - WuWa
     - 
     - Chisa mods
   * - **ChisaParfait**
     - WuWa
     - ChisaSkin1, ParfaitChisa
     - Chisa Parfait skin mods
   * - **Citlali**
     - GI
     - 
     - Citlali mods
   * - **CitlaliWhisperofStars**
     - GI
     - CitlaliStars, CitlaliWhisper, StarsCitlali, WhisperCitlali, WhisperofStarsCitlali
     - Citlali Whisper of Stars mods
   * - **Diluc**
     - GI
     - | AngelShareOwner, 
       | DarkNightBlaze, 
       | DawnWineryMaster, 
       | KaeyasBrother
     - Diluc mods
   * - **DilucFlamme**
     - GI
     - | DarkNightHero, 
       | RedDeadOfTheNight
     - Diluc Red Dead of Night mods
   * - **Fischl**
     - GI
     - | FischlvonLuftschlossNarfidort, 
       | 8thGraderSyndrome, Amy, 
       | Chunibyo, 
       | Delusional, 
       | MeinFraulein, 
       | OzsMiss, 
       | PrincessofCondemnation, 
       | PrinzessinderVerurteilung, 
       | TheCondemedPrincess
     - Fischl mods
   * - **FischlHighness**
     - GI
     - | ImmernachtreichPrincess, 
       | OzsPrincess, 
       | PrincessAmy, 
       | PrincessFischlvonLuftschlossNarfidort, 
       | PrincessoftheEverlastingNight, 
       | Prinzessin, 
       | PrinzessinFischlvonLuftschlossNarfidort, 
       | PrinzessinderImmernachtreich, 
       | RealPrinzessinderVerurteilung
     - Fischl Summer mods
   * - **Ganyu**
     - GI
     - | Cocogoat
     - Ganyu mods
   * - **GanyuTwilight**
     - GI
     - | GanyuLanternRite,
       | LanternRiteGanyu,
       | CocogoatTwilight,
       | CocogoatLanternRite,
       | LanternRiteCocogoat
     - Ganyu Lantern Rite mods
   * - **HuTao**
     - GI
     - | 77thDirectoroftheWangshengFuneralParlor, 
       | QiqiKidnapper
     - Hu Tao mods
   * - **Jean**
     - GI
     - | KleesBabySitter, 
       | ActingGrandMaster
     - Jean mods
   * - **JeanCN**
     - GI
     - | KleesBabySitterCN, 
       | ActingGrandMasterCN
     - Jean Chinese mods
   * - **JeanSea**
     - GI
     - | ActingGrandMasterSea,
       | KleesBabySitterSea
     - Jean Summertime mods
   * - **Kaeya**
     - GI
     - | DilucsBrother, CavalryCaptain
     - Kaeya mods
   * - **KaeyaSailwind**
     - GI
     - | DilucsBrotherSailwind, 
       | CavalryCaptainSailwind, 
       | TheftKaeya, TheftDilucsBrother, 
       | TheftCavalryCaptain, 
       | KaeyaTheft, DilucsBrotherTheft, 
       | CavalryCaptainTheft
     - Kaeya Summertime mods
   * - **Keqing**
     - GI
     - | Kequeen,
       | ZhongliSimp
       | MoraxSimp
     - Keqing mods
   * - **KeqingOpulent**
     - GI
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
     - Keqing Lantern Rite mods
   * - **Kirara**
     - GI
     - | CatBox, KonomiyaExpress, 
       | Nekomata
     - Kirara mods
   * - **KiraraBoots**
     - GI
     - | CatBoxWithBoots, 
       | KonomiyaExpressInBoots, 
       | NekomataInBoots, 
       | PussInBoots
     - Kirara in Boots mods
   * - **Klee**
     - GI
     - | DestroyerofWorlds, 
       | DodocoBuddy, 
       | SparkKnight
     - Klee mods
   * - **KleeBlossomingStarlight**
     - GI
     - | DodocoLittleWitchBuddy, 
       | FlandreScarlet, 
       | MagicDestroyerofWorlds, 
       | RedVelvetMage, 
       | ScarletFlandre
     - Klee Summertime mods
   * - **Lisa**
     - GI
     - | CutieLibrarian
     - Lisa mods
   * - **LisaStudent**
     - GI
     - | LisaSumeru, 
       | SumeruLisa, 
       | AkademiyaLisa, 
       | LisaAkademiya
     - Lisa Sumeru mods
   * - **Lumine**
     - GI
     - | FemaleTraveler, Hotaru, TravelerFemale, TravelerGirl
     - Lumine mods
   * - **LumineHeaven**
     - GI
     - | AsHeavenAndEarthAreMadeAnewLumine, HeavenLumine, LumineAsHeavenAndEarthAreMadeAnew, LumineSkin, TravelerGirlHeaven
     - Lumine As Heaven and Earth Are Made Anew mods
   * - **Mona**
     - GI
     - | BigHat, NoMora
     - Mona mods
   * - **MonaCN**
     - GI
     - | NoMoraCN, BigHatCN
     - Mona Chinese mods
   * - **Neuvillette**
     - GI
     - | ChiefJustice, HydroDragon, HydroSovereign, Iudex, Neuv
     - Neuvillette mods
   * - **NeuvilletteMelusent**
     - GI
     - | MelusentGiftNeuvillette, MelusentNeuv, MelusentNeuvillette, NeuvMelusent, NeuvilletteMelusentGift
     - Neuvillette Melusent Gift mods
   * - **Nilou**
     - GI
     - | BloomGirl, Dancer, Morgiana
     - Nilou mods
   * - **NilouBreeze**
     - GI
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
     - Nilou Forest Fairy mods
   * - **Ningguang**
     - GI
     - | GeoMommy,
       | SugarMommy
     - Ningguang mods
   * - **NingguangOrchid**
     - GI
     - | NingguangLanternRite,
       | LanternRiteNingguang,
       | GeoMommyOrchid,
       | SugarMommyOrchid,
       | GeoMommyLaternRite,
       | SugarMommyLanternRite,
       | LaternRiteGeoMommy,
       | LanternRiteSugarMommy
     - Ningguang Lantern Rite mods
   * - **Raiden**
     - GI
     - | Ei, CrydenShogun, SmolEi, 
       | RaidenEi, Shogun, Shotgun, 
       | RaidenShotgun,
       | Cryden, RaidenShogun
     - Raiden mods
   * - **Rosaria**
     - GI
     - | GothGirl
     - Rosaria mods
   * - **RosariaCN**
     - GI
     - | GothGirlCN
     - Rosaria Chinese mods
   * - **Sanhua**
     - WuWa
     - | JinhsiBodyguard
     - Sanhua mods
   * - **SanhuaExorcist**
     - WuWa
     - | ExorcistJinhsiBodyguard, 
       | ExorcistSanhua, 
       | JinhsiBodyguardExorcist, 
       | JinhsiBodyguardMoonChasing, 
       | MoonChasingJinhsiBodyguard, 
       | MoonChasingSanhua, 
       | SanhuaMoonChasing, 
       | SanhuaSkin1
     - Sanhua Moon Chasing skin mods
   * - **Shenhe**
     - GI
     - | YelansBestie,
       | RedRopes
     - Shenhe mods
   * - **ShenheFrostFlower**
     - GI
     - | ShenheLanternRite,
       | LanternRiteShenhe,
       | YelansBestieFrostFlower,
       | YelansBestieLanternRite,
       | LanternRiteYelansBestie,
       | RedRopesFrostFlower,
       | RedRopesLanternRite,
       | LanternRiteRedRopes
     - Shenhe Lantern Rite mods
   * - **Xiangling**
     - GI
     - | CookingFanatic,
       | HeadChefoftheWanminRestaurant,
       | ChefMaosDaughter,
       | GuobasBuddy
     - Xiangling mods
   * - **XianglingCheer**
     - GI
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
     - Xiangling Lantern Rite mods
   * - **Xingqiu**
     - GI
     - | Bookworm, ChongyunsBestie, 
       | GuhuaGeek, 
       | SecondSonofTheFeiyunCommerceGuild
     - Xingqiu mods
   * - **XingqiuBamboo**
     - GI
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
     - Xingqiu Lantern Rite mods
   * - **Yaoyao**
     - GI
     - | BubuPharmacyApprentice,
       | YaoYao,
       | YueguisMaster
     - Yaoyao mods
   * - **YaoyaoBamboo**
     - GI
     - | BambooYaoyao,
       | LanternRiteYaoyao,
       | RainlitBambooReverieYaoyao,
       | RainlitYaoyao,
       | YaoyaoLanternRite,
       | YaoyaoRainlit,
       | YaoyaoRainlitBambooReverie
     - Yaoyao Rainlit Bamboo Reverie mods
   * - **Yelan**
     - GI
     - | ShenhesBestie,
       | TsaritsaJacketStealer
     - Yelan mods
   * - **YelanTranquil**
     - GI
     - | YelanTranquilBanquet,
       | TranquilBanquetYelan,
       | YelanSummer,
       | SummerYelan,
       | ShenhesSummerBestie,
       | TsaritsaJacketStealerButDoesntNeedItSinceItIsSummer
     - Yelan Tranquil Banquet mods


:raw-html:`<br />`
:raw-html:`<br />`

Game Types
----------

Below are the supported types of games

:raw-html:`<br />`

.. note::
    The names/aliases for the game types are not case sensitive

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Name
     - Aliases
   * - **GI**
     - | Genshin,
       | GenshinImpact
   * - **WuWa**
     - | WutheringWaves


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
   * - **Normal**
     - | Only perform file downloads at places in a .ini file where a resource is missing
       |
       | This is the default when the option is not specified
   * - **Always**
     - Will always perform file downloads for every mod, if possible, using pessimistic assumptions

.. _section: https://en.wikipedia.org/wiki/INI_file#Sections
.. _sections: https://en.wikipedia.org/wiki/INI_file#Sections