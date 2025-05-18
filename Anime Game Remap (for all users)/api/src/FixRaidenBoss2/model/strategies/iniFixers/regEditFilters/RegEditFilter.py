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
from .BaseRegEditFilter import BaseRegEditFilter
from ....iftemplate.IfContentPart import IfContentPart

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ..BaseIniFixer import BaseIniFixer
    from ..GIMIObjReplaceFixer import GIMIObjReplaceFixer
##### EndLocalImports


##### Script
class RegEditFilter(BaseRegEditFilter):
    """
    This class inherits from :class:`BaseRegEditFilter`

    class for editting registers within an :class:`IfContentPart`
    """

    def clear(self):
        """
        Clears any saved state within this class
        """

        pass

    def edit(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer") -> IfContentPart:
        """
        Edits the registers for the current :class:`IfContentPart`. Includes boilerplate of clearing all saved states and handling texture adds/edits

        .. note::
            If you are inheriting this class, you probably want to override the :meth:`RegEditFilter._editReg` method instead

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
        result = self._editReg(part, modType, fixModName, obj, sectionName, fixer)
        self.handleTexAdd(part, modType, fixModName, obj, sectionName, fixer)
        self.handleTexEdit(part, modType, fixModName, obj, sectionName, fixer)
        return result

    def _editReg(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "BaseIniFixer") -> IfContentPart:
        """
        The main function to edit the registers for the current :class:`IfContentPart`

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

        fixer: :class:`BaseIniFixer`
            The fixer that is editting the registers

        Returns 
        -------
        :class:`IfContentPart`
            The resultant part of the :class:`IfTemplate` that got its registers editted
        """

        pass

    def handleTexAdd(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        """
        Does any post-processing on the added textures of the corresponding :class:`GIMIObjReplaceFixer`

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
        """

        pass

    def handleTexEdit(self, part: IfContentPart, modType: "ModType", fixModName: str, obj: str, sectionName: str, fixer: "GIMIObjReplaceFixer"):
        """
        Does any post-processing on the added textures of the corresponding :class:`GIMIObjReplaceFixer`

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
        """

        pass
##### EndScript
