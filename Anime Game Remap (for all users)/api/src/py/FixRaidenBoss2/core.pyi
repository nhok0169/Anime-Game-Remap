"""
C++ internal core of AGRemap
"""
from __future__ import annotations
import collections.abc
import typing
__all__: list[str] = ['BaseBufEditor', 'BaseDFA', 'BaseIniClassifier', 'BaseIniFixer', 'BaseIniGraphEdit', 'BaseIniGraphGroupEdit', 'BaseIniGraphPartEdit', 'BaseIniParser', 'BaseIniPartEdit', 'BaseIniRemover', 'BaseLogger', 'BaseRegEdit', 'BaseResEdit', 'BaseSLR1Parser', 'BaseTokenizer', 'BiMap', 'BinaryFile', 'BlendFile', 'BufBaseFloat', 'BufBaseInt', 'BufDataType', 'BufEditor', 'BufElementType', 'BufFloat', 'BufFloat16', 'BufSignedInt', 'BufType', 'BufUnSignedInt', 'BufUnorm', 'CachedFileStats', 'CallGraph', 'CppAhoCorasickDFA', 'CppAlgo', 'CppBaseIniFixer', 'CppBaseIniParser', 'CppBaseIniRemover', 'CppBasePixelTransform', 'CppBaseTexEditor', 'CppBaseTexFilter', 'CppBufFile', 'CppColour', 'CppColourRange', 'CppColourReplace', 'CppColourReplaceFilter', 'CppCorrectGamma', 'CppGammaFilter', 'CppGlobalModTypes', 'CppHashTools', 'CppHighlightShadow', 'CppHueAdjust', 'CppIniFixBuilderArgs', 'CppIniParseBuilderArgs', 'CppIniRemoveBuilderArgs', 'CppIntTools', 'CppInvertAlpha', 'CppInvertAlphaFilter', 'CppListTools', 'CppPixelFilter', 'CppTempControl', 'CppTexCreator', 'CppTexEditor', 'CppTextureFile', 'CppTintTransform', 'CppTransparency', 'CppTransparencyAdjustFilter', 'CppTrie', 'CppVersion', 'DFA', 'FileDownload', 'FileStats', 'FilteredTokenizer', 'GIBuilder', 'GIMIFixer', 'GIMIParser', 'GIMISectionClassifier', 'GameTypeId', 'GameTypeIdTools', 'GlobalRemapIniRemover', 'GraphGroupEdit', 'GraphGroupRemap', 'GraphInherit', 'GraphRemove', 'GraphRename', 'GraphTools', 'Hash128', 'Hash64', 'Hashes', 'IOrderedMultiMap', 'IbFile', 'IfContentPart', 'IfContentPartColourChange', 'IfContentPartColouring', 'IfPredParser', 'IfPredPart', 'IfPredTokenizer', 'IfTemplate', 'IfTemplateNode', 'IfTemplatePart', 'IfTemplateTree', 'Indices', 'IniClassifier', 'IniClassifyStats', 'IniDownloadModel', 'IniFile', 'IniFixBuilder', 'IniFixResource', 'IniFixResourceModel', 'IniFixingContext', 'IniGraphGroup', 'IniGroupedResource', 'IniParseBuilder', 'IniRemovalContext', 'IniRemoveBuilder', 'IniResource', 'IniResourceModel', 'IniSectionGraph', 'IniSectionGraphSectionIterator', 'IniSrcResourceModel', 'IniTexModel', 'KeyRemapData', 'Logger', 'ModAssets', 'ModDictAssets', 'ModMappedAssets', 'ModType', 'ModTypeId', 'ModTypeIdData', 'ModTypeIdTools', 'MultiModFixer', 'OrderedMultiMap', 'OrderedMultiMapIterator', 'OrderedMultiMapSqrt', 'OrderedMultiMapSqrtIterator', 'ParseContext', 'ParseNode', 'ParseTree', 'PositionFile', 'Ranges', 'RangesInt', 'RegAdd', 'RegFillMissing', 'RegNewVals', 'RegRemap', 'RegRemove', 'RegSurroundedAdd', 'RemapBlendReplace', 'RemapBlendResource', 'RemapIniDownload', 'RemapIniFixResource', 'RemapIniGroupedResource', 'RemapIniRemover', 'RemapIniResource', 'RemapIniResourceMixin', 'RemapStats', 'RemapTexAddResource', 'RemapTexEditResource', 'RemappedKeyData', 'ReplaceIf', 'ReplaceList', 'ResCreate', 'ResGroupCollect', 'ResIdentity', 'ResRegCollect', 'ResReplace', 'SectionIterData', 'SectionIterDataIterator', 'SectionIterQueryData', 'SectionIterQueryDataIterator', 'SympyParser', 'SympyTokenizer', 'TexCreate', 'TexReplace', 'Token', 'VGRemap', 'VGRemaps', 'VbFile', 'VertexCounts', 'Z3Context', 'Z3Predicate', 'appendAllToOrderedMultiMap']
class BaseBufEditor:
    """
    
    Base class to edit some ``.buf`` file
        
    """
    def __init__(self) -> None:
        ...
    def fix(self, bufFile: CppBufFile, fixedBufFile: typing.Any = None) -> typing.Any:
        """
        Edits the ``.buf`` file. No-op by default
        
        Parameters
        ----------
        bufFile: :class:`CppBufFile`
            The binary ``.buf`` file to be modified
        
        fixedBufFile: Optional[:class:`str`]
            The name of the fixed ``.buf`` file. If this is ``None``, the fixed bytes are returned directly
            instead of being written to a file :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        Union[:class:`str`, :class:`bytearray`]
            If the argument ``fixedBufFile`` is ``None``, then will return an array of bytes for the fixed
            ``.buf`` file :raw-html:`<br />` :raw-html:`<br />`
            Otherwise will return the filename to the fixed ``.buf`` file
        """
class BaseDFA:
    pass
class BaseIniClassifier:
    """
    
    Base class to help classify the type of mod given the mod's .ini files
        
    """
    def __init__(self) -> None:
        ...
    @typing.overload
    def checkIsFixedMod(self, iniTxt: str, gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> tuple[bool, bool]:
        """
        Determines whether the .ini file is fixed and/or belongs to a mod
        
        Cheaper than :meth:`classify` when only these yes/no answers are needed -- see :meth:`classify`'s
        own doc comment for what "belongs to a mod"/"is fixed" mean
        
        Parameters
        ----------
        iniTxt: Union[:class:`str`, List[:class:`str`]]
            The text of the .ini file to read from, given as either:
        
            * the full text OR
            * lines of text with each line ending with a newline character
        
        gameTypeId: Optional[:class:`GameTypeId`]
            The game the .ini file is expected to belong to, if known
        
            **Default**: ``None``
        
        Returns
        -------
        Tuple[:class:`bool`, :class:`bool`]
            Whether the .ini file is fixed, and whether it belongs to a mod, in that order
        """
    @typing.overload
    def checkIsFixedMod(self, iniTxt: collections.abc.Sequence[str], gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> tuple[bool, bool]:
        ...
    @typing.overload
    def checkIsMod(self, iniTxt: str, gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> bool:
        """
        Determines whether the .ini file belongs to a mod
        
        Cheaper than :meth:`classify` when only this yes/no answer is needed -- see :meth:`classify`'s own
        doc comment for what "belongs to a mod" means
        
        Parameters
        ----------
        iniTxt: Union[:class:`str`, List[:class:`str`]]
            The text of the .ini file to read from, given as either:
        
            * the full text OR
            * lines of text with each line ending with a newline character
        
        gameTypeId: Optional[:class:`GameTypeId`]
            The game the .ini file is expected to belong to, if known
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`bool`
            Whether the .ini file belongs to a mod
        """
    @typing.overload
    def checkIsMod(self, iniTxt: collections.abc.Sequence[str], gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> bool:
        ...
    @typing.overload
    def classify(self, iniTxt: str, gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> IniClassifyStats:
        """
        Determines the type of mod given the text from the mod's .ini file
        
        Parameters
        ----------
        iniTxt: Union[:class:`str`, List[:class:`str`]]
            The text of the .ini file to read from, given as either:
        
            * the full text OR
            * lines of text with each line ending with a newline character
        
        gameTypeId: Optional[:class:`GameTypeId`]
            The game the .ini file is expected to belong to, if known
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IniClassifyStats`
            The stats about the classification of the .ini file
        """
    @typing.overload
    def classify(self, iniTxt: collections.abc.Sequence[str], gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> IniClassifyStats:
        ...
    def clear(self) -> None:
        """
        Clears the state of the classifier
        """
class BaseIniFixer(CppBaseIniFixer):
    """
    
    Base class to fix a .ini file
    
    Parameters
    ----------
    parser: :class:`BaseIniParser`
        The associated parser to retrieve data for the fix
        
    """
    def __init__(self, parser: typing.Any = None) -> None:
        ...
    def clear(self) -> None:
        """
        Resets any saved states within the fixer
        """
    def fix(self, keepBackup: bool = True, fixOnly: bool = False, hideOrig: bool = False, context: typing.Any = None) -> typing.Any:
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
        
        hideOrig: :class:`bool`
            Whether to hide the mod for the original character. :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        context: Optional[:class:`IniFixingContext`]
            The per-call options for this fix. If ``None``, a default one is built, which says this
            fixer is the .ini file's last :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        Optional[Dict[Union[:class:`str`, :class:`int`], :class:`str`]]
            The new content of the fixed .ini file(s) -- always ``None`` here, since this base class fixes
            nothing. See :meth:`GIMIFixer.fix` for a real one
        """
    @property
    def _iniFile(self) -> typing.Any:
        """
        :class:`IniFile`: The .ini file that will be fixed
        """
    @_iniFile.setter
    def _iniFile(self, arg0: typing.Any) -> None:
        ...
    @property
    def _parser(self) -> typing.Any:
        """
        :class:`BaseIniParser`: The associated parser to retrieve data for the fix
        """
    @_parser.setter
    def _parser(self, arg0: typing.Any) -> None:
        ...
class BaseIniGraphEdit(BaseIniGraphPartEdit):
    """
    
    This class inherits from :class:`BaseIniGraphPartEdit`
    
    Base class for a filter that edits some caller/callee graph of :class:`IniSectionGraph`
        
    """
    def __init__(self) -> None:
        ...
    def edit(self, graph: typing.Any, modType: typing.Any, modName: str = '', partFilter: typing.Any = None, trackKeys: bool = False, keysToTrack: typing.Any = None) -> typing.Any:
        """
        Edits the caller/callee graph of :class:`IniSectionGraph`
        
        .. note::
            The base implementation is a no-op that hands 'graph' straight back, matching the pure-Python
            original's ``pass``
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
            The filter used to indicate the valid order indices to process some :class:`IfContentPart` in
            the graph :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        trackKeys: :class:`bool`
            The **caller's** key-tracking default, handed down by whatever is driving this edit
            (:class:`GraphGroupEdit` passes its own ``trackKeys`` here) :raw-html:`<br />` :raw-html:`<br />`
        
            A subclass with its own key-tracking setting decides how to combine the two; a subclass without
            one simply ignores it :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        keysToTrack: Optional[Set[:class:`str`]]
            The **caller's** key-tracking key set, handed down the same way. ``None`` means every key
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IniSectionGraph`
            The resultant graph that got editted
        """
    def editFromIni(self, graph: typing.Any, ini: typing.Any, modType: typing.Any, modName: str = '', partFilter: typing.Any = None, trackKeys: bool = False, keysToTrack: typing.Any = None) -> typing.Any:
        """
        Edits the caller/callee graph of :class:`IniSectionGraph` with state info from 'ini'
        
        .. note::
            This forwards straight to :meth:`edit` and ignores 'ini' entirely, exactly as the pure-Python
            original does
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit
        
        ini: Optional[:class:`IniFile`]
            The associated .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
            The filter used to indicate the valid order indices to process some :class:`IfContentPart` in
            the graph :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        trackKeys: :class:`bool`
            The **caller's** key-tracking default, handed down by whatever is driving this edit
            (:class:`GraphGroupEdit` passes its own ``trackKeys`` here) :raw-html:`<br />` :raw-html:`<br />`
        
            A subclass with its own key-tracking setting decides how to combine the two; a subclass without
            one simply ignores it :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        keysToTrack: Optional[Set[:class:`str`]]
            The **caller's** key-tracking key set, handed down the same way. ``None`` means every key
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IniSectionGraph`
            The resultant graph that got editted
        """
class BaseIniGraphGroupEdit(BaseIniPartEdit):
    """
    
    This class inherits from :class:`BaseIniPartEdit`
    
    Base class for a filter that edits a group of caller/callee graphs across many .ini files
        
    """
    @staticmethod
    def addGraph(graphGroups: list, id: typing.Any, graph: typing.Any) -> bool:
        """
        Adds a graph to the group of graphs
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        id: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The id for where to add the graph. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`
        
            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        graph: :class:`IniSectionGraph`
            The graph to add
        
        Returns
        -------
        :class:`bool`
            Whether the graph has been added
        """
    @staticmethod
    def getGraph(graphGroups: list, id: typing.Any, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the corresponding graph from a group of graphs
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        id: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The id to retrieve the graph. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`
        
            #. The index for the .ini file
            #. The name of the component
            #. The name of the object
        
        errorOnNotFound: :class:`bool`
            If no graphs are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no graphs are found :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Raises
        ------
        `KeyError`_
            If no graphs are found
        
        Returns
        -------
        Union[:class:`IniSectionGraph`, Any]
            Either the found graph or the value specified at 'default', if no graphs were found and
            'errorOnNotFound' is set to ``False``
        """
    def __init__(self) -> None:
        ...
    def edit(self, graphGroups: list, modType: typing.Any, modName: str = '') -> list:
        """
        Edits a group of caller/callee graphs
        
        .. note::
            The base implementation is a no-op that hands 'graphGroups' straight back, matching the
            pure-Python original's ``pass``
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modType: :class:`ModType`
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The resultant group of graphs that got editted
        """
    def editFromIni(self, graphGroups: list, ini: typing.Any, modType: typing.Any, modName: str = '') -> typing.Any:
        """
        Edits a group of caller/callee graphs with state info from 'ini'
        
        .. note::
            This forwards straight to :meth:`edit` and ignores 'ini' entirely, exactly as the pure-Python
            original does
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        ini: :class:`IniFile`
            The associated original .ini file
        
        modType: :class:`ModType`
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The resultant group of graphs that got editted
        """
class BaseIniGraphPartEdit(BaseIniPartEdit):
    """
    
    This class inherits from :class:`BaseIniPartEdit`
    
    Base class for a filter that edits some part of a caller/callee graph (:class:`IniSectionGraph`)
    within a `.ini` file
    
    Adds nothing of its own over :class:`BaseIniPartEdit` -- exactly like the pure-Python original,
    this exists purely to mark the graph-editing half of the edit hierarchy apart from the rest
        
    """
    def __init__(self) -> None:
        ...
class BaseIniParser(CppBaseIniParser):
    """
    
    Base class to parse a .ini file
    
    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to parse
        
    """
    def __init__(self, iniFile: typing.Any = None) -> None:
        ...
    def clear(self) -> None:
        """
        Clears any saved data
        """
    def parse(self) -> typing.Any:
        """
        Parses the .ini file
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The parsed groups of caller/callee graphs found in the .ini file -- always empty here, since
            this base class parses nothing. See :meth:`GIMIParser.parse` for a real one
        """
    @property
    def _iniFile(self) -> typing.Any:
        """
        :class:`IniFile`: The .ini file that will be parsed
        """
    @_iniFile.setter
    def _iniFile(self, arg0: typing.Any) -> None:
        ...
    @property
    def _modsToFix(self) -> typing.Any:
        """
        Set[:class:`str`]: The names of the mods that will be fixed to
        """
    @_modsToFix.setter
    def _modsToFix(self, arg0: typing.Any) -> None:
        ...
class BaseIniPartEdit:
    """
    
    Base class for a filter that edits some part of a `.ini` file
    
    .. note::
        The deleted pure-Python original also declared ``edit``/``editFromIni`` here, as
        ``(*args, modType, modName = "", **kwargs) -> Any``. That signature has no C++ equivalent --
        every subclass takes genuinely different arguments and returns a different type -- so each
        subclass family declares its own **typed** ``edit``/``editFromIni`` pair instead (see
        :class:`BaseRegEdit`), and only :meth:`clear` (which really is common) lives here
        
    """
    def __init__(self) -> None:
        ...
    def clear(self) -> None:
        """
        Clears any saved state information. No-op by default
        """
class BaseIniRemover(CppBaseIniRemover):
    """
    
    Base class to remove fixes from a .ini file
    
    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to remove the fix from
        
    """
    def __init__(self, iniFile: typing.Any = None) -> None:
        ...
    def remove(self, parse: bool = False, writeBack: bool = True, context: typing.Any = None) -> str:
        """
        Removes the fix from the .ini file
        
        Parameters
        ----------
        parse: :class:`bool`
            Whether to also parse for the .*RemapBlend.buf files that need to be removed :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        writeBack: :class:`bool`
            Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        context: :class:`IniRemovalContext`
            The per-call options for this removal :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, which means a default-constructed one
        
        Returns
        -------
        :class:`str`
            The new content of the .ini file
        """
    @property
    def iniFile(self) -> typing.Any:
        """
        :class:`IniFile`: The .ini file that the fix will be removed from
        """
    @iniFile.setter
    def iniFile(self, arg0: typing.Any) -> None:
        ...
class BaseLogger:
    """
    
    Base class for the *view* of the project's MVC architecture -- everything the remap reports back to the user
    (progress messages, headings, lists, errors, prompts) goes through one of these :raw-html:`<br />` :raw-html:`<br />`
    
    This class owns all of the *formatting* and *bookkeeping* (the prefix, the heading stack, the ``.txt`` log
    transcript, the verbosity flags) and funnels every rendered line through exactly two abstract methods,
    :meth:`write` and :meth:`read`, which a subclass implements for wherever the output actually needs to go:
    
    * :class:`Logger` writes through ``print``/``input`` -- the console (CLI) view
    * a GUI, or a backend server that needs to forward the messages to a frontend app, subclasses this and
      implements :meth:`write`/:meth:`read` for its own transport instead
    
    Every higher-level method (:meth:`log`, :meth:`openHeading`, :meth:`error`, ...) can also be overridden, so a
    view that wants *structured* events rather than pre-rendered text (e.g. a backend telling a frontend
    "a heading opened", not "here is a line of ``=`` characters") can override at that level instead and never see
    the text rendering at all. The defaults render text and route it through :meth:`log`, so overriding just
    :meth:`write` is enough for any plain text sink :raw-html:`<br />` :raw-html:`<br />`
    
    .. note::
        A subclass written in Python is fully supported -- its overrides are reached both from Python callers
        and from C++ code holding the logger through this base class
    
    Parameters
    ----------
    prefix: :class:`str`
        line that is printed before any message is printed out :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ""
    
    logTxt: :class:`bool`
        Whether to log all the printed messages into a .txt file once the fix is done :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
    
    verbose: :class:`bool`
        Whether to print out output :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    DefaultHeadingChar: typing.ClassVar[str] = '='
    DefaultHeadingSideLen: typing.ClassVar[int] = 2
    ErrorHeader: typing.ClassVar[str] = '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
    @staticmethod
    def getBulletStr(txt: str) -> str:
        """
        Creates the string for an item in an unordered list
        
        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out
        
        Returns
        -------
        :class:`str`
            The text formatted as an item in an unordered list
        """
    @staticmethod
    def getNumberedStr(txt: str, num: typing.SupportsInt | typing.SupportsIndex) -> str:
        """
        Creates the string for an ordered list
        
        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out
        
        num: :class:`int`
            The number we want to print out before the text for the ordered list
        
        Returns
        -------
        :class:`str`
            The text formatted as an item in an ordered list
        """
    def __init__(self, prefix: str = '', logTxt: bool = False, verbose: bool = True) -> None:
        ...
    def box(self, message: str, header: str) -> None:
        """
        Prints the message to be sandwiched by the text defined in the argument, ``header``
        
        Parameters
        ----------
        message: :class:`str`
            The message we want to print out. Printed one line at a time
        
        header: :class:`str`
            The string that we want to sandwich our message against
        """
    def bulletPoint(self, txt: str) -> None:
        """
        Prints out an item in an unordered list
        
        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out
        """
    def clear(self) -> None:
        """
        Clears out any saved text from the logger
        """
    def closeHeading(self) -> None:
        """
        Prints out a closing heading that corresponds to a previous opening heading printed (see line 3 of the example at :class:`Heading`)
        """
    def error(self, message: str) -> None:
        """
        Prints an error message :raw-html:`<br />` :raw-html:`<br />`
        
        An error is always displayed, even when :attr:`verbose` is ``False`` -- unless the messages are being logged to a
        .txt file (:attr:`logTxt`), in which case the current verbosity is respected and the error only ends up in
        :attr:`loggedTxt`
        
        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        """
    def getStr(self, message: str) -> str:
        """
        Retrieves the string to be printed out by the logger
        
        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        
        Returns
        -------
        :class:`str`
            The transformed text that the logger prints out
        """
    @typing.overload
    def handleException(self, exception: typing.Any) -> None:
        """
        Prints the message for an error
        
        Parameters
        ----------
        exception: :class:`BaseException`
            The error we want to handle
        """
    @typing.overload
    def handleException(self, exceptionType: str, message: str, traceback: str = '') -> None:
        """
        Prints the message for an error, from its already-separated parts -- for an error that did not come from a live
        exception object (e.g. one reported by another process, or by the C++ core) :raw-html:`<br />` :raw-html:`<br />`
        
        Renders the same text as the overload above and hands it to :meth:`error`
        
        Parameters
        ----------
        exceptionType: :class:`str`
            The name of the type of the error
        
        message: :class:`str`
            The error's own message
        
        traceback: :class:`str`
            Where the error came from, if known :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ""
        """
    def input(self, desc: str) -> str:
        """
        Handles user input from the console
        
        Parameters
        ----------
        desc: :class:`str`
            The question/description being asked to the user for input
        
        Returns
        -------
        :class:`str`
            The resultant input the user entered
        """
    def list(self, lst: collections.abc.Sequence[str], transform: typing.Any = None) -> None:
        """
        Prints out an ordered list
        
        Parameters
        ----------
        lst: List[:class:`str`]
            The list of messages we want to print out
        
        transform: Optional[Callable[[:class:`str`], :class:`str`]]
            A function used to do any processing on each message in the list of messages
        
            If this parameter is ``None``, then the list of message will not go through any type of processing :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def log(self, message: str) -> None:
        """
        Regularly prints text onto the console
        
        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        """
    def openHeading(self, txt: str, sideLen: typing.SupportsInt | typing.SupportsIndex = 2, headingChar: str = '=') -> None:
        """
        Prints out an opening heading
        
        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out
        
        sideLen: :class:`int`
            How many characters we want for the side border of the heading :raw-html:`<br />`
            (see line 1 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: 2
        
        headingChar: :class:`str`
            The type of character used to print the side border of the heading :raw-html:`<br />`
            (see line 3 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: "="
        """
    def read(self, desc: str) -> str:
        """
        Asks the user for a line of input :raw-html:`<br />` :raw-html:`<br />`
        
        The raw source :meth:`input` ends up in, matching the builtin ``input(desc)``: ``desc`` is displayed *without* a
        trailing newline, then one line is read back :raw-html:`<br />` :raw-html:`<br />`
        
        **Abstract** -- a subclass must implement this
        
        Parameters
        ----------
        desc: :class:`str`
            The question/description being asked to the user for input
        
        Returns
        -------
        :class:`str`
            The resultant input the user entered
        """
    def space(self) -> None:
        """
        Prints out a space
        """
    def split(self) -> None:
        """
        Prints out a new line, if anything has been logged since :attr:`prefix` was last set
        """
    def waitExit(self) -> None:
        """
        Prints the message used when the script finishes running, and waits for the user to press ENTER
        """
    def write(self, message: str) -> None:
        """
        Writes one already-rendered message to wherever this view displays output :raw-html:`<br />` :raw-html:`<br />`
        
        The raw sink every printing method ends up in. Only called when :attr:`verbose` is ``True``. A ``message`` is a single
        line without its trailing newline -- the sink adds whatever line ending it needs (matching ``print``) :raw-html:`<br />` :raw-html:`<br />`
        
        **Abstract** -- a subclass must implement this
        
        Parameters
        ----------
        message: :class:`str`
            The rendered message to display
        """
    @property
    def headings(self) -> list[tuple[str, int, str]]:
        """
        List[Tuple[:class:`str`, :class:`int`, :class:`str`]]: The stack of headings that have been opened (by calling :meth:`openHeading`), but have not been closed yet (have not called :meth:`closeHeading` yet) :raw-html:`<br />` :raw-html:`<br />`
        
        Each heading is a ``(title, sideLen, headingChar)`` tuple, innermost (most recently opened) last. A fresh copy on every access
        """
    @property
    def includePrefix(self) -> bool:
        """
        :class:`bool`: Whether to include the prefix string when printing out a message
        """
    @includePrefix.setter
    def includePrefix(self, arg0: bool) -> None:
        ...
    @property
    def logTxt(self) -> bool:
        """
        :class:`bool`: Whether to log all the printed messages into a .txt file once the fix is done
        """
    @logTxt.setter
    def logTxt(self, arg0: bool) -> None:
        ...
    @property
    def loggedTxt(self) -> str:
        """
        :class:`str`: The text to be logged into a .txt file
        """
    @property
    def prefix(self) -> str:
        """
        :class:`str`: The line of text that is printed before any message is printed out
        """
    @prefix.setter
    def prefix(self, arg1: str) -> None:
        ...
    @property
    def verbose(self) -> bool:
        """
        :class:`bool`: Whether to print out output
        """
    @verbose.setter
    def verbose(self, arg0: bool) -> None:
        ...
class BaseRegEdit(BaseIniGraphPartEdit):
    """
    
    This class inherits from :class:`BaseIniGraphPartEdit`
    
    Base class for a filter that edits some registers within an :class:`IfContentPart`
        
    """
    def __init__(self) -> None:
        ...
    def edit(self, part: typing.Any, sectionName: str, modType: typing.Any, modName: str = '', partRanges: typing.Any = None) -> typing.Any:
        """
        Edits the registers for the current :class:`IfContentPart`. No-op by default, returning 'part'
        untouched
        
        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the `IfTemplate` that is being editted
        
        sectionName: :class:`str`
            The name of the `section`_ that is being editted
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partRanges: Optional[:class:`Ranges`]
            The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IfContentPart`
            The resultant part of the `IfTemplate` that got its registers editted
        """
    def editFromIni(self, part: typing.Any, sectionName: str, ini: typing.Any, modType: typing.Any, modName: str = '', partRanges: typing.Any = None) -> typing.Any:
        """
        Edits the registers for the current :class:`IfContentPart` with state info from 'ini'
        
        .. note::
            This forwards straight to :meth:`edit` and ignores 'ini' entirely, exactly as the pure-Python
            original does
        
        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the `IfTemplate` that is being editted
        
        sectionName: :class:`str`
            The name of the `section`_ that is being editted
        
        ini: Optional[:class:`IniFile`]
            The associated .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partRanges: Optional[:class:`Ranges`]
            The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IfContentPart`
            The resultant part of the `IfTemplate` that got its registers editted
        """
class BaseResEdit:
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
        
    """
    @staticmethod
    def fileAddGraphId(file: str, graphId: str = '') -> str:
        """
        Adds the unique id for the :class:`IniSectionGraph` of the resource to the name of the file
        
        Parameters
        ----------
        file: :class:`str`
            The path to the file to add the id to
        
        graphId: :class:`str`
            The id to add :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The file with the id added
        """
    @staticmethod
    def getFileId(modObj: typing.Any, sectionName: str, part: typing.Any, orderInd: typing.SupportsInt | typing.SupportsIndex, file: str) -> str:
        """
        Retrieves a unique id for a file within a single .ini file
        
        .. note::
            The returned value is not byte-identical to the one the pure-Python original produced -- it is
            an opaque, within-one-run dictionary key that is never persisted or written to a file
        
        Parameters
        ----------
        modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The mod object holding the newly created :class:`IniSectionGraph` for the resource
        
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
        :class:`str`
            The unique id for the file
        """
    def __init__(self, resType: str, resModObj: typing.Any, graphReplaceMode: typing.Any = None) -> None:
        ...
    def buildResModel(self, resType: str, ini: typing.Any, srcPath: str, *args, **kwargs) -> typing.Any:
        """
        Builds the model for the resource
        
        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource
        
        ini: :class:`IniFile`
            The .ini file to build the resource for
        
        srcPath: :class:`str`
            The file path to the original resource
        
        Returns
        -------
        :class:`IniResource`
            The built resource
        """
    def buildResModels(self, graph: typing.Any, ini: typing.Any = None, modType: typing.Any = None, resources: typing.Any = None, resourceFilter: typing.Any = None, modName: str = '', graphId: str = '', resModObj: typing.Any = None) -> None:
        """
        Builds and saves the resources, given the :class:`IniSectionGraph` for a resource
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph for the particular resource
        
        ini: Optional[:class:`IniFile`]
            The .ini file to build the resource for
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored, keyed by the unique id for the source file (created
            from :meth:`getFileId`). If ``None``, the models are appended to :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for -- takes the source file and its
            assigned id :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        graphId: :class:`str`
            The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resModObj: Optional[Tuple[:class:`int`, :class:`str`, :class:`str`]]
            The mod object used to create the unique id for the resources :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def buildResources(self, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', resourceFilter: typing.Any = None, resources: typing.Any = None, copySections: bool = False) -> list:
        """
        Builds the :class:`IniSectionGraph` and the corresponding models for the resources
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
            The associated original .ini file being fixed
        
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored. If ``None``, they are appended to
            :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        copySections: :class:`bool`
            Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The group of graphs that now includes the newly created graph for the resource
        
            .. tip::
                You can access the newly generated graph using :attr:`resModObj` on the group of graphs
        """
    def clear(self) -> None:
        """
        Clears any saved state information
        """
    def collectResourceName(self, oldResourceName: str, newResourceName: str) -> tuple[str, str]:
        """
        Collects the name of the fixed resource `section`_ (used for the 'collectedSections' parameter in
        :meth:`buildResources`)
        
        Parameters
        ----------
        oldResourceName: :class:`str`
            The old name of the resource `section`_
        
        newResourceName: :class:`str`
            The fixed name for the resource `section`_ (created by :meth:`getFixResourceName`)
        
        Returns
        -------
        Tuple[:class:`str`, :class:`str`]
            A tuple where the first value is the old resource name and the second is the new resource name
        """
    def getFixFile(self, file: str, modType: typing.Any = None, modName: str = '', graphId: str = '') -> str:
        """
        Retrieves the file path to the fixed resource
        
        Parameters
        ----------
        file: :class:`str`
            The file path to the original resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
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
    def getFixResourceName(self, resource: str, modType: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Retrieves the name of the fixed resource `section`_
        
        Parameters
        ----------
        resource: :class:`str`
            The name of the original resource `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        Optional[:class:`str`]
            The `section`_ name of the fixed resource. ``None`` indicates there was no name change between
            the original resource and the fixed resource
        """
    def getResGraph(self, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', rename: bool = True, copySections: bool = False) -> typing.Any:
        """
        Retrieves the particular :class:`IniSectionGraph` for the resource
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource -- the keys are the old names of the
            `sections`_ and the values are the fixed names
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
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
    def renameUncollectedSection(self, sectionName: str, modType: typing.Any = None, modName: str = '') -> str:
        """
        The name an uncollected `section`_ gets renamed to -- :meth:`getFixResourceName`, or the
        `section`_'s own name when that reports no change
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """
    @property
    def graphReplaceMode(self) -> typing.Any:
        """
        :class:`IniGraphReplaceMode`: What to do when the corresponding :class:`IniSectionGraph` to
        construct already exists
        """
    @graphReplaceMode.setter
    def graphReplaceMode(self, arg1: typing.Any) -> None:
        ...
    @property
    def resModObj(self) -> typing.Any:
        """
        Tuple[:class:`int`, :class:`str`, :class:`str`]: The mod object to hold the newly created
        :class:`IniSectionGraph` for the resource
        """
    @resModObj.setter
    def resModObj(self, arg1: typing.Any) -> None:
        ...
    @property
    def resType(self) -> str:
        """
        :class:`str`: The name of the type of resource
        """
    @resType.setter
    def resType(self, arg0: str) -> None:
        ...
class BaseSLR1Parser:
    """
    
    The base class used for bottom-up `SLR(1)`_ parsing
    
    Parameters
    -----------
    productions: Dict[Hashable, Tuple[:class:`str`, List[:class:`str`]]]
        The production rules of the `CFG (Context Free Grammer)`_, keyed by the id of each production rule
    
    startSymbol: Hashable
        The starting non-terminal symbol
    
    startToken: :class:`str`
        The name of the starting token for an input string
    
        **Default**: ``STARTTOKEN``
    
    endToken: :class:`str`
        The name of the ending token for an input string
    
        **Default**: ``ENDTOKEN``
    
    nullToken: :class:`str`
        The name for the empty token
    
        **Default**: ``EPSILON``
    
    setup: :class:`bool`
        Whether to initialize all the setup for the parser automatically by calling :meth:`setup`
    
        **Default**: ``True``
        
    """
    def __init__(self, productions: dict, startSymbol: str, startToken: str = 'STARTTOKEN', endToken: str = 'ENDTOKEN', nullToken: str = 'EPSILON', setup: bool = True) -> None:
        ...
    def clear(self) -> None:
        """
        Clears all the setup from the parser
        """
    def getFirst(self, symbols: collections.abc.Sequence[str], nullable: collections.abc.Mapping[str, bool], first: collections.abc.Mapping[str, collections.abc.Set[str]]) -> set[str]:
        """
        Retrieves the first terminal symbols to appear given a list of symbols
        
        Parameters
        ----------
        symbols: List[:class:`str`]
            The symbols to read
        
        nullable: Dict[:class:`str`, :class:`bool`]
            The `Nullable Set`_
        
        first: Dict[:class:`str`, Set[:class:`str`]]
            The `First Set`_ for only each single non-terminal symbol
        
        Returns
        -------
        Set[:class:`str`]
            The first terminal symbols to appear given 'symbols'
        """
    def getFirstSet(self, updateNullable: bool = True) -> dict[str, set[str]]:
        """
        Computes the `First Set`_ for only each single non-terminal symbol
        
        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The first terminal symbols to appear for a non-terminal symbol
        """
    def getFollowSet(self, updateNullable: bool = True, updateFirst: bool = True) -> dict[str, set[str]]:
        """
        Computes the `Follow Set`_
        
        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_
        
            **Default**: ``True``
        
        updateFirst: :class:`bool`
            Whether to update the `First Set`_
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The `Follow Set`_
        """
    def getNonTermSymbols(self) -> set[str]:
        """
        Retrieves the set of non-terminal symbols of the `CFG`_
        
        Returns
        -------
        Set[:class:`str`]
            The set of non-terminal symbols
        """
    def getNullableSet(self) -> dict[str, bool]:
        """
        Computes the `Nullable Set`_
        
        Returns
        -------
        Dict[:class:`str`, :class:`bool`]
            Whether each non-terminal symbol is nullable
        """
    def parse(self, tokens: collections.abc.Sequence[Token], ctx: ParseContext = None) -> ParseTree:
        """
        Parses an input text
        
        Parameters
        ----------
        tokens: List[:class:`Token`]
            The tokenized tokens of the input text :raw-html:`<br />` :raw-html:`<br />`
        
            Usually obtained by running some sort of tokenizer, such as :class:`BaseTokenizer`
        
        ctx: Optional[:class:`ParseContext`]
            The context for parsing :raw-html:`<br />` :raw-html:`<br />`
        
            If this argument is ``None``, a context is constructed from the concatenation of every
            token's value
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`SyntaxErr`
            If the parse tree cannot be constructed
        
        Returns
        -------
        :class:`ParseTree`
            The constructed parse tree
        """
    def setup(self) -> None:
        """
        Initializes any necessary setup for the parser
        """
    @property
    def endToken(self) -> str:
        """
        :class:`str`: The name of the ending token for an input string
        """
    @endToken.setter
    def endToken(self, arg0: str) -> None:
        ...
    @property
    def first(self) -> dict[str, set[str]]:
        """
        Dict[:class:`str`, Set[:class:`str`]]: The `First Set`_ for only each single non-terminal symbol
        """
    @first.setter
    def first(self, arg0: collections.abc.Mapping[str, collections.abc.Set[str]]) -> None:
        ...
    @property
    def follow(self) -> dict[str, set[str]]:
        """
        Dict[:class:`str`, Set[:class:`str`]]: The `Follow Set`_
        """
    @follow.setter
    def follow(self, arg0: collections.abc.Mapping[str, collections.abc.Set[str]]) -> None:
        ...
    @property
    def nonTermSymbols(self) -> set[str]:
        """
        Set[:class:`str`]: The set of non-terminal symbols of the `CFG`_, as of the last time 'productions' was set
        """
    @property
    def nullToken(self) -> str:
        """
        :class:`str`: The name for the empty token
        """
    @nullToken.setter
    def nullToken(self, arg0: str) -> None:
        ...
    @property
    def nullable(self) -> dict[str, bool]:
        """
        Dict[:class:`str`, :class:`bool`]: The `Nullable Set`_
        
        The keys are the non-terminal symbols and the values are whether each symbol is nullable
        """
    @nullable.setter
    def nullable(self, arg0: collections.abc.Mapping[str, bool]) -> None:
        ...
    @property
    def productions(self) -> dict:
        """
        Dict[Hashable, Tuple[:class:`str`, List[:class:`str`]]]: The production rules of the `CFG`_, keyed by the id of each production rule
        """
    @property
    def startSymbol(self) -> str:
        """
        :class:`str`: The starting non-terminal symbol
        
        :getter: Retrieves the starting non-terminal symbol
        :setter: Sets the new starting non-terminal symbol
        """
    @startSymbol.setter
    def startSymbol(self, arg1: str) -> None:
        ...
    @property
    def startToken(self) -> str:
        """
        :class:`str`: The name of the starting token for an input string
        """
    @startToken.setter
    def startToken(self, arg0: str) -> None:
        ...
class BaseTokenizer:
    """
    
    The base class used for tokenizing text
    
    Parameters
    ----------
    tokens: Dict[:class:`str`, :class:`str`]
        The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
    
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, tokens: collections.abc.Mapping[str, str], setup: bool = True) -> None:
        ...
    def addASCIIRangeTransitions(self, srcId: str, startChar: str, endChar: str, destId: str) -> None:
        """
        Adds a group of transitions from one state to another according to a range of `ASCII`_ characters
        
        Parameters
        ----------
        srcId: :class:`str`
            The id of the source state for the transition
        
        startChar: :class:`str`
            The starting character within the ASCII range to add a transition for
        
        endChar: :class:`str`
            The ending character within the ASCII range to add a transition for
        
        destId: :class:`str`
            The id of the destination state for the transition
        """
    def addKeyword(self, keyword: str) -> str:
        """
        Adds a keyword into the `DFA`_ of the tokenizer
        
        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add
        
        Returns
        -------
        :class:`str`
            The id of the accepting node in the `DFA`_
        """
    def addStartState(self) -> str:
        """
        Adds the start state representing an empty string
        
        Returns
        -------
        :class:`str`
            The id of the start state
        """
    def clear(self) -> None:
        """
        Clears the `DFA`_ of the tokenizer
        """
    def reset(self) -> None:
        """
        Resets the state of the `DFA`_ for the tokenizer
        """
    def setup(self) -> None:
        """
        Performs any necessary setup to the tokenizer
        """
    @typing.overload
    def simplifiedMaximalMunch(self, src: ParseContext, includeFiltered: bool = False) -> list[Token]:
        """
        Tokenizes the source text into tokens using the `Simplified Maximal Munch`_ algorithm
        
        Parameters
        ----------
        src: Union[:class:`str`, :class:`ParseContext`]
            The source text to be tokenized
        
        includeFiltered: :class:`bool`
            Ignored by this base class -- see :class:`FilteredTokenizer` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Raises
        ------
        :class:`SyntaxErr`
            The provided source text cannot be correctly tokenized
        
        Returns
        -------
        List[:class:`Token`]
            The list of tokens to the source text
        """
    @typing.overload
    def simplifiedMaximalMunch(self, src: str, includeFiltered: bool = False) -> list[Token]:
        ...
    @property
    def startStateId(self) -> str:
        """
        :class:`str`: The id of the starting state of the `DFA`_
        """
    @property
    def tokens(self) -> dict[str, str]:
        """
        Dict[:class:`str`, :class:`str`]: The tokens used for tokenization
        
        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
        """
class BiMap:
    """
    
            A one-to-one dictionary
            
    """
    def __init__(self) -> None:
        ...
    def __len__(self) -> int:
        ...
    def add(self, key: typing.Any, val: typing.Any) -> bool:
        ...
    def clear(self) -> None:
        ...
    def empty(self) -> bool:
        ...
    def findKey(self, val: typing.Any) -> typing.Any | None:
        ...
    def findValue(self, key: typing.Any) -> typing.Any | None:
        ...
    def getKey(self, val: typing.Any) -> typing.Any:
        ...
    def getValue(self, key: typing.Any) -> typing.Any:
        ...
    def insert(self, key: typing.Any, val: typing.Any) -> None:
        ...
class BinaryFile:
    """
    
    A class to handle binary files
        
    """
    def __init__(self, src: typing.Any) -> None:
        """
        Constructs a new binary file
        
        Parameters
        ----------
        src: Union[:class:`str`, :class:`bytes`]
            The source file or bytes for the file
        """
    def read(self) -> bytes:
        """
        Reads the data within a file
        
        Returns
        -------
        :class:`bytes`
            The read bytes
        """
    @property
    def data(self) -> bytes:
        """
        :class:`bytes`: The bytes read in from the source
        """
    @property
    def src(self) -> typing.Any:
        """
        Union[:class:`str`, :class:`bytes`]: The source file or bytes for the file
        """
    @src.setter
    def src(self, arg1: typing.Any) -> None:
        ...
class BlendFile(CppBufFile):
    """
    
    This class inherits from :class:`CppBufFile`
    
    Used for handling ``Blend.buf`` files
    
    .. note::
        We observe that a ``Blend.buf`` file is a binary file defined as:
    
        * a line corresponds to the data for a particular vertex in the mod
        * each line contains 32 bytes (256 bits)
        * each line uses little-endian mode (MSB is to the right while LSB is to the left)
        * the first 16 bytes of a line are for the blend weights, each weight is 4 bytes or 32 bits (4 weights/line)
        * the last 16 bytes of a line are for the corresponding indices for the blend weights, each index is 4 bytes or 32 bits (4 indices/line)
        * the blend weights are floating points while the blend indices are unsigned integers
        
    """
    @staticmethod
    def getMissingIndicesRemap(src: collections.abc.Mapping[str, collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex]], vgRemap: VGRemap) -> dict[int, int]:
        """
        Retrieves the temporary remap for any missing blend indices not included in 'vgRemap'
        
        Parameters
        ----------
        src: Dict[:class:`str`, Union[List[:class:`int`], List[:class:`float`]]]
            The data for the blend weights and the blend indices for a particular vertex
        
        vgRemap: :class:`VGRemap`
            The vertex group remap for correcting the Blend.buf file
        
        Returns
        -------
        Dict[:class:`int`, :class:`int`]
            The temporary remap for the missing indices. The keys are the missing indices found and the
            values are the temporary remapped values for these missing indices
        """
    @staticmethod
    def remapIndices(src: collections.abc.Mapping[str, collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex]], vgRemap: VGRemap, remapMissingIndices: bool = True) -> dict[str, list[int | int | float]]:
        """
        Remaps the vertex group indices for a particular line (vertex)
        
        Parameters
        ----------
        src: Dict[:class:`str`, Union[List[:class:`int`], List[:class:`float`]]]
            The data for the blend weights and the blend indices for a particular vertex
        
        vgRemap: :class:`VGRemap`
            The vertex group remap for correcting the Blend.buf file
        
        remapMissingIndices: :class:`bool`
            Whether to deactivate any missing blend indices that cannot be identified. **Default**: ``True``
        
        Returns
        -------
        Dict[:class:`str`, Union[List[:class:`int`], List[:class:`float`]]]
            The new data for the blend weights/blend indices, with the blend indices remapped
        """
    def __init__(self, src: typing.Any, elements: typing.Any = None) -> None:
        """
        Constructs a new blend file and immediately reads it
        
        Parameters
        ----------
        src: Union[:class:`str`, :class:`bytes`]
            The source file or bytes for the blend file
        
        elements: Optional[List[:class:`BufElementType`]]
            The sequence of elements within the ``.buf`` file. If this argument is ``None`` or empty, will
            use the elements specified for some GIMI character. **Default**: ``None``
        
        Raises
        ------
        :class:`BufFileNotRecognized`
            If 'src' holds a file path that cannot be read as a valid blend file
        
        :class:`BadBufData`
            If 'src' holds raw bytes that are not valid for a blend file
        """
    def remap(self, vgRemap: VGRemap, fixedBlendFile: typing.Any = None, remapMissingIndices: bool = True) -> typing.Any:
        """
        Remaps the blend indices in a ``Blend.buf`` file
        
        Parameters
        ----------
        vgRemap: :class:`VGRemap`
            The vertex group remap for correcting the Blend.buf file
        
        fixedBlendFile: Optional[:class:`str`]
            The file path for the fixed ``Blend.buf`` file. **Default**: ``None``
        
        remapMissingIndices: :class:`bool`
            Whether to deactivate any missing blend indices that cannot be identified. **Default**: ``True``
        
        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If ``fixedBlendFile`` is ``None`` and no correction was needed, returns ``None``. If
            ``fixedBlendFile`` is ``None`` and correction was needed, returns the fixed bytes. Otherwise
            returns ``fixedBlendFile`` itself
        """
class BufBaseFloat(BufDataType):
    """
    
    This class inherits from :class:`BufDataType`
    
    The type definition for a generic 32-bit IEEE 754 `floating point`_ number within a ``.buf`` file
        
    """
    def __init__(self, name: str, size: typing.SupportsInt | typing.SupportsIndex, isBigEndian: bool = False) -> None:
        """
        Constructs a new `floating point`_ type
        
        Parameters
        ----------
        name: :class:`str`
            The name of the type
        
        size: :class:`int`
            The byte size for the data type
        
        isBigEndian: :class:`bool`
            Whether the type is in big endian mode. **Default**: ``False``
        """
class BufBaseInt(BufDataType):
    """
    
    This class inherits from :class:`BufDataType`
    
    The type definition for some generic integer type within a ``.buf`` file, at most 8 bytes wide
    (see :class:`BufDataType`'s class-level warning)
        
    """
    def __init__(self, name: str, size: typing.SupportsInt | typing.SupportsIndex, isBigEndian: bool = False, isSigned: bool = True) -> None:
        """
        Constructs a new integer type
        
        Parameters
        ----------
        name: :class:`str`
            The name of the type
        
        size: :class:`int`
            The byte size for the data type
        
        isBigEndian: :class:`bool`
            Whether the type is in big endian mode. **Default**: ``False``
        
        isSigned: :class:`bool`
            Whether the type is signed. **Default**: ``True``
        """
    @property
    def isSigned(self) -> bool:
        """
        :class:`bool`: Whether the data type is signed
        """
class BufDataType(BufType):
    """
    
    This class inherits from :class:`BufType`
    
    The abstract base for an elementary data type within a ``.buf`` file (eg. a single integer or
    `floating point`_ number) -- a real format is one of :class:`BufBaseInt`'s or
    :class:`BufBaseFloat`'s concrete subclasses, or :class:`BufUnorm`
    
    .. warning::
        Unlike the pure-Python original this replaces (where any subclass could be defined in plain
        Python and used immediately), a brand-new elementary data type not already covered by one of
        this class's existing subclasses needs a real C++ subclass and a rebuild of this extension --
        ``decode``/``encode`` are not overridable from pure Python here
        
    """
    def decode(self, src: bytes) -> int | int | float:
        """
        Decode the raw bytes to the required format for the type
        
        .. warning::
            Please make sure the number of bytes passed into 'src' matches :attr:`size`
        
        Parameters
        ----------
        src: :class:`bytes`
            The raw bytes to decode
        
        Returns
        -------
        Union[:class:`int`, :class:`float`]
            The decoded value for the type
        """
    def encode(self, src: typing.SupportsInt | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex) -> bytes:
        """
        Encodes the format of the type back to raw bytes
        
        .. warning::
            Please make sure 'src' is within the acceptable range for the type
        
        Parameters
        ----------
        src: Union[:class:`int`, :class:`float`]
            The decoded value to encode
        
        Returns
        -------
        :class:`bytes`
            The encoded raw bytes
        """
    @property
    def isBigEndian(self) -> bool:
        """
        :class:`bool`: The `endianness`_ for the data type
        """
    @isBigEndian.setter
    def isBigEndian(self, arg1: bool) -> None:
        ...
    @property
    def size(self) -> int:
        """
        :class:`int`: The byte size for the data type (at most 8 bytes)
        
        Raises
        ------
        :class:`ValueError`
            If set to ``0`` or a value greater than ``8``
        """
    @size.setter
    def size(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class BufEditor(BaseBufEditor):
    """
    
    This class inherits from :class:`BaseBufEditor`
    
    Class to edit some ``.buf`` file by running a fixed sequence of filters over it
    
    Parameters
    ----------
    filters: Optional[List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]]
        The filters used to edit the data for each line in the ``.buf`` file :raw-html:`<br />` :raw-html:`<br />`
    
        The filters take in the following arguments:
    
        #. The data for a particular line
        #. The starting byte index of the line that is read
        #. The line index being processed
        #. The size of each line :raw-html:`<br />` :raw-html:`<br />`
    
        The output of the filters is the resultant data that consists where the keys are the names of
        the elements within a line in the ``.buf`` file and the values are the resultant data for each
        element in the line :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, filters: typing.Any = None) -> None:
        ...
    @property
    def filters(self) -> list:
        """
        List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]:
        The filters used to edit the data for each line in the ``.buf`` file
        """
    @filters.setter
    def filters(self, arg1: typing.Any) -> None:
        ...
class BufElementType(BufType):
    """
    
    This class inherits from :class:`BufType`
    
    The type definition for an element within a ``.buf`` file
        
    """
    def __init__(self, name: str, formatName: str, dataTypes: typing.Any) -> None:
        """
        Constructs a new element type
        
        Parameters
        ----------
        name: :class:`str`
            The name of the element
        
        formatName: :class:`str`
            The name of the type format according to 3dmigoto
        
        dataTypes: List[:class:`BufDataType`]
            The data types composed within the element, in byte order -- each is cloned, so the same
            passed-in instance can safely be reused for other elements afterward
        """
    def decode(self, src: bytes) -> list[int | int | float]:
        """
        Decodes a raw sequence of bytes into one decoded value per data type composing this element
        
        Parameters
        ----------
        src: :class:`bytes`
            The source bytes to decode
        
        Returns
        -------
        List[Union[:class:`int`, :class:`float`]]
            The decoded values, one per entry of :attr:`dataTypes`, in the same order
        """
    def encode(self, src: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex]) -> bytes:
        """
        Encodes the decoded values for this element back to raw bytes
        
        Parameters
        ----------
        src: List[Union[:class:`int`, :class:`float`]]
            The decoded values to encode, one per entry of :attr:`dataTypes`
        
        Returns
        -------
        :class:`bytes`
            The encoded raw bytes
        """
    @property
    def dataTypes(self) -> list[BufDataType]:
        """
        List[:class:`BufDataType`]: The data types composed within the element
        
        Assigning a new list clones each new data type the same way the constructor's own ``dataTypes``
        parameter does
        """
    @dataTypes.setter
    def dataTypes(self, arg1: typing.Any) -> None:
        ...
    @property
    def formatName(self) -> str:
        """
        :class:`str`: The name of the type format according to 3dmigoto
        """
    @formatName.setter
    def formatName(self, arg1: str) -> None:
        ...
    @property
    def size(self) -> int:
        """
        :class:`int`: The byte size for the element
        """
class BufFloat(BufBaseFloat):
    """
    
    This class inherits from :class:`BufBaseFloat`
    
    The type definition for a 32-bit `floating point`_ number within a ``.buf`` file
        
    """
    def __init__(self, isBigEndian: bool = False) -> None:
        """
        Constructs a new 32-bit `floating point`_ type
        
        Parameters
        ----------
        isBigEndian: :class:`bool`
            Whether the type is in big endian mode. **Default**: ``False``
        """
class BufFloat16(BufBaseFloat):
    """
    
    This class inherits from :class:`BufBaseFloat`
    
    The type definition for a 16-bit `half precision floating point`_ number within a ``.buf`` file
        
    """
    def __init__(self, isBigEndian: bool = False) -> None:
        """
        Constructs a new 16-bit `half precision floating point`_ type
        
        Parameters
        ----------
        isBigEndian: :class:`bool`
            Whether the type is in big endian mode. **Default**: ``False``
        """
class BufSignedInt(BufBaseInt):
    """
    
    This class inherits from :class:`BufBaseInt`
    
    The type definition for some signed integer type within a ``.buf`` file
        
    """
    def __init__(self, name: str = 'SignedInt32', size: typing.SupportsInt | typing.SupportsIndex = 4, isBigEndian: bool = False) -> None:
        """
        Constructs a new signed integer type
        
        Parameters
        ----------
        name: :class:`str`
            The name of the type. **Default**: ``"SignedInt32"``
        
        size: :class:`int`
            The byte size for the data type. **Default**: ``4``
        
        isBigEndian: :class:`bool`
            Whether the type is in big endian mode. **Default**: ``False``
        """
class BufType:
    """
    
    The common base for any type used to describe the structure of a ``.buf`` file
    
    .. note::
        Unlike the pure-Python original this replaces, this class has no ``decode``/``encode`` methods
        of its own -- see :class:`BufDataType`/:class:`BufElementType` (whose Python originals both
        overrode ``decode``/``encode`` with genuinely incompatible signatures -- a single value vs. a
        list of values -- that only Python's duck typing let share one base method name)
        
    """
    @property
    def name(self) -> str:
        """
        :class:`str`: The name of the type
        """
    @name.setter
    def name(self, arg1: str) -> None:
        ...
class BufUnSignedInt(BufBaseInt):
    """
    
    This class inherits from :class:`BufBaseInt`
    
    The type definition for some unsigned integer type within a ``.buf`` file
        
    """
    def __init__(self, name: str = 'UnsignedInt32', size: typing.SupportsInt | typing.SupportsIndex = 4, isBigEndian: bool = False) -> None:
        """
        Constructs a new unsigned integer type
        
        Parameters
        ----------
        name: :class:`str`
            The name of the type. **Default**: ``"UnsignedInt32"``
        
        size: :class:`int`
            The byte size for the data type. **Default**: ``4``
        
        isBigEndian: :class:`bool`
            Whether the type is in big endian mode. **Default**: ``False``
        """
class BufUnorm(BufBaseInt):
    """
    
    This class inherits from :class:`BufBaseInt`
    
    The type definition for an `unsigned normalized integer`_ number within a ``.buf`` file
        
    """
    def __init__(self, name: str, size: typing.SupportsInt | typing.SupportsIndex, isBigEndian: bool = False) -> None:
        """
        Constructs a new `unsigned normalized integer`_ type
        
        Parameters
        ----------
        name: :class:`str`
            The name of the type
        
        size: :class:`int`
            The byte size for the data type
        
        isBigEndian: :class:`bool`
            Whether the type is in big endian mode. **Default**: ``False``
        """
class CachedFileStats(FileStats):
    """
    
    This class inherits from :class:`FileStats`
    
    Adds tracking for a file retrieved via a cache hit, on top of what :class:`FileStats` already tracks
        
    """
    def __init__(self) -> None:
        ...
    def addHit(self, filePath: str) -> None:
        """
        Adds a new file path to the paths of cache hit files
        
        Parameters
        ----------
        filePath: :class:`str`
            the new file path that was hit
        """
    def update(self, modFolder: str | None = None, newFixed: collections.abc.Set[str] | None = None, newSkipped: collections.abc.Mapping[str, typing.Any] | None = None, newRemoved: collections.abc.Set[str] | None = None, newUndoed: collections.abc.Set[str] | None = None, newVisitedAtRemoval: collections.abc.Set[str] | None = None, newHit: collections.abc.Set[str] | None = None) -> None:
        """
        Same as :meth:`FileStats.update`, with an additional 'newHit' argument
        
        Parameters
        ----------
        newHit: Optional[Set[:class:`str`]]
            The new file paths that got a cache hit
        
            **Default**: ``None``
        """
    def updateHit(self, newHit: collections.abc.Set[str]) -> None:
        """
        Updates the file paths that have a cache hit
        
        Parameters
        ----------
        newHit: Set[:class:`str`]
            The new file paths that got a hit
        """
    @property
    def hit(self) -> set[str]:
        """
        Set[:class:`str`]: The paths to the files retrieved during a cache hit
        """
    @hit.setter
    def hit(self, arg0: collections.abc.Set[str]) -> None:
        ...
class CallGraph:
    """
    
    The result of :meth:`IniSectionGraph.buildCallGraph` -- a `call graph`_ over the
    :class:`IfContentPart`\\s of an :class:`IniSectionGraph`, suitable for the `dataflow analysis`_
    tools at :class:`GraphTools`
    
    Nodes are either an integer equal to ``id(part)`` for the real :class:`IfContentPart`, or a virtual
    ``("exit", id(part))`` node (only present for a part that actually makes a ``run =`` call)
    representing the point control reaches once that call has *returned* -- see :meth:`exitNodeOf`
        
    """
    def exitNodeOf(self, partId: typing.SupportsInt | typing.SupportsIndex) -> typing.Any:
        """
        Retrieves the node representing "once 'partId's own ``run =`` call (if it makes one) has returned"
        
        For a part that makes no call, this is just 'partId' itself -- there's no call to distinguish
        "before" from "after", so the part's own node already serves both purposes
        
        Parameters
        ----------
        partId: :class:`int`
            The ``id()`` of the :class:`IfContentPart` to look up (see :attr:`partsById`)
        
        Returns
        -------
        Any
            Either ``("exit", partId)`` (if the part makes a ``run =`` call) or 'partId' itself
        """
    @property
    def backwardEdges(self) -> dict:
        """
        Dict[Any, List[Any]]: The reverse of :attr:`forwardEdges`
        """
    @property
    def forwardEdges(self) -> dict:
        """
        Dict[Any, List[Any]]: ``node -> list of the nodes that can run directly after it``
        """
    @property
    def partsById(self) -> dict:
        """
        Dict[:class:`int`, :class:`IfContentPart`]: The ``id()`` of every reachable :class:`IfContentPart`, mapped to the part itself
        """
    @property
    def rootNodeIds(self) -> set:
        """
        Set[:class:`int`]: The ``id()`` of every part that's a genuine entry point of one of the graph's own target `section`_\\s
        """
class CppAhoCorasickDFA:
    """
    
    A class for an `Aho-Corasick`_ `DFA`_ implemented in C++
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: txt in x
    
            Determines if a keyword is found within 'txt'
    
        .. describe:: x[keyword]
    
            Retrieves the corresponding value to 'keyword'
    
        .. describe:: x[keyword] = val
    
            Sets the new `KVP`_
    
        .. describe:: len(x)
    
            Retrieves the number of elements
    
    Parameters
    ----------
    data: Optional[Dict[:class:`str`, T]]
        Any initial data to insert :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the keywords to put into the `DFA`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    handleDuplicate: Optional[Callable[[:class:`str`, T, T], T]]
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`
    
        The function takes in the following parameters:
    
        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_
    
        If this value is ``None``, will return the value of the new `KVP`_ by default :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
            
    """
    def __contains__(self, txt: str) -> bool:
        """
        Determines if a keyword is found within 'txt'
        """
    def __getitem__(self, keyword: str) -> typing.Any:
        """
        Retrieves the corresponding value to 'keyword'
        """
    def __init__(self, data: collections.abc.Mapping[str, typing.Any] | None = None, handleDuplicate: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any] | None = None) -> None:
        ...
    def __len__(self) -> int:
        """
        Retrieves the number of elements
        """
    def __setitem__(self, keyword: str, val: typing.Any) -> bool:
        """
        Sets the new `KVP`_
        """
    def add(self, keyword: str, value: typing.Any) -> bool:
        """
        Adds a new keyword
        
        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add
        
        value: T
            The value associated with the keyword
        
        Returns
        -------
        :class:`bool`
            Whether the keyword has already been inserted
        """
    def build(self, data: collections.abc.Mapping[str, typing.Any] | None = None) -> None:
        """
        Rebuilds the `DFA`_
        
        Parameters
        ----------
        data: Optional[Dict[:class:`str`, T]]
            Any initial data to put into the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the keywords to put into the `DFA`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def clear(self) -> None:
        """
        Clears the data
        """
    def contains(self, txt: str) -> bool:
        """
        Determines if 'txt' contains a corresponding keyword from the `DFA`_
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search
        
        Returns
        -------
        :class:`bool`
            Whether text contains a corresponding keyword
        """
    def find(self, txt: str) -> tuple[str | None, int]:
        """
        Finds the first keyword within 'txt'
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for the keyword
        
        Returns
        -------
        Tuple[Optional[:class:`str`], :class:`int`]
            Data of the found keyword containing:
        
            #. The keyword found
            #. The starting index of where the keyword was found. The index is only valid if the keyword is found.
        """
    def findAll(self, txt: str) -> dict[str, list[tuple[int, int]]]:
        """
        Finds all occurences of the keywords from the `DFA`_ in the given text
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords
        
        Returns
        -------
        Dict[:class:`str`, List[Tuple[:class:`int`, :class:`int`]]]
            The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`
        
            * The keys are the keywords found
            * The values are all instances of the keyword found
            * The tuple contains the starting index of the found instance and the ending index of the found instance
        """
    def findFirstAll(self, txt: str) -> dict[str, tuple[int, int]]:
        """
        Finds the first occurences of the keywords from the `DFA`_ in the given text
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords
        
        Returns
        -------
        Dict[:class:`str`, Tuple[:class:`int`, :class:`int`]]
            The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`
        
            * The keys are the keywords found
            * The tuple contains the starting index of the found instance and the ending index of the first found instance
        """
    def findMaximal(self, txt: str, count: typing.SupportsInt | typing.SupportsIndex = 1, pred: collections.abc.Callable[[str], bool] | None = None) -> tuple[str | None, int] | tuple[list[str], list[int]]:
        """
        Finds the first few largest keywords within 'txt'
        
        .. note::
            This function is a greedy version of :meth:`find` or `Maximal Munch`_ that consumes only a limited amount of tokens
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for the keyword
        
        count: :class:`int`
            The count of how many keywords to find in the search string :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``1``
        
        pred: Optional[Callable[[:class:`str`], :class:`bool`]]
            If provided, only a keyword satisfying this predicate can be picked -- among the keywords
            ending at a given position, the largest one satisfying 'pred' is picked, not necessarily the
            largest one overall :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        Tuple[Union[Optional[:class:`str`], List[:class:`str`]], Union[:class:`int`, List[:class:`int`]]]
            Data of the found keyword: :raw-html:`<br />` :raw-html:`<br />`
        
            * If the 'count' argument is less than or equal to 1, then the data will contain:
        
                #. The keyword found
                #. The starting index of where the keyword was found.
        
            * If the 'count' argument is greater than 1, then the data will contain:
        
                #. The list of keywords found
                #. The corresponding starting indices for where the keyword were found
        """
    @typing.overload
    def get(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None) -> tuple[str | None, typing.Any]:
        """
        Retrieves the corresponding value from the first keyword fround in 'txt'
        
        .. note::
            This function retrieves the corresponding value after running :meth:`find`
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword
        
        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found
        
        Raises
        ------
        :class:`KeyError`
            If no keywords are found
        
        Returns
        -------
        Tuple[Optional[:class:`str`], Union[T, Any]]
            Retrieves the following resultant data:
        
            #. The first keyword found
            #. Either the found value for the first keyword found or the value specified at 'default', if no keywords were found and
                'errorOnNotFound' is set to ``False``
        """
    @typing.overload
    def get(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None) -> tuple[str | None, typing.Any]:
        """
        Retrieves the corresponding value from the first keyword fround in 'txt'
        
        .. note::
            This function retrieves the corresponding value after running :meth:`find`
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword
        
        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`KeyError`
            If no keywords are found
        
        Returns
        -------
        Tuple[Optional[:class:`str`], Union[T, Any]]
            Retrieves the following resultant data:
        
            #. The first keyword found
            #. Either the found value for the first keyword found or the value specified at 'default', if no keywords were found and
                'errorOnNotFound' is set to ``False``
        """
    def getAll(self, txt: str) -> dict[str, typing.Any]:
        """
        Retrieves all the corresponding values to all the keywords found within 'txt'
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for keywords
        
        Returns
        -------
        Dict[:class:`str`, T]
            The corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the keywords found and the values are the values to the keywords
        """
    def getKeyVal(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the corresponding value of the key given in 'txt'
        
        Parameters
        ----------
        txt: :class:`str`
            The keyword to find the corresponding value
        
        errorOnNotFound: :class:`bool`  
            If no keywords are found, whether to raise an exception
        
        default: Any
            If 'errorsOnNotFound' is ``False``, then the default value to return if no keywords are found
        
        Raises
        ------
        :class:`KeyError`
            If the keyword is not found
        
        Returns
        -------
        Union[T, Any]
            Either the found value for the corresponding keyword or the value specified at 'default', if no keywords were found and
            'errorOnNotFound' is set to ``False``
        """
    def getMaximal(self, txt: str, errorOnNotFound: bool = True, default: typing.Any = None, count: typing.SupportsInt | typing.SupportsIndex = 1, pred: collections.abc.Callable[[str], bool] | None = None) -> tuple[str | None, typing.Any] | tuple[list[str], list[typing.Any]]:
        """
        Retrieves the corresponding value from the first largest keyword fround in 'txt'
        
        .. note::
            This function retrieves the corresponding value after running :meth:`findMaximal`
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search for a keyword
        
        errorOnNotFound: :class:`bool`
            If no keywords are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        count: :class:`int`
            The count of how many keywords to find in the search string :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``1``
        
        pred: Optional[Callable[[:class:`str`], :class:`bool`]]
            If provided, only a keyword satisfying this predicate can be picked -- among the keywords
            ending at a given position, the largest one satisfying 'pred' is picked, not necessarily the
            largest one overall :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`KeyError`
            If no keywords are found
        
        Returns
        -------
        Tuple[Union[Optional[:class:`str`], List[:class:`str`]], Union[T, Any, List[T]]]
            Retrieves the following resultant data: :raw-html:`<br />` :raw-html:`<br />`
        
            * If the 'count' argument is less than or equal to 1, then the data contains:
        
                #. The first largest keyword found
                #. Either the found value for the first largest keyword found or the value specified at 'default', if no keywords were found and
                   'errorOnNotFound' is set to ``False``
        
            * If the 'count' argument is greater than 1, then the data contains:
        
                #. The list of keywords found
                #. The corresponding found values to the keywords
        """
    def maximalStartsWith(self, txt: str) -> str | None:
        """
        Finds the largest keyword that is a prefix of the search text
        
        Parameters
        ----------
        txt: :class:`str`
            The text to search keywords
        
        Returns 
        -------
        Optional[:class:`str`]
            The keyword that is found to be the prefix of the search text, if available
        """
    @property
    def handleDuplicate(self) -> collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]:
        """
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`
        
        The function takes in the following parameters:
        
        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_
        
        :getter: Retrieves the function
        :setter: Sets the new function
        :type: Callable[[:class:`str`, T, T], T]
        """
    @handleDuplicate.setter
    def handleDuplicate(self, arg1: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]) -> None:
        ...
class CppAlgo:
    """
    C++ Tools for handling algorithm operations
    """
    @staticmethod
    def binarySearch(lst: collections.abc.Sequence[typing.Any], target: typing.Any, compare: collections.abc.Callable) -> tuple[bool, int]:
        """
        Performs a binary search for the target element in a sorted list.
        
        Parameters
        ----------
        lst: List[T]
            The sorted list to search.
        
        target: T
            The target element to search for.
        
        compare: Callable[[T, T], int]
            The compare function used to compare list elements with the target.
        
        Returns
        -------
        Tuple[bool, int]
            A tuple where the first value indicates whether the target was found,
            and the second value is the index of the found element or insertion point.
        """
    @staticmethod
    def merge(sorted_lsts: collections.abc.Sequence[collections.abc.Sequence[typing.Any]], compare: collections.abc.Callable) -> list:
        """
        Merges multiple sorted lists into one sorted list.
        
        Parameters
        ----------
        sorted_lsts: List[List[T]]
            The sorted lists to merge.
        
        compare: Callable[[T, T], int]
            The compare function used to order list elements.
        
        Returns
        -------
        List[T]
            The merged list of all input elements in sorted order.
        """
class CppBaseIniFixer:
    """
    
    The shared C++ base of every fixer, exposed so that one built on the C++ side -- by a
    :class:`IniFixBuilder`'s default factory, or by anything in ``AGRemapCore`` -- can still cross into
    `Python`_ :raw-html:`<br />` :raw-html:`<br />`
    
    Not usually what you want: a fixer created **from** `Python`_ is a :class:`BaseIniFixer`, which
    inherits from this and carries the extra `Python`_ state. This class exists so the boundary never
    has to hand back ``None`` for a core-side object it has no richer type for
        
    """
class CppBaseIniParser:
    """
    
    The shared C++ base of every parser, exposed so that one built on the C++ side -- by a
    :class:`IniParseBuilder`'s default factory, or by anything in ``AGRemapCore`` -- can still cross into
    `Python`_ :raw-html:`<br />` :raw-html:`<br />`
    
    Not usually what you want: a parser created **from** `Python`_ is a :class:`BaseIniParser`, which
    inherits from this and carries the extra `Python`_ state. This class exists so the boundary never
    has to hand back ``None`` for a core-side object it has no richer type for
        
    """
class CppBaseIniRemover:
    """
    
    The shared C++ base of every remover, exposed so that one built on the C++ side -- by a
    :class:`IniRemoveBuilder`'s default factory, or by anything in ``AGRemapCore`` -- can still cross into
    `Python`_ :raw-html:`<br />` :raw-html:`<br />`
    
    Not usually what you want: a remover created **from** `Python`_ is a :class:`BaseIniRemover`, which
    inherits from this and carries the extra `Python`_ state. This class exists so the boundary never
    has to hand back ``None`` for a core-side object it has no richer type for
        
    """
class CppBasePixelTransform:
    """
    
    Base class for transforming a pixel in a texture file
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x(pixel, xCoord, yCoord)
    
            Calls :meth:`transform` for the pixel transform, ``x``
        
    """
    def __call__(self, pixel: CppColour, x: typing.SupportsInt | typing.SupportsIndex, y: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Calls :meth:`transform` for the pixel transform
        """
    def __init__(self) -> None:
        ...
    def transform(self, pixel: CppColour, x: typing.SupportsInt | typing.SupportsIndex, y: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Applies a transformation to 'pixel'. No-op by default
        
        Parameters
        ----------
        pixel: :class:`CppColour`
            The pixel to be edited
        
        x: :class:`int`
            x-coordinate of the pixel
        
        y: :class:`int`
            y-coordinate of the pixel
        """
class CppBaseTexEditor:
    """
    
    Base class to edit some ``.dds`` file
        
    """
    def __init__(self) -> None:
        ...
    def fix(self, texFile: CppTextureFile, fixedTexFile: str) -> None:
        """
        Edits the texture file. No-op by default
        
        Parameters
        ----------
        texFile: :class:`CppTextureFile`
            The texture ``.dds`` file to be modified
        
        fixedTexFile: :class:`str`
            The name of the fixed texture file
        """
class CppBaseTexFilter:
    """
    
    Base class for transforming a texture file
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x(texFile)
    
            Calls :meth:`transform` for the filter, ``x``
        
    """
    def __call__(self, texFile: CppTextureFile) -> None:
        """
        Calls :meth:`transform` for the filter
        """
    def __init__(self) -> None:
        ...
    def transform(self, texFile: CppTextureFile) -> None:
        """
        Applies a transformation to 'texFile'. No-op by default
        
        Parameters
        ----------
        texFile: :class:`CppTextureFile`
            The texture to be edited
        """
class CppBufFile(BinaryFile):
    """
    
    This class inherits from :class:`BinaryFile`
    
    A class to handle ``.buf`` files
    
    A ``.buf`` file is a binary file made up of a sequence of same-sized "lines" (one line per vertex),
    each one composed of the same sequence of :class:`BufElementType`\\s -- there is no header or
    footer, just the lines themselves back-to-back
        
    """
    def __init__(self, src: typing.Any, elements: typing.Any, fileType: str = 'Buffer') -> None:
        """
        Constructs a new ``.buf`` file and immediately reads it
        
        Parameters
        ----------
        src: Union[:class:`str`, :class:`bytes`]
            The source file or bytes for the ``.buf`` file
        
        elements: List[:class:`BufElementType`]
            The sequence of elements within the ``.buf`` file -- each is cloned, so the same passed-in
            instance can safely be reused for other ``.buf`` files afterward
        
        fileType: :class:`str`
            The name for the type of ``.buf`` file. **Default**: ``"Buffer"``
        
        Raises
        ------
        :class:`BufFileNotRecognized`
            If 'src' holds a file path that cannot be read as a valid ``.buf`` file of this format
        
        :class:`BadBufData`
            If 'src' holds raw bytes that are not valid for this format
        """
    def decodeAll(self) -> dict:
        """
        Decodes the whole ``.buf`` file at once, column by column -- the bulk counterpart to
        :meth:`decodeLine`
        
        Where :meth:`decodeLine` builds a fresh dict per line, this decodes every line in C++ and hands
        back one `NumPy`_ array per column, so a whole file costs a single crossing into C++ instead of one
        per line. :meth:`BufTools.toDataFrame` is built on this
        
        .. note::
            Each array's dtype follows the data type it came from -- ``int64`` for a signed integer,
            ``uint64`` for an unsigned one and ``float64`` for a `floating point`_ one -- so an integer
            element stays integral rather than being widened to a float
        
        Returns
        -------
        Dict[Tuple[:class:`str`, :class:`int`], `numpy.ndarray`_]
            One entry per column, keyed by ``(elementKey, indexWithinElement)``, each holding that
            column's value for every line in line order
        """
    def decodeLine(self, src: bytes) -> dict[str, list[int | int | float]]:
        """
        Decodes a line (a vertex) within the ``.buf`` file
        
        Parameters
        ----------
        src: :class:`bytes`
            The source bytes to decode
        
        Returns
        -------
        Dict[:class:`str`, List[Any]]
            The decoded values for the line
        
            The keys are the names to the elements and the values are what is decoded
        """
    def encodeAll(self, columns: dict) -> None:
        """
        Encodes whole columns back into the ``.buf`` file's bytes -- the inverse of :meth:`decodeAll`, and
        the bulk counterpart to :meth:`encodeLine`. :meth:`BufTools.fromDataFrame` is built on this
        
        The columns are matched to the file's current :attr:`elements` by their key, so their order does not
        matter; a column the file has no data type for is ignored, and a data type with no matching column
        encodes as 0. The number of lines produced is the longest column's length
        
        .. note::
            :attr:`data` cannot be assigned directly, so this sets :attr:`src` to the newly encoded bytes
            and re-reads from it -- a ``.buf`` file originally constructed from a file path therefore ends
            up with raw bytes as its :attr:`src`, and the file on disk is untouched
        
        Parameters
        ----------
        columns: Dict[Tuple[:class:`str`, :class:`int`], `numpy.ndarray`_]
            The columns to encode, as produced by :meth:`decodeAll`
        
        Raises
        ------
        :class:`BadBufData`
            If the encoded bytes do not divide evenly into lines for the file's current :attr:`elements`
        """
    def encodeLine(self, src: collections.abc.Mapping[str, collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex | typing.SupportsInt | typing.SupportsIndex | typing.SupportsFloat | typing.SupportsIndex]]) -> bytes:
        """
        Encodes the data about a vertex to their corresponding bytes for the line
        
        Parameters
        ----------
        src: Dict[:class:`str`, List[Any]]
            The corresponding data for the vertex
        
            The keys are the names for the elements and the values are the data for the elements
        
        Returns
        -------
        :class:`bytes`
            The encoded bytes for the line
        """
    def fix(self, fixedFile: typing.Any = None, filters: typing.Any = None) -> typing.Any:
        """
        Fixes the ``.buf`` file
        
        Parameters
        ----------
        fixedFile: Optional[:class:`str`]
            The file path for the fixed ``.buf`` file. **Default**: ``None``
        
        filters: Optional[List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]]
            The filters to process each element, applied in order to each line
        
            The filters take in the following arguments:
        
            #. The data for a particular line
            #. The starting byte index of the line that is read
            #. The line index being processed (``i / bytesPerLine`` -- a `floating point`_ value, matching
               this codebase's pure-Python original exactly)
            #. The size of each line
        
            The output of the filters is the resultant data that consists where the keys are the names of
            the elements within a line in the ``.buf`` file and the values are the resultant data for each
            element in the line. **Default**: ``None``
        
        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If the argument ``fixedFile`` is ``None``, then will return an array of bytes for the fixed
            ``.buf`` file. Otherwise will return the filename to the fixed ``.buf`` file
        """
    def getDumpStr(self, prefix: str = 'vb0') -> str:
        """
        The **data** section of the dump text for this ``.buf`` file -- the text a 3dmigoto frame analysis
        writes, which `Blender`_ can then import
        
        One line per element per line (vertex), in the elements' declared order, shaped as
        ``prefix[lineInd]+byteOffset elementKey: value, value, ...``, with a blank line between lines:
        
        .. code-block::
        
            vb0[0]+000 POSITION: 1.0, 2.0, 3.0
            vb0[0]+012 TEXCOORD: 0.25, 0.5
        
            vb0[1]+000 POSITION: 4.0, 5.0, 6.0
            vb0[1]+012 TEXCOORD: 0.5, 0.5
        
        .. note::
            **This is deliberately only the data.** A real dump file also needs a header, and that header
            differs by the kind of buffer being dumped -- see :class:`VbFile` and :class:`IbFile`, which add
            one each
        
        .. note::
            An entry is named by its *element key*, so a second element sharing a name is suffixed with its
            occurrence (``TEXCOORD``, then ``TEXCOORD1``), matching both :meth:`decodeLine`'s own keys and
            what 3dmigoto writes
        
        Parameters
        ----------
        prefix: :class:`str`
            The buffer name each entry is prefixed with -- the vertex buffer slot a real dump was taken
            from :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``"vb0"``
        
        Returns
        -------
        :class:`str`
            The data section of the dump text. Empty when the file has no lines
        """
    def getFlatDumpStr(self, valueSep: str = ' ') -> str:
        """
        The **data** section of the dump text in an *index buffer*'s flat form -- every one of a line's
        values on one line, separated by 'valueSep', with no element name or byte offset
        
        This is what a ``.ib`` file's dump looks like (``0 1 2`` per triangular face), as opposed to the
        per-element form :meth:`getDumpStr` produces for a vertex buffer
        
        Parameters
        ----------
        valueSep: :class:`str`
            What to put between two values on the same line :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``" "``
        
        Returns
        -------
        :class:`str`
            The data section of the dump text. Empty when the file has no lines
        """
    def isValid(self) -> bool:
        """
        Whether the size of the data is divisible by the # of bytes per line
        
        Returns
        -------
        :class:`bool`
            Whether the provided data for the ``.buf`` file is valid
        """
    def merge(self, bufFiles: collections.abc.Sequence[CppBufFile]) -> None:
        """
        Merges several other ``.buf`` files into this one, line by line
        
        A GI character's vertex buffer does not live in one file -- it is split across a ``Position.buf``,
        a ``Blend.buf`` and a ``Texcoord.buf``, one line each per vertex. This stitches such a set back
        together: line *i* of the result is line *i* of every source concatenated in the order given, and
        this file's :attr:`elements` becomes every source's elements in that same order
        
        .. note::
            The sources are left untouched -- their elements are deep-copied in, so each stays usable
            afterwards
        
        .. note::
            The number of lines produced is the **smallest** line count among the sources, so a ragged set
            truncates rather than reading past the end of the shortest file
        
        .. note::
            :attr:`data` cannot be assigned directly, so this sets :attr:`src` to the merged bytes and
            re-reads from it -- this file ends up with raw bytes as its :attr:`src`
        
        Parameters
        ----------
        bufFiles: List[:class:`CppBufFile`]
            The ``.buf`` files to merge, in the byte order their elements should appear in a line
        
        Raises
        ------
        :class:`BadBufData`
            If the merged bytes do not divide evenly into lines
        """
    def read(self) -> bytes:
        """
        Reads the bytes in the ``.buf`` file
        
        Returns
        -------
        :class:`bytes`
            The read bytes
        
        Raises
        ------
        :class:`BufFileNotRecognized`
            If :attr:`src` holds a file path that cannot be read as a valid ``.buf`` file of this format
        
        :class:`BadBufData`
            If :attr:`src` holds raw bytes that are not valid for this format
        """
    def readDumpStr(self, text: str) -> None:
        """
        Reads dump text back into this ``.buf`` file's bytes -- the inverse of :meth:`getDumpStr`
        
        The values are encoded with this file's **current** :attr:`elements`, one text line per element and
        one blank-line-separated block per line (vertex), so a round trip out through :meth:`getDumpStr` and
        back in returns the bytes it started with
        
        .. note::
            A complete dump file works too, not just the data section :meth:`getDumpStr` returns: anything
            up to and including a ``vertex-data:`` marker is skipped. Each line's values are taken from
            after its last ``:``, so the ``prefix[i]+offset elementKey:`` part is ignored rather than having
            to match
        
        .. note::
            A block with fewer values than the elements need is zero-filled, and extra values are dropped,
            so every block always contributes exactly :attr:`bytesPerLine` bytes
        
        .. note::
            :attr:`data` cannot be assigned directly, so this sets :attr:`src` to the parsed bytes and
            re-reads from it
        
        Parameters
        ----------
        text: :class:`str`
            The dump text to read
        
        Raises
        ------
        :class:`BadBufData`
            If the parsed bytes do not divide evenly into lines
        """
    def readFlatDumpStr(self, text: str, valueSep: str = ' ') -> None:
        """
        Reads *index buffer* dump text back into this ``.buf`` file's bytes -- the inverse of
        :meth:`getFlatDumpStr`
        
        One text line per line of the file, its values separated by 'valueSep'
        
        .. note::
            A complete dump file works too: a header line is recognised by containing a ``:`` (every line of
            a ``.ib`` dump's header does, and none of its data lines do) and skipped
        
        Parameters
        ----------
        text: :class:`str`
            The dump text to read
        
        valueSep: :class:`str`
            What separates two values on the same line :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``" "``
        
        Raises
        ------
        :class:`BadBufData`
            If the parsed bytes do not divide evenly into lines
        """
    @property
    def bytesPerLine(self) -> int:
        """
        :class:`int`: The number of bytes per line (per vertex)
        """
    @property
    def elements(self) -> list[BufElementType]:
        """
        List[:class:`BufElementType`]: The sequence of elements within the ``.buf`` file
        
        Assigning a new list clones each new element the same way the constructor's own ``elements``
        parameter does
        """
    @elements.setter
    def elements(self, arg1: typing.Any) -> None:
        ...
    @property
    def fileType(self) -> str:
        """
        :class:`str`: The name for the type of ``.buf`` file
        """
    @fileType.setter
    def fileType(self, arg1: str) -> None:
        ...
class CppColour:
    """
    
    Class to store data for a colour
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: hash(x)
    
            Retrieves the hash id for the colour based off :meth:`getId`
        
    """
    @staticmethod
    def boolToColourChannel(val: bool, min: typing.SupportsInt | typing.SupportsIndex = 0, max: typing.SupportsInt | typing.SupportsIndex = 255) -> int:
        """
        Converts a boolean value to a value for a colour channel
        
        Parameters
        ----------
        val: :class:`bool`
            The boolean value to convert
        
        min: :class:`int`
            The minimum bound for the colour channel. **Default**: ``0``
        
        max: :class:`int`
            The maximum bound for the colour channel. **Default**: ``255``
        
        Returns
        -------
        :class:`int`
            The corresponding value for the colour channel based off the boolean
        """
    @staticmethod
    def boundColourChannel(val: typing.SupportsInt | typing.SupportsIndex, min: typing.SupportsInt | typing.SupportsIndex = 0, max: typing.SupportsInt | typing.SupportsIndex = 255) -> int:
        """
        Makes a colour channel value be in between the minimum and maximum value
        
        Parameters
        ----------
        val: :class:`int`
            The value of the channel
        
        min: :class:`int`
            The minimum bound for the colour channel. **Default**: ``0``
        
        max: :class:`int`
            The maximum bound for the colour channel. **Default**: ``255``
        
        Returns
        -------
        :class:`int`
            The bounded value
        """
    def __hash__(self) -> int:
        ...
    def __init__(self, red: typing.SupportsInt | typing.SupportsIndex = 255, green: typing.SupportsInt | typing.SupportsIndex = 255, blue: typing.SupportsInt | typing.SupportsIndex = 255, alpha: typing.SupportsInt | typing.SupportsIndex = 255) -> None:
        """
        Constructs a new colour
        
        Parameters
        ----------
        red: :class:`int`
            The red channel for the colour. **Default**: ``255``
        
        green: :class:`int`
            The green channel for the colour. **Default**: ``255``
        
        blue: :class:`int`
            The blue channel for the colour. **Default**: ``255``
        
        alpha: :class:`int`
            The transparency (alpha) channel for the colour, with a range from 0-255. 0 = transparent,
            255 = opaque. **Default**: ``255``
        """
    def copy(self, colour: CppColour, withAlpha: bool = True) -> None:
        """
        Copies the colour value from 'colour'
        
        Parameters
        ----------
        colour: :class:`CppColour`
            The colour to copy from
        
        withAlpha: :class:`bool`
            Whether to also copy the alpha channel. **Default**: ``True``
        """
    def fromTuple(self, colourTuple: tuple[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]) -> None:
        """
        Updates the colour based off 'colourTuple'
        
        Parameters
        ----------
        colourTuple: Tuple[:class:`int`, :class:`int`, :class:`int`, :class:`int`]
            The raw values for the colour in RGBA format
        """
    def getId(self) -> str:
        """
        Retrieves a unique id for the colour
        
        .. note::
            The id generated will not correspond to any id generated from a colour range
        
        Returns
        -------
        :class:`str`
            The id for the colour
        """
    def getTuple(self) -> tuple[int, int, int, int]:
        """
        Retrieves the tuple representation of the colour in RGBA format
        
        Returns
        -------
        Tuple[:class:`int`, :class:`int`, :class:`int`, :class:`int`]
            The colour tuple containing the following colour channel values, in order:
        
            #. Red
            #. Green
            #. Blue
            #. Alpha
        """
    def match(self, colour: CppColour) -> bool:
        """
        Whether 'colour' matches this colour
        
        Parameters
        ----------
        colour: :class:`CppColour`
            The colour to check
        
        Returns
        -------
        :class:`bool`
            Whether the colour matches this colour
        """
    @property
    def alpha(self) -> int:
        """
        :class:`int`: The transparency (alpha) channel for the colour, with a range from 0-255. 0 =
        transparent, 255 = opaque
        """
    @alpha.setter
    def alpha(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def blue(self) -> int:
        """
        :class:`int`: The blue channel for the colour
        """
    @blue.setter
    def blue(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def green(self) -> int:
        """
        :class:`int`: The green channel for the colour
        """
    @green.setter
    def green(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def red(self) -> int:
        """
        :class:`int`: The red channel for the colour
        """
    @red.setter
    def red(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class CppColourRange:
    """
    
    Class to store a range for a colour
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: hash(x)
    
            Retrieves the hash id for the colour range based off :meth:`getId`
        
    """
    def __hash__(self) -> int:
        ...
    def __init__(self, min: CppColour, max: CppColour) -> None:
        """
        Constructs a new colour range
        
        Parameters
        ----------
        min: :class:`CppColour`
            The minimum range for the RGBA values
        
        max: :class:`CppColour`
            The maximum range for the RGBA values
        """
    def getId(self) -> str:
        """
        Retrieves a unique id for the colour range
        
        .. note::
            The id generated will not correspond to any id generated for a single colour
        
        Returns
        -------
        :class:`str`
            The id for the colour range
        """
    def match(self, colour: CppColour) -> bool:
        """
        Whether 'colour' is within the colour range
        
        Parameters
        ----------
        colour: :class:`CppColour`
            The colour to check
        
        Returns
        -------
        :class:`bool`
            Whether the colour is within the colour range
        """
    @property
    def max(self) -> CppColour:
        """
        :class:`CppColour`: The maximum range for the RGBA values
        """
    @max.setter
    def max(self, arg0: CppColour) -> None:
        ...
    @property
    def min(self) -> CppColour:
        """
        :class:`CppColour`: The minimum range for the RGBA values
        """
    @min.setter
    def min(self, arg0: CppColour) -> None:
        ...
class CppColourReplace(CppBasePixelTransform):
    """
    
    This class inherits from :class:`CppBasePixelTransform`
    
    Replaces a coloured pixel
        
    """
    def __init__(self, replaceColour: CppColour, coloursToReplace: typing.Any = None, replaceAlpha: bool = True) -> None:
        """
        Constructs a new colour-replace pixel transform
        
        Parameters
        ----------
        replaceColour: :class:`CppColour`
            The colour to fill in
        
        coloursToReplace: Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]
            The colours to find to be replaced. If this value is ``None``, then will always replace the
            colour of the pixel. **Default**: ``None``
        
        replaceAlpha: :class:`bool`
            Whether to also replace the alpha channel of the original colour. **Default**: ``True``
        """
    @property
    def coloursToReplace(self) -> typing.Any:
        """
        Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]: The colours to find to be
        replaced. If this value is ``None``, then will always replace the colour of the pixel
        """
    @coloursToReplace.setter
    def coloursToReplace(self, arg1: typing.Any) -> None:
        ...
    @property
    def replaceAlpha(self) -> bool:
        """
        :class:`bool`: Whether to also replace the alpha channel of the original colour
        """
    @replaceAlpha.setter
    def replaceAlpha(self, arg0: bool) -> None:
        ...
    @property
    def replaceColour(self) -> CppColour:
        """
        :class:`CppColour`: The colour to fill in
        """
    @replaceColour.setter
    def replaceColour(self, arg0: CppColour) -> None:
        ...
class CppColourReplaceFilter(CppBaseTexFilter):
    """
    
    This class inherits from :class:`CppBaseTexFilter`
    
    Replaces specific colours in the image
        
    """
    def __init__(self, replaceColour: CppColour, coloursToReplace: typing.Any = None, replaceAlpha: bool = True) -> None:
        """
        Constructs a new colour-replace filter
        
        Parameters
        ----------
        replaceColour: :class:`CppColour`
            The colour to fill in
        
        coloursToReplace: Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]
            The colours to find to be replaced. If this value is ``None``, then will always replace the
            colour of the pixel. **Default**: ``None``
        
        replaceAlpha: :class:`bool`
            Whether to also replace the alpha channel of the original colour. **Default**: ``True``
        """
    def transform(self, texFile: typing.Any) -> None:
        """
        Replaces the matching colours across the entire image
        
        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be edited
        """
    @property
    def coloursToReplace(self) -> typing.Any:
        """
        Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]: The colours to find to be
        replaced. If this value is ``None``, then will always replace the colour of the pixel
        """
    @coloursToReplace.setter
    def coloursToReplace(self, arg1: typing.Any) -> None:
        ...
    @property
    def replaceAlpha(self) -> bool:
        """
        :class:`bool`: Whether to also replace the alpha channel of the original colour
        """
    @replaceAlpha.setter
    def replaceAlpha(self, arg0: bool) -> None:
        ...
    @property
    def replaceColour(self) -> CppColour:
        """
        :class:`CppColour`: The colour to fill in
        """
    @replaceColour.setter
    def replaceColour(self, arg0: CppColour) -> None:
        ...
class CppCorrectGamma(CppBasePixelTransform):
    """
    
    This class inherits from :class:`CppBasePixelTransform`
    
    Performs a `Gamma Correction`_ on an individual pixel using the following simple power-law
    relationship:
    
    .. code-block::
    
        V_out = V_in ^ (1 / gamma)
    
    Where ``V_out`` is the perceived brightness by human eyes while ``V_in`` is the actual brightness
    of the image.
    
    .. note::
        Higher :attr:`gamma` values make the image look brighter and less saturated; lower
        :attr:`gamma` values make the image look darker and more saturated
        
    """
    @staticmethod
    def correctGamma(pixelValue: typing.SupportsInt | typing.SupportsIndex, gamma: typing.SupportsFloat | typing.SupportsIndex) -> int:
        """
        The equation for the gamma correction done at every colour channel pixel
        
        Parameters
        ----------
        pixelValue: :class:`int`
            The value of the pixel for some colour channel, in [0, 255]
        
        gamma: :class:`float`
            The luminance parameter for how bright humans perceive the image
        
        Returns
        -------
        :class:`int`
            The gamma corrected pixel value
        """
    def __init__(self, gamma: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
        Constructs a new gamma-correction pixel transform
        
        Parameters
        ----------
        gamma: :class:`float`
            The luminance parameter for how bright humans perceive the image
        """
    @property
    def gamma(self) -> float:
        """
        :class:`float`: The luminance parameter for how bright humans perceive the image
        """
    @gamma.setter
    def gamma(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class CppGammaFilter(CppBaseTexFilter):
    """
    
    This class inherits from :class:`CppBaseTexFilter`
    
    Performs a `Gamma Correction`_ on the texture file, using the following simple power-law
    relationship, applied independently to every pixel's R/G/B channels (the alpha channel is left
    untouched):
    
    .. code-block::
    
        V_out = V_in ^ (1 / gamma)
    
    Where ``V_out`` is the perceived brightness by human eyes while ``V_in`` is the actual brightness
    of the image.
    
    .. note::
        Higher :attr:`gamma` values make the image look brighter and less saturated; lower
        :attr:`gamma` values make the image look darker and more saturated
        
    """
    def __init__(self, gamma: typing.SupportsFloat | typing.SupportsIndex) -> None:
        """
        Constructs a new gamma filter
        
        Parameters
        ----------
        gamma: :class:`float`
            The luminance parameter for how bright humans perceive the image
        """
    @property
    def gamma(self) -> float:
        """
        :class:`float`: The luminance parameter for how bright humans perceive the image
        """
    @gamma.setter
    def gamma(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class CppGlobalModTypes:
    """
    
    Every :class:`ModType` the software ships with, and the one place that files them into
    :class:`ModTypeIdTools`'s global registry
    
    The counterpart to the pure-Python :class:`ModTypes` enum, whose ``getAll()`` likewise builds the
    shipped mod types on demand
    
    .. important::
        :meth:`registerAll` is **not** called automatically by anything in ``AGRemapCore``, and that is
        deliberate. :meth:`ModTypeIdTools.getModType` and :meth:`ModTypeIdTools.findByName` report only
        what was explicitly registered, which is what lets a caller do :meth:`ModTypeIdTools.clear`
        followed by :meth:`ModTypeIdTools.registerModType` and get a registry holding *exactly* the mod
        types it asked for. Self-populating those lookups on first use would quietly break that
    
        :meth:`registerMissing` **is** called automatically, by :meth:`GlobalIniClassifiers.classifier`
        -- but only ever to fill in ids nothing has registered, never to overwrite one a caller
        registered for itself
        
    """
    @staticmethod
    def all() -> list[ModType]:
        """
        Retrieves every shipped :class:`ModType`, freshly built on each call
        
        Fresh rather than shared because a :class:`ModType` owns mutable asset tables, so one caller
        adding a hash must not be visible to every other one
        
        Returns
        -------
        List[:class:`ModType`]
            All the shipped mod types
        """
    @staticmethod
    def registerAll() -> None:
        """
        Files every mod type from :meth:`all` into :class:`ModTypeIdTools`'s global registry, so
        :meth:`ModTypeIdTools.getModType` can resolve them by id and :meth:`ModTypeIdTools.findByName` by
        name or alias
        
        Idempotent: registering a mod type twice replaces the existing entry rather than duplicating it.
        Note it registers *in addition to* whatever is already there rather than replacing the registry --
        call :meth:`ModTypeIdTools.clear` first to start from empty
        """
    @staticmethod
    def registerMissing() -> None:
        """
        Files every shipped :class:`ModType` that is **not already registered** into
        :class:`ModTypeIdTools`'s global registry, leaving every id the caller registered for itself alone
        
        The difference from :meth:`registerAll` is only what happens on a collision: that one overwrites,
        this one yields. This is what the implicit population behind :meth:`GlobalIniClassifiers.classifier`
        uses, so that classifying a .ini file can no longer silently replace a :class:`ModType` you
        registered under one of the shipped ids
        """
class CppHashTools:
    """
    C++ tools for deterministically hashing data
    """
    @staticmethod
    def clear() -> None:
        """
        Clears any saved internal state this class accumulates across calls (currently just the
        collision-disambiguation frequency counts used by :meth:`getShortDeterministicHashStr`)
        """
    @staticmethod
    @typing.overload
    def getDeterministicHash(data: bytes) -> Hash128:
        """
        Deterministically hashes a buffer of bytes
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`Hash128`
            The resultant deterministic hash
        """
    @staticmethod
    @typing.overload
    def getDeterministicHash(str: str) -> Hash128:
        """
        Deterministically hashes a string
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`Hash128`
            The resultant deterministic hash
        """
    @staticmethod
    @typing.overload
    def getDeterministicHashStr(data: bytes) -> str:
        """
        Deterministically hashes a buffer of bytes
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`str`
            The resultant deterministic hash, as a base64 string (see :meth:`Hash128.toBase64`)
        """
    @staticmethod
    @typing.overload
    def getDeterministicHashStr(str: str) -> str:
        """
        Deterministically hashes a string
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`str`
            The resultant deterministic hash, as a base64 string (see :meth:`Hash128.toBase64`)
        """
    @staticmethod
    @typing.overload
    def getShortDeterministicHashStr(data: bytes) -> str:
        """
        Deterministically hashes a buffer of bytes into a short, compact base64 string :raw-html:`<br />` :raw-html:`<br />`
        
        The hash is reduced modulo :math:`2^{16}` before being converted to base64, so unlike
        :meth:`getDeterministicHashStr`, collisions across different inputs are expected. To
        disambiguate a collision, every occurrence of a short hash value after the first has
        ``_<frequency>`` appended, where ``<frequency>`` (itself base64-encoded) counts how many times
        that short hash value has already been produced by this method
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`str`
            The resultant short, possibly-colliding hash, as a base64 string
        """
    @staticmethod
    @typing.overload
    def getShortDeterministicHashStr(str: str) -> str:
        """
        Deterministically hashes a string into a short, compact base64 string :raw-html:`<br />` :raw-html:`<br />`
        
        See :meth:`getShortDeterministicHashStr` (the ``bytes`` overload) for the full explanation of
        the collision-disambiguation behaviour
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`str`
            The resultant short, possibly-colliding hash, as a base64 string
        """
class CppHighlightShadow(CppBasePixelTransform):
    """
    
    This class inherits from :class:`CppBasePixelTransform`
    
    A filter that approximates the adjustment of the shadow/highlight of an image
    
    .. note::
        Reference: `Highlight Shadow Approximation Reference`_
        
    """
    def __init__(self, highlight: typing.SupportsFloat | typing.SupportsIndex = 0, shadow: typing.SupportsFloat | typing.SupportsIndex = 0) -> None:
        """
        Constructs a new highlight/shadow pixel transform
        
        Parameters
        ----------
        highlight: :class:`float`
            The amount of highlight to apply to the pixel. Range from -1 to 1, and 0 = no change.
            **Default**: ``0``
        
        shadow: :class:`float`
            The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change.
            **Default**: ``0``
        """
    @property
    def highlight(self) -> float:
        """
        :class:`float`: The amount of highlight to apply to the pixel. Range from -1 to 1, and 0 = no change
        """
    @highlight.setter
    def highlight(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
    @property
    def shadow(self) -> float:
        """
        :class:`float`: The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change
        """
    @shadow.setter
    def shadow(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class CppHueAdjust(CppBaseTexFilter):
    """
    
    This class inherits from :class:`CppBaseTexFilter`
    
    Adjusts the hue of a texture file
        
    """
    def __init__(self, hue: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Constructs a new hue-adjust filter
        
        Parameters
        ----------
        hue: :class:`int`
            The hue to adjust the image. Value is from -180 to 180
        """
    def transform(self, texFile: typing.Any) -> None:
        """
        Adjusts the hue across the image
        
        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be edited
        """
    @property
    def hue(self) -> int:
        """
        :class:`int`: The hue to adjust the image. Value is from -180 to 180
        """
    @hue.setter
    def hue(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class CppIniFixBuilderArgs:
    """
    
    The version-dependent lookup table a :class:`IniFixBuilder` resolves its factory from
    
    Opaque: there is no way to build one from Python yet. It is exposed so a builder that *has* one --
    every mod type from :meth:`GIBuilder.all` does -- can say so
        
    """
class CppIniParseBuilderArgs:
    """
    
    The version-dependent lookup table a :class:`IniParseBuilder` resolves its factory from
    
    Opaque: there is no way to build one from Python yet. It is exposed so a builder that *has* one --
    every mod type from :meth:`GIBuilder.all` does -- can say so
        
    """
class CppIniRemoveBuilderArgs:
    """
    
    The version-dependent lookup table a :class:`IniRemoveBuilder` resolves its factory from
    
    Opaque: there is no way to build one from Python yet. It is exposed so that a builder which
    *has* one -- every mod type from :meth:`GIBuilder.all` does -- can say so
        
    """
class CppIntTools:
    """
    C++ Tools for handling integers
    """
    @staticmethod
    def toBase(num: typing.SupportsInt | typing.SupportsIndex, base: typing.SupportsInt | typing.SupportsIndex) -> tuple[list[int], bool]:
        """
                                Converts a base 10 number to an arbitrary base number
        
                                Parameters
                                ----------
                                num: :class:`int`
                                    The base 10 number to convert
        
                                base: :class:`int`
                                    The base to convert to
        
                                Raises
                                ------
                                :class:`TypeError`
                                    The base is smaller or equal to 1
        
                                Returns
                                -------
                                Tuple[List[:class:`int`], :class:`bool`]
                                    Retrieves the following data in the tuple:
        
                                    #. The digits in the converted number
                                    #. Whether the number is negative
        """
    @staticmethod
    def toBase64(num: typing.SupportsInt | typing.SupportsIndex, getDigit: collections.abc.Sequence[str] | None = None, negativeChar: str = '-') -> str:
        """
        Converts a base 10 number to a base 64 number
        
        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert
        
        getDigit: List[:class:`str`]
            how to get the string representation of a digit. :raw-html:`<br />` :raw-html:`<br />`
        
            * If this argument is a list, each element is the string representation of the digit at the particular index of the string/list.
            * If this argument is ``None``, then will use the following string for each digit:
        
            ``ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_``
        
            This is the same digit representation as the `standard base 64`_ except that the 63rd digit (``/``) is replaced with the ``_`` character :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        negativeChar: :class:`str`
            The character representation for the negative symbol :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``"-"``
        
        Returns
        -------
        :class:`str`
            The converted string representation of the arbitrary base 64 number
        """
    @staticmethod
    def toStrBase(num: typing.SupportsInt | typing.SupportsIndex, base: typing.SupportsInt | typing.SupportsIndex, getDigit: collections.abc.Sequence[str], negativeChar: str) -> str:
        """
        Converts a base 10 number to an arbitrary base number, such that the characters in this arbitrary based number
        are all characters
        
        Parameters
        ----------
        num: :class:`int`
            The base 10 number to convert
        
        base: :class:`int`
            The base to convert to
        
        getDigit: List[:class:`str`]
            The string representations of each digit. Each element is the string representation
            of the digit at the particular index of the list.
        
        negativeChar: :class:`str`
            The character representation for the negative symbol
        
        Returns
        -------
        :class:`str`
            The converted string representation of the arbitrary base number
        """
class CppInvertAlpha(CppBasePixelTransform):
    """
    
    This class inherits from :class:`CppBasePixelTransform`
    
    Inverts the alpha channel of a pixel
        
    """
    def __init__(self) -> None:
        ...
class CppInvertAlphaFilter(CppBaseTexFilter):
    """
    
    This class inherits from :class:`CppBaseTexFilter`
    
    Inverts the alpha channel of an image
        
    """
    def __init__(self) -> None:
        ...
    def transform(self, texFile: typing.Any) -> None:
        """
        Inverts the alpha channel across the image
        
        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be edited
        """
class CppListTools:
    """
    C++ Tools for handling with Lists
    """
    @staticmethod
    def addLstsByInds(lst: list, subLsts: collections.abc.Mapping[typing.SupportsInt | typing.SupportsIndex, list]) -> list:
        """
                                Inserts multiple sublists into the main list by index
        
                                Parameters
                                ----------
                                lst: List[T]
                                    The main list to work with
                                
                                subLsts: Dict[:class:`int`, List[T]]
                                    The sublists to insert into the main list :raw-html:`<br />` :raw-html:`<br />`
        
                                    The keys are the indices to insert the sublists and the values are the sublists
        
                                Returns
                                -------
                                List[T]
                                    The resultant combined list
        """
    @staticmethod
    def getIndsAfterRemove(removedInds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], lstLen: typing.SupportsInt | typing.SupportsIndex) -> list[int]:
        """
                                Retrieve the index shifts in some data structure,
                                after the list got elements removed by indices
        
                                Parameters
                                ----------
                                removedInds: List[:class:`int`] 
                                    The indices to elements that got removed from the list :raw-html:`<br />` :raw-html:`<br />`
        
                                    Assume that the list in sorted order
        
                                lstLen: :class:`int`
                                    The length of the original list, before its elements got removed
        
                                Returns
                                -------
                                List[:class:`int`]
                                    A list containing how much each index is shifted
        """
    @staticmethod
    def removeByInds(lst: list, inds: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> list:
        """
                                Removes many indices from a list
        
                                Parameters
                                ----------
                                lst: List[T]
                                    The desired list to have its parts removed
        
                                inds: Set[:class:`int`]
                                    The indices to the elements in the list that needs to be removed :raw-html:`<br />` :raw-html:`<br />`
        
                                Returns
                                -------
                                List[T]
                                    The new list with elements specified by indices removed
        """
    @staticmethod
    def removeParts(lst: list, part_indices: list) -> list:
        """
                                Removes many indices from a list
        
                                Parameters
                                ----------
                                lst: List[T]
                                    The desired list to have its parts removed
        
                                inds: Set[:class:`int`]
                                    The indices to the elements in the list that needs to be removed
        
                                Returns
                                -------
                                List[T]
                                    The new list with elements specified by indices removed
        """
class CppPixelFilter(CppBaseTexFilter):
    """
    
    This class inherits from :class:`CppBaseTexFilter`
    
    Manipulates each pixel within an image
    
    .. note::
        Every whole-image filter in this codebase (eg. :class:`ColourReplaceFilter`) is, under the
        hood, also just a C++ loop over every pixel -- `Compressonator`_ has no vectorized whole-image
        pixel-remap API the way `Pillow`_ did for the pure-Python original, so there's no "whole image
        at once" fast path left to prefer instead. A :class:`CppBasePixelTransform` placed in
        :attr:`transforms` runs directly in C++ for every pixel, at the same cost as a dedicated
        filter's own inlined loop body -- only a plain Python callable placed in :attr:`transforms`
        still pays a real per-pixel Python call
        
    """
    def __init__(self) -> None:
        ...
    def transform(self, texFile: typing.Any) -> None:
        """
        Changes each individual pixel in the image
        
        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be edited
        """
class CppTempControl(CppBasePixelTransform):
    """
    
    This class inherits from :class:`CppBasePixelTransform`
    
    Controls the temperature of a texture file using a modified version of the
    `Simple Image Temperature/Tint Adjust Algorithm`_ such that the colour channels increase/decrease
    linearly with respect to their corresponding pixel value and the user selected temperature
        
    """
    def __init__(self, temp: typing.SupportsFloat | typing.SupportsIndex = 0) -> None:
        """
        Constructs a new temperature-control pixel transform
        
        Parameters
        ----------
        temp: :class:`float`
            The temperature to set the image. Range from -1 to 1. **Default**: ``0``
        """
    @property
    def temp(self) -> float:
        """
        :class:`float`: The temperature to set the image. Range from -1 to 1
        """
    @temp.setter
    def temp(self, arg0: typing.SupportsFloat | typing.SupportsIndex) -> None:
        ...
class CppTexCreator(CppBaseTexEditor):
    """
    
    This class inherits from :class:`CppBaseTexEditor`
    
    Creates a brand new ``.dds`` file if the file does not already exist
        
    """
    def __init__(self, width: typing.SupportsInt | typing.SupportsIndex, height: typing.SupportsInt | typing.SupportsIndex, colour: CppColour = ...) -> None:
        """
        Constructs a new texture creator
        
        Parameters
        ----------
        width: :class:`int`
            The width, in pixels, of the texture to create
        
        height: :class:`int`
            The height, in pixels, of the texture to create
        
        colour: :class:`CppColour`
            The fill colour of the texture to create. **Default**: opaque white
        """
    @property
    def colour(self) -> CppColour:
        """
        :class:`CppColour`: The fill colour of the texture to create
        """
    @colour.setter
    def colour(self, arg0: CppColour) -> None:
        ...
    @property
    def height(self) -> int:
        """
        :class:`int`: The height, in pixels, of the texture to create
        """
    @height.setter
    def height(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def width(self) -> int:
        """
        :class:`int`: The width, in pixels, of the texture to create
        """
    @width.setter
    def width(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class CppTexEditor(CppBaseTexEditor):
    """
    
    This class inherits from :class:`CppBaseTexEditor`
    
    The pure-C++-SDK-facing engine behind :class:`TexEditor` -- runs a fixed sequence of
    :class:`CppBaseTexFilter`\\s over a texture file. :meth:`~CppBaseTexEditor.fix` is a no-op unless a
    filter list was passed to the constructor.
    
    .. note::
        The Python-facing :class:`TexEditor` overrides :meth:`~CppBaseTexEditor.fix` itself instead of
        using this class's filter list, so that its own ``filters`` attribute can hold arbitrary Python
        callables (not just objects this constructor can accept) -- see that class for the
        Python-visible behavior
        
    """
    def __init__(self) -> None:
        ...
class CppTextureFile:
    """
    
    The `Compressonator`_-backed engine behind :class:`TextureFile` -- decodes/encodes a ``.dds``
    texture file to/from a flat, uncompressed RGBA8 pixel buffer (see :meth:`getPixels`/
    :meth:`setPixels`), remembering the original compressed format so :meth:`save` can re-encode back
    to it.
    
    .. note::
        This class is Pillow-free -- :class:`TextureFile` itself layers a real `Pillow`_ ``Image`` (at
        its ``img`` attribute) on top of this class's raw pixel buffer, for the sake of the other
        texture filters in this codebase that still work directly against a real Pillow image
        
    """
    def __init__(self, src: str) -> None:
        """
        Constructs a new texture file. Does not read anything from disk yet -- see :meth:`open`
        
        Parameters
        ----------
        src: :class:`str`
            The source file path for the texture file
        """
    def getPixel(self, x: typing.SupportsInt | typing.SupportsIndex, y: typing.SupportsInt | typing.SupportsIndex) -> CppColour:
        """
        The colour of the pixel at ('x', 'y'). No bounds checking is performed
        
        Parameters
        ----------
        x: :class:`int`
            The x-coordinate of the pixel
        
        y: :class:`int`
            The y-coordinate of the pixel
        
        Returns
        -------
        :class:`CppColour`
            The colour of the pixel
        """
    def getPixels(self) -> bytes:
        """
        The current pixel buffer, as flat RGBA8 bytes (4 bytes per pixel, row-major)
        
        Returns
        -------
        :class:`bytes`
            The current pixel buffer
        """
    def open(self) -> None:
        """
        Opens the texture file at :attr:`src`, decoding it into :meth:`getPixels`
        
        If the file does not exist, :attr:`hasImage` becomes ``False`` and :meth:`getPixels` is cleared
        """
    def save(self) -> None:
        """
        Saves :meth:`getPixels` to the texture file at :attr:`src`
        
        If :attr:`gamma` is set, the R/G/B channels of :meth:`getPixels` are gamma-corrected first (see
        :class:`CppGammaFilter`), in place. The file is re-encoded to whatever compressed format it was
        originally :meth:`open`-ed with -- or, for a texture file that was never successfully opened (eg. a
        brand new file), BC7
        """
    def setPixel(self, x: typing.SupportsInt | typing.SupportsIndex, y: typing.SupportsInt | typing.SupportsIndex, colour: CppColour) -> None:
        """
        Sets the colour of the pixel at ('x', 'y'). No bounds checking is performed
        
        Parameters
        ----------
        x: :class:`int`
            The x-coordinate of the pixel
        
        y: :class:`int`
            The y-coordinate of the pixel
        
        colour: :class:`CppColour`
            The new colour for the pixel
        """
    def setPixels(self, pixels: bytes, width: typing.SupportsInt | typing.SupportsIndex, height: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Replaces the current pixel buffer, eg. with pixels edited outside of this class
        
        Parameters
        ----------
        pixels: :class:`bytes`
            The new flat RGBA8 pixel buffer (4 bytes per pixel, row-major)
        
        width: :class:`int`
            The width, in pixels, of 'pixels'
        
        height: :class:`int`
            The height, in pixels, of 'pixels'
        """
    @property
    def gamma(self) -> float | None:
        """
        Optional[:class:`float`]: The luminance parameter used to gamma-correct the R/G/B channels on the
        next :meth:`save`, or ``None`` to skip gamma correction entirely
        """
    @gamma.setter
    def gamma(self, arg1: typing.SupportsFloat | typing.SupportsIndex | None) -> None:
        ...
    @property
    def hasImage(self) -> bool:
        """
        :class:`bool`: Whether a texture is currently loaded (:meth:`open` succeeded and found a real file)
        """
    @property
    def height(self) -> int:
        """
        :class:`int`: The height, in pixels, of the currently loaded texture (0 if :attr:`hasImage` is
        ``False``)
        """
    @property
    def src(self) -> str:
        """
        :class:`str`: The source file path for the texture file
        """
    @src.setter
    def src(self, arg1: str) -> None:
        ...
    @property
    def width(self) -> int:
        """
        :class:`int`: The width, in pixels, of the currently loaded texture (0 if :attr:`hasImage` is
        ``False``)
        """
class CppTintTransform(CppBasePixelTransform):
    """
    
    This class inherits from :class:`CppBasePixelTransform`
    
    Controls the tint of a texture file using the `Simple Image Temperature/Tint Adjust Algorithm`_
        
    """
    def __init__(self, tint: typing.SupportsInt | typing.SupportsIndex = 0) -> None:
        """
        Constructs a new tint pixel transform
        
        Parameters
        ----------
        tint: :class:`int`
            The tint to set the image. Range from -100 to 100. **Default**: ``0``
        """
    @property
    def tint(self) -> int:
        """
        :class:`int`: The tint to set the image. Range from -100 to 100
        """
    @tint.setter
    def tint(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class CppTransparency(CppBasePixelTransform):
    """
    
    This class inherits from :class:`CppBasePixelTransform`
    
    Adjusts the transparency (alpha channel) of a pixel
        
    """
    def __init__(self, alphaChange: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Constructs a new transparency pixel transform
        
        Parameters
        ----------
        alphaChange: :class:`int`
            How much to adjust the alpha channel of the pixel. Range from -255 to 255
        
            .. note::
                The alpha channel for an image is inclusively bounded from 0 to 255
        """
    @property
    def alphaChange(self) -> int:
        """
        :class:`int`: How much to adjust the alpha channel of the pixel. Range from -255 to 255
        """
    @alphaChange.setter
    def alphaChange(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class CppTransparencyAdjustFilter(CppBaseTexFilter):
    """
    
    This class inherits from :class:`CppBaseTexFilter`
    
    Adjust the transparency (alpha channel) for an image
        
    """
    def __init__(self, alphaChange: typing.SupportsInt | typing.SupportsIndex, coloursToFilter: typing.Any = None) -> None:
        """
        Constructs a new transparency-adjust filter
        
        Parameters
        ----------
        alphaChange: :class:`int`
            How much to adjust the alpha channel of each pixel. Range from -255 to 255
        
            .. note::
                The alpha channel for an image is inclusively bounded from 0 to 255
        
        coloursToFilter: Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]
            The specific colours to have their transparency adjusted. If this value is ``None``, then will
            adjust the transparency for the entire image. **Default**: ``None``
        """
    def transform(self, texFile: typing.Any) -> None:
        """
        Adjusts the transparency across the image
        
        Parameters
        ----------
        texFile: :class:`TextureFile`
            The texture to be edited
        """
    @property
    def alphaChange(self) -> int:
        """
        :class:`int`: How much to adjust the alpha channel of each pixel. Range from -255 to 255
        """
    @alphaChange.setter
    def alphaChange(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def coloursToFilter(self) -> typing.Any:
        """
        Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]: The specific colours to have
        their transparency adjusted. If this value is ``None``, then will adjust the transparency for the
        entire image
        """
    @coloursToFilter.setter
    def coloursToFilter(self, arg1: typing.Any) -> None:
        ...
class CppTrie:
    """
    
    A class for a basic `trie`_ implemented in C++
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines if 'key' is found
    
        .. describe:: x[key]
    
            Retrieves the corresponding value to 'key'
    
        .. describe:: x[key] = val
    
            Sets the new `KVP`_
    
        .. describe:: len(x)
    
            Retrieves the number of elements
    
    Parameters
    ----------
    data: Optional[Dict[:class:`str`, T]]
        Any initial data to insert :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the keywords to put into the `trie`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    handleDuplicate: Optional[Callable[[:class:`str`, T, T], T]]
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`
    
        The function takes in the following parameters:
    
        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_
    
        If this value is ``None``, will return the value of the new `KVP`_ by default :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
            
    """
    def __contains__(self, key: str) -> bool:
        """
        Determines if 'key' is found
        """
    def __getitem__(self, key: str) -> typing.Any:
        """
        Retrieves the corresponding value to 'key'
        """
    def __init__(self, data: collections.abc.Mapping[str, typing.Any] | None = None, handleDuplicate: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any] | None = None) -> None:
        ...
    def __len__(self) -> int:
        """
        Retrieves the number of elements
        """
    def __setitem__(self, key: str, val: typing.Any) -> bool:
        """
        Sets the new `KVP`_
        """
    def add(self, keyword: str, value: typing.Any) -> bool:
        """
        Adds a new keyword
        
        Parameters
        ----------
        keyword: :class:`str`
            The keyword to add
        
        value: T
            The value associated with the keyword
        
        Returns
        -------
        :class:`bool`
            Whether the keyword has already been inserted
        """
    def build(self, data: collections.abc.Mapping[str, typing.Any] | None = None) -> None:
        """
        Rebuilds the `trie`_
        
        Parameters
        ----------
        data: Optional[Dict[:class:`str`, T]]
            Any initial data to put into the `trie`_ :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the keywords to put into the trie and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def clear(self) -> None:
        """
        Clears the data
        """
    def get(self, keyword: str, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the corresponding value to 'keyword'
        
        Parameters
        ----------
        keyword: :class:`str`
            The keyword to get the corresponding value for
        
        errorOnNotFound: :class:`bool`  
            If the keyword is not found, whether to raise an exception
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if 'keyword' is not found
        
        Raises
        ------
        :class:`KeyError`
            If 'keyword' is not found
        
        Returns
        -------
        Union[T, Any]
            Either the found value for the keyword or the value specified at 'default', if 'keyword' is not found and
            'errorOnNotFound' is set to ``False``
        """
    @property
    def handleDuplicate(self) -> collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]:
        """
        Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`
        
        The function takes in the following parameters:
        
        #. The duplicate keyword in both `KVPs`_
        #. The value of the existing `KVP`_
        #. The value of the new `KVP`_
        
        :getter: Retrieves the function
        :setter: Sets the new function
        :type: Callable[[:class:`str`, T, T], T]
        """
    @handleDuplicate.setter
    def handleDuplicate(self, arg1: collections.abc.Callable[[str, typing.Any, typing.Any], typing.Any]) -> None:
        ...
class CppVersion:
    """
    
    A single `PEP 440`_ version value -- a from-scratch C++ port of Python's `packaging.version.Version`_,
    matching its parsing/normalization/comparison behaviour exactly (verified empirically against the
    real ``packaging`` library during development, not just read off its source)
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' are the same version
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' are different versions
    
        .. describe:: x < y, x <= y, x > y, x >= y
    
            Compares two versions following `PEP 440`_'s ordering rules
    
        .. describe:: hash(x)
    
            Retrieves a hash of 'x' itself, so that 'x' can be used as a key in a :class:`dict`/:class:`set`
    
        .. describe:: str(x)
    
            Equivalent to ``x.toString()``
        
    """
    @staticmethod
    def parse(raw: str) -> FixRaidenBoss2.core.CppVersion | None:
        """
        Parses a raw version string
        
        Parameters
        ----------
        raw: :class:`str`
            The raw version string to parse
        
        Returns
        -------
        Optional[:class:`CppVersion`]
            The parsed version, or ``None`` if 'raw' does not conform to `PEP 440`_ in any way
        """
    def __eq__(self, other: CppVersion) -> bool:
        """
        Determines whether 'self' and 'other' are the same version
        """
    def __ge__(self, other: CppVersion) -> bool:
        ...
    def __gt__(self, other: CppVersion) -> bool:
        ...
    def __hash__(self) -> int:
        """
        Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set
        """
    def __le__(self, other: CppVersion) -> bool:
        ...
    def __lt__(self, other: CppVersion) -> bool:
        ...
    def __ne__(self, other: CppVersion) -> bool:
        """
        Determines whether 'self' and 'other' are different versions
        """
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def toString(self) -> str:
        """
        Converts the version back into its normalized, round-trippable string form
        
        Returns
        -------
        :class:`str`
            The string form of the version
        """
    @property
    def base_version(self) -> str:
        """
        :class:`str`: The epoch and release segment only, with no pre/post/dev/local segment
        """
    @property
    def dev(self) -> int | None:
        """
        Optional[:class:`int`]: The dev-release number, or ``None`` if there is none
        """
    @property
    def epoch(self) -> int:
        """
        :class:`int`: The epoch of the version (``0`` if none was specified)
        """
    @property
    def is_devrelease(self) -> bool:
        """
        :class:`bool`: Whether this is a dev-release
        """
    @property
    def is_postrelease(self) -> bool:
        """
        :class:`bool`: Whether this is a post-release
        """
    @property
    def is_prerelease(self) -> bool:
        """
        :class:`bool`: Whether this is a pre-release (has a pre-release or dev-release segment)
        """
    @property
    def local(self) -> str | None:
        """
        Optional[:class:`str`]: The local version segment, dot-joined, or ``None`` if there is none
        """
    @property
    def major(self) -> int:
        """
        :class:`int`: The first component of :attr:`release`, or ``0`` if unavailable
        """
    @property
    def micro(self) -> int:
        """
        :class:`int`: The third component of :attr:`release`, or ``0`` if unavailable
        """
    @property
    def minor(self) -> int:
        """
        :class:`int`: The second component of :attr:`release`, or ``0`` if unavailable
        """
    @property
    def post(self) -> int | None:
        """
        Optional[:class:`int`]: The post-release number, or ``None`` if there is none
        """
    @property
    def pre(self) -> tuple[str, int] | None:
        """
        Optional[Tuple[:class:`str`, :class:`int`]]: The pre-release segment (normalized letter and number), or ``None`` if there is none
        """
    @property
    def public(self) -> str:
        """
        :class:`str`: :meth:`toString` without the local segment
        """
    @property
    def release(self) -> list[int]:
        """
        Tuple[:class:`int`, ...]: The numeric components of the release segment, in order, including any
        trailing zeros (e.g. ``CppVersion.parse("2.0.0").release == (2, 0, 0)``)
        """
class DFA(BaseDFA):
    """
    
    Class for a `DFA (Deterministic Finite Automaton)`_
            
    """
    def __init__(self) -> None:
        ...
    def acceptLen(self) -> int:
        """
        Retrieves the number of accepting states in the `DFA`_
        
        Returns
        -------
        :class:`int`
            The number of accepting states in the `DFA`_
        """
    def addFuncTransition(self, srcId: typing.Any, func: collections.abc.Callable, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_ such that the transition is based off a predicate function
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        func: Callable[[Hashable], :class:`bool`]
            The predicate function that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            The function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addKeywordTransition(self, srcId: typing.Any, keyword: typing.Any, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_ such that the transition is based off a keyword
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keyword: `Hashable`_
            The keyword or predicate function that will trigger a transition from the source state to the destination state
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addState(self, id: typing.Any, isAccept: bool | None = None, isStart: bool = False) -> bool:
        """
        Add a new state to the `DFA`
        
        Parameters
        ----------
        id: Hashable
            The id for the state
        
        isAccept: Optional[:class:`bool`]
            Whether the state is an accepting state :raw-html:`<br />` :raw-html:`<br />`
        
            * If this value is ``None`` and the state already exists, then will not change whether the existing state is accepting or not.
            * Otherwise, if this value is ``None`` and the state does not already exists, then will not set the state as accepting. :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        isStart: :class:`bool`
            Whether to set the state as the new starting state
        
            .. warning::
                A `DFA`_ can only have 1 start state
        
            .. warning::
                If the `DFA`_ is empty and you add a new state, will set this state as the start state
        
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        :class:`bool`
            Whether the state was newly added
        """
    def addTransition(self, srcId: typing.Any, keyword: typing.Any, destId: typing.Any) -> None:
        """
        Adds a transition to the `DFA`_
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keyword: Union[`Hashable`_, Callable[[Hashable], :class:`bool`]]
            The keyword or predicate function that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            If keyword is a predicate function, the function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def addTransitions(self, srcId: typing.Any, keywords: typing.Any, destId: typing.Any) -> None:
        """
        Adds a group of transitions from one state to another state
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
            .. caution::
                The id to the source state must refer to an existing state to the `DFA`_
        
        keywords: Union[List[Union[`Hashable`_, Callable[[Hashable], :class:`bool`]]], `Hashable`_, Callable[[Hashable], :class:`bool`]]
            The keywords or predicate functions that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`
        
            For predicate functions, the function will take in a keyword as an argument
        
            .. warning::
                If the source state already has such a transition, then will overwrite the destination state for this transition
        
        destId: `Hashable`_
            The id of the destionation state for the transition
        
            .. note::
                The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
                will create a new state in the `DFA`_
        """
    def clear(self) -> None:
        """
        Clears the `DFA`_
        """
    def getKeywordToState(self, srcId: typing.Any, keyword: typing.Any) -> typing.Any | None:
        """
        Retrieves the destination state of a keyword transition from a particular state
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state for the transition
        
        keyword: `Hashable`_
            The keyword for the transition
        
        Returns
        -------
        Optional[`Hashable`_]
            The id of the destination state of the transition, or ``None`` if no such transition exists
            from 'srcId'
        """
    def getKeywordTransitions(self, id: typing.Any) -> tuple[list[typing.Any], bool]:
        """
        Retrieves all the keyword transitions connected to a particular state
        
        Parameters
        ----------
        id: `Hashable`_
            The id of the state to retrieve the keyword transitions for
        
        Returns
        -------
        Tuple[List[Hashable], :class:`bool`]
            A tuple containing:
        
            #. The keyword transitions connected to 'id'
            #. Whether 'id' corresponds to an existing state in the `DFA`_ -- if ``False``, the list is empty
        """
    def hasKeywordTransition(self, srcId: typing.Any, keyword: typing.Any) -> bool:
        """
        Determines whether a keyword transition exists from a particular state
        
        Parameters
        ----------
        srcId: `Hashable`_
            The id of the source state to check
        
        keyword: `Hashable`_
            The keyword for the transition to check
        
        Returns
        -------
        :class:`bool`
            Whether the transition exists from 'srcId'
        """
    def isAccept(self, id: typing.Any) -> bool:
        """
        Determines whether some state is an accepting state
        
        Parameters
        ----------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the corresponding state is an accepting state
        """
    def isStart(self, id: typing.Any) -> bool:
        """
        Determines whether some state is a starting state
        
        Parameters
        ----------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the corresponding state is a starting state
        """
    def reset(self) -> None:
        """
        Resets the `DFA`_ to return back to its starting state
        """
    def stateExists(self, id: typing.Any) -> bool:
        """
        Determines whether some state exists in the `DFA`_
        
        Parameters
        ----------
        id: `Hashable`_
            The id of the state
        
        Returns
        -------
        :class:`bool`
            Whether the id corresponds to a state in the `DFA`_
        """
    def stateLen(self) -> int:
        """
        Retrieves the number of states in the `DFA`_
        
        Returns
        -------
        :class:`int`
            The number of states in the `DFA`_
        """
    def transition(self, keyword: typing.Any) -> tuple[typing.Any, bool, bool]:
        """
        Transitions to a new state
        
        Parameters
        ----------
        keyword: Hashable
            The keyword to trigger the transition to the new state
        
        Returns
        -------
        Tuple[Hashable, :class:`bool`, :class:`bool`]
            Resultant data regarding the new transitioned state, which includes:
        
            #. The id of the new state
            #. Whether the new state is an accepting state
            #. Whether a transition was taken
        """
    @property
    def currentStateId(self) -> typing.Any | None:
        """
        The id of the state the `DFA`_ is currently at
        
        .. warning::
            The setter will not set the new id for the state if the newly current id does not correspond
            to any state within the `DFA`_
        
        :getter: Retrieves the id of the current state
        :setter: Sets the new id of the current state the `DFA`_ is on
        :type: Hashable
        """
    @currentStateId.setter
    def currentStateId(self, arg1: typing.Any) -> None:
        ...
    @property
    def startId(self) -> typing.Any | None:
        """
        The id to the start state
        
        .. warning::
            The setter will not set the new id for the state if the newly given start id does not correspond
            to any state within the `DFA`_
        
        :getter: Retrieves the start id
        :setter: Sets the new start id
        :type: Hashable
        """
    @startId.setter
    def startId(self, arg1: typing.Any) -> None:
        ...
class FileDownload:
    """
    
    Class to handle file downloads from some server
        
    """
    def __init__(self, url: str, filename: str, cache: bool = True) -> None:
        """
        Constructs a new file download
        
        Parameters
        ----------
        url: :class:`str`
            The link to the file download
        
        filename: :class:`str`
            The base name of the file (with extension)
        
        cache: :class:`bool`
            Whether to copy the previously-downloaded file if possible, instead of downloading another copy
        
            **Default**: ``True``
        """
    def download(self, folder: str, proxy: str | None = None) -> str:
        """
        Downloads the required file
        
        Parameters
        ----------
        folder: :class:`str`
            The folder to store the downloaded file (created if it doesn't already exist)
        
        proxy: Optional[:class:`str`]
            The link to the proxy server used for any internet network access, if any
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`str`
            The full path to the downloaded file
        """
    def get(self, folder: str, proxy: str | None = None) -> tuple[str, bool, bool]:
        """
        Retrieves the required file -- either from :meth:`download`, or (if 'cache' is ``True`` and a
        previous download already exists) by copying the previously-downloaded file instead
        
        Parameters
        ----------
        folder: :class:`str`
            The folder to store the downloaded file
        
        proxy: Optional[:class:`str`]
            The link to the proxy server used for any internet network access, if any
        
            **Default**: ``None``
        
        Returns
        -------
        Tuple[:class:`str`, :class:`bool`, :class:`bool`]
            A tuple containing, in order: the path to the downloaded file; whether a download actually
            occurred; whether a previous download to the file already existed before this call
        """
    @property
    def cache(self) -> bool:
        """
        :class:`bool`: Whether to copy the previously-downloaded file if possible, instead of downloading another copy
        """
    @cache.setter
    def cache(self, arg0: bool) -> None:
        ...
    @property
    def filename(self) -> str:
        """
        :class:`str`: The base name of the file (with extension)
        """
    @filename.setter
    def filename(self, arg0: str) -> None:
        ...
    @property
    def url(self) -> str:
        """
        :class:`str`: The link to the file download
        """
    @url.setter
    def url(self, arg0: str) -> None:
        ...
class FileStats:
    """
    
    Keeps track of different types of files encountered by the program
    
    .. note::
        'skipped'/'skippedByMods' hold real Python exception objects (any :class:`Exception` instance),
        not a C++-level exception type -- this is a Python-facing adaptation of the underlying
        AGRemapCore::FileStats (which stores these as an opaque C++ ``std::exception_ptr`` instead, for
        a pure-C++ caller); see this binding's own source comment for why
        
    """
    def __init__(self) -> None:
        ...
    def addFixed(self, filePath: str) -> None:
        """
        Adds a file path to the paths of fixed files
        
        Parameters
        ----------
        filePath: :class:`str`
            the new file path to a fixed file
        """
    def addRemoved(self, filePath: str) -> None:
        """
        Adds a new file path that got removed
        
        Parameters
        ----------
        filePath: :class:`str`
            The file path that got removed
        """
    def addSkipped(self, filePath: str, error: typing.Any, modFolder: str | None = None) -> None:
        """
        Adds a new file path to the paths of skipped files
        
        Parameters
        ----------
        filePath: :class:`str`
            the new file path that got skipped
        
        error: :class:`Exception`
            The exception that caused the file to be skipped
        
        modFolder: Optional[:class:`str`]
            The mod folder that contains the file path. If this is ``None``, the mod folder is read from
            'filePath''s own parent directory
        
            **Default**: ``None``
        """
    def addUndoed(self, filePath: str) -> None:
        """
        Adds a new file path that got undone
        
        Parameters
        ----------
        filePath: :class:`str`
            The file path that got undone
        """
    def addVisitedAtRemoval(self, filePath: str) -> None:
        """
        Adds a new file path that got visited when the software attempts to remove the file
        
        Parameters
        ----------
        filePath: :class:`str`
            The file path that got visited
        """
    def clear(self) -> None:
        """
        Clears out all saved data about the files
        """
    def update(self, modFolder: str | None = None, newFixed: collections.abc.Set[str] | None = None, newSkipped: collections.abc.Mapping[str, typing.Any] | None = None, newRemoved: collections.abc.Set[str] | None = None, newUndoed: collections.abc.Set[str] | None = None, newVisitedAtRemoval: collections.abc.Set[str] | None = None) -> None:
        """
        Updates the overall file paths in this class -- see :meth:`updateFixed`, :meth:`updateSkipped`, and
        :meth:`updateRemoved` for more details
        
        Parameters
        ----------
        modFolder: Optional[:class:`str`]
            The folder where the files got skipped
        
            **Default**: ``None``
        
        newFixed: Optional[Set[:class:`str`]]
            The newly updated file paths that got fixed
        
            **Default**: ``None``
        
        newSkipped: Optional[Dict[:class:`str`, :class:`Exception`]]
            The newly skipped file paths due to errors within a particular mod folder
        
            **Default**: ``None``
        
        newRemoved: Optional[Set[:class:`str`]]
            The newly updated file paths that got removed
        
            **Default**: ``None``
        
        newUndoed: Optional[Set[:class:`str`]]
            The newly updated file paths that got their contents undone
        
            **Default**: ``None``
        
        newVisitedAtRemoval: Optional[Set[:class:`str`]]
            The newly updated file paths that got visited when the software attempts to remove those files
        
            **Default**: ``None``
        """
    def updateFixed(self, newFixed: collections.abc.Set[str]) -> None:
        """
        Updates the fixed file paths
        
        Parameters
        ----------
        newFixed: Set[:class:`str`]
            The newly added file paths that got fixed
        """
    def updateRemoved(self, newRemoved: collections.abc.Set[str]) -> None:
        """
        Updates the file paths that got removed
        
        Parameters
        ----------
        newRemoved: Set[:class:`str`]
            The newly updated file paths that got removed
        """
    def updateSkipped(self, newSkipped: collections.abc.Mapping[str, typing.Any], modFolder: str | None = None) -> None:
        """
        Updates the file paths that got skipped due to errors
        
        Parameters
        ----------
        newSkipped: Dict[:class:`str`, :class:`Exception`]
            The newly skipped file paths (and their errors), due to errors within a particular mod folder
        
        modFolder: Optional[:class:`str`]
            The folder where the files got skipped. If this is ``None``, the mod folder for each entry in
            'newSkipped' is instead read from that entry's own file path
        
            **Default**: ``None``
        """
    def updateUndoed(self, newUndoed: collections.abc.Set[str]) -> None:
        """
        Updates the file paths whose contents got undone to a previous state before the software was run
        
        Parameters
        ----------
        newUndoed: Set[:class:`str`]
            The newly updated file paths that got their contents undone
        """
    def updateVisitedAtRemoval(self, newVisitedAtRemoval: collections.abc.Set[str]) -> None:
        """
        Updates the file paths that got visited when the software attempts to remove those files
        
        Parameters
        ----------
        newVisitedAtRemoval: Set[:class:`str`]
            The newly updated file paths that got visited
        """
    @property
    def fixed(self) -> set[str]:
        """
        Set[:class:`str`]: The paths to the fixed files
        """
    @fixed.setter
    def fixed(self, arg0: collections.abc.Set[str]) -> None:
        ...
    @property
    def removed(self) -> set[str]:
        """
        Set[:class:`str`]: The file paths for files that got removed
        """
    @removed.setter
    def removed(self, arg0: collections.abc.Set[str]) -> None:
        ...
    @property
    def skipped(self) -> dict[str, typing.Any]:
        """
        Dict[:class:`str`, :class:`Exception`]: The exceptions tied to file paths that were skipped due to errors
        """
    @skipped.setter
    def skipped(self, arg0: collections.abc.Mapping[str, typing.Any]) -> None:
        ...
    @property
    def skippedByMods(self) -> dict[str, dict[str, typing.Any]]:
        """
        Dict[:class:`str`, Dict[:class:`str`, :class:`Exception`]]: The exceptions tied to file paths that were skipped due to errors, grouped by mod folder path
        """
    @skippedByMods.setter
    def skippedByMods(self, arg0: collections.abc.Mapping[str, collections.abc.Mapping[str, typing.Any]]) -> None:
        ...
    @property
    def undoed(self) -> set[str]:
        """
        Set[:class:`str`]: The file paths for files that got undone to a previous state before the software was run
        """
    @undoed.setter
    def undoed(self, arg0: collections.abc.Set[str]) -> None:
        ...
    @property
    def visitedAtRemoval(self) -> set[str]:
        """
        Set[:class:`str`]: The file paths for files that got visited when attempting to remove those files
        """
    @visitedAtRemoval.setter
    def visitedAtRemoval(self, arg0: collections.abc.Set[str]) -> None:
        ...
class FilteredTokenizer(BaseTokenizer):
    """
    
    This class inherits from :class:`BaseTokenizer`
    
    A tokenizer that still accepts all tokens, but does not include certain tokens into the tokenized result
    
    Parameters
    ----------
    tokens: Dict[:class:`str`, :class:`str`]
        The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
    
    keywordTokenIds: Set[:class:`str`]
        The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names
    
    filteredTokenIds: Set[:class:`str`]
        The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result
    
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, tokens: collections.abc.Mapping[str, str], keywordTokenIds: collections.abc.Set[str], filteredTokenIds: collections.abc.Set[str], setup: bool = True) -> None:
        ...
    @property
    def filteredTokenIds(self) -> set[str]:
        """
        Set[:class:`str`]: The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result
        """
    @property
    def keywordTokenIds(self) -> set[str]:
        """
        Set[:class:`str`]: The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names
        """
class GIBuilder:
    """
    
    Creates new :class:`ModType` objects for GI (Genshin Impact) mods
    
    Mirrors the pure-Python :class:`GIBuilder` class, but builds the lighter, C++-side
    :class:`ModType` (id, name, and aliases only) instead of the full pure-Python :class:`ModType`
        
    """
    @staticmethod
    def amber() -> ModType:
        """
        Creates the :class:`ModType` for Amber
        """
    @staticmethod
    def amberCN() -> ModType:
        """
        Creates the :class:`ModType` for AmberCN
        """
    @staticmethod
    def arlecchino() -> ModType:
        """
        Creates the :class:`ModType` for Arlecchino
        """
    @staticmethod
    def ayaka() -> ModType:
        """
        Creates the :class:`ModType` for Ayaka
        """
    @staticmethod
    def ayakaSpringBloom() -> ModType:
        """
        Creates the :class:`ModType` for AyakaSpringBloom
        """
    @staticmethod
    def barbara() -> ModType:
        """
        Creates the :class:`ModType` for Barbara
        """
    @staticmethod
    def barbaraSummerTime() -> ModType:
        """
        Creates the :class:`ModType` for BarbaraSummerTime
        """
    @staticmethod
    def cherryHutao() -> ModType:
        """
        Creates the :class:`ModType` for CherryHuTao
        """
    @staticmethod
    def diluc() -> ModType:
        """
        Creates the :class:`ModType` for Diluc
        """
    @staticmethod
    def dilucFlamme() -> ModType:
        """
        Creates the :class:`ModType` for DilucFlamme
        """
    @staticmethod
    def fischl() -> ModType:
        """
        Creates the :class:`ModType` for Fischl
        """
    @staticmethod
    def fischlHighness() -> ModType:
        """
        Creates the :class:`ModType` for FischlHighness
        """
    @staticmethod
    def ganyu() -> ModType:
        """
        Creates the :class:`ModType` for Ganyu
        """
    @staticmethod
    def ganyuTwilight() -> ModType:
        """
        Creates the :class:`ModType` for GanyuTwilight
        """
    @staticmethod
    def huTao() -> ModType:
        """
        Creates the :class:`ModType` for HuTao
        """
    @staticmethod
    def jean() -> ModType:
        """
        Creates the :class:`ModType` for Jean
        """
    @staticmethod
    def jeanCN() -> ModType:
        """
        Creates the :class:`ModType` for JeanCN
        """
    @staticmethod
    def jeanSea() -> ModType:
        """
        Creates the :class:`ModType` for JeanSea
        """
    @staticmethod
    def kaeya() -> ModType:
        """
        Creates the :class:`ModType` for Kaeya
        """
    @staticmethod
    def kaeyaSailwind() -> ModType:
        """
        Creates the :class:`ModType` for KaeyaSailwind
        """
    @staticmethod
    def keqing() -> ModType:
        """
        Creates the :class:`ModType` for Keqing
        """
    @staticmethod
    def keqingOpulent() -> ModType:
        """
        Creates the :class:`ModType` for KeqingOpulent
        """
    @staticmethod
    def kirara() -> ModType:
        """
        Creates the :class:`ModType` for Kirara
        """
    @staticmethod
    def kiraraBoots() -> ModType:
        """
        Creates the :class:`ModType` for KiraraBoots
        """
    @staticmethod
    def klee() -> ModType:
        """
        Creates the :class:`ModType` for Klee
        """
    @staticmethod
    def kleeBlossomingStarlight() -> ModType:
        """
        Creates the :class:`ModType` for KleeBlossomingStarlight
        """
    @staticmethod
    def lisa() -> ModType:
        """
        Creates the :class:`ModType` for Lisa
        """
    @staticmethod
    def lisaStudent() -> ModType:
        """
        Creates the :class:`ModType` for LisaStudent
        """
    @staticmethod
    def mona() -> ModType:
        """
        Creates the :class:`ModType` for Mona
        """
    @staticmethod
    def monaCN() -> ModType:
        """
        Creates the :class:`ModType` for MonaCN
        """
    @staticmethod
    def nilou() -> ModType:
        """
        Creates the :class:`ModType` for Nilou
        """
    @staticmethod
    def nilouBreeze() -> ModType:
        """
        Creates the :class:`ModType` for NilouBreeze
        """
    @staticmethod
    def ningguang() -> ModType:
        """
        Creates the :class:`ModType` for Ningguang
        """
    @staticmethod
    def ningguangOrchid() -> ModType:
        """
        Creates the :class:`ModType` for Ningguang
        """
    @staticmethod
    def raiden() -> ModType:
        """
        Creates the :class:`ModType` for Ei
        """
    @staticmethod
    def rosaria() -> ModType:
        """
        Creates the :class:`ModType` for Rosaria
        """
    @staticmethod
    def rosariaCN() -> ModType:
        """
        Creates the :class:`ModType` for RosariaCN
        """
    @staticmethod
    def shenhe() -> ModType:
        """
        Creates the :class:`ModType` for Shenhe
        """
    @staticmethod
    def shenheFrostFlower() -> ModType:
        """
        Creates the :class:`ModType` for ShenheFrostFlower
        """
    @staticmethod
    def xiangling() -> ModType:
        """
        Creates the :class:`ModType` for Xiangling
        """
    @staticmethod
    def xianglingCheer() -> ModType:
        """
        Creates the :class:`ModType` for XianglingCheer
        """
    @staticmethod
    def xingqiu() -> ModType:
        """
        Creates the :class:`ModType` for Xingqiu
        """
    @staticmethod
    def xingqiuBamboo() -> ModType:
        """
        Creates the :class:`ModType` for XingqiuBamboo
        """
class GIMIFixer(BaseIniFixer):
    """
    
    This class inherits from :class:`BaseIniFixer`
    
    Fixes a .ini file used by a ``GIMI``-style importer
    
    Parameters
    ----------
    parser: :class:`GIMIParser`
        The associated parser to retrieve data for the fix
    
    graphGroupEdits: Optional[List[:class:`BaseIniGraphGroupEdit`]]
        The edits to apply to the parsed caller/callee graphs, run in order, once for each mod being
        fixed to :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    modsToFix: Optional[List[:class:`str`]]
        The names of the mods to fix to :raw-html:`<br />` :raw-html:`<br />`
    
        If this argument is ``None``, will ask the .ini file's own :class:`ModType` instead -- see
        :meth:`getModsToFix` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    prevFixer: Optional[:class:`GIMIFixer`]
        A fixer whose already-edited graph groups this one continues from instead of starting fresh
        from the parser :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, parser: typing.Any, graphGroupEdits: typing.Any = None, modsToFix: typing.Any = None, prevFixer: typing.Any = None) -> None:
        ...
    def clear(self) -> None:
        """
        Resets any saved states within the fixer
        """
    def fix(self, keepBackup: bool = True, fixOnly: bool = False, hideOrig: bool = False, context: typing.Any = None) -> typing.Any:
        """
        Fixes the .ini file
        
        Parameters
        ----------
        keepBackup: :class:`bool`
            Whether to keep backups for the .ini file. Ignored when ``context.isFirstModType`` is ``False``,
            since several fixers chain over one .ini file and only the first of them should move it aside :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        fixOnly: :class:`bool`
            Whether to only fix the .ini file without undoing any fixes :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        hideOrig: :class:`bool`
            Whether to hide the mod for the original character. Ignored when ``context.isLastModType`` is ``False``, since
            several fixers chain over one .ini file and only the last of them should rewrite it :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        context: Optional[:class:`IniFixingContext`]
            The per-call options for this fix. If ``None``, a default one is built, which says this
            fixer is the .ini file's last :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        Dict[Union[:class:`str`, :class:`int`], :class:`str`]
            The new content of the fixed .ini file(s) :raw-html:`<br />` :raw-html:`<br />`
        
            * The keys are the file paths each fixed .ini file was written to. A .ini file with no path at
              all is keyed by its group's index instead, and nothing is written for it
            * The values are that file's new content, including the original content and the boilerplate
        """
    def getFix(self, onlyEditObjGraphs: bool = False) -> typing.Any:
        """
        Retrieves only the content of the fix, without writing anything
        
        Parameters
        ----------
        onlyEditObjGraphs: :class:`bool`
            Whether to only run :attr:`graphGroupEdits` :raw-html:`<br />` :raw-html:`<br />`
        
            If this value is ``True``, returns nothing and the results are left on :attr:`graphGroups`
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        Optional[Dict[Union[:class:`str`, :class:`int`], :class:`IniGraphGroup`]]
            The content of the fix :raw-html:`<br />` :raw-html:`<br />`
        
            * The keys are the file paths each group should be written to. A .ini file with no path at all
              is keyed by its group's index instead
            * The values are the edited graph groups themselves
        """
    def getModsToFix(self) -> list[str]:
        """
        Retrieves the mods to fix to
        
        Returns
        -------
        List[:class:`str`]
            :attr:`modsToFix` when it was set explicitly, otherwise whatever the .ini file's own
            :class:`ModType` says -- empty when the .ini file was never classified
        """
    def groupToStr(self, groupInd: typing.SupportsInt | typing.SupportsIndex) -> str:
        """
        Renders one of :attr:`graphGroups` to .ini text -- every graph in it, joined by blank lines
        
        Parameters
        ----------
        groupInd: :class:`int`
            Which group to render
        
        Returns
        -------
        :class:`str`
            The rendered .ini text
        """
    @property
    def _iniFile(self) -> typing.Any:
        """
        :class:`IniFile`: The .ini file that will be fixed
        """
    @_iniFile.setter
    def _iniFile(self, arg0: typing.Any) -> None:
        ...
    @property
    def _parser(self) -> typing.Any:
        """
        :class:`BaseIniParser`: The associated parser to retrieve data for the fix
        """
    @_parser.setter
    def _parser(self, arg0: typing.Any) -> None:
        ...
    @property
    def graphGroupEdits(self) -> typing.Any:
        """
        List[:class:`BaseIniGraphGroupEdit`]: The edits to apply to the parsed caller/callee graphs
        """
    @graphGroupEdits.setter
    def graphGroupEdits(self, arg0: typing.Any) -> None:
        ...
    @property
    def graphGroups(self) -> typing.Any:
        """
        List[:class:`IniGraphGroup`]: The graph groups this fixer edited, one per .ini file the fix
        produces -- empty until :meth:`getFix` or :meth:`fix` has run
        """
    @graphGroups.setter
    def graphGroups(self, arg1: typing.Any) -> None:
        ...
    @property
    def modsToFix(self) -> typing.Any:
        """
        Optional[List[:class:`str`]]: The names of the mods to fix to, or ``None`` to ask the .ini file
        """
    @modsToFix.setter
    def modsToFix(self, arg0: typing.Any) -> None:
        ...
    @property
    def prevFixer(self) -> typing.Any:
        """
        Optional[:class:`GIMIFixer`]: A fixer whose already-edited graph groups this one continues from
        """
    @prevFixer.setter
    def prevFixer(self, arg0: typing.Any) -> None:
        ...
class GIMIParser(BaseIniParser):
    """
    
    This class inherits from :class:`BaseIniParser`
    
    Parses a .ini file used by a ``GIMI``-style importer
    
    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to parse
    
    modObjs: Optional[Set[Tuple[:class:`str`, :class:`str`]]]
        The mod objects to parse :raw-html:`<br />` :raw-html:`<br />`
    
        Each tuple contains:
    
        #. The name of the component
        #. The name of the object within the component
    
        .. tip::
            You can also interpret mod objects as the suffix part ending of some ``TextureOverride``
            `section`_ :raw-html:`<br />` :raw-html:`<br />`
    
            eg.
    
            ``[TextureOverrideHuTaoBody]`` --> ``("", "Body")``
            ``[TextureOverrideYelanBangB]`` --> ``("Bang", "B")``
            ``[TextureOverrideTexture16]`` --> ``("", "Texture16")``
    
        .. note::
            Iteration order matters -- it decides the order the command graphs are built in -- so an
            ``OrderedSet`` (or a plain ``list``) is preferred over a bare ``set`` here
    
        **Default**: ``None``
    
    objTargetFuncs: Optional[List[Callable[[:class:`GIMIParser`, :class:`str`, :class:`IfTemplate`, :class:`bool`, Optional[:class:`IfContentPart`], Optional[:class:`IfContentPartColouring`]], List[Tuple[:class:`str`, :class:`str`]]]]]
        A list of custom functions to define how to retrieve the root `sections`_ of the mod objects
        :raw-html:`<br />` :raw-html:`<br />`
    
        Each function takes in:
    
        #. This parser
        #. The name of the `section`_ to parse
        #. The content of the `section`_ to parse
        #. Whether to only return 1 result
        #. The :class:`IfContentPart` that is being parsed. Only available if :attr:`trackKeys` is ``True``
        #. The `KVPs`_ to track. Only available if :attr:`trackKeys` is ``True``
    
        and returns the corresponding mod objects the `section`_ belongs to, or ``None`` if it belongs
        to none. :raw-html:`<br />` :raw-html:`<br />`
    
        If this argument is ``None``, will use :meth:`classifyByTextureOverrideName` (or a default
        :class:`GIMISectionClassifier`, when the .ini file was classified and :attr:`trackKeys` is on)
        :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    downloads: Optional[Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`DownloadData`]]]
        The files to download if the mod is missing some required files :raw-html:`<br />` :raw-html:`<br />`
    
        * The outer keys are tuples that contain the name of the component and the mod object
        * The inner keys are the names of the registers
    
        .. note::
            The :attr:`DownloadData.name` for each :class:`DownloadData` should be unique
    
        **Default**: ``None``
    
    commandEdits: Optional[:class:`GraphGroupEdit`]
        Any further edits to the parsed caller/callee graphs for ``TextureOverride`` related command
        `sections`_ :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    makeGlobalGraph: :class:`bool`
        Whether to make the graph for the entire .ini file :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
    
    disjointModObjs: :class:`bool`
        Whether the sets of `sections`_ for each mod object should be disjoint or not :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
    
    trackKeys: :class:`bool`
        Whether to track the `KVPs`_ in the .ini file :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
    
    keysToTrack: Optional[Set[:class:`str`]]
        Specific keys to track in the .ini file. If ``None``, keeps track of every key encountered
        :raw-html:`<br />` :raw-html:`<br />`
    
        .. note::
            Only takes effect if 'trackKeys' and 'makeGlobalGraph' are both ``True``
    
        **Default**: ``None``
        
    """
    @staticmethod
    def classifyByTextureOverrideName(parser: GIMIParser, sectionName: str, disjoint: bool = True, modObjs: typing.Any = None, fromRoots: bool = True) -> list:
        """
        Classify the ``TextureOverride`` `sections`_ to the specified mod objects
        
        Parameters
        ----------
        parser: :class:`GIMIParser`
            The parser used for the classification
        
        sectionName: :class:`str`
            The name of the `section`_ to classify
        
        disjoint: :class:`bool`
            Whether to classify the `section`_ to only 1 mod object or allow classification to multiple mod
            objects :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        modObjs: Optional[Set[Tuple[:class:`str`, :class:`str`]]]
            The mod objects for classification. If ``None``, uses the mod objects at :attr:`modObjs` for
            the argument, 'parser' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        fromRoots: :class:`bool`
            Whether to make sure 'parser''s :attr:`globalGraph` has been built first :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        Returns
        -------
        List[Tuple[:class:`str`, :class:`str`]]
            The mod objects the `section`_ has been classified to
        """
    def __init__(self, iniFile: typing.Any, modObjs: typing.Any = None, objTargetFuncs: typing.Any = None, downloads: typing.Any = None, commandEdits: typing.Any = None, makeGlobalGraph: bool = True, disjointModObjs: bool = True, trackKeys: bool = True, keysToTrack: typing.Any = None, modTypeId: typing.SupportsInt | typing.SupportsIndex | None = None) -> None:
        ...
    def _getSectionTargets(self) -> None:
        """
        Retrieves the "entry points" names of the ``TextureOverride`` `sections`_ for each mod object
        specified at :attr:`modObjs`
        """
    def addDownloads(self, partsNeedDownload: dict) -> None:
        """
        Adds the required download resources to the corresponding `sections`_ and their parts
        
        Parameters
        ----------
        partsNeedDownload: Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, Union[Set[:class:`IfContentPart`], Set[:class:`IfTemplate`]]]]
            What :meth:`getDownloads` returned
        """
    def buildGlobalGraph(self) -> typing.Any:
        """
        Builds the graph for the entire .ini file
        
        Returns
        -------
        :class:`IniSectionGraph`
            The built graph
        """
    def clear(self) -> None:
        """
        Clears any saved data
        """
    def collectParseResult(self) -> typing.Any:
        """
        Collects whatever the *last* :meth:`parse` produced into the same ``[IniGraphGroup]`` that method
        returns, without parsing again
        
        .. note::
            :class:`GIMIFixer` needs exactly this: by the time a fixer runs, :meth:`IniFile.parse` has
            already driven the parser, so re-parsing would synthesize every download resource a second time
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            A one-element list -- see :meth:`parse` for the group's exact shape
        """
    def editCommands(self) -> None:
        """
        Edits the caller/callee graphs for ``TextureOverride`` related command `sections`_
        """
    def getDownloads(self) -> dict:
        """
        Retrieves the particular parts of `sections`_ that require a file download
        
        Returns
        -------
        Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, Union[Set[:class:`IfContentPart`], Set[:class:`IfTemplate`]]]]
            The parts or `sections`_ needing each register's download
        """
    def parse(self) -> typing.Any:
        """
        Parses the .ini file
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            A one-element list holding every graph this parse produced :raw-html:`<br />` :raw-html:`<br />`
        
            The group's ``graphs`` dict holds, in this order:
        
            #. every graph in :attr:`commandGraphs`, under its own ``(component, mod object)`` key
            #. every graph in :attr:`downloadResourceGraphs`, under
               ``("download", <the download's name>)``. A download whose name is already in the group is
               skipped, so one resource shared by several registers appears once
        
            :raw-html:`<br />`
        
            .. note::
                The graphs are the parser's own live objects, not copies, and the ``dict`` is a fresh one
                -- so adding to the returned group does not touch :attr:`commandGraphs`, but editing a
                graph in it does edit the parser's. Call :meth:`IniSectionGraph.deepcopy` on whichever
                graphs you need to keep independent, exactly as :class:`GIMIFixer` does
        """
    def parseCommands(self) -> None:
        """
        Parses particular command `sections`_ within the mod, specified from :attr:`modObjs`
        """
    def removeAddedIfTemplates(self) -> None:
        """
        Removes the newly added :class:`IfTemplate`\\s generated by this parser or its associated
        :class:`BaseIniFixer`\\s from :attr:`IniFile.sectionIfTemplates`
        """
    def setupDownloads(self) -> None:
        """
        Setup the required downloads resources, if not already setup
        """
    @property
    def _iniFile(self) -> typing.Any:
        """
        :class:`IniFile`: The .ini file that will be parsed
        """
    @_iniFile.setter
    def _iniFile(self, arg0: typing.Any) -> None:
        ...
    @property
    def _modsToFix(self) -> typing.Any:
        """
        Set[:class:`str`]: The names of the mods that will be fixed to
        """
    @_modsToFix.setter
    def _modsToFix(self, arg0: typing.Any) -> None:
        ...
    @property
    def _sectionTargets(self) -> dict:
        """
        Dict[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]: The names of the `sections`_ used as
        the "entry point" to a particular group of `sections`_ in the ``TextureOverride`` `section`_
        caller/callee `graph`_
        
        .. warning::
            These `sections`_ are not necessarily the roots of the graph (they may instead be a child to
            some other `section`_)
        """
    @property
    def commandEdits(self) -> typing.Any:
        """
        Optional[:class:`GraphGroupEdit`]: Any further edits to the parsed caller/callee graphs for ``TextureOverride`` related command `sections`_
        """
    @commandEdits.setter
    def commandEdits(self, arg0: typing.Any) -> None:
        ...
    @property
    def commandGraphs(self) -> typing.Any:
        """
        Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]: The caller/callee graphs for
        ``TextureOverride`` related command `sections`_ :raw-html:`<br />` :raw-html:`<br />`
        
        .. note::
            This is the *same* dict object every time -- it is group ``0`` of the parser's own
            ``List[IniGraphGroup]``, which is what makes :meth:`editCommands`' in-place editing work
        """
    @commandGraphs.setter
    def commandGraphs(self, arg1: typing.Any) -> None:
        ...
    @property
    def components(self) -> set:
        """
        Set[:class:`str`]: The different components to parse
        """
    @property
    def disjointModObjs(self) -> bool:
        """
        :class:`bool`: Whether the sets of `sections`_ for each mod object should be disjoint or not
        """
    @disjointModObjs.setter
    def disjointModObjs(self, arg0: bool) -> None:
        ...
    @property
    def downloadResourceGraphs(self) -> dict:
        """
        Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`IniSectionGraph`]]: The
        caller/callee graphs for `sections`_ related to download resources :raw-html:`<br />` :raw-html:`<br />`
        
        .. note::
            A fresh dict is built on each access (the graphs inside are the same objects every time)
        """
    @property
    def downloads(self) -> typing.Any:
        """
        Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`DownloadData`]]: The files to download if the mod is missing some required files
        """
    @downloads.setter
    def downloads(self, arg0: typing.Any) -> None:
        ...
    @property
    def globalGraph(self) -> typing.Any:
        """
        Optional[:class:`IniSectionGraph`]: The graph for the entire .ini file
        """
    @property
    def keysToTrack(self) -> typing.Any:
        """
        Optional[Set[:class:`str`]]: Specific keys to track in the .ini file
        """
    @keysToTrack.setter
    def keysToTrack(self, arg1: typing.Any) -> None:
        ...
    @property
    def makeGlobalGraph(self) -> bool:
        """
        :class:`bool`: Whether to make the graph for the entire .ini file
        """
    @makeGlobalGraph.setter
    def makeGlobalGraph(self, arg0: bool) -> None:
        ...
    @property
    def modObjs(self) -> typing.Any:
        """
        Set[Tuple[:class:`str`, :class:`str`]]: The different mod objects to parse
        """
    @modObjs.setter
    def modObjs(self, arg1: typing.Any) -> None:
        ...
    @property
    def modTypeId(self) -> int | None:
        """
        Optional[:class:`int`]: The :class:`ModTypeId` value this parser was built for
        
        A .ini file can classify as several mod types; this is the one this parser reads assets for. When
        ``None``, the parser falls back to the .ini file's :attr:`IniFile.availableType` -- the old
        behaviour, and the only thing available for a parser built by hand rather than through a builder
        """
    @modTypeId.setter
    def modTypeId(self, arg1: typing.SupportsInt | typing.SupportsIndex | None) -> None:
        ...
    @property
    def objTargetFuncs(self) -> typing.Any:
        """
        List[Callable]: The custom functions defining how to retrieve the root `sections`_ of the mod objects
        """
    @objTargetFuncs.setter
    def objTargetFuncs(self, arg0: typing.Any) -> None:
        ...
    @property
    def tempKwargs(self) -> dict:
        """
        Dict[:class:`str`, Any]: Temporary user-defined keyword variables for the user to use. Only cleared by :meth:`clear`
        """
    @tempKwargs.setter
    def tempKwargs(self, arg0: dict) -> None:
        ...
    @property
    def trackKeys(self) -> bool:
        """
        :class:`bool`: Whether to track the `KVPs`_ in the .ini file
        """
    @trackKeys.setter
    def trackKeys(self, arg0: bool) -> None:
        ...
class GIMISectionClassifier:
    """
    
    A callable class used to classify `sections`_ based on their ``hash`` value and their
    ``match_first_index`` value
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x(parser, sectionName, section, disjoint, part, kvps)
    
            Classifies the mod objects based on the current :class:`IfContentPart`. For more details on
            the arguments to pass, see :attr:`GIMIParser.objTargetFuncs`
    
    Parameters
    ----------
    hashKeyOnlyToModObj: Dict[:class:`str`, Tuple[:class:`str`, :class:`str`]]
        Mapping for mod objects that are only identified by ``hash`` value :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the names for the type of hashes (most inner keys at :attr:`ModData.Hashes`) and
        the values are tuples that contain the corresponding component and mod object to classify the
        `section`_
    
    hashes: :class:`Hashes`
        The assets for the ``hashes``
    
    indexKeyToModObj: Optional[Dict[:class:`str`, Dict[Tuple[:class:`str`, :class:`str`], Tuple[:class:`str`, :class:`str`]]]]
        Mapping for mod objects that are identified by both ``hash`` value and their
        ``match_first_index`` value :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    indices: Optional[:class:`Indices`]
        The assets for the ``match_first_index`` values :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    version: Optional[Union[:class:`str`, :class:`float`, :class:`CppVersion`]]
        The version of the .ini file. If ``None``, assumes the latest version :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    hashNonVersionVals: Optional[Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]]
        The filter values used when searching :attr:`hashes` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``UnHashableNone``
    
    indexNonVersionVals: Optional[Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]]
        The filter values used when searching :attr:`indices` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``UnHashableNone``
        
    """
    @staticmethod
    def buildDefaultClassifier(modType: typing.Any, version: typing.Any = None) -> GIMISectionClassifier:
        """
        Builds the default classifier for the `sections`_
        
        :raw-html:`<br />`
        
        The classifier comes pre-filled with the mod object mappings every mod type shares -- one entry per
        hash type the hash data table ships, named by that hash type's own key (``blend_vb`` becomes
        ``("", "blend_vb")``, and a ``compName;objName`` key becomes ``(compName, objName)``). The
        ``ib``-suffixed hash types land in :attr:`GIMISectionClassifier.indexKeyToModObj` instead, since a
        ``hash`` value alone can't tell those apart; every other one lands in
        :attr:`GIMISectionClassifier.hashKeyOnlyToModObj` :raw-html:`<br />` :raw-html:`<br />`
        
        The mappings are this classifier's own :class:`dict`\\s -- editing them to add whatever else a
        particular mod type needs won't write through to any other classifier
        
        Parameters
        ----------
        modType: :class:`ModType`
            The type of mod
        
        version: Optional[Union[:class:`str`, :class:`float`, :class:`CppVersion`]]
            The version of the .ini file. If ``None``, assumes the latest version :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`GIMISectionClassifier`
            The built classifier
        """
    @staticmethod
    def buildDefaultClassifierFromIni(ini: typing.Any, modTypeId: typing.SupportsInt | typing.SupportsIndex | None = None) -> GIMISectionClassifier:
        """
        Builds the default classifier for the `sections`_ from a .ini file
        
        :raw-html:`<br />`
        
        The classifier comes pre-filled with the mod object mappings every mod type shares -- one entry per
        hash type the hash data table ships, named by that hash type's own key (``blend_vb`` becomes
        ``("", "blend_vb")``, and a ``compName;objName`` key becomes ``(compName, objName)``). The
        ``ib``-suffixed hash types land in :attr:`GIMISectionClassifier.indexKeyToModObj` instead, since a
        ``hash`` value alone can't tell those apart; every other one lands in
        :attr:`GIMISectionClassifier.hashKeyOnlyToModObj` :raw-html:`<br />` :raw-html:`<br />`
        
        The mappings are this classifier's own :class:`dict`\\s -- editing them to add whatever else a
        particular mod type needs won't write through to any other classifier
        
        Parameters
        ----------
        ini: :class:`IniFile`
            The .ini file
        
        Returns
        -------
        :class:`GIMISectionClassifier`
            The built classifier
        """
    def __call__(self, parser: typing.Any, sectionName: str, section: typing.Any, disjoint: bool, part: typing.Any, kvps: typing.Any) -> list:
        ...
    def __init__(self, hashKeyOnlyToModObj: typing.Any, hashes: typing.Any, indexKeyToModObj: typing.Any = None, indices: typing.Any = None, version: typing.Any = None, hashNonVersionVals: typing.Any = None, indexNonVersionVals: typing.Any = None) -> None:
        ...
    def classify(self, sectionName: str, section: typing.Any, partKeys: typing.Any) -> list:
        """
        Classifies which mod objects a particular :class:`IfContentPart` belongs to
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_ where the part belongs in
        
        section: :class:`IfTemplate`
            The `section`_ where the part belongs in
        
        partKeys: :class:`IfContentPartColouring`
            The current state of the `KVPs`_ for the part
        
        Returns
        -------
        List[Tuple[:class:`str`, :class:`str`]]
            The classified mod objects
        """
    @property
    def hashKeyOnlyToModObj(self) -> typing.Any:
        """
        Dict[:class:`str`, Tuple[:class:`str`, :class:`str`]]: Mapping for mod objects that are only identified by ``hash`` value
        """
    @hashKeyOnlyToModObj.setter
    def hashKeyOnlyToModObj(self, arg0: typing.Any) -> None:
        ...
    @property
    def hashNonVersionVals(self) -> typing.Any:
        """
        The filter values used when searching :attr:`hashes`
        """
    @hashNonVersionVals.setter
    def hashNonVersionVals(self, arg0: typing.Any) -> None:
        ...
    @property
    def hashes(self) -> typing.Any:
        """
        :class:`Hashes`: The assets for the ``hash`` values
        """
    @hashes.setter
    def hashes(self, arg0: typing.Any) -> None:
        ...
    @property
    def indexKeyToModObj(self) -> typing.Any:
        """
        Dict[:class:`str`, Dict[Tuple[:class:`str`, :class:`str`], Tuple[:class:`str`, :class:`str`]]]: Mapping for mod objects that are identified by both ``hash`` value and their ``match_first_index`` value
        """
    @indexKeyToModObj.setter
    def indexKeyToModObj(self, arg0: typing.Any) -> None:
        ...
    @property
    def indexNonVersionVals(self) -> typing.Any:
        """
        The filter values used when searching :attr:`indices`
        """
    @indexNonVersionVals.setter
    def indexNonVersionVals(self, arg0: typing.Any) -> None:
        ...
    @property
    def indices(self) -> typing.Any:
        """
        Optional[:class:`Indices`]: The assets for the ``match_first_index`` values
        """
    @indices.setter
    def indices(self, arg0: typing.Any) -> None:
        ...
    @property
    def version(self) -> typing.Any:
        """
        Optional[Union[:class:`str`, :class:`float`, :class:`CppVersion`]]: The version of the .ini file
        """
    @version.setter
    def version(self, arg0: typing.Any) -> None:
        ...
class GameTypeId:
    """
    
    The names of the different supported games
        
    
    Members:
    
      GI : Genshin Impact
    
      WuWa : Wuthering Waves
    """
    GI: typing.ClassVar[GameTypeId]  # value = <GameTypeId.GI: 0>
    WuWa: typing.ClassVar[GameTypeId]  # value = <GameTypeId.WuWa: 1>
    __members__: typing.ClassVar[dict[str, GameTypeId]]  # value = {'GI': <GameTypeId.GI: 0>, 'WuWa': <GameTypeId.WuWa: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class GameTypeIdTools:
    """
    
    Tools for handling :class:`GameTypeId`
        
    """
    @staticmethod
    def getEnum(value: typing.SupportsInt | typing.SupportsIndex) -> FixRaidenBoss2.core.GameTypeId | None:
        """
        Retrieves the corresponding :class:`GameTypeId` for some integer value, checking that the value
        actually corresponds to one of :class:`GameTypeId`'s declared values
        
        Parameters
        ----------
        value: :class:`int`
            The integer value to convert
        
        Returns
        -------
        Optional[:class:`GameTypeId`]
            The corresponding :class:`GameTypeId`, if 'value' is valid
        """
    @staticmethod
    def getName(value: GameTypeId) -> str:
        """
        Retrieves the corresponding name for a :class:`GameTypeId`
        
        Parameters
        ----------
        value: :class:`GameTypeId`
            The :class:`GameTypeId` to retrieve the name for
        
        Returns
        -------
        :class:`str`
            The name for 'value'
        """
class GlobalRemapIniRemover(RemapIniRemover):
    """
    
    This class inherits from :class:`RemapIniRemover`
    
    General use class for removing the fixes from .ini files, without asking which type of mod the fix
    belonged to
    
    Everything about how the fix is found is :class:`RemapIniRemover`'s -- the only difference is that
    :meth:`GlobalRemapIniRemover.remove` always behaves as though it were passed a
    :class:`IniRemovalContext` with ``ignoreModType`` set.
    
    This is the remover for a .ini file that belongs to a mod but could not be attributed to any type of
    mod: :class:`RemapIniRemover`'s stricter rule decides a ``Remap``-named leftover `section`_ outside
    the fix boilerplate by asking whether its ``hash`` belongs to one of the .ini file's types of mod,
    and a file with no types of mod cannot answer that at all -- so every such leftover would be left
    standing. :meth:`IniFile.removeFix` reaches for this class in exactly that state, when its
    ``readAllIni`` was asked for.
    
    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to remove the fix from
        
    """
    def __init__(self, iniFile: typing.Any = None) -> None:
        ...
    def remove(self, parse: bool = False, writeBack: bool = True, context: typing.Any = None) -> str:
        """
        Removes the fix from the .ini file, without asking which type of mod it belonged to
        
        The fix is whatever the fix boilerplate surrounds, plus every ``Remap``-named leftover outside it --
        whoever they belong to -- together with everything those reference and everything that references
        them.
        
        Parameters
        ----------
        parse: :class:`bool`
            Ignored -- the resources that went with the removed `sections`_ are always collected, and are
            available from :meth:`RemapIniRemover.getRemovedResources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        writeBack: :class:`bool`
            Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        context: :class:`IniRemovalContext`
            The per-call options for this removal. Its ``ignoreModType`` is ignored -- this class always
            behaves as though it were ``True`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, which means a default-constructed one
        
        Returns
        -------
        :class:`str`
            The new content of the .ini file
        """
    @property
    def iniFile(self) -> typing.Any:
        """
        :class:`IniFile`: The .ini file that the fix will be removed from
        """
    @iniFile.setter
    def iniFile(self, arg0: typing.Any) -> None:
        ...
class GraphGroupEdit(BaseIniGraphGroupEdit):
    """
    
    This class inherits from :class:`BaseIniGraphGroupEdit`
    
    Edits the individual :class:`IniSectionGraph` from a group of graphs
    
    Parameters
    ----------
    edits: List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`]]]]
        The specific edits to make on the individual graphs :raw-html:`<br />` :raw-html:`<br />`
    
        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values of the dictionary are the individual edits for the corresponding graph
    
    trackKeys: Union[:class:`bool`, List[Dict[Tuple[:class:`str`, :class:`str`], :class:`bool`]]]
        Whether to track the `KVPs`_ in the .ini file for the edits passed into :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`
    
        For a :class:`BaseRegEdit`, this class walks the parts itself and hands each :attr:`keyFilters`
        entry a populated :attr:`SectionIterData.colouring`. For a :class:`BaseIniGraphEdit` -- which
        walks the graph itself and so never sees a colouring this class built -- the flag is instead
        **handed down** to that edit's own ``edit``/``editFromIni`` as its ``trackKeys`` argument, for
        the edit to honour. An edit carrying its own key-tracking setting combines the two (see
        :class:`RegFillMissing`); one without simply ignores it :raw-html:`<br />` :raw-html:`<br />`
    
        If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more
        granular flag setting can be made. The structure of the granular version of the data is as
        follows: :raw-html:`<br />` :raw-html:`<br />`
    
        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values of the dictionary are the values of the flag
    
        :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
    
    keysToTrack: Optional[List[Dict[Tuple[:class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]]
        Specific keys to track in the .ini file for the edits passed into :attr:`edits` -- handed down
        to a :class:`BaseIniGraphEdit` the same way :attr:`trackKeys` is :raw-html:`<br />` :raw-html:`<br />`
    
        * Each element of the outer list contains the edits for each .ini file
        * The keys in the dictionary contain the name of the component and the name of the mod object
        * The values are the keys to track for each graph. If the value is ``None``, then will keep
          track of all the keys encountered in some :class:`IfContentPart` for that graph
    
        :raw-html:`<br />`
    
        **Default**: ``None``
    
    keyFilters: Optional[List[Dict[Tuple[:class:`str`, :class:`str`], Union[List[Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]], Callable[[:class:`SectionIterData`], :class:`bool`]]]]]
        Functions to only process on specific parts of a `section`_ :raw-html:`<br />` :raw-html:`<br />`
    
        * Each element of the outer list contains the predicates for each .ini file
        * The keys are the name of the component and the name of the mod object
        * The values are functions that retrieve the ranges of valid order indices to process for some
          :class:`IfContentPart`. A single function (instead of a list) applies to every edit of that graph
    
        :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, edits: typing.Any, trackKeys: typing.Any = False, keysToTrack: typing.Any = None, keyFilters: typing.Any = None) -> None:
        ...
    def edit(self, graphGroups: list, modType: typing.Any, modName: str = '') -> list:
        """
        Edits a group of caller/callee graphs
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after editing
        """
    def editFromIni(self, graphGroups: list, ini: typing.Any, modType: typing.Any, modName: str = '') -> list:
        """
        Edits a group of caller/callee graphs with state info from 'ini'
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        ini: :class:`IniFile`
            The associated original .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after editing
        """
    @property
    def edits(self) -> typing.Any:
        """
        List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`]]]]:
        The specific edits to make on the individual graphs
        """
    @edits.setter
    def edits(self, arg1: typing.Any) -> None:
        ...
    @property
    def keyFilters(self) -> typing.Any:
        """
        List[Dict[Tuple[:class:`str`, :class:`str`], Union[List[Optional[Callable]], Callable]]]: Functions
        for any :class:`BaseRegEdit` to only process on specific parts of a `section`_
        """
    @keyFilters.setter
    def keyFilters(self, arg1: typing.Any) -> None:
        ...
    @property
    def keysToTrack(self) -> typing.Any:
        """
        List[Dict[Tuple[:class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]: Specific keys to track
        in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits`
        """
    @keysToTrack.setter
    def keysToTrack(self, arg1: typing.Any) -> None:
        ...
    @property
    def trackKeys(self) -> typing.Any:
        """
        Union[:class:`bool`, List[Dict[Tuple[:class:`str`, :class:`str`], :class:`bool`]]]: Whether to track
        the `KVPs`_ in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits`
        """
    @trackKeys.setter
    def trackKeys(self, arg1: typing.Any) -> None:
        ...
class GraphGroupRemap(BaseIniGraphGroupEdit):
    """
    
    This class inherits from :class:`BaseIniGraphGroupEdit`
    
    Remaps the graphs from a group of graphs
    
    Parameters
    ----------
    remap: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], List[Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]
        The remap for the graphs :raw-html:`<br />` :raw-html:`<br />`
    
        * The keys of the dictionary are the mod objects to remap from.
        * The values of the dictionary are the mod objects to remap to.
        * The tuples include:
    
            #. The index of the .ini file for the graph
            #. The name of the component for the graph
            #. The name of the mod object for the graph
            #. An optional rename function if the tuple has 4 values. The rename function takes in the old name of the `section`_
        
    """
    @staticmethod
    def copyGraph(fromGraph: typing.Any, modObj: typing.Any, newModObj: typing.Any, renameFunc: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Deep-copies 'fromGraph' and renames every `section`_ in the copy
        
        Parameters
        ----------
        fromGraph: :class:`IniSectionGraph`
            The graph to copy
        
        modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The id of the graph being copied from
        
        newModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The id of the graph being copied to
        
        renameFunc: Optional[Callable[[:class:`str`], :class:`str`]]
            The rename function to use. When ``None``, falls back to
            :meth:`IniNamingTools.getObjRemapFixName` against 'modObj'/'newModObj' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        modName: :class:`str`
            The name of the mod to fix to, used by that fallback :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`IniSectionGraph`
            The new, renamed copy
        """
    def __init__(self, remap: typing.Any) -> None:
        ...
    def edit(self, graphGroups: list, modType: typing.Any, modName: str = '') -> list:
        """
        Remaps the graphs, building each new graph with :meth:`copyGraph`
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Unused by this edit
        
        modName: :class:`str`
            The name of the mod to fix to, handed to :meth:`copyGraph` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after the graphs were remapped
        """
    def remapGraphs(self, graphGroups: list, createToGraph: typing.Any) -> list:
        """
        Remaps the graphs from a group of graphs
        
        .. note::
            A target whose ``(component, object)`` key is already taken in the destination .ini file's group
            goes into an **additional** group for that same .ini file (created on demand), rather than
            overwriting the existing graph
        
        .. note::
            A source .ini file whose original group is left with no graphs at all is dropped entirely
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to remap
        
        createToGraph: Callable[[:class:`IniSectionGraph`, Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Callable[[:class:`str`], :class:`str`]]], :class:`IniSectionGraph`]
            The function used to create the new remapped graph :raw-html:`<br />` :raw-html:`<br />`
        
            The function takes in the following parameters:
        
            #. The graph to map from
            #. The id of the graph to map from. The tuple contains the index of the .ini file for the graph, the name of the component and the name of the mod object
            #. The id of the graph to map to. Note that the index of the .ini file may not correspond to the actual index of which .ini file holds the graph
            #. An optional rename function
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after the graphs were remapped
        """
    @property
    def remap(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], List[Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]:
        The remap for the graphs
        """
    @remap.setter
    def remap(self, arg1: typing.Any) -> None:
        ...
class GraphInherit(BaseIniGraphGroupEdit):
    """
    
    This class inherits from :class:`BaseIniGraphGroupEdit`
    
    Merges the graph at 'dst' into the graph at 'src', by inserting consecutive `KVPs`_ into 'src' that
    reference every root `section`_ of the graph at 'dst'
    
    .. note::
        This only inserts the reference `KVPs`_ into 'src' -- the `sections`_ of 'dst' themselves are
        left untouched (and still need to be reachable/present elsewhere for the reference to resolve,
        the same way a plain ``run =`` reference to another `section`_ works)
    
    .. note::
        If either the graph at 'src' or the graph at 'dst' cannot be found, nothing is inserted and the
        original 'graphGroups' is returned as-is -- no exception is raised
    
    Parameters
    ----------
    src: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The id of the source :class:`IniSectionGraph` to insert the reference `KVPs`_ into. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`
    
        #. The index for the .ini file
        #. The name of the component
        #. The name of the object
    
    dst: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The id of the :class:`IniSectionGraph` to merge into 'src'. Same tuple format as 'src'
    
    reg: :class:`str`
        The name of the register used to reference the root `sections`_ of the graph at 'dst'
    
    latest: :class:`bool`
        Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the front :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
    
    partFilter: Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]
        The filter used to indicate which areas of some :class:`IfContentPart` within the graph at 'src'
        are valid to insert the `KVPs`_ :raw-html:`<br />` :raw-html:`<br />`
    
        If this value is ``None``, then the `KVPs`_ are instead inserted directly at the very
        front/back (based on 'latest') of every root `section`_ of the graph at 'src', instead of being
        filtered through every :class:`IfContentPart` of the graph :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, src: typing.Any, dst: typing.Any, reg: str, latest: bool = True, partFilter: typing.Any = None) -> None:
        ...
    def edit(self, graphGroups: list, modType: typing.Any, modName: str = '') -> list:
        """
        Inserts the reference `KVPs`_ from the graph at :attr:`dst` into the graph at :attr:`src`
        
        With no :attr:`partFilter`, the `KVPs`_ go straight to the very front/back (based on :attr:`latest`)
        of every root `section`_ of the graph at :attr:`src`. With one, they instead go at the
        earliest/latest valid index of every :class:`IfContentPart` the filter accepts
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Only ever handed to :attr:`partFilter`
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after editing
        """
    @property
    def dst(self) -> typing.Any:
        """
        Tuple[:class:`int`, :class:`str`, :class:`str`]: The id of the :class:`IniSectionGraph` to merge
        into :attr:`src`
        """
    @dst.setter
    def dst(self, arg1: typing.Any) -> None:
        ...
    @property
    def latest(self) -> bool:
        """
        :class:`bool`: Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the
        front
        """
    @latest.setter
    def latest(self, arg0: bool) -> None:
        ...
    @property
    def partFilter(self) -> typing.Any:
        """
        Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]:
        The filter used to indicate which areas of some :class:`IfContentPart` within the graph at
        :attr:`src` are valid to insert the `KVPs`_
        """
    @partFilter.setter
    def partFilter(self, arg1: typing.Any) -> None:
        ...
    @property
    def reg(self) -> str:
        """
        :class:`str`: The name of the register used to reference the root `sections`_ of the graph at
        :attr:`dst`
        """
    @reg.setter
    def reg(self, arg0: str) -> None:
        ...
    @property
    def src(self) -> typing.Any:
        """
        Tuple[:class:`int`, :class:`str`, :class:`str`]: The id of the source :class:`IniSectionGraph` to
        insert the reference `KVPs`_ into
        """
    @src.setter
    def src(self, arg1: typing.Any) -> None:
        ...
class GraphRemove(BaseIniGraphGroupEdit):
    """
    
    This class inherits from :class:`BaseIniGraphGroupEdit`
    
    Removes some graphs from a group of graphs
    
    .. note::
        A graph id that names no existing graph (a missing ``(component, object)`` key, or an
        out-of-range .ini index) is skipped silently -- no exception
    
    Parameters
    ----------
    graphIds: List[Tuple[:class:`int`, :class:`str`, :class:`str`]]
        The ids of the graphs to remove. Each tuple contains: :raw-html:`<br />` :raw-html:`<br />`
    
        #. The index for the .ini file
        #. The name of the component
        #. The name of the object
        
    """
    def __init__(self, graphIds: typing.Any) -> None:
        ...
    def edit(self, graphGroups: list, modType: typing.Any, modName: str = '') -> list:
        """
        Removes every graph named by :attr:`graphIds` from 'graphGroups'
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Unused by this edit
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after the graphs were removed
        """
    @property
    def graphIds(self) -> typing.Any:
        """
        List[Tuple[:class:`int`, :class:`str`, :class:`str`]]: The ids of the graphs to remove
        """
    @graphIds.setter
    def graphIds(self, arg1: typing.Any) -> None:
        ...
class GraphRename(BaseIniGraphEdit):
    """
    
    This class inherits from :class:`BaseIniGraphEdit`
    
    Renames the `sections`_ of some caller/callee graph of :class:`IniSectionGraph`
    
    Parameters
    ----------
    renameFunc: Callable[[:class:`str`], :class:`str`]
        Function used to rename a `section`_. The function takes in the name of the old `section`_ and
        returns the new name for the `section`_
        
    """
    def __init__(self, renameFunc: typing.Any) -> None:
        ...
    def edit(self, graph: typing.Any, modType: typing.Any, modName: str = '', partFilter: typing.Any = None, trackKeys: bool = False, keysToTrack: typing.Any = None) -> typing.Any:
        """
        Renames every `section`_ of 'graph' by :attr:`renameFunc`
        
        Every ``run =`` reference to a renamed `section`_ is rewritten too, and the graph is rebuilt -- so a
        rename never leaves a dangling caller/callee edge behind
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Unused by this edit
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
            The filter used to indicate the valid order indices to process some :class:`IfContentPart` in
            the graph. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        trackKeys: :class:`bool`
            The caller's key-tracking default. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        keysToTrack: Optional[Set[:class:`str`]]
            The caller's key-tracking key set. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IniSectionGraph`
            The same graph that was passed in, after every `section`_ was renamed
        """
    @property
    def renameFunc(self) -> typing.Any:
        """
        Callable[[:class:`str`], :class:`str`]: Function used to rename a `section`_. The function takes in
        the name of the old `section`_ and returns the new name for the `section`_
        """
    @renameFunc.setter
    def renameFunc(self, arg1: typing.Any) -> None:
        ...
class GraphTools:
    """
    
    Tools for handling with generic directed graphs, represented as adjacency lists (``node -> list of
    the nodes directly reachable from it``)
    
    Nodes can be any hashable value -- these tools have no notion of what a node "is" (a section, an
    IfContentPart, a plain str, ...); that meaning is entirely up to the caller.
        
    """
    @staticmethod
    def clampFactsToReachable(facts: collections.abc.Mapping[typing.Any, bool], reachableNodes: collections.abc.Set[typing.Any]) -> dict[typing.Any, bool]:
        """
        Forces every fact about a node not in 'reachableNodes' down to False.
        
        Parameters
        ----------
        facts: Dict[Any, bool]
            The raw facts to clamp, as returned by runForwardMustFixpoint/runBackwardMustFixpoint.
        
        reachableNodes: Set[Any]
            See getReachableNodes.
        
        Returns
        -------
        Dict[Any, bool]
            The clamped facts.
        """
    @staticmethod
    def getReachableNodes(forwardEdges: collections.abc.Mapping[typing.Any, collections.abc.Sequence[typing.Any]], rootNodes: collections.abc.Set[typing.Any]) -> set[typing.Any]:
        """
        Computes every node reachable from 'rootNodes', via plain forward graph reachability (BFS/DFS --
        no dataflow facts involved, just "is there some path here at all").
        
        Parameters
        ----------
        forwardEdges: Dict[Any, List[Any]]
            The graph to search, as ``node -> list of the nodes directly reachable from it``.
        
        rootNodes: Set[Any]
            The nodes to start searching from.
        
        Returns
        -------
        Set[Any]
            Every node reachable from 'rootNodes' (including 'rootNodes' themselves).
        """
    @staticmethod
    def runBackwardMustFixpoint(forwardEdges: collections.abc.Mapping[typing.Any, collections.abc.Sequence[typing.Any]], backwardEdges: collections.abc.Mapping[typing.Any, collections.abc.Sequence[typing.Any]], localFacts: collections.abc.Mapping[typing.Any, bool]) -> dict[typing.Any, bool]:
        """
        The mirror of runForwardMustFixpoint: a backward MUST (very-busy-expressions-style) dataflow
        analysis over the same kind of graph, computing whether some boolean property is guaranteed to be
        established somewhere after every node exits.
        
        Parameters
        ----------
        forwardEdges: Dict[Any, List[Any]]
            The graph to analyze, as ``node -> list of the nodes that can run directly after it``.
        
        backwardEdges: Dict[Any, List[Any]]
            The reverse of 'forwardEdges'.
        
        localFacts: Dict[Any, bool]
            For every node, whether its own content, by itself, already establishes the property being
            tracked. A node missing from this dict is treated as False.
        
        Returns
        -------
        Dict[Any, bool]
            Every node reachable in the graph structure, mapped to whether the property is guaranteed to
            be satisfied somewhere after that node exits.
        """
    @staticmethod
    def runForwardMustFixpoint(forwardEdges: collections.abc.Mapping[typing.Any, collections.abc.Sequence[typing.Any]], backwardEdges: collections.abc.Mapping[typing.Any, collections.abc.Sequence[typing.Any]], rootNodes: collections.abc.Set[typing.Any], localFacts: collections.abc.Mapping[typing.Any, tuple[bool, bool]]) -> dict[typing.Any, bool]:
        """
        Runs a forward, MUST (available-expressions-style) dataflow analysis over a graph, computing
        whether some boolean property has been established entering every node -- correctly handling
        cycles via fixpoint iteration (Kildall's/worklist algorithm).
        
        Parameters
        ----------
        forwardEdges: Dict[Any, List[Any]]
            The graph to analyze, as ``node -> list of the nodes that can run directly after it``.
        
        backwardEdges: Dict[Any, List[Any]]
            The reverse of 'forwardEdges'.
        
        rootNodes: Set[Any]
            The nodes that are true entry points of the graph.
        
        localFacts: Dict[Any, Tuple[bool, bool]]
            For every node with content of its own worth examining, a tuple of (touches, localSatisfied).
            A node missing from this dict is treated as a pure pass-through.
        
        Returns
        -------
        Dict[Any, bool]
            Every node reachable in the graph structure, mapped to whether the property is satisfied
            entering that node.
        """
class Hash128:
    """
    
    A deterministic 128-bit hash id, the long counterpart to :class:`Hash64`
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' store the same hash value
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' store different hash values
    
        .. describe:: x < y
    
            An arbitrary but consistent (and deterministic) total ordering
    
        .. describe:: hash(x)
    
            Retrieves a hash of 'x' itself, so that 'x' can be used as a key in a :class:`dict`/:class:`set`
    
        .. describe:: str(x)
    
            Equivalent to ``x.toHexString()``
        
    """
    @staticmethod
    @typing.overload
    def hash(data: bytes) -> Hash128:
        """
        Deterministically hashes a buffer of bytes
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`Hash128`
            The resultant hash
        """
    @staticmethod
    @typing.overload
    def hash(str: str) -> Hash128:
        """
        Deterministically hashes a string
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`Hash128`
            The resultant hash
        """
    def __eq__(self, other: Hash128) -> bool:
        """
        Determines whether 'self' and 'other' store the same hash value
        """
    def __hash__(self) -> int:
        """
        Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Constructs a hash with both halves set to 0
        """
    @typing.overload
    def __init__(self, high: typing.SupportsInt | typing.SupportsIndex, low: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Constructs a hash from its 2 64-bit halves
        
        Parameters
        ----------
        high: :class:`int`
            The high 64 bits of the hash
        
        low: :class:`int`
            The low 64 bits of the hash
        """
    def __lt__(self, other: Hash128) -> bool:
        """
        An arbitrary but consistent (and deterministic) total ordering
        """
    def __ne__(self, other: Hash128) -> bool:
        """
        Determines whether 'self' and 'other' store different hash values
        """
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def toBase64(self) -> str:
        """
        Converts the hash to a fixed-length base64 string
        
        Returns
        -------
        :class:`str`
            The base64 string
        """
    def toHexString(self) -> str:
        """
        Converts the hash to a fixed-length, lowercase hex string
        
        Returns
        -------
        :class:`str`
            The hex string
        """
    @property
    def high(self) -> int:
        """
        :class:`int`: The high 64 bits of the hash
        """
    @property
    def low(self) -> int:
        """
        :class:`int`: The low 64 bits of the hash
        """
class Hash64:
    """
    
    A deterministic 64-bit hash id, the short counterpart to :class:`Hash128`
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' store the same hash value
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' store different hash values
    
        .. describe:: x < y
    
            An arbitrary but consistent (and deterministic) total ordering
    
        .. describe:: hash(x)
    
            Retrieves a hash of 'x' itself, so that 'x' can be used as a key in a :class:`dict`/:class:`set`
    
        .. describe:: str(x)
    
            Equivalent to ``x.toHexString()``
        
    """
    @staticmethod
    @typing.overload
    def hash(data: bytes) -> Hash64:
        """
        Deterministically hashes a buffer of bytes
        
        Parameters
        ----------
        data: :class:`bytes`
            The buffer of bytes to hash
        
        Returns
        -------
        :class:`Hash64`
            The resultant hash
        """
    @staticmethod
    @typing.overload
    def hash(str: str) -> Hash64:
        """
        Deterministically hashes a string
        
        Parameters
        ----------
        str: :class:`str`
            The string to hash
        
        Returns
        -------
        :class:`Hash64`
            The resultant hash
        """
    def __eq__(self, other: Hash64) -> bool:
        """
        Determines whether 'self' and 'other' store the same hash value
        """
    def __hash__(self) -> int:
        """
        Retrieves a hash of this instance itself, so that it can be used as a key in a dict/set
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Constructs a hash with a value of 0
        """
    @typing.overload
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        """
        Constructs a hash from its raw 64-bit value
        
        Parameters
        ----------
        value: :class:`int`
            The raw 64-bit value of the hash
        """
    def __lt__(self, other: Hash64) -> bool:
        """
        An arbitrary but consistent (and deterministic) total ordering
        """
    def __ne__(self, other: Hash64) -> bool:
        """
        Determines whether 'self' and 'other' store different hash values
        """
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def toBase64(self) -> str:
        """
        Converts the hash to a fixed-length base64 string
        
        Returns
        -------
        :class:`str`
            The base64 string
        """
    def toHexString(self) -> str:
        """
        Converts the hash to a fixed-length, lowercase hex string
        
        Returns
        -------
        :class:`str`
            The hex string
        """
    @property
    def value(self) -> int:
        """
        :class:`int`: The raw 64-bit value of the hash
        """
class Hashes(ModMappedAssets):
    """
    
    This class inherits from :class:`ModMappedAssets`
    
    Class for managing hashes for a mod, pre-populated with this project's real hash data
    
    :raw-html:`<br />`
    
    .. note::
        Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/
        ``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:
    
        * version (version index)
        * name
        * type
        
    """
    def __init__(self, map: typing.Any = None) -> None:
        """
        Constructs a new, fully-populated hash lookup table
        
        Parameters
        ----------
        map: Optional[Dict[Any, List[Any]]]
            The `adjacency list`_ that maps the hashes to fix from to the hashes to fix to using the
            predefined mods
        
            **Default**: ``None``
        """
class IOrderedMultiMap:
    """
    
    An abstract ordered-multimap interface: implement every method below (in a Python subclass) to
    plug an entirely custom backing structure into any C++ code that accepts this interface --
    :class:`OrderedMultiMap` and :class:`OrderedMultiMapSqrt` are two such implementations,
    each exposed as an :class:`IOrderedMultiMap` via their own ``asInterface()`` method.
    
    .. note::
        Unlike :class:`OrderedMultiMap`, the ``ranges`` parameter accepted throughout this
        interface takes either a bound :class:`Ranges` instance or a plain
        ``List[Tuple[Optional[int], Optional[int]]]`` of ``(start, end)`` bounds -- a Python
        subclass's own override of a ``ranges``-taking method always receives the latter, plain
        shape.
            
    """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> IOrderedMultiMap:
        """
        Creates a copy of this instance (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> IOrderedMultiMap:
        """
        Creates a deep copy of this instance (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def __init__(self) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        """
        Iterates every entry in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def __len__(self) -> int:
        """
        Retrieves the number of entries
        """
    def clone(self) -> IOrderedMultiMap:
        """
        Creates a deep copy of this instance
        """
    def contains(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def containsKey(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def count(self, key: typing.Any) -> int:
        """
        Retrieves how many entries share a given key
        """
    def empty(self) -> bool:
        """
        Checks whether the map is empty
        """
    def entries(self) -> list[tuple[typing.Any, typing.Any]]:
        """
        Retrieves a copy of the full ordered sequence, as ``(key, value)`` pairs
        """
    def getAll(self, key: typing.Any, ordered: bool = True, ranges: typing.Any = None) -> list[typing.Any]:
        """
        Retrieves all values currently stored under a key; see :meth:`CppOrderedMultiMap.getAll` for the
        full semantics
        """
    def getAllWithInds(self, key: typing.Any, ordered: bool = True, ranges: typing.Any = None) -> list[tuple[int, typing.Any]]:
        """
        Retrieves all values currently stored under a key, each paired with its true positional index;
        see :meth:`CppOrderedMultiMap.getAllWithInds` for the full semantics
        """
    def getByInd(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def getByIndWithOccurrence(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[int, typing.Any]:
        """
        Retrieves the entry at a true positional index, paired with its occurrence index
        """
    def getKeys(self) -> list[typing.Any]:
        """
        Retrieves every distinct key currently in the map, as a :class:`list` rather than a real
        ``set`` -- unlike :meth:`CppOrderedMultiMap.getKeys`, this interface has no way to guarantee an
        arbitrary key type is hashable
        """
    def insert(self, key: typing.Any, value: typing.Any) -> None:
        """
        Appends a key-value pair to the end
        """
    def insertAllAt(self, items: dict, sortIndices: bool = True, ranges: typing.Any = None) -> int:
        """
        Bulk indexed insert; see :meth:`OrderedMultiMap.insertAllAt` for the full semantics
        """
    def insertAllEnd(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Appends a batch of key-value pairs to the end, in the order given
        """
    def insertAllStart(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Inserts a batch of key-value pairs at the beginning, in the order given
        """
    def insertAt(self, index: typing.SupportsInt | typing.SupportsIndex, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair so it ends up at position 'index' (0-based); see :meth:`OrderedMultiMap.insertAt` for the full index semantics
        """
    def insertStart(self, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair at the beginning
        """
    def items(self) -> list[tuple[typing.Any, typing.Any, int, int]]:
        """
        Retrieves a copy of the full ordered sequence, as ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def keySize(self) -> int:
        """
        Retrieves the number of distinct keys
        """
    def length(self) -> int:
        """
        Retrieves the number of entries
        """
    def remapKeys(self, keyRemap: dict, ranges: typing.Any = None) -> None:
        """
        Bulk-renames keys; see :meth:`OrderedMultiMap.remapKeys` for the full semantics
        """
    def removeAt(self, pos: typing.SupportsInt | typing.SupportsIndex, ranges: typing.Any = None) -> bool:
        """
        Removes the entry currently at position 'pos'
        """
    def removeKey(self, key: typing.Any, ranges: typing.Any = None, check: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex, typing.Any], bool] | None = None) -> int:
        """
        Removes every entry with this key, subject to optional 'ranges'/'check' filters; see
        :meth:`OrderedMultiMap.removeKey` for the full semantics -- 'check', if provided, is
        ``check(index, value)``
        """
    def reorder(self, orderMap: dict, ranges: typing.Any = None) -> None:
        """
        Reorders existing entries in place; see :meth:`OrderedMultiMap.reorder` for the full semantics
        """
    def replaceVals(self, newVals: dict, addNew: bool = True, ranges: typing.Any = None) -> None:
        """
        Bulk-updates values by key; see :meth:`OrderedMultiMap.replaceVals` for the full semantics
        """
    def setValByInd(self, index: typing.SupportsInt | typing.SupportsIndex, value: typing.Any) -> None:
        """
        Sets the value of the entry at a true positional index, leaving its key untouched
        """
    def size(self) -> int:
        """
        Retrieves the number of entries
        """
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[IOrderedMultiMap]:
        """
        Splits this map into several smaller maps at the given indices; see :meth:`OrderedMultiMap.splitByInds` for the full semantics
        """
class IbFile(CppBufFile):
    """
    
    This class inherits from :class:`CppBufFile`
    
    Used for handling ``.ib`` (index buffer) files
    
    .. note::
        Where a ``.buf`` file is split into *vertex lines*, a ``.ib`` file is split into *face lines* --
        each line names the vertices making up one triangular face of the mod's mesh. Every face in a
        3dmigoto mod is atomically a triangle, so a line is always :attr:`VerticesPerTriangle` 32-bit
        unsigned integers
    
    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the ``.ib`` file
    
    Raises
    ------
    :class:`BufFileNotRecognized`
        If 'src' holds a file path that cannot be read as a valid ``.ib`` file
    
    :class:`BadBufData`
        If 'src' holds raw bytes that are not valid for a ``.ib`` file
        
    """
    TriangleBufElementKey: typing.ClassVar[str] = 'Triangle'
    VerticesPerTriangle: typing.ClassVar[int] = 3
    def __init__(self, src: typing.Any) -> None:
        ...
    def getDumpStr(self, firstIndex: typing.SupportsInt | typing.SupportsIndex = 0) -> str:
        """
        Retrieves the full text for converting this ``.ib`` file into a dumped *ib.txt* file
        
        .. note::
            Unlike :meth:`CppBufFile.getDumpStr`, this returns a *complete* dump -- header included -- and
            its data section comes from :meth:`CppBufFile.getFlatDumpStr`, a ``.ib`` file's own flat,
            space-separated form rather than the per-element form a vertex buffer's data uses:
        
            .. code-block::
        
                byte offset: 0
                first index: 0
                index count: 6
                topology: trianglelist
                format: DXGI_FORMAT_R16_UINT
        
                0 1 2
                3 4 5
        
        Parameters
        ----------
        firstIndex: :class:`int`
            The index this file's first vertex index is numbered from (see :meth:`makeDumpHeader`) :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``0``
        
        Returns
        -------
        :class:`str`
            The text for the dumped *ib.txt* file
        """
    def getIndexCount(self) -> int:
        """
        Retrieves the number of vertex indices in the file -- :attr:`VerticesPerTriangle` times
        :meth:`getTriangleCount`
        
        Returns
        -------
        :class:`int`
            The number of indices
        """
    def getTriangleCount(self) -> int:
        """
        Retrieves the number of triangular faces making up the mod's mesh
        
        Returns
        -------
        :class:`int`
            The number of faces
        """
    def makeDumpHeader(self, firstIndex: typing.SupportsInt | typing.SupportsIndex = 0) -> str:
        """
        Makes the header for a dumped *ib.txt* file
        
        Parameters
        ----------
        firstIndex: :class:`int`
            The index this file's first vertex index is numbered from :raw-html:`<br />` :raw-html:`<br />`
        
            A mod's faces are spread over several ``.ib`` files (one per mod object), which a dump numbers
            continuously -- so each file after the first starts where the previous one's
            :meth:`getIndexCount` left off :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``0``
        
        Returns
        -------
        :class:`str`
            The header text
        """
    def readDumpStr(self, text: str) -> None:
        """
        Reads a dumped *ib.txt* file's text back into this ``.ib`` file's bytes -- the inverse of
        :meth:`getDumpStr`
        
        .. note::
            This is a convenience for calling :meth:`CppBufFile.readFlatDumpStr` -- an index buffer's dump
            uses the flat, space-separated form, not the per-element form a vertex buffer's does. The header
            is skipped, so a whole dump file can be handed straight in
        
        Parameters
        ----------
        text: :class:`str`
            The text of the dumped *ib.txt* file
        
        Raises
        ------
        :class:`BadBufData`
            If the parsed bytes do not divide evenly into face lines
        """
class IfContentPart(IfTemplatePart):
    """
    
    This class inherits from :class:`IfTemplatePart`
    
    The content part of an `IfTemplate`, holding the key-value pairs (e.g. a `.ini` section's
    registers) for one part of the template.
    
    This class owns its data purely through a caller-supplied :class:`IOrderedMultiMap`
    implementation -- pick which concrete ordered-multimap backs a given :class:`IfContentPart`
    (:class:`CppOrderedMultiMap`/:class:`CppOrderedMultiMapSqrt` via their ``asInterface()`` method,
    or any custom :class:`IOrderedMultiMap` implementation of your own, including one implemented
    from Python), and every method on this class is a thin, renamed delegation straight to that
    implementation -- the semantics for every operation are exactly :class:`CppOrderedMultiMap`'s
    documented rules; only the *method names* below intentionally echo this project's deprecated,
    pre-C++-port `IfContentPart` naming (e.g. ``insertAllAt`` -> ``addKVPsByInds``).
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines if 'key' exists in the content part
    
        .. describe:: len(x)
    
            Retrieves the number of KVPs in the content part
    
        .. describe:: x[index]
    
            Retrieves the ``(key, value)`` pair at the given true positional index, if ``index`` is
            an :class:`int`
    
        .. describe:: x[key]
    
            Retrieves every value currently stored under ``key``, in true positional order
            (equivalent to :meth:`getVals` with ``ordered=True``), if ``key`` is a :class:`str` --
            raises :class:`KeyError` if ``key`` doesn't exist
    
        .. describe:: iter(x)
    
            Iterates every KVP in true positional order, yielding ``(key, value, occurrenceIndex,
            orderIndex)`` tuples
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
        deep copy (equivalent to ``x.clone()``)
    
    Parameters
    ----------
    src: Optional[Dict[Any, List[Tuple[:class:`int`, Any]]]]
        Initial data to populate ``content`` with, as key -> list of ``(index, value)`` pairs, one
        entry per occurrence of that key :raw-html:`<br />` :raw-html:`<br />`
    
        ``index`` orders every occurrence relative to every other occurrence *across all keys*
        (gathered, stable-sorted by ``index`` ascending, then appended in that order); it is **not**
        a strict absolute position, so gaps and duplicate/out-of-order values are fine. If
        ``content`` already holds data (a pre-populated instance was passed in rather than left to
        default), ``src``'s entries are appended after it, not merged/interleaved with it.
        :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, meaning no initial data is inserted
    
    depth: :class:`int`
        The depth this part is within the owning `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``0``
    
    content: Optional[:class:`IOrderedMultiMap`]
        The backing ordered-multimap implementation to use, taken by ownership -- see this class's
        top-level warning about what that means for 'content' afterward :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, meaning a fresh, empty :class:`CppOrderedMultiMap` is used
    
    id: Optional[:class:`int`]
        The id for the part. If this parameter is ``None``, will generate a new id for the part.
    
        **Default**: ``None``
            
    """
    @staticmethod
    def buildFromOrder(src: typing.Any, depth: typing.SupportsInt | typing.SupportsIndex = 0, content: typing.Any = None, id: typing.Any = None) -> IfContentPart:
        """
        Creates a new part, populated from a flat, already-ordered list of key-value pairs -- a thin
        convenience over default-constructing then calling :meth:`addKVPs`
        
        Parameters
        ----------
        src: List[Tuple[Any, Any]]
            The key-value pairs to populate ``content`` with, appended in the order given (``src[0]``
            ends up first, right after whatever ``content`` already held, and so on) -- a key may repeat
            here directly, e.g. ``[("a", "1"), ("b", "2"), ("a", "3")]``
        
        depth: :class:`int`
            The depth this part is within the owning `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``0``
        
        content: Optional[:class:`IOrderedMultiMap`]
            The backing ordered-multimap implementation to use, taken by ownership -- see
            :class:`IfContentPart`'s top-level warning about what that means for 'content' afterward
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning a fresh, empty :class:`CppOrderedMultiMap` is used
        
        id: Optional[:class:`int`]
            The id for the part. If this parameter is ``None``, will generate a new id for the part.
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IfContentPart`
            The newly-created part
        """
    def __contains__(self, key: str) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> IfContentPart:
        """
        Creates a copy of this part (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> IfContentPart:
        """
        Creates a deep copy of this part (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """
    @typing.overload
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[str, str]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    @typing.overload
    def __getitem__(self, key: str) -> list[str]:
        """
        Retrieves all values currently stored under a key, in true positional order (equivalent to :meth:`getVals` with ``ordered=True``); raises :class:`KeyError` if the key doesn't exist
        """
    def __init__(self, src: typing.Any = None, depth: typing.SupportsInt | typing.SupportsIndex = 0, content: typing.Any = None, id: typing.Any = None) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        """
        Iterates every KVP in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def __len__(self) -> int:
        """
        Retrieves the number of KVPs
        """
    def addKVP(self, key: str, value: str) -> None:
        """
        Appends a KVP to the end
        """
    def addKVPAt(self, index: typing.SupportsInt | typing.SupportsIndex, key: str, value: str) -> None:
        """
        Inserts a KVP so it ends up at position 'index' (0-based); see :meth:`CppOrderedMultiMap.insertAt` for the full index semantics
        """
    def addKVPToFront(self, key: str, value: str) -> None:
        """
        Inserts a KVP at the beginning
        """
    def addKVPs(self, kvps: collections.abc.Sequence[tuple[str, str]]) -> None:
        """
        Appends a batch of KVPs to the end, in the order given
        """
    def addKVPsByInds(self, kvps: dict, sortIndices: bool = True, ranges: typing.Any = None) -> int:
        """
        Bulk indexed insert of KVPs; see :meth:`CppOrderedMultiMap.insertAllAt` for the full semantics
        """
    def addKVPsToFront(self, kvps: collections.abc.Sequence[tuple[str, str]]) -> None:
        """
        Inserts a batch of KVPs at the beginning, in the order given
        """
    def clone(self, newId: bool = False) -> IfContentPart:
        """
        Creates a deep copy of this part, at the same depth
        
        Parameters
        ----------
        newId: :class:`bool`
            Whether to generate a new id for the cloned part :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning the clone keeps this part's own :attr:`id`
        
        Returns
        -------
        :class:`IfContentPart`
            The cloned part
        """
    def contains(self, key: str) -> bool:
        """
        Checks whether a key exists
        """
    def containsKey(self, key: str) -> bool:
        """
        Checks whether a key exists
        """
    def count(self, key: str) -> int:
        """
        Retrieves how many KVPs share a given key
        """
    def empty(self) -> bool:
        """
        Checks whether the part has no KVPs
        """
    def entries(self) -> list[tuple[str, str]]:
        """
        Retrieves a copy of the full ordered sequence, as ``(key, value)`` pairs
        """
    @typing.overload
    def get(self, key: typing.SupportsInt | typing.SupportsIndex, errorOnNotFound: bool = False, default: typing.Any = None, ordered: bool = True, withInds: bool = False, ranges: typing.Any = None) -> typing.Any:
        """
        Retrieves the ``(key, value)`` pair at a true positional index (if ``key`` is an :class:`int`) or
        every value currently stored under a key (if ``key`` is a :class:`str`) -- like :meth:`__getitem__`,
        except not finding anything is configurable instead of always raising.
        
        Parameters
        ----------
        key: Union[:class:`int`, :class:`str`]
            The true positional index or key to look up
        
        errorOnNotFound: :class:`bool`
            If ``True`` and nothing is found, raises :class:`KeyError` (``key`` was a :class:`str`) or
            :class:`IndexError` (``key`` was an :class:`int`, out of range). If ``False`` (the default),
            returns ``default`` instead of raising.
        
        default: Any
            The value returned when nothing is found and ``errorOnNotFound`` is ``False`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        ordered: :class:`bool`
            Only takes effect when ``key`` is a :class:`str` -- same purpose as ``ordered`` from
            :meth:`CppOrderedMultiMap.getAll` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        withInds: :class:`bool`
            Only takes effect when ``key`` is a :class:`str` -- if ``True``, each returned value is
            paired with its true positional index (equivalent to :meth:`getValsWithInds`); if ``False``
            (the default), values are returned bare (equivalent to :meth:`getVals`)
        
        ranges: Optional[:class:`Ranges`]
            Only takes effect when ``key`` is a :class:`str` -- if provided, only occurrences whose true
            positional index (same convention as :meth:`getByInd`) falls within ``ranges`` are
            considered :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is considered
        
        Returns
        -------
        Any
            The result described above, or ``default`` if nothing was found and ``errorOnNotFound`` is
            ``False``
        """
    @typing.overload
    def get(self, key: str, errorOnNotFound: bool = False, default: typing.Any = None, ordered: bool = True, withInds: bool = False, ranges: typing.Any = None) -> typing.Any:
        """
        Same as the ``int``-keyed overload above, for a :class:`str` ``key`` -- see its docstring for
        the full parameter descriptions
        """
    def getByInd(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[str, str]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def getByIndWithOccurrence(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[int, str]:
        """
        Retrieves the KVP at a true positional index, paired with its occurrence index
        """
    def getKeys(self) -> set[str]:
        """
        Retrieves every distinct key currently in this part
        
        Returns
        -------
        Set[Any]
            Every distinct key, as a set (unordered)
        """
    def getVals(self, key: str, ordered: bool = True, ranges: typing.Any = None) -> list[str]:
        """
        Retrieves all values currently stored under a key
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Any]
            The values for this key, in the requested order
        """
    def getValsWithInds(self, key: str, ordered: bool = True, ranges: typing.Any = None) -> list[tuple[int, str]]:
        """
        Retrieves all values currently stored under a key, each paired with its true positional index
        (equivalent to :meth:`getVals`, except each value is paired with its true positional index)
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Tuple[:class:`int`, Any]]
            The ``(index, value)`` pairs for this key, in the requested order
        """
    def items(self) -> list[tuple[str, str, int, int]]:
        """
        Retrieves a copy of the full ordered sequence, as ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def keySize(self) -> int:
        """
        Retrieves the number of distinct keys
        """
    def length(self) -> int:
        """
        Retrieves the number of KVPs
        """
    def remapKeys(self, keyRemap: dict, ranges: typing.Any = None) -> None:
        """
        Bulk-renames keys; see :meth:`CppOrderedMultiMap.remapKeys` for the full semantics
        """
    def removeKVPAt(self, pos: typing.SupportsInt | typing.SupportsIndex, ranges: typing.Any = None) -> bool:
        """
        Removes the KVP currently at position 'pos'
        """
    def removeKey(self, key: str, ranges: typing.Any = None, check: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex, str], bool] | None = None) -> int:
        """
        Removes every KVP with this key, subject to two independent, optional filters -- both must hold
        (where provided) for a given occurrence to actually be removed. With neither filter provided,
        this is unconditional removal of every KVP with this key.
        
        Parameters
        ----------
        key: Any
            The key whose KVPs to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, the occurrence's true positional index (same convention as :meth:`getByInd`)
            must fall within 'ranges'
        
        check: Optional[Callable[[int, Any], bool]]
            If provided, ``check(index, value)`` must return ``True``, given that occurrence's true
            positional index and value
        
        Returns
        -------
        :class:`int`
            How many KVPs were actually removed
        """
    def removeKeys(self, keys: dict, ranges: typing.Any = None) -> int:
        """
        Removes multiple, independently-specified keys -- a thin loop over :meth:`removeKey`, sharing
        one 'ranges' filter across all of them.
        
        Parameters
        ----------
        keys: Dict[Any, Optional[Callable[[int, Any], bool]]]
            Each key to remove, mapped to its own optional check predicate -- ``check(index, value)``
            must return ``True`` for a given occurrence of that key to actually be removed, if provided;
            if omitted, every occurrence of that key is removed unconditionally (subject to 'ranges'
            below)
        
        ranges: Optional[:class:`Ranges`]
            If provided, an occurrence's true positional index (same convention as :meth:`getByInd`)
            must fall within 'ranges' for it to be eligible for removal, for every key in 'keys'
        
        Returns
        -------
        :class:`int`
            How many KVPs were actually removed, summed across every key in 'keys'
        """
    def reorder(self, orderMap: dict, ranges: typing.Any = None) -> None:
        """
        Reorders existing KVPs in place; see :meth:`CppOrderedMultiMap.reorder` for the full semantics
        """
    def replaceVals(self, newVals: dict, addNew: bool = True, ranges: typing.Any = None) -> None:
        """
        Bulk-updates values by key; see :meth:`CppOrderedMultiMap.replaceVals` for the full semantics
        """
    def setValByInd(self, index: typing.SupportsInt | typing.SupportsIndex, value: str) -> None:
        """
        Sets the value of the KVP at a true positional index, leaving its key untouched
        """
    def size(self) -> int:
        """
        Retrieves the number of KVPs
        """
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[IfContentPart]:
        """
        Splits this part into several smaller parts at the given indices, each at the same depth as
        this part; see :meth:`CppOrderedMultiMap.splitByInds` for the full semantics
        
        Returns
        -------
        List[:class:`IfContentPart`]
            The resulting parts, left to right
        """
    def toStr(self, linePrefix: str = '') -> str:
        """
        Retrieves the part as a string, one ``key = value`` line per KVP in true positional order
        
        Parameters
        ----------
        linePrefix: :class:`str`
            The string that will prefix every line :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The string representation of the part
        """
    @property
    def depth(self) -> int:
        """
        :class:`int`: The depth this part is within the owning `IfTemplate`
        """
    @depth.setter
    def depth(self, arg1: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class IfContentPartColourChange:
    """
    
    Class to store the change in state of a particular key for a :class:`IfContentPartColouring`
    
    Parameters
    ----------
    old: Optional[Any]
        The old value of a particular key -- either a plain value (the key's value came from some
        previous :class:`IfContentPart`), or a ``List[Tuple[int, Any]]`` (the key's values come
        from the current :class:`IfContentPart`, each paired with its index of occurrence) :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, meaning the key didn't exist beforehand
            
    """
    def __copy__(self) -> IfContentPartColourChange:
        """
        Creates a copy of this change record (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> IfContentPartColourChange:
        """
        Creates a copy of this change record (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """
    def __init__(self, old: typing.Any = None) -> None:
        ...
    def clone(self) -> IfContentPartColourChange:
        """
        Creates a copy of this change record
        """
    def restore(self, colouring: IfContentPartColouring, key: str) -> None:
        """
        Restores the old value for a particular key within ``colouring``
        
        Parameters
        ----------
        colouring: :class:`IfContentPartColouring`
            The colouring to restore a value within
        
        key: :class:`str`
            The key to restore -- if ``key`` isn't currently in ``colouring``, this has no effect
        """
    @property
    def old(self) -> typing.Any:
        """
        Optional[Any]: The old value of a particular key
        """
    @old.setter
    def old(self, arg1: typing.Any) -> None:
        ...
class IfContentPartColouring:
    """
    
    Class that keeps track of the current state of the `KVPs`_ within a :class:`IfContentPart` --
    the C++-backed port of the deprecated pure-Python original (since removed)
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines if 'key' currently has a tracked state
    
        .. describe:: len(x)
    
            Retrieves the number of keys currently tracked
    
        .. describe:: x[key]
    
            Retrieves the current state for 'key'; raises :class:`KeyError` if not tracked
    
        .. describe:: x[key] = value
    
            Sets the current state for 'key'
    
        .. describe:: del x[key]
    
            Removes the current state for 'key'; raises :class:`KeyError` if not tracked
    
        .. describe:: iter(x)
    
            Iterates every currently-tracked key, in insertion order
    
    :raw-html:`<br />` :raw-html:`<br />`
    
    * The keys are the names of the register keys
    * The values are either:
    
        * A plain value, indicating the value of the `KVP`_ comes from some previous :class:`IfContentPart`, OR
        * A ``List[Tuple[int, Any]]``. The list indicates that the values of the corresponding key
          come from the current :class:`IfContentPart`, each tuple containing the new state value
          for the corresponding key and its index of occurrence within the current part
    
    Parameters
    ----------
    src: Optional[Dict[Any, Any]]
        Initial state to populate this colouring with, in the same key -> value shape described above :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
            
    """
    def __contains__(self, key: str) -> bool:
        """
        Determines whether 'key' currently has a tracked state
        """
    def __copy__(self) -> IfContentPartColouring:
        """
        Creates a copy of this colouring (equivalent to :meth:`clone`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> IfContentPartColouring:
        """
        Creates a copy of this colouring (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """
    def __delitem__(self, key: str) -> None:
        """
        Removes the current state for 'key'; raises :class:`KeyError` if not tracked
        """
    def __getitem__(self, key: str) -> typing.Any:
        """
        Retrieves the current state for 'key'; raises :class:`KeyError` if not tracked
        """
    def __init__(self, src: typing.Any = None) -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        """
        Iterates every currently-tracked key, in insertion order
        """
    def __len__(self) -> int:
        """
        Retrieves the number of keys currently tracked
        """
    def __setitem__(self, key: str, value: typing.Any) -> None:
        """
        Sets the current state for 'key', inserting it if not already tracked
        """
    def clear(self) -> None:
        """
        Removes every tracked key
        """
    def clone(self) -> IfContentPartColouring:
        """
        Creates a copy of this colouring
        """
    def contains(self, key: str) -> bool:
        """
        Checks whether 'key' currently has a tracked state
        """
    def empty(self) -> bool:
        """
        Checks whether no keys are currently tracked
        """
    def erase(self, key: str) -> bool:
        """
        Removes the current state for 'key', if any; returns whether 'key' was actually tracked
        """
    def get(self, key: str, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the current state for 'key', or 'default' if not tracked
        """
    def getIndVals(self, key: str, filter: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex | None, str], bool] | None = None) -> list[tuple[int | None, str]]:
        """
        Retrieves both the corresponding values and the index of where the value occurs
        
        .. note::
            Unlike :meth:`getVals`, ``filter`` is only ever applied when ``key``'s state comes from the
            current :class:`IfContentPart` (a list of indexed occurrences) -- a value carried over from
            a previous part is always returned unfiltered, as ``(None, value)``.
        
        Parameters
        ----------
        key: Any
            The key to search for
        
        filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`
        
            The predicate takes in the following parameters:
        
            #. The index the value appears in the current :class:`IfContentPart`. If this argument is
               ``None``, then the value was carried over from a previous part
            #. The corresponding value
        
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        List[Tuple[Optional[:class:`int`], Any]]
            Both the values and their index within the current :class:`IfContentPart`. Empty if ``key``
            isn't tracked.
        """
    def getRanges(self, keysExists: collections.abc.Mapping[str, bool] | None = None, keyFilters: collections.abc.Mapping[str, collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex | None, str], bool]] | None = None, existsRequireAll: bool = True, filtersRequireAll: bool = True, globalRequireAll: bool = True, includeKeyDefs: bool = True) -> Ranges:
        """
        Retrieves the ranges of indices within the current part that satisfy specified conditions for each key
        
        Parameters
        ----------
        keysExists: Optional[Dict[Any, :class:`bool`]]
            Checks whether a key exists or does not exist :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the names of the registers and the values are whether to check for the
            existence/non-existence of the register :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        keyFilters: Optional[Dict[Any, Callable[[Optional[:class:`int`], Any], :class:`bool`]]]
            The conditions to satisfy for each key :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the names of the registers and the values are the predicates, taking the same
            parameters as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        existsRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy all existence checks at ``keysExists`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        filtersRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy all the predicates specified at ``keyFilters`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        globalRequireAll: :class:`bool`
            Whether the retrieved ranges must satisfy checks in both ``keysExists`` and ``keyFilters`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        includeKeyDefs: :class:`bool`
            Whether to include indices where the values for the keys specified at ``keysExists`` or
            ``keyFilters`` are being (re)defined :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        Returns
        -------
        :class:`Ranges`
            The valid ranges that satisfy the specified conditions
        """
    def getUniqueVals(self, key: str, filter: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex | None, str], bool] | None = None) -> set[str]:
        """
        Same as :meth:`getVals`, except the result is deduplicated into a real ``set`` -- a departure from
        the deprecated Python source's own ``getVals(unique=True)``, split into its own method the same
        way :class:`IfContentPart` itself splits ``getVals``/``getKeys`` rather than returning a value
        whose type depends on an argument
        
        Parameters
        ----------
        key: Any
            The key to search for
        
        filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            Same meaning as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        Set[Any]
            The resultant unique values. Empty if ``key`` isn't tracked.
        """
    def getVals(self, key: str, filter: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex | None, str], bool] | None = None) -> list[str]:
        """
        Retrieves the values for a given key, keeping duplicates and occurrence order
        
        Parameters
        ----------
        key: Any
            The key to search for
        
        filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            Same meaning as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        List[Any]
            The resultant values. Empty if ``key`` isn't tracked.
        """
    def items(self) -> list[tuple[str, typing.Any]]:
        """
        Retrieves every currently-tracked ``(key, state)`` pair, in insertion order
        """
    def keys(self) -> list[str]:
        """
        Retrieves every currently-tracked key, in insertion order
        """
    def restore(self, colourChange: collections.abc.Mapping[str, IfContentPartColourChange]) -> None:
        """
        Restores to a previous state
        
        Parameters
        ----------
        colourChange: Dict[Any, :class:`IfContentPartColourChange`]
            The change in the state, as returned by :meth:`updateColouring`
        """
    def set(self, key: str, value: typing.Any) -> None:
        """
        Sets the current state for 'key', inserting it if not already tracked
        """
    def size(self) -> int:
        """
        Retrieves the number of keys currently tracked
        """
    def updateColouring(self, ifContentPart: IfContentPart, targetKeys: collections.abc.Set[str] | None = None, updatePreviousKVPs: bool = True) -> dict[str, IfContentPartColourChange]:
        """
        Updates the current state of the `KVPs`_ based on the current :class:`IfContentPart`
        
        Parameters
        ----------
        ifContentPart: :class:`IfContentPart`
            The part to update the new `KVPs`_ from
        
        targetKeys: Optional[Set[Any]]
            The target keys to keep track of :raw-html:`<br />` :raw-html:`<br />`
        
            If this value is ``None``, then will keep track of all the keys :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        updatePreviousKVPs: :class:`bool`
            Whether to also update the `KVP`_ values from previous :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[Any, :class:`IfContentPartColourChange`]
            The change in the state :raw-html:`<br />` :raw-html:`<br />`
        
            The keys are the names of the keys and the values are the state change for the keys
        """
class IfPredParser:
    """
    
    The context-free parser used for conditional predicates within a .ini file
    
    eg.
    
    .. code-block:: ini
        :linenos:
        :emphasize-lines: 1,3
    
        if pred1
            ...
        else if pred2
            ...
        endif
    
    Parameters
    -----------
    startToken: :class:`str`
        The name of the starting token for an input string
    
        **Default**: ``STARTTOKEN``
    
    endToken: :class:`str`
        The name of the ending token for an input string
    
        **Default**: ``ENDTOKEN``
    
    nullToken: :class:`str`
        The name for the empty token
    
        **Default**: ``EPSILON``
    
    setup: :class:`bool`
        Whether to initialize all the setup for the parser automatically by calling :meth:`setup`
    
        **Default**: ``True``
        
    """
    def __init__(self, startToken: str = 'STARTTOKEN', endToken: str = 'ENDTOKEN', nullToken: str = 'EPSILON', setup: bool = True) -> None:
        ...
    def clear(self) -> None:
        """
        Clears all the setup from the parser
        """
    def getFirst(self, symbols: collections.abc.Sequence[str], nullable: collections.abc.Mapping[str, bool], first: collections.abc.Mapping[str, collections.abc.Set[str]]) -> set[str]:
        """
        Retrieves the first terminal symbols to appear given a list of symbols
        
        Parameters
        ----------
        symbols: List[:class:`str`]
            The symbols to read
        
        nullable: Dict[:class:`str`, :class:`bool`]
            The `Nullable Set`_
        
        first: Dict[:class:`str`, Set[:class:`str`]]
            The `First Set`_ for only each single non-terminal symbol
        
        Returns
        -------
        Set[:class:`str`]
            The first terminal symbols to appear given 'symbols'
        """
    def getFirstSet(self, updateNullable: bool = True) -> dict[str, set[str]]:
        """
        Computes the `First Set`_ for only each single non-terminal symbol
        
        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The first terminal symbols to appear for a non-terminal symbol
        """
    def getFollowSet(self, updateNullable: bool = True, updateFirst: bool = True) -> dict[str, set[str]]:
        """
        Computes the `Follow Set`_
        
        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_
        
            **Default**: ``True``
        
        updateFirst: :class:`bool`
            Whether to update the `First Set`_
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The `Follow Set`_
        """
    def getNonTermSymbols(self) -> set[str]:
        """
        Retrieves the set of non-terminal symbols of the `CFG`_
        
        Returns
        -------
        Set[:class:`str`]
            The set of non-terminal symbols
        """
    def getNullableSet(self) -> dict[str, bool]:
        """
        Computes the `Nullable Set`_
        
        Returns
        -------
        Dict[:class:`str`, :class:`bool`]
            Whether each non-terminal symbol is nullable
        """
    def parse(self, tokens: collections.abc.Sequence[Token], ctx: ParseContext = None) -> ParseTree:
        """
        Parses an input text
        
        Parameters
        ----------
        tokens: List[:class:`Token`]
            The tokenized tokens of the input text :raw-html:`<br />` :raw-html:`<br />`
        
            Usually obtained by running some sort of tokenizer, such as :class:`BaseTokenizer`
        
        ctx: Optional[:class:`ParseContext`]
            The context for parsing :raw-html:`<br />` :raw-html:`<br />`
        
            If this argument is ``None``, a context is constructed from the concatenation of every
            token's value
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`SyntaxErr`
            If the parse tree cannot be constructed
        
        Returns
        -------
        :class:`ParseTree`
            The constructed parse tree
        """
    def setup(self) -> None:
        """
        Initializes any necessary setup for the parser
        """
    @property
    def endToken(self) -> str:
        """
        :class:`str`: The name of the ending token for an input string
        """
    @endToken.setter
    def endToken(self, arg0: str) -> None:
        ...
    @property
    def first(self) -> dict[str, set[str]]:
        """
        Dict[:class:`str`, Set[:class:`str`]]: The `First Set`_ for only each single non-terminal symbol
        """
    @first.setter
    def first(self, arg0: collections.abc.Mapping[str, collections.abc.Set[str]]) -> None:
        ...
    @property
    def follow(self) -> dict[str, set[str]]:
        """
        Dict[:class:`str`, Set[:class:`str`]]: The `Follow Set`_
        """
    @follow.setter
    def follow(self, arg0: collections.abc.Mapping[str, collections.abc.Set[str]]) -> None:
        ...
    @property
    def nonTermSymbols(self) -> set[str]:
        """
        Set[:class:`str`]: The set of non-terminal symbols of the `CFG`_, as of the last time 'productions' was set
        """
    @property
    def nullToken(self) -> str:
        """
        :class:`str`: The name for the empty token
        """
    @nullToken.setter
    def nullToken(self, arg0: str) -> None:
        ...
    @property
    def nullable(self) -> dict[str, bool]:
        """
        Dict[:class:`str`, :class:`bool`]: The `Nullable Set`_
        
        The keys are the non-terminal symbols and the values are whether each symbol is nullable
        """
    @nullable.setter
    def nullable(self, arg0: collections.abc.Mapping[str, bool]) -> None:
        ...
    @property
    def productions(self) -> dict:
        """
        Dict[Hashable, Tuple[:class:`str`, List[:class:`str`]]]: The production rules of the `CFG`_, keyed by the id of each production rule
        """
    @property
    def startSymbol(self) -> str:
        """
        :class:`str`: The starting non-terminal symbol
        
        :getter: Retrieves the starting non-terminal symbol
        :setter: Sets the new starting non-terminal symbol
        """
    @startSymbol.setter
    def startSymbol(self, arg1: str) -> None:
        ...
    @property
    def startToken(self) -> str:
        """
        :class:`str`: The name of the starting token for an input string
        """
    @startToken.setter
    def startToken(self, arg0: str) -> None:
        ...
class IfPredPart(IfTemplatePart):
    """
    
    This class inherits from :class:`IfTemplatePart`
    
    Class for defining the predicate part of an `IfTemplate`, using a `Z3`_ predicate rather than a
    `sympy`_ query (see :attr:`query`)
    
    Parameters
    ----------
    src: :class:`str`
        The original string within the `IfTemplate`
    
    type: :class:`IfPredPartType`
        The type of predicate encountered
    
    z3Ctx: :class:`Z3Context`
        The `Z3`_ context :attr:`query` will belong to -- shared across every :class:`IfPredPart`
        constructed against the same :class:`Z3Context`, so the same-named variable across several
        predicates interns to the same `Z3`_ constant
    
    ctx: Optional[:class:`ParseContext`]
        The context for parsing the predicate, if 'type' is :attr:`IfPredPartType.If`/
        :attr:`IfPredPartType.Elif` and 'query' isn't already given :raw-html:`<br />` :raw-html:`<br />`
    
        If given, this is mutated in place (its ``lines`` replaced with :meth:`getTestStr`'s result)
        so it reflects exactly what was parsed. If ``None``, a fresh, throwaway :class:`ParseContext`
        is constructed internally instead
    
        **Default**: ``None``
    
    query: Optional[:class:`Z3Predicate`]
        The associated `Z3`_ predicate :raw-html:`<br />` :raw-html:`<br />`
    
        If this value is ``None`` and 'type' is :attr:`IfPredPartType.If`/:attr:`IfPredPartType.Elif`,
        will parse the predicate from 'src' instead (see :meth:`getLogicQuery`)
    
        **Default**: ``None``
    
    id: Optional[:class:`int`]
        The id for the part. If this parameter is ``None``, will generate a new id for the part.
    
        **Default**: ``None``
        
    """
    @staticmethod
    def getIfPredStr(predicate: Z3Predicate) -> str | None:
        """
        Generates the .ini predicate text used in the if/else-if/else parts of a .ini file for some
        already-built `Z3`_ predicate
        
        Parameters
        ----------
        predicate: :class:`Z3Predicate`
            The predicate to render
        
        Returns
        -------
        Optional[:class:`str`]
            The generated predicate text, or ``None`` if 'predicate' contains a construct with no .ini
            predicate equivalent
        """
    @staticmethod
    def getLogicQuery(ctx: ParseContext, z3Ctx: Z3Context) -> FixRaidenBoss2.core.Z3Predicate | None:
        """
        Generates the corresponding `Z3`_ predicate from a conditional predicate's source text
        
        Parameters
        ----------
        ctx: :class:`ParseContext`
            The parsing context for reading the conditional predicate
        
        z3Ctx: :class:`Z3Context`
            The `Z3`_ context the generated predicate will belong to
        
        Returns
        -------
        Optional[:class:`Z3Predicate`]
            The generated `Z3`_ predicate, or ``None`` if 'ctx' could not be tokenized/parsed/converted
        """
    @staticmethod
    def reparent(predicate: Z3Predicate, target: Z3Context) -> FixRaidenBoss2.core.Z3Predicate | None:
        """
        Rebuilds 'predicate' as an equivalent :class:`Z3Predicate` belonging to a *different*
        :class:`Z3Context` -- the only way to move a predicate across `Z3`_ contexts at all, since two
        predicates can only be combined (eg. via ``&``) when they already share the same context (see
        :class:`Z3Predicate`'s own warning)
        
        Parameters
        ----------
        predicate: :class:`Z3Predicate`
            The predicate to reparent
        
        target: :class:`Z3Context`
            The `Z3`_ context the returned predicate will belong to
        
        Returns
        -------
        Optional[:class:`Z3Predicate`]
            The reparented predicate, or ``None`` if 'predicate' contains a construct with no .ini
            predicate equivalent, or otherwise fails to re-parse against 'target'
        """
    def __copy__(self) -> IfPredPart:
        ...
    def __deepcopy__(self, arg0: dict) -> IfPredPart:
        ...
    def __init__(self, src: str, type: typing.Any, z3Ctx: Z3Context, ctx: ParseContext = None, query: FixRaidenBoss2.core.Z3Predicate | None = None, id: typing.Any = None) -> None:
        ...
    def clone(self, newId: bool = False) -> IfPredPart:
        """
        Creates a copy of this part
        
        Parameters
        ----------
        newId: :class:`bool`
            Whether to generate a new id for the part
        
            **Default**: ``False``
        
        Returns
        -------
        :class:`IfPredPart`
            The cloned part
        """
    def getTestStr(self) -> str:
        """
        Retrieves :attr:`src` with :attr:`type`'s own leading keyword (and, for
        :attr:`IfPredPartType.If`/:attr:`IfPredPartType.Elif`, a trailing ``then`` keyword) stripped --
        the actual predicate text to parse
        
        Returns
        -------
        :class:`str`
            The stripped predicate text
        """
    def toStr(self, linePrefix: str | None = None) -> str:
        """
        Retrieves the part as a string
        
        Parameters
        ----------
        linePrefix: Optional[:class:`str`]
            The string that will prefix :attr:`src` :raw-html:`<br />` :raw-html:`<br />`
        
            If ``None``, :attr:`src` is used as-is. Otherwise, any left spacing from :attr:`src` is
            stripped and 'linePrefix' is prepended instead
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`str`
            The string representation of the part
        """
    @property
    def query(self) -> FixRaidenBoss2.core.Z3Predicate | None:
        """
        Optional[:class:`Z3Predicate`]: The associated `Z3`_ predicate for this part -- ``None``
        for :attr:`IfPredPartType.EndIf`, or when parsing 'src' failed
        """
    @query.setter
    def query(self, arg0: FixRaidenBoss2.core.Z3Predicate | None) -> None:
        ...
    @property
    def src(self) -> str:
        """
        :class:`str`: The original string within the `IfTemplate`
        """
    @src.setter
    def src(self, arg0: str) -> None:
        ...
    @property
    def type(self) -> typing.Any:
        """
        :class:`IfPredPartType`: The type of predicate encountered
        """
    @type.setter
    def type(self, arg1: typing.Any) -> None:
        ...
class IfPredTokenizer(FilteredTokenizer):
    """
    
    This class inherits from :class:`FilteredTokenizer`
    
    The tokenizer used for conditional predicates within a .ini file
    
    eg.
    
    .. code-block:: ini
        :linenos:
        :emphasize-lines: 1,3
    
        if pred1
            ...
        else if pred2
            ...
        endif
    
    Parameters
    ----------
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, setup: bool = True) -> None:
        ...
class IfTemplate:
    """
    
    Data for storing information about a `section`_ in a .ini file
    
    .. note::
        Assuming every ``if``/``else`` clause must be on its own line, we have that an
        :class:`IfTemplate` has a form looking similar to this:
    
        .. code-block:: ini
            :linenos:
            :emphasize-lines: 1,2,5,7,12,16,17
    
            ...(does stuff)...
            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                endif
            endif
            ...(does stuff)...
            ...(does stuff)...
    
        We split the above structure into parts (:class:`IfTemplatePart`) where each part is either:
    
        #. **An If Predicate Part** (:class:`IfPredPart`): a single line containing the keywords ``if``, ``else`` or ``endif`` :raw-html:`<br />` **OR** :raw-html:`<br />`
        #. **A Content Part** (:class:`IfContentPart`): a group of lines that *"does stuff"*
    
        **Note that:** an :class:`IfTemplate` does not need to contain any parts containing the
        keywords ``if``, ``else`` or ``endif``. This case covers the scenario when the user does not
        use if/else statements for a particular `section`_.
    
        Based on the above assumptions, we can assume that every ``[section]`` in a .ini file contains
        this :class:`IfTemplate`.
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: for element in x
    
            Iterates over all the parts of the :class:`IfTemplate`, ``x``
    
        .. describe:: len(x)
    
            Retrieves the number of parts
    
        .. describe:: x[num]
    
            Retrieves the part at index ``num`` (``None`` if that slot has been cleared)
    
        .. describe:: x[num] = newPart
    
            Sets the part at index ``num`` -- pass ``None`` to clear the slot
    
    Parameters
    ----------
    parts: List[Optional[:class:`IfTemplatePart`]]
        The individual parts of the :class:`IfTemplate` -- ownership of each is taken from the passed-in
        Python objects (the same contract as :class:`IfContentPart`'s own ``content`` parameter)
    
    name: :class:`str`
        The name of the `section`_. **Default**: ``""``
    
    prefix: :class:`str`
        Any prefix that precedes the content. **Default**: ``""``
    
    suffix: :class:`str`
        Any suffix that follows the content. **Default**: ``""``
        
    """
    @staticmethod
    def build(rawParts: list, name: str = '', ctx: typing.Any = None, z3Ctx: typing.Any = None) -> IfTemplate:
        """
        Builds the :class:`IfTemplate`
        
        Parameters
        ----------
        rawParts: List[Tuple[:class:`int`, Union[:class:`str`, Dict[Any, List[Tuple[:class:`int`, Any]]]]]]
            The list of raw parts found -- each tuple is a starting line number paired with either a
            conditional-predicate string (parsed via :class:`IfPredPart`) or an :class:`IfContentPart`-shaped
            ``src`` dict
        
        name: :class:`str`
            The name of the `section`_. **Default**: ``""``
        
        ctx: Optional[:class:`ParseContext`]
            The context for parsing conditional predicates. **Default**: ``None``
        
        z3Ctx: Optional[:class:`Z3Context`]
            The `Z3`_ context every parsed :class:`IfPredPart` will share. **Default**: ``None``
        """
    def __copy__(self) -> IfTemplate:
        ...
    def __deepcopy__(self, memo: dict) -> IfTemplate:
        ...
    def __getitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> typing.Any:
        ...
    def __init__(self, parts: typing.Any, name: str = '', prefix: str = '', suffix: str = '') -> None:
        ...
    def __iter__(self) -> collections.abc.Iterator:
        ...
    def __len__(self) -> int:
        ...
    def __setitem__(self, arg0: typing.SupportsInt | typing.SupportsIndex, arg1: typing.Any) -> None:
        ...
    def add(self, part: typing.Any, updateTree: bool = False) -> typing.Any:
        """
        Adds a part to the :class:`IfTemplate` -- ownership of 'part' is taken (same contract as the
        constructor's own ``parts``)
        
        Parameters
        ----------
        part: :class:`IfTemplatePart`
            The part to add
        
        updateTree: :class:`bool`
            Whether to update the parse tree. **Default**: ``False``
        """
    def addBottomContentPart(self) -> IfContentPart:
        """
        Adds a new :class:`IfContentPart` at the very end of this :class:`IfTemplate`, if needed
        """
    def addKVPToBack(self, key: str, val: str) -> None:
        """
        Adds a KVP to the bottom of this :class:`IfTemplate`
        """
    def addKVPToFront(self, key: str, val: str) -> None:
        """
        Adds a KVP to the top of this :class:`IfTemplate`
        """
    def addKVPsToBack(self, kvps: collections.abc.Sequence[tuple[str, str]]) -> None:
        """
        Adds some KVPs to the bottom of this :class:`IfTemplate`
        """
    def addKVPsToFront(self, kvps: collections.abc.Sequence[tuple[str, str]]) -> None:
        """
        Adds some KVPs to the top of this :class:`IfTemplate`
        """
    def addTopContentPart(self) -> IfContentPart:
        """
        Adds a new :class:`IfContentPart` at the root of this :class:`IfTemplate`, if needed
        """
    def deepcopy(self, newPartIds: bool = True) -> IfTemplate:
        """
        Performs a deep copy on the object
        
        Parameters
        ----------
        newPartIds: :class:`bool`
            Whether to refresh the ids for each part. **Default**: ``True``
        
        Returns
        -------
        :class:`IfTemplate`
            The copied object
        """
    def find(self, pred: typing.Any = None, postProcessor: typing.Any = None) -> dict:
        """
        Searches the :class:`IfTemplate` for parts that meet a certain condition
        
        Parameters
        ----------
        pred: Optional[Callable[[:class:`IfTemplate`, :class:`int`, :class:`IfTemplatePart`], :class:`bool`]]
            The predicate used to filter the parts. If ``None``, every part matches. **Default**: ``None``
        
        postProcessor: Optional[Callable[[:class:`IfTemplate`, :class:`int`, :class:`IfTemplatePart`], Any]]
            Post-processes each matching part. If ``None``, returns the found part itself. **Default**: ``None``
        
        Returns
        -------
        Dict[:class:`int`, Any]
            The filtered parts, keyed by their index
        """
    def normalize(self) -> None:
        """
        Normalizes the branching structure within this :class:`IfTemplate` to follow :class:`IfTemplateNormTree`'s structure
        """
    def rebuild(self) -> None:
        """
        Updates the parse tree and the reference to other sections that this object calls
        """
    def refreshPartIds(self) -> None:
        """
        Regenerates the ids for the parts
        """
    def toStr(self, linePrefix: str = '', autoindent: bool = True) -> str:
        """
        Converts this :class:`IfTemplate` to a string
        
        Parameters
        ----------
        linePrefix: :class:`str`
            The string that will prefix every line. **Default**: ``""``
        
        autoindent: :class:`bool`
            Whether to compute the proper tab indent. **Default**: ``True``
        
        Returns
        -------
        :class:`str`
            The string representation
        """
    @property
    def calledSubCommands(self) -> dict:
        """
        Dict[:class:`int`, List[:class:`str`]]: Any other `sections`_ this :class:`IfTemplate` references, by ``run =``
        """
    @property
    def name(self) -> str:
        """
        :class:`str`: The name of the `section`_
        """
    @name.setter
    def name(self, arg0: str) -> None:
        ...
    @property
    def parts(self) -> list:
        """
        List[Optional[:class:`IfTemplatePart`]]: The individual parts -- reassigning this does not itself update :attr:`tree`/:attr:`partsById`/:attr:`calledSubCommands`; call :meth:`rebuild` afterward if needed
        """
    @parts.setter
    def parts(self, arg1: typing.Any) -> None:
        ...
    @property
    def partsById(self) -> dict:
        """
        Dict[:class:`int`, :class:`IfTemplatePart`]: The parts, keyed by their id
        """
    @property
    def prefix(self) -> str:
        """
        :class:`str`: Any prefix that precedes the content
        """
    @prefix.setter
    def prefix(self, arg0: str) -> None:
        ...
    @property
    def suffix(self) -> str:
        """
        :class:`str`: Any suffix that follows the content
        """
    @suffix.setter
    def suffix(self, arg0: str) -> None:
        ...
    @property
    def tree(self) -> IfTemplateTree:
        """
        :class:`IfTemplateTree`: The parse tree for this :class:`IfTemplate`
        """
class IfTemplateNode:
    """
    
    A node within the parse tree of some :class:`IfTemplate`. This node contains a subset of the
    :class:`IfContentPart`\\s from the original :class:`IfTemplate`
    
    .. note::
        For more details on the structure of the parse tree of an :class:`IfTemplate`, see
        :class:`IfTemplateTree`
    
    .. warning::
        This node does not own the :class:`IfContentPart`/:class:`IfPredPart` instances referenced
        from :attr:`parts`/:attr:`ifPredPart` -- see this class's own binding header comment for the
        full lifetime contract. In short: keep whichever :class:`IfTemplate` this node's tree belongs
        to alive for as long as any reference obtained from this node is in use.
    
    Parameters
    ----------
    id: Optional[:class:`int`]
        The id for the node :raw-html:`<br />` :raw-html:`<br />`
    
        If this argument is ``None``, then will generate the id for the node
    
        **Default**: ``None``
    
    ifPredPart: Optional[:class:`IfPredPart`]
        The predicate part that is associated with this node -- stored by reference, not copied; kept
        alive at least as long as this node (see this class's own top-level note)
        :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, id: typing.Any = None, ifPredPart: typing.Any = None) -> None:
        ...
    def addChild(self, node: IfTemplateNode) -> None:
        """
        Adds a child to the node -- stored by reference, not copied; kept alive at least as long as this
        node (see this class's own top-level note)
        
        Parameters
        ----------
        node: :class:`IfTemplateNode`
            The child to be added
        """
    def addIfContentPart(self, part: IfContentPart) -> None:
        """
        Adds an :class:`IfContentPart` to the node -- stored by reference, not copied; kept alive at least
        as long as this node (see this class's own top-level note)
        
        Parameters
        ----------
        part: :class:`IfContentPart`
            The content part of the :class:`IfTemplate` to add to this node
        """
    def getKeyMissingPart(self, key: str) -> tuple[typing.Any, bool]:
        """
        Retrieves the first :class:`IfContentPart` if 'key' is not found in this node, without accounting
        for the key being in any other subcommands or other children nodes
        
        Parameters
        ----------
        key: :class:`str`
            The key to find
        
        Returns
        -------
        Tuple[Optional[:class:`IfContentPart`], :class:`bool`]
            A tuple containing:
        
            #. The first part found, if all the :class:`IfContentPart`\\s within the node does not contain the key
            #. Whether a :class:`IfContentPart` is found within the node
        """
    def getKeyPart(self, key: str) -> IfContentPart:
        """
        Retrieves the latest :class:`IfContentPart` that contains 'key'
        
        Parameters
        ----------
        key: :class:`str`
            The key to find
        
        Returns
        -------
        Optional[:class:`IfContentPart`]
            The found part if available
        """
    def getKeyVal(self, key: str) -> typing.Any:
        """
        Retrieves the latest value that corresponds to 'key'
        
        Parameters
        ----------
        key: :class:`str`
            The key to find
        
        Returns
        -------
        Optional[:class:`str`]
            The found value if available
        """
    def getKeyValues(self, key: str) -> list[list[tuple[int, str]]]:
        """
        Retrieves all the corresponding values to a certain key within the node
        
        Parameters
        ----------
        key: :class:`str`
            The key to find
        
        Returns
        -------
        List[List[Tuple[:class:`int`, :class:`str`]]]
            All the corresponding values to the key in the node :raw-html:`<br />` :raw-html:`<br />`
        
            * The outer elements in the list are the values for each part in the node
            * The inner elements of the list are the different instance of the `KVP`_ within each part
            * The tuple contains the order index an occurence of the `KVP`_ appears in the part and the corresponding value for the `KVP`_
        """
    def hasKey(self, key: str) -> bool:
        """
        Purely checks whether the key exists within the parts of the node without accounting for whether
        the key exists in other subcommands called by this node or other children nodes that have the key
        
        Parameters
        ----------
        key: :class:`str`
            The key to check
        
        Returns
        -------
        :class:`bool`
            Whether the key exists
        """
    @property
    def children(self) -> dict:
        """
        Dict[:class:`int`, :class:`IfTemplateNode`]: The children to this node -- the keys are the ids of the children nodes and the values are the corresponding nodes for the children
        """
    @property
    def id(self) -> int:
        """
        Hashable: The id for the node
        """
    @property
    def ifPredPart(self) -> IfPredPart:
        """
        Optional[:class:`IfPredPart`]: The predicate part that is associated with this node
        """
    @property
    def parts(self) -> list:
        """
        List[Union[:class:`IfContentPart`, :class:`IfTemplateNode`]]: The parts of the :class:`IfTemplate` within the node
        """
class IfTemplatePart:
    """
    
    Base class for some part in an `IfTemplate`
    
    Parameters
    ----------
    id: Optional[:class:`int`]
        The id for the part. If this parameter is ``None``, will generate a new id for the part.
    
        **Default**: ``None``
            
    """
    def __init__(self, id: typing.Any = None) -> None:
        ...
    def refreshId(self) -> int:
        """
        Regenerates the id for the part
        
        Returns
        -------
        :class:`int`
            The newly generated id
        """
    @property
    def id(self) -> int:
        """
        :class:`int`: The id for the part
        """
class IfTemplateTree:
    """
    
    The parse tree for some :class:`IfTemplate`, reached via :attr:`IfTemplate.tree` -- never
    constructed directly.
    
    .. note::
        The parse tree is structured such that:
    
        * A node is composed of :class:`IfContentPart`\\s or other nodes
        * The children to the node occur when the node enters a specific branching condition :raw-html:`<br />` :raw-html:`<br />`
    
        eg. Suppose we have this branching structure
    
        .. code-block:: ini
            :linenos:
    
            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                ...(does stuff)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                    ...(does stuff)...
                endif
                ...(does stuff)...
                if
                endif
            endif
            ...(does stuff)...
    
        :raw-html:`<br />`
    
        Let ``C`` be some :class:`IfContentPart` (the parts that say ``...(does stuff)...``)
    
        Let ``B`` be some branching point (the parts that say ``if`` or ``else``)
    
        Let ``[...]`` be some node
    
        Let ``X`` be a node without any parts
    
        The parse tree generated for the above code would be:
    
        .. code-block::
    
                   [C B B C]
                      | |
                 +----+ +----+
                 |           |
               [B B]     [C B C B]
                | |         |   |
             +--+ +--+    [B C] X
             |       |     |
            [C]     [C]   [C]
    
    .. note::
        A leaf node with no parts at all (the ``X`` above -- an empty condition, eg. a bare
        ``if``/``endif`` with nothing between them) only ever shows up in a tree built for
        :meth:`IfTemplate.add`'s own bookkeeping. Every :attr:`IfTemplate.tree` a real caller sees is
        always built one of two other ways instead, and the difference matters if you're inspecting
        :attr:`root`/:attr:`IfTemplateNode.parts` directly:
    
        * **By default** (how :attr:`IfTemplate.tree` is built by the constructor): an otherwise-empty
          leaf node gets one synthetic, empty :class:`IfContentPart` placeholder instead of staying
          empty -- so
    
          .. code-block:: ini
    
              if
              endif
    
          (parse subtree ``[B]`` -> ``X``) becomes, for tree-building purposes, equivalent to
    
          .. code-block:: ini
    
              if
                  ...(does nothing)...
              endif
    
          (parse subtree ``[B]`` -> ``[C]``) -- every leaf in the worked example above that would
          otherwise be an empty ``X`` node picks up this placeholder instead, eg. the ``if`` with
          nothing in it right before the final ``endif``.
        * **After calling** :meth:`IfTemplate.normalize` **specifically**: on top of the placeholder
          behavior above, an empty ``else`` clause is also synthesized for any conditional that
          doesn't already end with a single ``else`` -- so
    
          .. code-block:: ini
    
              if
                  ...(does stuff)...
              else if
                  ...(does stuff)...
              endif
    
          (parse subtree ``[B B]`` with two leaf children) becomes
    
          .. code-block:: ini
    
              if
                  ...(does stuff)...
              else if
                  ...(does stuff)...
              else
                  ...(does nothing)...
              endif
    
          (parse subtree ``[B B B]`` with three leaf children -- the new ``else`` picking up the same
          empty-placeholder treatment as above). Applied to the whole worked example above, every
          ``if``/``elif`` chain that doesn't already end with a plain ``else`` gains one, including
          the already-empty ``if``/``endif`` at the bottom (which ends up with *two* leaf children --
          one placeholder for the original empty ``if`` branch, one for its synthesized ``else``).
    
    .. note::
        The nodes are the `IfContentPart`\\s of the `IfTemplate`, wrapped in :class:`IfTemplateNode`\\s
        forming the tree structure. See :class:`IfTemplateNode` for the parts/children each node holds.
        
    """
    def clear(self) -> None:
        """
        Clears the tree
        """
    @property
    def root(self) -> IfTemplateNode:
        """
        Optional[:class:`IfTemplateNode`]: The root node in the parse tree
        """
class Indices(ModMappedAssets):
    """
    
    This class inherits from :class:`ModMappedAssets`
    
    Class for managing indices for a mod, pre-populated with this project's real index data
    
    :raw-html:`<br />`
    
    .. note::
        Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/
        ``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:
    
        * version (version index)
        * name
        * component
        * type
        
    """
    def __init__(self, map: typing.Any = None) -> None:
        """
        Constructs a new, fully-populated index lookup table
        
        Parameters
        ----------
        map: Optional[Dict[Any, List[Any]]]
            The `adjacency list`_ that maps the indices to fix from to the indices to fix to using the
            predefined mods
        
            **Default**: ``None``
        """
class IniClassifier(BaseIniClassifier):
    """
    
    This class inherits from :class:`BaseIniClassifier`
    
    Class to help classify the type of mod given the mod's .ini files
    
    Parameters
    ----------
    checkHasTextureOverride: :class:`bool`
        Whether :meth:`addGIModType`/section-name reading should require a section name to start with
        ``TextureOverride`` before doing anything else with it
    
        **Default**: ``True``
        
    """
    def __init__(self, checkHasTextureOverride: bool = True) -> None:
        ...
    def addGIModType(self, modType: ModTypeIdData, hashes: collections.abc.Set[str], sectionKeywords: collections.abc.Set[str]) -> bool:
        """
        Registers a GI mod type into the classifier
        
        Fails (returns ``False``) without registering anything if ``modType.modTypeId`` is already
        registered, or if ``modType.gameTypeId`` isn't :attr:`GameTypeId.GI`
        
        Parameters
        ----------
        modType: :class:`ModTypeIdData`
            The mod type to register
        
        hashes: Set[:class:`str`]
            The hashes that identify 'modType'
        
        sectionKeywords: Set[:class:`str`]
            The section keywords that identify 'modType'
        
        Returns
        -------
        :class:`bool`
            Whether 'modType' was newly registered
        """
    def addWuWaModType(self, modType: ModTypeIdData, hashes: collections.abc.Set[str]) -> bool:
        """
        Registers a WuWa mod type into the classifier
        
        Fails (returns ``False``) without registering anything if ``modType.modTypeId`` is already
        registered, or if ``modType.gameTypeId`` isn't :attr:`GameTypeId.WuWa`
        
        Parameters
        ----------
        modType: :class:`ModTypeIdData`
            The mod type to register
        
        hashes: Set[:class:`str`]
            The hashes that identify 'modType'
        
        Returns
        -------
        :class:`bool`
            Whether 'modType' was newly registered
        """
    def getModType(self, modTypeId: typing.SupportsInt | typing.SupportsIndex) -> ModTypeIdData:
        """
        Retrieves the registered :class:`ModTypeIdData` for a :class:`ModTypeId`
        
        Parameters
        ----------
        modTypeId: :class:`int`
            The id for the :class:`ModTypeId` to retrieve the :class:`ModTypeIdData` for
        
        Raises
        ------
        IndexError
            Raised if 'modTypeId' is not registered
        
        Returns
        -------
        :class:`ModTypeIdData`
            The corresponding :class:`ModTypeIdData`
        """
class IniClassifyStats:
    """
    
    Stores the statistics about the classification result of a .ini file
    
    Parameters
    ----------
    modType: Dict[:class:`int`, :class:`ModTypeIdData`]
        The types of mod found, keyed by their id
    
        **Default**: ``{}``
    
    isMod: :class:`bool`
        Whether the .ini file belongs to a mod
    
        **Default**: ``False``
    
    isFixed: :class:`bool`
        Whether the .ini file is fixed
    
        **Default**: ``False``
        
    """
    def __init__(self, modType: dict = {}, isMod: bool = False, isFixed: bool = False) -> None:
        ...
    @property
    def isFixed(self) -> bool:
        """
        :class:`bool`: Whether the .ini file is fixed
        """
    @isFixed.setter
    def isFixed(self, arg0: bool) -> None:
        ...
    @property
    def isMod(self) -> bool:
        """
        :class:`bool`: Whether the .ini file belongs to a mod
        """
    @isMod.setter
    def isMod(self, arg0: bool) -> None:
        ...
    @property
    def modType(self) -> dict:
        """
        Dict[:class:`int`, :class:`ModTypeIdData`]: The types of mod found, keyed by their id
        """
    @modType.setter
    def modType(self, arg1: dict) -> None:
        ...
class IniDownloadModel(IniSrcResourceModel):
    """
    
    This class inherits from :class:`IniSrcResourceModel`
    
    Contains data about a particular resource to download in the original .ini file
        
    """
    def __init__(self, iniFolderPath: str, paths: dict, downloads: dict) -> None:
        """
        Constructs new data for a resource to download
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The folder path to where the .ini file of the resource is located
        
        paths: Dict[:class:`int`, List[:class:`str`]]
            See :class:`IniSrcResourceModel`'s constructor
        
        downloads: Dict[:class:`int`, List[:class:`FileDownload`]]
            The downloader associated with each file -- the keys are the indices to the
            :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some
            resource, and the values are the downloaders for the files within that :class:`IfContentPart`.
            Ownership of each downloader is transferred into this model
        """
    @property
    def downloads(self) -> dict:
        """
        Dict[:class:`int`, List[:class:`FileDownload`]]: The downloader associated with each file
        """
    @downloads.setter
    def downloads(self, arg1: dict) -> None:
        ...
class IniFile:
    """
    
    Class for handling .ini files -- the C++-backed counterpart to the pure-Python :class:`IniFile`
    :raw-html:`<br />` :raw-html:`<br />`
    
    .. note::
        Mod types cross this boundary as **ids**, not as pure-Python :class:`ModType` objects: this
        class resolves a mod type's parse/fix/remove builders through the global registry keyed by
        ``modTypeId``, or through whatever ``overrideModTypes`` files under that id. See
        :class:`ModType`
    
    Parameters
    ----------
    file: Optional[:class:`str`]
        The file path to the .ini file. If ``None``, this object is backed only by 'txt' and never
        touches the disk :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    txt: :class:`str`
        The text content of the .ini file, used when 'file' is ``None`` :raw-html:`<br />`
        :raw-html:`<br />`
    
        **Default**: ``""``
    
    gameTypeId: Optional[:class:`int`]
        The id for the game the .ini file's mod belongs to :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    filteredFromModTypeIds: Optional[Set[:class:`int`]]
        The ids of the only mod types this .ini file is allowed to be classified as :raw-html:`<br />`
        :raw-html:`<br />`
    
        **Default**: ``None``, meaning no filter
    
    forcedFromModTypeIds: Optional[Set[:class:`int`]]
        The ids of the mod types this .ini file is classified as regardless of what its content says
        :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    overrideModTypes: Optional[Dict[:class:`int`, :class:`ModType`]]
        Mod types to resolve by id ahead of the global registry :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    iniClassifier: Optional[:class:`BaseIniClassifier`]
        The strategy used to classify the .ini file :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, meaning the global default classifier
    
    downloadMode: Optional[:class:`DownloadMode`]
        How file downloads referenced by the .ini file are handled :raw-html:`<br />`
        :raw-html:`<br />`
    
        **Default**: ``None``, meaning :attr:`DownloadMode.Normal`
    
    fromVersion: Optional[:class:`CppVersion`]
        The version of the mod being fixed :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    toVersion: Optional[:class:`CppVersion`]
        The version of the mod being fixed to :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    filteredToModTypeIds: Optional[Set[:class:`int`]]
        The ids of the only mod types this .ini file is allowed to be remapped onto :raw-html:`<br />`
        :raw-html:`<br />`
    
        **Default**: ``None``, meaning no filter
        
    """
    @staticmethod
    def getSectionNameFromLine(line: str) -> str:
        """
        Retrieves the name of the `section`_ a line declares
        
        Parameters
        ----------
        line: :class:`str`
            The line to read the name out of
        
        Returns
        -------
        :class:`str`
            The name of the `section`_
        """
    @staticmethod
    def isSectionHeaderLine(line: str) -> bool:
        """
        Determines whether a line of text declares a `section`_
        
        Parameters
        ----------
        line: :class:`str`
            The line to check
        
        Returns
        -------
        :class:`bool`
            Whether the line declares a `section`_
        """
    def __init__(self, file: str | None = None, txt: str = '', gameTypeId: typing.SupportsInt | typing.SupportsIndex | None = None, filteredFromModTypeIds: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex] | None = None, forcedFromModTypeIds: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex] | None = None, overrideModTypes: collections.abc.Mapping[typing.SupportsInt | typing.SupportsIndex, ModType] | None = None, iniClassifier: BaseIniClassifier = None, downloadMode: typing.Any = None, fromVersion: FixRaidenBoss2.core.CppVersion | None = None, toVersion: FixRaidenBoss2.core.CppVersion | None = None, filteredToModTypeIds: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex] | None = None) -> None:
        ...
    def classify(self) -> None:
        """
        Classifies the .ini file -- whether it belongs to a mod, which types of mod, and whether it has
        already been fixed :raw-html:`<br />` :raw-html:`<br />`
        
        The results are read back off :attr:`IniFile.isModIni`, :attr:`IniFile.availableType` and
        :attr:`IniFile.isFixed`
        """
    def clear(self, eraseSourceTxt: bool = False) -> None:
        """
        Clears all the saved data for the .ini file -- everything :meth:`clearRead` and
        :meth:`clearModels` clear, plus the classification results and parsed `sections`_
        
        Parameters
        ----------
        eraseSourceTxt: :class:`bool`
            See :meth:`clearRead` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        """
    def clearModels(self) -> None:
        """
        Clears every resource model built for the .ini file, without clearing the text read in from it
        
        .. note::
            To clear the read text instead, see :meth:`clearRead`
        """
    def clearRead(self, eraseSourceTxt: bool = False) -> None:
        """
        Clears the text data read in from the .ini file
        
        Parameters
        ----------
        eraseSourceTxt: :class:`bool`
            Whether to also erase the text this object was constructed with, when
            :attr:`IniFile.file` is ``None`` and that text is its only data source :raw-html:`<br />`
            :raw-html:`<br />`
        
            **Default**: ``False``
        """
    def disableIni(self, makeCopy: bool = False) -> str | None:
        """
        Disables the .ini file by renaming it so the mod loader stops reading it
        
        Parameters
        ----------
        makeCopy: :class:`bool`
            Whether to keep a copy of the file at its original path :raw-html:`<br />`
            :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        Optional[:class:`str`]
            The path the file was moved to, or ``None`` if there was no file to disable
        """
    def fix(self, keepBackup: bool = True, fixOnly: bool = False, hideOrig: bool = False) -> dict[str, str]:
        """
        Fixes the .ini file, running one fixer per mod type the file was classified as
        
        Parameters
        ----------
        keepBackup: :class:`bool`
            Whether to keep a backup copy of the original .ini file :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        fixOnly: :class:`bool`
            Whether to only fix the .ini file without removing any previous fix :raw-html:`<br />`
            :raw-html:`<br />`
        
            **Default**: ``False``
        
        hideOrig: :class:`bool`
            Whether to comment out the `sections`_ the fix touched, so only the remapped mod displays
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        Dict[:class:`str`, :class:`str`]
            The fix produced for each .ini file written, keyed by file path
        """
    def getFileDownloads(self) -> list[IniResource]:
        """
        Retrieves every file download the .ini file references
        
        .. danger::
            Same ownership caveat as :meth:`getResources`
        
        Returns
        -------
        List[:class:`IniResource`]
            The file downloads
        """
    def getIfTemplates(self, flush: bool = False) -> dict:
        """
        Retrieves every parsed `section`_ of the .ini file, keyed by section name
        
        .. danger::
            The returned sections are owned by this .ini file. :meth:`clear` destroys them
        
        Parameters
        ----------
        flush: :class:`bool`
            Whether to re-read the sections rather than reuse what was already parsed :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        Dict[:class:`str`, :class:`IfTemplate`]
            The sections, in the order they appear in the file
        """
    def getModTypes(self) -> dict:
        """
        Retrieves the mod types the .ini file was classified as, keyed by :class:`ModTypeId` value
        
        Returns
        -------
        Dict[:class:`int`, :class:`ModType`]
            The mod types, in the order they were classified
        """
    def getReferencedFolders(self) -> list[str]:
        """
        Retrieves all the folders referenced by the .ini file, in the order first seen
        
        The parent folder of each resource's source path, across both :meth:`getResources` and
        :meth:`getFileDownloads`, plus the parent folder of the fixed path of every one of those that is
        an :class:`IniFixResource`
        
        .. note::
            That second half is a deliberate divergence from the pure-Python original, whose own
            ``getReferencedFolders()`` only ever looked at a resource's *source* side. The fix **writes**
            files to a fixed path, so a folder walk built on this method has to be able to reach that
            folder even when no source path points into it
        
        Returns
        -------
        List[:class:`str`]
            The absolute paths to all the folders
        """
    def getResources(self) -> list[IniResource]:
        """
        Retrieves every resource the .ini file references
        
        .. danger::
            The returned resources are owned by this .ini file. :meth:`clear` and :meth:`clearModels`
            destroy them
        
        Returns
        -------
        List[:class:`IniResource`]
            The resources
        """
    def getSection(self, name: str) -> IfTemplate:
        """
        Retrieves a `section`_ of the .ini file by name
        
        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_
        
        Returns
        -------
        Optional[:class:`IfTemplate`]
            The `section`_, or ``None`` if no `section`_ goes by that name
        """
    def getSectionNames(self) -> list[str]:
        """
        Retrieves the names of all the `sections`_ in the .ini file, in the order they are declared
        
        Returns
        -------
        List[:class:`str`]
            The names of the `sections`_
        """
    def parse(self, flushIfTemplates: bool = True) -> None:
        """
        Parses the .ini file, building up the resources it references
        
        .. note::
            The parsed graph groups themselves stay on the C++ side. Read the results off
            :meth:`getResources` / :meth:`getFileDownloads`
        
        Parameters
        ----------
        flushIfTemplates: :class:`bool`
            Whether to re-read the `sections`_ of the .ini file :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        """
    def readFileLines(self) -> list[str]:
        """
        Reads the text lines of the .ini file off disk
        
        Returns
        -------
        List[:class:`str`]
            The text lines read, each keeping its own line ending
        """
    def removeFix(self, parse: bool = False, writeBack: bool = True, readAllIni: bool = False) -> str:
        """
        Removes a previous fix from the .ini file
        
        Parameters
        ----------
        parse: :class:`bool`
            Whether to parse the .ini file first :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        writeBack: :class:`bool`
            Whether to write the result back out to disk :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        readAllIni: :class:`bool`
            Whether the caller is removing the fix from every .ini file it encountered rather than only the
            ones it could recognize -- :attr:`RemapService.readAllInis` / the script's ``--all`` flag
            :raw-html:`<br />` :raw-html:`<br />`
        
            A .ini file that belongs to a mod but was not attributed to any type of mod is swept by a
            :class:`GlobalRemapIniRemover` when this is set, and by the ordinary remover when it is not. It does
            not decide *whether* the fix is removed :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        :class:`str`
            The text of the .ini file with the fix removed
        """
    def removeSection(self, name: str) -> None:
        """
        Removes a `section`_ from the .ini file, keeping the declaration order of the rest
        
        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_ to remove
        """
    def setFileTxt(self, txt: str) -> None:
        """
        Replaces the .ini file's text content, without touching the file on disk
        
        Parameters
        ----------
        txt: :class:`str`
            The new text content
        """
    def write(self, txt: str | None = None) -> str:
        """
        Writes text back out to the .ini file
        
        Parameters
        ----------
        txt: Optional[:class:`str`]
            The text to write. If ``None``, writes this object's current
            :attr:`IniFile.fileTxt` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`str`
            The text that was written
        """
    @property
    def availableType(self) -> ModType:
        """
        Optional[:class:`ModType`]: The type of mod the .ini file was classified as, or ``None`` if it
        was not classified as any
        
        .. warning::
            A .ini file can classify as **more than one** mod type, and this answers with whichever comes
            first in :meth:`getModTypes`' insertion order. Use :meth:`getModTypes` whenever "all of them" is
            the right question -- which it usually is. This exists for the callers that genuinely want a
            single mod type
        """
    @property
    def defaultModTypeIds(self) -> list:
        """
        List[:class:`int`]: The :class:`ModTypeId` values to fall back on when the classifier recognises nothing
        
        In play in exactly one situation: :meth:`classify` ran the classifier and it recognised **no** mod
        type at all, in which case :meth:`getModTypes` is built from these ids instead. Deliberately not in
        play when ``forcedFromModTypeIds`` was given (the classifier is never consulted for mod types there),
        nor when the classifier *did* recognise a mod type that ``filteredFromModTypeIds`` then rejected
        (that would quietly undo the caller's own filter)
        
        Also stops :meth:`classify` forcing :attr:`isModIni` false when a mod-type filter was given and
        nothing survived it
        
        Reads back as a **list**, not a set, because the order is meaningful -- it is the order the fallback
        mod types land in :meth:`getModTypes`, which :meth:`fix` walks. Accepts any iterable when set
        """
    @defaultModTypeIds.setter
    def defaultModTypeIds(self, arg1: typing.Any) -> None:
        ...
    @property
    def downloadMode(self) -> str:
        """
        :class:`str`: How the .ini file's referenced downloads are handled
        
        Reads back as the :class:`DownloadMode` string value (``"normal"``, ``"disabled"``, ``"always"``);
        accepts either a :class:`DownloadMode` or its value when set
        """
    @downloadMode.setter
    def downloadMode(self, arg1: typing.Any) -> None:
        ...
    @property
    def file(self) -> str | None:
        """
        Optional[:class:`str`]: The file path to the .ini file, or ``None`` if this object is backed only
        by its text
        """
    @property
    def fileLines(self) -> list[str]:
        """
        List[:class:`str`]: The text lines of the .ini file, each keeping its own line ending
        """
    @property
    def fileLinesRead(self) -> bool:
        """
        :class:`bool`: Whether the .ini file has been read
        """
    @property
    def fileTxt(self) -> str:
        """
        :class:`str`: The text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`
        
        Setting this re-reads the text lines from the new value and marks the file as not fixed
        """
    @fileTxt.setter
    def fileTxt(self, arg1: str) -> None:
        ...
    @property
    def filteredToModTypeIds(self) -> set[int] | None:
        """
        Optional[Set[:class:`int`]]: Only fix to these mod types, by :class:`ModTypeId` value
        
        ``None`` means no filter -- an **empty set** is deliberately different, and selects nothing
        """
    @filteredToModTypeIds.setter
    def filteredToModTypeIds(self, arg0: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex] | None) -> None:
        ...
    @property
    def folder(self) -> str:
        """
        :class:`str`: The folder the .ini file resides in, or ``""`` when it has no path
        
        .. note::
            This deliberately differs from the pure-Python :attr:`IniFile.folder`, which falls back to the
            folder the script is run from. Derived from :attr:`IniFile.file` rather than stored
        """
    @property
    def fromVersion(self) -> FixRaidenBoss2.core.CppVersion | None:
        """
        Optional[:class:`CppVersion`]: The game version the .ini file originates from
        
        Accepts a :class:`str`, :class:`int`, :class:`float` or :class:`CppVersion` when set
        """
    @fromVersion.setter
    def fromVersion(self, arg1: typing.Any) -> None:
        ...
    @property
    def isClassified(self) -> bool:
        """
        :class:`bool`: Whether the type of mod has already been identified for the .ini file
        """
    @property
    def isFixed(self) -> bool:
        """
        :class:`bool`: Whether the .ini file has already been fixed
        """
    @isFixed.setter
    def isFixed(self, arg1: bool) -> None:
        ...
    @property
    def isModIni(self) -> bool:
        """
        :class:`bool`: Whether the .ini file belongs to a mod -- the result of :meth:`classify`
        """
    @property
    def toVersion(self) -> FixRaidenBoss2.core.CppVersion | None:
        """
        Optional[:class:`CppVersion`]: The game version to fix the .ini file to
        
        Accepts a :class:`str`, :class:`int`, :class:`float` or :class:`CppVersion` when set
        """
    @toVersion.setter
    def toVersion(self, arg1: typing.Any) -> None:
        ...
class IniFixBuilder:
    """
    
    A factory that builds the :class:`CppBaseIniFixer` that fixes one mod onto another
    
    What :attr:`ModType.iniFixBuilder` holds, and what the pure-Python builder of this name was
    replaced by. It comes in two flavours:
    
    * **Fixed** -- one factory used for every .ini file, whatever its version
    * **Version-dependent** -- a lookup table consulted on every :meth:`build`
    
    Unlike the parse and remove builders, one source mod may be fixed onto **several** targets, which
    is what :meth:`buildAll` exists for -- normally you want that rather than :meth:`build`
    
    Parameters
    ----------
    factory: Optional[Callable[[:class:`CppBaseIniParser`, :class:`str`], Optional[:class:`BaseIniFixer`]]]
        Called to build each fixer, with the parser that read the .ini file and the name of the mod
        being fixed **to** :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, which uses :meth:`defaultFactory`
    
    .. note::
        Only the fixed flavour is constructible from Python -- see :class:`CppIniFixBuilderArgs`
        
    """
    @staticmethod
    def defaultFactory() -> collections.abc.Callable:
        """
        The factory used when none is supplied -- builds a :class:`GIMIFixer` owning its own fix context
        
        Returns
        -------
        Callable[[:class:`CppBaseIniParser`, :class:`str`], :class:`CppBaseIniFixer`]
            The default factory
        """
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, factory: typing.Any = None) -> None:
        ...
    def build(self, parser: CppBaseIniParser, fromModName: str, toModName: str, fromVersion: FixRaidenBoss2.core.CppVersion | None = None, toVersion: FixRaidenBoss2.core.CppVersion | None = None) -> CppBaseIniFixer:
        """
        Builds the fixer for **one** target mod
        
        Use :meth:`buildAll` when the target is not known up front, which is the normal case
        
        Parameters
        ----------
        parser: :class:`CppBaseIniParser`
            The parser that read the .ini file being fixed
        
        fromModName: :class:`str`
            The name of the mod being fixed **from**
        
        toModName: :class:`str`
            The name of the mod being fixed **to**
        
        fromVersion: Optional[:class:`CppVersion`]
            The game version the .ini file originates from :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        toVersion: Optional[:class:`CppVersion`]
            The game version to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`CppBaseIniFixer`
            The built fixer -- the very same object a Python factory returned, when one was given
        """
    def buildAll(self, parser: CppBaseIniParser, fromModName: str, fromVersion: FixRaidenBoss2.core.CppVersion | None = None, toVersion: FixRaidenBoss2.core.CppVersion | None = None, filteredToModNames: collections.abc.Set[str] | None = None) -> list[tuple[str, CppBaseIniFixer]]:
        """
        Builds one fixer per mod 'fromModName' can be fixed onto
        
        Parameters
        ----------
        parser: :class:`CppBaseIniParser`
            The parser that read the .ini file being fixed
        
        fromModName: :class:`str`
            The name of the mod being fixed **from**
        
        fromVersion: Optional[:class:`CppVersion`]
            The game version the .ini file originates from :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        toVersion: Optional[:class:`CppVersion`]
            The game version to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        filteredToModNames: Optional[Set[:class:`str`]]
            Only build fixers for these target mods :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every target
        
        Returns
        -------
        List[Tuple[:class:`str`, :class:`CppBaseIniFixer`]]
            One ``(toModName, fixer)`` pair per target mod, in no particular order
        """
    @property
    def builderArgs(self) -> CppIniFixBuilderArgs:
        """
        The lookup table this builder resolves factories from, or ``None`` if it is a fixed-factory builder
        
        :class:`CppIniFixBuilderArgs`
        """
    @property
    def errorOnNotFound(self) -> bool:
        """
        Whether :meth:`build` raises rather than falling back when the key has no row
        
        :class:`bool`
        """
class IniFixResource(IniResource):
    """
    
    This class inherits from :class:`IniResource`
    
    Base class for a resource to be fixed in the .ini file
        
    """
    def __init__(self, type: str, iniFolderPath: str, srcPath: str, fixedPath: str) -> None:
        """
        Constructs a new resource to be fixed
        
        Parameters
        ----------
        type: :class:`str`
            The name for the type of resource
        
        iniFolderPath: :class:`str`
            The path to the folder of the .ini file
        
        srcPath: :class:`str`
            The file path to the resource (resolved to an absolute path against 'iniFolderPath')
        
        fixedPath: :class:`str`
            The file path to the fixed resource (resolved to an absolute path against 'iniFolderPath')
        """
    @property
    def fixedPath(self) -> str:
        """
        :class:`str`: The full file path to the fixed resource
        """
    @fixedPath.setter
    def fixedPath(self, arg0: str) -> None:
        ...
class IniFixResourceModel(IniResourceModel):
    """
    
    This class inherits from :class:`IniResourceModel`
    
    Contains data for fixing a particular resource in a .ini file
        
    """
    def __init__(self, iniFolderPath: str, fixedPaths: dict, origPaths: dict | None = None) -> None:
        """
        Constructs new data for fixing a resource in a .ini file
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The folder path to where the .ini file of the resource is located
        
        fixedPaths: Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]
            The file paths to the fixed files for the resource -- the outer keys are the indices to the
            :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some
            resource, the inner keys are the names for the type of mod to fix to, and the inner values are
            the file paths within that :class:`IfContentPart`
        
        origPaths: Optional[Dict[:class:`int`, List[:class:`str`]]]
            The file paths for the (unfixed) resource. ``None`` if there's no original-file data at all
        
            **Default**: ``None``
        """
    def clear(self) -> None:
        """
        Clears out all the path data stored
        """
    def items(self) -> list[tuple[str, str, str | None, str | None]]:
        """
        Every fixed/orig path combination across every :class:`IfContentPart` and mod type in 'fixedPaths',
        in the same order 'fixedPaths' itself iterates -- the equivalent of iterating directly over the
        pure-Python original (``for fixedPath, fullPath, origPath, origFullPath in x``)
        
        Returns
        -------
        List[Tuple[:class:`str`, :class:`str`, Optional[:class:`str`], Optional[:class:`str`]]]
            The flattened ``(fixedPath, fullPath, origPath, origFullPath)`` tuples
        """
    @property
    def fixedPaths(self) -> dict:
        """
        Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]: The file paths to the fixed files for the resource
        """
    @fixedPaths.setter
    def fixedPaths(self, arg1: dict) -> None:
        ...
    @property
    def fullPaths(self) -> dict:
        """
        Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]: The absolute paths to the fixed resource files, keyed the same way as 'fixedPaths'
        """
    @fullPaths.setter
    def fullPaths(self, arg1: dict) -> None:
        ...
    @property
    def origFullPaths(self) -> dict:
        """
        Dict[:class:`int`, List[:class:`str`]]: The absolute paths to the (unfixed) resource files, keyed the same way as 'origPaths'
        """
    @origFullPaths.setter
    def origFullPaths(self, arg1: dict) -> None:
        ...
    @property
    def origPaths(self) -> typing.Any:
        """
        Optional[Dict[:class:`int`, List[:class:`str`]]]: The file paths for the (unfixed) resource, if any
        """
    @origPaths.setter
    def origPaths(self, arg1: dict | None) -> None:
        ...
class IniFixingContext:
    """
    
    The per-call options handed to :meth:`BaseIniFixer.fix`
    
    .. note::
        Not to be confused with the .ini file a fixer writes through, despite the near-identical name.
        This is a plain bag of options describing *one particular fix* and knows nothing about any file
    
    Parameters
    ----------
    isFirstModType: :class:`bool`
        Whether this fixer is running for the first :class:`ModType` of the .ini file :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
    
    isLastModType: :class:`bool`
        Whether this fixer is running for the last :class:`ModType` of the .ini file :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, isFirstModType: bool = True, isLastModType: bool = True) -> None:
        ...
    @property
    def isFirstModType(self) -> bool:
        """
        :class:`bool`: Whether this fixer is running for the first :class:`ModType` of the .ini file :raw-html:`<br />` :raw-html:`<br />`
        
        The mirror image of :attr:`IniFixingContext.isLastModType`, and there for the same reason: several
        fixers chain over one .ini file, so anything that touches the file itself rather than only the fix
        has to happen exactly once :raw-html:`<br />` :raw-html:`<br />`
        
        :meth:`GIMIFixer.fix` uses it to gate ``keepBackup`` -- disabling the existing .ini file as a backup
        is the whole file's business, and a later mod type doing it again would be backing up a file the
        first pass already moved aside. The condition it gates is otherwise unchanged: ``keepBackup`` still
        also needs ``fixOnly`` and an .ini file that already exists on disk :raw-html:`<br />` :raw-html:`<br />`
        
        **Default**: ``True``, so a fixer driven directly -- the only one, hence both the first and the last
        -- backs up as it always did
        """
    @isFirstModType.setter
    def isFirstModType(self, arg0: bool) -> None:
        ...
    @property
    def isLastModType(self) -> bool:
        """
        :class:`bool`: Whether this fixer is running for the last :class:`ModType` of the .ini file :raw-html:`<br />` :raw-html:`<br />`
        
        One .ini file can be fixed by several fixers in turn -- one per mod type it was classified as, and
        one per target mod each of those fixes to. They chain over the same file, so anything that rewrites
        the file's own text rather than only adding to the fix has to happen exactly once, at the end :raw-html:`<br />` :raw-html:`<br />`
        
        :meth:`GIMIFixer.fix` uses it for its ``hideOrig`` pass for that reason: commenting out the original
        mod's `sections`_ is the whole file's business, not one mod type's. With this ``False``, ``hideOrig``
        is ignored :raw-html:`<br />` :raw-html:`<br />`
        
        **Default**: ``True``, so a fixer driven directly -- one fixer, one call, no chain -- behaves as
        though it were the only one, which it is. It is the *chaining* caller that has to say otherwise, and
        :class:`IniFile` is the one that does
        """
    @isLastModType.setter
    def isLastModType(self, arg0: bool) -> None:
        ...
class IniGraphGroup:
    """
    
    A class to represent a group of caller/callee graphs within a .ini file
        
    """
    def __init__(self, graphs: typing.Any = None) -> None:
        """
        Constructs a new group of graphs
        
        Parameters
        ----------
        graphs: Optional[Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]]
            The group of graphs -- the keys contain the name of the component and the name of the mod
            object, and the values are the associated graph. If ``None``, a fresh empty ``dict`` is used
        
            **Default**: ``None``
        """
    def addGraph(self, modObj: typing.Any, graph: typing.Any) -> None:
        """
        Adds a new graph
        
        Parameters
        ----------
        modObj: Tuple[:class:`str`, :class:`str`]
            The associated component and mod object for the graph
        
        graph: :class:`IniSectionGraph`
            The new graph to add
        """
    def removeGraph(self, modObj: typing.Any) -> typing.Any:
        """
        Removes a graph based on the specified component and mod object
        
        Parameters
        ----------
        modObj: Tuple[:class:`str`, :class:`str`]
            The name of the component and mod object
        
        Returns
        -------
        Optional[:class:`IniSectionGraph`]
            The associated graph, if removed
        """
    def toStr(self, autoindent: bool = True) -> str:
        """
        Converts all the sections in the group of graphs to a string
        
        Parameters
        ----------
        autoindent: :class:`bool`
            Whether to compute the proper tab indent for the section
        
            **Default**: ``True``
        
        Returns
        -------
        :class:`str`
            The string representation
        """
    @property
    def graphs(self) -> dict:
        """
        Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]: The group of graphs -- the keys
        contain the name of the component and the name of the mod object, and the values are the associated
        graph
        """
    @graphs.setter
    def graphs(self, arg0: dict) -> None:
        ...
class IniGroupedResource:
    """
    
    Base class for a group of resources
    
    .. note::
        'resources' is a real Python ``dict`` here (not a typed mapping to some resource class) --
        this class's one real caller (``ResGroupCollect``) uses it as general-purpose scratch storage
        keyed by arbitrary hashable values, not just resource type names; see this binding's own
        source comment for why
        
    """
    def __deepcopy__(self, memo: typing.Any) -> typing.Any:
        """
        Supports ``copy.deepcopy()`` on this object
        """
    def __init__(self, name: str, resources: typing.Any = None, fixFunc: collections.abc.Callable[[...], bool] = None, isBuilt: bool = True) -> None:
        """
        Constructs a new group of resources
        
        Parameters
        ----------
        name: :class:`str`
            The name of the group of resources
        
        resources: Optional[Dict[Any, Any]]
            The group of resources -- general-purpose scratch storage, keyed and valued by whatever the
            caller needs (see this class's own note above). If ``None``, a fresh empty ``dict`` is used
        
            **Default**: ``None``
        
        fixFunc: Optional[Callable[[:class:`IniGroupedResource`], :class:`bool`]]
            Custom function for fixing the resource, overriding the default (no-op) behavior if given
        
            **Default**: ``None``
        
        isBuilt: :class:`bool`
            Whether the grouped resource is ready to be fixed
        
            **Default**: ``True``
        """
    def addResource(self, resType: typing.Any, resource: typing.Any) -> None:
        """
        Adds an individual resource to the resource group
        
        Parameters
        ----------
        resType: Any
            The key for the resource
        
        resource: Any
            The resource to add
        """
    def fix(self) -> bool:
        """
        Fixes the resource -- calls 'fixFunc' if set, otherwise does nothing
        
        Returns
        -------
        :class:`bool`
            Whether the resource was fixed
        """
    def isMissing(self, collected: typing.Any) -> bool:
        """
        Given a subset of the collected resource keys so far, is this grouped resource missing some
        resource from the given subset
        
        Parameters
        ----------
        collected: Iterable[Any]
            The subset of the keys of the collected resources so far
        
        Returns
        -------
        :class:`bool`
            Whether this grouped resource is missing some resource from the specified subset
        """
    @property
    def fixFunc(self) -> collections.abc.Callable[[...], bool]:
        """
        Optional[Callable[[:class:`IniGroupedResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default (no-op) behavior if set
        """
    @fixFunc.setter
    def fixFunc(self, arg0: collections.abc.Callable[[...], bool]) -> None:
        ...
    @property
    def isBuilt(self) -> bool:
        """
        :class:`bool`: Whether the grouped resource is ready to be fixed
        """
    @isBuilt.setter
    def isBuilt(self, arg0: bool) -> None:
        ...
    @property
    def name(self) -> str:
        """
        :class:`str`: The name of the group of resources
        """
    @name.setter
    def name(self, arg0: str) -> None:
        ...
    @property
    def resources(self) -> dict:
        """
        Dict[Any, Any]: The group of resources -- general-purpose scratch storage (see the class's own note)
        """
    @resources.setter
    def resources(self, arg0: dict) -> None:
        ...
class IniParseBuilder:
    """
    
    A factory that builds the :class:`CppBaseIniParser` for one .ini file
    
    What :attr:`ModType.iniParseBuilder` holds, and what the pure-Python builder of this name was
    replaced by. It comes in two flavours:
    
    * **Fixed** -- one factory used for every .ini file, whatever its version
    * **Version-dependent** -- a lookup table consulted by ``(modName, version)`` on every
      :meth:`build`, so a 5.7-era .ini file gets a different parser than a 4.0-era one
    
    Parameters
    ----------
    factory: Optional[Callable[[:class:`IniFile`, Optional[:class:`int`]], Optional[:class:`BaseIniParser`]]]
        Called to build each parser, with the .ini file it will read and the id of the mod type it is
        being built for :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, which uses :meth:`defaultFactory`
    
    .. note::
        Only the fixed flavour is constructible from Python -- see :class:`CppIniParseBuilderArgs`
        
    """
    @staticmethod
    def defaultFactory() -> collections.abc.Callable:
        """
        The factory used when none is supplied -- builds a :class:`GIMIParser` owning its own parse context
        
        Returns
        -------
        Callable[[:class:`IniFile`, Optional[:class:`int`]], :class:`CppBaseIniParser`]
            The default factory
        """
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, factory: typing.Any = None) -> None:
        ...
    def build(self, iniFile: IniFile, modName: str, version: FixRaidenBoss2.core.CppVersion | None = None, modTypeId: typing.SupportsInt | typing.SupportsIndex | None = None) -> CppBaseIniParser:
        """
        Builds the parser for one .ini file
        
        Parameters
        ----------
        iniFile: :class:`IniFile`
            The .ini file the built parser will read
        
        modName: :class:`str`
            The name of the mod to build the parser for :raw-html:`<br />` :raw-html:`<br />`
        
            Ignored entirely by a fixed-factory builder
        
        version: Optional[:class:`CppVersion`]
            The game version the .ini file originates from :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning the latest listed version
        
        modTypeId: Optional[:class:`int`]
            Which of the .ini file's mod types the parser is being built for :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`CppBaseIniParser`
            The built parser -- the very same object a Python factory returned, when one was given
        """
    @property
    def builderArgs(self) -> CppIniParseBuilderArgs:
        """
        The lookup table this builder resolves factories from, or ``None`` if it is a fixed-factory builder
        
        :class:`CppIniParseBuilderArgs`
        """
    @property
    def errorOnNotFound(self) -> bool:
        """
        Whether :meth:`build` raises rather than falling back when the mod name has no row
        
        :class:`bool`
        """
class IniRemovalContext:
    """
    
    The per-call options handed to :meth:`BaseIniRemover.remove`
    
    .. note::
        Not to be confused with the .ini file a remover reads through, despite the near-identical name.
        This is a plain bag of options describing *one particular removal* and knows nothing about any
        file
    
    Parameters
    ----------
    ignoreModType: :class:`bool`
        Whether to remove the fix without asking which :class:`ModType` it belongs to :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
        
    """
    def __init__(self, ignoreModType: bool = False) -> None:
        ...
    @property
    def ignoreModType(self) -> bool:
        """
        :class:`bool`: Whether to remove the fix without asking which :class:`ModType` it belongs to :raw-html:`<br />` :raw-html:`<br />`
        
        :meth:`RemapIniRemover.remove` normally treats a candidate `section`_ as this software's own output only
        when it carries this software's marker -- either it sits inside the fix boilerplate, or some
        reachable colouring state gives its ``hash`` `KVP`_ a value belonging to one of the .ini file's mod
        types. That second half is what recognizes the ``Remap``-named leftovers *outside* the boilerplate :raw-html:`<br />` :raw-html:`<br />`
        
        With this set, the hash half is skipped and **every** candidate is taken -- every `section`_ inside
        the boilerplate plus every ``Remap``-named `section`_ outside it, whoever they belong to. That is
        what the pure-Python ``RemapIniRemover`` this replaced always did :raw-html:`<br />` :raw-html:`<br />`
        
        :class:`IniFile` asks for it on its **last** mod type, so that every earlier pass takes only what it
        can prove is its own and the final pass clears whatever is still standing. Without it, a leftover
        carrying no usable ``hash`` would survive every pass -- the exact debris an interrupted or partly
        undone fix leaves behind
        """
    @ignoreModType.setter
    def ignoreModType(self, arg0: bool) -> None:
        ...
class IniRemoveBuilder:
    """
    
    A factory that builds the :class:`CppBaseIniRemover` for one .ini file
    
    What :attr:`ModType.iniRemoveBuilder` holds, and what the pure-Python builder of this name was
    replaced by. It comes in two flavours:
    
    * **Fixed** -- one factory used for every .ini file, whatever its version
    * **Version-dependent** -- a lookup table consulted by ``(modName, version)`` on every
      :meth:`build`
    
    Parameters
    ----------
    factory: Optional[Callable[[:class:`IniFile`], Optional[:class:`BaseIniRemover`]]]
        Called to build each remover, with the .ini file the remover will act on :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``, which uses :meth:`defaultFactory`
    
    .. note::
        Only the fixed flavour is constructible from Python -- see :class:`CppIniRemoveBuilderArgs`
        
    """
    @staticmethod
    def defaultFactory() -> collections.abc.Callable:
        """
        The factory used when none is supplied -- builds a :class:`RemapIniRemover`
        
        Returns
        -------
        Callable[[:class:`IniFile`], :class:`CppBaseIniRemover`]
            The default factory
        """
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, factory: typing.Any = None) -> None:
        ...
    def build(self, iniFile: IniFile, modName: str = '', version: FixRaidenBoss2.core.CppVersion | None = None) -> CppBaseIniRemover:
        """
        Builds the remover for one .ini file
        
        Parameters
        ----------
        iniFile: :class:`IniFile`
            The .ini file the built remover will act on
        
        modName: :class:`str`
            The name of the mod to build the remover for :raw-html:`<br />` :raw-html:`<br />`
        
            Ignored entirely by a fixed-factory builder :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        version: Optional[:class:`CppVersion`]
            The game version the .ini file originates from :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning the latest listed version
        
        Returns
        -------
        :class:`CppBaseIniRemover`
            The built remover -- a :class:`BaseIniRemover` when the factory came from Python
        """
    @property
    def builderArgs(self) -> CppIniRemoveBuilderArgs:
        """
        The lookup table this builder resolves factories from, or ``None`` if it is a fixed-factory builder
        
        :class:`CppIniRemoveBuilderArgs`
        """
    @property
    def errorOnNotFound(self) -> bool:
        """
        Whether :meth:`build` raises rather than falling back when the mod name has no row
        
        :class:`bool`
        """
class IniResource:
    """
    
    Base class for a resource in the .ini file
        
    """
    def __init__(self, type: str, iniFolderPath: str, srcPath: str) -> None:
        """
        Constructs a new resource
        
        Parameters
        ----------
        type: :class:`str`
            The name for the type of resource
        
        iniFolderPath: :class:`str`
            The path to the folder of the .ini file
        
        srcPath: :class:`str`
            The file path to the resource (resolved to an absolute path against 'iniFolderPath')
        """
    @property
    def srcPath(self) -> str:
        """
        :class:`str`: The full file path to the resource
        """
    @srcPath.setter
    def srcPath(self, arg0: str) -> None:
        ...
    @property
    def type(self) -> str:
        """
        :class:`str`: The name for the type of resource
        """
    @type.setter
    def type(self, arg0: str) -> None:
        ...
class IniResourceModel:
    """
    
    Contains data for some particular resource in a .ini file
        
    """
    def __init__(self, iniFolderPath: str) -> None:
        """
        Constructs new data for a resource in a .ini file
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The folder path to where the .ini file of the resource is located
        """
    @property
    def iniFolderPath(self) -> str:
        """
        :class:`str`: The folder path to where the .ini file of the resource is located
        """
    @iniFolderPath.setter
    def iniFolderPath(self, arg0: str) -> None:
        ...
class IniSectionGraph:
    """
    
    Class for constructing a directed subgraph for how the `sections`_ in the .ini file are ran
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: for sectionName, section in x
    
            Iterates through all the `sections`_ of the graph using `DFS`_
    
    Parameters
    ----------
    sections: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ of the constructed subgraph
    
    targetSectionNames: Union[Set[:class:`str`], List[:class:`str`]]
        Names of the desired `sections`_ we want our subgraph to have
    
    build: :class:`bool`
        Whether to build the graph. **Default**: ``True``
    
    copySections: :class:`bool`
        Whether to make a deep copy of the referenced `sections`_. **Default**: ``False``
    
    z3Ctx: Optional[:class:`Z3Context`]
        The `Z3`_ context every :class:`Z3Predicate` produced by this graph belongs to. **Default**: ``None``
        
    """
    @staticmethod
    def computeSectionPredecessors(section: IfTemplate) -> dict:
        """
        Computes, for every :class:`IfContentPart` in a `section`_'s flat, textually-ordered parts list, the
        ``id()`` of every :class:`IfContentPart` that must run immediately before it on some path through
        this `section`_ alone
        
        Parameters
        ----------
        section: :class:`IfTemplate`
            The `section`_ to compute predecessors for
        
        Returns
        -------
        Dict[:class:`int`, List[:class:`int`]]
            The ``id()`` of every :class:`IfContentPart`, mapped to the ``id()`` of its predecessors
        """
    @staticmethod
    def iterSectsByContentPart(sections: dict, roots: list, states: typing.SupportsInt | typing.SupportsIndex = 1, colour: bool = False, colourKeys: typing.Any = None) -> SectionIterDataIterator:
        """
        An iterator that iterates through all :class:`IfContentPart` of the `sections`_ using `DFS`_
        """
    def __copy__(self) -> IniSectionGraph:
        ...
    def __deepcopy__(self, memo: dict) -> IniSectionGraph:
        ...
    def __init__(self, sections: dict, targetSectionNames: typing.Any, build: bool = True, copySections: bool = False, z3Ctx: typing.Any = None) -> None:
        ...
    def __iter__(self) -> IniSectionGraphSectionIterator:
        """
        Iterates through all the `sections`_ of the graph using `DFS`_, yielding ``(sectionName, section)`` pairs
        """
    def build(self, sections: typing.Any = None, targetSectionNames: typing.Any = None, copySections: bool = False) -> None:
        """
        Constructs the subgraph for the `sections`_ using `DFS`_
        """
    def buildCallGraph(self) -> CallGraph:
        """
        Builds a `call graph`_ over the :class:`IfContentPart`\\s of this graph
        """
    def buildPartPredecessorGraph(self) -> dict:
        """
        Builds a graph-wide version of :meth:`computeSectionPredecessors`, additionally linking a
        ``run =`` call's own part as a predecessor of whatever `section`_ it calls into
        
        Returns
        -------
        Dict[:class:`int`, List[:class:`int`]]
            The ``id()`` of every reachable :class:`IfContentPart`, mapped to the ``id()`` of its predecessors
        """
    def combine(self, newGraphs: list) -> None:
        """
        Combines this graph with other graphs
        
        Parameters
        ----------
        newGraphs: List[:class:`IniSectionGraph`]
            The new graphs to combine with
        """
    def deepcopy(self, minimal: bool = True, newPartIds: bool = True) -> IniSectionGraph:
        """
        Performs a deep copy on the object
        """
    def getChildren(self, targetSections: typing.Any, getNeighbourChildren: bool = True) -> dict:
        """
        Retrieves the children `sections`_ of the `sections`_ specified at 'targetSections'
        """
    def getKeyMissingParts(self, key: str) -> dict:
        """
        Retrieves the parts in the `sections`_ that are not covered by 'key'
        """
    def getNeighbourNames(self, sectionName: str) -> typing.Any:
        """
        Retrieves the names of the out-neighbour `sections`_
        """
    def getNeighbours(self, sectionName: str) -> dict:
        """
        Retrieves the out-neighbours of some `section`_
        """
    def getRootSections(self) -> list:
        """
        Retrieves the `sections`_ corresponding to the roots of the graph
        """
    def getSection(self, sectionName: str, raiseException: bool = True) -> typing.Any:
        """
        Retrieves the :class:`IfTemplate` for a certain `section`_
        """
    def isEmpty(self) -> bool:
        """
        Determines whether the graph is empty
        """
    def isKeyFullyCover(self, key: str) -> typing.Any:
        """
        Determines whether a key fully covers all the conditional branches of a `section`_
        """
    def iterByContentPart(self, states: typing.SupportsInt | typing.SupportsIndex = 1, colour: bool = False, colourKeys: typing.Any = None) -> SectionIterDataIterator:
        """
        An iterator that iterates through all :class:`IfContentPart` of the `sections`_ of this graph using `DFS`_
        """
    def iterByQuery(self, queryPath: typing.Any = None, simplify: bool = False, states: typing.SupportsInt | typing.SupportsIndex = 1, colour: bool = False, colourKeys: typing.Any = None) -> SectionIterQueryDataIterator:
        """
        An iterator that iterates through all the :class:`IfContentPart`\\s of the graph and also retrieves
        the conditional logical predicate that each :class:`IfContentPart` resides in
        """
    def normalize(self) -> None:
        """
        Normalizes the branching structure of all `sections`_ in :attr:`sections`
        """
    def processIfContentByQuery(self, processIfContent: collections.abc.Callable, queryPath: typing.Any = None, simplify: bool = False, states: typing.SupportsInt | typing.SupportsIndex = 1, colour: bool = False, colourKeys: typing.Any = None) -> None:
        """
        Processes all :class:`IfContentPart`\\s of the graph that require the conditional logic predicate they reside in
        """
    def refreshPartIds(self, minimal: bool = True) -> None:
        """
        Regenerates the ids of the parts
        """
    def rename(self, renameFunc: collections.abc.Callable) -> None:
        """
        Renames the `sections`_ and reconstructs the graph
        """
    def rootsAreFullyCovered(self, key: str) -> typing.Any:
        """
        Convenience over :meth:`isKeyFullyCover`, filtered to :attr:`roots`
        """
    def toStr(self, autoindent: bool = True) -> str:
        """
        Converts all the `sections`_ of this graph to a string, walked outwards from :attr:`roots` using
        `DFS`_ and joined with blank lines
        
        Parameters
        -----------
        autoindent: :class:`bool`
            Whether to compute the proper tab indent for each `section`_
        
            **Default**: ``True``
        
        Returns
        --------
        :class:`str`
            The string representation
        """
    @property
    def neighbours(self) -> typing.Any:
        """
        Dict[:class:`str`, List[:class:`str`]]: The out-neighbours of the subgraph
        """
    @property
    def roots(self) -> typing.Any:
        """
        List[:class:`str`]: The root nodes of the subgraph
        """
    @property
    def sections(self) -> dict:
        """
        Dict[:class:`str`, :class:`IfTemplate`]: All the `sections`_ of the constructed subgraph
        """
    @property
    def targetSectionNames(self) -> typing.Any:
        """
        List[:class:`str`]: Names of the desired `sections`_ we want our subgraph to have
        """
    @targetSectionNames.setter
    def targetSectionNames(self, arg1: typing.Any) -> None:
        ...
class IniSectionGraphSectionIterator:
    def __iter__(self) -> IniSectionGraphSectionIterator:
        ...
    def __next__(self) -> typing.Any:
        ...
class IniSrcResourceModel(IniResourceModel):
    """
    
    This class inherits from :class:`IniResourceModel`
    
    Contains data for a particular resource in the original .ini file
        
    """
    def __init__(self, iniFolderPath: str, paths: dict) -> None:
        """
        Constructs new data for a resource in the original .ini file
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The folder path to where the .ini file of the resource is located
        
        paths: Dict[:class:`int`, List[:class:`str`]]
            The file paths to the resource -- the keys are the indices to the :class:`IfContentPart` that
            the resource file appears in the :class:`IfTemplate` for some resource, and the values are the
            file paths within that :class:`IfContentPart`
        """
    def items(self) -> list[tuple[str, str]]:
        """
        Every ``(path, fullPath)`` pair across every :class:`IfContentPart` in 'paths', in the same order
        'paths' itself iterates -- the equivalent of iterating directly over the pure-Python original
        (``for path, fullPath in x``)
        
        Returns
        -------
        List[Tuple[:class:`str`, :class:`str`]]
            The flattened ``(path, fullPath)`` pairs
        """
    @property
    def fullPaths(self) -> dict:
        """
        Dict[:class:`int`, List[:class:`str`]]: The absolute paths to the resource, keyed the same way as 'paths'
        """
    @fullPaths.setter
    def fullPaths(self, arg1: dict) -> None:
        ...
    @property
    def paths(self) -> dict:
        """
        Dict[:class:`int`, List[:class:`str`]]: The file paths to the resource, keyed by :class:`IfContentPart` index
        """
    @paths.setter
    def paths(self, arg1: dict) -> None:
        ...
class IniTexModel(IniFixResourceModel):
    """
    
    This class inherits from :class:`IniFixResourceModel`
    
    Contains data for editing some texture files in a .ini file
        
    """
    def __init__(self, iniFolderPath: str, fixedPaths: dict, texEdits: dict, origPaths: dict | None = None) -> None:
        """
        Constructs new data for editing a texture file in a .ini file
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The folder path to where the .ini file of the resource is located
        
        fixedPaths: Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]
            See :class:`IniFixResourceModel`'s constructor
        
        texEdits: Dict[:class:`int`, Dict[:class:`str`, List[:class:`CppBaseTexEditor`]]]
            The texture editors used to edit the texture -- the outer keys are the indices to the
            :class:`IfContentPart` that the ``.dds`` file appears in the :class:`IfTemplate` for some
            texture, the inner keys are the names for the type of mod to fix to, and the inner values are
            the different texture editors used on the ``.dds`` files. Ownership of each editor is
            transferred into this model
        
        origPaths: Optional[Dict[:class:`int`, List[:class:`str`]]]
            See :class:`IniFixResourceModel`'s constructor
        
            **Default**: ``None``
        """
    def clear(self) -> None:
        """
        Clears out all the path/texture-editor data stored
        """
    @property
    def texEdits(self) -> dict:
        """
        Dict[:class:`int`, Dict[:class:`str`, List[:class:`CppBaseTexEditor`]]]: The texture editors used to edit the texture
        """
    @texEdits.setter
    def texEdits(self, arg1: dict) -> None:
        ...
class KeyRemapData:
    """
    
    A :meth:`OrderedMultiMap.remapKeys` ``keyRemap`` value (alongside a bare list of
    keys/:class:`RemappedKeyData`) that additionally controls what happens to an occurrence when
    none of its rules fire.
    
    Parameters
    ----------
    remappedKeys: List[Union[Any, :class:`RemappedKeyData`]]
        The remap rules for this key -- identical in meaning to passing a bare list directly
    
    keepKeyWithoutRemap: :class:`bool`
        If ``True``, an occurrence for which zero rules fired (an empty list, or every rule's
        ``check`` ``False``) retains its original ``(key, value)`` pair instead of being removed.
        Evaluated per occurrence :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``, matching a bare list's behavior: zero firings means removal
            
    """
    def __init__(self, remappedKeys: collections.abc.Sequence, keepKeyWithoutRemap: bool = False) -> None:
        ...
    @property
    def keepKeyWithoutRemap(self) -> bool:
        """
        :class:`bool`: Whether a non-firing occurrence retains its original pair
        """
class Logger(BaseLogger):
    """
    
    This class inherits from :class:`BaseLogger`
    
    The console view -- pretty prints output to display on the console (through ``print``), and reads the user's
    answers back with ``input``
    
    .. note::
        This is the view every existing part of the package (:class:`RemapService`, :class:`Mod`, ...) is written
        against. To send the same messages somewhere else -- a GUI, a socket to a frontend app -- subclass
        :class:`BaseLogger` (or this class) and implement :meth:`write`/:meth:`read`
    
    Parameters
    ----------
    prefix: :class:`str`
        line that is printed before any message is printed out :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ""
    
    logTxt: :class:`bool`
        Whether to log all the printed messages into a .txt file once the fix is done :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
    
    verbose: :class:`bool`
        Whether to print out output :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, prefix: str = '', logTxt: bool = False, verbose: bool = True) -> None:
        ...
    def read(self, desc: str) -> str:
        """
        Asks the user for a line of input on the console, with ``input``
        
        Parameters
        ----------
        desc: :class:`str`
            The question/description being asked to the user for input
        
        Returns
        -------
        :class:`str`
            The resultant input the user entered
        """
    def write(self, message: str) -> None:
        """
        Prints the message onto the console, with ``print``
        
        Parameters
        ----------
        message: :class:`str`
            The rendered message to display
        """
class ModAssets:
    """
    
    Class to handle assets of any type for a mod where retrieval is based on some keys where 1 or more
    of the keys refer to some versioning
    
    :raw-html:`<br />`
    
    .. tip::
        If the assets have more than 1 column that refers to some version, use this data structure.
        Otherwise if your asset has only 1 column that refers to some version, it is recommended to use
        :class:`ModDictAssets` instead, since that uses a hash based access instead of a linear scan
        
    """
    NameKey: typing.ClassVar[str] = 'name'
    ValueKey: typing.ClassVar[str] = 'value'
    VersionKey: typing.ClassVar[str] = 'version'
    def __copy__(self) -> ModAssets:
        ...
    def __deepcopy__(self, memo: dict) -> ModAssets:
        ...
    def __init__(self, repo: typing.Any, indices: typing.Any = None, versionIndices: typing.Any = None, valueCol: typing.Any = None, **kwargs) -> None:
        """
        Constructs a new asset lookup table
        
        :raw-html:`<br />`
        
        .. note::
            Any extra keyword argument is accepted and ignored, matching the pure-Python original this
            replaced (whose own constructor ended in ``**kwargs``)
        
        Parameters
        ----------
        repo: Union[List[Tuple[List[Any], Any]], dict]
            The original source for the assets -- either an already-flattened list of ``(indexVals, value)``
            tuples, or a nested dict exactly ``len(indices)`` levels deep
        
        indices: Optional[List[:class:`str`]]
            The names of the index columns to query to retrieve the main content of the asset
            :raw-html:`<br />` :raw-html:`<br />`
        
            If this value is ``None``, then will set 2 index columns by the names "version" and "name"
        
            **Default**: ``None``
        
        versionIndices: Optional[Set[:class:`str`]]
            The names of the index columns that refer to some version -- any name not also in 'indices' is
            ignored :raw-html:`<br />` :raw-html:`<br />`
        
            If this value is ``None``, then will set an index to the name "version"
        
            **Default**: ``None``
        
        valueCol: Optional[:class:`str`]
            Unused by the lookup (rows already carry their own value, rather than one being selected by
            column name) -- kept for constructor-signature backward compatibility
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`KeyError`
            If 'indices' contains a duplicate name
        
        :class:`ValueError`
            If 'repo' is a dict that isn't nested exactly ``len(indices)`` levels deep
        """
    def __len__(self) -> int:
        """
        The total number of rows currently in the table
        """
    def addRows(self, rows: typing.Any) -> None:
        """
        Adds new rows to the table (an addition beyond the pure-Python original, which has no
        incremental-add capability at all) -- overwrites the value of any row whose full key already exists
        
        Parameters
        ----------
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The rows to add, in the same shape as the constructor's own 'repo' argument
        """
    def clone(self) -> ModAssets:
        """
        Creates an independent copy of this table
        
        Returns
        -------
        :class:`ModAssets`
            The copied table
        """
    def get(self, nonVersionVals: typing.Any, versionVals: typing.Any = None, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the corresponding asset
        
        Parameters
        ----------
        nonVersionVals: Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]
            The values of the index columns that do not reference a version -- a bare value (taken as the
            first such column), a positional list, or a dict keyed by index name. A column left
            unspecified matches anything there
        
        versionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            The values of the index columns that reference a version, in the same accepted shapes
            :raw-html:`<br />` :raw-html:`<br />`
        
            .. note::
                If the value for a particular version column is ``None``, then will get the latest version
                for that column -- among the rows still matching everything resolved before it, since
                version columns are resolved sequentially in index order
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            If no assets are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        default: Any
            If 'errorOnNotFound' is ``False``, then the default value to return if no assets are found
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`KeyError`
            If the corresponding asset based on the search parameters is not found and 'errorOnNotFound' is
            set to ``True``
        
        Returns
        -------
        Any
            Either the found asset, or the value specified from 'default' if 'errorOnNotFound' is set to
            ``False``
        """
    @property
    def indices(self) -> list[str]:
        """
        List[:class:`str`]: The names of the index columns to query to retrieve the main content of an asset
        """
    @property
    def nonVersionColumnCount(self) -> int:
        """
        :class:`int`: The number of non-version columns
        """
    @property
    def totalIndices(self) -> int:
        """
        :class:`int`: The total number of index columns
        """
    @property
    def valueCol(self) -> str:
        """
        :class:`str`: Unused by the lookup -- see the constructor's own note
        """
    @property
    def versionColumnCount(self) -> int:
        """
        :class:`int`: The number of version columns
        """
    @property
    def versionIndices(self) -> set:
        """
        Set[:class:`str`]: The names of the index columns that refer to some version
        """
class ModDictAssets:
    """
    
    Handles assets of any type for a mod where retrieval is based on some keys where only one of the
    keys refers to some versioning
    
    :raw-html:`<br />`
    
    Internally, the source data is never a nested dict -- rows are stored already-flattened, as
    ``(indexVals, value)`` tuples, where ``indexVals`` holds every index column's raw value in index
    order (including the version index's own raw, not-yet-parsed value). The constructor takes rows
    already in that shape; :meth:`fromNestedDict` builds an instance from a real nested dict instead
    (the shape ``HashData``/``IndexData`` are written as), flattening it in C++ rather than Python
        
    """
    @staticmethod
    def fromNestedDict(totalIndices: typing.SupportsInt | typing.SupportsIndex, versionIndexPos: typing.SupportsInt | typing.SupportsIndex, repo: dict) -> ModDictAssets:
        """
        Constructs a new asset lookup table from a real nested dict, flattening it first
        
        Parameters
        ----------
        totalIndices: :class:`int`
            The total number of index columns (including the version index)
        
        versionIndexPos: :class:`int`
            The position (0-based) of the version index within a row's index values
        
        repo: dict
            The nested dict to flatten, exactly 'totalIndices' levels deep (e.g. for
            ``totalIndices = 3``: ``{version: {name: {type: leafValue}}}``)
        
        Raises
        ------
        :class:`ValueError`
            If 'repo' is not nested exactly 'totalIndices' levels deep
        """
    def __init__(self, totalIndices: typing.SupportsInt | typing.SupportsIndex, versionIndexPos: typing.SupportsInt | typing.SupportsIndex, rows: typing.Any = []) -> None:
        """
        Constructs a new asset lookup table
        
        Parameters
        ----------
        totalIndices: :class:`int`
            The total number of index columns (including the version index)
        
        versionIndexPos: :class:`int`
            The position (0-based) of the version index within a row's index values
        
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The initial rows to populate the table with -- either a flat list of ``(indexVals, value)``
            tuples, or a real nested dict ('totalIndices' levels deep) -- see :meth:`addRows`
        
            **Default**: ``[]``
        """
    def __len__(self) -> int:
        """
        The total number of rows currently in the table, across every non-version index group
        """
    def addRows(self, rows: typing.Any) -> None:
        """
        Adds new rows to the table, overwriting the value of any row whose full key (every non-version
        index value, plus its parsed version) already exists
        
        Parameters
        ----------
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The rows to add, in the same shape as the constructor's own 'rows' argument (a flat list or a
            real nested dict)
        
        Raises
        ------
        :class:`ValueError`
            If any row's index values don't match :attr:`totalIndices` in length, or if a row's version
            index value fails to parse as a version
        """
    def get(self, nonVersionVals: collections.abc.Sequence[str], version: typing.Any = None, errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the corresponding asset
        
        Parameters
        ----------
        nonVersionVals: List[Any]
            The values of every index column that does not refer to a version, in index order (with the
            version column's position skipped)
        
        version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The specific version to query the asset -- the latest available version is used if this is
            ``None`` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if no matching asset is found
        
            **Default**: ``True``
        
        Raises
        ------
        :class:`ValueError`
            If 'nonVersionVals' doesn't have exactly :attr:`totalIndices` ``- 1`` elements, or if
            'version' doesn't parse as a valid version
        
        :class:`KeyError`
            If no matching asset is found and 'errorOnNotFound' is ``True``
        
        Returns
        -------
        Any
            The found asset, or ``None`` if none is found and 'errorOnNotFound' is ``False``
        """
    def toNestedDict(self) -> dict:
        """
        Rebuilds the original nested-dict form of this table's data (``{indexVal0: {indexVal1: {... :
        value}}}``, in index-column order, the version column's original raw value replaced with its
        normalized string form) -- the inverse of :meth:`fromNestedDict`/the constructor's own nested-dict
        'rows' shape
        
        Returns
        -------
        dict
            The reconstructed nested dict
        """
    @property
    def totalIndices(self) -> int:
        """
        :class:`int`: The total number of index columns (including the version index)
        """
    @property
    def versionIndexPos(self) -> int:
        """
        :class:`int`: The position (0-based) of the version index within a row's index values
        """
class ModMappedAssets:
    """
    
    Handles assets of any type where asset retrieval is based on a mapping -- a `bipartite graph`_
    that maps assets to fix from to assets to fix to
        
    """
    def __init__(self, repo: ModDictAssets, map: typing.Any = None, nonVersionIndexNames: typing.Any = None) -> None:
        """
        Constructs a new mapped asset table
        
        Parameters
        ----------
        repo: :class:`ModDictAssets`
            The underlying asset data
        
        map: Optional[Dict[Any, List[Any]]]
            The initial adjacency list mapping assets to fix from to assets to fix to
        
            **Default**: ``None``
        
        nonVersionIndexNames: Optional[List[:class:`str`]]
            The names of 'repo''s non-version index columns, in position order -- when given, ``hasFrom``/
            ``getKey``/``replace``/``replaceAll``/``_convertNonVersionVals`` accept a flexible bare value,
            a list, or a dict keyed by one of these names for their non-version-values filter, instead of
            requiring an already-positional list. ``None`` (the default) keeps the strictly positional
            behaviour, appropriate for any use that isn't backed by named indices
        
            **Default**: ``None``
        """
    def _convertNonVersionVals(self, indexVals: typing.Any) -> list:
        """
        Normalizes a flexible non-version-values filter into the plain positional
        ``List[Optional[Any]]`` shape :meth:`getKey`/:meth:`hasFrom`/:meth:`replace` accept for their own
        'nonVersionVals'/'fromNonVersionVals' argument (``None`` = wildcard at that position) --
        :attr:`nonVersionIndexNames` names each position :raw-html:`<br />` :raw-html:`<br />`
        
        .. note::
            Calling this directly is rarely necessary any more -- :meth:`getKey`/:meth:`hasFrom`/
            :meth:`replace`/:meth:`replaceAll` all already accept the same flexible shape for their own
            non-version-values argument. Kept as public API for callers that want to convert once and
            reuse the result across several calls (e.g. ``GIMIParser.py``, filtering many hash/index
            values per parse against the same fixed non-version filter)
        
        Parameters
        ----------
        indexVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            The raw, flexibly-shaped filter values to normalize -- ``None`` means "no values given at
            all" (every position wildcarded)
        
        Raises
        ------
        :class:`ValueError`
            If this instance wasn't constructed with 'nonVersionIndexNames'
        
        Returns
        -------
        List[Optional[Any]]
            The normalized, positional filter values
        """
    def addMap(self, assetMap: dict, rows: typing.Any = []) -> None:
        """
        Merges new entries into the existing adjacency list (see :attr:`map`) -- for any 'fromAsset'
        already present, new 'toAsset' values are appended after the existing ones, skipping any that are
        already present
        
        Parameters
        ----------
        assetMap: Dict[Any, List[Any]]
            The new adjacency entries to merge in
        
        rows: Union[List[Tuple[List[Any], Any]], dict]
            Any new rows needed to support 'assetMap' -- either a flat list or a real nested dict --
            if non-empty, added to :attr:`repo` first (matches the pure-Python original's ``addMap``,
            whose own ``assets`` argument is a nested dict in exactly this same shape)
        
            **Default**: ``[]``
        """
    def addRepoRows(self, rows: typing.Any) -> None:
        """
        Adds new rows to :attr:`repo`, then rebuilds the reverse index to reflect them
        
        Parameters
        ----------
        rows: Union[List[Tuple[List[Any], Any]], dict]
            The rows to add -- either a flat list or a real nested dict -- see :meth:`ModDictAssets.addRows`
        """
    def get(self, nonVersionVals: collections.abc.Sequence[str], version: typing.Any = None, errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the corresponding asset -- forwards directly to :attr:`repo`'s own :meth:`ModDictAssets.get`
        """
    def getKey(self, asset: str, fromVersion: typing.Any = None, fromNonVersionVals: typing.Any = None, errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the key that produced 'asset', disambiguating between multiple candidates via
        'fromNonVersionVals' -- the first remaining candidate wins if more than one still matches after
        filtering
        
        Parameters
        ----------
        asset: Any
            The asset value to search for
        
        fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to search from -- see :meth:`hasFrom`
        
            **Default**: ``None``
        
        fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            The non-version value filter -- see :meth:`hasFrom`
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if no matching key is found
        
            **Default**: ``True``
        
        Raises
        ------
        :class:`KeyError`
            If no matching key is found and 'errorOnNotFound' is ``True``
        
        Returns
        -------
        Optional[Tuple[Any, ...]]
            The found key, or ``None`` if none is found and 'errorOnNotFound' is ``False`` -- deliberately
            just the key, not the version it was resolved at (matching the exact contract real callers
            like GIMIParser rely on; see the C++ core's own note on this)
        """
    def hasFrom(self, asset: str, version: typing.Any = None, nonVersionVals: typing.Any = None) -> bool:
        """
        Determines whether 'asset' exists in the assets to map from
        
        Parameters
        ----------
        asset: Any
            The asset to search for
        
        version: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to search from -- the latest available version is used if this is ``None``
        
            **Default**: ``None``
        
        nonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            A per-position filter over the candidate keys' non-version index values -- ``None`` at a
            position means "match any value there"; ``None`` for the whole argument means "no filtering
            at all". If :attr:`nonVersionIndexNames` was given, also accepts a bare value (filters only
            the first position) or a dict keyed by index name
        
            **Default**: ``None``
        """
    def replace(self, asset: str, fromVersion: typing.Any = None, fromNonVersionVals: typing.Any = None, toVersion: typing.Any = None, toAssetName: str, errorOnNotFound: bool = True) -> typing.Any:
        """
        Retrieves the single corresponding asset to replace 'asset' with, for one specific target asset
        name
        
        Parameters
        ----------
        asset: Any
            The asset to be replaced
        
        fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to replace from -- see :meth:`getKey`
        
            **Default**: ``None``
        
        fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[str, Any]]]
            The non-version value filter -- see :meth:`getKey`
        
            **Default**: ``None``
        
        toVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to replace to -- the latest available version is used if this is ``None``
        
            **Default**: ``None``
        
        toAssetName: Any
            The specific name of the asset to map to
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if 'asset' (or a mapping for it) isn't found at all --
            once past that point, "toAssetName isn't actually mapped from asset's name" or "no data
            exists for it at the queried version" always just returns ``None``, regardless of this flag
        
            **Default**: ``True``
        
        Returns
        -------
        Any
            The replacement asset, or ``None`` if none is found
        """
    def replaceAll(self, asset: str, fromVersion: typing.Any = None, fromNonVersionVals: typing.Any = None, toVersion: typing.Any = None, toAssetNames: typing.Any = None, errorOnNotFound: bool = True) -> dict:
        """
        Retrieves every corresponding asset to replace 'asset' with
        
        Parameters
        ----------
        asset: Any
            The asset to be replaced
        
        fromVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to replace from -- see :meth:`getKey`
        
            **Default**: ``None``
        
        fromNonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[str, Any]]]
            The non-version value filter -- see :meth:`getKey`
        
            **Default**: ``None``
        
        toVersion: Optional[Union[:class:`str`, :class:`int`, :class:`float`, :class:`CppVersion`]]
            The version to replace to -- the latest available version is used if this is ``None``
        
            **Default**: ``None``
        
        toAssetNames: Optional[List[Any]]
            The specific names of the assets to map to -- every asset name 'asset' maps to is used if
            this is ``None``
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if 'asset' (or a mapping for it) isn't found at all --
            see :meth:`replace`'s note on this parameter
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[Any, Any]
            The corresponding assets for the fix to replace, keyed by asset name -- empty if nothing is
            found
        """
    @property
    def fixFrom(self) -> set:
        """
        Set[Any]: Always empty -- matches the pure-Python original, which declares this but never
        populates it anywhere
        """
    @property
    def fixTo(self) -> set:
        """
        Set[Any]: Always empty -- matches the pure-Python original, which declares this but never
        populates it anywhere
        """
    @property
    def fromAssets(self) -> list[str]:
        """
        List[Any]: Every asset value that has at least one known originating key -- a property (not a
        method), matching the pure-Python original's contract exactly (real callers, e.g. IniFile.py's
        ``type.hashes.fromAssets``, access it as one)
        """
    @property
    def map(self) -> dict:
        """
        Dict[Any, List[Any]]: The adjacency list mapping assets to fix from to assets to fix to
        """
    @property
    def nonVersionIndexNames(self) -> typing.Any:
        """
        Optional[List[:class:`str`]]: The names of the non-version index columns, in position order --
        ``None`` if this instance wasn't constructed with them (see the constructor's own note)
        """
    @property
    def repo(self) -> ModDictAssets:
        """
        :class:`ModDictAssets`: The underlying asset data
        """
class ModType:
    """
    
    Heavy data for a type of mod
    
    Meant to carry the full C++-side representation of a mod type -- contrast with the cheap
    :class:`ModTypeIdData` an ini classifier (e.g. :class:`BaseIniClassifier`) holds instead. The
    Python-side :class:`ModType` is meant to build itself using this data.
    
    Parameters
    ----------
    gameTypeId: :class:`int`
        The id for the game this type of mod belongs to -- stored as-is, with no validation that it
        corresponds to one of :class:`GameTypeId`'s declared values (see :class:`GameTypeIdTools` if
        that's needed)
    
    modTypeId: :class:`int`
        The id for this specific type of mod -- stored as-is, with no validation that it corresponds
        to one of :class:`ModTypeId`'s declared values (see :class:`ModTypeIdTools` if that's needed),
        so a custom mod type using some id not registered in :class:`ModTypeId` can still be represented
    
    name: :class:`str`
        The default name for the type of mod
    
    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``[]``
        
    """
    def __init__(self, gameTypeId: typing.SupportsInt | typing.SupportsIndex, modTypeId: typing.SupportsInt | typing.SupportsIndex, name: str, aliases: collections.abc.Sequence[str] = [], hashes: Hashes = None, indices: Indices = None, vertexCounts: VertexCounts = None, vgRemaps: VGRemaps = None) -> None:
        ...
    def fixIni(self, iniFile: IniFile, keepBackup: bool = True, fixOnly: bool = False) -> None:
        """
        Fixes a .ini file, but **only if that file was classified as this mod type** -- a no-op otherwise
        
        Returns nothing, matching the pure-Python original: the fix it produces is written out by
        :meth:`IniFile.fix` rather than handed back. Call that directly to see it
        
        Parameters
        ----------
        iniFile: :class:`IniFile`
            The .ini file to fix
        
        keepBackup: :class:`bool`
            Whether to keep a backup copy of the original .ini file :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        fixOnly: :class:`bool`
            Whether to only fix without removing any previous fix :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        """
    def getHashRanges(self, partColours: IfContentPartColouring, version: FixRaidenBoss2.core.CppVersion | None = None, nonVersionVals: typing.Any = None) -> Ranges:
        """
        Retrieves the valid ranges of order indices within an :class:`IfContentPart` whose ``hash`` values
        belong to this mod type
        
        Parameters
        ----------
        partColours: :class:`IfContentPartColouring`
            The current states of the :class:`IfContentPart`
        
        version: Optional[:class:`CppVersion`]
            The version the hashes should come from :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning any
        
        nonVersionVals: Optional[List[Optional[:class:`str`]]]
            Values for the non-version index columns, used to narrow which instance of a hash is wanted
            :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`Ranges`
            The valid ranges of indices
        """
    def getHelpStr(self) -> str:
        """
        Retrieves the help text describing this mod type, as the CLI prints it
        
        Returns
        -------
        :class:`str`
            The help text
        """
    def getModsToFix(self) -> set[str]:
        """
        Retrieves the names of the mods this mod type can be fixed onto
        
        .. warning::
            **Deliberately not bug-compatible with the pure-Python** :meth:`ModType.getModsToFix`. That one
            unions ``hashes.fixTo`` and ``indices.fixTo`` -- two sets it declares and then never populates
            anywhere, so it returns an empty set for every mod type, always. This reads the remap targets
            that actually exist
        
        Returns
        -------
        Set[:class:`str`]
            The names of the mods to fix to
        """
    def getVGRemap(self, modName: str, fromVersion: FixRaidenBoss2.core.CppVersion | None = None, toVersion: FixRaidenBoss2.core.CppVersion | None = None, fromComp: str | None = None, toComp: str | None = None) -> FixRaidenBoss2.core.VGRemap | None:
        """
        Retrieves the vertex group remap for fixing this mod type onto another
        
        Parameters
        ----------
        modName: :class:`str`
            The name of the mod being fixed onto
        
        fromVersion: Optional[:class:`CppVersion`]
            The version being fixed from :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning the latest
        
        toVersion: Optional[:class:`CppVersion`]
            The version being fixed to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning the latest
        
        fromComp: Optional[:class:`str`]
            The component being fixed from. ``None`` leaves the column unconstrained :raw-html:`<br />`
            :raw-html:`<br />`
        
            **Default**: ``None``
        
        toComp: Optional[:class:`str`]
            The component being fixed onto, with the same ``None`` meaning :raw-html:`<br />`
            :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        Optional[:class:`VGRemap`]
            The remap, or ``None`` if the table has no matching row
        """
    def getVertexCount(self, version: FixRaidenBoss2.core.CppVersion | None = None) -> int | None:
        """
        Retrieves the number of vertices for this mod
        
        Parameters
        ----------
        version: Optional[:class:`CppVersion`]
            The game version wanted :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning the latest
        
        Returns
        -------
        Optional[:class:`int`]
            The vertex count, or ``None`` if this mod type has no row for it
        """
    def isName(self, name: str) -> bool:
        """
        Determines whether this mod type goes by some name
        
        Compared case-insensitively against :attr:`ModType.name` and every entry in
        :attr:`ModType.aliases`
        
        Parameters
        ----------
        name: :class:`str`
            The name to check
        
        Returns
        -------
        :class:`bool`
            Whether this mod type goes by 'name'
        """
    @property
    def aliases(self) -> list[str]:
        """
        List[:class:`str`]: Other alternative names for the type of mod
        """
    @aliases.setter
    def aliases(self, arg0: collections.abc.Sequence[str]) -> None:
        ...
    @property
    def gameTypeId(self) -> int:
        """
        :class:`int`: The id for the game this type of mod belongs to
        """
    @gameTypeId.setter
    def gameTypeId(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def hashes(self) -> Hashes:
        """
        :class:`Hashes`: The hashes related to the mod and its fix
        
        A :class:`ModType` constructed without one gets a **fully-populated** :class:`Hashes` -- every hash
        the software ships with -- not an empty table
        
        .. note::
            Shared, not copied: two :class:`ModType`\\s handed the same table both see any
            :meth:`ModMappedAssets.addRepoRows`/:meth:`ModMappedAssets.addMap` made through either of them
        """
    @hashes.setter
    def hashes(self, arg0: Hashes) -> None:
        ...
    @property
    def indices(self) -> Indices:
        """
        :class:`Indices`: The indices related to the mod and its fix
        
        Same defaulting and sharing rules as :attr:`hashes`
        """
    @indices.setter
    def indices(self, arg0: Indices) -> None:
        ...
    @property
    def iniFixBuilder(self) -> IniFixBuilder:
        """
        :class:`IniFixBuilder`: The builder for the fixer that fixes a .ini file of this mod type
        """
    @iniFixBuilder.setter
    def iniFixBuilder(self, arg0: IniFixBuilder) -> None:
        ...
    @property
    def iniParseBuilder(self) -> IniParseBuilder:
        """
        :class:`IniParseBuilder`: The builder for the parser that reads a .ini file of this mod type
        """
    @iniParseBuilder.setter
    def iniParseBuilder(self, arg0: IniParseBuilder) -> None:
        ...
    @property
    def iniRemoveBuilder(self) -> IniRemoveBuilder:
        """
        :class:`IniRemoveBuilder`: The builder for the remover that removes a previous fix
        """
    @iniRemoveBuilder.setter
    def iniRemoveBuilder(self, arg0: IniRemoveBuilder) -> None:
        ...
    @property
    def modTypeId(self) -> int:
        """
        :class:`int`: The id for this specific type of mod
        """
    @modTypeId.setter
    def modTypeId(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def name(self) -> str:
        """
        :class:`str`: The default name for the type of mod
        """
    @name.setter
    def name(self, arg0: str) -> None:
        ...
    @property
    def vertexCounts(self) -> VertexCounts:
        """
        :class:`VertexCounts`: The vertex counts related to the mod
        
        Same defaulting and sharing rules as :attr:`hashes`
        """
    @vertexCounts.setter
    def vertexCounts(self, arg0: VertexCounts) -> None:
        ...
    @property
    def vgRemaps(self) -> VGRemaps:
        """
        :class:`VGRemaps`: The vertex group remaps for the mod
        
        .. warning::
            Unlike :attr:`hashes`/:attr:`indices`/:attr:`vertexCounts`, the default here is the **shared**
            table every mod type uses, not a fresh one -- so mutating a defaulted :attr:`vgRemaps` is
            visible to every other mod type that also defaulted. That mirrors the pure-Python original's
            own ``ModDataAssets.VGRemaps.value`` default
        """
    @vgRemaps.setter
    def vgRemaps(self, arg0: VGRemaps) -> None:
        ...
class ModTypeId:
    """
    
    The names of the different types of mods this fix will fix from or fix to
    
    Mirrors the keys of the pure-Python ``ModTypeNames`` enum (``constants/ModTypeNames.py``)
        
    
    Members:
    
      Amber : Amber from GI
    
      AmberCN : Amber Chinese version from GI
    
      Ayaka : Ayaka from GI
    
      AyakaSpringbloom : Ayaka Fontaine skin from GI
    
      Arlecchino : Arlecchino from GI
    
      ArlecchinoBoss : The first phase of the Arlecchino boss from GI
    
      Barbara : Barbara from GI
    
      BarbaraSummertime : Barbara summer skin from GI
    
      CherryHuTao : Hu Tao Lantern Rite skin from GI
    
      Diluc : Diluc from GI
    
      DilucFlamme : Diluc Red Dead of the Night skin from GI
    
      Fischl : Fischl from GI
    
      FischlHighness : Fischl summer skin from GI
    
      Ganyu : Ganyu from GI
    
      GanyuTwilight : Ganyu Lantern Rite skin from GI
    
      HuTao : HuTao from GI
    
      Jean : Jean from GI
    
      JeanCN : Jean Chinese version from GI
    
      JeanSea : Jean summer skin from GI
    
      Kaeya : Kaeya from GI
    
      KaeyaSailwind : KaeyaSailwind from GI
    
      Keqing : Keqing from GI
    
      KeqingOpulent : Keqing Lantern Rite skin from GI
    
      Kirara : Kirara from GI
    
      KiraraBoots : Kirara summer skin from GI
    
      Klee : Klee from GI
    
      KleeBlossomingStarlight : Klee summer skin from GI
    
      Lisa : Lisa from GI
    
      LisaStudent : Lisa Sumeru skin from GI
    
      Mona : Mona from GI
    
      MonaCN : Mona Chinese version from GI
    
      Nilou : Nilou from GI
    
      NilouBreeze : Nilou summer skin from GI
    
      Ningguang : Ningguang from GI
    
      NingguangOrchid : Ningguang Lantern Rite from GI
    
      Raiden : Ei from GI
    
      RaidenBoss : The first phase of the Raiden Shogun boss from GI
    
      Rosaria : Rosaria from GI
    
      RosariaCN : Rosaria Chinese version from GI
    
      Shenhe : Shenhe from GI
    
      ShenheFrostFlower : Shenhe Lantern Rite skin from GI
    
      Xiangling : Xiangling from GI
    
      XianglingCheer : Xiangling Lantern Rite skin from GI
    
      Xingqiu : Xingqiu from GI
    
      XingqiuBamboo : Xingqiu Lantern Rite skin from GI
    """
    Amber: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Amber: 0>
    AmberCN: typing.ClassVar[ModTypeId]  # value = <ModTypeId.AmberCN: 1>
    Arlecchino: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Arlecchino: 4>
    ArlecchinoBoss: typing.ClassVar[ModTypeId]  # value = <ModTypeId.ArlecchinoBoss: 5>
    Ayaka: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Ayaka: 2>
    AyakaSpringbloom: typing.ClassVar[ModTypeId]  # value = <ModTypeId.AyakaSpringbloom: 3>
    Barbara: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Barbara: 6>
    BarbaraSummertime: typing.ClassVar[ModTypeId]  # value = <ModTypeId.BarbaraSummertime: 7>
    CherryHuTao: typing.ClassVar[ModTypeId]  # value = <ModTypeId.CherryHuTao: 8>
    Diluc: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Diluc: 9>
    DilucFlamme: typing.ClassVar[ModTypeId]  # value = <ModTypeId.DilucFlamme: 10>
    Fischl: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Fischl: 11>
    FischlHighness: typing.ClassVar[ModTypeId]  # value = <ModTypeId.FischlHighness: 12>
    Ganyu: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Ganyu: 13>
    GanyuTwilight: typing.ClassVar[ModTypeId]  # value = <ModTypeId.GanyuTwilight: 14>
    HuTao: typing.ClassVar[ModTypeId]  # value = <ModTypeId.HuTao: 15>
    Jean: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Jean: 16>
    JeanCN: typing.ClassVar[ModTypeId]  # value = <ModTypeId.JeanCN: 17>
    JeanSea: typing.ClassVar[ModTypeId]  # value = <ModTypeId.JeanSea: 18>
    Kaeya: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Kaeya: 19>
    KaeyaSailwind: typing.ClassVar[ModTypeId]  # value = <ModTypeId.KaeyaSailwind: 20>
    Keqing: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Keqing: 21>
    KeqingOpulent: typing.ClassVar[ModTypeId]  # value = <ModTypeId.KeqingOpulent: 22>
    Kirara: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Kirara: 23>
    KiraraBoots: typing.ClassVar[ModTypeId]  # value = <ModTypeId.KiraraBoots: 24>
    Klee: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Klee: 25>
    KleeBlossomingStarlight: typing.ClassVar[ModTypeId]  # value = <ModTypeId.KleeBlossomingStarlight: 26>
    Lisa: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Lisa: 27>
    LisaStudent: typing.ClassVar[ModTypeId]  # value = <ModTypeId.LisaStudent: 28>
    Mona: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Mona: 29>
    MonaCN: typing.ClassVar[ModTypeId]  # value = <ModTypeId.MonaCN: 30>
    Nilou: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Nilou: 31>
    NilouBreeze: typing.ClassVar[ModTypeId]  # value = <ModTypeId.NilouBreeze: 32>
    Ningguang: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Ningguang: 33>
    NingguangOrchid: typing.ClassVar[ModTypeId]  # value = <ModTypeId.NingguangOrchid: 34>
    Raiden: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Raiden: 35>
    RaidenBoss: typing.ClassVar[ModTypeId]  # value = <ModTypeId.RaidenBoss: 36>
    Rosaria: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Rosaria: 37>
    RosariaCN: typing.ClassVar[ModTypeId]  # value = <ModTypeId.RosariaCN: 38>
    Shenhe: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Shenhe: 39>
    ShenheFrostFlower: typing.ClassVar[ModTypeId]  # value = <ModTypeId.ShenheFrostFlower: 40>
    Xiangling: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Xiangling: 41>
    XianglingCheer: typing.ClassVar[ModTypeId]  # value = <ModTypeId.XianglingCheer: 42>
    Xingqiu: typing.ClassVar[ModTypeId]  # value = <ModTypeId.Xingqiu: 43>
    XingqiuBamboo: typing.ClassVar[ModTypeId]  # value = <ModTypeId.XingqiuBamboo: 44>
    __members__: typing.ClassVar[dict[str, ModTypeId]]  # value = {'Amber': <ModTypeId.Amber: 0>, 'AmberCN': <ModTypeId.AmberCN: 1>, 'Ayaka': <ModTypeId.Ayaka: 2>, 'AyakaSpringbloom': <ModTypeId.AyakaSpringbloom: 3>, 'Arlecchino': <ModTypeId.Arlecchino: 4>, 'ArlecchinoBoss': <ModTypeId.ArlecchinoBoss: 5>, 'Barbara': <ModTypeId.Barbara: 6>, 'BarbaraSummertime': <ModTypeId.BarbaraSummertime: 7>, 'CherryHuTao': <ModTypeId.CherryHuTao: 8>, 'Diluc': <ModTypeId.Diluc: 9>, 'DilucFlamme': <ModTypeId.DilucFlamme: 10>, 'Fischl': <ModTypeId.Fischl: 11>, 'FischlHighness': <ModTypeId.FischlHighness: 12>, 'Ganyu': <ModTypeId.Ganyu: 13>, 'GanyuTwilight': <ModTypeId.GanyuTwilight: 14>, 'HuTao': <ModTypeId.HuTao: 15>, 'Jean': <ModTypeId.Jean: 16>, 'JeanCN': <ModTypeId.JeanCN: 17>, 'JeanSea': <ModTypeId.JeanSea: 18>, 'Kaeya': <ModTypeId.Kaeya: 19>, 'KaeyaSailwind': <ModTypeId.KaeyaSailwind: 20>, 'Keqing': <ModTypeId.Keqing: 21>, 'KeqingOpulent': <ModTypeId.KeqingOpulent: 22>, 'Kirara': <ModTypeId.Kirara: 23>, 'KiraraBoots': <ModTypeId.KiraraBoots: 24>, 'Klee': <ModTypeId.Klee: 25>, 'KleeBlossomingStarlight': <ModTypeId.KleeBlossomingStarlight: 26>, 'Lisa': <ModTypeId.Lisa: 27>, 'LisaStudent': <ModTypeId.LisaStudent: 28>, 'Mona': <ModTypeId.Mona: 29>, 'MonaCN': <ModTypeId.MonaCN: 30>, 'Nilou': <ModTypeId.Nilou: 31>, 'NilouBreeze': <ModTypeId.NilouBreeze: 32>, 'Ningguang': <ModTypeId.Ningguang: 33>, 'NingguangOrchid': <ModTypeId.NingguangOrchid: 34>, 'Raiden': <ModTypeId.Raiden: 35>, 'RaidenBoss': <ModTypeId.RaidenBoss: 36>, 'Rosaria': <ModTypeId.Rosaria: 37>, 'RosariaCN': <ModTypeId.RosariaCN: 38>, 'Shenhe': <ModTypeId.Shenhe: 39>, 'ShenheFrostFlower': <ModTypeId.ShenheFrostFlower: 40>, 'Xiangling': <ModTypeId.Xiangling: 41>, 'XianglingCheer': <ModTypeId.XianglingCheer: 42>, 'Xingqiu': <ModTypeId.Xingqiu: 43>, 'XingqiuBamboo': <ModTypeId.XingqiuBamboo: 44>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class ModTypeIdData:
    """
    
    Cheap data for a type of mod, held by an ini classifier (e.g. :class:`BaseIniClassifier`)
    
    Not meant to be a full representation of a mod type on its own -- the Python-side :class:`ModType`
    is meant to build its own richer representation from this data
    
    Parameters
    ----------
    gameTypeId: :class:`int`
        The id for the game this type of mod belongs to -- stored as-is, with no validation that it
        corresponds to one of :class:`GameTypeId`'s declared values (see :class:`GameTypeIdTools` if
        that's needed)
    
    modTypeId: :class:`int`
        The id for this specific type of mod -- stored as-is, with no validation that it corresponds
        to one of :class:`ModTypeId`'s declared values (see :class:`ModTypeIdTools` if that's needed),
        so a custom mod type using some id not registered in :class:`ModTypeId` can still be represented
        
    """
    def __init__(self, gameTypeId: typing.SupportsInt | typing.SupportsIndex, modTypeId: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def gameTypeId(self) -> int:
        """
        :class:`int`: The id for the game this type of mod belongs to
        """
    @gameTypeId.setter
    def gameTypeId(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def modTypeId(self) -> int:
        """
        :class:`int`: The id for this specific type of mod
        """
    @modTypeId.setter
    def modTypeId(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class ModTypeIdTools:
    """
    
    Tools for handling :class:`ModTypeId`
        
    """
    @staticmethod
    def clear() -> None:
        """
        Clears the global registry -- every :class:`ModType` registered via :meth:`registerModType` is
        forgotten, and :meth:`getModType`/:meth:`findByName` behave as if nothing was ever registered
        
        Mirrors :meth:`HashTools.clear`/:meth:`CppHashTools.clear` -- meant for resetting shared global
        state between independent uses (e.g. between unit tests)
        
        .. note::
            The next thing to ask for the default classifier re-files the shipped mod types (see
            :meth:`GlobalModTypes.registerMissing`), so clearing does not permanently break classification
        """
    @staticmethod
    def findByName(name: str, gameTypeId: FixRaidenBoss2.core.GameTypeId | None = None) -> FixRaidenBoss2.core.ModTypeId | None:
        """
        Finds the :class:`ModTypeId` whose registered :class:`ModType` name or alias maximally matches
        some string, similar to how :meth:`BaseIniClassifier.classify`'s section-name reading searches
        its own registered keywords
        
        Only searches names/aliases of :class:`ModType` s that have actually been registered (via
        :meth:`registerModType`) -- an unregistered :class:`ModTypeId` can never be found this way, even if
        'name' textually matches what :meth:`getName` would return for it :raw-html:`<br />` :raw-html:`<br />`
        
        If more than one registered :class:`ModTypeId` shares the maximally-matched name (or alias) -- after
        filtering by 'gameTypeId', when given -- the match is ambiguous and ``None`` is returned rather than
        guessing
        
        Parameters
        ----------
        name: :class:`str`
            The string to search for a registered :class:`ModType` name/alias within
        
        gameTypeId: Optional[:class:`GameTypeId`]
            If provided, only considers a :class:`ModType` registered under this :class:`GameTypeId` (via
            ``modType.gameTypeId``) a candidate match :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        Optional[:class:`ModTypeId`]
            The matched :class:`ModTypeId`, if exactly one unambiguous match was found
        """
    @staticmethod
    def generation() -> int:
        """
        :class:`int`: How many times the registry has been emptied by :meth:`clear`
        
        Starts at ``1`` and is bumped by every :meth:`clear`, so a caller that populated the registry can
        tell whether the registry it populated is still the one being read. Deliberately not bumped by
        :meth:`registerModType` -- it counts invalidations, not writes
        """
    @staticmethod
    def getEnum(value: typing.SupportsInt | typing.SupportsIndex) -> FixRaidenBoss2.core.ModTypeId | None:
        """
        Retrieves the corresponding :class:`ModTypeId` for some integer value, checking that the value
        actually corresponds to one of :class:`ModTypeId`'s declared values
        
        Parameters
        ----------
        value: :class:`int`
            The integer value to convert
        
        Returns
        -------
        Optional[:class:`ModTypeId`]
            The corresponding :class:`ModTypeId`, if 'value' is valid
        """
    @staticmethod
    def getHashRemapTargets(value: ModTypeId) -> list[ModTypeId]:
        """
        Retrieves the mod types a given mod type's **hashes** can be remapped onto
        
        This is the remap graph itself. It mirrors the ``map`` argument the pure-Python :class:`GIBuilder`
        passes to each mod type's :class:`Hashes`, lifted out of the 43 individual factories into one table
        so a target is named by :class:`ModTypeId` rather than by a bare string
        
        .. note::
            Two :class:`ModTypeId`\\s -- ``RaidenBoss`` and ``ArlecchinoBoss`` -- only ever appear as
            *targets* and are never a source, which is why :class:`GIBuilder` has no factory for them
        
        Parameters
        ----------
        value: :class:`ModTypeId`
            The mod type to look up the remap targets of
        
        Returns
        -------
        List[:class:`ModTypeId`]
            The mod types 'value' remaps onto, or an empty list if it remaps onto none
        """
    @staticmethod
    def getIndexRemapTargets(value: ModTypeId) -> list[ModTypeId]:
        """
        Retrieves the mod types a given mod type's **indices** can be remapped onto
        
        Identical to :meth:`getHashRemapTargets` for every mod type but one: ``Raiden`` remaps by hash only
        
        Parameters
        ----------
        value: :class:`ModTypeId`
            The mod type to look up the remap targets of
        
        Returns
        -------
        List[:class:`ModTypeId`]
            The mod types 'value' remaps onto, or an empty list if it remaps onto none
        """
    @staticmethod
    def getModType(modTypeId: typing.SupportsInt | typing.SupportsIndex) -> ... | None:
        """
        Retrieves the :class:`ModType` registered for a :class:`ModTypeId`, if one has been registered
        (via :meth:`registerModType`)
        
        This is a plain lookup into a global registry shared by every caller of :class:`ModTypeIdTools` --
        it never builds a :class:`ModType` itself. If a :class:`ModTypeId` is never registered, nothing
        is ever built for it, since building one can be expensive; only a :class:`ModTypeId` that's actually
        been registered (typically by whichever builder -- e.g. :class:`GIBuilder` -- actually owns it)
        can be retrieved here
        
        Parameters
        ----------
        modTypeId: :class:`int`
            The integer id for the :class:`ModTypeId` to retrieve the registered :class:`ModType` for --
            stored/looked-up as-is, with no validation that it corresponds to one of :class:`ModTypeId`'s
            declared values, so a custom mod type using some id not registered in :class:`ModTypeId` can
            still be looked up here
        
        Returns
        -------
        Optional[:class:`ModType`]
            The registered :class:`ModType`, if one exists for 'modTypeId'
        """
    @staticmethod
    def getName(value: ModTypeId) -> str:
        """
        Retrieves the corresponding name for a :class:`ModTypeId`
        
        Parameters
        ----------
        value: :class:`ModTypeId`
            The :class:`ModTypeId` to retrieve the name for
        
        Returns
        -------
        :class:`str`
            The name for 'value'
        """
    @staticmethod
    def registerModType(modType: ...) -> None:
        """
        Registers a :class:`ModType` into the global registry, under the :class:`ModTypeId` it owns
        (``modType.modTypeId``) :raw-html:`<br />` :raw-html:`<br />`
        
        If a :class:`ModType` is already registered for that :class:`ModTypeId`, it gets overwritten with
        the new one
        
        Parameters
        ----------
        modType: :class:`ModType`
            The :class:`ModType` to register
        """
class MultiModFixer(BaseIniFixer):
    """
    
    This class inherits from :class:`BaseIniFixer`
    
    A fixer that owns no fixing logic of its own and instead **delegates to one child fixer per mod
    type**, keyed by the :class:`int` value of that mod type's :class:`ModTypeId`
    
    Which children run is decided by the .ini file's :attr:`IniFile.filteredToModTypeIds`: a child
    whose id is absent from that filter is skipped. ``None`` there means no filter, so every child
    runs -- an **empty set** is deliberately different, and selects nothing
    
    .. note::
        A child is a plain :class:`BaseIniFixer`, and this class *is* one, so a :class:`MultiModFixer`
        can be a child of another and the nesting composes. The :class:`IniFixingContext` handed to a
        child is **narrowed** rather than replaced, so a nested fixer's first child holds the .ini
        file's first word only if its parent was told the same
    
    .. note::
        Children run in **ascending id order**, not the order of the dict they were given in. Only one
        child may take the .ini file's backup and only one may hide the original mod, so which is
        "first" and which is "last" has to be answerable the same way twice
    
    Parameters
    ----------
    children: Optional[Dict[:class:`int`, :class:`BaseIniFixer`]]
        The child fixers, keyed by the :class:`int` value of the :class:`ModTypeId` each one fixes. A
        ``None`` child is skipped rather than treated as an error :raw-html:`<br />`
        :raw-html:`<br />`
    
        **Default**: ``None``, meaning no children -- which makes fixing a no-op
    
    parser: Optional[:class:`BaseIniParser`]
        The parser to retrieve the data to fix from :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, children: typing.Any = None, parser: typing.Any = None) -> None:
        ...
    @property
    def children(self) -> dict:
        """
        Dict[:class:`int`, :class:`BaseIniFixer`]: The child fixers, keyed by mod type id
        """
    @children.setter
    def children(self, arg1: typing.Any) -> None:
        ...
class OrderedMultiMap:
    """
    
    An ordered multimap implemented in C++: preserves insertion/positional order, allows duplicate
    keys, and gives both fast key-based access and fast positional access. Backed by a plain linked
    list -- positional access walks from whichever end is closer to the requested index.
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines whether 'key' exists
    
        .. describe:: len(x)
    
            Retrieves the number of entries
    
        .. describe:: x[index]
    
            Retrieves the ``(key, value)`` pair at the given true positional index
    
        .. describe:: iter(x)
    
            Iterates every entry in true positional order, yielding ``(key, value,
            occurrenceIndex, orderIndex)`` tuples
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
        deep copy (equivalent to ``x.copy()``)
    
    Parameters
    ----------
    items: Optional[List[Tuple[Any, Any]]]
        Key-value pairs to insert at the end, in order :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None`` (an empty instance)
            
    """
    @staticmethod
    def fromIndexed(indexed: dict) -> OrderedMultiMap:
        """
        Builds an instance from a fully-indexed description: for each key, a list of ``(index, value)``
        pairs. The index is treated as a sort key, not a strict absolute position: every ``(index, key,
        value)`` triple across every key is gathered, stable-sorted by index ascending, and inserted in
        that order -- gaps and out-of-order values just determine relative order, and duplicate indices
        land consecutively (tie-broken by encounter order: list order within a key, then 'indexed's own
        dict order across different keys).
        
        Parameters
        ----------
        indexed: Dict[Any, List[Tuple[:class:`int`, Any]]]
            The key -> list of ``(index, value)`` pairs to build from
        
        Returns
        -------
        :class:`OrderedMultiMap`
            The newly-built instance
        """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> OrderedMultiMap:
        """
        Creates a copy of this instance (equivalent to :meth:`copy`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> OrderedMultiMap:
        """
        Creates a deep copy of this instance (equivalent to :meth:`copy`); supports ``copy.deepcopy()``
        """
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def __init__(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]] | None = None) -> None:
        ...
    def __iter__(self) -> OrderedMultiMapIterator:
        """
        Iterates every entry in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def __len__(self) -> int:
        """
        Retrieves the number of entries
        """
    def asInterface(self) -> ...:
        """
        Creates an independent snapshot of this instance, viewed through the generic
        :class:`IOrderedMultiMap` interface -- like :meth:`copy`, this is a deep copy; mutating the
        result does not affect this instance (or vice versa)
        
        Returns
        -------
        :class:`IOrderedMultiMap`
            An independent :class:`IOrderedMultiMap`-typed snapshot of this instance
        """
    def contains(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def containsKey(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def copy(self) -> OrderedMultiMap:
        """
        Creates a deep copy of this instance -- rebuilt entry-by-entry, so the copy shares no internal
        state with the original
        
        Returns
        -------
        :class:`OrderedMultiMap`
            The newly-created copy
        """
    def count(self, key: typing.Any) -> int:
        """
        Retrieves how many entries share a given key
        """
    def empty(self) -> bool:
        """
        Checks whether the map is empty
        """
    def entries(self) -> list[tuple[typing.Any, typing.Any]]:
        """
        Retrieves read-only access to the full ordered sequence
        
        Returns
        -------
        List[Tuple[Any, Any]]
            The full ordered sequence of ``(key, value)`` pairs
        """
    def getAll(self, key: typing.Any, ordered: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> list[typing.Any]:
        """
        Retrieves all values currently stored under a key
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Any]
            The values for this key, in the requested order
        """
    def getAllWithInds(self, key: typing.Any, ordered: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> list[tuple[int, typing.Any]]:
        """
        Retrieves all values currently stored under a key, each paired with its true positional index
        (equivalent to :meth:`getAll`, except each value is paired with its true positional index)
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Tuple[:class:`int`, Any]]
            The ``(order index, value)`` pairs for this key, in the requested order
        """
    def getByInd(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the entry at a true positional index
        
        Parameters
        ----------
        index: :class:`int`
            The position to retrieve. Python-style negative indices are supported
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        
        Returns
        -------
        Tuple[Any, Any]
            The ``(key, value)`` pair at that position
        """
    def getByIndWithOccurrence(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[int, typing.Any]:
        """
        Retrieves the entry at a true positional index, paired with its occurrence index (how many
        times this same key already appeared earlier in the sequence, 0-based) instead of its key
        (equivalent to :meth:`getByInd`, except the entry's value is paired with its occurrence index)
        
        Parameters
        ----------
        index: :class:`int`
            The position to retrieve. Python-style negative indices are supported
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        
        Returns
        -------
        Tuple[:class:`int`, Any]
            The ``(occurrence index, value)`` pair at that position
        """
    def getKeys(self) -> set[typing.Any]:
        """
        Retrieves every distinct key currently in the map
        
        Returns
        -------
        Set[Any]
            Every distinct key, as a set (unordered)
        """
    def insert(self, key: typing.Any, value: typing.Any) -> None:
        """
        Appends a key-value pair to the end
        """
    def insertAllAt(self, items: dict, sortIndices: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> int:
        """
        Bulk indexed insert: inserts many key-value pairs at their own target indices in a single pass.
        Index semantics match :meth:`insertAt` (Python-style negative indices, clamping), but with
        "original position" (numpy-style) semantics: each index refers to a position in the sequence as
        it was *before* this call, not a position in the growing result.
        
        Parameters
        ----------
        items: Dict[:class:`int`, Tuple[Any, Any]]
            Maps an index to insert at -> the key-value pair to insert there
        
        sortIndices: :class:`bool`
            If ``True`` (the default), 'items' is stable-sorted by normalized index first. If you
            already know 'items' iterates in ascending normalized-index order, pass ``False`` to skip
            that sort -- **this precondition is unchecked**, and violating it produces a silently wrong
            (not crashing) result
        
        ranges: Optional[:class:`Ranges`]
            If provided, an entry is only inserted when its normalized target index falls within
            'ranges'; filtered entries are dropped before sorting/the insertion pass :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`int`
            How many entries were actually inserted
        """
    def insertAllEnd(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Appends a batch of key-value pairs to the end, in the order given
        """
    def insertAllStart(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Inserts a batch of key-value pairs at the beginning, in the order given -- ``items[0]`` ends up
        first, ``items[1]`` right after it, and so on, all before whatever was originally at the front
        """
    def insertAt(self, index: typing.SupportsInt | typing.SupportsIndex, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair so it ends up at position 'index' (0-based) :raw-html:`<br />` :raw-html:`<br />`
        
        Supports Python-style negative indices. Out-of-range indices are clamped rather than rejected:
        an index greater than ``len(self)`` is treated as ``len(self)`` (append); an index less than
        ``-(len(self) + 1)`` is treated as ``-(len(self) + 1)`` (front)
        
        Parameters
        ----------
        index: :class:`int`
            The target position
        
        key: Any
            The key of the pair to insert
        
        value: Any
            The value of the pair to insert
        """
    def insertStart(self, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair at the beginning
        """
    def keySize(self) -> int:
        """
        Retrieves the number of distinct keys
        """
    def length(self) -> int:
        """
        Retrieves the number of entries
        """
    def remapKeys(self, keyRemap: dict, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Bulk-renames keys. 'keyRemap' maps an old key -> either a bare list of keys/
        :class:`RemappedKeyData`, or a :class:`KeyRemapData`. :raw-html:`<br />` :raw-html:`<br />`
        
        For every existing entry, walked in true positional order: if its key is not a key in
        'keyRemap', it's left completely unchanged. Otherwise, each rule in the mapped list is evaluated
        independently against this occurrence -- a plain key always fires, a :class:`RemappedKeyData`
        fires if it has no ``check``, or ``check(oldKey, oldValue)`` is ``True``. Every rule that fires
        produces one new entry (that rule's key, this occurrence's original value); a
        :class:`RemappedKeyData` with ``toInd`` set instead moves its entry (as part of a group with
        every other entry sharing that same ``toInd`` across every occurrence) to that target index,
        using :meth:`reorder`'s exact index semantics. :raw-html:`<br />` :raw-html:`<br />`
        
        If zero rules fire for a given occurrence: with a bare list, or ``keepKeyWithoutRemap=False``,
        that occurrence is removed entirely. With ``keepKeyWithoutRemap=True`` (via
        :class:`KeyRemapData`), it retains its original ``(key, value)`` pair instead. :raw-html:`<br />` :raw-html:`<br />`
        
        Old keys mentioned in 'keyRemap' that don't actually exist right now are simply never
        triggered -- no error, nothing happens. This is a single pass over the original entries:
        newly-created entries are never looked up in 'keyRemap' again, so there's no cascading/recursive
        re-application.
        
        Parameters
        ----------
        keyRemap: Dict[Any, Union[List[Union[Any, :class:`RemappedKeyData`]], :class:`KeyRemapData`]]
            The old key -> remap rules mapping to apply
        
        ranges: Optional[:class:`Ranges`]
            If provided, an occurrence outside 'ranges' is treated exactly as if its key were never
            mentioned in 'keyRemap' at all -- a pure pass-through :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def removeAt(self, pos: typing.SupportsInt | typing.SupportsIndex, ranges: FixRaidenBoss2.core.Ranges | None = None) -> bool:
        """
        Removes the entry currently at position 'pos'
        
        Parameters
        ----------
        pos: :class:`int`
            The position of the entry to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, removal only proceeds when 'pos' falls within 'ranges'; otherwise this call is
            a no-op, same as an out-of-bounds 'pos' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`bool`
            Whether an entry was actually removed
        """
    def removeKey(self, key: typing.Any, ranges: FixRaidenBoss2.core.Ranges | None = None, check: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex, typing.Any], bool] | None = None) -> int:
        """
        Removes every entry with this key, subject to two independent, optional filters -- both must
        hold (where provided) for a given occurrence to actually be removed. With neither filter
        provided, this is unconditional removal of every entry with this key.
        
        Parameters
        ----------
        key: Any
            The key whose entries to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, the occurrence's true positional index (same convention as :meth:`getByInd`)
            must fall within 'ranges' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        check: Optional[Callable[[:class:`int`, Any], :class:`bool`]]
            If provided, ``check(index, value)`` must return ``True``, given that occurrence's true
            positional index and value :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`int`
            How many entries were actually removed
        """
    def reorder(self, orderMap: dict, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Reorders existing entries in place. 'orderMap' maps an old index -> new index for a subset (or
        all) of the current entries; every entry not mentioned keeps its relative order and fills
        whatever slots are left over :raw-html:`<br />` :raw-html:`<br />`
        
        **Old-index (key) semantics:** must be in ``[-len(self), len(self) - 1]`` -- anything outside
        that raises :class:`IndexError`. :raw-html:`<br />` :raw-html:`<br />`
        
        **New-index (value) semantics:** also Python-style, but out-of-range values are bucketed rather
        than rejected: a value ``>= len(self)`` goes in a trailing cluster at the very end, a value
        ``< -len(self)`` goes in a leading cluster at the very front, and within a cluster a smaller raw
        value sorts earlier. :raw-html:`<br />` :raw-html:`<br />`
        
        **Conflicts:** if two distinct entries of 'orderMap' target the same physical old entry, or the
        same effective new-index target, dict iteration order (Python 3.7+ insertion order) breaks the
        tie.
        
        Parameters
        ----------
        orderMap: Dict[:class:`int`, :class:`int`]
            The old index -> new index mapping to apply
        
        ranges: Optional[:class:`Ranges`]
            If provided, an 'orderMap' entry only takes effect when its old index falls within 'ranges';
            otherwise it's ignored entirely, and the old position it would have pinned floats instead :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def replaceVals(self, newVals: dict, addNew: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Bulk-updates values by key. 'newVals' maps a key -> either a bare replacement value, a
        :class:`ReplaceList` (positional, by existing true left-to-right order), or a :class:`ReplaceIf`
        (conditional, by predicate).
        
        Parameters
        ----------
        newVals: Dict[Any, Union[Any, :class:`ReplaceList`, :class:`ReplaceIf`]]
            The key -> replace spec mapping to apply
        
        addNew: :class:`bool`
            What to do when a key in 'newVals' doesn't currently exist. If ``True`` (the default), it's
            added, appended at the end (a bare value -> one entry; :class:`ReplaceList` -> one entry per
            value, in order; :class:`ReplaceIf` -> one entry with just the value, predicate ignored
            since there's nothing existing to test it against). If ``False``, the key is skipped
            entirely; no error.
        
        ranges: Optional[:class:`Ranges`]
            If provided, gates whether an existing entry's value actually gets replaced, on top of
            whatever the spec itself already decides -- both must hold. Not consulted for 'addNew' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def setValByInd(self, index: typing.SupportsInt | typing.SupportsIndex, value: typing.Any) -> None:
        """
        Sets the value of the entry at a true positional index, leaving its key untouched
        
        Parameters
        ----------
        index: :class:`int`
            The position to update. Python-style negative indices are supported
        
        value: Any
            The new value for that entry
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        """
    def size(self) -> int:
        """
        Retrieves the number of entries
        """
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[OrderedMultiMap]:
        """
        Splits this map into several smaller maps at the given indices, preserving relative order both
        within each part and across parts. Each resulting part is a genuinely independent new instance --
        mutating one part never affects the original or any sibling part.
        
        'inds' uses the same index convention as :meth:`getByInd`. Each index becomes a boundary using
        standard slice semantics: everything before it goes in the earlier part, everything from it
        onward starts the next part.
        
        Parameters
        ----------
        inds: List[:class:`int`]
            The indices at which to split
        
        includeSplitKVP: :class:`bool`
            What happens to the entry at each split point. If ``True`` (the default), it starts the
            later part. If ``False``, it's dropped entirely, belonging to neither part
        
        includeEmptyParts: :class:`bool`
            Whether empty parts are included in the result or silently dropped :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        sortIndices: :class:`bool`
            If ``True`` (the default), 'inds' is normalized, deduplicated, and sorted ascending first.
            If you already know 'inds' iterates in that exact order, pass ``False`` to skip that pass --
            **this precondition is unchecked**, and violating it produces a silently wrong (not
            crashing) result
        
        Raises
        ------
        :class:`IndexError`
            If an index in 'inds' is out of range
        
        Returns
        -------
        List[:class:`OrderedMultiMap`]
            The resulting parts, left to right
        """
class OrderedMultiMapIterator:
    """
    
    A forward iterator over a :class:`OrderedMultiMap`, yielding ``(key, value,
    occurrenceIndex, orderIndex)`` tuples in true positional order.
            
    """
    def __iter__(self) -> OrderedMultiMapIterator:
        ...
    def __next__(self) -> tuple[typing.Any, typing.Any, int, int]:
        ...
class OrderedMultiMapSqrt:
    """
    
    An ordered multimap implemented in C++: preserves insertion/positional order, allows duplicate
    keys, and gives both fast key-based access and fast positional access. Behaviorally
    interchangeable with :class:`OrderedMultiMap` -- backed instead by O(sqrt(n)) block
    decomposition, giving O(sqrt(n)) positional access (:meth:`getByInd`, :meth:`insertAt`,
    :meth:`removeAt`, etc.) instead of :class:`OrderedMultiMap`'s O(n) worst case for a middle
    index, at the cost of more rebalancing machinery underneath.
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: key in x
    
            Determines whether 'key' exists
    
        .. describe:: len(x)
    
            Retrieves the number of entries
    
        .. describe:: x[index]
    
            Retrieves the ``(key, value)`` pair at the given true positional index
    
        .. describe:: iter(x)
    
            Iterates every entry in true positional order, yielding ``(key, value,
            occurrenceIndex, orderIndex)`` tuples
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
        deep copy (equivalent to ``x.copy()``)
    
    Parameters
    ----------
    items: Optional[List[Tuple[Any, Any]]]
        Key-value pairs to insert at the end, in order :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None`` (an empty instance)
            
    """
    @staticmethod
    def fromIndexed(indexed: dict) -> OrderedMultiMapSqrt:
        """
        Builds an instance from a fully-indexed description: for each key, a list of ``(index, value)``
        pairs. The index is treated as a sort key, not a strict absolute position: every ``(index, key,
        value)`` triple across every key is gathered, stable-sorted by index ascending, and inserted in
        that order -- gaps and out-of-order values just determine relative order, and duplicate indices
        land consecutively (tie-broken by encounter order: list order within a key, then 'indexed's own
        dict order across different keys).
        
        Parameters
        ----------
        indexed: Dict[Any, List[Tuple[:class:`int`, Any]]]
            The key -> list of ``(index, value)`` pairs to build from
        
        Returns
        -------
        :class:`OrderedMultiMapSqrt`
            The newly-built instance
        """
    def __contains__(self, key: typing.Any) -> bool:
        """
        Determines whether 'key' exists
        """
    def __copy__(self) -> OrderedMultiMapSqrt:
        """
        Creates a copy of this instance (equivalent to :meth:`copy`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> OrderedMultiMapSqrt:
        """
        Creates a deep copy of this instance (equivalent to :meth:`copy`); supports ``copy.deepcopy()``
        """
    def __getitem__(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the ``(key, value)`` pair at a true positional index
        """
    def __init__(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]] | None = None) -> None:
        ...
    def __iter__(self) -> OrderedMultiMapSqrtIterator:
        """
        Iterates every entry in true positional order, yielding ``(key, value, occurrenceIndex, orderIndex)`` tuples
        """
    def __len__(self) -> int:
        """
        Retrieves the number of entries
        """
    def asInterface(self) -> ...:
        """
        Creates an independent snapshot of this instance, viewed through the generic
        :class:`IOrderedMultiMap` interface -- like :meth:`copy`, this is a deep copy; mutating the
        result does not affect this instance (or vice versa)
        
        Returns
        -------
        :class:`IOrderedMultiMap`
            An independent :class:`IOrderedMultiMap`-typed snapshot of this instance
        """
    def contains(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def containsKey(self, key: typing.Any) -> bool:
        """
        Checks whether a key exists
        """
    def copy(self) -> OrderedMultiMapSqrt:
        """
        Creates a deep copy of this instance -- rebuilt entry-by-entry, so the copy shares no internal
        state with the original
        
        Returns
        -------
        :class:`OrderedMultiMapSqrt`
            The newly-created copy
        """
    def count(self, key: typing.Any) -> int:
        """
        Retrieves how many entries share a given key
        """
    def empty(self) -> bool:
        """
        Checks whether the map is empty
        """
    def entries(self) -> list[tuple[typing.Any, typing.Any]]:
        """
        Retrieves a copy of the full ordered sequence
        
        Returns
        -------
        List[Tuple[Any, Any]]
            The full ordered sequence of ``(key, value)`` pairs
        """
    def getAll(self, key: typing.Any, ordered: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> list[typing.Any]:
        """
        Retrieves all values currently stored under a key
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Any]
            The values for this key, in the requested order
        """
    def getAllWithInds(self, key: typing.Any, ordered: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> list[tuple[int, typing.Any]]:
        """
        Retrieves all values currently stored under a key, each paired with its true positional index
        (equivalent to :meth:`getAll`, except each value is paired with its true positional index)
        
        Parameters
        ----------
        key: Any
            The key to look up
        
        ordered: :class:`bool`
            If ``True`` (the default), returned in true left-to-right positional order. If ``False``,
            returned in whatever order they were added to this key
        
        ranges: Optional[:class:`Ranges`]
            If provided, only occurrences whose true positional index (same convention as
            :meth:`getByInd`) falls within ``ranges`` are included :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, meaning every occurrence is included
        
        Returns
        -------
        List[Tuple[:class:`int`, Any]]
            The ``(order index, value)`` pairs for this key, in the requested order
        """
    def getByInd(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[typing.Any, typing.Any]:
        """
        Retrieves the entry at a true positional index
        
        Parameters
        ----------
        index: :class:`int`
            The position to retrieve. Python-style negative indices are supported
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        
        Returns
        -------
        Tuple[Any, Any]
            The ``(key, value)`` pair at that position
        """
    def getByIndWithOccurrence(self, index: typing.SupportsInt | typing.SupportsIndex) -> tuple[int, typing.Any]:
        """
        Retrieves the entry at a true positional index, paired with its occurrence index (how many
        times this same key already appeared earlier in the sequence, 0-based) instead of its key
        (equivalent to :meth:`getByInd`, except the entry's value is paired with its occurrence index)
        
        Parameters
        ----------
        index: :class:`int`
            The position to retrieve. Python-style negative indices are supported
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        
        Returns
        -------
        Tuple[:class:`int`, Any]
            The ``(occurrence index, value)`` pair at that position
        """
    def getKeys(self) -> set[typing.Any]:
        """
        Retrieves every distinct key currently in the map
        
        Returns
        -------
        Set[Any]
            Every distinct key, as a set (unordered)
        """
    def insert(self, key: typing.Any, value: typing.Any) -> None:
        """
        Appends a key-value pair to the end
        """
    def insertAllAt(self, items: dict, sortIndices: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> int:
        """
        Bulk indexed insert: inserts many key-value pairs at their own target indices in a single pass.
        Index semantics match :meth:`insertAt` (Python-style negative indices, clamping), but with
        "original position" (numpy-style) semantics: each index refers to a position in the sequence as
        it was *before* this call, not a position in the growing result.
        
        Parameters
        ----------
        items: Dict[:class:`int`, Tuple[Any, Any]]
            Maps an index to insert at -> the key-value pair to insert there
        
        sortIndices: :class:`bool`
            If ``True`` (the default), 'items' is stable-sorted by normalized index first. If you
            already know 'items' iterates in ascending normalized-index order, pass ``False`` to skip
            that sort -- **this precondition is unchecked**, and violating it produces a silently wrong
            (not crashing) result
        
        ranges: Optional[:class:`Ranges`]
            If provided, an entry is only inserted when its normalized target index falls within
            'ranges'; filtered entries are dropped before sorting/the insertion pass :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`int`
            How many entries were actually inserted
        """
    def insertAllEnd(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Appends a batch of key-value pairs to the end, in the order given
        """
    def insertAllStart(self, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
        """
        Inserts a batch of key-value pairs at the beginning, in the order given -- ``items[0]`` ends up
        first, ``items[1]`` right after it, and so on, all before whatever was originally at the front
        """
    def insertAt(self, index: typing.SupportsInt | typing.SupportsIndex, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair so it ends up at position 'index' (0-based) :raw-html:`<br />` :raw-html:`<br />`
        
        Supports Python-style negative indices. Out-of-range indices are clamped rather than rejected:
        an index greater than ``len(self)`` is treated as ``len(self)`` (append); an index less than
        ``-(len(self) + 1)`` is treated as ``-(len(self) + 1)`` (front)
        
        Parameters
        ----------
        index: :class:`int`
            The target position
        
        key: Any
            The key of the pair to insert
        
        value: Any
            The value of the pair to insert
        """
    def insertStart(self, key: typing.Any, value: typing.Any) -> None:
        """
        Inserts a key-value pair at the beginning
        """
    def keySize(self) -> int:
        """
        Retrieves the number of distinct keys
        """
    def length(self) -> int:
        """
        Retrieves the number of entries
        """
    def remapKeys(self, keyRemap: dict, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Bulk-renames keys. 'keyRemap' maps an old key -> either a bare list of keys/
        :class:`CppRemappedKeyData`, or a :class:`CppKeyRemapData`. :raw-html:`<br />` :raw-html:`<br />`
        
        For every existing entry, walked in true positional order: if its key is not a key in
        'keyRemap', it's left completely unchanged. Otherwise, each rule in the mapped list is evaluated
        independently against this occurrence -- a plain key always fires, a :class:`CppRemappedKeyData`
        fires if it has no ``check``, or ``check(oldKey, oldValue)`` is ``True``. Every rule that fires
        produces one new entry (that rule's key, this occurrence's original value); a
        :class:`CppRemappedKeyData` with ``toInd`` set instead moves its entry (as part of a group with
        every other entry sharing that same ``toInd`` across every occurrence) to that target index,
        using :meth:`reorder`'s exact index semantics. :raw-html:`<br />` :raw-html:`<br />`
        
        If zero rules fire for a given occurrence: with a bare list, or ``keepKeyWithoutRemap=False``,
        that occurrence is removed entirely. With ``keepKeyWithoutRemap=True`` (via
        :class:`CppKeyRemapData`), it retains its original ``(key, value)`` pair instead. :raw-html:`<br />` :raw-html:`<br />`
        
        Old keys mentioned in 'keyRemap' that don't actually exist right now are simply never
        triggered -- no error, nothing happens. This is a single pass over the original entries:
        newly-created entries are never looked up in 'keyRemap' again, so there's no cascading/recursive
        re-application.
        
        Parameters
        ----------
        keyRemap: Dict[Any, Union[List[Union[Any, :class:`CppRemappedKeyData`]], :class:`CppKeyRemapData`]]
            The old key -> remap rules mapping to apply
        
        ranges: Optional[:class:`Ranges`]
            If provided, an occurrence outside 'ranges' is treated exactly as if its key were never
            mentioned in 'keyRemap' at all -- a pure pass-through :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def removeAt(self, pos: typing.SupportsInt | typing.SupportsIndex, ranges: FixRaidenBoss2.core.Ranges | None = None) -> bool:
        """
        Removes the entry currently at position 'pos'
        
        Parameters
        ----------
        pos: :class:`int`
            The position of the entry to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, removal only proceeds when 'pos' falls within 'ranges'; otherwise this call is
            a no-op, same as an out-of-bounds 'pos' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`bool`
            Whether an entry was actually removed
        """
    def removeKey(self, key: typing.Any, ranges: FixRaidenBoss2.core.Ranges | None = None, check: collections.abc.Callable[[typing.SupportsInt | typing.SupportsIndex, typing.Any], bool] | None = None) -> int:
        """
        Removes every entry with this key, subject to two independent, optional filters -- both must
        hold (where provided) for a given occurrence to actually be removed. With neither filter
        provided, this is unconditional removal of every entry with this key.
        
        Parameters
        ----------
        key: Any
            The key whose entries to remove
        
        ranges: Optional[:class:`Ranges`]
            If provided, the occurrence's true positional index (same convention as :meth:`getByInd`)
            must fall within 'ranges' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        check: Optional[Callable[[:class:`int`, Any], :class:`bool`]]
            If provided, ``check(index, value)`` must return ``True``, given that occurrence's true
            positional index and value :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`int`
            How many entries were actually removed
        """
    def reorder(self, orderMap: dict, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Reorders existing entries in place. 'orderMap' maps an old index -> new index for a subset (or
        all) of the current entries; every entry not mentioned keeps its relative order and fills
        whatever slots are left over :raw-html:`<br />` :raw-html:`<br />`
        
        **Old-index (key) semantics:** must be in ``[-len(self), len(self) - 1]`` -- anything outside
        that raises :class:`IndexError`. :raw-html:`<br />` :raw-html:`<br />`
        
        **New-index (value) semantics:** also Python-style, but out-of-range values are bucketed rather
        than rejected: a value ``>= len(self)`` goes in a trailing cluster at the very end, a value
        ``< -len(self)`` goes in a leading cluster at the very front, and within a cluster a smaller raw
        value sorts earlier. :raw-html:`<br />` :raw-html:`<br />`
        
        **Conflicts:** if two distinct entries of 'orderMap' target the same physical old entry, or the
        same effective new-index target, dict iteration order (Python 3.7+ insertion order) breaks the
        tie.
        
        Parameters
        ----------
        orderMap: Dict[:class:`int`, :class:`int`]
            The old index -> new index mapping to apply
        
        ranges: Optional[:class:`Ranges`]
            If provided, an 'orderMap' entry only takes effect when its old index falls within 'ranges';
            otherwise it's ignored entirely, and the old position it would have pinned floats instead :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def replaceVals(self, newVals: dict, addNew: bool = True, ranges: FixRaidenBoss2.core.Ranges | None = None) -> None:
        """
        Bulk-updates values by key. 'newVals' maps a key -> either a bare replacement value, a
        :class:`ReplaceList` (positional, by existing true left-to-right order), or a :class:`ReplaceIf`
        (conditional, by predicate).
        
        Parameters
        ----------
        newVals: Dict[Any, Union[Any, :class:`ReplaceList`, :class:`ReplaceIf`]]
            The key -> replace spec mapping to apply
        
        addNew: :class:`bool`
            What to do when a key in 'newVals' doesn't currently exist. If ``True`` (the default), it's
            added, appended at the end (a bare value -> one entry; :class:`ReplaceList` -> one entry per
            value, in order; :class:`ReplaceIf` -> one entry with just the value, predicate ignored
            since there's nothing existing to test it against). If ``False``, the key is skipped
            entirely; no error.
        
        ranges: Optional[:class:`Ranges`]
            If provided, gates whether an existing entry's value actually gets replaced, on top of
            whatever the spec itself already decides -- both must hold. Not consulted for 'addNew' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def setValByInd(self, index: typing.SupportsInt | typing.SupportsIndex, value: typing.Any) -> None:
        """
        Sets the value of the entry at a true positional index, leaving its key untouched
        
        Parameters
        ----------
        index: :class:`int`
            The position to update. Python-style negative indices are supported
        
        value: Any
            The new value for that entry
        
        Raises
        ------
        :class:`IndexError`
            If 'index' is out of range
        """
    def size(self) -> int:
        """
        Retrieves the number of entries
        """
    def splitByInds(self, inds: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], includeSplitKVP: bool = True, includeEmptyParts: bool = False, sortIndices: bool = True) -> list[OrderedMultiMapSqrt]:
        """
        Splits this map into several smaller maps at the given indices, preserving relative order both
        within each part and across parts. Each resulting part is a genuinely independent new instance --
        mutating one part never affects the original or any sibling part.
        
        'inds' uses the same index convention as :meth:`getByInd`. Each index becomes a boundary using
        standard slice semantics: everything before it goes in the earlier part, everything from it
        onward starts the next part.
        
        Parameters
        ----------
        inds: List[:class:`int`]
            The indices at which to split
        
        includeSplitKVP: :class:`bool`
            What happens to the entry at each split point. If ``True`` (the default), it starts the
            later part. If ``False``, it's dropped entirely, belonging to neither part
        
        includeEmptyParts: :class:`bool`
            Whether empty parts are included in the result or silently dropped :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        sortIndices: :class:`bool`
            If ``True`` (the default), 'inds' is normalized, deduplicated, and sorted ascending first.
            If you already know 'inds' iterates in that exact order, pass ``False`` to skip that pass --
            **this precondition is unchecked**, and violating it produces a silently wrong (not
            crashing) result
        
        Raises
        ------
        :class:`IndexError`
            If an index in 'inds' is out of range
        
        Returns
        -------
        List[:class:`OrderedMultiMapSqrt`]
            The resulting parts, left to right
        """
class OrderedMultiMapSqrtIterator:
    """
    
    A forward iterator over a :class:`OrderedMultiMapSqrt`, yielding ``(key, value,
    occurrenceIndex, orderIndex)`` tuples in true positional order.
            
    """
    def __iter__(self) -> OrderedMultiMapSqrtIterator:
        ...
    def __next__(self) -> tuple[typing.Any, typing.Any, int, int]:
        ...
class ParseContext:
    """
    
    Context for parsing some text
    
    Parameters
    ----------
    src: Union[:class:`str`, List[:class:`str`]]
        The source text to parse
    
        If this argument is a list, then assumes that the lines of the source text is given
    
        **Default**: ``""``
    
    file: Optional[:class:`str`]
        The file path to the source text
    
        **Default**: ``None``
    
    startLineNo: :class:`int`
        The starting line of the source text
    
        **Default**: ``1``
        
    """
    @typing.overload
    def __init__(self, src: str = '', file: str | None = None, startLineNo: typing.SupportsInt | typing.SupportsIndex = 1) -> None:
        ...
    @typing.overload
    def __init__(self, src: collections.abc.Sequence[str], file: str | None = None, startLineNo: typing.SupportsInt | typing.SupportsIndex = 1) -> None:
        ...
    def getEndLineNo(self) -> int:
        """
        Retrieves the line number after the last line
        
        Returns
        -------
        :class:`int`
            The ending line number after the last line
        """
    @property
    def file(self) -> str | None:
        """
        Optional[:class:`str`]: The file path to the source text
        """
    @file.setter
    def file(self, arg0: str | None) -> None:
        ...
    @property
    def lines(self) -> list[str]:
        """
        List[:class:`str`]: The lines of the source text
        """
    @lines.setter
    def lines(self, arg0: collections.abc.Sequence[str]) -> None:
        ...
    @property
    def startLineNo(self) -> int:
        """
        :class:`int`: The starting line of the source text
        """
    @startLineNo.setter
    def startLineNo(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
class ParseNode:
    """
    
    A node within a parse tree, created from a parser that interprets some `CFG`_
    
    Parameters
    ----------
    id: Hashable
        The id for the node
    
    prodId: Optional[Hashable]
        The id for the chosen production from the `CFG`_
    
        **Default**: ``None``
    
    token: Optional[:class:`Token`]
        The token that this node references
    
        **Default**: ``None``
        
    """
    def __init__(self, id: typing.Any, prodId: typing.Any | None = None, token: FixRaidenBoss2.core.Token | None = None) -> None:
        ...
    @property
    def id(self) -> typing.Any:
        """
        Hashable: The id of the node
        """
    @property
    def prodId(self) -> typing.Any | None:
        """
        Optional[Hashable]: The id for the chosen production from the `CFG`_
        """
    @prodId.setter
    def prodId(self, arg0: typing.Any | None) -> None:
        ...
    @property
    def token(self) -> FixRaidenBoss2.core.Token | None:
        """
        Optional[:class:`Token`]: The token that this node references
        """
    @token.setter
    def token(self, arg0: FixRaidenBoss2.core.Token | None) -> None:
        ...
class ParseTree:
    """
    
    The generated parse tree after parsing some text
    
    Parameters
    ----------
    nodes: Dict[Hashable, :class:`ParseNode`]
        The nodes in the tree
    
        The keys are the ids of the node and the values are the nodes
    
    children: Dict[Hashable, List[Hashable]]
        The children relations of the nodes
    
        The keys are the ids of the parent nodes and the values are the ids of the children nodes
    
    rootId: Hashable
        The id of the root node
        
    """
    def __init__(self, nodes: collections.abc.Mapping[typing.Any, ParseNode], children: collections.abc.Mapping[typing.Any, collections.abc.Sequence[typing.Any]], rootId: typing.Any) -> None:
        ...
    def getNode(self, nodeId: typing.Any, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves a node based on the passed id
        
        Parameters
        ----------
        nodeId: Hashable
            The node id to search for
        
        errorOnNotFound: :class:`bool`
            Whether to raise if no matching node is found
        
            **Default**: ``True``
        
        default: Any
            The default value to return if no matching node is found and 'errorOnNotFound' is ``False``
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`KeyError`
            No matching node is found and 'errorOnNotFound' is ``True``
        
        Returns
        -------
        Optional[:class:`ParseNode`]
            The corresponding node, if found
        """
    def isChild(self, nodeId: typing.Any) -> bool:
        """
        Determines whether the id of some node has no children of its own
        
        Parameters
        ----------
        nodeId: Hashable
            The id of the node
        
        Returns
        -------
        :class:`bool`
            Whether the id corresponds to a node with no children of its own
        """
    @property
    def children(self) -> dict[typing.Any, list[typing.Any]]:
        """
        Dict[Hashable, List[Hashable]]: The children relations of the nodes
        
        The keys are the ids of the parent nodes and the values are the ids of the children nodes
        """
    @children.setter
    def children(self, arg0: collections.abc.Mapping[typing.Any, collections.abc.Sequence[typing.Any]]) -> None:
        ...
    @property
    def nodes(self) -> dict[typing.Any, ParseNode]:
        """
        Dict[Hashable, :class:`ParseNode`]: The nodes in the tree
        
        The keys are the ids of the node and the values are the nodes
        """
    @nodes.setter
    def nodes(self, arg0: collections.abc.Mapping[typing.Any, ParseNode]) -> None:
        ...
    @property
    def rootId(self) -> typing.Any:
        """
        Hashable: The id of the root node
        
        :getter: Retrieves the id of the root node
        :setter: Sets the new id of the root node
        """
    @rootId.setter
    def rootId(self, arg1: typing.Any) -> None:
        ...
class PositionFile(CppBufFile):
    """
    
    This class inherits from :class:`CppBufFile`
    
    Used for handling ``Position.buf`` files
    
    .. note::
        We observe that a ``Position.buf`` file is a binary file defined as:
    
        * a line corresponds to the data for a particular vertex in the mod
        * each line contains 40 bytes (320 bits)
        * each line uses little-endian mode (MSB is to the right while LSB is to the left)
        * the first 12 bytes of a line are the coordinate position of a vertex in an R3 vector space, each scalar value in the coordinate is 4 bytes or 32 bits (3 scalar values/line)
        * the next 12 bytes of a line corresponds to the normal vector of a vertex, each scalar value in the vector is 4 bytes or 32 bits (3 scalar values/line)
        * the last 16 bytes of a line corresponds to the tangent vector of a vertex, each scalar value in the vector is 4 bytes or 32 bits (4 scalar values/line)
        * all scalar values in the file are `floating point`_ values
        
    """
    def __init__(self, src: typing.Any) -> None:
        """
        Constructs a new position file and immediately reads it
        
        Parameters
        ----------
        src: Union[:class:`str`, :class:`bytes`]
            The source file or bytes for the ``.buf`` file
        
        Raises
        ------
        :class:`BufFileNotRecognized`
            If 'src' holds a file path that cannot be read as a valid position file
        
        :class:`BadBufData`
            If 'src' holds raw bytes that are not valid for a position file
        """
class Ranges:
    """
    
    A class representing a collection of integer ranges
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: value in x
    
            Determines whether 'value' falls within any of the ranges
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' store the same ranges
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' store different ranges
    
        .. describe:: x - y
    
            Computes the set difference between 'x' and 'y' (equivalent to 'x.difference(y)')
    
        .. describe:: ~x
    
            Computes the negation (complement) of 'x' (equivalent to 'x.negate()')
    
        .. describe:: x + y
    
            Computes the union of 'x' and 'y' (equivalent to 'x.union([y])')
    
        .. describe:: x += y
    
            Performs the union of 'x' with 'y', in place (equivalent to 'x.update([y])')
    
        .. describe:: x & y
    
            Computes the intersection of 'x' and 'y' (equivalent to 'x.intersect([y])')
    
        .. describe:: x &= y
    
            Performs the intersection of 'x' with 'y', in place (equivalent to 'x.intersectUpdate([y])')
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a deep copy (equivalent to 'x.deepcopy()')
    
    .. note::
        This constructor is overloaded. Instead of a list of range tuples, ``Ranges`` can also be constructed directly
        from a ``List[int]`` or a ``Set[int]``, in which case the resulting ranges cover exactly those values (equivalent
        to :meth:`createFromList` / :meth:`createFromSet`)
    
    Parameters
    ----------
    ranges: List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]
        The ranges to store :raw-html:`<br />` :raw-html:`<br />`
    
        Each range is a tuple containing the starting (inclusive) index and the ending (exclusive) index of the range :raw-html:`<br />` :raw-html:`<br />`
    
        If the starting index is ``None``, the range is unbounded towards -infinity. If the ending index is ``None``, the range is unbounded towards +infinity
    
    normalize: :class:`bool`
        Whether to normalize 'ranges' before storing it :raw-html:`<br />` :raw-html:`<br />`
    
        If ``True``, 'ranges' is sorted and any overlapping or touching ranges are merged, producing the minimal set of disjoint ranges :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
            
    """
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def createEmpty() -> Ranges:
        """
        Creates a :class:`Ranges` with no ranges
        
        Returns
        -------
        :class:`Ranges`
            A new, empty instance
        """
    @staticmethod
    def createFromList(values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> Ranges:
        """
        Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
        
        Consecutive integers are merged into contiguous ranges (e.g. ``[1, 2, 3, 5]`` becomes ``[1,4)`` and ``[5,6)``)
        
        Parameters
        ----------
        values: List[:class:`int`]
            The values to include
        
        Returns
        -------
        :class:`Ranges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFromSet(values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> Ranges:
        """
        Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
        
        Consecutive integers are merged into contiguous ranges (e.g. ``{1, 2, 3, 5}`` becomes ``[1,4)`` and ``[5,6)``)
        
        Parameters
        ----------
        values: Set[:class:`int`]
            The values to include
        
        Returns
        -------
        :class:`Ranges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFull() -> Ranges:
        """
        Creates a :class:`Ranges` spanning from -infinity to +infinity
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing a single range from -infinity to +infinity
        """
    def __add__(self, other: Ranges) -> Ranges:
        """
        Computes the union of this instance with 'other' (equivalent to 'self.union([other])')
        """
    def __and__(self, other: Ranges) -> Ranges:
        """
        Computes the intersection of this instance with 'other' (equivalent to 'self.intersect([other])')
        """
    def __contains__(self, value: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
        Determines whether 'value' falls within any of the stored ranges
        """
    def __copy__(self) -> Ranges:
        """
        Creates a copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> Ranges:
        """
        Creates a deep copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.deepcopy()``
        """
    def __eq__(self, other: Ranges) -> bool:
        """
        Determines whether 'self' and 'other' store the same ranges
        """
    def __iadd__(self, other: Ranges) -> Ranges:
        """
        Performs the union of this instance with 'other', in place (equivalent to 'self.update([other])')
        """
    def __iand__(self, other: Ranges) -> Ranges:
        """
        Performs the intersection of this instance with 'other', in place (equivalent to 'self.intersectUpdate([other])')
        """
    @typing.overload
    def __init__(self, ranges: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex | None, typing.SupportsInt | typing.SupportsIndex | None]], normalize: bool = True) -> None:
        ...
    @typing.overload
    def __init__(self, values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    def __invert__(self) -> Ranges:
        """
        Computes the negation (complement) of this instance (equivalent to :meth:`negate`)
        """
    def __ne__(self, other: Ranges) -> bool:
        """
        Determines whether 'self' and 'other' store different ranges
        """
    def __sub__(self, other: Ranges) -> Ranges:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
        """
    def add(self, value: typing.SupportsInt | typing.SupportsIndex, normalize: bool = False) -> None:
        """
        Adds 'value' to the stored ranges, extending or merging existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`
        
        If 'value' is already contained within the stored ranges, this has no effect
        
        Parameters
        ----------
        value: :class:`int`
            The value to add
        
        normalize: :class:`bool`
            Whether to (re)normalize ``self`` before adding 'value' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
            on that to locate the insertion point efficiently). If that assumption doesn't hold, pass ``True``, or the
            result may be incorrect
        """
    def deepcopy(self) -> Ranges:
        """
        Creates a deep copy of this instance
        
        Returns
        -------
        :class:`Ranges`
            A new instance that is a deep copy of this one
        """
    def difference(self, other: Ranges) -> Ranges:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
        
        Parameters
        ----------
        other: :class:`Ranges`
            The ranges to subtract from this instance
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the values in ``self`` that are not in 'other'
        """
    def getOverlaps(self, ranges: collections.abc.Sequence[Ranges], requireAll: bool = True, normalizeSelf: bool = False, normalizeOthers: bool = False) -> Ranges:
        """
        Computes the overlap between this instance and a list of other :class:`Ranges`
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to compute the overlap against
        
        requireAll: :class:`bool`
            When ``True`` (the default), the result is the intersection of ``self`` and *every* entry in 'ranges'
            (a value must fall within ``self`` and all of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`
        
            When ``False``, the result is the intersection of ``self`` and the *union* of 'ranges'
            (a value must fall within ``self`` and at least one of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the overlap :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the overlap :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the computed overlap
        """
    def has(self, value: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
        Determines whether 'value' falls within any of the stored ranges
        
        Parameters
        ----------
        value: :class:`int`
            The value to check
        
        Returns
        -------
        :class:`bool`
            Whether 'value' is contained within any of the stored ranges
        """
    def intersect(self, ranges: collections.abc.Sequence[Ranges], normalizeSelf: bool = False, normalizeOthers: bool = False) -> Ranges:
        """
        Computes the intersection of this instance and a list of other :class:`Ranges` :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``self.getOverlaps(ranges, True, normalizeSelf, normalizeOthers)``
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to intersect with
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the intersection of ``self`` and every entry in 'ranges'
        """
    def intersectUpdate(self, ranges: collections.abc.Sequence[Ranges], normalizeSelf: bool = False, normalizeOthers: bool = False) -> None:
        """
        Performs the intersection of this instance with a list of other :class:`Ranges`, in place :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``self.intersect(ranges, normalizeSelf, normalizeOthers)``, assigned back to ``self``
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to intersect with
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        """
    def isEmpty(self) -> bool:
        """
        Determines whether there are no stored ranges
        
        Returns
        -------
        :class:`bool`
            Whether there are no stored ranges
        """
    def isFull(self) -> bool:
        """
        Determines whether the stored ranges span from -infinity to +infinity
        
        .. note::
            This checks for a single stored range of ``(None, None)``. If this instance was constructed with ``normalize=False``, several ranges
            could collectively cover -infinity to +infinity without having been merged into one, in which case this returns ``False``
        
        Returns
        -------
        :class:`bool`
            Whether the stored ranges span from -infinity to +infinity
        """
    def negate(self) -> Ranges:
        """
        Computes the negation (complement) of this instance :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``Ranges.createFull() - self``
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing every value not in ``self``
        """
    def remove(self, value: typing.SupportsInt | typing.SupportsIndex, normalize: bool = False) -> None:
        """
        Removes 'value' from the stored ranges, shrinking, splitting, or erasing existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`
        
        If 'value' isn't contained within the stored ranges, this has no effect
        
        Parameters
        ----------
        value: :class:`int`
            The value to remove
        
        normalize: :class:`bool`
            Whether to (re)normalize ``self`` before removing 'value' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
            on that to locate 'value' efficiently). If that assumption doesn't hold, pass ``True``, or the result may
            be incorrect
        """
    def union(self, ranges: collections.abc.Sequence[Ranges]) -> Ranges:
        """
        Computes the union of this instance with a list of other :class:`Ranges`
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to union with
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the union of ``self`` and 'ranges'
        """
    def update(self, ranges: collections.abc.Sequence[Ranges]) -> None:
        """
        Performs the union of this instance with a list of other :class:`Ranges`, in place
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to union with
        """
    @property
    def ranges(self) -> list[tuple[int | None, int | None]]:
        """
        List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]: The stored ranges
        """
    @ranges.setter
    def ranges(self, arg0: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex | None, typing.SupportsInt | typing.SupportsIndex | None]]) -> None:
        ...
class RangesInt:
    """
    
    A class representing a collection of integer ranges
    
    :raw-html:`<br />`
    
    .. container:: operations
    
        **Supported Operations:**
    
        .. describe:: value in x
    
            Determines whether 'value' falls within any of the ranges
    
        .. describe:: x == y
    
            Determines whether 'x' and 'y' store the same ranges
    
        .. describe:: x != y
    
            Determines whether 'x' and 'y' store different ranges
    
        .. describe:: x - y
    
            Computes the set difference between 'x' and 'y' (equivalent to 'x.difference(y)')
    
        .. describe:: ~x
    
            Computes the negation (complement) of 'x' (equivalent to 'x.negate()')
    
        .. describe:: x + y
    
            Computes the union of 'x' and 'y' (equivalent to 'x.union([y])')
    
        .. describe:: x += y
    
            Performs the union of 'x' with 'y', in place (equivalent to 'x.update([y])')
    
        .. describe:: x & y
    
            Computes the intersection of 'x' and 'y' (equivalent to 'x.intersect([y])')
    
        .. describe:: x &= y
    
            Performs the intersection of 'x' with 'y', in place (equivalent to 'x.intersectUpdate([y])')
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a deep copy (equivalent to 'x.deepcopy()')
    
    .. note::
        This constructor is overloaded. Instead of a list of range tuples, ``Ranges`` can also be constructed directly
        from a ``List[int]`` or a ``Set[int]``, in which case the resulting ranges cover exactly those values (equivalent
        to :meth:`createFromList` / :meth:`createFromSet`)
    
    Parameters
    ----------
    ranges: List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]
        The ranges to store :raw-html:`<br />` :raw-html:`<br />`
    
        Each range is a tuple containing the starting (inclusive) index and the ending (exclusive) index of the range :raw-html:`<br />` :raw-html:`<br />`
    
        If the starting index is ``None``, the range is unbounded towards -infinity. If the ending index is ``None``, the range is unbounded towards +infinity
    
    normalize: :class:`bool`
        Whether to normalize 'ranges' before storing it :raw-html:`<br />` :raw-html:`<br />`
    
        If ``True``, 'ranges' is sorted and any overlapping or touching ranges are merged, producing the minimal set of disjoint ranges :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
            
    """
    __hash__: typing.ClassVar[None] = None
    @staticmethod
    def createEmpty() -> RangesInt:
        """
        Creates a :class:`Ranges` with no ranges
        
        Returns
        -------
        :class:`Ranges`
            A new, empty instance
        """
    @staticmethod
    def createFromList(values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> RangesInt:
        """
        Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
        
        Consecutive integers are merged into contiguous ranges (e.g. ``[1, 2, 3, 5]`` becomes ``[1,4)`` and ``[5,6)``)
        
        Parameters
        ----------
        values: List[:class:`int`]
            The values to include
        
        Returns
        -------
        :class:`Ranges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFromSet(values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> RangesInt:
        """
        Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`
        
        Consecutive integers are merged into contiguous ranges (e.g. ``{1, 2, 3, 5}`` becomes ``[1,4)`` and ``[5,6)``)
        
        Parameters
        ----------
        values: Set[:class:`int`]
            The values to include
        
        Returns
        -------
        :class:`Ranges`
            A new instance whose ranges cover exactly the values in 'values'
        """
    @staticmethod
    def createFull() -> RangesInt:
        """
        Creates a :class:`Ranges` spanning from -infinity to +infinity
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing a single range from -infinity to +infinity
        """
    def __add__(self, other: RangesInt) -> RangesInt:
        """
        Computes the union of this instance with 'other' (equivalent to 'self.union([other])')
        """
    def __and__(self, other: RangesInt) -> RangesInt:
        """
        Computes the intersection of this instance with 'other' (equivalent to 'self.intersect([other])')
        """
    def __contains__(self, value: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
        Determines whether 'value' falls within any of the stored ranges
        """
    def __copy__(self) -> RangesInt:
        """
        Creates a copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.copy()``
        """
    def __deepcopy__(self, memo: dict) -> RangesInt:
        """
        Creates a deep copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.deepcopy()``
        """
    def __eq__(self, other: RangesInt) -> bool:
        """
        Determines whether 'self' and 'other' store the same ranges
        """
    def __iadd__(self, other: RangesInt) -> RangesInt:
        """
        Performs the union of this instance with 'other', in place (equivalent to 'self.update([other])')
        """
    def __iand__(self, other: RangesInt) -> RangesInt:
        """
        Performs the intersection of this instance with 'other', in place (equivalent to 'self.intersectUpdate([other])')
        """
    @typing.overload
    def __init__(self, ranges: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex | None, typing.SupportsInt | typing.SupportsIndex | None]], normalize: bool = True) -> None:
        ...
    @typing.overload
    def __init__(self, values: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    @typing.overload
    def __init__(self, values: collections.abc.Set[typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
    def __invert__(self) -> RangesInt:
        """
        Computes the negation (complement) of this instance (equivalent to :meth:`negate`)
        """
    def __ne__(self, other: RangesInt) -> bool:
        """
        Determines whether 'self' and 'other' store different ranges
        """
    def __sub__(self, other: RangesInt) -> RangesInt:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
        """
    def add(self, value: typing.SupportsInt | typing.SupportsIndex, normalize: bool = False) -> None:
        """
        Adds 'value' to the stored ranges, extending or merging existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`
        
        If 'value' is already contained within the stored ranges, this has no effect
        
        Parameters
        ----------
        value: :class:`int`
            The value to add
        
        normalize: :class:`bool`
            Whether to (re)normalize ``self`` before adding 'value' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
            on that to locate the insertion point efficiently). If that assumption doesn't hold, pass ``True``, or the
            result may be incorrect
        """
    def deepcopy(self) -> RangesInt:
        """
        Creates a deep copy of this instance
        
        Returns
        -------
        :class:`Ranges`
            A new instance that is a deep copy of this one
        """
    def difference(self, other: RangesInt) -> RangesInt:
        """
        Computes the set difference between this instance and 'other' (``self - other``)
        
        Parameters
        ----------
        other: :class:`Ranges`
            The ranges to subtract from this instance
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the values in ``self`` that are not in 'other'
        """
    def getOverlaps(self, ranges: collections.abc.Sequence[RangesInt], requireAll: bool = True, normalizeSelf: bool = False, normalizeOthers: bool = False) -> RangesInt:
        """
        Computes the overlap between this instance and a list of other :class:`Ranges`
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to compute the overlap against
        
        requireAll: :class:`bool`
            When ``True`` (the default), the result is the intersection of ``self`` and *every* entry in 'ranges'
            (a value must fall within ``self`` and all of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`
        
            When ``False``, the result is the intersection of ``self`` and the *union* of 'ranges'
            (a value must fall within ``self`` and at least one of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the overlap :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the overlap :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the computed overlap
        """
    def has(self, value: typing.SupportsInt | typing.SupportsIndex) -> bool:
        """
        Determines whether 'value' falls within any of the stored ranges
        
        Parameters
        ----------
        value: :class:`int`
            The value to check
        
        Returns
        -------
        :class:`bool`
            Whether 'value' is contained within any of the stored ranges
        """
    def intersect(self, ranges: collections.abc.Sequence[RangesInt], normalizeSelf: bool = False, normalizeOthers: bool = False) -> RangesInt:
        """
        Computes the intersection of this instance and a list of other :class:`Ranges` :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``self.getOverlaps(ranges, True, normalizeSelf, normalizeOthers)``
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to intersect with
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the intersection of ``self`` and every entry in 'ranges'
        """
    def intersectUpdate(self, ranges: collections.abc.Sequence[RangesInt], normalizeSelf: bool = False, normalizeOthers: bool = False) -> None:
        """
        Performs the intersection of this instance with a list of other :class:`Ranges`, in place :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``self.intersect(ranges, normalizeSelf, normalizeOthers)``, assigned back to ``self``
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to intersect with
        
        normalizeSelf: :class:`bool`
            Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
            assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        
        normalizeOthers: :class:`bool`
            Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
            that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        """
    def isEmpty(self) -> bool:
        """
        Determines whether there are no stored ranges
        
        Returns
        -------
        :class:`bool`
            Whether there are no stored ranges
        """
    def isFull(self) -> bool:
        """
        Determines whether the stored ranges span from -infinity to +infinity
        
        .. note::
            This checks for a single stored range of ``(None, None)``. If this instance was constructed with ``normalize=False``, several ranges
            could collectively cover -infinity to +infinity without having been merged into one, in which case this returns ``False``
        
        Returns
        -------
        :class:`bool`
            Whether the stored ranges span from -infinity to +infinity
        """
    def negate(self) -> RangesInt:
        """
        Computes the negation (complement) of this instance :raw-html:`<br />` :raw-html:`<br />`
        
        Equivalent to ``Ranges.createFull() - self``
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing every value not in ``self``
        """
    def remove(self, value: typing.SupportsInt | typing.SupportsIndex, normalize: bool = False) -> None:
        """
        Removes 'value' from the stored ranges, shrinking, splitting, or erasing existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`
        
        If 'value' isn't contained within the stored ranges, this has no effect
        
        Parameters
        ----------
        value: :class:`int`
            The value to remove
        
        normalize: :class:`bool`
            Whether to (re)normalize ``self`` before removing 'value' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
            on that to locate 'value' efficiently). If that assumption doesn't hold, pass ``True``, or the result may
            be incorrect
        """
    def union(self, ranges: collections.abc.Sequence[RangesInt]) -> RangesInt:
        """
        Computes the union of this instance with a list of other :class:`Ranges`
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to union with
        
        Returns
        -------
        :class:`Ranges`
            A new instance containing the union of ``self`` and 'ranges'
        """
    def update(self, ranges: collections.abc.Sequence[RangesInt]) -> None:
        """
        Performs the union of this instance with a list of other :class:`Ranges`, in place
        
        Parameters
        ----------
        ranges: List[:class:`Ranges`]
            The list of other ranges to union with
        """
    @property
    def ranges(self) -> list[tuple[int | None, int | None]]:
        """
        List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]: The stored ranges
        """
    @ranges.setter
    def ranges(self, arg0: collections.abc.Sequence[tuple[typing.SupportsInt | typing.SupportsIndex | None, typing.SupportsInt | typing.SupportsIndex | None]]) -> None:
        ...
class RegAdd(BaseRegEdit):
    """
    
    This class inherits from :class:`BaseRegEdit`
    
    Bulk adds some `KVPs`_ into some :class:`IfContentPart`
    
    Parameters
    ----------
    vals: List[Tuple[:class:`str`, :class:`str`]]
        The `KVPs`_ to add, in the order given
    
    latest: :class:`bool`
        Whether to add :attr:`vals` at the end of the :class:`IfContentPart` (or, if 'partRanges' is
        provided to :meth:`edit`, at the end of that window), instead of at the beginning :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, vals: typing.Any, latest: bool = True) -> None:
        ...
    def edit(self, part: typing.Any, sectionName: str, modType: typing.Any, modName: str = '', partRanges: typing.Any = None) -> typing.Any:
        """
        Adds every `KVP`_ in :attr:`vals` into 'part'
        
        With no 'partRanges', the `KVPs`_ go straight to the true beginning/end of 'part' (based on
        :attr:`latest`). With a 'partRanges' window, they instead go right after the last valid index of
        that window (or right before its first valid index, when :attr:`latest` is ``False``) -- an
        unbounded window edge falls back to the true end/beginning of 'part'. An empty :attr:`vals` or an
        empty 'partRanges' leaves 'part' untouched
        
        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the `IfTemplate` that is being editted
        
        sectionName: :class:`str`
            The name of the `section`_ that is being editted. Unused by this edit
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Unused by this edit
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partRanges: Optional[:class:`Ranges`]
            The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IfContentPart`
            The same part that was passed in, after editing
        """
    @property
    def latest(self) -> bool:
        """
        :class:`bool`: Whether to add :attr:`vals` at the end of the :class:`IfContentPart` (or, if
        'partRanges' is provided to :meth:`edit`, at the end of that window), instead of at the beginning
        """
    @latest.setter
    def latest(self, arg0: bool) -> None:
        ...
    @property
    def vals(self) -> typing.Any:
        """
        List[Tuple[:class:`str`, :class:`str`]]: The `KVPs`_ to add, in the order given
        """
    @vals.setter
    def vals(self, arg1: typing.Any) -> None:
        ...
class RegFillMissing(BaseIniGraphEdit):
    """
    
    This class inherits from :class:`BaseIniGraphEdit`
    
    Fills the :class:`IfContentPart`\\s of some caller/callee graph that are missing a particular
    register
    
    Parameters
    ----------
    reg: :class:`str`
        The register to search for
    
    fillMissing: Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]
        How to fill in the :class:`IfContentPart`\\s with their corresponding values :raw-html:`<br />` :raw-html:`<br />`
    
        If this argument is a string, will add the following line to: ``reg = fillMissing``
        If this argument is a list of tuples, will add the `KVPs`_ specified by each tuple into the missing part
        Otherwise, will modify the missing part according to the specified function
    
    fillMode: Optional[:class:`RegFillMissingMode`]
        What mode used to search and fill the missing register :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``RegFillMissingMode.FillMissing``
    
    dependOnDownload: :class:`bool`
        Whether the editting is dependent on :attr:`IniFile.downloadMode` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
    
    trackKeys: :class:`bool`
        Whether to keep track of the `KVPs`_ seen so far for colouring while walking the graph, so that
        the ``partFilter`` given to :meth:`edit` receives a populated
        :attr:`SectionIterData.colouring` to decide from :raw-html:`<br />` :raw-html:`<br />`
    
        When ``False``, that ``colouring`` is ``None`` and a filter can only discriminate on the part
        or the `section`_ itself :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
    
    keysToTrack: Optional[Set[:class:`str`]]
        Which keys 'trackKeys' should colour :raw-html:`<br />` :raw-html:`<br />`
    
        If this value is ``None``, then **every** key is tracked :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    @staticmethod
    def addCover(graph: typing.Any, reg: str, fillMissing: typing.Any) -> typing.Any:
        """
        Fills a fresh top :class:`IfContentPart` at each of 'graph''s roots, if 'reg' is missing in some
        :class:`IfContentPart` of 'graph'
        
        Nothing is added at all when every root already fully covers 'reg'
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to search
        
        reg: :class:`str`
            The register to search
        
        fillMissing: Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]
            How to modify the parts that are missing the desired register -- the same three shapes
            :attr:`fillMissing` accepts
        
        Returns
        -------
        :class:`IniSectionGraph`
            The same graph that was passed in, with its roots covered
        """
    @staticmethod
    def fillMissingGraph(graph: typing.Any, reg: str, fillMissing: typing.Any) -> typing.Any:
        """
        Fills the :class:`IfContentPart`\\s from 'graph' that are missing 'reg'
        
        Each part is filled at most once, even when it is reachable from more than one `section`_
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to search
        
        reg: :class:`str`
            The register to search
        
        fillMissing: Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]
            How to modify the parts that are missing the desired register -- the same three shapes
            :attr:`fillMissing` accepts
        
        Returns
        -------
        :class:`IniSectionGraph`
            The same graph that was passed in, with its missing parts filled
        """
    def __init__(self, reg: str, fillMissing: typing.Any, fillMode: typing.Any = None, dependOnDownload: bool = False, trackKeys: bool = False, keysToTrack: typing.Any = None) -> None:
        ...
    def edit(self, graph: typing.Any, modType: typing.Any, modName: str = '', partFilter: typing.Any = None, trackKeys: bool = False, keysToTrack: typing.Any = None) -> typing.Any:
        """
        Fills the parts of 'graph' that are missing :attr:`reg`, by whichever strategy :attr:`fillMode`
        names -- :meth:`fillMissingGraph` for ``RegFillMissingMode.FillMissing``, :meth:`addCover` for
        ``RegFillMissingMode.TopdownCover``
        
        'partFilter' restricts *which* parts get filled: it is asked once per candidate part, and an empty
        :class:`Ranges` result skips that one. Under ``RegFillMissingMode.TopdownCover`` it is asked once
        per root instead, against that root's own first :class:`IfContentPart`. This is the same convention
        :class:`GraphGroupEdit` already applies to its register edits -- only *which* parts are chosen; a
        non-empty result's actual ranges are not consulted, since filling a part appends a whole `KVP`_
        rather than editing occurrences at particular order indices
        
        Set :attr:`trackKeys` to give that filter a populated :attr:`SectionIterData.colouring` to decide
        from, narrowed to :attr:`keysToTrack`
        
        .. note::
            The pure-Python original accepted 'partFilter' and dropped it, so this edit applied to every
            missing part unconditionally. Honouring it is a deliberate behaviour change; an omitted
            'partFilter' still fills everything, exactly as before
        
        .. note::
            Under ``RegFillMissingMode.TopdownCover`` the colouring handed to 'partFilter' is empty by
            construction -- nothing precedes a root -- so :attr:`SectionIterData.sectionName` /
            :attr:`SectionIterData.section` are the useful discriminators there, not the tracked `KVPs`_.
            A root `section`_ holding no :class:`IfContentPart` at all is accepted, there being nothing to
            discriminate on
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Not read here -- only handed to 'partFilter'
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
            Which parts may be filled -- an empty :class:`Ranges` result skips that part, ``None`` accepts
            every part :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IniSectionGraph`
            The same graph that was passed in, after editing
        """
    def editFromIni(self, graph: typing.Any, ini: typing.Any, modType: typing.Any, modName: str = '', partFilter: typing.Any = None, trackKeys: bool = False, keysToTrack: typing.Any = None) -> typing.Any:
        """
        Fills the parts of 'graph' that are missing :attr:`reg`, honouring the download mode 'ini' was read
        under
        
        When :attr:`dependOnDownload` is ``False`` this is just :meth:`edit`. Otherwise
        ``DownloadMode.Disabled`` skips the edit entirely, and ``DownloadMode.Always`` normalizes the
        graph's branching structure first, so that a part missing the register on *some* branch is
        guaranteed to be its own :class:`IfContentPart`
        
        .. note::
            An 'ini' of ``None``, or one carrying no ``downloadMode`` attribute, reads as
            ``DownloadMode.Normal`` -- the mode under which this behaves identically to
            :attr:`dependOnDownload` being ``False``
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit
        
        ini: Optional[:class:`IniFile`]
            The associated .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Not read here -- only handed to 'partFilter'
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
            Which parts may be filled -- an empty :class:`Ranges` result skips that part. Forwarded
            straight to :meth:`edit` :raw-html:`<br />` :raw-html:`<br />`
        
            .. note::
                The third argument handed to 'partFilter' is always ``None`` rather than 'ini'.
                :meth:`edit` is reached through genuine Python attribute lookup (so a subclass's own
                override still wins), and its signature -- inherited from :class:`BaseIniGraphEdit` -- has
                nowhere to carry an .ini file. A plain C++ caller of the core class does get the real one
        
            :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IniSectionGraph`
            The same graph that was passed in, after editing
        """
    @property
    def dependOnDownload(self) -> bool:
        """
        :class:`bool`: Whether the editting is dependent on :attr:`IniFile.downloadMode`
        """
    @dependOnDownload.setter
    def dependOnDownload(self, arg1: bool) -> None:
        ...
    @property
    def fillMissing(self) -> typing.Any:
        """
        Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]: How to
        fill in the :class:`IfContentPart`\\s with their corresponding values
        """
    @fillMissing.setter
    def fillMissing(self, arg1: typing.Any) -> None:
        ...
    @property
    def fillMode(self) -> typing.Any:
        """
        :class:`RegFillMissingMode`: What mode used to search and fill the missing register
        """
    @fillMode.setter
    def fillMode(self, arg1: typing.Any) -> None:
        ...
    @property
    def keysToTrack(self) -> typing.Any:
        """
        Optional[Set[:class:`str`]]: Which keys :attr:`trackKeys` should colour, or ``None`` for every key
        """
    @keysToTrack.setter
    def keysToTrack(self, arg1: typing.Any) -> None:
        ...
    @property
    def reg(self) -> str:
        """
        :class:`str`: The register to search for
        """
    @reg.setter
    def reg(self, arg1: str) -> None:
        ...
    @property
    def trackKeys(self) -> bool:
        """
        :class:`bool`: Whether to keep track of the `KVPs`_ seen so far for colouring while walking the
        graph, so that the ``partFilter`` given to :meth:`edit` receives a populated
        :attr:`SectionIterData.colouring`
        """
    @trackKeys.setter
    def trackKeys(self, arg1: bool) -> None:
        ...
class RegNewVals(BaseRegEdit):
    """
    
    This class inherits from :class:`BaseRegEdit`
    
    Class for assigning new values to specific registers for some :class:`IfContentPart`
    
    .. note::
        A :class:`ReplaceIf` value's predicate is called as ``predicate(oldValue, modType)`` here --
        one argument wider than every ``replaceVals`` calls it with, since a register edit always
        knows which :class:`ModType` it is running for and deciding what to write based on that is
        the whole point of this class over a plain :meth:`IfContentPart.replaceVals` call. A
        single-argument predicate will raise :class:`TypeError` when :meth:`edit` runs
    
    Parameters
    ----------
    vals: Dict[:class:`str`, Union[:class:`str`, :class:`ReplaceList`, :class:`ReplaceIf`]]
        Defines which registers will have their values changed :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the names of the register and the values are the new values. Each value also
        accepts the richer forms :meth:`IfContentPart.replaceVals` takes -- a :class:`ReplaceList`
        (positional, by existing true left-to-right order) or a :class:`ReplaceIf` (conditional, by
        the wider predicate described above)
    
    addNewKVPs: :class:`bool`
        Whether to add new `KVPs`_ if the register keys do not exist in the :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
        
    """
    def __init__(self, vals: typing.Any, addNewKVPs: bool = False) -> None:
        ...
    def edit(self, part: typing.Any, sectionName: str, modType: typing.Any, modName: str = '', partRanges: typing.Any = None) -> typing.Any:
        """
        Assigns the new values in :attr:`vals` to 'part', by forwarding to
        :meth:`IfContentPart.replaceVals`
        
        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the `IfTemplate` that is being editted
        
        sectionName: :class:`str`
            The name of the `section`_ that is being editted. Unused by this edit
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Passed through as the second argument to every :class:`ReplaceIf`
            predicate in :attr:`vals` -- see this class's own note
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partRanges: Optional[:class:`Ranges`]
            The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IfContentPart`
            The same part that was passed in, after editing
        """
    @property
    def addNewKVPs(self) -> bool:
        """
        :class:`bool`: Whether to add new `KVPs`_ if the register keys do not exist in the
        :class:`IfContentPart`
        """
    @addNewKVPs.setter
    def addNewKVPs(self, arg0: bool) -> None:
        ...
    @property
    def vals(self) -> typing.Any:
        """
        Dict[:class:`str`, Union[:class:`str`, :class:`ReplaceList`, :class:`ReplaceIf`]]: Defines which
        registers will have their values changed, where the keys are the names of the register and the
        values are the new values
        """
    @vals.setter
    def vals(self, arg1: typing.Any) -> None:
        ...
class RegRemap(BaseRegEdit):
    """
    
    This class inherits from :class:`BaseRegEdit`
    
    Bulk-renames the register keys for some :class:`IfContentPart`
    
    Parameters
    ----------
    keyRemap: Dict[:class:`str`, Union[List[Union[:class:`str`, :class:`CppRemappedKeyData`]], :class:`CppKeyRemapData`]]
        The old key -> remap rules mapping to apply :raw-html:`<br />` :raw-html:`<br />`
    
        See :meth:`IfContentPart.remapKeys` for the full semantics of how a rule set is evaluated
        for a given key's occurrences
        
    """
    def __init__(self, keyRemap: typing.Any) -> None:
        ...
    def edit(self, part: typing.Any, sectionName: str, modType: typing.Any, modName: str = '', partRanges: typing.Any = None) -> typing.Any:
        """
        Applies :attr:`keyRemap` to 'part', by forwarding straight to :meth:`IfContentPart.remapKeys`
        
        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the `IfTemplate` that is being editted
        
        sectionName: :class:`str`
            The name of the `section`_ that is being editted. Unused by this edit
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Unused by this edit
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partRanges: Optional[:class:`Ranges`]
            The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IfContentPart`
            The same part that was passed in, after editing
        """
    @property
    def keyRemap(self) -> typing.Any:
        """
        Dict[:class:`str`, Union[List[Union[:class:`str`, :class:`CppRemappedKeyData`]], :class:`CppKeyRemapData`]]:
        The old key -> remap rules mapping to apply
        """
    @keyRemap.setter
    def keyRemap(self, arg1: typing.Any) -> None:
        ...
class RegRemove(BaseRegEdit):
    """
    
    This class inherits from :class:`BaseRegEdit`
    
    Bulk-removes register keys for some :class:`IfContentPart`
    
    Parameters
    ----------
    removeKeys: Dict[Any, Optional[Callable[[:class:`int`, Any], :class:`bool`]]]
        Each key to remove, mapped to its own optional check predicate :raw-html:`<br />` :raw-html:`<br />`
    
        See :meth:`IfContentPart.removeKeys` for the full semantics of how the predicates decide
        which occurrences of a key actually get removed
        
    """
    def __init__(self, removeKeys: typing.Any) -> None:
        ...
    def edit(self, part: typing.Any, sectionName: str, modType: typing.Any, modName: str = '', partRanges: typing.Any = None) -> typing.Any:
        """
        Removes every key in :attr:`removeKeys` from 'part', by forwarding straight to
        :meth:`IfContentPart.removeKeys`
        
        Parameters
        ----------
        part: :class:`IfContentPart`
            The part of the `IfTemplate` that is being editted
        
        sectionName: :class:`str`
            The name of the `section`_ that is being editted. Unused by this edit
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Unused by this edit
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partRanges: Optional[:class:`Ranges`]
            The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`IfContentPart`
            The same part that was passed in, after editing
        """
    @property
    def removeKeys(self) -> typing.Any:
        """
        Dict[Any, Optional[Callable[[:class:`int`, Any], :class:`bool`]]]: Each key to remove, mapped to
        its own optional check predicate
        """
    @removeKeys.setter
    def removeKeys(self, arg1: typing.Any) -> None:
        ...
class RegSurroundedAdd(BaseIniGraphEdit):
    """
    
    This class inherits from :class:`BaseIniGraphEdit`
    
    Adds a `KVP`_ into some caller/callee graph of :class:`IniSectionGraph`, at every location that is
    `surrounded` by a particular set of registers: after every register specified at 'beforeRegs' has
    been seen at least once (and accepted by its predicate) and before every register specified at
    'afterRegs' has been seen at least once (and accepted by its predicate)
    
    Parameters
    ----------
    addition: Tuple[:class:`str`, :class:`str`]
        The `KVP`_ to add
    
    beforeRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
        The registers that must come before :attr:`addition` (ie. :attr:`addition` gets added after
        these registers) :raw-html:`<br />` :raw-html:`<br />`
    
        * The keys are the names of the registers
        * The values are the predicates for which particular occurence of the register to accept,
          taking in the value of the occurence -- ``None`` accepts any occurence
    
        This condition is only satisfied once at least one accepted occurence has been seen for
        **every** key specified in this argument :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    afterRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
        The registers that must come after :attr:`addition` -- same format/semantics as
        :attr:`beforeRegs`, except the condition applies for coming after :attr:`addition` instead of
        before it :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    latest: :class:`bool`
        Whether to add :attr:`addition` at the latest valid location within the surrounded window,
        instead of the earliest one :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
        
    """
    def __init__(self, addition: typing.Any, beforeRegs: typing.Any = None, afterRegs: typing.Any = None, latest: bool = False) -> None:
        ...
    def edit(self, graph: typing.Any, modType: typing.Any, modName: str = '', partFilter: typing.Any = None, trackKeys: bool = False, keysToTrack: typing.Any = None) -> typing.Any:
        """
        Fills 'graph' with a `surrounded` window insertion of :attr:`addition`, honouring :attr:`latest`
        for which valid location within each window is chosen
        
        .. note::
            'trackKeys'/'keysToTrack' are the caller's key-tracking defaults, handed down by
            :class:`BaseIniGraphEdit`'s contract (:class:`GraphGroupEdit` passes its own). This edit builds
            its own colourings from its own :attr:`beforeRegs`/:attr:`afterRegs`, so it has no use for
            them -- they are accepted only so the shared call convention keeps working
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix. Unused by this edit -- only forwarded to 'partFilter'
        
        modName: :class:`str`
            The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
            Which order indices may be used within a part -- ``None`` accepts every index :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        trackKeys: :class:`bool`
            Unused by this edit. **Default**: ``False``
        
        keysToTrack: Optional[Set[:class:`str`]]
            Unused by this edit. **Default**: ``None``
        
        Returns
        -------
        :class:`IniSectionGraph`
            The same graph that was passed in, after editing
        """
    @property
    def addition(self) -> tuple[str, str]:
        """
        Tuple[:class:`str`, :class:`str`]: The `KVP`_ to add
        """
    @addition.setter
    def addition(self, arg1: typing.Any) -> None:
        ...
    @property
    def afterRegs(self) -> dict:
        """
        Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]: The registers that must come
        after :attr:`addition`
        """
    @afterRegs.setter
    def afterRegs(self, arg1: typing.Any) -> None:
        ...
    @property
    def beforeRegs(self) -> dict:
        """
        Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]: The registers that must come
        before :attr:`addition`
        """
    @beforeRegs.setter
    def beforeRegs(self, arg1: typing.Any) -> None:
        ...
    @property
    def latest(self) -> bool:
        """
        :class:`bool`: Whether to add :attr:`addition` at the latest valid location within the surrounded
        window, instead of the earliest one
        """
    @latest.setter
    def latest(self, arg0: bool) -> None:
        ...
class RemapBlendReplace(BaseResEdit):
    """
    
    This class inherits from :class:`ResReplace`
    
    Class that builds the necessary part to replace some Blend.buf file
    
    Parameters
    ----------
    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`
    
        The tuple contains:
    
        #. The index for the .ini file
        #. The name of the component
        #. The name of the object
    
    resType: :class:`str`
        The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``resourceRemapBlend``
    
    fixFunc: Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]
        A custom function for fixing the Blend.buf file :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    resSubType: Optional[:class:`str`]
        The name of the subtype of the resource :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    fromComp: Optional[:class:`str`]
        The specific component to remap from :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    toComp: Optional[:class:`str`]
        The specific component to remap to :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    @staticmethod
    def fileAddGraphId(file: str, graphId: str = '') -> str:
        """
        Adds the unique id for the :class:`IniSectionGraph` of the resource to the name of the file
        
        Parameters
        ----------
        file: :class:`str`
            The path to the file to add the id to
        
        graphId: :class:`str`
            The id to add :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The file with the id added
        """
    @staticmethod
    def getFileId(modObj: typing.Any, sectionName: str, part: typing.Any, orderInd: typing.SupportsInt | typing.SupportsIndex, file: str) -> str:
        """
        Retrieves a unique id for a file within a single .ini file
        
        .. note::
            The returned value is not byte-identical to the one the pure-Python original produced -- it is
            an opaque, within-one-run dictionary key that is never persisted or written to a file
        
        Parameters
        ----------
        modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The mod object holding the newly created :class:`IniSectionGraph` for the resource
        
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
        :class:`str`
            The unique id for the file
        """
    def __init__(self, resModObj: typing.Any, resType: str = 'resourceRemapBlend', fixFunc: typing.Any = None, resSubType: typing.Any = None, fromComp: typing.Any = None, toComp: typing.Any = None) -> None:
        ...
    def buildResModel(self, resType: str, ini: typing.Any, srcPath: str, fixedPath: str, modType: typing.Any = None, *args, modName: str = '', **kwargs) -> typing.Any:
        """
        Builds the model for the resource
        
        .. note::
            The ``type`` of the built resource comes from :attr:`resType`, not from the 'resType' argument
            -- faithful to the pure-Python original
        
        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource. Unused
        
        ini: :class:`IniFile`
            The .ini file to build the resource for
        
        srcPath: :class:`str`
            The file path to the original resource
        
        fixedPath: :class:`str`
            The file path to the fixed resource
        
        modType: :class:`ModType`
            The type of mod being fixed -- the vertex group remap comes from it
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`RemapBlendResource`
            The built resource
        """
    def buildResModels(self: BaseResEdit, graph: typing.Any, ini: typing.Any = None, modType: typing.Any = None, resources: typing.Any = None, resourceFilter: typing.Any = None, modName: str = '', graphId: str = '', resModObj: typing.Any = None) -> None:
        """
        Builds and saves the resources, given the :class:`IniSectionGraph` for a resource
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph for the particular resource
        
        ini: Optional[:class:`IniFile`]
            The .ini file to build the resource for
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored, keyed by the unique id for the source file (created
            from :meth:`getFileId`). If ``None``, the models are appended to :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for -- takes the source file and its
            assigned id :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        graphId: :class:`str`
            The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resModObj: Optional[Tuple[:class:`int`, :class:`str`, :class:`str`]]
            The mod object used to create the unique id for the resources :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def buildResources(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', resourceFilter: typing.Any = None, resources: typing.Any = None, copySections: bool = False) -> list:
        """
        Builds the :class:`IniSectionGraph` and the corresponding models for the resources
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
            The associated original .ini file being fixed
        
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored. If ``None``, they are appended to
            :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        copySections: :class:`bool`
            Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The group of graphs that now includes the newly created graph for the resource
        
            .. tip::
                You can access the newly generated graph using :attr:`resModObj` on the group of graphs
        """
    def clear(self: BaseResEdit) -> None:
        """
        Clears any saved state information
        """
    def collectResourceName(self: BaseResEdit, oldResourceName: str, newResourceName: str) -> tuple[str, str]:
        """
        Collects the name of the fixed resource `section`_ (used for the 'collectedSections' parameter in
        :meth:`buildResources`)
        
        Parameters
        ----------
        oldResourceName: :class:`str`
            The old name of the resource `section`_
        
        newResourceName: :class:`str`
            The fixed name for the resource `section`_ (created by :meth:`getFixResourceName`)
        
        Returns
        -------
        Tuple[:class:`str`, :class:`str`]
            A tuple where the first value is the old resource name and the second is the new resource name
        """
    def getFixFile(self: BaseResEdit, file: str, modType: typing.Any = None, modName: str = '', graphId: str = '') -> str:
        """
        Retrieves the file path to the fixed resource
        
        Parameters
        ----------
        file: :class:`str`
            The file path to the original resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
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
    def getFixResourceName(self: BaseResEdit, resource: str, modType: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Retrieves the name of the fixed resource `section`_
        
        Parameters
        ----------
        resource: :class:`str`
            The name of the original resource `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        Optional[:class:`str`]
            The `section`_ name of the fixed resource. ``None`` indicates there was no name change between
            the original resource and the fixed resource
        """
    def getResGraph(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', rename: bool = True, copySections: bool = False) -> typing.Any:
        """
        Retrieves the particular :class:`IniSectionGraph` for the resource
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource -- the keys are the old names of the
            `sections`_ and the values are the fixed names
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
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
    def renameUncollectedSection(self: BaseResEdit, sectionName: str, modType: typing.Any = None, modName: str = '') -> str:
        """
        The name an uncollected `section`_ gets renamed to -- :meth:`getFixResourceName`, or the
        `section`_'s own name when that reports no change
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """
    @property
    def fixFunc(self) -> typing.Any:
        """
        Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]: A custom function for fixing the
        Blend.buf file
        """
    @fixFunc.setter
    def fixFunc(self, arg1: typing.Any) -> None:
        ...
    @property
    def fromComp(self) -> typing.Any:
        """
        Optional[:class:`str`]: The specific component to remap from
        """
    @fromComp.setter
    def fromComp(self, arg1: typing.Any) -> None:
        ...
    @property
    def graphReplaceMode(self) -> typing.Any:
        """
        :class:`IniGraphReplaceMode`: What to do when the corresponding :class:`IniSectionGraph` to
        construct already exists
        """
    @graphReplaceMode.setter
    def graphReplaceMode(self, arg1: typing.Any) -> None:
        ...
    @property
    def resModObj(self) -> typing.Any:
        """
        Tuple[:class:`int`, :class:`str`, :class:`str`]: The mod object to hold the newly created
        :class:`IniSectionGraph` for the resource
        """
    @resModObj.setter
    def resModObj(self, arg1: typing.Any) -> None:
        ...
    @property
    def resSubType(self) -> typing.Any:
        """
        Optional[:class:`str`]: The name of the subtype of the resource
        """
    @resSubType.setter
    def resSubType(self, arg1: typing.Any) -> None:
        ...
    @property
    def resType(self) -> str:
        """
        :class:`str`: The name of the type of resource
        """
    @resType.setter
    def resType(self, arg0: str) -> None:
        ...
    @property
    def toComp(self) -> typing.Any:
        """
        Optional[:class:`str`]: The specific component to remap to
        """
    @toComp.setter
    def toComp(self, arg1: typing.Any) -> None:
        ...
class RemapBlendResource(RemapIniFixResource):
    """
    
    This class inherits from :class:`RemapIniFixResource`
    
    Class for fixing some ``Blend.buf`` file used by the overall remap process
        
    """
    def __init__(self, iniFolderPath: str, srcPath: str, fixedPath: str, vgRemap: VGRemap, type: str = 'resourceRemapBlend', fixFunc: collections.abc.Callable[[RemapBlendResource], bool] = None, blendElements: typing.Any = None) -> None:
        """
        Constructs a new blend resource
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The path to the folder of the .ini file
        
        srcPath: :class:`str`
            The file path to the resource
        
        fixedPath: :class:`str`
            The file path to the fixed resource
        
        vgRemap: :class:`VGRemap`
            The vertex group remap for the ``Blend.buf`` file
        
        type: :class:`str`
            The name for the type of resource
        
            **Default**: ``"resourceRemapBlend"``
        
        fixFunc: Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]
            Custom function for fixing the resource, overriding the default behavior if given
        
            **Default**: ``None``
        
        blendElements: Optional[List[:class:`BufElementType`]]
            The sequence of elements for constructing the ``Blend.buf`` file. If this is ``None`` or empty,
            the elements for a GIMI character are used instead
        
            **Default**: ``None``
        """
    def createBlend(self) -> BlendFile:
        """
        Creates the blend file -- a fresh copy of the stored blend elements is cloned into it
        
        Returns
        -------
        :class:`BlendFile`
            The created blend file
        """
    def fix(self) -> bool:
        """
        Fixes the resource -- calls the custom 'fixFunc' if set at construction, otherwise performs a
        vertex group remap on the ``Blend.buf`` file
        
        Returns
        -------
        :class:`bool`
            Whether the resource was fixed
        """
    @property
    def fixFunc(self) -> collections.abc.Callable[[RemapBlendResource], bool]:
        """
        Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default behavior if set
        """
    @fixFunc.setter
    def fixFunc(self, arg0: collections.abc.Callable[[RemapBlendResource], bool]) -> None:
        ...
    @property
    def vgRemap(self) -> VGRemap:
        """
        :class:`VGRemap`: The vertex group remap for the ``Blend.buf`` file
        """
    @vgRemap.setter
    def vgRemap(self, arg0: VGRemap) -> None:
        ...
class RemapIniDownload(RemapIniResource):
    """
    
    This class inherits from :class:`RemapIniResource`
    
    Class for some download resource in a .ini file that's used by the overall remap process --
    unlike the deprecated pure-Python original, this class does not accept a ``Mod`` object anywhere --
    :meth:`remapFix`'s progress-reporting callbacks ('downloadHandler'/'cacheHitHandler') are supplied
    by the caller directly instead
        
    """
    def __init__(self, iniFolderPath: str, srcPath: str, download: typing.Any, type: str = 'download', fixFunc: collections.abc.Callable[[RemapIniDownload, CachedFileStats], bool] = None) -> None:
        """
        Constructs a new download resource
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The path to the folder of the .ini file
        
        srcPath: :class:`str`
            The file path to the resource
        
        download: :class:`FileDownload`
            The downloader associated with the file. Ownership is transferred into this resource
        
        type: :class:`str`
            The name for the type of resource
        
            **Default**: ``"download"``
        
        fixFunc: Optional[Callable[[:class:`RemapIniDownload`, :class:`CachedFileStats`], :class:`bool`]]
            Custom function for fixing the resource, overriding the default download behavior if given --
            takes this resource and the download stats to mutate, and returns whether a fresh download
            occurred (as opposed to a cache hit)
        
            **Default**: ``None``
        """
    def fix(self, downloadStats: CachedFileStats, proxy: str | None = None) -> bool:
        """
        Downloads the resource -- calls the custom 'fixFunc' if set at construction, otherwise the default
        download behavior
        
        Parameters
        ----------
        downloadStats: :class:`CachedFileStats`
            The stats for the file download to mutate
        
        proxy: Optional[:class:`str`]
            The link to the proxy server used for any internet network requests made, if any
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`bool`
            Whether a fresh download occurred (``False`` for a cache hit)
        """
    def remapFix(self, stats: RemapStats, proxy: str | None = None, downloadHandler: collections.abc.Callable[[str], None] = None, cacheHitHandler: collections.abc.Callable[[str], None] = None) -> bool:
        """
        Fixes the resource for the overall remap process -- same as :meth:`fix`, additionally invoking
        'downloadHandler'/'cacheHitHandler' (whichever applies) with this resource's ``srcPath`` once the
        download/cache-hit completes
        
        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process
        
        proxy: Optional[:class:`str`]
            The link to the proxy server used for any internet network requests made, if any
        
            **Default**: ``None``
        
        downloadHandler: Optional[Callable[[:class:`str`], Any]]
            Called with 'srcPath' if a fresh download occurred
        
            **Default**: ``None``
        
        cacheHitHandler: Optional[Callable[[:class:`str`], Any]]
            Called with 'srcPath' if the file was retrieved from the cache instead
        
            **Default**: ``None``
        
        Returns
        -------
        :class:`bool`
            Whether a fresh download occurred (``False`` for a cache hit)
        """
class RemapIniFixResource(IniFixResource, RemapIniResourceMixin):
    """
    
    This class inherits from :class:`IniFixResource` and :class:`RemapIniResourceMixin`
    
    Base class for some resource to fix in a .ini file that's used by the overall remap process
        
    """
    def __init__(self, type: str, iniFolderPath: str, srcPath: str, fixedPath: str) -> None:
        """
        Constructs a new resource to fix -- see :class:`IniFixResource`'s constructor for the parameters
        """
class RemapIniGroupedResource(IniGroupedResource, RemapIniResourceMixin):
    """
    
    This class inherits from :class:`IniGroupedResource` and :class:`RemapIniResourceMixin`
    
    Base class for a group of resources to fix in a .ini file that's used by the overall remap process
        
    """
    def __init__(self, name: str, resources: typing.Any = None, fixFunc: collections.abc.Callable[[...], bool] = None, isBuilt: bool = True) -> None:
        """
        Constructs a new group of resources to fix -- see :class:`IniGroupedResource`'s constructor for the parameters
        """
class RemapIniRemover(BaseIniRemover):
    """
    
    This class inherits from :class:`BaseIniRemover`
    
    Class for removing the fixes from .ini files, by reachability rather than by name
    
    Parameters
    ----------
    iniFile: :class:`IniFile`
        The .ini file to remove the fix from
        
    """
    def __init__(self, iniFile: typing.Any = None) -> None:
        ...
    def getRemovedResources(self) -> typing.Any:
        """
        Every resource the last :meth:`RemapIniRemover.remove` took out with the `sections`_ that declared it
        
        Returns
        -------
        Dict[:class:`str`, List[:class:`IniResource`]]
            The removed resources, keyed by the type of resource -- one of ``"blend"``, ``"position"``,
            ``"texcoord"``, ``"buf"``, ``"texEdit"``, ``"texAdd"``, ``"download"`` or ``"other"``, each of
            which names a :class:`RemapStats` attribute
        """
    def getRemovedSectionNames(self) -> list[str]:
        """
        Every `section`_ name the last :meth:`RemapIniRemover.remove` deleted
        
        Returns
        -------
        List[:class:`str`]
            The names of the removed `sections`_, in the order the .ini file declared them
        """
    def getTargetSectionNames(self) -> list[str]:
        """
        The `sections`_ the last :meth:`RemapIniRemover.remove` treated as this software's own output
        
        Returns
        -------
        List[:class:`str`]
            The names of the target `sections`_, in the order the .ini file declared them
        """
    def remove(self, parse: bool = False, writeBack: bool = True, context: typing.Any = None) -> str:
        """
        Removes the fix from the .ini file
        
        The fix is whatever the fix boilerplate surrounds, plus any ``Remap``-named leftovers outside it
        that carry one of the mod type's hashes -- together with everything those reference and everything
        that references them.
        
        Pass a :class:`IniRemovalContext` with ``ignoreModType`` set to drop the hash question and take
        every ``Remap``-named leftover regardless of whose it is.
        
        Parameters
        ----------
        parse: :class:`bool`
            Ignored -- the resources that went with the removed `sections`_ are always collected, and are
            available from :meth:`RemapIniRemover.getRemovedResources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        writeBack: :class:`bool`
            Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``True``
        
        context: :class:`IniRemovalContext`
            The per-call options for this removal :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``, which means a default-constructed one -- ie. the strict rule above
        
        Returns
        -------
        :class:`str`
            The new content of the .ini file
        """
    @property
    def iniFile(self) -> typing.Any:
        """
        :class:`IniFile`: The .ini file that the fix will be removed from
        """
    @iniFile.setter
    def iniFile(self, arg0: typing.Any) -> None:
        ...
class RemapIniResource(IniResource, RemapIniResourceMixin):
    """
    
    This class inherits from :class:`IniResource` and :class:`RemapIniResourceMixin`
    
    Base class for some resource in a .ini file that's used by the overall remap process
        
    """
    def __init__(self, type: str, iniFolderPath: str, srcPath: str) -> None:
        """
        Constructs a new resource -- see :class:`IniResource`'s constructor for the parameters
        """
class RemapIniResourceMixin:
    """
    
    Interface for a resource in a .ini file that's used by the overall remap process
        
    """
    def __init__(self) -> None:
        ...
    def fixEncounteredError(self, stats: RemapStats) -> bool:
        """
        Determines whether the fixed resource has previously encountered an error
        
        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process
        
        Returns
        -------
        :class:`bool`
            Whether the fixed resource has encountered an error
        """
    def fixExists(self, stats: RemapStats) -> bool:
        """
        Determines whether the fixed resource already exists on disk
        
        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process
        
        Returns
        -------
        :class:`bool`
            Whether the fixed resource already exists
        """
    def fixIsFixed(self, stats: RemapStats) -> bool:
        """
        Determines whether the fixed resource was already fixed
        
        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process
        
        Returns
        -------
        :class:`bool`
            Whether the fixed resource was already fixed
        """
    def hasRequired(self) -> bool:
        """
        Determines whether all the necessary data has been collected to fix this resource
        
        Returns
        -------
        :class:`bool`
            Whether all the required data is gathered
        """
    def srcEncounteredError(self, stats: RemapStats) -> bool:
        """
        Determines whether the (unfixed) resource has previously encountered an error
        
        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process
        
        Returns
        -------
        :class:`bool`
            Whether the resource has encountered an error
        """
    def srcIsFixed(self, stats: RemapStats) -> bool:
        """
        Determines whether the (unfixed) resource was already fixed
        
        Parameters
        ----------
        stats: :class:`RemapStats`
            The stats tracked by the remap process
        
        Returns
        -------
        :class:`bool`
            Whether the resource was already fixed
        """
class RemapStats:
    """
    
    The file stats for the overall remap process
        
    """
    def __init__(self) -> None:
        ...
    def clear(self) -> None:
        """
        Clears all the stats for the remap process
        """
    @property
    def blend(self) -> FileStats:
        """
        :class:`FileStats`: Stats about whether some ``Blend.buf`` files got fixed/skipped/removed
        """
    @blend.setter
    def blend(self, arg0: FileStats) -> None:
        ...
    @property
    def buf(self) -> FileStats:
        """
        :class:`FileStats`: Stats about whether some other ``.buf`` files got fixed/skipped/removed
        """
    @buf.setter
    def buf(self, arg0: FileStats) -> None:
        ...
    @property
    def download(self) -> CachedFileStats:
        """
        :class:`CachedFileStats`: Stats about whether some downloaded mod files have been recently downloaded/removed
        """
    @download.setter
    def download(self, arg0: CachedFileStats) -> None:
        ...
    @property
    def ini(self) -> FileStats:
        """
        :class:`FileStats`: Stats about whether some .ini files got fixed/skipped/undone
        """
    @ini.setter
    def ini(self, arg0: FileStats) -> None:
        ...
    @property
    def other(self) -> FileStats:
        """
        :class:`FileStats`: Stats about whether some files of no recognized kind got fixed/skipped/removed
        """
    @other.setter
    def other(self, arg0: FileStats) -> None:
        ...
    @property
    def position(self) -> FileStats:
        """
        :class:`FileStats`: Stats about whether some ``Position.buf`` files got fixed/skipped/removed
        """
    @position.setter
    def position(self, arg0: FileStats) -> None:
        ...
    @property
    def texAdd(self) -> FileStats:
        """
        :class:`FileStats`: Stats about whether a brand new texture file created by this software has been created/removed
        """
    @texAdd.setter
    def texAdd(self, arg0: FileStats) -> None:
        ...
    @property
    def texEdit(self) -> FileStats:
        """
        :class:`FileStats`: Stats about whether an existing texture file has been edited/removed
        """
    @texEdit.setter
    def texEdit(self, arg0: FileStats) -> None:
        ...
    @property
    def texcoord(self) -> FileStats:
        """
        :class:`FileStats`: Stats about whether some ``Texcoord.buf`` files got fixed/skipped/removed
        """
    @texcoord.setter
    def texcoord(self, arg0: FileStats) -> None:
        ...
class RemapTexAddResource(RemapIniResource):
    """
    
    This class inherits from :class:`RemapIniResource`
    
    Class for adding a brand new texture file used by the overall remap process
        
    """
    def __init__(self, iniFolderPath: str, srcPath: str, texCreator: CppTexCreator, type: str = 'resourceRemapTexAdd', fixFunc: collections.abc.Callable[[RemapTexAddResource], bool] = None) -> None:
        """
        Constructs a new texture-add resource
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The path to the folder of the .ini file
        
        srcPath: :class:`str`
            The file path to the resource
        
        texCreator: :class:`CppTexCreator`
            The texture creator used to create the ``.dds`` file if it's missing
        
        type: :class:`str`
            The name for the type of resource
        
            **Default**: ``"resourceRemapTexAdd"``
        
        fixFunc: Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]
            Custom function for fixing the resource, overriding the default behavior if given
        
            **Default**: ``None``
        """
    def fix(self) -> bool:
        """
        Fixes the resource -- calls the custom 'fixFunc' if set at construction, otherwise creates the
        texture file at ``srcPath`` if it doesn't already exist
        
        Returns
        -------
        :class:`bool`
            Whether the resource was fixed
        """
    @property
    def fixFunc(self) -> collections.abc.Callable[[RemapTexAddResource], bool]:
        """
        Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default behavior if set
        """
    @fixFunc.setter
    def fixFunc(self, arg0: collections.abc.Callable[[RemapTexAddResource], bool]) -> None:
        ...
    @property
    def texCreator(self) -> CppTexCreator:
        """
        :class:`CppTexCreator`: The texture creator used to create the ``.dds`` file if it's missing
        """
    @texCreator.setter
    def texCreator(self, arg0: CppTexCreator) -> None:
        ...
class RemapTexEditResource(RemapIniFixResource):
    """
    
    This class inherits from :class:`RemapIniFixResource`
    
    Class for editing a texture file used by the overall remap process
    
    The texture counterpart to :class:`RemapBlendResource`, and shaped like it rather than like
    :class:`RemapTexAddResource`: an **edit** reads one file and writes another, so it carries the
    ``srcPath``/``fixedPath`` pair. An *add* has only the one path
        
    """
    def __init__(self, iniFolderPath: str, srcPath: str, fixedPath: str, texEditor: CppTexEditor, type: str = 'resourceRemapTexEdit', fixFunc: collections.abc.Callable[[RemapTexEditResource], bool] = None) -> None:
        """
        Constructs a new texture-edit resource
        
        Parameters
        ----------
        iniFolderPath: :class:`str`
            The path to the folder of the .ini file
        
        srcPath: :class:`str`
            The file path to the resource
        
        fixedPath: :class:`str`
            The file path to the fixed resource
        
        texEditor: :class:`CppTexEditor`
            The texture editor used to edit the ``.dds`` file
        
        type: :class:`str`
            The name for the type of resource
        
            **Default**: ``"resourceRemapTexEdit"``
        
        fixFunc: Optional[Callable[[:class:`RemapTexEditResource`], :class:`bool`]]
            Custom function for fixing the resource, overriding the default behavior if given
        
            **Default**: ``None``
        """
    def fix(self) -> bool:
        """
        Fixes the resource -- calls the custom 'fixFunc' if set at construction, otherwise edits the
        texture at ``srcPath`` and writes the result to ``fixedPath``
        
        Returns
        -------
        :class:`bool`
            Whether the resource was fixed
        """
    @property
    def fixFunc(self) -> collections.abc.Callable[[RemapTexEditResource], bool]:
        """
        Optional[Callable[[:class:`RemapTexEditResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default behavior if set
        """
    @fixFunc.setter
    def fixFunc(self, arg0: collections.abc.Callable[[RemapTexEditResource], bool]) -> None:
        ...
    @property
    def texEditor(self) -> CppTexEditor:
        """
        :class:`CppTexEditor`: The texture editor used to edit the ``.dds`` file
        """
    @texEditor.setter
    def texEditor(self, arg0: CppTexEditor) -> None:
        ...
class RemappedKeyData:
    """
    
    A single rule inside a :meth:`OrderedMultiMap.remapKeys` remap list, expressing a
    *conditional* and/or *repositioned* rename (as opposed to a plain key, which always fires and
    stays in place).
    
    Parameters
    ----------
    key: Any
        The new key to remap matching occurrences to
    
    check: Optional[Callable[[Any, Any], :class:`bool`]]
        An optional predicate over ``(oldKey, oldValue)``. If omitted, this rule always fires. If
        present, it fires only when the predicate returns ``True`` for a given occurrence :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    toInd: Optional[:class:`int`]
        An optional target index. If present, every entry this rule produces (across all
        occurrences it fires for) is moved, as a group, to this index once the remap is complete --
        using the exact same index semantics, conflict resolution, and front/back
        overflow-clustering rules as :meth:`OrderedMultiMap.reorder`'s target index. If omitted,
        produced entries stay wherever the remap naturally places them (in place of the occurrence
        that produced them) :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
            
    """
    def __init__(self, key: typing.Any, check: collections.abc.Callable[[typing.Any, typing.Any], bool] | None = None, toInd: typing.SupportsInt | typing.SupportsIndex | None = None) -> None:
        ...
    @property
    def check(self) -> collections.abc.Callable[[typing.Any, typing.Any], bool] | None:
        """
        Optional[Callable[[Any, Any], :class:`bool`]]: The optional firing predicate
        """
    @property
    def key(self) -> typing.Any:
        """
        Any: The new key to remap matching occurrences to
        """
    @property
    def toInd(self) -> int | None:
        """
        Optional[:class:`int`]: The optional repositioning target index
        """
class ReplaceIf:
    """
    
    A :meth:`OrderedMultiMap.replaceVals` spec: replace this key's value with ``value``,
    wherever ``predicate(oldValue)`` is ``True``.
    
    .. note::
        ``predicate`` is stored as-is and each consumer decides what it passes to it. Every
        ``replaceVals`` (:meth:`OrderedMultiMap.replaceVals`, :meth:`OrderedMultiMapSqrt.replaceVals`,
        :meth:`IOrderedMultiMap.replaceVals`, :meth:`IfContentPart.replaceVals`) calls it with just the
        old value. The one deliberate exception is :class:`RegNewVals`, which calls it as
        ``predicate(oldValue, modType)`` -- see that class for why.
    
    Parameters
    ----------
    value: Any
        The replacement value
    
    predicate: Callable[..., :class:`bool`]
        The predicate deciding whether a given old value should be replaced. Takes ``(oldValue)``
        for every ``replaceVals``, or ``(oldValue, modType)`` when handed to :class:`RegNewVals`
            
    """
    def __init__(self, value: typing.Any, predicate: typing.Any) -> None:
        ...
    @property
    def predicate(self) -> typing.Any:
        """
        Callable[..., :class:`bool`]: The replacement predicate
        """
    @property
    def value(self) -> typing.Any:
        """
        Any: The replacement value
        """
class ReplaceList:
    """
    
    A :meth:`OrderedMultiMap.replaceVals` spec: update this key's entries positionally from a
    list of values -- the i-th existing entry (true left-to-right order) gets ``values[i]``. A list
    shorter than the key's entry count leaves the remaining entries untouched; a longer list just
    has its extra values unused.
    
    Parameters
    ----------
    values: List[Any]
        The values to assign positionally
            
    """
    def __init__(self, values: collections.abc.Sequence[typing.Any]) -> None:
        ...
    @property
    def values(self) -> list[typing.Any]:
        """
        List[Any]: The values to assign positionally
        """
class ResCreate(BaseResEdit):
    """
    
    This class inherits from :class:`BaseResEdit`
    
    Class that creates the necessary parts for a brand-new fixed resource, building its `sections`_ from
    scratch rather than from the .ini file's existing ones
    
    Parameters
    ----------
    resType: :class:`str`
        The name of the type of resource
    
    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource
    
    graphReplaceMode: :class:`IniGraphReplaceMode`
        What to do when the corresponding :class:`IniSectionGraph` to construct already exists :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``IniGraphReplaceMode.Ignore``
        
    """
    @staticmethod
    def fileAddGraphId(file: str, graphId: str = '') -> str:
        """
        Adds the unique id for the :class:`IniSectionGraph` of the resource to the name of the file
        
        Parameters
        ----------
        file: :class:`str`
            The path to the file to add the id to
        
        graphId: :class:`str`
            The id to add :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The file with the id added
        """
    @staticmethod
    def getFileId(modObj: typing.Any, sectionName: str, part: typing.Any, orderInd: typing.SupportsInt | typing.SupportsIndex, file: str) -> str:
        """
        Retrieves a unique id for a file within a single .ini file
        
        .. note::
            The returned value is not byte-identical to the one the pure-Python original produced -- it is
            an opaque, within-one-run dictionary key that is never persisted or written to a file
        
        Parameters
        ----------
        modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The mod object holding the newly created :class:`IniSectionGraph` for the resource
        
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
        :class:`str`
            The unique id for the file
        """
    def __init__(self, resType: str, resModObj: typing.Any, graphReplaceMode: typing.Any = None) -> None:
        ...
    def buildResModel(self, resType: str, ini: typing.Any, srcPath: str, modType: typing.Any = None, *args, modName: str = '', **kwargs) -> typing.Any:
        """
        Builds the model for the resource
        
        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource
        
        ini: :class:`IniFile`
            The .ini file to build the resource for
        
        srcPath: :class:`str`
            The file path to the original resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`IniResource`
            The built resource
        """
    def buildResModels(self: BaseResEdit, graph: typing.Any, ini: typing.Any = None, modType: typing.Any = None, resources: typing.Any = None, resourceFilter: typing.Any = None, modName: str = '', graphId: str = '', resModObj: typing.Any = None) -> None:
        """
        Builds and saves the resources, given the :class:`IniSectionGraph` for a resource
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph for the particular resource
        
        ini: Optional[:class:`IniFile`]
            The .ini file to build the resource for
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored, keyed by the unique id for the source file (created
            from :meth:`getFileId`). If ``None``, the models are appended to :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for -- takes the source file and its
            assigned id :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        graphId: :class:`str`
            The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resModObj: Optional[Tuple[:class:`int`, :class:`str`, :class:`str`]]
            The mod object used to create the unique id for the resources :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def buildResources(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', resourceFilter: typing.Any = None, resources: typing.Any = None, copySections: bool = False) -> list:
        """
        Builds the :class:`IniSectionGraph` and the corresponding models for the resources
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
            The associated original .ini file being fixed
        
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored. If ``None``, they are appended to
            :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        copySections: :class:`bool`
            Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The group of graphs that now includes the newly created graph for the resource
        
            .. tip::
                You can access the newly generated graph using :attr:`resModObj` on the group of graphs
        """
    def buildSection(self, sectionName: str, modType: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Builds a `section`_ for the resource
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name for the `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix from
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        Optional[:class:`IfTemplate`]
            The generated `section`_
        """
    def clear(self: BaseResEdit) -> None:
        """
        Clears any saved state information
        """
    def collectResourceName(self: BaseResEdit, oldResourceName: str, newResourceName: str) -> tuple[str, str]:
        """
        Collects the name of the fixed resource `section`_ (used for the 'collectedSections' parameter in
        :meth:`buildResources`)
        
        Parameters
        ----------
        oldResourceName: :class:`str`
            The old name of the resource `section`_
        
        newResourceName: :class:`str`
            The fixed name for the resource `section`_ (created by :meth:`getFixResourceName`)
        
        Returns
        -------
        Tuple[:class:`str`, :class:`str`]
            A tuple where the first value is the old resource name and the second is the new resource name
        """
    def getFixFile(self: BaseResEdit, file: str, modType: typing.Any = None, modName: str = '', graphId: str = '') -> str:
        """
        Retrieves the file path to the fixed resource
        
        Parameters
        ----------
        file: :class:`str`
            The file path to the original resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
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
    def getFixResourceName(self: BaseResEdit, resource: str, modType: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Retrieves the name of the fixed resource `section`_
        
        Parameters
        ----------
        resource: :class:`str`
            The name of the original resource `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        Optional[:class:`str`]
            The `section`_ name of the fixed resource. ``None`` indicates there was no name change between
            the original resource and the fixed resource
        """
    def getResGraph(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', rename: bool = True, copySections: bool = False) -> typing.Any:
        """
        Retrieves the particular :class:`IniSectionGraph` for the resource
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource -- the keys are the old names of the
            `sections`_ and the values are the fixed names
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
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
    def renameUncollectedSection(self: BaseResEdit, sectionName: str, modType: typing.Any = None, modName: str = '') -> str:
        """
        The name an uncollected `section`_ gets renamed to -- :meth:`getFixResourceName`, or the
        `section`_'s own name when that reports no change
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """
    @property
    def graphReplaceMode(self) -> typing.Any:
        """
        :class:`IniGraphReplaceMode`: What to do when the corresponding :class:`IniSectionGraph` to
        construct already exists
        """
    @graphReplaceMode.setter
    def graphReplaceMode(self, arg1: typing.Any) -> None:
        ...
    @property
    def resModObj(self) -> typing.Any:
        """
        Tuple[:class:`int`, :class:`str`, :class:`str`]: The mod object to hold the newly created
        :class:`IniSectionGraph` for the resource
        """
    @resModObj.setter
    def resModObj(self, arg1: typing.Any) -> None:
        ...
    @property
    def resType(self) -> str:
        """
        :class:`str`: The name of the type of resource
        """
    @resType.setter
    def resType(self, arg0: str) -> None:
        ...
class ResGroupCollect(BaseIniGraphGroupEdit):
    """
    
    This class inherits from :class:`BaseIniGraphGroupEdit`
    
    Creates the :class:`IniSectionGraph` for a particular group of resources
    
    Where :class:`ResRegCollect` handles one resource at a time, this handles several that belong
    together and works out which combinations of them can actually co-occur -- two resources belong in
    the same group only if the conditional branches they live under are simultaneously satisfiable
    
    Parameters
    ----------
    resGroupTypes: List[:class:`str`]
        The unique names for the type of resource groups
    
    srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]]
        The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`
    
        * The outer keys are the mod object for a particular type of resource in a resource group
        * The inner keys are the location of which :class:`IniSectionGraph` to search for
        * The values are the source registers
    
    resEdits: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, :class:`BaseResEdit`]]
        Describes how each resource in a resource group should be built :raw-html:`<br />` :raw-html:`<br />`
    
        * The outer keys are the mod object for a particular type of resource in a resource group
        * The inner keys are the names for the type of resource groups
        * The values are the edits for the type of resource
    
    groupedResBuilders: Dict[:class:`str`, :class:`IniGroupedResBuilder`]
        The builders used to construct a type of grouped resource
    
    partPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterQueryData`], :class:`Ranges`]]]]
        The predicates for which particular order indices to process for some :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    resPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterQueryData`], :class:`bool`]]]]
        The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    remaps: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]]
        Whether to remap the graphs searched from :attr:`srcRegs`. The values follow the same format as
        :attr:`GraphGroupRemap.remap` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]]
        Whether to track the `KVPs`_ in the .ini file when searching for particular resources :raw-html:`<br />` :raw-html:`<br />`
    
        If this parameter is a boolean, this flag will be globally used for all graphs :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
    
    keysToTrack: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]]
        Specific keys to track in the .ini file when searching particular resources :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    resGroupTypesSameTopology: :class:`bool`
        A flag used to enable an optimization to reduce the number of `satisfiable (SAT) problems`_
        needed to be computed when there are multiple types of resource groups. The flag assumes that
        each resource type for all types of resource groups have the same topology :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
    
    id: Optional[:class:`int`]
        The unique id for this object. If this value is ``None``, then an id is autogenerated :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, resGroupTypes: typing.Any, srcRegs: typing.Any, resEdits: typing.Any, groupedResBuilders: typing.Any, partPredicates: typing.Any = None, resPredicates: typing.Any = None, remaps: typing.Any = None, trackKeys: typing.Any = False, keysToTrack: typing.Any = None, resGroupTypesSameTopology: bool = False, id: typing.Any = None) -> None:
        ...
    def clear(self) -> None:
        """
        Clears :attr:`resCalls` and every resource edit's own saved state
        """
    def edit(self, graphGroups: list, modType: typing.Any, modName: str = '') -> list:
        """
        Collects and groups the references to the resources
        
        .. note::
            With no .ini file there is nothing to build the resources *for*, so this collects and groups but
            builds nothing -- exactly as the pure-Python original did
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after editing
        """
    def editFromIni(self, graphGroups: list, ini: typing.Any, modType: typing.Any, modName: str = '') -> list:
        """
        Collects, groups, replicates and connects the resources for 'ini'
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        ini: :class:`IniFile`
            The associated original .ini file
        
        modType: :class:`ModType`
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after editing
        """
    @property
    def groupedResBuilders(self) -> typing.Any:
        """
        Dict[:class:`str`, :class:`IniGroupedResBuilder`]: The builders used to construct a type of grouped
        resource
        """
    @groupedResBuilders.setter
    def groupedResBuilders(self, arg1: typing.Any) -> None:
        ...
    @property
    def id(self) -> int:
        """
        :class:`int`: The unique id for this object
        """
    @id.setter
    def id(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def keysToTrack(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]:
        Specific keys to track in the .ini file when searching particular resources
        """
    @keysToTrack.setter
    def keysToTrack(self, arg1: typing.Any) -> None:
        ...
    @property
    def partPredicates(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable]]:
        The predicates for which particular order indices to process for some :class:`IfContentPart`
        """
    @partPredicates.setter
    def partPredicates(self, arg1: typing.Any) -> None:
        ...
    @property
    def remaps(self) -> typing.Any:
        """
        Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Tuple]]]: Whether
        to remap the graphs searched from :attr:`srcRegs`
        """
    @remaps.setter
    def remaps(self, arg1: typing.Any) -> None:
        ...
    @property
    def resCalls(self) -> typing.Any:
        """
        Dict: The calls to each resource, keyed by resource type, then source graph, then `section`_ name,
        then part id, then order index. The values are ``(resource section name, query)`` tuples
        
        .. note::
            This is scratch state, rebuilt by every :meth:`edit` and cleared again afterwards. Reading it
            back gives a freshly built ``dict``, not a live view
        """
    @property
    def resEdits(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, :class:`BaseResEdit`]]:
        Describes how each resource in a resource group should be built
        """
    @resEdits.setter
    def resEdits(self, arg1: typing.Any) -> None:
        ...
    @property
    def resGroupTypes(self) -> typing.Any:
        """
        List[:class:`str`]: The unique names for the type of resource groups
        """
    @resGroupTypes.setter
    def resGroupTypes(self, arg1: typing.Any) -> None:
        ...
    @property
    def resGroupTypesSameTopology(self) -> bool:
        """
        :class:`bool`: Whether each resource type for all types of resource groups have the same topology
        """
    @resGroupTypesSameTopology.setter
    def resGroupTypesSameTopology(self, arg0: bool) -> None:
        ...
    @property
    def resPredicates(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable]]:
        The predicates to check whether some reference to the resource should be used
        """
    @resPredicates.setter
    def resPredicates(self, arg1: typing.Any) -> None:
        ...
    @property
    def srcRegs(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]]:
        The different registers that reference the particular resource
        """
    @srcRegs.setter
    def srcRegs(self, arg1: typing.Any) -> None:
        ...
    @property
    def trackKeys(self) -> typing.Any:
        """
        Union[:class:`bool`, Dict]: Whether to track the `KVPs`_ in the .ini file when searching for
        particular resources
        """
    @trackKeys.setter
    def trackKeys(self, arg1: typing.Any) -> None:
        ...
class ResIdentity(BaseResEdit):
    """
    
    This class inherits from :class:`BaseResEdit`
    
    Class to only build the :class:`IniSectionGraph` for the original collected resource
    
    Parameters
    ----------
    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource
    
    createResModel: :class:`bool`
        Whether to build the models for the resources :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    @staticmethod
    def fileAddGraphId(file: str, graphId: str = '') -> str:
        """
        Adds the unique id for the :class:`IniSectionGraph` of the resource to the name of the file
        
        Parameters
        ----------
        file: :class:`str`
            The path to the file to add the id to
        
        graphId: :class:`str`
            The id to add :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The file with the id added
        """
    @staticmethod
    def getFileId(modObj: typing.Any, sectionName: str, part: typing.Any, orderInd: typing.SupportsInt | typing.SupportsIndex, file: str) -> str:
        """
        Retrieves a unique id for a file within a single .ini file
        
        .. note::
            The returned value is not byte-identical to the one the pure-Python original produced -- it is
            an opaque, within-one-run dictionary key that is never persisted or written to a file
        
        Parameters
        ----------
        modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The mod object holding the newly created :class:`IniSectionGraph` for the resource
        
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
        :class:`str`
            The unique id for the file
        """
    def __init__(self, resModObj: typing.Any, createResModel: bool = True) -> None:
        ...
    def buildResModels(self: BaseResEdit, graph: typing.Any, ini: typing.Any = None, modType: typing.Any = None, resources: typing.Any = None, resourceFilter: typing.Any = None, modName: str = '', graphId: str = '', resModObj: typing.Any = None) -> None:
        """
        Builds and saves the resources, given the :class:`IniSectionGraph` for a resource
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph for the particular resource
        
        ini: Optional[:class:`IniFile`]
            The .ini file to build the resource for
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored, keyed by the unique id for the source file (created
            from :meth:`getFileId`). If ``None``, the models are appended to :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for -- takes the source file and its
            assigned id :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        graphId: :class:`str`
            The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resModObj: Optional[Tuple[:class:`int`, :class:`str`, :class:`str`]]
            The mod object used to create the unique id for the resources :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def buildResources(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', resourceFilter: typing.Any = None, resources: typing.Any = None, copySections: bool = False) -> list:
        """
        Builds the :class:`IniSectionGraph` and the corresponding models for the resources
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
            The associated original .ini file being fixed
        
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored. If ``None``, they are appended to
            :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        copySections: :class:`bool`
            Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The group of graphs that now includes the newly created graph for the resource
        
            .. tip::
                You can access the newly generated graph using :attr:`resModObj` on the group of graphs
        """
    def clear(self: BaseResEdit) -> None:
        """
        Clears any saved state information
        """
    def collectResourceName(self: BaseResEdit, oldResourceName: str, newResourceName: str) -> tuple[str, str]:
        """
        Collects the name of the fixed resource `section`_ (used for the 'collectedSections' parameter in
        :meth:`buildResources`)
        
        Parameters
        ----------
        oldResourceName: :class:`str`
            The old name of the resource `section`_
        
        newResourceName: :class:`str`
            The fixed name for the resource `section`_ (created by :meth:`getFixResourceName`)
        
        Returns
        -------
        Tuple[:class:`str`, :class:`str`]
            A tuple where the first value is the old resource name and the second is the new resource name
        """
    def getFixFile(self: BaseResEdit, file: str, modType: typing.Any = None, modName: str = '', graphId: str = '') -> str:
        """
        Retrieves the file path to the fixed resource
        
        Parameters
        ----------
        file: :class:`str`
            The file path to the original resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
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
    def getFixResourceName(self: BaseResEdit, resource: str, modType: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Retrieves the name of the fixed resource `section`_
        
        Parameters
        ----------
        resource: :class:`str`
            The name of the original resource `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        Optional[:class:`str`]
            The `section`_ name of the fixed resource. ``None`` indicates there was no name change between
            the original resource and the fixed resource
        """
    def getResGraph(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', rename: bool = True, copySections: bool = False) -> typing.Any:
        """
        Retrieves the particular :class:`IniSectionGraph` for the resource
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource -- the keys are the old names of the
            `sections`_ and the values are the fixed names
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
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
    def renameUncollectedSection(self: BaseResEdit, sectionName: str, modType: typing.Any = None, modName: str = '') -> str:
        """
        The name an uncollected `section`_ gets renamed to -- :meth:`getFixResourceName`, or the
        `section`_'s own name when that reports no change
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """
    @property
    def createResModel(self) -> bool:
        """
        :class:`bool`: Whether to build the models for the resources
        """
    @createResModel.setter
    def createResModel(self, arg0: bool) -> None:
        ...
    @property
    def graphReplaceMode(self) -> typing.Any:
        """
        :class:`IniGraphReplaceMode`: What to do when the corresponding :class:`IniSectionGraph` to
        construct already exists
        """
    @graphReplaceMode.setter
    def graphReplaceMode(self, arg1: typing.Any) -> None:
        ...
    @property
    def resModObj(self) -> typing.Any:
        """
        Tuple[:class:`int`, :class:`str`, :class:`str`]: The mod object to hold the newly created
        :class:`IniSectionGraph` for the resource
        """
    @resModObj.setter
    def resModObj(self, arg1: typing.Any) -> None:
        ...
    @property
    def resType(self) -> str:
        """
        :class:`str`: The name of the type of resource
        """
    @resType.setter
    def resType(self, arg0: str) -> None:
        ...
class ResRegCollect(BaseIniGraphGroupEdit):
    """
    
    This class inherits from :class:`BaseIniGraphGroupEdit`
    
    Creates the :class:`IniSectionGraph` for a particular resource
    
    Parameters
    ----------
    srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]
        The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`
    
        The keys in the dictionary are the location of which :class:`IniSectionGraph` to search for,
        which contains:
    
        #. The index for the .ini file
        #. The name of the component
        #. The name of the object
    
    resEdits: Dict[:class:`str`, :class:`BaseResEdit`]
        Describes how a resource should be built :raw-html:`<br />` :raw-html:`<br />`
    
        The keys are the names for the subtype of the resource and the values are the edit for each type
        of resource
    
    partPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`Ranges`]]]
        The predicates for which particular order indices to process for some :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    resPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]]
        The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`
    
        Each predicate takes in:
    
        #. The register name that holds the reference
        #. The name of the resource reference
        #. The data that contains info on the part and its `section`_
    
        :raw-html:`<br />`
    
        **Default**: ``None``
    
    remaps: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]]
        Whether to remap the graphs searched from :attr:`srcRegs`. The values follow the same format as
        :attr:`GraphGroupRemap.remap` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]
        Whether to track the `KVPs`_ in the .ini file when searching for particular resources :raw-html:`<br />` :raw-html:`<br />`
    
        If this parameter is a boolean, this flag will be globally used for all graphs :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``False``
    
    keysToTrack: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
        Specific keys to track in the .ini file when searching particular resources. A ``None`` value
        tracks every key encountered :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, srcRegs: typing.Any, resEdits: typing.Any, partPredicates: typing.Any = None, resPredicates: typing.Any = None, remaps: typing.Any = None, trackKeys: typing.Any = False, keysToTrack: typing.Any = None) -> None:
        ...
    def clear(self) -> None:
        """
        Clears :attr:`resCalls` and every resource edit's own saved state
        """
    def edit(self, graphGroups: list, modType: typing.Any, modName: str = '') -> list:
        """
        Collects and remaps the references to the resource
        
        .. note::
            With no .ini file there is nothing to build the resources *for*, so this collects and remaps but
            builds nothing -- exactly as the pure-Python original did
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after editing
        """
    def editFromIni(self, graphGroups: list, ini: typing.Any, modType: typing.Any, modName: str = '') -> list:
        """
        Collects, remaps and builds the resource for 'ini'
        
        Parameters
        ----------
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        ini: :class:`IniFile`
            The associated original .ini file
        
        modType: :class:`ModType`
            The type of mod to fix
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The same list that was passed in, after editing
        """
    @property
    def keysToTrack(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]: Specific keys to
        track in the .ini file when searching particular resources
        """
    @keysToTrack.setter
    def keysToTrack(self, arg1: typing.Any) -> None:
        ...
    @property
    def partPredicates(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`Ranges`]]:
        The predicates for which particular order indices to process for some :class:`IfContentPart`
        """
    @partPredicates.setter
    def partPredicates(self, arg1: typing.Any) -> None:
        ...
    @property
    def remaps(self) -> typing.Any:
        """
        Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Tuple]]]: Whether
        to remap the graphs searched from :attr:`srcRegs`
        """
    @remaps.setter
    def remaps(self, arg1: typing.Any) -> None:
        ...
    @property
    def resCalls(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Dict[:class:`int`, Dict[:class:`int`, :class:`str`]]]]:
        The calls to the resource
        
        * The outer keys are the id of the graph the call was found in
        * The second outer keys are the names of the `sections`_
        * The third outer keys are the id of the part within the `section`_
        * The inner keys are the order index the resource call is found at in the part
        * The values are the names of the resource `sections`_
        
        .. note::
            This is scratch state, rebuilt by every :meth:`edit` and cleared again by :meth:`editFromIni`.
            Reading it back gives a freshly built ``dict``, not a live view
        """
    @property
    def resEdits(self) -> typing.Any:
        """
        Dict[:class:`str`, :class:`BaseResEdit`]: Describes how a resource should be built
        """
    @resEdits.setter
    def resEdits(self, arg1: typing.Any) -> None:
        ...
    @property
    def resPredicates(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]:
        The predicates to check whether some reference to the resource should be used
        """
    @resPredicates.setter
    def resPredicates(self, arg1: typing.Any) -> None:
        ...
    @property
    def srcRegs(self) -> typing.Any:
        """
        Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]: The different registers that
        reference the particular resource
        """
    @srcRegs.setter
    def srcRegs(self, arg1: typing.Any) -> None:
        ...
    @property
    def trackKeys(self) -> typing.Any:
        """
        Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]: Whether
        to track the `KVPs`_ in the .ini file when searching for particular resources
        """
    @trackKeys.setter
    def trackKeys(self, arg1: typing.Any) -> None:
        ...
class ResReplace(BaseResEdit):
    """
    
    This class inherits from :class:`BaseResEdit`
    
    Class that creates the necessary parts for a fixed resource by building upon the existing
    :class:`IniSectionGraph` of the original resource
    
    Parameters
    ----------
    resType: :class:`str`
        The name of the type of resource
    
    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource
    
    graphReplaceMode: :class:`IniGraphReplaceMode`
        What to do when the corresponding :class:`IniSectionGraph` to construct already exists :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``IniGraphReplaceMode.Ignore``
        
    """
    @staticmethod
    def fileAddGraphId(file: str, graphId: str = '') -> str:
        """
        Adds the unique id for the :class:`IniSectionGraph` of the resource to the name of the file
        
        Parameters
        ----------
        file: :class:`str`
            The path to the file to add the id to
        
        graphId: :class:`str`
            The id to add :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The file with the id added
        """
    @staticmethod
    def getFileId(modObj: typing.Any, sectionName: str, part: typing.Any, orderInd: typing.SupportsInt | typing.SupportsIndex, file: str) -> str:
        """
        Retrieves a unique id for a file within a single .ini file
        
        .. note::
            The returned value is not byte-identical to the one the pure-Python original produced -- it is
            an opaque, within-one-run dictionary key that is never persisted or written to a file
        
        Parameters
        ----------
        modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The mod object holding the newly created :class:`IniSectionGraph` for the resource
        
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
        :class:`str`
            The unique id for the file
        """
    def __init__(self, resType: str, resModObj: typing.Any, graphReplaceMode: typing.Any = None) -> None:
        ...
    def buildResModel(self, resType: str, ini: typing.Any, srcPath: str, fixedPath: str, modType: typing.Any = None, *args, modName: str = '', **kwargs) -> typing.Any:
        """
        Builds the model for the resource
        
        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource
        
        ini: :class:`IniFile`
            The .ini file to build the resource for
        
        srcPath: :class:`str`
            The file path to the original resource
        
        fixedPath: :class:`str`
            The file path to the fixed resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`IniFixResource`
            The built resource
        """
    def buildResModels(self: BaseResEdit, graph: typing.Any, ini: typing.Any = None, modType: typing.Any = None, resources: typing.Any = None, resourceFilter: typing.Any = None, modName: str = '', graphId: str = '', resModObj: typing.Any = None) -> None:
        """
        Builds and saves the resources, given the :class:`IniSectionGraph` for a resource
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph for the particular resource
        
        ini: Optional[:class:`IniFile`]
            The .ini file to build the resource for
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored, keyed by the unique id for the source file (created
            from :meth:`getFileId`). If ``None``, the models are appended to :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for -- takes the source file and its
            assigned id :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        graphId: :class:`str`
            The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resModObj: Optional[Tuple[:class:`int`, :class:`str`, :class:`str`]]
            The mod object used to create the unique id for the resources :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def buildResources(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', resourceFilter: typing.Any = None, resources: typing.Any = None, copySections: bool = False) -> list:
        """
        Builds the :class:`IniSectionGraph` and the corresponding models for the resources
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
            The associated original .ini file being fixed
        
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored. If ``None``, they are appended to
            :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        copySections: :class:`bool`
            Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The group of graphs that now includes the newly created graph for the resource
        
            .. tip::
                You can access the newly generated graph using :attr:`resModObj` on the group of graphs
        """
    def clear(self: BaseResEdit) -> None:
        """
        Clears any saved state information
        """
    def collectResourceName(self: BaseResEdit, oldResourceName: str, newResourceName: str) -> tuple[str, str]:
        """
        Collects the name of the fixed resource `section`_ (used for the 'collectedSections' parameter in
        :meth:`buildResources`)
        
        Parameters
        ----------
        oldResourceName: :class:`str`
            The old name of the resource `section`_
        
        newResourceName: :class:`str`
            The fixed name for the resource `section`_ (created by :meth:`getFixResourceName`)
        
        Returns
        -------
        Tuple[:class:`str`, :class:`str`]
            A tuple where the first value is the old resource name and the second is the new resource name
        """
    def getFixFile(self: BaseResEdit, file: str, modType: typing.Any = None, modName: str = '', graphId: str = '') -> str:
        """
        Retrieves the file path to the fixed resource
        
        Parameters
        ----------
        file: :class:`str`
            The file path to the original resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
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
    def getFixResourceName(self: BaseResEdit, resource: str, modType: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Retrieves the name of the fixed resource `section`_
        
        Parameters
        ----------
        resource: :class:`str`
            The name of the original resource `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        Optional[:class:`str`]
            The `section`_ name of the fixed resource. ``None`` indicates there was no name change between
            the original resource and the fixed resource
        """
    def getResGraph(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', rename: bool = True, copySections: bool = False) -> typing.Any:
        """
        Retrieves the particular :class:`IniSectionGraph` for the resource
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource -- the keys are the old names of the
            `sections`_ and the values are the fixed names
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
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
    def renameUncollectedSection(self: BaseResEdit, sectionName: str, modType: typing.Any = None, modName: str = '') -> str:
        """
        The name an uncollected `section`_ gets renamed to -- :meth:`getFixResourceName`, or the
        `section`_'s own name when that reports no change
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """
    @property
    def graphReplaceMode(self) -> typing.Any:
        """
        :class:`IniGraphReplaceMode`: What to do when the corresponding :class:`IniSectionGraph` to
        construct already exists
        """
    @graphReplaceMode.setter
    def graphReplaceMode(self, arg1: typing.Any) -> None:
        ...
    @property
    def resModObj(self) -> typing.Any:
        """
        Tuple[:class:`int`, :class:`str`, :class:`str`]: The mod object to hold the newly created
        :class:`IniSectionGraph` for the resource
        """
    @resModObj.setter
    def resModObj(self, arg1: typing.Any) -> None:
        ...
    @property
    def resType(self) -> str:
        """
        :class:`str`: The name of the type of resource
        """
    @resType.setter
    def resType(self, arg0: str) -> None:
        ...
class SectionIterData:
    """
    
    A class that contains the needed data for each iteration after calling :meth:`IniSectionGraph.iterSectsByContentPart`
    
    Parameters
    ----------
    sectionName: :class:`str`
        The name of the `section`_
    
    section: :class:`IfTemplate`
        The corresponding `section`_ the part resides in
    
    part: :class:`IfContentPart`
        The corresponding part
    
    state: :class:`int`
        The current state of the `section`_
    
    colouring: Optional[:class:`IfContentPartColouring`]
        The current `KVP`_ states of the :class:`IfContentPart`
        
    """
    @property
    def colouring(self) -> typing.Any:
        """
        Optional[:class:`IfContentPartColouring`]: The current `KVP`_ states of the :class:`IfContentPart`
        """
    @property
    def part(self) -> IfContentPart:
        """
        :class:`IfContentPart`: The corresponding part
        """
    @property
    def section(self) -> IfTemplate:
        """
        :class:`IfTemplate`: The corresponding `section`_ the part resides in
        """
    @property
    def sectionName(self) -> str:
        """
        :class:`str`: The name of the `section`_
        """
    @property
    def state(self) -> int:
        """
        :class:`int`: The current state of the `section`_
        """
class SectionIterDataIterator:
    def __iter__(self) -> SectionIterDataIterator:
        ...
    def __next__(self) -> typing.Any:
        ...
class SectionIterQueryData:
    """
    
    A class that contains the needed data for each iteration after calling :meth:`IniSectionGraph.iterByQuery`
    
    Parameters
    ----------
    part: :class:`IfContentPart`
        The part retrieved
    
    query: :class:`Z3Predicate`
        The corresponding logical query that the part resides in
    
    sectionName: :class:`str`
        The name of the `section`_ the part resides in
    
    section: :class:`IfTemplate`
        The corresponding `section`_ the part resides in
    
    rootSectionName: :class:`str`
        The name of the root `section`_ the part resides in
    
    rootSection: :class:`IfTemplate`
        The corresponding root `section`_ the part resides in
    
    state: :class:`int`
        The current state the `section`_ is in
    
    colouring: Optional[:class:`IfContentPartColouring`]
        The current `KVP`_ states of the :class:`IfContentPart`
        
    """
    @property
    def colouring(self) -> typing.Any:
        """
        Optional[:class:`IfContentPartColouring`]: The current `KVP`_ states of the :class:`IfContentPart`
        """
    @property
    def part(self) -> IfContentPart:
        """
        :class:`IfContentPart`: The part retrieved
        """
    @property
    def query(self) -> Z3Predicate:
        """
        :class:`Z3Predicate`: The corresponding logical query that the part resides in
        """
    @property
    def rootSection(self) -> IfTemplate:
        """
        :class:`IfTemplate`: The corresponding root `section`_ the part resides in
        """
    @property
    def rootSectionName(self) -> str:
        """
        :class:`str`: The name of the root `section`_ the part resides in
        """
    @property
    def section(self) -> IfTemplate:
        """
        :class:`IfTemplate`: The corresponding `section`_ the part resides in
        """
    @property
    def sectionName(self) -> str:
        """
        :class:`str`: The name of the `section`_ the part resides in
        """
    @property
    def state(self) -> int:
        """
        :class:`int`: The current state the `section`_ is in
        """
class SectionIterQueryDataIterator:
    def __iter__(self) -> SectionIterQueryDataIterator:
        ...
    def __next__(self) -> typing.Any:
        ...
class SympyParser:
    """
    
    The context-free parser used for a subset of the string representation of a `sympy logic query`_
    
    eg.
    
    .. code-block:: ini
        :linenos:
    
        ~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))
    
    Parameters
    -----------
    startToken: :class:`str`
        The name of the starting token for an input string
    
        **Default**: ``STARTTOKEN``
    
    endToken: :class:`str`
        The name of the ending token for an input string
    
        **Default**: ``ENDTOKEN``
    
    nullToken: :class:`str`
        The name for the empty token
    
        **Default**: ``EPSILON``
    
    setup: :class:`bool`
        Whether to initialize all the setup for the parser automatically by calling :meth:`setup`
    
        **Default**: ``True``
        
    """
    def __init__(self, startToken: str = 'STARTTOKEN', endToken: str = 'ENDTOKEN', nullToken: str = 'EPSILON', setup: bool = True) -> None:
        ...
    def clear(self) -> None:
        """
        Clears all the setup from the parser
        """
    def getFirst(self, symbols: collections.abc.Sequence[str], nullable: collections.abc.Mapping[str, bool], first: collections.abc.Mapping[str, collections.abc.Set[str]]) -> set[str]:
        """
        Retrieves the first terminal symbols to appear given a list of symbols
        
        Parameters
        ----------
        symbols: List[:class:`str`]
            The symbols to read
        
        nullable: Dict[:class:`str`, :class:`bool`]
            The `Nullable Set`_
        
        first: Dict[:class:`str`, Set[:class:`str`]]
            The `First Set`_ for only each single non-terminal symbol
        
        Returns
        -------
        Set[:class:`str`]
            The first terminal symbols to appear given 'symbols'
        """
    def getFirstSet(self, updateNullable: bool = True) -> dict[str, set[str]]:
        """
        Computes the `First Set`_ for only each single non-terminal symbol
        
        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The first terminal symbols to appear for a non-terminal symbol
        """
    def getFollowSet(self, updateNullable: bool = True, updateFirst: bool = True) -> dict[str, set[str]]:
        """
        Computes the `Follow Set`_
        
        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_
        
            **Default**: ``True``
        
        updateFirst: :class:`bool`
            Whether to update the `First Set`_
        
            **Default**: ``True``
        
        Returns
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The `Follow Set`_
        """
    def getNonTermSymbols(self) -> set[str]:
        """
        Retrieves the set of non-terminal symbols of the `CFG`_
        
        Returns
        -------
        Set[:class:`str`]
            The set of non-terminal symbols
        """
    def getNullableSet(self) -> dict[str, bool]:
        """
        Computes the `Nullable Set`_
        
        Returns
        -------
        Dict[:class:`str`, :class:`bool`]
            Whether each non-terminal symbol is nullable
        """
    def parse(self, tokens: collections.abc.Sequence[Token], ctx: ParseContext = None) -> ParseTree:
        """
        Parses an input text
        
        Parameters
        ----------
        tokens: List[:class:`Token`]
            The tokenized tokens of the input text :raw-html:`<br />` :raw-html:`<br />`
        
            Usually obtained by running some sort of tokenizer, such as :class:`BaseTokenizer`
        
        ctx: Optional[:class:`ParseContext`]
            The context for parsing :raw-html:`<br />` :raw-html:`<br />`
        
            If this argument is ``None``, a context is constructed from the concatenation of every
            token's value
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`SyntaxErr`
            If the parse tree cannot be constructed
        
        Returns
        -------
        :class:`ParseTree`
            The constructed parse tree
        """
    def setup(self) -> None:
        """
        Initializes any necessary setup for the parser
        """
    @property
    def endToken(self) -> str:
        """
        :class:`str`: The name of the ending token for an input string
        """
    @endToken.setter
    def endToken(self, arg0: str) -> None:
        ...
    @property
    def first(self) -> dict[str, set[str]]:
        """
        Dict[:class:`str`, Set[:class:`str`]]: The `First Set`_ for only each single non-terminal symbol
        """
    @first.setter
    def first(self, arg0: collections.abc.Mapping[str, collections.abc.Set[str]]) -> None:
        ...
    @property
    def follow(self) -> dict[str, set[str]]:
        """
        Dict[:class:`str`, Set[:class:`str`]]: The `Follow Set`_
        """
    @follow.setter
    def follow(self, arg0: collections.abc.Mapping[str, collections.abc.Set[str]]) -> None:
        ...
    @property
    def nonTermSymbols(self) -> set[str]:
        """
        Set[:class:`str`]: The set of non-terminal symbols of the `CFG`_, as of the last time 'productions' was set
        """
    @property
    def nullToken(self) -> str:
        """
        :class:`str`: The name for the empty token
        """
    @nullToken.setter
    def nullToken(self, arg0: str) -> None:
        ...
    @property
    def nullable(self) -> dict[str, bool]:
        """
        Dict[:class:`str`, :class:`bool`]: The `Nullable Set`_
        
        The keys are the non-terminal symbols and the values are whether each symbol is nullable
        """
    @nullable.setter
    def nullable(self, arg0: collections.abc.Mapping[str, bool]) -> None:
        ...
    @property
    def productions(self) -> dict:
        """
        Dict[Hashable, Tuple[:class:`str`, List[:class:`str`]]]: The production rules of the `CFG`_, keyed by the id of each production rule
        """
    @property
    def startSymbol(self) -> str:
        """
        :class:`str`: The starting non-terminal symbol
        
        :getter: Retrieves the starting non-terminal symbol
        :setter: Sets the new starting non-terminal symbol
        """
    @startSymbol.setter
    def startSymbol(self, arg1: str) -> None:
        ...
    @property
    def startToken(self) -> str:
        """
        :class:`str`: The name of the starting token for an input string
        """
    @startToken.setter
    def startToken(self, arg0: str) -> None:
        ...
class SympyTokenizer(FilteredTokenizer):
    """
    
    This class inherits from :class:`FilteredTokenizer`
    
    The tokenizer used for a subset of the string representation of a `sympy logic query`_
    
    eg.
    
    .. code-block::
        :linenos:
    
        ~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))
    
    Parameters
    ----------
    setup: :class:`bool`
        Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``True``
        
    """
    def __init__(self, setup: bool = True) -> None:
        ...
class TexCreate(BaseResEdit):
    """
    
    This class inherits from :class:`ResCreate`
    
    Class that builds the necessary parts to create some new texture file
    
    Parameters
    ----------
    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`
    
        The tuple contains:
    
        #. The index for the .ini file
        #. The name of the component
        #. The name of the object
    
    texName: :class:`str`
        The name for the type of texture
    
    texCreator: :class:`TexCreator`
        The editor for the texture file
    
    resType: :class:`str`
        The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``resourceRemapTexAdd``
    
    fixFunc: Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]
        The custom function for creating the texture :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    @staticmethod
    def fileAddGraphId(file: str, graphId: str = '') -> str:
        """
        Adds the unique id for the :class:`IniSectionGraph` of the resource to the name of the file
        
        Parameters
        ----------
        file: :class:`str`
            The path to the file to add the id to
        
        graphId: :class:`str`
            The id to add :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The file with the id added
        """
    @staticmethod
    def getFileId(modObj: typing.Any, sectionName: str, part: typing.Any, orderInd: typing.SupportsInt | typing.SupportsIndex, file: str) -> str:
        """
        Retrieves a unique id for a file within a single .ini file
        
        .. note::
            The returned value is not byte-identical to the one the pure-Python original produced -- it is
            an opaque, within-one-run dictionary key that is never persisted or written to a file
        
        Parameters
        ----------
        modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
            The mod object holding the newly created :class:`IniSectionGraph` for the resource
        
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
        :class:`str`
            The unique id for the file
        """
    def __init__(self, resModObj: typing.Any, texName: str, texCreator: typing.Any, resType: str = 'resourceRemapTexAdd', fixFunc: typing.Any = None) -> None:
        ...
    def buildResModel(self, resType: str, ini: typing.Any, srcPath: str, modType: typing.Any = None, *args, modName: str = '', **kwargs) -> typing.Any:
        """
        Builds the model for the resource
        
        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource
        
        ini: :class:`IniFile`
            The .ini file to build the resource for
        
        srcPath: :class:`str`
            The file path to the original resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`RemapTexAddResource`
            The built resource
        """
    def buildResModels(self: BaseResEdit, graph: typing.Any, ini: typing.Any = None, modType: typing.Any = None, resources: typing.Any = None, resourceFilter: typing.Any = None, modName: str = '', graphId: str = '', resModObj: typing.Any = None) -> None:
        """
        Builds and saves the resources, given the :class:`IniSectionGraph` for a resource
        
        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph for the particular resource
        
        ini: Optional[:class:`IniFile`]
            The .ini file to build the resource for
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored, keyed by the unique id for the source file (created
            from :meth:`getFileId`). If ``None``, the models are appended to :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for -- takes the source file and its
            assigned id :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        graphId: :class:`str`
            The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resModObj: Optional[Tuple[:class:`int`, :class:`str`, :class:`str`]]
            The mod object used to create the unique id for the resources :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        """
    def buildResources(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', resourceFilter: typing.Any = None, resources: typing.Any = None, copySections: bool = False) -> list:
        """
        Builds the :class:`IniSectionGraph` and the corresponding models for the resources
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
            The associated original .ini file being fixed
        
        graphGroups: List[:class:`IniGraphGroup`]
            The group of graphs to edit for each .ini file
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
            A predicate deciding which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
            Where the built resource models are stored. If ``None``, they are appended to
            :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``
        
        copySections: :class:`bool`
            Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``False``
        
        Returns
        -------
        List[:class:`IniGraphGroup`]
            The group of graphs that now includes the newly created graph for the resource
        
            .. tip::
                You can access the newly generated graph using :attr:`resModObj` on the group of graphs
        """
    def buildSection(self, sectionName: str, modType: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Builds a `section`_ for the resource -- a single ``filename =`` `KVP`_ pointing at the ``.dds`` file
        this edit creates
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name for the `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod to fix from
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`IfTemplate`
            The generated `section`_
        """
    def clear(self: BaseResEdit) -> None:
        """
        Clears any saved state information
        """
    def collectResourceName(self: BaseResEdit, oldResourceName: str, newResourceName: str) -> tuple[str, str]:
        """
        Collects the name of the fixed resource `section`_ (used for the 'collectedSections' parameter in
        :meth:`buildResources`)
        
        Parameters
        ----------
        oldResourceName: :class:`str`
            The old name of the resource `section`_
        
        newResourceName: :class:`str`
            The fixed name for the resource `section`_ (created by :meth:`getFixResourceName`)
        
        Returns
        -------
        Tuple[:class:`str`, :class:`str`]
            A tuple where the first value is the old resource name and the second is the new resource name
        """
    def getFixFile(self: BaseResEdit, file: str, modType: typing.Any = None, modName: str = '', graphId: str = '') -> str:
        """
        Retrieves the file path to the fixed resource
        
        Parameters
        ----------
        file: :class:`str`
            The file path to the original resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
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
    def getFixResourceName(self: BaseResEdit, resource: str, modType: typing.Any = None, modName: str = '') -> typing.Any:
        """
        Retrieves the name of the fixed resource `section`_
        
        Parameters
        ----------
        resource: :class:`str`
            The name of the original resource `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        Optional[:class:`str`]
            The `section`_ name of the fixed resource. ``None`` indicates there was no name change between
            the original resource and the fixed resource
        """
    def getResGraph(self: BaseResEdit, collectedSections: typing.Any, modType: typing.Any, ini: typing.Any, graphGroups: list, modName: str = '', rename: bool = True, copySections: bool = False) -> typing.Any:
        """
        Retrieves the particular :class:`IniSectionGraph` for the resource
        
        Parameters
        ----------
        collectedSections: Dict[:class:`str`, :class:`str`]
            The target `sections`_ that reference the resource -- the keys are the old names of the
            `sections`_ and the values are the fixed names
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        ini: Optional[:class:`IniFile`]
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
    def renameUncollectedSection(self: BaseResEdit, sectionName: str, modType: typing.Any = None, modName: str = '') -> str:
        """
        The name an uncollected `section`_ gets renamed to -- :meth:`getFixResourceName`, or the
        `section`_'s own name when that reports no change
        
        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed
        
        modName: :class:`str`
            The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`str`
            The new name for the `section`_
        """
    @property
    def fixFunc(self) -> typing.Any:
        """
        Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]: The custom function for creating
        the texture
        """
    @fixFunc.setter
    def fixFunc(self, arg1: typing.Any) -> None:
        ...
    @property
    def graphReplaceMode(self) -> typing.Any:
        """
        :class:`IniGraphReplaceMode`: What to do when the corresponding :class:`IniSectionGraph` to
        construct already exists
        """
    @graphReplaceMode.setter
    def graphReplaceMode(self, arg1: typing.Any) -> None:
        ...
    @property
    def resModObj(self) -> typing.Any:
        """
        Tuple[:class:`int`, :class:`str`, :class:`str`]: The mod object to hold the newly created
        :class:`IniSectionGraph` for the resource
        """
    @resModObj.setter
    def resModObj(self, arg1: typing.Any) -> None:
        ...
    @property
    def resType(self) -> str:
        """
        :class:`str`: The name of the type of resource
        """
    @resType.setter
    def resType(self, arg0: str) -> None:
        ...
    @property
    def texCreator(self) -> typing.Any:
        """
        :class:`TexCreator`: The editor for the texture file
        """
    @texCreator.setter
    def texCreator(self, arg1: typing.Any) -> None:
        ...
    @property
    def texName(self) -> str:
        """
        :class:`str`: The name for the type of texture
        """
    @texName.setter
    def texName(self, arg0: str) -> None:
        ...
class TexReplace(BaseResEdit):
    """
    
    This class inherits from :class:`ResReplace`
    
    Class that builds the necessary parts to edit an existing texture file
    
    The counterpart to :class:`TexCreate`, which *creates* a brand new one, and the texture analogue of
    :class:`RemapBlendReplace` -- an edit has an original resource to build its fixed name on, which is
    what separates a :class:`ResReplace` from a :class:`ResCreate`
    
    Parameters
    ----------
    resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
        The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`
    
        The tuple contains:
    
        #. The index for the .ini file
        #. The name of the component
        #. The name of the object
    
    texEditor: :class:`CppTexEditor`
        The texture editor used to edit the ``.dds`` file
    
    resType: :class:`str`
        The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``"resourceRemapTexEdit"``
    
    fixFunc: Optional[Callable[[:class:`RemapTexEditResource`], :class:`bool`]]
        A custom function for editing the texture file :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
    
    resSubType: Optional[:class:`str`]
        The name of the subtype of the resource :raw-html:`<br />` :raw-html:`<br />`
    
        **Default**: ``None``
        
    """
    def __init__(self, resModObj: typing.Any, texEditor: typing.Any, resType: str = 'resourceRemapTexEdit', fixFunc: typing.Any = None, resSubType: typing.Any = None) -> None:
        ...
    def buildResModel(self, resType: str, ini: typing.Any, srcPath: str, fixedPath: str, modType: typing.Any = None, *args, modName: str = '', **kwargs) -> typing.Any:
        """
        Builds the model for the resource
        
        Parameters
        ----------
        resType: :class:`str`
            The name for the type of resource
        
        ini: :class:`IniFile`
            The .ini file to build the resource for
        
        srcPath: :class:`str`
            The file path to the original resource
        
        fixedPath: :class:`str`
            The file path to the fixed resource
        
        modType: Optional[:class:`ModType`]
            The type of mod being fixed. Unused
        
        modName: :class:`str`
            The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``""``
        
        Returns
        -------
        :class:`RemapTexEditResource`
            The built resource
        """
    @property
    def fixFunc(self) -> typing.Any:
        """
        Optional[Callable[[:class:`RemapTexEditResource`], :class:`bool`]]: A custom function for editing the texture file
        """
    @fixFunc.setter
    def fixFunc(self, arg0: typing.Any) -> None:
        ...
    @property
    def texEditor(self) -> typing.Any:
        """
        :class:`CppTexEditor`: The texture editor used to edit the ``.dds`` file
        """
    @texEditor.setter
    def texEditor(self, arg0: typing.Any) -> None:
        ...
class Token:
    """
    
    A token when parsing some language
    
    Parameters
    ----------
    type: Optional[:class:`str`]
        The name for the type of token, if available
    
    val: :class:`str`
        The value of the token
    
    lineNo: :class:`int`
        The line number the token belongs to
    
    charNo: :class:`int`
        The character number the token belongs to within some line
        
    """
    def __init__(self, type: str | None, val: str, lineNo: typing.SupportsInt | typing.SupportsIndex, charNo: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def charNo(self) -> int:
        """
        :class:`int`: The character number the token belongs to within some line
        """
    @charNo.setter
    def charNo(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def lineNo(self) -> int:
        """
        :class:`int`: The line number the token belongs to
        """
    @lineNo.setter
    def lineNo(self, arg0: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @property
    def type(self) -> str | None:
        """
        Optional[:class:`str`]: The name for the type of token, if available
        """
    @type.setter
    def type(self, arg0: str | None) -> None:
        ...
    @property
    def val(self) -> str:
        """
        :class:`str`: The value of the token
        """
    @val.setter
    def val(self, arg0: str) -> None:
        ...
class VGRemap:
    """
    
    Class for handling the vertex group remaps for mods
        
    """
    def __init__(self, vgRemap: collections.abc.Mapping[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex] = {}) -> None:
        """
        Constructs a new vertex group remap
        
        Parameters
        ----------
        vgRemap: Dict[:class:`int`, :class:`int`]
            The vertex group remap from one type of mod to another. **Default**: ``{}``
        """
    @property
    def maxIndex(self) -> int | None:
        """
        Optional[:class:`int`]: The maximum index in the vertex group remap, or ``None`` if :attr:`remap` is empty
        """
    @property
    def remap(self) -> dict[int, int]:
        """
        Dict[:class:`int`, :class:`int`]: The vertex group remap
        """
    @remap.setter
    def remap(self, arg1: collections.abc.Mapping[typing.SupportsInt | typing.SupportsIndex, typing.SupportsInt | typing.SupportsIndex]) -> None:
        ...
class VGRemaps:
    """
    
    Class to handle the Vertex Group Remaps of a mod, pre-populated with this project's real remap data
    
    :raw-html:`<br />`
    
    .. note::
        Names of the available indices used for querying with the :meth:`get` method are:
    
        * fromVersion (version index)
        * fromChar
        * fromComp
        * toVersion (version index)
        * toChar
        * toComp
    
        A non-version column left unspecified is a **wildcard** (match anything there), and a version
        column left unspecified resolves to the latest available value among the rows still matching
        everything resolved before it
        
    """
    def __copy__(self) -> VGRemaps:
        ...
    def __deepcopy__(self, memo: dict) -> VGRemaps:
        ...
    def __init__(self) -> None:
        """
        Constructs a new, fully-populated vertex group remap table
        
        :raw-html:`<br />`
        
        .. note::
            Unlike the pure-Python original there is no 'repo' argument -- nothing in this project passed
            one, and :meth:`addRows` already covers extending the table. Note that
            :attr:`ModDataAssets.VGRemaps` hands out a **shared** instance, so mutating that one is visible
            to every :class:`ModType` that fell back to it; construct one directly for an independent table
        """
    def __len__(self) -> int:
        """
        The total number of rows currently in the table
        """
    def addRows(self, rows: typing.Any) -> None:
        """
        Adds new rows to the table, overwriting the value of any row whose full key already exists
        
        Parameters
        ----------
        rows: Union[List[Tuple[List[:class:`str`], Any]], dict]
            The rows to add -- either a flat list of ``(indexVals, remap)`` tuples, or a real nested dict
            exactly :attr:`totalIndices` levels deep
            (``{fromVersion: {fromChar: {fromComp: {toVersion: {toChar: {toComp: remap}}}}}}``)
            :raw-html:`<br />` :raw-html:`<br />`
        
            Each leaf may be either a :class:`VGRemap` or the plain ``{fromIndex: toIndex}`` dict one is
            built from
        
        Raises
        ------
        :class:`ValueError`
            If the nesting depth doesn't match :attr:`totalIndices`, a leaf is neither a :class:`VGRemap`
            nor a dict, or a row's version value fails to parse
        """
    def clone(self) -> VGRemaps:
        """
        Creates an independent copy of this table
        
        Returns
        -------
        :class:`VGRemaps`
            The copied table
        """
    def get(self, nonVersionVals: typing.Any = None, versionVals: typing.Any = None, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the corresponding vertex group remap
        
        Parameters
        ----------
        nonVersionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            The values of the index columns that do not reference a version -- a bare value (taken as
            ``fromChar``), a positional list, or a dict keyed by index name. Any column left unspecified
            matches anything there
        
            **Default**: ``None``
        
        versionVals: Optional[Union[Any, List[Optional[Any]], Dict[:class:`str`, Any]]]
            The versions to query at, same accepted shapes. A column left unspecified resolves to the
            latest available value for it :raw-html:`<br />` :raw-html:`<br />`
        
            The two version columns are resolved sequentially, in index order -- ``fromVersion``'s
            floor-match narrows the candidate rows before ``toVersion`` is resolved against them
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if no matching remap is found
        
            **Default**: ``True``
        
        default: Any
            If 'errorOnNotFound' is ``False``, the value to return when nothing is found
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`KeyError`
            If no matching remap is found and 'errorOnNotFound' is ``True``
        
        Returns
        -------
        :class:`VGRemap`
            The found remap, or 'default' if none is found and 'errorOnNotFound' is ``False``
        """
    @property
    def nonVersionColumnCount(self) -> int:
        """
        :class:`int`: The number of non-version columns
        """
    @property
    def totalIndices(self) -> int:
        """
        :class:`int`: The total number of index columns
        """
    @property
    def versionColumnCount(self) -> int:
        """
        :class:`int`: The number of version columns
        """
class VbFile(CppBufFile):
    """
    
    This class inherits from :class:`CppBufFile`
    
    Used for handling ``.vb`` (vertex buffer) files
    
    .. note::
        A GI character's ``.vb`` data does not live in one file -- it is split across a
        ``Position.buf``, a ``Blend.buf`` and a ``Texcoord.buf``, one line each per vertex. Use
        :meth:`CppBufFile.merge` to stitch such a set back together, which fills in both the bytes and
        the elements:
    
        .. code-block::
    
            vbFile = VbFile(b"", [])
            vbFile.merge([PositionFile(positionPath), BlendFile(blendPath), texcoordFile])
    
    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the ``.vb`` file
    
    elements: List[:class:`BufElementType`]
        The sequence of elements within a vertex line, in byte order :raw-html:`<br />` :raw-html:`<br />`
    
        Required rather than defaulted: unlike a :class:`BlendFile` or a :class:`PositionFile`, a
        ``.vb`` file has no single fixed layout -- how many texture coordinates it carries varies by mod
        
    """
    @staticmethod
    def parseDumpHeader(text: str) -> typing.Any:
        """
        Builds the elements of a vertex line out of a dumped *vb.txt* file's header
        
        .. note::
            The header names each element and gives its `DXGI format`_, which together are everything a
            ``.vb`` file needs -- so a dump can be read back without being told what its layout was
        
        Parameters
        ----------
        text: :class:`str`
            The text of the dumped *vb.txt* file
        
        Returns
        -------
        Optional[List[:class:`BufElementType`]]
            The elements the header declares, in byte order, or ``None`` when the text has no header to
            read them from (or when one of its formats could not be parsed)
        """
    @staticmethod
    def parseFormatName(formatName: str) -> list:
        """
        Builds the data types making up an element from the `DXGI format`_ name a dump's header gives for it
        
        .. note::
            The channels decide *how many* data types there are and how wide each one is, and the suffix
            decides what kind they are -- so ``R32G32B32_FLOAT`` is 3 four-byte floats, ``R8G8B8A8_UNORM``
            is 4 one-byte `unsigned normalized integers`_, and ``R32G32B32A32_SINT`` is 4 four-byte signed
            integers
        
        Parameters
        ----------
        formatName: :class:`str`
            The format name to parse, eg. ``"R32G32B32_FLOAT"``
        
        Returns
        -------
        List[:class:`BufDataType`]
            The data types making up the element. Empty if the format is not one this understands
        """
    def __init__(self, src: typing.Any, elements: typing.Any) -> None:
        ...
    def getDumpStr(self, prefix: str = 'vb0') -> str:
        """
        Retrieves the full text for converting this ``.vb`` file into a dumped *vb.txt* file
        
        .. note::
            Unlike :meth:`CppBufFile.getDumpStr`, this returns a *complete* dump -- it is
            :meth:`makeDumpHeader` followed by the data section the parent class produces
        
        Parameters
        ----------
        prefix: :class:`str`
            The buffer name each entry is prefixed with -- the vertex buffer slot a real dump was taken
            from :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``"vb0"``
        
        Returns
        -------
        :class:`str`
            The text for the dumped *vb.txt* file
        """
    def getVertexCount(self) -> int:
        """
        Retrieves the number of vertices making up the mod's mesh
        
        Returns
        -------
        :class:`int`
            The number of vertices
        """
    def makeDumpHeader(self) -> str:
        """
        Makes the header for a dumped *vb.txt* file
        
        .. note::
            An element's ``SemanticIndex`` is its occurrence among the elements sharing its
            :attr:`BufElementType.name` -- so the first ``TEXCOORD`` is index 0, the next is 1, and so on,
            which is how 3dmigoto tells several same-named elements apart
        
        Returns
        -------
        :class:`str`
            The header text, ending with the ``vertex-data:`` marker the data section follows
        """
    def readDumpStr(self, text: str) -> None:
        """
        Reads a dumped *vb.txt* file's text back into this ``.vb`` file's bytes -- the inverse of
        :meth:`getDumpStr`
        
        .. note::
            Unlike :meth:`CppBufFile.readDumpStr`, which encodes against whatever :attr:`elements` the file
            already has, this first rebuilds those elements from the dump's own header (see
            :meth:`parseDumpHeader`) when there is one -- so a dump can be read straight back without being
            told its layout. A header-less text falls through to the current elements
        
        Parameters
        ----------
        text: :class:`str`
            The text of the dumped *vb.txt* file
        
        Raises
        ------
        :class:`BadBufData`
            If the parsed bytes do not divide evenly into vertex lines
        """
class VertexCounts:
    """
    
    Class for managing the vertex counts of a mod, pre-populated with this project's real vertex
    count data
    
    :raw-html:`<br />`
    
    .. note::
        Names of the available indices used for querying with the :meth:`get` method are:
    
        * version (version index)
        * name
        * component
    
        ``component`` is ``""`` on every row the software currently ships, so a caller wanting a mod's
        overall count can simply leave it out -- an unspecified non-version column is filled in with
        ``""``, not treated as a wildcard (this table is hashed on the whole key, so it has no
        wildcards to give)
        
    """
    def __copy__(self) -> VertexCounts:
        ...
    def __deepcopy__(self, memo: dict) -> VertexCounts:
        ...
    def __init__(self) -> None:
        """
        Constructs a new, fully-populated vertex count lookup table
        
        :raw-html:`<br />`
        
        .. note::
            Unlike the pure-Python original there is no 'repo' argument to swap the whole table out with --
            nothing in this project ever passed one, and :meth:`addRows` already covers extending it
        """
    def __len__(self) -> int:
        """
        The total number of rows currently in the table
        """
    def addRows(self, rows: typing.Any) -> None:
        """
        Adds new rows to the table, overwriting the value of any row whose full key (every non-version
        index value, plus its parsed version) already exists
        
        Parameters
        ----------
        rows: Union[List[Tuple[List[:class:`str`], :class:`int`]], dict]
            The rows to add -- either a flat list of ``(indexVals, count)`` tuples, or a real nested dict
            exactly :attr:`totalIndices` levels deep (``{version: {name: {component: count}}}``)
        
        Raises
        ------
        :class:`ValueError`
            If the nesting depth doesn't match :attr:`totalIndices`, or a row's version value fails to parse
        """
    def clone(self) -> VertexCounts:
        """
        Creates an independent copy of this table
        
        Returns
        -------
        :class:`VertexCounts`
            The copied table
        """
    def get(self, nonVersionVals: typing.Any, versionVals: typing.Any = None, errorOnNotFound: bool = True, default: typing.Any = None) -> typing.Any:
        """
        Retrieves the corresponding vertex count
        
        Parameters
        ----------
        nonVersionVals: Union[Any, List[Any], Dict[:class:`str`, Any]]
            The values of the index columns that do not reference a version -- a bare value (taken as
            ``name``), a positional list, or a dict keyed by index name. Any column left unspecified is
            filled in with ``""``
        
        versionVals: Optional[Union[Any, List[Any], Dict[:class:`str`, Any]]]
            The version to query at -- the latest available version for the key is used if this is ``None``
        
            **Default**: ``None``
        
        errorOnNotFound: :class:`bool`
            Whether to raise :class:`KeyError` if no matching vertex count is found
        
            **Default**: ``True``
        
        default: Any
            If 'errorOnNotFound' is ``False``, the value to return when nothing is found
        
            **Default**: ``None``
        
        Raises
        ------
        :class:`KeyError`
            If no matching vertex count is found and 'errorOnNotFound' is ``True``
        
        Returns
        -------
        :class:`int`
            The found vertex count, or 'default' if none is found and 'errorOnNotFound' is ``False``
        """
    @property
    def totalIndices(self) -> int:
        """
        :class:`int`: The total number of index columns (including the version index)
        """
    @property
    def versionIndexPos(self) -> int:
        """
        :class:`int`: The position (0-based) of the version index within a row's index values
        """
class Z3Context:
    """
    
    An opaque handle to a `Z3`_ context
    
    Every named variable used across several :class:`IfPredPart`/:class:`Z3Predicate` values that
    share the same :class:`Z3Context` refers to the same underlying `Z3`_ constant -- construct one
    :class:`Z3Context` per logical group of predicates that should be comparable/combinable together
    (eg. one per .ini file being read), not a fresh one per predicate.
        
    """
    def __init__(self) -> None:
        ...
class Z3Predicate:
    """
    
    An opaque, boolean-sorted `Z3`_ predicate -- produced by :meth:`IfPredPart.getLogicQuery`, and the
    input :meth:`IfPredPart.getIfPredStr` expects
    
    .. note::
        Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
        real, independent copy
    
    .. warning::
        ``&``/``|``/``~`` (and :meth:`isSatisfiable`, since it builds a solver over this predicate's
        own context) all require every operand to belong to the same :class:`Z3Context` (see
        :meth:`belongsTo`/:meth:`sameContext`) -- combining predicates from two different contexts is
        a `Z3`_-level precondition violation that is not guaranteed to raise a catchable error. Use
        :meth:`IfPredPart.reparent` to move a predicate into a different context first if it isn't
        already guaranteed to match
        
    """
    @staticmethod
    def falseValue(ctx: Z3Context) -> Z3Predicate:
        """
        The literal ``False`` predicate, in the given `Z3`_ context
        
        Parameters
        ----------
        ctx: :class:`Z3Context`
            The context the returned predicate will belong to
        
        Returns
        -------
        :class:`Z3Predicate`
            The literal ``False`` predicate
        """
    @staticmethod
    def trueValue(ctx: Z3Context) -> Z3Predicate:
        """
        The literal ``True`` predicate, in the given `Z3`_ context
        
        Parameters
        ----------
        ctx: :class:`Z3Context`
            The context the returned predicate will belong to
        
        Returns
        -------
        :class:`Z3Predicate`
            The literal ``True`` predicate
        """
    def __and__(self, other: Z3Predicate) -> Z3Predicate:
        """
        Logical AND with another predicate; supports the ``&`` operator
        
        Parameters
        ----------
        other: :class:`Z3Predicate`
            The predicate to combine with -- must belong to the same :class:`Z3Context` as this predicate
            (see :meth:`belongsTo`)
        
        Returns
        -------
        :class:`Z3Predicate`
            The combined predicate, in this predicate's own context
        """
    def __copy__(self) -> Z3Predicate:
        ...
    def __deepcopy__(self, arg0: dict) -> Z3Predicate:
        ...
    def __invert__(self) -> Z3Predicate:
        """
        Logical negation of this predicate; supports the ``~`` operator
        
        Returns
        -------
        :class:`Z3Predicate`
            The negated predicate, in this predicate's own context
        """
    def __or__(self, other: Z3Predicate) -> Z3Predicate:
        """
        Logical OR with another predicate; supports the ``|`` operator
        
        Parameters
        ----------
        other: :class:`Z3Predicate`
            The predicate to combine with -- must belong to the same :class:`Z3Context` as this predicate
            (see :meth:`belongsTo`)
        
        Returns
        -------
        :class:`Z3Predicate`
            The combined predicate, in this predicate's own context
        """
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def belongsTo(self, ctx: Z3Context) -> bool:
        """
        Whether this predicate belongs to 'ctx' (a plain identity check, not a check of logical equivalence)
        
        Parameters
        ----------
        ctx: :class:`Z3Context`
            The context to compare against
        
        Returns
        -------
        :class:`bool`
            Whether this predicate belongs to 'ctx'
        """
    def isSatisfiable(self) -> bool:
        """
        Whether this predicate is satisfiable -- ie. whether some assignment of its free variables makes
        it evaluate to ``True``, checked via a real `Z3`_ solver
        
        Returns
        -------
        :class:`bool`
            Whether this predicate is satisfiable
        """
    def sameContext(self, other: Z3Predicate) -> bool:
        """
        Whether 'other' belongs to the same :class:`Z3Context` as this predicate (a plain identity check,
        not a check of logical equivalence)
        
        Parameters
        ----------
        other: :class:`Z3Predicate`
            The predicate to compare against
        
        Returns
        -------
        :class:`bool`
            Whether both predicates share the same underlying `Z3`_ context
        """
    def simplify(self) -> Z3Predicate:
        """
        A simplified, logically-equivalent form of this predicate
        
        Returns
        -------
        :class:`Z3Predicate`
            The simplified predicate
        """
    def toString(self) -> str:
        """
        The predicate rendered as a `Z3`_ SMT-LIB2 expression string
        
        Returns
        -------
        :class:`str`
            The string form of the predicate
        """
def appendAllToOrderedMultiMap(target: IOrderedMultiMap, items: collections.abc.Sequence[tuple[typing.Any, typing.Any]]) -> None:
    """
    Appends every ``(key, value)`` pair to any :class:`IOrderedMultiMap` implementation, in order --
    a small example of code written once against the interface, working identically whether
    'target' is :class:`OrderedMultiMap`/:class:`OrderedMultiMapSqrt` (via their
    ``asInterface()``) or a user's own Python subclass of :class:`IOrderedMultiMap`.
    
    Parameters
    ----------
    target: :class:`IOrderedMultiMap`
        The map to append to
    
    items: List[Tuple[Any, Any]]
        The key-value pairs to append, in order
    """
