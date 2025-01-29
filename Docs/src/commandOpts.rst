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
   * - -ho, --hideOriginal
     - Show only the mod on the remapped character and do not show the mod on the original character
   * - -l str, -\-log str
     - | The folder location to log the printed out text into a seperate .txt file.
       | If this option is not specified, then will not log the printed out text.
   * - -a, -\-all
     - | Parses all \*.ini files that the program encounters. 
       | This option supersedes the `-\-types` option.
       |
       | Note: Usually, you would also need to specify what particular mod you want 
       | to fix using the `-\-defaultType` option. Otherwise, you will be 
       | defaulted to fixing 'raiden' mods.
   * - -dt str, -\-defaultType str
     - | The default mod type to use if the \*.ini file belongs to some unknown mod
       | If the -\-all is set to True, then this argument will be 'raiden'.
       |
       | Otherwise, if this value is not specified, 
       | then any mods with unknown types will be skipped
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

:raw-html:`<br />`
:raw-html:`<br />`

Mod Types
---------

Below are the supported types of mods

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
       | ^\s*\[\s*TextureOverride.*(Amber)((?!(RemapBlend|CN)).)*Blend.*\s*\]
   * - **AmberCN**
     - | ColleisBestieCN, 
       | BaronBunnyCN
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(AmberCN)((?!RemapBlend).)*Blend.*\s*\]
   * - **Ayaka**
     - | Ayaya, 
       | NewArchonOfEternity, 
       | Yandere
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(Ayaka)((?!(RemapBlend|SpringBloom)).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(AyakaSpringBloom)((?!(RemapBlend)).)*Blend.*\s*\]
   * - **Arlecchino**
     - | Father, Knave,
       | Perrie, Peruere,
       | Harlequin
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Arlecchino)((?!RemapBlend).)*Blend.*\s*\]
   * - **Barbara**
     - | Idol, Healer
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Barbara)((?!RemapBlend|Summertime).)*Blend.*\s*\]
   * - **BarbaraSummertime**
     - | IdolSummertime,
       | HealerSummertime,
       | BarbaraBikini
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(BarbaraSummertime)((?!RemapBlend).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(CherryHuTao|HuTaoCherry)((?!RemapBlend).)*Blend.*\s*\]
   * - **Diluc**
     - | AngelShareOwner, 
       | DarkNightBlaze, 
       | DawnWineryMaster, 
       | KaeyasBrother
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(Diluc)((?!RemapBlend|Flamme).)*Blend.*\s*\]
   * - **DilucFlamme**
     - | DarkNightHero, 
       | RedDeadOfTheNight
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(DilucFlamme)((?!RemapBlend).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(Fischl)((?!RemapBlend|Highness).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(FischlHighness)((?!RemapBlend).)*Blend.*\s*\]
   * - **Ganyu**
     - | Cocogoat
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Ganyu)((?!(RemapBlend|Twilight)).)*Blend.*\s*\]
   * - **GanyuTwilight**
     - | GanyuLanternRite,
       | LanternRiteGanyu,
       | CocogoatTwilight,
       | CocogoatLanternRite,
       | LanternRiteCocogoat
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(GanyuTwilight)((?!(RemapBlend)).)*Blend.*\s*\]
   * - **HuTao**
     - | 77thDirectoroftheWangshengFuneralParlor, 
       | QiqiKidnapper
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride((?!Cherry).)*(HuTao)((?!RemapBlend|Cherry).)*Blend.*\s*\]
   * - **Jean**
     - | KleesBabySitter, 
       | ActingGrandMaster
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Jean)((?!(RemapBlend|CN|Sea)).)*Blend.*\s*\]
   * - **JeanCN**
     - | KleesBabySitterCN, 
       | ActingGrandMasterCN
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(JeanCN)((?!RemapBlend|Sea).)*Blend.*\s*\]
   * - **JeanSea**
     - | ActingGrandMasterSea,
       | KleesBabySitterSea
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(JeanSea)((?!RemapBlend|CN).)*Blend.*\s*\]
   * - **Keqing**
     - | Kequeen,
       | ZhongliSimp
       | MoraxSimp
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Keqing)((?!(RemapBlend|Opulent)).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(KeqingOpulent)((?!RemapBlend).)*Blend.*\s*\]
   * - **Kirara**
     - | CatBox, KonomiyaExpress, 
       | Nekomata
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(Kirara)((?!RemapBlend|Boots).)*Blend.*\s*\]
   * - **KiraraBoots**
     - | CatBoxWithBoots, 
       | KonomiyaExpressInBoots, 
       | NekomataInBoots, 
       | PussInBoots
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(KiraraBoots)((?!RemapBlend).)*Blend.*\s*\]
   * - **Klee**
     - | DestroyerofWorlds, 
       | DodocoBuddy, 
       | SparkKnight
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(Klee)((?!RemapBlend|BlossomingStarlight).)*Blend.*\s*\]
   * - **KleeBlossomingStarlight**
     - | DodocoLittleWitchBuddy, 
       | FlandreScarlet, 
       | MagicDestroyerofWorlds, 
       | RedVelvetMage
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(KleeBlossomingStarlight)((?!RemapBlend).)*Blend.*\s*\]
   * - **Mona**
     - | BigHat, NoMora
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Mona)((?!(RemapBlend|CN)).)*Blend.*\s*\]
   * - **MonaCN**
     - | NoMoraCN, BigHatCN
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(MonaCN)((?!RemapBlend).)*Blend.*\s*\]
   * - **Nilou**
     - | BloomGirl, Dancer, Morgiana
     - | check if the .ini file contains a section matching the regex, 
       | ^\s*\[\s*TextureOverride.*(Nilou)((?!(RemapBlend|Breeze)).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(NilouBreeze)((?!(RemapBlend)).)*Blend.*\s*\]
   * - **Ningguang**
     - | GeoMommy,
       | SugarMommy
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Ningguang)((?!(RemapBlend|Orchid)).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(NingguangOrchid)((?!RemapBlend).)*Blend.*\s*\]
   * - **Raiden**
     - | Ei, CrydenShogun, SmolEi, 
       | RaidenEi, Shogun, Shotgun, 
       | RaidenShotgun,
       | Cryden, RaidenShogun
     - | check if the .ini file contains a section matching the regex,
       | `^\\s\*\\[\\s\*TextureOverride.\*(Raiden|Shogun)((?!RemapBlend).)\*Blend.\*\\s*\\]`
   * - **Rosaria**
     - | GothGirl
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Rosaria)((?!(RemapBlend|CN)).)*Blend.*\s*\]
   * - **RosariaCN**
     - | GothGirlCN
     - |  check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(RosariaCN)((?!RemapBlend).)*Blend.*\s*\]
   * - **Shenhe**
     - | YelansBestie,
       | RedRopes
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(ShenheFrostFlower)((?!RemapBlend).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(ShenheFrostFlower)((?!RemapBlend).)*Blend.*\s*\]
   * - **Xingqiu**
     - | Bookworm, ChongyunsBestie, 
       | GuhuaGeek, 
       | SecondSonofTheFeiyunCommerceGuild
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Xingqiu)((?!RemapBlend|Bamboo).)*Blend.*\s*\]
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
       | ^\s*\[\s*TextureOverride.*(XingqiuBamboo)((?!RemapBlend).)*Blend.*\s*\]

