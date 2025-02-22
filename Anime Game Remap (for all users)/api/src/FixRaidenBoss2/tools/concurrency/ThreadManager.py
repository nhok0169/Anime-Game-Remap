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
from threading import Thread
from typing import Optional
##### EndExtImports

##### LocalImports
from .ConcurrentManager import ConcurrentManager
##### EndLocalImports


##### Script
class ThreadManager(ConcurrentManager[Thread]):
    """
    Class to manage running many threads

    Paramaters
    ----------
    jobNo: Optional[:class:`int`]
        The number of processes to run at once :raw-html:`<br />` :raw-html:`<br />`

        If this argument is ``None``, will run all the processes at once :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, jobNo: Optional[int] = None):
        super().__init__(Thread, jobNo = jobNo)
##### EndScript
