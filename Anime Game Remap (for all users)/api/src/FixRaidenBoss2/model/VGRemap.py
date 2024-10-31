##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### ExtImports
from typing import Dict
##### EndExtImports


##### Script
class VGRemap():
    """
    Class for handling the vertex group remaps for mods

    Parameters
    ----------
    vgRemap: Dict[:class:`int`, :class:`int`] 
        The vertex group remap from one type of mod to another
    """

    def __init__(self, vgRemap: Dict[int, int]):
        self._maxIndex = 0
        self.remap = vgRemap

    @property
    def remap(self):
        """
        The vertex group remap

        :getter: Retrieves the remap
        :setter: Sets a new remap
        :type: Dict[:class:`int`, :class:`int`]
        """

        return self._remap

    @remap.setter
    def remap(self, newVgRemap: Dict[int, int]):
        self._remap = newVgRemap
        if (self._remap):
            self._maxIndex = max(list(self._remap.keys()))
        else:
            self._maxIndex = None

    @property
    def maxIndex(self):
        """
        The maximum index in the vertex group remap

        :getter: Retrieves the max index
        :type: :class:`int`
        """

        return self._maxIndex
##### EndScript
