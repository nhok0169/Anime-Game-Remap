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
from typing import Callable
##### EndExtImports

##### LocalImports
from ....constants.IniConsts import IniKeywords
from .BaseIniFixer import BaseIniFixer
from ..iniParsers.GIMIParser import GIMIParser
from ....tools.Heading import Heading
from ...iftemplate.IfContentPart import IfContentPart
from ...IniSectionGraph import IniSectionGraph
##### EndLocalImports


##### Script
class GIMIFixer(BaseIniFixer):
    """
    This class inherits from :class:`BaseIniFixer`

    Fixes a .ini file used by a GIMI related importer

    Parameters
    ----------
    parser: :class:`GIMIParser`
        The associated parser to retrieve data for the fix
    """

    def __init__(self, parser: GIMIParser):
        super().__init__(parser)


    def _fillTextureOverrideRemapBlend(self, modName: str, sectionName: str, part: IfContentPart, partIndex: int, linePrefix: str, origSectionName: str) -> str:
        """
        Creates the **content part** of an :class:`IfTemplate` for the new sections created by this fix related to the ``[TextureOverride.*Blend.*]`` `sections`_

        .. tip::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the section

        part: :class:`IfContentPart`
            The content part of the :class:`IfTemplate` of the original [TextureOverrideBlend] `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName, varValue, _, _ in part:
            # filling in the subcommand
            if (varName == IniKeywords.Run.value):
                subCommandName = self._getRemapName(varValue, modName, sectionGraph = self._parser.blendCommandsGraph)
                subCommandStr = f"{IniKeywords.Run.value} = {subCommandName}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # filling in the hash
            elif (varName == IniKeywords.Hash.value):
                hash = self._getHashReplacement(varValue, modName)
                addFix += f"{linePrefix}{IniKeywords.Hash.value} = {hash}\n"

            # filling in the vb1 resource
            elif (varName == IniKeywords.Vb1.value):
                blendName = varValue
                remapBlendName = self._getRemapName(blendName, modName, sectionGraph = self._parser.blendResourceCommandsGraph, remapNameFunc = self._iniFile.getRemapBlendResourceName)
                fixStr = f'{IniKeywords.Vb1.value} = {remapBlendName}'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the handling
            elif (varName == IniKeywords.Handling.value):
                fixStr = f'{IniKeywords.Handling.value} = skip'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the draw value
            elif (varName == IniKeywords.Draw.value):
                fixStr = f'{IniKeywords.Draw.value} = {varValue}'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the indices
            elif (varName == IniKeywords.MatchFirstIndex.value):
                index = self._getIndexReplacement(varValue, modName)
                addFix += f"{linePrefix}{IniKeywords.MatchFirstIndex.value} = {index}\n"

            else:
                addFix += f"{linePrefix}{varName} = {varValue}\n"
                
        return addFix
    
    def _fillTextureOverrideRemapPosition(self, modName: str, sectionName: str, part: IfContentPart, partIndex: int, linePrefix: str, origSectionName: str) -> str:
        """
        Creates the **content part** of an :class:`IfTemplate` for the new sections created by this fix related to the ``[TextureOverride.*Position.*]`` `sections`_

        .. tip::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the section

        part: :class:`IfContentPart`
            The content part of the :class:`IfTemplate` of the original [TextureOverridePosition] `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName, varValue, _, _ in part:
            # filling in the subcommand
            if (varName == IniKeywords.Run.value):
                subCommandName = self._getRemapName(varValue, modName, sectionGraph = self._parser.positionCommandsGraph)
                subCommandStr = f"{IniKeywords.Run.value} = {subCommandName}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # filling in the hash
            elif (varName == IniKeywords.Hash.value):
                hash = self._getHashReplacement(varValue, modName)
                addFix += f"{linePrefix}{IniKeywords.Hash.value} = {hash}\n"

            # filling in the vb0 resource
            elif (varName == IniKeywords.Vb0.value):
                positionName = varValue
                remapPositionName = self._getRemapName(positionName, modName, sectionGraph = self._parser.positionResourceCommandsGraph, remapNameFunc = self._iniFile.getRemapPositionResourceName)
                fixStr = f'{IniKeywords.Vb0.value} = {remapPositionName}'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the indices
            elif (varName == IniKeywords.MatchFirstIndex.value):
                index = self._getIndexReplacement(varValue, modName)
                addFix += f"{linePrefix}{IniKeywords.MatchFirstIndex.value} = {index}\n"

            else:
                addFix += f"{linePrefix}{varName} = {varValue}\n"
                
        return addFix
    
    def _fillNonBlendSections(self, modName: str, sectionName: str, part: IfContentPart, partIndex: int, linePrefix: str, origSectionName: str) -> str:
        """
        Creates the **content part** of an :class:`IfTemplate` for the new sections created by this fix that are not related to the ``[TextureOverride.*Blend.*]`` `sections`_

        .. tip::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the section

        part: :class:`IfContentPart`
            The content part of the :class:`IfTemplate` of the original [TextureOverrideBlend] `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName, varValue, _, _ in part:
            # filling in the hash
            if (varName == IniKeywords.Hash.value):
                newHash = self._getHashReplacement(varValue, modName)
                addFix += f"{linePrefix}{IniKeywords.Hash.value} = {newHash}\n"

            # filling in the subcommand
            elif (varName == IniKeywords.Run.value):
                subCommand = self._getRemapName(varValue, modName, sectionGraph = self._parser.nonBlendHashIndexCommandsGraph, remapNameFunc = self._iniFile.getRemapFixName)
                subCommandStr = f"{IniKeywords.Run.value} = {subCommand}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # filling in the index
            elif (varName == IniKeywords.MatchFirstIndex.value):
                newIndex = self._getIndexReplacement(varValue, modName)
                addFix += f"{linePrefix}{IniKeywords.MatchFirstIndex.value} = {newIndex}\n"

            else:
                addFix += f"{linePrefix}{varName} = {varValue}\n"

        return addFix
    

    # fill the attributes for the sections related to the resources
    def _fillRemapBlendResource(self, modName: str, sectionName: str, part: IfContentPart, partIndex: int, linePrefix: str, origSectionName: str):
        """
        Creates the **content part** of an :class:`IfTemplate` for the new `sections`_ created by this fix related to the ``[Resource.*Blend.*]`` `sections`_

        .. tip::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the `section`_

        part: :class:`IfContentPart`
            The content part of the :class:`IfTemplate` of the original ``[Resource.*Blend.*]`` `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName, varValue, keyInd, _ in part:
            # filling in the subcommand
            if (varName == IniKeywords.Run.value):
                subCommand = self._getRemapName(varValue, modName, sectionGraph = self._parser.blendResourceCommandsGraph, remapNameFunc = self._iniFile.getRemapBlendResourceName)
                subCommandStr = f"{IniKeywords.Run.value} = {subCommand}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # add in the type of file
            elif (varName == "type"):
                addFix += f"{linePrefix}type = Buffer\n"

            # add in the stride for the file
            elif (varName == "stride"):
                addFix += f"{linePrefix}stride = 32\n"

            # add in the file
            elif (varName == "filename"):
                remapModel = self._iniFile.remapBlendModels[origSectionName]
                fixedBlendFile = remapModel.fixedPaths[partIndex][modName][keyInd]
                addFix += f"{linePrefix}filename = {fixedBlendFile}\n"

            else:
                addFix += f"{linePrefix}{varName} = {varValue}\n"

        return addFix

    def _fillRemapPositionResource(self, modName: str, sectionName: str, part: IfContentPart, partIndex: int, linePrefix: str, origSectionName: str):
        """
        Creates the **content part** of an :class:`IfTemplate` for the new `sections`_ created by this fix related to the ``[Resource.*Position.*]`` `sections`_

        .. tip::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the `section`_

        part: :class:`IfContentPart`
            The content part of the :class:`IfTemplate` of the original ``[Resource.*Position.*]`` `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName, varValue, keyInd, _ in part:
            # filling in the subcommand
            if (varName == IniKeywords.Run.value):
                subCommand = self._getRemapName(varValue, modName, sectionGraph = self._parser.positionResourceCommandsGraph, remapNameFunc = self._iniFile.getRemapPositionResourceName)
                subCommandStr = f"{IniKeywords.Run.value} = {subCommand}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # add in the type of file
            elif (varName == "type"):
                addFix += f"{linePrefix}type = Buffer\n"

            # add in the stride for the file
            elif (varName == "stride"):
                addFix += f"{linePrefix}stride = 40\n"

            # add in the file
            elif (varName == "filename"):
                remapModel = self._iniFile.remapPositionModels[origSectionName]
                fixedPositionFile = remapModel.fixedPaths[partIndex][modName][keyInd]
                addFix += f"{linePrefix}filename = {fixedPositionFile}\n"

            else:
                addFix += f"{linePrefix}{varName} = {varValue}\n"

        return addFix
    
    # _fixElementCommands(modName, commandGraph, remapNameFunc, fillFunc, fix, 
    #                     includeEndNewLine, addToRemapSections): Get the fix string for a specific type of element
    def _fixElementCommands(self, modName: str, commandGraph: IniSectionGraph, remapNameFunc: Callable[[str, str], str],
                            fillFunc: Callable[[str, str, IfContentPart, int, int, str], str], fix: str = "", includeEndNewLine: bool = True,
                            addToRemapSections: bool = False):

        commandTuples = commandGraph.runSequence
        commandsLen = len(commandTuples)
        for i in range(commandsLen):
            commandTuple = commandTuples[i]
            section = commandTuple[0]
            ifTemplate = commandTuple[1]

            if (addToRemapSections):
                self._iniFile._remappedSectionNames.add(section)

            resourceName = self._getRemapName(section, modName, sectionGraph = commandGraph, remapNameFunc = remapNameFunc)
            fix += self.fillIfTemplate(modName, resourceName, ifTemplate, fillFunc, origSectionName = section)

            if (includeEndNewLine or i < commandsLen - 1):
                fix += "\n"

        return fix

    # _fixBlendCommands(modName, fix): Get the fix string for all the texture override blend sections
    def _fixBlendCommands(self, modName: str, fix: str = ""):
        return self._fixElementCommands(modName, self._parser.blendCommandsGraph, self._iniFile.getRemapBlendName, 
                                        self._fillTextureOverrideRemapBlend, fix =  fix, addToRemapSections = True)
    
    # _fixPositionCommands(modName, fix): Get the fix string for all the texture override position sections
    def _fixPositionCommands(self, modName: str, fix: str = ""):
        return self._fixElementCommands(modName, self._parser.positionCommandsGraph, self._iniFile.getRemapPositionName, 
                                        self._fillTextureOverrideRemapPosition, fix = fix, addToRemapSections = True)
    
    # _fixNonBlendHashIndexCommands(modName, fix): get the fix string for non-blend sections
    def _fixNonBlendHashIndexCommands(self, modName: str, fix: str = ""):
        return self._fixElementCommands(modName, self._parser.nonBlendHashIndexCommandsGraph, self._iniFile.getRemapFixName,
                                        self._fillNonBlendSections, fix = fix, addToRemapSections = True)
    
    # _fixBlendResourceCommands(modName, fix, includeEndNewLine): get the fix string for the blend resources
    def _fixBlendResourceCommands(self, modName: str, fix: str = "", includeEndNewLine: bool = True):
        return self._fixElementCommands(modName, self._parser.blendResourceCommandsGraph, 
                                         self._iniFile.getRemapBlendResourceName, self._fillRemapBlendResource, fix = fix, includeEndNewLine = includeEndNewLine)
    
    # _fixPositionResourceCommands(modName, fix): get the fix string for the position resources
    def _fixPositionResourceCommands(self, modName: str, fix: str = ""):
        return self._fixElementCommands(modName, self._parser.positionResourceCommandsGraph, 
                                         self._iniFile.getRemapPositionResourceName, self._fillRemapPositionResource, fix = fix, includeEndNewLine = False)

    def fixMod(self, modName: str, fix: str = "") -> str:
        """
        Generates the newly added code in the .ini file for the fix of a single type of mod

        .. note::
            eg.
                If we are making the fix from ``Jean`` -> ``JeanCN`` and ``JeanSeaBreeze``,
                The code below will only make the fix for ``JeanCN``

            .. code-block::

                fixMod("JeanCN")


        Parameters
        ----------
        modName: :class:`str`
            The name of the mod to fix

        fix: :class:`str`
            Any existing text we want the result of the fix to add onto :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ""

        Returns
        -------
        :class:`str`
            The text for the newly generated code in the .ini file
        """

        hasPositionSections = bool(self._parser.positionCommandsGraph.sections)
        hasNonBlendSections = bool(self._parser.nonBlendHashIndexCommandsGraph.sections)
        hasBlendResources = bool(self._iniFile.remapBlendModels)
        hasPositionResources = bool(self._iniFile.remapPositionModels)

        if (self._parser.blendCommandsGraph.sections or hasBlendResources or hasNonBlendSections or hasPositionSections):
            fix += "\n"

        fix = self._fixBlendCommands(modName, fix = fix)
        if (hasPositionSections):
            fix += "\n"

        fix = self._fixPositionCommands(modName, fix = fix)
        if (hasNonBlendSections):
            fix += "\n"

        fix = self._fixNonBlendHashIndexCommands(modName, fix = fix)
        if (hasBlendResources):
            fix += "\n"

        fix = self._fixBlendResourceCommands(modName, fix = fix, includeEndNewLine = False)
        if (hasPositionResources):
            fix += "\n"

        fix = self._fixPositionResourceCommands(modName, fix = fix)
        return fix

    def getFix(self, fixStr: str = ""):
        heading = Heading("", sideLen = 5, sideChar = "*")
        sortedModsToFix = list(self._parser._modsToFix)
        sortedModsToFix.sort()

        for modName in sortedModsToFix:
            heading.title = modName
            currentFix = self.fixMod(modName)

            if (currentFix):
                fixStr += f"\n\n; {heading.open()}{currentFix}\n; {heading.close()}"

        return fixStr
##### EndScript
