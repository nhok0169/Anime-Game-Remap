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
import argparse
##### EndExtImports

##### LocalImports
from ..tools.TextTools import TextTools
from .CommandFormatter import CommandFormatter
from .enums.CommandOpts import CommandOpts
from .enums.ShortCommandOpts import ShortCommandOpts
from ..constants.FileTypes import FileTypes
from ..constants.FileExt import FileExt
from ..constants.DownloadMode import DownloadMode
##### EndLocalImports


##### Script
# CommandBuilder: Class for building the command
class CommandBuilder():
    def __init__(self):
        self._argParser = argparse.ArgumentParser(description='Ports mods from characters onto their skin counterparts', formatter_class=CommandFormatter)
        self._addArguments()
        self._args = argparse.Namespace()


    def parse(self) -> argparse.Namespace:
        self._args = self._argParser.parse_args()
        self.parseArgs()
        return self._args

    def parseArgs(self):
        if (self._args.types is not None):
            self._args.types = self._args.types.split(",")

        if (self._args.remappedTypes is not None):
            self._args.remappedTypes = self._args.remappedTypes.split(",")

    def _addArguments(self):
        self._argParser.add_argument(ShortCommandOpts.Src.value, CommandOpts.Src.value, action='store', type=str, help="The starting path to run this fix. If this option is not specified, then will run the fix from the current directory.")
        self._argParser.add_argument(ShortCommandOpts.Version.value, CommandOpts.Version.value, action='store', type=str, help="The game version we want the fix to be compatible with. If this option is not specified, then will use the latest game version")
        self._argParser.add_argument(ShortCommandOpts.DeleteBackup.value, CommandOpts.DeleteBackup.value, action='store_true', help=f'deletes backup copies of the original {FileExt.Ini.value} files')
        self._argParser.add_argument(ShortCommandOpts.FixOnly.value, CommandOpts.FixOnly.value, action='store_true', help='only fixes the mod without cleaning any previous runs of the script')
        self._argParser.add_argument(ShortCommandOpts.Revert.value, CommandOpts.Revert.value, action='store_true', help='Undo the previous runs of the script')
        self._argParser.add_argument(ShortCommandOpts.HideOriginal.value, CommandOpts.HideOriginal.value, action = 'store_true', help="Show only the mod on the remapped character and do not show the mod on the original character")
        self._argParser.add_argument(ShortCommandOpts.Log.value, CommandOpts.Log.value, action='store', type=str, help=f'The folder location to log the printed out text into a seperate {FileExt.Txt.value} file. If this option is not specified, then will not log the printed out text.')
        self._argParser.add_argument(ShortCommandOpts.All.value, CommandOpts.All.value, action='store_true', help=f"""Parses all {FileTypes.Ini.value}s that the program encounters. This option supersedes the {CommandOpts.Types.value} option

For {FileTypes.Ini.value} where a mod cannot be identified, usually, you would also need to specify what particular mod the {FileTypes.Ini.value} defaults to using the {CommandOpts.DefaultType.value} option. 
Otherwise, you will be defaulted to fixing 'raiden' mods.""")
        self._argParser.add_argument(ShortCommandOpts.DefaultType.value, CommandOpts.DefaultType.value, action='store', type=str, help=f'''The default mod type to use if the {FileTypes.Ini.value} belongs to some unknown mod.

- If {CommandOpts.ForceType.value} is set to True, this option has not effect                          
- If the {CommandOpts.All.value} is set to True and no values are specified for this option, the default argument for this option is set to 'raiden'
- Otherwise, this option has not effect and any unknown mods will be skipped

See below for the different names/aliases of the supported types of mods.''')
        
        self._argParser.add_argument(ShortCommandOpts.ForceType.value, CommandOpts.ForceType.value, action='store', type=str, help=f"""Forcibly assumes the mod type for all {FileTypes.Ini.value} parsed.

This option supersedes the {CommandOpts.Types.value} option and the {CommandOpts.All.value} option.

See below for the different names/aliases of the supported types of mods.""")

        self._argParser.add_argument('-t', CommandOpts.Types.value, action='store', type=str, help=f'''Parses {FileTypes.Ini.value}s that the program encounters for only specific types of mods. If the {CommandOpts.Types.value} option has been specified, this option has no effect. 
By default, if this option is not specified, will parse the {FileTypes.Ini.value}s for all the supported types of mods. 

Please specify the types of mods using the the mod type's name or alias, then seperate each name/alias with a comma(,)
eg. raiden,arlecchino,ayaya

See below for the different names/aliases of the supported types of mods.''')

        self._argParser.add_argument(ShortCommandOpts.FixedTypes.value, CommandOpts.FixedTypes.value, action='store', type=str, help=f"""From all the mods to fix, specified by the {CommandOpts.Types.value} option, will specifically remap those mods to the mods specified by this option. 
For a mod specified by the {CommandOpts.Types.value} option, if none of its corresponding remapped mods are specified by this option, then the mod specified by the {CommandOpts.Types.value} option will be remapped to all its corresponding mods.
 
-------------------
eg.

If this program was ran with the following options:
{CommandOpts.Types.value} kequeen,jean
{CommandOpts.FixedTypes.value} jeanSea

the program will do the following remap:
keqing --> keqingOpulent
Jean --> JeanSea

Note that Jean will not remap to JeanCN
-------------------


By default, if this option is not specified, will remap all the mods specified in {CommandOpts.Types.value} to their corresponding remapped mods. 

Please specify the types of mods using the the mod type's name or alias, then seperate each name/alias with a comma(,)
eg. raiden,arlecchino,ayaya

See below for the different names/aliases of the supported types of mods.""")

        allDownloadModes = list(map(lambda mode: f"\n- {TextTools.capitalize(mode.value)}", DownloadMode))
        allDownloadModes = "".join(allDownloadModes)

        hardTexDrivenStr = TextTools.capitalize(DownloadMode.HardTexDriven.value)
        self._argParser.add_argument(ShortCommandOpts.Download.value, CommandOpts.Download.value, action = 'store', type=str, help=f"""The download mode to handle file downloads need. Below is a condensed list of all the available download modes. By default, '{hardTexDrivenStr}' is selected
For more info on the download modes, please visit the link below:
https://anime-game-remap.readthedocs.io/en/latest/commandOpts.html#download-modes
{allDownloadModes}
""")
        self._argParser.add_argument(ShortCommandOpts.Proxy.value, CommandOpts.Proxy.value, action='store', type=str, help="The link to the proxy server for those whose internet access must go through a proxy. The software will make all internet network requests through this proxy")

    def addEpilog(self, epilog: str):
        self._argParser.epilog = epilog
##### EndScript