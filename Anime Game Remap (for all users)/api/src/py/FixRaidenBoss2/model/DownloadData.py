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
from typing import Optional, List, Tuple
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

    refToSection: : class:`bool`
        Whether to add the download reference to only the top of some `section`_ or to add the download reference to all
        the needed :class:`IfContentPart`s of the `section`_ `<br />` :raw-html:`<br />`

        **Default**: ``False``

    downloadRefKVPs: Optional[List[Tuple[:class:`str`, :class:`str`]]]
        Any additional `KVPs`_ to add after the download reference of some :class:`IfContentPart` `<br />` :raw-html:`<br />`

        **Default**: ``None``

    resourceKVPs: Optional[List[Tuple[:class:`str`, :class:`str`]]]
        Any additional `KVPs`_ to add before the download filepath to some :class:`IfContentPart` `<br />` :raw-html:`<br />`

        **Default**: ``None``

    Arguments
    ---------
    name: :class:`str`
        The name of the download resource in the .ini file

    download: :class:`FileDownload`
        The file download to initiate

    refToSection: : class:`bool`
        Whether to add the download reference to only the top of some `section`_ or to add the download reference to all
        the needed :class:`IfContentPart`s of the `section`_

    downloadRefKVPs: List[Tuple[:class:`str`, :class:`str`]]
        Any additional `KVPs`_ to add after the download reference of some :class:`IfContentPart`

    resourceKVPs: List[Tuple[:class:`str`, :class:`str`]]
        Any additional `KVPs`_ to add before the download filepath to some :class:`IfContentPart`
    """

    def __init__(self, name: str, download: FileDownload, refToSection: bool = False, downloadRefKVPs: Optional[List[Tuple[str, str]]] = None, 
                 resourceKVPs: Optional[List[Tuple[str, str]]] = None):
        self.name = name
        self.download = download
        self.refToSection = refToSection
        self.downloadRefKVPs = {} if (downloadRefKVPs is None) else downloadRefKVPs
        self.resourceKVPs = {} if (resourceKVPs is None) else resourceKVPs

    def addToPart(self, part: IfContentPart, key: str, val: str):
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
        """

        part.addKVP(key, val)

        if (self.downloadRefKVPs):
            part.addKVPs(self.downloadRefKVPs)

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

        ifTemplate.addKVPToFront(key, val)

    def createResSection(self, sectionName: str) -> IfTemplate:
        """
        Create the `section`_ containing the download resource

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the section

        Returns
        -------
        :class:`IfTemplate`
            The created `section`_
        """

        contentPart = IfContentPart({"filename": [(0, self.download.filename)]}, 0)
        if (self.resourceKVPs):
            contentPart.addKVPsToFront(self.resourceKVPs)

        return IfTemplate([contentPart], name = sectionName)


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

    vertexCount: :class:`int`
        The number of vertices in the model (.vb file or its .buf counterparts)

        :raw-html:`<br />`

        .. tip::
            From :class:`BlendFile`, we know that a line in a Blend.buf file for a character usually contains 32 bytes.

            Since a line in a ``Blend.buf`` file usually references a single vertex,
            You can calculate the vertex count by doing:

            .. code-block::

                (# of bytes in the Blend.buf file) / 32 = vertexCount

    downloadRefKVPs: Optional[List[Tuple[:class:`str`, :class:`str`]]]
        Any additional `KVPs`_ to add after the download reference of some :class:`IfContentPart` `<br />` :raw-html:`<br />`

        **Default**: ``None``

    resourceKVPs: Optional[List[Tuple[:class:`str`, :class:`str`]]]
        Any additional `KVPs`_ to add before the download filepath to some :class:`IfContentPart` `<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, name: str, download: FileDownload, vertexCount: int, 
                 downloadRefKVPs: Optional[List[Tuple[str, str]]] = None,
                 resourceKVPs: Optional[List[Tuple[str, str]]] = None):
        super().__init__(name, download, downloadRefKVPs = downloadRefKVPs, resourceKVPs = resourceKVPs)
        self.vertexCount = vertexCount

    def addToPart(self, part: IfContentPart, key: str, val: str):
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
        """

        super().addToPart(part, key, val)
        part.addKVP(IniKeywords.Handling.value, "skip")
        part.addKVP(IniKeywords.Draw.value, f"{self.vertexCount},0")

        if (self.downloadRefKVPs):
            part.addKVPs(self.downloadRefKVPs)
##### EndScript