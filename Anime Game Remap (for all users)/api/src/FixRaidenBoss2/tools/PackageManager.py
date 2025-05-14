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
import pip._internal as pip
import importlib
from typing import  Dict, Optional, List
from types import ModuleType
##### EndExtImports

##### LocalImports
from .PackageData import PackageData
##### EndLocalImports


##### Script
class PackageManager():
    """
    Class to handle external packages for the library at runtime

    Attributes
    ----------
    proxy: Optional[:class:`str`]
        The link to the proxy server used for any internet network requests made :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    options: Optional[List[:class:`str`]]
        Additional options to supply to into `pip`_ :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Parameters
    ----------
    proxy: Optional[:class:`str`]
        The link to the proxy server used for any internet network requests made

    options: List[:class:`str`]
        Additional options to supply to into `pip`_
    """

    def __init__(self, proxy: Optional[str] = None, options: Optional[List[str]] = None):
        self._packages: Dict[str, ModuleType] = {}
        self.proxy = proxy
        self.options = [] if (options is None) else options

    def load(self, module: str, installName: Optional[str] = None, installOptions: Optional[List[str]] = None, save: bool = True) -> ModuleType:
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

        installOptions: Optional[List[:class:`str`]]
            Additional installation options to supply into `pip`_ :raw-html:`<br />`

            .. note::
                The following `pip`_ options are already supplied by this class:

                * -U, --upgrade 
                * --proxy

            :raw-html:`<br />` :raw-html:`<br />`

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

        if (installOptions is None):
            installOptions = []

        try:
            return importlib.import_module(module)
        except ModuleNotFoundError:
            proxyOptions = ["--proxy", self.proxy] if (self.proxy is not None) else []

            pip.main(['install', '-U'] + proxyOptions + self.options + installOptions + [installName])

        result = importlib.import_module(module)
        if (save):
            self._packages[module] = result
        
        return result
    
    def get(self, packageData: PackageData, installOptions: Optional[List[str]] = None):
        """
        Retrieves an external package

        Parameters
        ----------
        packageData: :class:`PackageData`
            The data needed for install the external package

        installOptions: Optional[List[:class:`str`]]
            Additional installation options to supply to `pip`_

            .. note::
                Please see the ``installOptions`` argument in :meth:`load` for more details

        Returns
        -------
        `Module`_
            The module to the external package
        """

        result = None
        try:
            result = self._packages[packageData.module]
        except KeyError:
            result = self.load(packageData.module, installName = packageData.installName, installOptions = installOptions)

        return result
##### EndScript