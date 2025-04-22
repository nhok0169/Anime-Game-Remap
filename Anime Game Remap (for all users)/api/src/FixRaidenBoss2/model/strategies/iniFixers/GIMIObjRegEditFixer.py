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
from typing import Optional, List
##### EndExtImports

##### LocalImports
from .GIMIObjSplitFixer import GIMIObjSplitFixer
from ..iniParsers.GIMIObjParser import GIMIObjParser
from .regEditFilters.BaseRegEditFilter import BaseRegEditFilter
##### EndLocalImports


##### Script
class GIMIObjRegEditFixer(GIMIObjSplitFixer):
    """
    This class inherits from :class:`GIMIObjSplitFixer`

    Fixes a .ini file used by a GIMI related importer where particular mod objects (head, body, dress, etc...) in the mod to remap
    needs to have their registers remapped or removed

    .. note::
        For the order of how the registers are fixed, please see :class:`GIMIObjReplaceFixer`

    Parameters
    ----------
    parser: :class:`GIMIObjParser`
        The associated parser to retrieve data for the fix

    preRegEditFilters: Optional[List[:class:`BaseRegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart`. 
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`

        Whether these filters reference the mod objects to be fixed of the new mod objects of the fixed mods 
        is determined by :attr:`GIMIObjRegEditFixer.preRegEditOldObj` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    postRegEditFilters: Optional[List[:class:`BaseRegEditFilter`]]
        Filters used to edit the registers of a certain :class:`IfContentPart` for the new mod objects of the fixed mods. 
        Filters are executed based on the order specified in the list. :raw-html:`<br />` :raw-html:`<br />`
        
        .. note::
            These filters are preceded by the filters at :class:`GIMIObjReplaceFixer.preRegEditFilters`

        :raw-html:`<br />`

        **Default**: ``None``

    fileDownloads: Optional[Dict[:class:`str`, Dict[:class:`str`, Tuple[:class:`str`, :class:`FileDownload`]]]]
        The files to download if the mod is missing some required files :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the mod objects
        * The inner keys are the names of the registers
        * The inner values contain:
            
            * The name to the file resource 
            * The corrresponding file download

        eg. :raw-html:`<br />`

        .. code-block::

            {"head": {"ps-t1": ("Diffuse", FileDownload("someServer.com/headDiffuse.dds", "headDiffuse.dds"))}, 
             "body": {"ps-t3": ("ShadowRamp", FileDownload("someServer.com/bodyShadowRamp.dds", "bodyShadowRamp.dds")), "ps-t0": ("Diffuse", FileDownload("someServer.com/bodyDiffuse.dds", "bodyDiffuse.dds"))}, 
             "dress": {"ps-t0": ("Diffuse", FileDownload("someServer.com/dressDiffuse.dds", "dressDiffuse.dds"))}} 
        
        :raw-html:`<br />` :raw-html:`<br />`


        **Default**: ``None``
    """

    def __init__(self, parser: GIMIObjParser, preRegEditFilters: Optional[List[BaseRegEditFilter]] = None, 
                 postRegEditFilters: Optional[List[BaseRegEditFilter]] = None):
        super().__init__(parser, {}, preRegEditFilters = preRegEditFilters, postRegEditFilters = postRegEditFilters)

        parserObjs = sorted(self._parser.objs)
        for obj in parserObjs:
            if (obj not in self.objs):
                self.objs[obj] = [obj] 
##### EndScript