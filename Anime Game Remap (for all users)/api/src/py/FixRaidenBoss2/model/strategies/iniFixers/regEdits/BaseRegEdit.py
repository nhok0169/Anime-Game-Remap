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
from typing import TYPE_CHECKING, Optional
##### EndExtImports

##### CppLocalImports
from .....core import Ranges
from .....core import IfContentPart
##### EndCppLocalImports

##### LocalImports
from ..BaseIniGraphPartEdit import BaseIniGraphPartEdit

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class BaseRegEdit(BaseIniGraphPartEdit):
    """
    This class inherits from :class:`BaseIniGraphPartEdit`

    Base class for a filter that edits some registers within an :class:`IfContentPart`
    """

    def editFromIni(self, part: IfContentPart, sectionName: str, ini: "IniFile", modType: "ModType", modName: str = "", partRanges: Optional[Ranges] = None) -> IfContentPart:
        """
        Edits the registers for the current :class:`IfContentPart` with state info from 'ini'

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the :class:`IfTemplate` that is being editted

        ini: :class:`IniFile`
            The associated .ini file

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        partRanges: Optional[:class:`Ranges`]
            The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns 
        -------
        :class:`IfContentPart`
            The resultant part of the :class:`IfTemplate` that got its registers editted
        """

        return self.edit(part, sectionName, modType, modName = modName, partRanges = partRanges)

    def edit(self, part: IfContentPart, sectionName: str, modType: "ModType", modName: str = "", partRanges: Optional[Ranges] = None) -> IfContentPart:
        """
        Edits the registers for the current :class:`IfContentPart`

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the :class:`IfTemplate` that is being editted

        sectionName: :class:`str`
            The name of the `section`_ that is being editted

        modType: :class:`ModType`
            The type of mod to fix

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        partRanges: Optional[:class:`Ranges`]
            The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns 
        -------
        :class:`IfContentPart`
            The resultant part of the :class:`IfTemplate` that got its registers editted
        """

        pass
##### EndScript