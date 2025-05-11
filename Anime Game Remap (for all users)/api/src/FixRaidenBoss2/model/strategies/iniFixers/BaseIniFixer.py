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
from typing import Optional, Callable, Union, Any, Dict, List
##### EndExtImports

##### LocalImports
from ....constants.IfPredPartType import IfPredPartType
from ....constants.IniConsts import IniKeywords
from ...IniSectionGraph import IniSectionGraph
from ....exceptions.NoModType import NoModType
from ..iniParsers.BaseIniParser import BaseIniParser
from ...iftemplate.IfTemplate import IfTemplate
from ...iftemplate.IfPredPart import IfPredPart
from ...iftemplate.IfContentPart import IfContentPart
##### EndLocalImports


##### Script
class BaseIniFixer():
    """
    Base class to fix a .ini file

    Parameters
    ----------
    parser: :class:`BaseIniParser`
        The associated parser to retrieve data for the fix

    Attributes
    ----------
    _parser: :class:`BaseIniParser`
        The associated parser to retrieve data for the fix

    _iniFile: :class:`IniFile`
        The .ini file that will be fixed
    """

    def __init__(self, parser: BaseIniParser):
        self._parser = parser
        self._iniFile = parser._iniFile
        self._fixId = 0

    def clear(self):
        """
        Resets any saved states within the fixer
        """

        self._fixId

    # _getAssetReplacement(assset, assetRepoAttName, notFoundStr): Retrieves the replacement for 'asset'
    def _getAssetReplacement(self, asset: str, assetRepoAttName: str, modName: str, notFoundVal: Any = None) -> Union[str, Any]:
        """
        Retrieves the replacement for 'asset'

        Parameters
        ----------
        asset: :class:`str`
            The asset to be replaced

        assetRepoAttName: :class:`str`
            The name of the :class:`ModIdAssets` repo in :meth:`IniFile.availableType`

        modName: :class:`str`
            The name of the mod we want the replacement for

        notFoundVal: Any
            The value to be returns if the replacement is not found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Union[:class:`str`, Any]
            The found replacement asset or the value from 'notFoundVal' if the replacement was not found
        """

        result = ""
        type = self._iniFile.availableType

        if (type is not None):
            assetRepo = getattr(type, assetRepoAttName)
            result = assetRepo.replace(asset, version = self._iniFile.version, toAssets = modName)
        else:
            raise NoModType()

        if (result is None):
            return notFoundVal
        return result

    def _getAsset(self, assetType: str, assetRepoAttName: str, modName: str, notFoundVal: Any = None) -> Union[str, Any]:
        """
        Retrieves the corresponding asset

        Parameters
        ----------
        assetType: :class:`str`
            The name for the type of asset to retrieve

        assetRepoAttName: :class:`str`
            The name of the :class:`ModIdAssets` repo in :meth:`IniFile.availableType`

        modName: :class:`str`
            The name of the mod we want the asset for

        notFoundVal: Any
            The value to be returned if the replacement is not found :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Union[:class:`str`, Any]
            The found asset or the value from 'notFoundVal' if the asset was not found
        """

        result = ""
        type = self._iniFile.availableType

        if (type is not None):
            assetRepo = getattr(type, assetRepoAttName)

            try:
                result = assetRepo.get(modName, assetType, version = self._iniFile.version)
            except:
                result = notFoundVal
        else:
            raise NoModType()

        return result

    # _getHashReplacement(hash): Retrieves the hash replacement for 'hash'
    def _getHashReplacement(self, hash: str, modName: str) -> str:
        """
        Retrieves the replacement for 'hash'

        Parameters
        ----------
        hash: :class:`str`
            The hash to be replaced

        modName: :class:`str`
            The name of the mod we want the replacement for

        Returns
        -------
        :class:`str`
            The corresponding replacement for the hash or "HashNotFound" if there are not replacements
        """

        return self._getAssetReplacement(hash, "hashes", modName, notFoundVal = IniKeywords.HashNotFound.value)
    
    # _getIndexReplacement(index): Retrieves the index replacement for 'index'
    def _getIndexReplacement(self, index: str, modName: str) -> str:
        """
        Retrieves the replacement for 'index'

        Parameters
        ----------
        index: :class:`str`
            The index to be replaced

        modName: :class:`str`
            The name of the mod we want the replacement for

        Returns
        -------
        :class:`str`
            The corresponding replacement for the index or "IndexNotFound" if there are not replacements
        """

        return self._getAssetReplacement(index, "indices", modName, notFoundVal = IniKeywords.IndexNotFound.value)
    
    def _getHash(self, hashType: str, modName: str) -> str:
        """
        Retrieves the corresponding hash

        Parameters 
        ----------
        hashType: :class:`str`
            The name for the type of hash to retrieve

        modName: :class:`str`
            The name for the type of mod to get the hash from

        Returns
        -------
        :class:`str`
            The found hash or "HashNotFound" if the corresponding hash is not found        
        """

        return self._getAsset(hashType, "hashes", modName, notFoundVal = IniKeywords.HashNotFound.value)
    
    def _getIndex(self, indexType: str, modName: str) -> str:
        """
        Retrieves the corresponding index

        Parameters 
        ----------
        indexType: :class:`str`
            The name for the type of index to retrieve

        modName: :class:`str`
            The name for the type of mod to get the index from

        Returns
        -------
        :class:`str`
            The found index or "IndexNotFound" if the corresponding index is not found     
        """

        return self._getAsset(indexType, "indices", modName, notFoundVal = IniKeywords.IndexNotFound.value)

    # _getRemapName(sectionName, modName, sectionGraph, remapNameFunc): Retrieves the required remap name for the fix
    def _getRemapName(self, sectionName: str, modName: str, sectionGraph: Optional[IniSectionGraph] = None, remapNameFunc: Optional[Callable[[str, str], str]] = None) -> str:
        error = False
        if (sectionGraph is None):
            error = True

        if (not error):
            try:
                return sectionGraph.remapNames[sectionName][modName]
            except KeyError:
                error = True

        if (sectionName not in self._iniFile.sectionIfTemplates):
            return sectionName

        if (remapNameFunc is None):
            remapNameFunc = self._iniFile.getRemapBlendName

        result = remapNameFunc(sectionName, modName)
        try:
            sectionGraph.remapNames[sectionName]
        except KeyError:
            sectionGraph.remapNames[sectionName] = {}

        sectionGraph.remapNames[sectionName][modName] = result
        return result

    # fills the if..else template in the .ini for each section
    def fillIfTemplate(self, modName: str, sectionName: str, ifTemplate: IfTemplate, fillFunc: Callable[[str, str, IfContentPart, int, int, str], str], origSectionName: Optional[str] = None) -> str:
        """
        Creates a new :class:`IfTemplate` for an existing `section`_ in the .ini file

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name of the `section`_

        ifTemplate: :class:`IfTemplate`
            The :class:`IfTemplate` of the orginal `section`_

        fillFunc: Callable[[:class:`str`, :class:`str`, :class:`IfContentPart`, :class:`int`, :class:`str`, :class:`str`], :class:`str`]]
            The function to create a new **content part** for the new :class:`IfTemplate`
            :raw-html:`<br />` :raw-html:`<br />`

            .. tip::
                For more info about an 'IfTemplate', see :class:`IfTemplate`

            :raw-html:`<br />`
            The parameter order for the function is:

            #. The name for the type of mod to fix to
            #. The new section name
            #. The corresponding **content part** in the original :class:`IfTemplate`
            #. The index for the content part in the original :class:`IfTemplate`
            #. The string to prefix every line in the **content part** of the :class:`IfTemplate`
            #. The original name of the section

        origSectionName: Optional[:class:`str`]
            The original name of the section.

            If this argument is set to ``None``, then will assume this argument has the same value as the argument for ``sectionName`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`str`
            The text for the newly created :class:`IfTemplate`
        """

        addFix = f"[{sectionName}]\n"
        partIndex = 0
        linePrefix = ""

        if (origSectionName is None):
            origSectionName = sectionName

        for part in ifTemplate:
            # adding in the if..else statements
            if (isinstance(part, IfPredPart)):
                addFix += part.pred
                
                linePrefix = re.match(r"^[( |\t)]*", part.pred)
                if (linePrefix):
                    linePrefix = linePrefix.group(0)
                    linePrefixLen = len(linePrefix)

                    linePrefix = part.pred[:linePrefixLen]

                    if (part.type != IfPredPartType.EndIf):
                        linePrefix += "\t"
                partIndex += 1
                continue
            
            # add in the content within the if..else statements
            addFix += fillFunc(modName, sectionName, part, partIndex, linePrefix, origSectionName)

            partIndex += 1
            
        return addFix

    def getFix(self, fixStr: str = "") -> str:
        """
        Retrieves the text to fix the .ini file
        """
        pass

    # _fix(keepBackup, fixOnly, update, hideOrig, withBoilerPlate, withSrc): Internal function to fix the .ini file
    def _fix(self, keepBackup: bool = True, fixOnly: bool = False, update: bool = False, hideOrig: bool = False, withBoilerPlate: bool = True, withSrc: bool = True, fixId: int = 0) -> Union[str, List[str]]:
        self._fixId = fixId
        return self._iniFile._fix(keepBackup = keepBackup, fixOnly = fixOnly, update = update, hideOrig = hideOrig, withBoilerPlate = withBoilerPlate, withSrc = withSrc)

    def fix(self, keepBackup: bool = True, fixOnly: bool = False, update: bool = False, hideOrig: bool = False) -> Union[str, List[str]]:
        """
        Fixes the .ini file

        Parameters
        ----------
        keepBackup: :class:`bool`
            Whether to keep backups for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether to only fix the .ini file without undoing any fixes :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        update: :class:`bool`
            Whether to also update the source text in the :class:`IniFile` object with the latest fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        hideOrig: :class:`bool`
            Whether to hide the mod for the original character :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        Union[:class:`str`, List[:class:`str`]]
            The new content of the .ini file which includes the fix and the new content of any other newly created .ini files related to fixing the particular .ini file
        """

        self._fixId = 0
        return self._fix(keepBackup = keepBackup, fixOnly = fixOnly, update = update, hideOrig = hideOrig)
##### EndScript
