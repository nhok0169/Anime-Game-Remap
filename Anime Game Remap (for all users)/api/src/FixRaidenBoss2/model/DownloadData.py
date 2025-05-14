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
from typing import Dict, Optional
##### EndExtImports

##### LocalImports
from ..constants.IniConsts import IniKeywords
from ..tools.files.FileDownload import FileDownload
from .iftemplate.IfContentPart import IfContentPart
from .iftemplate.IfTemplate import IfTemplate
##### EndLocalImports


##### Script
class DownloadData():
    """
    Download data used by the .ini files

    Parameters
    ----------
    name: :class:`str`
        The name of the download resource in the .ini file

    download: :class:`FileDownload`
        The file download to initiate

    resourceKeys: Optional[Dict[:class:`str`, :class:`str`]]
        Any additional `KVPs`_ to add to the resource `section`_ of the download resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Arguments
    ---------
    name: :class:`str`
        The name of the download resource in the .ini file

    download: :class:`FileDownload`
        The file download to initiate

    resourceKeys: Dict[:class:`str`, :class:`str`]
        Any additional `KVPs`_ to add to the resource `section`_ of the download resource
    """

    def __init__(self, name: str, download: FileDownload, resourceKeys: Optional[Dict[str, str]] = None):
        self.name = name
        self.download = download
        self.resourceKeys = {} if (resourceKeys is None) else resourceKeys

    def addToPart(self, part: IfContentPart, key: str, val: str, **kwargs):
        """
        Adds a reference to the download into 'part'

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part to add the reference

        key: :class:`str`
            The key to the download reference `KVP`_

        val: :class:`str`
            The value to the download reference `KVP`_

        **kwargs:
            Any additional keyword arguments for this method
        """

        part.addKVP(key, val)

    def addToSection(self, ifTemplate: IfTemplate, key: str, val: str):
        """
        Adds a reference to the download into the 'ifTemplate'

        Parameters
        ----------
        ifTemplate: :class:`IfTemplate`
            The `section`_ to add the reference

        key: :class:`str`
            The key to the download reference `KVP`_

        val: :class:`str`
            The value to the download reference `KVP`_
        """

        ifTemplateParts = ifTemplate.parts

        if (not ifTemplateParts or not isinstance(ifTemplateParts[0], IfContentPart)):
            ifTemplateParts.insert(0, IfContentPart({key: [(0, val)]}, 0))
        else:
            ifTemplateParts[0].addKVPToFront(key, val)


class BlendDownloadData(DownloadData):
    """
    This class inherits from :class:`DownloadData`

    Blend.buf download data used by the .ini files

    Parameters
    ----------
    name: :class:`str`
        The name of the download resource in the .ini file

    download: :class:`FileDownload`
        The file download to initiate

    resourceKeys: Optional[Dict[:class:`str`, :class:`str`]]
        Any additional `KVPs`_ to add to the resource `section`_ of the download resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def addToPart(self, part: IfContentPart, key: str, val: str, vertexCount: int = 0, **kwargs):
        """
        Adds a reference to the download into 'part'

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part to add the reference

        key: :class:`str`
            The key to the download reference `KVP`_

        val: :class:`str`
            The value to the download reference `KVP`_

        vertexCount: :class:`int`
            The number of vertices in the model (.vb file or its .buf counterparts)

            :raw-html:`<br />`

            .. tip::
                From :class:`BlendFile`, we know that a line in a Blend.buf file for a character usually contains 32 bytes.

                Since a line in a ``Blend.buf`` file usually references a single vertex,
                You can calculate the vertex count by doing:

                .. code-block::

                    (# of bytes in the Blend.buf file) / 32 = vertexCount

        **kwargs:
            Any additional keyword arguments for this method
        """

        super().addToPart(part, key, val)
        part.addKVP(IniKeywords.Handling.value, "skip")
        part.addKVP(IniKeywords.Draw.value, f"{vertexCount},0")
##### EndScript