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
from typing import TYPE_CHECKING
##### EndExtImports

##### LocalImports
from ....iftemplate.IfContentPart import IfContentPart

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ..GIMIObjReplaceFixer import GIMIObjReplaceFixer
##### EndLocalImports


##### Script
class BaseRegEditFilter():
    """
    Base class for editting registers within an :class:`IfContentPart`
    """

    def clear(self):
        """
        Clears any saved state within this class
        """

        pass

    def edit(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer") -> IfContentPart:
        """
        Edits the registers for the current :class:`IfContentPart`

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the :class:`IfTemplate` that is being editted

        modType: :class:`ModType`
            The type of mod that is being fix from

        fixModName: :class:`str`
            The name of the mod to fix to

        obj: :class:`str`
            The name of the mod object being fixed

        fixer: :class:`GIMIObjReplaceFixer`
            The fixer that is editting the registers

        Returns 
        -------
        :class:`IfContentPart`
            The resultant part of the :class:`IfTemplate` that got its registers editted
        """

        self.clear()
##### EndScript
