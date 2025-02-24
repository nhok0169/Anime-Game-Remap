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
from functools import lru_cache 
from typing import Type, List, Any, Dict, TYPE_CHECKING, Optional, Union, Callable, Tuple
##### EndExtImports

##### LocalImports
from ....tools.Builder import Builder
from .BaseIniParser import BaseIniParser

if (TYPE_CHECKING):
    from ...assets.IniParseBuilderArgs import IniParseBuilderArgs
    from ...files.IniFile import IniFile
##### EndLocalImports


##### Script
class IniParseBuilder(Builder[BaseIniParser]):
    """
    This class inherits from :class:`Builder`

    A class to help dynamically build a :class:`BaseIniParser`

    Parameters
    ----------
    buildCls: Union[Type[:class:`BaseIniParser`], :class:`IniParseBuilderArgs`]
        Either:
        
        #. The class to construct a :class:`BaseIniFixer` OR
        #. Some provider that gives the required arguments needed for this class

    args: Optional[List[Any]]
        The constant arguments used to build the object

        .. note::
            If the :attr:`buildCls` attribute is not a class of type Type[:class:`BaseIniParser`], then
            this parameter has no effect

        **Default**: ``None``

    kwargs: Optional[Dict[str, Any]]
        The constant keyword arguments used to build the object

        .. note::
            If the :attr:`buildCls` attribute is not a class of type Type[:class:`BaseIniParser`], then
            this parameter has no effect

        **Default**: ``None``

    Attributes
    ----------
    _buildCls: Optional[Type[:class:`BaseIniParser`]]
        The class for the parser, if available

    _builderArgs: Optional[:class:`IniParseBuilderArgs`]
        The provider for the arguments of this class, if available
    """

    def __init__(self, buildCls: Union[Type[BaseIniParser], "IniParseBuilderArgs"], args: Optional[List[Any]] = None, kwargs: Optional[Dict[str, Any]] = None):
        super().__init__(buildCls, args, kwargs)
        
        builderArgsProvided = not isinstance(self._buildCls, type)

        self._builderArgs = buildCls if (builderArgsProvided) else None
        if (builderArgsProvided):
            self._buildCls = None

    @lru_cache(maxsize = 64)
    def _getBuilderArgs(self, modName: str, version: Optional[int] = None) -> Callable[[], Tuple[BaseIniParser, List[Any], Dict[str, Any]]]:
        builderArgsGenerator = self._builderArgs.get(modName, version = version)
        return builderArgsGenerator()

    def build(self, iniFile: "IniFile", modName: Optional[str] = None, version: Optional[int] = None) -> BaseIniParser:
        """
        Builds the parser

        Parameters
        ----------
        iniFile: :class:`IniFile`
            The .ini file to parse

        modeName: Optional[:class:`str`]
            The name of the mod to build the parser for :raw-html:`<br />` :raw-html:`<br />`

            If this argument is ``None``, then will use the mod name extracted from :attr:`IniFile.availableType`

            .. warning::
                This argument has no effect if :attr:`_buildCls` is not ``None`` 

            **Default**: ``None``

        version: Optional[:class:`int`]
            The game version to fix the mod to :raw-html:`<br />` :raw-html:`<br />`

            If this argument is ``None``, will build the parser for the latest version of the game

            .. warning::
                This argument has no effect if :attr:`_buildCls` is not ``None`` 

            **Default**: ``None``
        
        Returns
        -------
        :class:`BaseIniParser`
            The built parser
        """

        if (modName is None):
            modName = iniFile.availableType

        if (self._builderArgs is not None):
            self._buildCls, self._args, self._kwargs, = self._getBuilderArgs(modName, version = version)

        return super().build(iniFile)
##### EndScript