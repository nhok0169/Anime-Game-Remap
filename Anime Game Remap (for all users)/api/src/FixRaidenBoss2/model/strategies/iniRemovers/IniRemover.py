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
from typing import TYPE_CHECKING, Set
##### EndExtImports

##### LocalImports
from ....constants.IniConsts import IniKeywords, IniBoilerPlate
from ....tools.TextTools import TextTools
from ....tools.files.FileDownload import FileDownload
from ...IniSectionGraph import IniSectionGraph
from ..texEditors.BaseTexEditor import BaseTexEditor
from .BaseIniRemover import BaseIniRemover

if (TYPE_CHECKING):
    from ...files.IniFile import IniFile
##### EndLocalImports


##### Script
class IniRemover(BaseIniRemover):
    """
    This class inherits from :class:`BaseIniRemover`

    Class for the basic removal of the fixes from .ini files
    
    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to remove the fix from
    """

    _fixRemovalPattern = re.compile(f"(; {IniBoilerPlate.OldHeading.value.open()}((.|\n)*?); {IniBoilerPlate.OldHeading.value.close()[:-2]}(-)*)|(; {IniBoilerPlate.DefaultHeading.value.open()}((.|\n)*?); {IniBoilerPlate.DefaultHeading.value.close()[:-2]}(-)*)")
    _removalPattern = re.compile(f"^\s*\[.*{IniKeywords.Remap.value}(" + IniKeywords.Blend.value + "|" + IniKeywords.Position.value + r"|Fix|Tex).*\]")
    _sectionRemovalPattern = re.compile(f".*{IniKeywords.Remap.value}(" + IniKeywords.Blend.value + "|" + IniKeywords.Position.value +  r"|Fix|Tex).*")
    _remapTexRemovalPattern = re.compile(IniKeywords.Resource.value + f".*" + IniKeywords.RemapTex.value + r".*")
    _remapDLRemovalPattern = re.compile(IniKeywords.Resource.value + f".*" + IniKeywords.RemapDL.value + r".*")

    def __init__(self, iniFile: "IniFile"):
        super().__init__(iniFile)

    #_makeRemovalRemapBlendModels(sectionNames): Retrieves the data needed for removing Blend.buf files from the .ini file
    def _makeRemovalRemapBlendModels(self, sectionNames: Set[str]):
        ifTemplates = self.iniFile.sectionIfTemplates
        for sectionName in sectionNames:
            if (sectionName in ifTemplates):
                ifTemplate = ifTemplates[sectionName]
                self.iniFile.remapBlendModels[sectionName] = self.iniFile.makeFixResourceModel(ifTemplate, toFix = {""}, getFixedFile = lambda origFile, modName: origFile)

    # _makeRemovalRemapPositionModels(sectionNames): Retrieves the data needed for removing Position.buf files from the .ini file
    def _makeRemovalRemapPositionModels(self, sectionNames: Set[str]):
        ifTemplates = self.iniFile.sectionIfTemplates
        for sectionName in sectionNames:
            if (sectionName in ifTemplates):
                ifTemplate = ifTemplates[sectionName]
                self.iniFile.remapPositionModels[sectionName] = self.iniFile.makeFixResourceModel(ifTemplate, toFix = {""}, getFixedFile = lambda origFile, modName: origFile)

    # _makeRemovalRemapTexModels(sectionNames): Retrieves the data needed for removing RemapTex.dds files from the .ini file
    def _makeRemovalRemapTexModels(self, sectionNames: Set[str]):
        ifTemplates = self.iniFile.sectionIfTemplates
        for sectionName in sectionNames:
            if (sectionName in ifTemplates):
                ifTemplate = ifTemplates[sectionName]
                self.iniFile.texAddModels[sectionName] = {}
                self.iniFile.texAddModels[sectionName][""] = self.iniFile.makeTexModel(ifTemplate, {""}, BaseTexEditor(), getFixedFile = lambda origFile, modName: origFile)

    # _makeRemovalRemapDLModels(sectionNames): Retrieves the data needed for removing RemapDL files from the .ini file
    def _makeRemovalRemapDLModels(self, sectionNames: Set[str]):
        ifTemplates = self.iniFile.sectionIfTemplates
        for sectionName in sectionNames:
            if (sectionName in ifTemplates):
                ifTemplate = ifTemplates[sectionName]
                self.iniFile.fileDownloadModels[sectionName] = {}
                self.iniFile.fileDownloadModels[sectionName] = self.iniFile.makeDLModel(ifTemplate, FileDownload("", ""))

    # _getRemovalResourceByKey(sectionsToRemove, key): Retrieves the names of specific resource sections
    #   to remove based off the 'key' that holds the resource
    def _getRemovalResourceByKey(self, sectionsToRemove: Set[str], key: str) -> Set[str]:
        result = set()
        allSections = self.iniFile.getIfTemplates()
        removalSectionGraph = IniSectionGraph(sectionsToRemove, allSections)
        self.iniFile.getResources(removalSectionGraph, lambda part: key in part, lambda part: part.getVals(key),
                                  lambda resource, part: result.update(set(resource)))

        result = set(filter(lambda section: re.match(self._sectionRemovalPattern, section), result))
        return result

    # _getRemovalBlendResource(sectionsToRemove): Retrieves the names of the Blend.buf resource sections to remove
    def _getRemovalBlendResource(self, sectionsToRemove: Set[str]) -> Set[str]:
        return self._getRemovalResourceByKey(sectionsToRemove, IniKeywords.Vb1.value)
    
    # _getRemovalPositionResource(sectionsToRemove): Retrieves the names of the Position.buf resource sections to remove
    def _getRemovalPositionResource(self, sectionsToRemove: Set[str]) -> Set[str]:
        return self._getRemovalResourceByKey(sectionsToRemove, IniKeywords.Vb0.value)
    
    # _getRemovalTexResource(sectionToRemove): Retrieves the names of the texture resource sections to remove
    def _getRemovalTexResource(self, sectionsToRemove: Set[str]) -> Set[str]:
        return set(filter(lambda section: re.match(self._remapTexRemovalPattern, section), sectionsToRemove))
    
    # _getRemovalDLResource(sectionsToRemove): Retrieves the names of the download resource sections to remove
    def _getRemovalDLResource(self, sectionsToRemove: Set[str]) -> Set[str]:
        return set(filter(lambda section: re.match(self._remapDLRemovalPattern, section), sectionsToRemove))

    @BaseIniRemover._readLines
    def _removeScriptFix(self, parse: bool = False, writeBack: bool = True) -> str:
        """
        Removes the dedicated section of the code in the .ini file that this script has made

        Parameters
        ----------
        parse: :class:`bool`
            Whether to keep track of the Blend.buf files that also need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        writeBack: :class:`bool`
            Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file
        """

        if (not parse):
            self.iniFile._fileTxt = re.sub(self._fixRemovalPattern, "", self.iniFile._fileTxt)
        else:
            removedSectionsIndices = []
            txtLinesToRemove = []

            # retrieve the indices the dedicated section is located
            rangesToRemove = [match.span() for match in re.finditer(self._fixRemovalPattern, self.iniFile._fileTxt)]
            for range in rangesToRemove:
                start = range[0]
                end = range[1]
                txtLines = TextTools.getTextLines(self.iniFile._fileTxt[start : end])

                removedSectionsIndices.append(range)
                txtLinesToRemove += txtLines

            # retrieve the names of the sections the dedicated sections reference
            sectionNames = set()
            for line in txtLinesToRemove:
                if (re.match(self.iniFile._sectionPattern, line)):
                    sectionName = self.iniFile._getSectionName(line)
                    sectionNames.add(sectionName)

            blendResourceSections = self._getRemovalBlendResource(sectionNames)
            positionResourceSections = self._getRemovalPositionResource(sectionNames)
            texSections = self._getRemovalTexResource(sectionNames)
            dlSections = self._getRemovalDLResource(sectionNames)

            # get the required files that need to be removed
            self._makeRemovalRemapBlendModels(blendResourceSections)
            self._makeRemovalRemapPositionModels(positionResourceSections)
            self._makeRemovalRemapTexModels(texSections)
            self._makeRemovalRemapDLModels(dlSections)

            for sectionName in sectionNames:
                self.iniFile.sectionIfTemplates.pop(sectionName, None)
            
            # remove the dedicated section
            self.iniFile._fileTxt = TextTools.removeParts(self.iniFile._fileTxt, removedSectionsIndices)

        self.iniFile.fileTxt = self.iniFile._fileTxt.strip()

        result = ""
        if (writeBack):
            result = self.iniFile.write()
            self.iniFile.clearRead()
        else:
            result = self.iniFile._fileTxt

        self.iniFile._isFixed = False
        return result

    @BaseIniRemover._readLines
    def _removeFixSections(self, parse: bool = False, writeBack: bool = True) -> str:
        """
        Removes the [.*RemapBlend.*] sections of the .ini file that this script has made

        Parameters
        ----------
        parse: :class:`bool`
            Whether to keep track of the Blend.buf files that also need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        writeBack: :class:`bool`
            Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file
        """

        if (not parse):
            self.iniFile.removeSectionOptions(self._removalPattern)
        else:
            sectionsToRemove = self.iniFile.getSectionOptions(self._removalPattern, postProcessor = self.iniFile._removeSection)

            sectionNames = set()
            removedSectionIndices = []

            # get the indices and sections to remove
            for sectionName in sectionsToRemove:
                sectionRanges = sectionsToRemove[sectionName]
                sectionNames.add(sectionName)

                for range in sectionRanges:
                    removedSectionIndices.append(range)

            blendResourceSections = self._getRemovalBlendResource(sectionNames)
            positionResourceSections = self._getRemovalPositionResource(sectionNames)
            texSections = self._getRemovalTexResource(sectionNames)
            dlSections = self._getRemovalDLResource(sectionNames)

            self._makeRemovalRemapBlendModels(blendResourceSections)
            self._makeRemovalRemapPositionModels(positionResourceSections)
            self._makeRemovalRemapTexModels(texSections)
            self._makeRemovalRemapDLModels(dlSections)

            for sectionName in sectionNames:
                self.iniFile.sectionIfTemplates.pop(sectionName, None)

            self.iniFile.fileLines = TextTools.removeLines(self.iniFile.fileLines, removedSectionIndices)

        result = ""
        if (writeBack):
            result = self.iniFile.write()
            self.iniFile.clearRead()
        else:
            result = self.iniFile._fileTxt

        self.iniFile._isFixed = False
        return result

    @BaseIniRemover._readLines
    def _removeFixComment(self, writeBack: bool = True) -> str:
        """
        Removes the ";RemapFixHideOrig -->" comment prefix that this script has made

        Parameters
        ----------
        writeBack: :class:`bool`
            Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file
        """

        self.iniFile.fileTxt = self.iniFile.fileTxt.replace(IniKeywords.HideOriginalComment.value, "")

        result = ""
        if (writeBack):
            result = self.iniFile.write()
            self.iniFile.clearRead()
        else:
            result = self.iniFile._fileTxt

        self.iniFile._isFixed = False
        self.iniFile._hideOriginalReplaced = True
        return result

    def remove(self, parse: bool = False, writeBack: bool = True) -> str:
        if (not self.iniFile.isModIni):
            parse = False

        self._removeScriptFix(parse = parse, writeBack = False)  
        self._removeFixSections(parse = parse, writeBack = False)
        result = self._removeFixComment(writeBack = writeBack)

        return result
##### EndScript