##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to remap your mods pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
import pip._internal as pip
import importlib
from typing import  Dict, Optional
from types import ModuleType
##### EndExtImports

##### LocalImports
from .PackageData import PackageData
##### EndLocalImports


##### Script
class PackageManager():
    """
    Class to handle external packages for the library at runtime
    """

    def __init__(self):
        self._packages: Dict[str, ModuleType] = {}

    def load(self, module: str, installName: Optional[str] = None, save: bool = True) -> ModuleType:
        """
        Imports an external package

        Parameters
        ----------
        module: :class:`str`
            The name of the module to import

        install: Optional[:class:`str`]
            The name of the installation for the package when using `pip`_ to download from `pypi`_ :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then assume that the name of the installation is the same as the name of the package :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        save: :class:`bool`
            Whether to save the installed package into this class

        Returns
        -------
        `Module`_
            The module to the external package
        """

        if (installName is None):
            installName = module

        try:
            return importlib.import_module(module)
        except ModuleNotFoundError:
            pip.main(['install', '-U', installName])

        result = importlib.import_module(module)
        if (save):
            self._packages[module] = result
        
        return result
    
    def get(self, packageData: PackageData):
        """
        Retrieves an external package

        Parameters
        ----------
        packageData: :class:`PackageData`
            The data needed for install the external package

        Returns
        -------
        `Module`_
            The module to the external package
        """

        result = None
        try:
            result = self._packages[packageData.module]
        except KeyError:
            result = self.load(packageData.module, installName = packageData.installName)

        return result
    
Packager = PackageManager()
##### EndScript