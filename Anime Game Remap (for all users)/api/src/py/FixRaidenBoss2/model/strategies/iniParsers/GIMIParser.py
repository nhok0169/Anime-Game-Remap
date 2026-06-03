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
from collections import defaultdict
from typing import TYPE_CHECKING, Set, Tuple, Optional, Dict, Union, Callable, List
##### EndExtImports

##### LocalImports
from ....constants.IniConsts import IniKeywords
from ....constants.DownloadMode import DownloadMode
from ....constants.GlobalPackageManager import GlobalPackageManager
from ....constants.Packages import PackageModules
from ....tools.tries.AhoCorasickBuilder import AhoCorasickBuilder
from .BaseIniParser import BaseIniParser
from ....constants.IniConsts import IniKeywords
from ....tools.TextTools import TextTools
from ...DownloadData import DownloadData
from ...IniSectionGraph import IniSectionGraph
from ...IniGraphGroup import IniGraphGroup
from ...iniresources.RemapIniResource import RemapIniDownload
from ...iftemplate.IfContentPart import IfContentPart
from ...iftemplate.IfTemplate import IfTemplate
from ...strategies.iniFixers.graphGroupEdits.GraphGroupEdit import GraphGroupEdit
from ...IniNamingTools import IniNamingTools

if (TYPE_CHECKING):
    from ...files.IniFile import IniFile
##### EndLocalImports


##### Script
class GIMIParser(BaseIniParser):
    """
    This class inherits from :class:`BaseIniParser`

    Parses a .ini file used by a GIMI related importer

    Attributes
    ----------
    iniFile: :class:`IniFile`
        The .ini file to parse

    modObjs: Optional[Set[Tuple[:class:`str`, :class:`str`]]]
        The mod objects to parse :raw-html:`<br />` :raw-html:`<br />`

        Each tuple contains:

        #. The name of the component
        #. The name of the object within the component :raw-html:`<br />` :raw-html:`<br />`

        .. tip::
            You can also interpret mod objects as the suffix part ending of some ``TextureOverride`` `section`_ :raw-html:`<br />` :raw-html:`<br />`

            eg.

            ``[TextureOverrideHuTaoBody]`` --> ``("", "Body")``
            ``[TextureOverrideYelanBangB]`` --> ``("Bang", "B")``
            ``[TexutureOverrideTexture16]`` --> ``("", "Texture16")``

        **Default**: ``None``

    objTargetFuncs: Optional[List[Callable[[:class:`GIMIParser`, :class:`str`, :class:`IfTemplate`, :class:`bool`], List[Tuple[:class:`str`, :class:`str`]]]]]
        A list of custom functions to define how retrieve the root `sections`_ of the mod objects :raw-html:`<br />` :raw-html:`<br />`

        Each function takes in:

        #. This parser
        #. The name of the `section`_ to parse
        #. The content of the `section`_ to parse
        #. Whether to only return 1 result

        and the function returns the corresponding mod objects that the `section`_ belongs to or ``None``if the `section`_ does not belong to any mod object. 
        Each tuple represents a mod object that contains the name of the component and the name of the object within the component.
    
        :raw-html:`<br />` :raw-html:`<br />`

        If this argument is ``None``, then will only use the :meth:`classifyByTextureOverrideName` function by default. :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    downloads: Optional[Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`DownloadData`]]]
        The files to download if the mod is missing some required files :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are tuples that contain the name of the component and the mod object
        * The inner keys are the names of the registers

         :raw-html:`<br />` :raw-html:`<br />`

        eg. :raw-html:`<br />`

        .. code-block::

            {("bang", "position"): {"vb0": ("Position", FileDownload("someServer.com/Position.buf", "Position.buf", {"type": "buffer", "stride": "40"}))}, 
             ("body", "A"): {"vb1": ("Blend", FileDownload("someServer.com/Blender.buf", "Blend.buf", {})), "vb999": ("NonExistantBlend", FileDownload("someServer.com/NonExistentBlend.buf", "fakeBlend.buf", {"type": "fakenews"}))}, 
             ("", "head"): {"ps-t0": ("Texcoord", FileDownload("someServer.com/texcoord.buf", "textensor.buf", {"model": "resnet50"}))}} 

        :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            The :attr:`DownloadData.name` for each :class:`DownloadData` should be unique

        **Default**: ``None``

    commandEdits: Optional[:class:`GraphGroupEdit`]
        Any further edits to the parsed caller/callee graphs for ``TexutreOverride`` related command `sections`_ :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    makeGlobalGraph: :class:`bool`
        Whether to make the graph for the entire .ini file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    disjointModObjs: :class:`bool`
        Whether the sets of `sections`_ for each mod object should be disjoint or not :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    downloads: Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`DownloadData`]]
        The files to download if the mod is missing some required files :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are tuples that contain the name of the component and the mod object
        * The inner keys are the names of the registers

    commandEdits: Optional[:class:`GraphGroupEdit`]
        Any further edits to the parsed caller/callee graphs for ``TexutreOverride`` related command `sections`_

    commandGraphs: Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]
        The caller/callee graphs for ``TextureOverride`` related  command `sections`_ :raw-html:`<br />` :raw-html:`<br />`

        * The keys are tuples that contain the name of the component and the mod object
        * The values contain the corresponding graph

    downloadResourceGraphs: Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`IniSectionGraph`]]
        The caller/callee graphs for `sections`_ related to download resources :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are tuples that contain the name of the component and the mod object
        * The inner keys are the registers within the mod objects
        * The values contain the corresponding graph

    makeGlobalGraph: :class:`bool`
        Whether to make the graph for the entire .ini file

    disjointModObjs: :class:`bool`
        Whether the sets of `sections`_ for each mod object should be disjoint or not

    globalGraph: Optional[:class:`IniSectionGraph`]
        The graph for the entire .ini file

    tempKwargs: Dict[:class:`str`, Any]
        Temporary user-defined keyword variables for the user to use. This attribute will only be cleared when :meth:`clear` is called

    _sectionTargets: Dict[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]
        The names of the `sections`_ that are used as the "entry point" to a particular group of `sections`_ in the
        ``TextureOverride`` `section`_ caller/callee `graph`_  :raw-html:`<br />` :raw-html:`<br />`

        The keys are tuples that contain the name of the component and the mod object for a particular group of `sections`_ and 
        sthe values are the root `section`_ names for that group :raw-html:`<br />`

        .. warning::
            These `sections`_ are not necessarily to roots of the graph (they may instead be a child to some other `section`_).

            Though based off heuristics, the probability of these `sections`_ being the roots are pretty high
    """

    TextureOverrideKey = IniKeywords.TextureOverride.value.lower()

    def __init__(self, iniFile: "IniFile", modObjs: Optional[Set[Tuple[str, str]]] = None, 
                 objTargetFuncs: Optional[List[Callable[["GIMIParser", str, IfTemplate, bool], List[Tuple[str, str]]]]] = None,
                 downloads: Optional[Dict[Tuple[str, str], Dict[str, DownloadData]]] = None,
                 commandEdits: Optional[GraphGroupEdit] = None, makeGlobalGraph: bool = True, disjointModObjs: bool = True):
        
        if (modObjs is None):
            modObjs = set()

        self._components: Set[str] = set()
        self.modObjs = modObjs
        self.objTargetFuncs = objTargetFuncs if (objTargetFuncs is not None) else []
        self.downloads = downloads if (downloads is not None) else {}
        self.commandEdits = commandEdits

        self.commandGraphs: Dict[Tuple[str, str], IniSectionGraph] = {}
        self.downloadResourceGraphs: Dict[Tuple[str, str]: Dict[str, IniSectionGraph]] = {}

        self._sectionTargets: Dict[Tuple[str, str], List[str]] = {}
        self._addedIfTemplateNames: Set[str] = set()
        self._downloadsAdded = False

        self.makeGlobalGraph = makeGlobalGraph
        self.globalGraph: Optional[IniSectionGraph] = None
        self.disjointModObjs = disjointModObjs

        self.tempKwargs = {}

        super().__init__(iniFile)

    @property
    def modObjs(self):
        """
        The different mod objects to parse :raw-html:`<br />` :raw-html:`<br />`

        Each tuple contains:

        #. The name of the component
        #. The name of the object within the component
        
        :getter: Returns the mod objects to parse
        :setter: Sets the new mod objects to parse
        :type: Set[Tuple[:class:`str`, :class:`str`]]
        """

        return self._modObjs

    @modObjs.setter
    def modObjs(self, newModObjs: Set[Tuple[str, str]]):
        self._modObjs = newModObjs

        newComponents = set()
        for modObj in self._modObjs:
            newComponents.add(modObj[0])

        self._components = newComponents

    @property
    def components(self):
        """
        The different components to parse

        :getter: Returns all the components
        :type: Set[:class:`str`]
        """

        return self._components
    
    def removeAddedIfTemplates(self):
        """
        Remove the newly added :class:`IfTemplate`s generated by this parser
        or its associated :class:`BaseIniFixer`s from :attr:`IniFile.sectionIfTemplates`
        """

        ifTemplates = self._iniFile.sectionIfTemplates
        for sectionName in ifTemplates:
            if (sectionName in self._addedIfTemplateNames):
                ifTemplates.pop(sectionName, None)

        self._addedIfTemplateNames.clear()

    def clear(self):
        super().clear()
        self.commandGraphs.clear()
        self.downloadResourceGraphs.clear()
        self.removeAddedIfTemplates()
        self.globalGraph = None
        self.tempKwargs.clear()

        self._downloadsAdded = False

    def buildGlobalGraph(self) -> IniSectionGraph:
        """
        Builds the graph for the entire .ini file

        Returns
        -------
        :class:`IniSectionGraph`
            The built graph
        """

        sections = self._iniFile.sectionIfTemplates
        return IniSectionGraph(sections, list(sections.keys()))
    
    @classmethod
    def classifyByTextureOverrideName(cls, parser: "GIMIParser", sectionName: str, section: IfTemplate, disjoint: bool = True, modObjs: Optional[Set[Tuple[str, str]]] = None, fromRoots: bool = True) -> List[Tuple[str, str]]:
        """
        Classify the ``TextureOverride`` `sections`_ to the specified mod objects

        Parameters
        ----------
        parser: :class:`GIMIParser`
            The parser used for the classification

        sectionName: :class:`str`
            The name of the `section`_ to classify

        section: :class:`IfTemplate`
            The content of the `section`_ to classify

        disjoint: :class:`bool`
            Whether to classify the `section`_ to only 1 mod object or allow classification to multiple mod objects :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        modObjs: Optional[Set[Tuple[:class:`str`, :class:`str`]]]
            The mod objects for classification, where each tuple contains:

            #. The name of the component
            #. The name of the object within the component

            :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use the mod objects specified at :attr:`modObjs` for the argument, ``parser`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``Nones``

        fromRoots: :class:`bool`
            Whether the `sections`_ to check are only from the root `sections`_ of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        List[Tuple[:class:`str`, :class:`str`]]
            The mod objects the `section`_ has been classified to :raw-html:`<br />` :raw-html:`<br />`

            Each mod object contains:

            #. The name of the component
            #. The name of the object within the component
        """

        if (fromRoots and parser.globalGraph is None):
            parser.globalGraph = parser.buildGlobalGraph()

        classifier = None
        classifierVarName = "textureOverrideClassifier"
        remapKey = IniKeywords.Remap.value.lower()

        # retrieve or build the classifier
        if (classifierVarName in parser.tempKwargs):
            classifier = parser.tempKwargs[classifierVarName]
        else:
            if (modObjs is None):
                modObjs = parser.modObjs

            classifierData = {}
            for modObj in modObjs:
                searchTxt = f"{modObj[0]}{modObj[1]}".lower()
                classifierData[searchTxt] = modObj

            classifierData[remapKey] = None
            classifier = AhoCorasickBuilder().build(data = classifierData)

            parser.tempKwargs[classifierVarName] = classifier

        # classify the section
        textureOverrideKeyLen = len(parser.TextureOverrideKey)
        cleanedSectionName = sectionName.lower().strip()

        if (not cleanedSectionName.startswith(parser.TextureOverrideKey)):
            return None

        cleanedSectionName = cleanedSectionName[textureOverrideKeyLen:]
        sectionKeySearch = classifier.getAll(cleanedSectionName)

        if (not sectionKeySearch or remapKey in sectionKeySearch):
            return None
        
        result = []
        for keyword in sectionKeySearch:
            if (not cleanedSectionName.endswith(keyword)):
                continue
            
            key = sectionKeySearch[keyword]
            result.append(key)
            
            if (disjoint):
                break

        return result

    def _getSectionTargets(self):
        """
        Retrieves the "entry points" names of the ``TextureOverride`` `sections`_ for each
        mod objects specified at :attr:`modObjs`
        """

        if (self.makeGlobalGraph):
            self.globalGraph = self.buildGlobalGraph()

        self._sectionTargets.clear()

        result = {}
        for modObj in self.modObjs:
            result[modObj] = []

        OrderedSet = GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet

        objTargetFuncs = self.objTargetFuncs
        if (not objTargetFuncs):
            objTargetFuncs = [lambda parser, sectionName, section, disjoint: self.classifyByTextureOverrideName(parser, sectionName, section, disjoint = disjoint, modObjs = self.modObjs, fromRoots = True)]

        sections = self._iniFile.sectionIfTemplates
        for sectionName in sections:
            section = sections[sectionName]
            sectionResult = OrderedSet([])

            for func in objTargetFuncs:
                funcResult = func(self, sectionName, section, self.disjointModObjs)
                sectionResult.update(OrderedSet(funcResult))

                if (sectionResult and self.disjointModObjs):
                    sectionResult = list(sectionResult)
                    sectionResult = [sectionResult[0]]
                    break

            for modObj in sectionResult:
                result[modObj].append(sectionName)

        self._sectionTargets = result

    def parseCommands(self):
        """
        Parses particular command `sections`_ within the mod, specified from :attr:`modObjs`
        """

        for modObj in self.modObjs:
            if (modObj not in self._sectionTargets):
                continue

            commandGraph = IniSectionGraph(self._iniFile.sectionIfTemplates, self._sectionTargets[modObj])
            self.commandGraphs[modObj] = commandGraph

    # getDownloads(): Retrieve the particular parts of sections that require a file download
    def getDownloads(self) -> Dict[Tuple[str, str], Dict[str, Union[Set[IfContentPart], Set[IfTemplate]]]]:
        result = defaultdict(lambda: {})
        visitedParts = set()

        for modObj in self.downloads:
            if (modObj not in self.commandGraphs):
                continue

            commandGraph = self.commandGraphs[modObj]
            objDownloads = self.downloads[modObj]

            for reg in objDownloads:
                download = objDownloads[reg]
                downloadResult = set()

                if (not download.refToSection):
                    parts = commandGraph.getKeyMissingParts(reg)

                    for sectionName in parts:
                        for part in parts[sectionName]:
                            if (part not in visitedParts):
                                downloadResult.add(part)
                                visitedParts.add(part)

                elif (not self._iniFile.downloadMode == DownloadMode.Always):
                    sectionCover = commandGraph.rootsAreFullyCovered(reg)
                    for sectionName in sectionCover:
                        if (not sectionCover[sectionName]):
                            downloadResult.add(commandGraph.getSection(sectionName))

                else:
                    for section in commandGraph.getRootSections():
                        downloadResult.add(section)

                result[modObj][reg] = downloadResult

        result = dict(result)
        return result
    
    # _createDownloadResource(): Create the download resource
    def _createDownloadResource(self, modTypeName: str, modObj: str, reg: str, downloadData: DownloadData, iniFolder: str) -> str:
        resourceSectionName = IniNamingTools.getRemapDLResourceName(f"{TextTools.capitalize(modTypeName)}{TextTools.capitalize(downloadData.name)}")

        resourceGraph = self.downloadResourceGraphs[modObj].get(reg)
        if (resourceGraph is None):
            resourceGraph = IniSectionGraph({}, [], build = False)
            self.downloadResourceGraphs[modObj][reg] = resourceGraph

        if (resourceSectionName not in resourceGraph.sections):
            downloadSection = downloadData.createResSection(resourceSectionName)
            resourceGraph.sections[resourceSectionName] = downloadSection
            resourceGraph.targetSectionNames.append(resourceSectionName)
            self._iniFile.sectionIfTemplates[resourceSectionName] = downloadSection

            download = RemapIniDownload(iniFolder, downloadData.download.filename, downloadData.download)
            self._iniFile.fileDownloads.append(download)

        return resourceSectionName
    
    # addDownloads(partsNeedDownload): Adds the required download resources to the corresponding sections and their parts
    def addDownloads(self, partsNeedDownload: Dict[Tuple[str, str], Dict[str, Union[Set[IfContentPart], Set[IfTemplate]]]]):
        modType = self._iniFile.availableType
        modTypeName = "" if (modType is None) else modType.name
        iniFolder = self._iniFile.folder
        
        self.downloadResourceGraphs = defaultdict(lambda: {}, self.downloadResourceGraphs)

        for modObj in partsNeedDownload:
            component, obj = modObj
            objPartNeedDownload = partsNeedDownload[modObj]

            for reg in objPartNeedDownload:
                if (modObj not in self.downloads or reg not in self.downloads[modObj] or modObj not in self.commandGraphs):
                    continue
                
                commandGraph = self.commandGraphs[modObj]
                downloadData = self.downloads[modObj][reg]
                resourceSectionName = self._createDownloadResource(modTypeName, modObj, reg, downloadData, iniFolder)

                # add the download reference
                needDownloadData = objPartNeedDownload[reg]

                if (not downloadData.refToSection):
                    for part in needDownloadData:
                        downloadData.addToPart(part, reg, resourceSectionName)

                    continue
                
                rootIfTemplates = []
                if (needDownloadData):
                    rootIfTemplates = needDownloadData

                for ifTemplate in rootIfTemplates:
                    downloadData.addToSection(ifTemplate, reg, resourceSectionName)

        # add in the downloads for sections with empty graphs
        commandGraphWasEmpty = {}

        for modObj in self.downloads:
            objDownloads = self.downloads[modObj]

            for reg in objDownloads:
                if (modObj not in self.commandGraphs):
                    continue
                
                commandGraph = self.commandGraphs[modObj]
                downloadData = objDownloads[reg]

                if (modObj not in commandGraphWasEmpty):
                    commandGraphWasEmpty[modObj] = commandGraph.isEmpty()
                    graphWasEmpty = commandGraphWasEmpty[modObj]
                    graphIsEmpty = graphWasEmpty
                else:
                    graphWasEmpty = commandGraphWasEmpty[modObj]
                    graphIsEmpty = commandGraph.isEmpty()

                if (not graphWasEmpty):
                    continue
                
                component, obj = modObj
                commandSectionName = IniNamingTools.getTextureOverrideRemapFix(component, obj, modName = modTypeName)
                resourceSectionName = self._createDownloadResource(modTypeName, modObj, reg, downloadData, iniFolder)

                rootIfTemplates = []

                if (graphIsEmpty):
                    commandIfTemplate = IfTemplate([], name = commandSectionName)
                    commandGraph.sections = {commandSectionName: commandIfTemplate}
                    commandGraph.build(targetSectionNames = [commandSectionName])

                    self._iniFile.sectionIfTemplates[commandSectionName] = commandIfTemplate
                    downloadData.addToSection(commandIfTemplate, reg, resourceSectionName)
                else:
                    commandIfTemplate = self._iniFile.sectionIfTemplates[commandSectionName]
                    rootIfTemplates.append(commandIfTemplate)

                    if (not downloadData.refToSection):
                        part = commandIfTemplate.parts[0]
                        downloadData.addToPart(part, reg, resourceSectionName)
                    else:
                        downloadData.addToSection(commandIfTemplate , reg, resourceSectionName)

        # build the resource graphs
        self.downloadResourceGraphs = dict(self.downloadResourceGraphs)

        for modObj in self.downloadResourceGraphs:
            objResGraphs = self.downloadResourceGraphs[modObj]

            for reg in objResGraphs:
                objResGraphs[reg].build()

    def setupDownloads(self):
        """
        Setup the required downloads resources, if not already setup
        """

        if (self._iniFile.downloadMode == DownloadMode.Disabled and not self._downloadsAdded):
            self._downloadsAdded = True
            return
        
        if (self._downloadsAdded):
            return
        
        if (self._iniFile.downloadMode == DownloadMode.Always):
            for modObj in self.commandGraphs:
                graph = self.commandGraphs[modObj]
                graph.normalize()

        self._downloadsAdded = True
        partsNeedDownload = self.getDownloads()
        self.addDownloads(partsNeedDownload)

    def editCommands(self):
        """
        Edits the caller/callee graphs for ``TextureOverride`` related  command `sections`_
        """

        if (self.commandEdits is None):
            return

        modType = self._iniFile.availableType
        modTypeName = "" if (modType is None) else modType.name
        
        graphGroups = [IniGraphGroup(graphs = self.commandGraphs)]
        graphGroups = self.commandEdits.editFromIni(graphGroups, self._iniFile, modType, modName = modTypeName)
        self.commandGraphs = graphGroups[0].graphs

    def parse(self):
        self._getSectionTargets()
        self.parseCommands()
        self.setupDownloads()
        self.editCommands()
##### EndScript