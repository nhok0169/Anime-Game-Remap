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
import re
from typing import TYPE_CHECKING, Set
##### EndExtImports

##### LocalImports
from ....constants.IniConsts import IniKeywords, IniBoilerPlate
from ....tools.TextTools import TextTools
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
    _removalPattern = re.compile(f"^\s*\[.*(" + IniKeywords.RemapBlend.value + "|" + IniKeywords.RemapFix.value + "|" + IniKeywords.RemapTex.value + r").*\]")
    _sectionRemovalPattern = re.compile(f".*(" + IniKeywords.RemapBlend.value + "|" + IniKeywords.RemapFix.value + "|" + IniKeywords.RemapTex.value + r").*")
    _remapTexRemovalPattern = re.compile(IniKeywords.Resource.value + f".*" + IniKeywords.RemapTex.value + r".*")

    def __init__(self, iniFile: "IniFile"):
        super().__init__(iniFile)

    #_makeRemovalRemapBlendModels(sectionNames): Retrieves the data needed for removing Blend.buf files from the .ini file
    def _makeRemovalRemapBlendModels(self, sectionNames: Set[str]):
        for sectionName in sectionNames:
            ifTemplate = None
            try:
                ifTemplate = self.iniFile.sectionIfTemplates[sectionName]
            except KeyError:
                continue

            self.iniFile.remapBlendModels[sectionName] = self.iniFile.makeResourceModel(ifTemplate, toFix = {""}, getFixedFile = lambda origFile, modName: origFile)

    # _makeRemovalRemapTexModels(sectionNames): Retrieves the data needed for removing RemapTex.dds files from the .ini file
    def _makeRemovalRemapTexModels(self, sectionNames: Set[str]):
        for sectionName in sectionNames:
            ifTemplate = None
            try:
                ifTemplate = self.iniFile.sectionIfTemplates[sectionName]
            except KeyError:
                continue
            
            self.iniFile.texAddModels[sectionName] = {}
            self.iniFile.texAddModels[sectionName][""] = self.iniFile.makeTexModel(ifTemplate, {""}, BaseTexEditor(), getFixedFile = lambda origFile, modName: origFile)

    # _getRemovalBlendResource(sectionsToRemove): Retrieves the names of the Blend.buf resource sections to remove
    def _getRemovalBlendResource(self, sectionsToRemove: Set[str]) -> Set[str]:
        result = set()
        allSections = self.iniFile.getIfTemplates()
        removalSectionGraph = IniSectionGraph(sectionsToRemove, allSections)
        self.iniFile.getResources(removalSectionGraph, lambda part: IniKeywords.Vb1.value in part, lambda part: part.getVals(IniKeywords.Vb1.value),
                                  lambda resource, part: result.update(set(resource)))

        result = set(filter(lambda section: re.match(self._sectionRemovalPattern, section), result))
        return result
    
    # _getRemovalTexResource(sectionToRemove): Retrieves the names of the texture resource sections to remove
    def _getRemovalTexResource(self, sectionsToRemove: Set[str]) -> Set[str]:
        return set(filter(lambda section: re.match(self._remapTexRemovalPattern, section), sectionsToRemove))

    @BaseIniRemover._readLines
    def _removeScriptFix(self, parse: bool = False) -> str:
        """
        Removes the dedicated section of the code in the .ini file that this script has made

        Parameters
        ----------
        parse: :class:`bool`
            Whether to keep track of the Blend.buf files that also need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

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

            resourceSections = self._getRemovalBlendResource(sectionNames)
            texSections = self._getRemovalTexResource(sectionNames)

            # get the Blend.buf / RemapTex.dds files that need to be removed
            self._makeRemovalRemapBlendModels(resourceSections)
            self._makeRemovalRemapTexModels(texSections)
            
            # remove the dedicated section
            self.iniFile._fileTxt = TextTools.removeParts(self.iniFile._fileTxt, removedSectionsIndices)

        self.iniFile.fileTxt = self.iniFile._fileTxt.strip()
        result = self.iniFile.write()

        self.iniFile.clearRead()
        self.iniFile._isFixed = False
        return result

    @BaseIniRemover._readLines
    def _removeFixSections(self, parse: bool = False) -> str:
        """
        Removes the [.*RemapBlend.*] sections of the .ini file that this script has made

        Parameters
        ----------
        parse: :class:`bool`
            Whether to keep track of the Blend.buf files that also need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

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

            resourceSections = self._getRemovalBlendResource(sectionNames)
            texSections = self._getRemovalTexResource(sectionNames)

            self._makeRemovalRemapBlendModels(resourceSections)
            self._makeRemovalRemapTexModels(texSections)

            self.iniFile.fileLines = TextTools.removeLines(self.iniFile.fileLines, removedSectionIndices)

        result = self.iniFile.write()

        self.iniFile.clearRead()
        self.iniFile._isFixed = False
        return result

    def remove(self, parse: bool = False) -> str:
        """
        Removes the fix from the .ini file

        Parameters
        ----------
        parse: :class:`bool`
            Whether to also parse for the .*RemapBlend.buf files that need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new content of the .ini file
        """

        if (not self.iniFile.isModIni):
            parse = False

        self._removeScriptFix(parse = parse)    
        result = self._removeFixSections(parse = parse)
        return result
##### EndScript