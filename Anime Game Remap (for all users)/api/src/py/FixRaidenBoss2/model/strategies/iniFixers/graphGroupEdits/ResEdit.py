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
from collections import deque
from typing import List, Optional, Tuple, TYPE_CHECKING, Dict, Callable
##### EndExtImports

##### LocalImports
from .....constants.IniConsts import IniKeywords
from .....constants.IniGraphReplaceMode import IniGraphReplaceMode
from .....tools.DictTools import DictTools
from .....tools.HashTools import HashTools
from ....IniNamingTools import IniNamingTools
from ....IniGraphGroup import IniGraphGroup
from ....iftemplate.IfTemplate import IfTemplate
from ....iftemplate.IfContentPart import IfContentPart
from ....iniresources.IniResource import IniResource, IniFixResource
from ....IniSectionGraph import IniSectionGraph

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class BaseResEdit():
    """
    Base class to construct the necessary parts for a particular resource in a .ini file

    Parameters
    ----------
    resType: :class:`str`
        The name of the type of resource

    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

        The tuple contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    graphReplaceMode: :class:`IniGraphReplaceMode`
        What to do when the corresponding :class:`IniSectionGraph` to construct already exists :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``IniGraphReplaceMode.Ignore``

    Attributes
    ----------
    resType: :class:`str`
        The name of the type of resource

    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

        The tuple contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    graphReplaceMode: :class:`IniGraphReplaceMode`
        What to do when the corresponding :class:`IniSectionGraph` to construct already exists
    """

    def __init__(self, resType: str, resModObj: Tuple[int, str, str], graphReplaceMode: IniGraphReplaceMode = IniGraphReplaceMode.Ignore):
        self.resType = resType
        self.resModObj = resModObj
        self.graphReplaceMode = graphReplaceMode

    @classmethod
    def getFileId(cls, modObj: Tuple[int, str, str], sectionName: str, part: IfContentPart, orderInd: int, file: str) -> str:
        """
        Retrieves a unique id for a file within a single .ini file

        Parameters
        ----------
        modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

            The tuple contains:

            #. The index for the .ini file
            #. The name of the component
            #. The name of the object

        sectionName: :class:`str`
            The name of the `section`_

        part: :class:`IfContentPart`
            The part where the file belongs to

        orderInd: :class:`int`
            The specific order index where the file occurs in the part

        file: :class:`str`
            The path for the file

        Returns
        -------
        Tuple[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`, :class:`int`, :class:`int`, :class:`str`]
            The unique id for the file
        """

        return HashTools.base64DeterministicShortUniqueHash(modObj, sectionName, part.id, orderInd, file)

    def clear(self):
        """
        Clears any saved state information
        """

        pass
    
    def collectResourceName(self, oldResourceName: str, newResourceName: str) -> Tuple[str, str]:
        """
        Collects the name of the fixed resource `section`_ (used for the ``collectedSections`` parameter in :meth:`buildResGraph`)

        Parameters
        ----------
        oldResourceName: :class:`str`
            The old name of the resource `section`_

        newResourceName: :class:`str`
            The fixed name for the resource `section`_ (created by :meth:`getFixResourceName`)

        Returns
        -------
        Tuple[:class:`str`, :class:`str`]
            A tuple where the first value is for the old resource name the the second value is the new resource name
        """

        return (oldResourceName, newResourceName)

    def getFixResourceName(self, resource: str, modType: "ModType", modName: str = "") -> Optional[str]:
        """
        Retrieves the name of the fixed resource `section`_

        Parameters
        ----------
        resource: :class:`str`
            The name of the original resource `section`_

        modType: :class:`ModType`
            The type of mod being fixed

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        Optional[:class:`str`]
            The `section`_ name of the fixed resource :raw-html:`<br />` :raw-html:`<br />`

            If ``None`` is returned, then indicates that there was no name change between the original resource and the fixed resource
        """

        return IniNamingTools.getRemapFixName(resource, modName = modName)
    
    @classmethod
    def fileAddGraphId(cls, file: str, graphId: str = "") -> str:
        """
        Adds the unique id for the :class:`IniSectionGraph` of the resource to the name of the file

        Parameters
        ----------
        file: :class:`str`
            The path to the file to add the id

        graphId: :class:`str`
            The id to add :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The file with the id added
        """

        fileParts = file.rsplit(".", 1)
        fileParts[0] += f"_{graphId}"
        return ".".join(fileParts)
    
    def getFixFile(self, file: str, modType: "ModType", modName: str = "", graphId: str = "") -> str:
        """
        Retrieves the file path to the fixed resource

        Parameters
        ----------
        file: :class:`str`
            The file path to the original resource

        modType: :class:`ModType`
            The type of mod being fixed

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        graphId: :class:`str`
            The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The file path to the fixed resource
        """

        return IniNamingTools.getFixedFile(file, modName = modName)

    def buildResModel(self, resType: str, ini: "IniFile", srcPath: str, *args, **kwargs) -> IniResource:
        """
        Builds the model for the resource

        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource

        ini: :class:`IniFile`
            The .ini file to build the resource for

        srcpath: :class:`str`
            The file path to the original resource

        Returns
        -------
        :class:`IniResource`
            The built resource
        """

        return IniResource(resType, ini.folder, srcPath)
    
    def buildResModels(self, graph: IniSectionGraph, ini: "IniFile", *args, resources: Optional[Dict[Tuple[str, int], List[IniResource]]] = None, resourceFilter: Optional[Callable[[str], bool]] = None, graphId: str = "", **kwargs):
        """
        Builds and saves the resources, given the :class:`IniSectionGraph` for a resource

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph for the particular resource

        ini: :class:`IniFile`
            The .ini file to build the resource for

        resources: Optional[Dict[Tuple[:class:`str`, Tuple[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`, :class:`int`, :class:`int`, :class:`str`]], List[:class:`IniResource`]]]
            The result where the built resource models are stored :raw-html:`<br />` :raw-html:`<br />`

            * The keys of the dictionary are tuples that consists of:
            
                * The source file
                * A unique id for the source file. Created from :meth:`getFileId`
            
            * The values are the resource models :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will assume the resource models are stored in :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        resourceFilter: Optional[Callable[[:class:`str`, Tuple[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`, :class:`int`, :class:`int`, :class:`str`]], :class:`bool`]]
            A predicate to determine which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`

            The predicate takes in:

            #. The source file
            #. An assigned id to the file. Created from :meth:`getFileId` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        graphId: :class:`str`
            The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``
        """

        for iterData in graph.iterByContentPart():
            sectionName = iterData.sectionName
            part = iterData.part

            regVals = part.get(IniKeywords.Filename.value, default = [], withInds = True)
            indsToRemove = set()

            for ind, val in regVals:
                fileKey = self.getFileId(self.resModObj, sectionName, part, ind, val)

                if (resourceFilter is not None and not resourceFilter(val, fileKey)):
                    indsToRemove.add(ind)
                    continue

                newVal = val
                if (graphId):
                    newVal = self.fileAddGraphId(val, graphId = HashTools.base64DeterministicShortUniqueHash(graphId))
                    part.setValByInd(ind, newVal)

                resource = self.buildResModel(self.resType, ini, newVal, *args, **kwargs)

                if (resources is None):
                    ini.resources.append(resource)
                elif (fileKey not in resources):
                    resources[fileKey] = deque([resource])
                else:
                    resources[fileKey].append(resource)

            if (indsToRemove):
                part.removeKey((IniKeywords.Filename.value, lambda ind, val: ind in indsToRemove))

    def renameUncollectedSection(self, sectionName: str, modType: "ModType", modName: str = "") -> str:
        result = self.getFixResourceName(sectionName, modType, modName = modName)
        if (result is None):
            return sectionName
        return result

    def getResGraph(self, collectedSections: Dict[str, str], modType: "ModType", ini: "IniFile", graphGroups: List[IniGraphGroup], modName: str = "", rename: bool = True, copySections: bool = False) -> Optional[IniSectionGraph]:
        """
        Retrieves the particular :class:`IniSectionGraph` for the resource

        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource :raw-html:`<br />` :raw-html:`<br />`

            The keys are the old name of the `sections`_ and the values are the fixed names of the `sections`_

        modType: :class:`ModType`
            The type of mod being fixed

        ini: :class:`IniFile`
            The associated original .ini file being fixed

        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        rename: :class:`bool`
            Whether to rename the `sections`_ for the graph :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        copySections: :class:`bool`
            Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        Optional[:class:`IniSectionGraph`]
            The retrieved graph
        """

        iniInd, comp, obj = self.resModObj
        if (iniInd >= len(graphGroups)):
            return None
        
        graphGroup = graphGroups[iniInd]
        modObj = (comp, obj)
        graph = graphGroup.graphs.get(modObj)

        if (graph is not None and self.graphReplaceMode == IniGraphReplaceMode.Ignore):
            if (rename):
                graph.rename(lambda sectionName: collectedSections[sectionName] if (sectionName in collectedSections) else sectionName)

            return graph

        if (graph is None or self.graphReplaceMode == IniGraphReplaceMode.Replace):
            graph = IniSectionGraph(sections = ini.sectionIfTemplates, targetSectionNames = list(collectedSections.keys()), copySections = copySections)

            if (rename):
                graph.rename(lambda sectionName: collectedSections[sectionName] if (sectionName in collectedSections) else self.renameUncollectedSection(sectionName, modType, modName = modName))

        elif (self.graphReplaceMode == IniGraphReplaceMode.Combine):
            graph.targetSectionNames += list(collectedSections.keys())
            DictTools.update(graph.sections, ini.sectionIfTemplates)
            graph.build(copySections = copySections)

            if (rename):
                graph.rename(lambda sectionName: collectedSections[sectionName] if (sectionName in collectedSections) else sectionName)

        return graph
    
    def buildResources(self, collectedSections: Dict[str, str], modType: "ModType", ini: "IniFile", graphGroups: List[IniGraphGroup], modName: str = "", 
                       resourceFilter: Optional[Callable[[str], bool]] = None, resources: Optional[Dict[Tuple[str, int], List[IniResource]]] = None,
                       copySections: bool = False) -> List[IniGraphGroup]:
        """
        Builds all the :class:`IniSectionGraph` and the corresponding models for the resources

        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource :raw-html:`<br />` :raw-html:`<br />`

            The keys are the old name of the `sections`_ and the values are the fixed names of the `sections`_

        modType: :class:`ModType`
            The type of mod being fixed

        ini: :class:`IniFile`
            The associated original .ini file being fixed

        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        resourceFilter: Optional[Callable[[:class:`str`, Tuple[:class:`str`, :class:`int`, :class:`int`, :class:`str`]], :class:`bool`]]
            A predicate to determine which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`

            The predicate takes in:

            #. The source file
            #. An assigned id to the file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        resources: Optional[Dict[Tuple[:class:`str`, Tuple[:class:`str`, :class:`int`, :class:`int`, :class:`str`]], List[:class:`IniResource`]]]
            The result where the built resource models are stored :raw-html:`<br />` :raw-html:`<br />`

            * The keys of the dictionary are tuples that consists of:
            
                * The source file
                * A unique id for the source file 
            
            * The values are the resource models :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will assume the resource models are stored in :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        copySections: :class:`bool`
            Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        List[:class:`IniGraphGroup`]
            The group of graphs that now include the newly created graph for the resource

            .. tip::
                You can access the newly generated graph using :attr:`resModObj` on the group of graphs
        """

        graph = self.getResGraph(collectedSections, modType, ini, graphGroups, modName = modName, copySections = copySections)
        if (graph is None):
            return graphGroups

        iniInd, comp, obj = self.resModObj

        if (iniInd >= len(graphGroups)):
            return graphGroups
        
        graphGroup = graphGroups[iniInd]
        modObj = (comp, obj)
        graphGroup.addGraph(modObj, graph)

        self.buildResModels(graph, ini, resources = resources, resourceFilter = resourceFilter)
        return graphGroups

class ResIdentity(BaseResEdit):
    """
    This class inherits from :class:`BaseResEdit`

    Class to only build the :class:`IniSectionGraph` for the original collected resource

    Parameters
    ----------
    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

        The tuple contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    createResModel: :class:`bool`
        Whether to build the models for the resources :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    createResModel: :class:`bool`
        Whether to build the models for the resources
    """

    def __init__(self, resModObj: Tuple[int, str, str], createResModel: bool = True):
        super().__init__("", resModObj)
        self.createResModel = createResModel

    def getFixResourceName(self, resource: str, modType: "ModType", modName: str = "") -> Optional[str]:
        return None
    
    def buildResModels(self, graph: IniSectionGraph, ini: "IniFile", *args, resources: Optional[Dict[Tuple[str, int], List[IniResource]]] = None, resourceFilter: Optional[Callable[[str], bool]] = None, graphId: str = "", **kwargs):
        if (not self.createResModel):
            return
        
        super().buildResModels(graph, ini, *args, resources = resources, resourceFilter = resourceFilter, graphId = graphId, **kwargs)


class ResReplace(BaseResEdit):
    """
    This class inherits from :class:`BaseResEdit`

    Class that creates the necesssary parts for a fixed resource by building upon the existing :class:`IniSectionGraph` of the original resource

    Parameters
    ----------
    resType: :class:`str`
        The name of the type of resource

    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

        The tuple contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the object

    graphReplaceMode: :class:`IniGraphReplaceMode`
        What to do when the corresponding :class:`IniSectionGraph` to construct already exists :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``IniGraphReplaceMode.Ignore``
    """
    
    def buildResModel(self, resType: str, ini: "IniFile", srcPath: str, fixedPath: str, modType: "ModType", *args, modName: str = "", **kwargs) -> IniFixResource:
        """
        Builds the model for the resource

        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource

        ini: :class:`IniFile`
            The .ini file to build the resource for

        srcpath: :class:`str`
            The file path to the original resource

        fixedPath: :class:`str`
            The file path to the fixed resource

        modType: :class:`ModType`
            The type of mod being fixed

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`IniResource`
            The built resource
        """

        return IniFixResource(resType, ini.folder, srcPath, fixedPath)
    
    def buildResModels(self, graph: IniSectionGraph, ini: "IniFile", modType: "ModType", *args, modName: str = "", resources: Optional[Dict[Tuple[str, int], List[IniResource]]] = None, resourceFilter: Optional[Callable[[str], bool]] = None, graphId: str = "", **kwargs):
        """
        Builds and saves the resources, given the :class:`IniSectionGraph` for a resource

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph for the particular resource

        ini: :class:`IniFile`
            The .ini file to build the resource for

        modType: :class:`ModType`
            The type of mod being fixed

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        resources: Optional[Dict[Tuple[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`, Tuple[:class:`str`, :class:`int`, :class:`int`, :class:`str`]], List[:class:`IniResource`]]]
            The result where the built resource models are stored :raw-html:`<br />` :raw-html:`<br />`

            * The keys of the dictionary are tuples that consists of:
            
                * The source file
                * A unique id for the source file. Created from :meth:`getFileId`
            
            * The values are the resource models :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will assume the resource models are stored in :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        resourceFilter: Optional[Callable[[:class:`str`, Tuple[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`, :class:`int`, :class:`int`, :class:`str`]], :class:`bool`]]
            A predicate to determine which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`

            The predicate takes in:

            #. The source file
            #. An assigned id to the file. Created from :meth:`getFileId` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        for iterData in graph.iterByContentPart():
            sectionName = iterData.sectionName
            part = iterData.part

            regVals = part.get(IniKeywords.Filename.value, default = [], withInds = True)
            indsToRemove = set()

            for ind, val in regVals:
                fileKey = self.getFileId(self.resModObj, sectionName, part, ind, val)

                if (resourceFilter is not None and not resourceFilter(val, fileKey)):
                    indsToRemove.add(ind)
                    continue
                
                newVal = self.getFixFile(val, modType, modName = modName, graphId = graphId)
                part.setValByInd(ind, newVal)
                resource = self.buildResModel(self.resType, ini, val, newVal, modType, *args, modName = modName, **kwargs)

                if (resources is None):
                    ini.resources.append(resource)
                elif (fileKey not in resources):
                    resources[fileKey] = deque([resource])
                else:
                    resources[fileKey].append(resource)

            if (indsToRemove):
                part.removeKey((IniKeywords.Filename.value, lambda ind, val: ind in indsToRemove))

    def buildResources(self, collectedSections: Dict[str, str], modType: "ModType", ini: "IniFile", graphGroups: List[IniGraphGroup], modName: str = "", 
                       resourceFilter: Optional[Callable[[str], bool]] = None, resources: Optional[Dict[Tuple[str, int], List[IniResource]]] = None, copySections: bool = False) -> List[IniGraphGroup]:
        graph = self.getResGraph(collectedSections, modType, ini, graphGroups, modName = modName, copySections = copySections)
        if (graph is None):
            return graphGroups

        iniInd, comp, obj = self.resModObj

        if (iniInd > len(graphGroups)):
            return graphGroups
        
        graphGroup = graphGroups[iniInd]
        modObj = (comp, obj)
        graphGroup.addGraph(modObj, graph)

        self.buildResModels(graph, ini, modType, modName = modName, resources = resources, resourceFilter = resourceFilter)
        return graphGroups


class ResCreate(BaseResEdit):
    """
    This class inherits from :class:`BaseResEdit`

    Class that creates the necesssary parts for a fixed resource

    Parameters
    ----------
    resType: :class:`str`
        The name of the type of resource

    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

        The tuple contains:

        #. The index for the .ini file
        #. The name of the component
        #. The name of the objects
    """

    def buildResModel(self, resType: str, ini: "IniFile", srcPath: str, modType: "ModType", *args, modName: str = "", **kwargs) -> IniResource:
        """
        Builds the model for the resource

        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource

        ini: :class:`IniFile`
            The .ini file to build the resource for

        srcpath: :class:`str`
            The file path to the original resource

        modType: :class:`ModType`
            The type of mod being fixed

        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`IniResource`
            The built resource
        """

        return IniResource(resType, ini.folder, srcPath)

    def buildSection(self, sectionName: str, modType: "ModType", modName: str = "") -> IfTemplate:
        """
        Builds a `section`_ for the resource

        Parameters
        ----------
        sectionName: :class:`str`
            The name for the `section`_

        modType: :class:`ModType`
            The type of mod to fix from
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`IfTemplate`
            The generated `section`_
        """

        pass
    
    def collectResourceName(self, oldResourceName: str, newResourceName: str) -> Tuple[str, str]:
        return (newResourceName, newResourceName)

    def getResGraph(self, collectedSections: Dict[str, str], modType: "ModType", ini: "IniFile", graphGroups: List[IniGraphGroup], modName: str = "", rename: bool = True, copySections: bool = False) -> Optional[IniSectionGraph]:
        iniInd, comp, obj = self.resModObj
        if (iniInd >= len(graphGroups)):
            return None
        
        graphGroup = graphGroups[iniInd]
        modObj = (comp, obj)
        graph = graphGroup.graphs.get(modObj)
        graphExists = graph is not None

        if (graphExists and self.graphReplaceMode == IniGraphReplaceMode.Ignore):
            return graph
        
        sections = {}
        for oldSectionName in collectedSections:
            newSectionName = collectedSections[oldSectionName]

            if (newSectionName not in sections):
                section = self.buildSection(newSectionName, modType, modName = modName)
                sections[newSectionName] = section

        if (graphExists and self.graphReplaceMode == IniGraphReplaceMode.Combine):
            DictTools.update(graph.sections, sections)
            graph.targetSectionNames += list(collectedSections.values())
            graph.build(copySections = copySections)
        elif (not graphExists or self.graphReplaceMode == IniGraphReplaceMode.Replace):
            graph = IniSectionGraph(sections, list(collectedSections.values()), copySections = copySections)

        return graph
    
    def buildResources(self, collectedSections: Dict[str, str], modType: "ModType", ini: "IniFile", graphGroups: List[IniGraphGroup], modName: str = "", 
                       resourceFilter: Optional[Callable[[str], bool]] = None, resources: Optional[Dict[str, List[IniResource]]] = None, copySections: bool = False) -> List[IniGraphGroup]:
        graph = self.getResGraph(collectedSections, modType, ini, graphGroups, modName = modName, copySections = copySections)
        if (graph is None):
            return graphGroups

        iniInd, comp, obj = self.resModObj

        if (iniInd >= len(graphGroups)):
            return graphGroups
        
        graphGroup = graphGroups[iniInd]
        modObj = (comp, obj)
        graphGroup.addGraph(modObj, graph)

        self.buildResModels(graph, ini, modType, modName = modName, resources = resources, resourceFilter = resourceFilter)
        return graphGroups
##### EndScript