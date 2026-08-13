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
from typing import List, Dict, Tuple, Optional
##### EndExtImports


##### Script
IfTemplatePartAutoId = 0


class IfTemplatePart():
    """
    Base class for some part in an :class:`IfTemplates`

    Parameters
    ----------
    id: Optional[:class:`int`]
        The id for the part. If this parameter is ``None``, will generate a new id for the part. :raw-html:`<br />` **OR** :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, id: Optional[int] = None):
        self._id = self._generateId() if (id is None) else id

    def toStr(self, *args, **kwargs) -> str:
        """
        Retrieves the part as a string

        Returns
        -------
        :class:`str`
            The string representation of the part        
        """

        pass

    def _generateId(self) -> int:
        global IfTemplatePartAutoId

        result = IfTemplatePartAutoId
        IfTemplatePartAutoId += 1
        return result
    
    @property
    def id(self) -> int:
        """
        The id for the part

        :getter: Retrieves the id
        :type: :class:`int`
        """

        return self._id
##### EndScript