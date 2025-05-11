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
import re
from functools import cmp_to_key
from typing import TYPE_CHECKING, Set, Optional, Callable, Dict, List, Any, Tuple
##### EndExtImports

##### LocalImports
from ....constants.IniConsts import IniKeywords
from ....constants.DownloadMode import DownloadMode
from .BaseIniParser import BaseIniParser
from ....constants.IniConsts import IniKeywords
from ....tools.TextTools import TextTools
from ....tools.files.FileDownload import FileDownload
from ...IniSectionGraph import IniSectionGraph
from ...iniresources.IniFixResourceModel import IniFixResourceModel
from ...iftemplate.IfContentPart import IfContentPart
from ...iftemplate.IfTemplate import IfTemplate

if (TYPE_CHECKING):
    from ...files.IniFile import IniFile
##### EndLocalImports


##### Script
class GIMIParser(BaseIniParser):
    """
    This class inherits from :class:`BaseIniParser`

    Parses a .ini file used by a GIMI related importer

    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to parse

    bufDownloads: Optional[Dict[:class:`str`, Dict[:class:`str`, Tuple[:class:`str`, :class:`FileDownload`, Dict[:class:`str`, :class:`str`]]]]]
        The .buf files to download if the mod is missing some required .buf files :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the type of buffer. The available names are: :attr:`IniKeywords.Blend`.value, :attr:`IniKeywords.Position`.value and :attr:`IniKeywords.Texcoord`.value
        * The inner keys are the names of the registers
        * The inner values contain:
            
            * The name to the file resource 
            * The corrresponding file download
            * Any additional `KVPs`_ to add to the resource `section`_ :raw-html:`<br />` :raw-html:`<br />`

        eg. :raw-html:`<br />`

        .. code-block::

            {IniKeywords.Position.value: {"vb0": ("Position", FileDownload("someServer.com/Position.buf", "Position.buf", {"type": "buffer", "stride": "40"}))}, 
             IniKeywords.Blend.value: {"vb1": ("Blend", FileDownload("someServer.com/Blender.buf", "Blend.buf", {})), "vb999": ("NonExistantBlend", FileDownload("someServer.com/NonExistentBlend.buf", "fakeBlend.buf", {"type": "fakenews"}))}, 
             IniKeywords.Texcoord.value: {"ps-t0": ("Texcoord", FileDownload("someServer.com/texcoord.buf", "textensor.buf", {"model": "resnet50"}))}} 

        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    blendCommandsGraph: :class:`IniSectionGraph`
        All the `sections`_ that use some ``[Resource.*Blend.*]`` section.

    nonBlendHashIndexCommandsGraph: :class:`IniSectionGraph`
        All the `sections`_ that are not used by the ``[Resource.*Blend.*]`` sections and contains the target hashes/indices that need to be replaced

    blendResourceCommandsGraph: :class:`IniSectionGraph`
        All the related `sections`_ to the ``[Resource.*Blend.*]`` `sections`_ that are used by `sections`_ related to the ``[TextureOverride.*Blend.*]`` sections.
        The keys are the name of the `sections`_.

    positionCommandsGraph: :class:`IniSectionGraph`
        All the `sections`_ that use some ``[Resource.*Position.*]`` `sections`_

    positionResourceCommandsGraph: :class:`IniSectionGraph`
        All the related `sections`_ to the ``[Resource.*Position.*]`` `sections`_ that are used by `sections`_ related to the ``[TextureOverride.*Position.*]`` sections.
        The keys are the name of the `sections`_

    _sectionRoots: Dict[:class:`str`, List[:class:`str`]]
        The names of the `sections`_ that are the root nodes to a particular group of `sections`_ in the
        `section`_ caller/callee `graph`_  :raw-html:`<br />` :raw-html:`<br />`

        The keys are the ids for a particular group of `sections`_ and the values are the root `section`_ names for that group

    bufDownloads: Dict[:class:`str`, Dict[:class:`str`, Tuple[:class:`str`, :class:`FileDownload`, Dict[:class:`str`, :class:`str`]]]]
        The .buf files to download if the mod is missing some required .buf files :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the type of buffer. The available names are: :attr:`IniKeywords.Blend`.value, :attr:`IniKeywords.Position`.value and :attr:`IniKeywords.Texcoord`.value
        * The inner keys are the names of the registers
        * The inner values contain:
            
            * The name to the file resource 
            * The corrresponding file download
            * Any additional `KVPs`_ to add to the resource `section`_ of the download
    """

    BlendRootPattern = re.compile(r"^textureoverride((?!remap).)*blend")
    PositionRootPattern = re.compile(r"^textureoverride((?!remap).)*position")

    def __init__(self, iniFile: "IniFile", bufDownloads: Optional[Dict[str, Dict[str, Tuple[str, FileDownload, Dict[str, str]]]]] = None):
        super().__init__(iniFile)
        self.bufDownloads = {} if bufDownloads is None else bufDownloads
        self.blendCommandsGraph = IniSectionGraph(set(), {})
        self.nonBlendHashIndexCommandsGraph = IniSectionGraph(set(), {})
        self.blendResourceCommandsGraph = IniSectionGraph(set(), {})
        self.positionCommandsGraph = IniSectionGraph(set(), {})
        self.positionResourceCommandsGraph = IniSectionGraph(set(), {})
        self._sectionRoots: Dict[str, List[str]] = {}

        self._positionEditModsToFix: Set[str] = set()
        self._bufDownloadParts: Dict [str, Dict[str, Set[IfContentPart]]] = {}
        self._bufReferencedDownloadNames: Dict[str, Dict[str, str]] = {}
        self._fixIdsWithDownloadsAdded: Set[int] = set()

    def clearParseDownloadSearch(self):
        self._bufDownloadParts.clear()

    def clearParseTempData(self):
        self.clearParseDownloadSearch()

    def clear(self):
        super().clear()
        self.blendCommandsGraph.build(newTargetSections = set(), newAllSections = {})
        self.nonBlendHashIndexCommandsGraph.build(newTargetSections = set(), newAllSections = {})
        self.blendResourceCommandsGraph.build(newTargetSections = set(), newAllSections = {})
        self.positionCommandsGraph.build(newTargetSections = set(), newAllSections = {})
        self.positionResourceCommandsGraph.build(newTargetSections = set(), newAllSections = {})
        self._sectionRoots.clear()

        self._positionEditModsToFix.clear()
        self._downloadsAdded = False
        self._bufReferencedDownloadNames.clear()
        self.clearParseTempData()

    # _getCommonMods(): Retrieves the common mods that need to be fixed between all target graphs
    #   that are used for the fix
    def _getCommonMods(self) -> Set[str]:
        modType = self._iniFile.type
        if (modType is None):
            return set()
        
        result = set()
        hashes = modType.hashes
        indices = modType.indices

        graphs = [self.blendCommandsGraph, self.nonBlendHashIndexCommandsGraph, self.blendResourceCommandsGraph]
        for graph in graphs:
            commonMods = graph.getCommonMods(hashes, indices, version = self._iniFile.version)
            if (not result):
                result = commonMods
            elif (commonMods):
                result = result.intersection(commonMods)

        return result
    
    def _setToFix(self) -> Set[str]:
        """
        Sets the names for the types of mods that will used in the fix

        Returns
        -------
        Set[:class:`str`]
            The names of the mods that will be used in the fix        
        """

        commonMods = self._getCommonMods()
        toFix = commonMods
        iniModsToFix = self._iniFile.modsToFix
        if (iniModsToFix):
            toFix = toFix.intersection(iniModsToFix)

        type = self._iniFile.availableType

        if (not toFix and type is not None):
            self._modsToFix = type.getModsToFix()
        elif (not toFix):
            self._modsToFix = commonMods
        else:
            self._modsToFix = toFix

        return self._modsToFix
    
    # _makeRemapNames(): Makes the required names used for the fix
    def _makeRemapNames(self):
        self.blendCommandsGraph.getRemapNames(self._modsToFix)
        self.positionCommandsGraph.getRemapNames(self._modsToFix)
        self.nonBlendHashIndexCommandsGraph.getRemapNames(self._modsToFix)
        self.blendResourceCommandsGraph.getRemapNames(self._modsToFix)

        if (self._positionEditModsToFix):
            self.positionResourceCommandsGraph.getRemapNames(self._positionEditModsToFix)

    def _makeRemapModels(self, result: Dict[str, IniFixResourceModel], resourceGraph: IniSectionGraph, getFixedFile: Optional[Callable[[str], str]] = None,
                         modsToFix: Optional[Set[str]] = None):
        """
        Creates all the data needed for fixing the ``[Resource.*Blend.*]`` `sections`_ in the .ini file

        Parameters
        ----------
        result: Dict[:class:`str`, :class:`IniResourceModel`]
            The result to store the data for fixing the resource `sections`_ :raw-html:`<br />` :raw-html:`<br />`

            The keys are the original names for the resource `sections`_ and the values are the required data for fixing the `sections`_

        resourceGraph: :class:`IniSectionGraph`
            The graph of `sections`_ for the resources

        getFixedFile: Optional[Callable[[:class:`str`], :class:`str`]]
            The function for transforming the file path of a found .*Blend.buf file into a .*RemapBlend.buf file :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :meth:`IniFile.getFixedBlendFile` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        modsToFix: Optional[Set[:class:`str`]]
            The mods to fix :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :attr:`_modsToFix` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (modsToFix is None):
            modsToFix = self._modsToFix

        resourceCommands = resourceGraph.sections
        for resourceKey in resourceCommands:
            resourceIftemplate = resourceCommands[resourceKey]
            remapBlendModel = self._iniFile.makeFixResourceModel(resourceIftemplate, toFix = modsToFix, getFixedFile = getFixedFile)
            result[resourceKey] = remapBlendModel
    
    def _getSectionRoots(self):
        """
        Retrieves the root `sections`_ names that correspond to a either 
        ``TextureOverride.*Blend`` or ``TextureOverride.*Position``
        """

        blendRoots = self._sectionRoots.get(IniKeywords.Blend.value)
        if (blendRoots is None):
            blendRoots = []
            self._sectionRoots[IniKeywords.Blend.value] = blendRoots

        positionRoots = self._sectionRoots.get(IniKeywords.Position.value)
        if (positionRoots is None):
            positionRoots = []
            self._sectionRoots[IniKeywords.Position.value] = positionRoots

        positionRoots = self._sectionRoots.get(IniKeywords.Position.value)
        if (positionRoots is None):
            positionRoots = []
            self._sectionRoots[IniKeywords.Position.value] = positionRoots

        for sectionName in self._iniFile.sectionIfTemplates:
            cleanedSectionName = sectionName.lower()
            if (re.search(self.BlendRootPattern, cleanedSectionName)):
                blendRoots.append(sectionName)
            elif (re.search(self.PositionRootPattern, cleanedSectionName)):
                positionRoots.append(sectionName)

    # _parseElementCommands(roots, commandsGraph): Parses the commands for particular element
    def _parseElementCommands(self, roots: Set[str], commandsGraph: IniSectionGraph):
        commandsGraph.build(newTargetSections = roots, newAllSections = self._iniFile.sectionIfTemplates)

    # _parseElementResources(commandsGraph, resourceGraph, isIfTemplateResource, getIfTemplateResource):
    #   Parses the resources for a particular element
    def _parseElementResources(self, commandsGraph: IniSectionGraph, resourceGraph: IniSectionGraph, isIfTemplateResource: Callable[[IfContentPart], Any], getIfTemplateResource: Callable[[IfContentPart], str]):
        resources = set()
        self._iniFile.getResources(commandsGraph, isIfTemplateResource, getIfTemplateResource, lambda resource, part: resources.update(resource))

        # sort the resources
        resourceCommandLst = list(map(lambda resourceName: (resourceName, self._iniFile.getMergedResourceIndex(resourceName)), resources))
        resourceCommandLst.sort(key = cmp_to_key(self._iniFile.compareResources))
        resourceCommandLst = list(map(lambda resourceTuple: resourceTuple[0], resourceCommandLst))

        # keep track of all the subcommands that the resources call
        resourceGraph.build(newTargetSections = resourceCommandLst, newAllSections = self._iniFile.sectionIfTemplates)

    # _parseBlend(): Parses all the blend command sections
    def _parseBlendCommands(self):
        blendRoots = self._sectionRoots[IniKeywords.Blend.value]
        if (blendRoots):
            self._parseElementCommands(blendRoots, self.blendCommandsGraph)

    # _parsePositionCommands(): Parses the position command sections
    def _parsePositionCommands(self):
        positionRoots = self._sectionRoots[IniKeywords.Position.value]
        if (not positionRoots):
            return
        
        type = self._iniFile.availableType
        positionModsToFix = type.positionEditors.fixTo

        iniModsToFix = self._iniFile.modsToFix
        if (iniModsToFix):
            positionModsToFix = positionModsToFix.intersection(iniModsToFix)

        for modToFix in positionModsToFix:
            positionEditor = type.getPositionEditor(modToFix, version = self._iniFile.version)
            if (positionEditor is not None):
                self._positionEditModsToFix.add(modToFix)

        positionDownloads = self.bufDownloads.get(IniKeywords.Position.value, {})
        if (not self._positionEditModsToFix and not positionDownloads):
            return
        
        positionRoots = self._sectionRoots[IniKeywords.Position.value]
        self._parseElementCommands(positionRoots, self.positionCommandsGraph)

    # _getTargetHashAndIndexSections(): Retrieves the sections with target hashes and indices
    def _getTargetHashAndIndexSections(self) -> Dict[str, IfTemplate]:
        notIncludeCommandNames = set(self.blendCommandsGraph.sections.keys()) | set(self.positionCommandsGraph.sections.keys())
        return self._iniFile.getTargetHashAndIndexSections(notIncludeCommandNames)

    def parseCommands(self):
        """
        Parses particular command `sections`_ within the mod
        """

        self._parseBlendCommands()
        self._parsePositionCommands()

        # build the DFS forest for the other sections that contain target hashes/indices that are not part of the blend commands
        hashIndexSections = self._getTargetHashAndIndexSections()
        hashIndexSections = list(hashIndexSections.keys())

        self.nonBlendHashIndexCommandsGraph.build(newTargetSections = hashIndexSections, newAllSections= self._iniFile.sectionIfTemplates)

    def parseResources(self):
        """
        Parses particular resource `sections`_ within the mod

        .. note::
            Needs :meth:`parseCommands` to be ran first, otherwise no resources will be parsed
        """

        self._parseElementResources(self.blendCommandsGraph, self.blendResourceCommandsGraph, 
                                    lambda part: IniKeywords.Vb1.value in part, 
                                    lambda part: set(map(lambda resourceData: resourceData[1], part.get(IniKeywords.Vb1.value, set()))))
        
        if (self._positionEditModsToFix):
            self._parseElementResources(self.positionCommandsGraph, self.positionResourceCommandsGraph,
                                        lambda part: IniKeywords.Vb0.value in part,
                                        lambda part: set(map(lambda resourceData: resourceData[1], part.get(IniKeywords.Vb0.value, set()))))

    def makeRemapData(self):
        """
        Creates any required remap internal data required by the fix
        """

        self._setToFix()
        self._makeRemapNames()
        self._makeRemapModels(self._iniFile.remapBlendModels, self.blendResourceCommandsGraph, getFixedFile = self._iniFile.getFixedBlendFile)

        if (self._positionEditModsToFix):
            self._makeRemapModels(self._iniFile.remapPositionModels, self.positionResourceCommandsGraph, getFixedFile = self._iniFile.getFixedPositionFile, modsToFix = self._positionEditModsToFix)

    def parse(self):
        self._getSectionRoots()

        self.blendCommandsGraph.remapNameFunc = self._iniFile.getRemapBlendName
        self.nonBlendHashIndexCommandsGraph.remapNameFunc = self._iniFile.getRemapFixName
        self.blendResourceCommandsGraph.remapNameFunc = self._iniFile.getRemapBlendResourceName
        self.positionCommandsGraph.remapNameFunc = self._iniFile.getRemapPositionName
        self.positionResourceCommandsGraph.remapNameFunc = self._iniFile.getRemapPositionResourceName

        self.parseCommands()
        self.setupDownloads(cleanup = False)
        self.parseResources()
        self.makeRemapData()

        self.clearParseTempData()

    def _getBufDownloads(self, sectionGraph: IniSectionGraph, bufKey: str):
        downloads = self.bufDownloads.get(bufKey, None)
        if (downloads is None):
            return
        
        bufDownloadParts = self._bufDownloadParts.get(bufKey)
        if (bufDownloadParts is None):
            bufDownloadParts = {}
            self._bufDownloadParts[bufKey] = bufDownloadParts

        for reg in downloads:
            result = set()
            sectionMissingParts = sectionGraph.targetsGetKeyMissingParts(reg)
            for sectionName in sectionMissingParts:
                result.update(sectionMissingParts[sectionName])

            bufDownloadParts[reg] = result

    # getDownloads(): Retrieve the particular sections or parts of sections that require a file download
    def getDownloads(self):
        self._bufDownloadParts.clear()

        downloadMode = self._iniFile.downloadMode
        if (downloadMode == DownloadMode.Disabled):
            return
        elif (downloadMode == DownloadMode.Always):
            self.normalizeSections(self.blendCommandsGraph)
            self.normalizeSections(self.positionCommandsGraph)
        
        self._getBufDownloads(self.blendCommandsGraph, IniKeywords.Blend.value)
        self._getBufDownloads(self.positionCommandsGraph, IniKeywords.Position.value)
    
    # _makeDownloadResourceIfTemplate(downloadname, modName, modObj, downloadFileBaseName, sectionName, downloadKvps): Creates the ifTemplate for a downloaded file
    def _makeDownloadResourceIfTemplate(self, downloadName: str, modName: str, modObj: str, downloadFileBaseName: str, sectionName: Optional[str] = None, downloadKvps: Optional[Dict[str, str]] = None):
        if (sectionName is None):
            sectionName = self._iniFile.getRemapDLResourceName(f"{modObj}{downloadName}", modName = modName)

        contentPart = IfContentPart({}, 0)
        if (downloadKvps is not None):
            for key in downloadKvps:
                val = downloadKvps[key]
                contentPart.addKVP(key, val)

        contentPart.addKVP("filename", downloadFileBaseName)
        return IfTemplate([contentPart], name = sectionName)
    
    def _addBufDownloads(self, bufKey: str, modTypeName: str):
        bufDownloadParts = self._bufDownloadParts.get(bufKey, {})
        bufDownloads = self.bufDownloads.get(bufKey, {})

        if (not bufDownloadParts and not bufDownloads):
            return
        
        bufDownloadNames = self._bufReferencedDownloadNames.get(bufKey)
        if (bufDownloadNames is None):
            bufDownloadNames = {}
            self._bufReferencedDownloadNames[bufKey] = bufDownloadNames

        for reg in bufDownloadParts:
            downloadData = bufDownloads[reg]
            downloadName, download, downloadKVPS = downloadData
            sectionName = self._iniFile.getRemapDLResourceName(f"{TextTools.capitalize(modTypeName)}{downloadName}")
            bufDownloadNames[reg] = sectionName

            ifTemplate = self._makeDownloadResourceIfTemplate(downloadName, modTypeName, "", download.filename, sectionName = sectionName, downloadKvps = downloadKVPS)
            self._iniFile.sectionIfTemplates[sectionName] = ifTemplate
            self._iniFile.fileDownloadModels[sectionName] = self._iniFile.makeDLModel(ifTemplate, download)

            for part in bufDownloadParts[reg]:
                part.addKVP(reg, sectionName)

    # addDownloads(): Adds the required download resources to the corresponding sections and their parts
    def addDownloads(self):
        modType = self._iniFile.availableType
        modTypeName = "" if (modType is None) else modType.name

        self._addBufDownloads(IniKeywords.Blend.value, modTypeName)
        self._addBufDownloads(IniKeywords.Position.value, modTypeName)

    def normalizeSections(self, sectionGraph: IniSectionGraph):
        """
        Normalize all the referenced `sections`_ within 'sectionGraph' to follow the branching
        structure described at :class:`IfTemplateNormTree`

        Parameters
        ----------
        sectionGraph: :class:`IniSectionGraph`
            The graph holding all the referenced sections
        """

        sections = sectionGraph.sections
        for sectionName in sections:
            sections[sectionName].normalize()

    def setupDownloads(self, cleanup: bool = True):
        """
        Setup the required downloads resources, if not already setup

        cleanup: :class:`bool`
            Whether to cleanup any temporary results from this method

            **Default**: ``True``
        """

        if (self._iniFile.downloadMode == DownloadMode.Disabled and not self._downloadsAdded):
            self._downloadsAdded = True
            return

        if (not self._downloadsAdded):
            self._downloadsAdded = True
            self.getDownloads()
            self.addDownloads()

            if (cleanup):
                self.clearParseDownloadSearch()

    def hasDownloads(self) -> bool:
        """
        Whether there are required downloads needed to be added

        .. note::
            requires :meth:`setupDownloads` to be ran first

        Returns
        -------
        :class:`bool`
            Whether downloads are needed to be added
        """

        return bool(self._bufReferencedDownloadNames)
##### EndScript