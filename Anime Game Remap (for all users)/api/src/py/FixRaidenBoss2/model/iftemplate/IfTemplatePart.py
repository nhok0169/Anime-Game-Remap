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


##### Script
IfTemplatePartAutoId = 0


class IfTemplatePart():
    """
    Base class for some part in an :class:`IfTemplates`    
    """

    def __init__(self):
        self._id = self._generateId()

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