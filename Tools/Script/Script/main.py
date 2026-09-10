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
import os
from typing import Optional
##### EndExtImports

##### LocalImports
from .apiRefs.ApiRefBuilder import ApiRefBuilder
from .apiRefs.BaseApiRef import BaseApiRef
from .controller.CommandBuilder import CommandBuilder
##### EndLocalImports


##### Script
def remapMain(apiRef: Optional[BaseApiRef] = None):
    """
    Runs the script

    :raw-html:`<br />`

    The script does not do the remapping itself -- it makes the API available and then hands the
    command line straight to it

    Parameters
    ----------
    apiRef: Optional[:class:`BaseApiRef`]
        How to reach the API :raw-html:`<br />` :raw-html:`<br />`

        When this is ``None``, whichever reference this script was compiled for is used. Running
        this out of the repo rather than from a compiled script is the case that passes one in,
        since the compiled-in values do not exist yet :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    if (apiRef is None):
        apiRef = ApiRefBuilder.build()

    command = CommandBuilder(apiRef.getOptions())

    # this script's own options have to be read before the API is fetched, since one of them decides
    #   whether the API gets downloaded at all. They are handed to the API's command afterwards so
    #   that they still appear in its --help rather than being rejected there as unrecognised.
    args, remainingArgs = command.preParse()

    api = apiRef.load(args)
    api.remapMain(commandSetup = command.addTo)


# Main Driver Code
if __name__ == "__main__":
    remapMain()
##### EndScript