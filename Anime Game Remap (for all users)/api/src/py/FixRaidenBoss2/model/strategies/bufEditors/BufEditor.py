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
from typing import List, Callable, Any, Optional, Dict, Union
##### EndExtImports

##### LocalImports
from ...files.BufFile import BufFile
from .BaseBufEditor import BaseBufEditor
##### EndLocalImports


##### Script
class BufEditor(BaseBufEditor):
    """
    This class inherits from :class:`BaseBufEditor`

    Class to edit some .buf file

    Parameters
    ----------
    filters: Optional[List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]]
        The filters used to edit the data for each line in the .buf file :raw-html:`<br />` :raw-html:`<br />`

        The filters take in the following arguments:

        #. The data for a particular line
        #. The starting byte index of the line that is read
        #. The line index being processed
        #. The size of each line :raw-html:`<br />` :raw-html:`<br />`

        The output of the filters is the resultant data that consists where the keys are the names of the elements within a line
        in the .buf file and the values are the resultant data for each element in the line :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, filters: Optional[List[Callable[[Dict[str, List[Any]], int, int, int], Dict[str, List[Any]]]]] = None):
        super().__init__()
        self.filters = [] if (filters is None) else filters

    def fix(self, bufFile: BufFile, fixedBufFile: str) -> Union[Optional[str], bytearray]:
        return bufFile.fix(fixedFile = fixedBufFile, filters = self.filters)
##### EndScript
