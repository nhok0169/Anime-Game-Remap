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
   * - -h, --help   
     - show this help message and exit
   * - -s str, --src str
     - | The path to the Raiden mod folder. If this option is not specified, then will
       | use the current directory as the mod folder.
   * - -d, --deleteBackup
     - deletes backup copies of the original .ini files
   * - -f, --fixOnly
     - only fixes the mod without cleaning any previous runs of the script
   * - -r, --revert 
     - reverts back previous runs of the script
   * - -l str, --log str
     - | The folder location to log the printed out text into a seperate .txt file.
       | If this option is not specified, then will not log the printed out text.
   * - -a, --all
     - | Parses all \*.ini files that the program encounters. 
       | This option supersedes the `--types` option.
   * - -n str, --defaultType
     - | The default mod type to use if the \*.ini file belongs to some unknown mod
       | If the --all is set to True, then this argument will be 'raiden'.
       |
       | Otherwise, if this value is not specified, 
       | then any mods with unknown types will be skipped
       | 
       | See below for the different names/aliases of the supported types of mods.
   * - -t str, --types str
     - | Parses \*.ini files that the program encounters for only specific types of mods.
       | If the `--all` option has been specified, this option has no effect.
       | By default, if this option is not specified, 
       | will parse the \*.ini files for all the supported types of mods.
       |
       | Please specify the types of mods using the the mod type's name or alias, 
       | then seperate each name/alias with a comma(,)
       |    *eg. raiden,arlecchino,ayaya*
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
   * - **Mona**
     - | BigHat, NoMora
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(Mona)((?!(RemapBlend|CN)).)*Blend.*\s*\]
   * - **MonaCN**
     - | NoMoraCN, BigHatCN
     - | check if the .ini file contains a section matching the regex,
       | ^\s*\[\s*TextureOverride.*(MonaCN)((?!RemapBlend).)*Blend.*\s*\]
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

