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
from typing import Optional, Callable, Dict, Any, Set
##### EndExtImports

##### LocalImports
from ...tools.files.FileService import FileService
##### EndLocalImports


##### Script
class IniResource():
    """
    Base class for a resource in the .ini file

    Parameters
    ----------
    type: :class:`str`
        The name for the type of resource

    iniFolderPath: :class:`str`
        The path to the folder of the .ini file

    srcPath: :class:`str`
        The file path to the resource

    fixFunc: Optional[Callable[[:class:`BaseIniResource`, ...], Any]]
        Custom function for fixing the resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    type: :class:`str`
        The name for the type of resource

    srcPath: :class:`str`
        The full file path to the resource

    fixFunc: Optional[Callable[[:class:`IniResource`, ...], Any]]
        Custom function for fixing the resource
    """

    def __init__(self, type: str, iniFolderPath: str, srcPath: str, fixFunc: Optional[Callable[["IniResource"], Any]] = None):
        self.type = type
        self.srcPath = FileService.absPathOfRelPath(srcPath, iniFolderPath)
        self.fixFunc = fixFunc

    def _fix(self, *args, **kwargs):
        pass

    def fix(self, *args, **kwargs):
        """
        Fixes the resource
        """

        if (self.fixFunc is None):
            return self._fix(*args, **kwargs)
        
        return self.fixFunc(self, *args, **kwargs)


class IniFixResource(IniResource):
    """
    This class inherits from :class:`IniResource`

    Base class for a resource to be fixed in the .ini file

    Parameters
    ----------
    type: :class:`str`
        The name for the type of resource

    iniFolderPath: :class:`str`
        The path to the folder of the .ini file

    srcPath: :class:`str`
        The file path to the resource

    fixedPath: :class:`str`
        The file path to the fixed resource

    fixFunc: Optional[Callable[[:class:`IniFixResource`, ...], Any]]
        Custom function for fixing the resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    fixedPath: :class:`str`
        The full file path to the fixed resource
    """

    def __init__(self, type: str, iniFolderPath: str, srcPath: str, fixedPath: str, fixFunc: Optional[Callable[["IniFixResource"], Any]] = None):
        super().__init__(type, iniFolderPath, srcPath, fixFunc = fixFunc)
        self.srcPath = FileService.absPathOfRelPath(srcPath, iniFolderPath)
        self.fixedPath = FileService.absPathOfRelPath(fixedPath, iniFolderPath)
        self.fixFunc = fixFunc


class IniGroupedResource():
    """
    Base class for a group of resources

    Parameters
    ----------
    name: :class:`str`
        The name of the group of resources

    resources: Optional[Dict[:class:`str`, Union[:class:`IniResource`, :class:`str`]]]
        The group of resources. 

        * The keys are the type of the resource
        * If :attr:`isBuilt` is set to ``True`` then all the values are expected to be of type :class:`IniResource`
          Otherwise, all the values are expected to be a string value representing the source file location for the resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    fixFunc: Optional[Callable[[:class:`IniGroupedResource`, ...], Any]]
        Custom function for fixing the resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    isBuilt: :class:`bool`
        Whether the grouped resource is ready to be fixed :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Parameters
    ----------
    name: :class:`str`
        The name of the group of resources

    resources: Optional[Dict[:class:`str`, Union[:class:`IniResource`, :class:`str`]]]
        The group of resources. 

        * The keys are the type of the resource
        * If :attr:`isBuilt` is set to ``True`` then all the values are expected to be of type :class:`IniResource`
          Otherwise, all the values are expected to be a string value representing the source file location for the resource

    fixFunc: Optional[Callable[[:class:`IniGroupedResource`, ...], Any]]
        Custom function for fixing the resource

    isBuilt: :class:`bool`
        Whether the grouped resource is ready to be fixed
    """

    def __init__(self, name: str, resources: Optional[Dict[str, IniResource]] = None, fixFunc: Optional[Callable[["IniGroupedResource"], Any]] = None, isBuilt: bool = True):
        self.name = name
        self.resources = resources if (resources is not None) else {}
        self.fixFunc = fixFunc
        self.isBuilt = isBuilt

    def _fix(self, *args, **kwargs):
        pass

    def fix(self, *args, **kwargs):
        """
        Fixes the resource
        """

        if (self.fixFunc is None):
            return self._fix(*args, **kwargs)
        
        return self.fixFunc(self, *args, **kwargs)
    
    def isMissing(self, collected: Set[str]) -> bool:
        """
        Given a subset of names of the collected resources so far, is this grouped resource missing some type of resource from the given subset

        Parameters
        ----------
        collected: Set[:class:`str`]
            the subset of the names of the collected resources so far

        Returns
        -------
        :class:`bool`
            Whether this grouped resource is missing some resource from the specified subset
        """

        return bool(collected - set(self.resources.keys()))

    def addResource(self, resType: str, resource: IniResource):
        """
        Adds an individual resource to the resource group

        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource

        resource: :class:`IniResource`
            The resource to adds
        """

        self.resources[resType] = resource
##### EndScript