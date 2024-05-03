Command Options
===============


.. list-table::
   :widths: 25 50
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
   * - -l, --log
     - Logs the printed out log into a seperate .txt file
   * - -a, --all
     - | Parses all \*.ini files that the program encounters instead of only parsing
       | \*.ini files that have the section [TextureOverrideRaidenShogunBlend]