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

##### ExtImport
from typing import TYPE_CHECKING, Callable, Tuple, Optional, Dict, Any, Set, List
##### EndExtImports

##### CppLocalImports
from .....core import BaseIniGraphEdit, IfContentPart, IfContentPartColouring, Ranges
##### EndCppLocalImports

##### LocalImports
from .....constants.IniConsts import IniKeywords
from .....core import IniSectionGraph
from .....tools.GraphToolsOld import GraphToolsOld as GraphTools  # TOREMOVE: GraphTools.py was renamed to GraphToolsOld.py alongside this file
from .....core import SectionIterData

if (TYPE_CHECKING):
    from ...ModType import ModType
    from ....files.IniFile import IniFile
##### EndLocalImports


##### Script
class RegSurroundedAddOld(BaseIniGraphEdit):
    """
    This class inherits from :class:`BaseIniGraphEdit`

    Adds a `KVP`_ into some caller/callee graph of :class:`IniSectionGraph`, at every location that is `surrounded`
    by a particular set of registers: after every register specified at 'beforeRegs' has been seen at least once
    (and accepted by its predicate) and before every register specified at 'afterRegs' has been seen at least once
    (and accepted by its predicate)

    Parameters
    ----------
    addition: Tuple[:class:`str`, :class:`str`]
        The `KVP`_ to add

    beforeRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
        The registers that must come before :attr:`addition` (ie. :attr:`addition` gets added after these
        registers) :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the names of the registers
        * The values are the predicates for which particular occurence of the register to accept, taking in the
          value of the occurence :raw-html:`<br />` :raw-html:`<br />`

          If a value is ``None``, then any occurence of the corresponding register is accepted

        :raw-html:`<br />`

        This condition is only satisfied once at least one accepted occurence has been seen for **every** key
        specified in this argument :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    afterRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
        The registers that must come after :attr:`addition` (ie. :attr:`addition` gets added before these
        registers) :raw-html:`<br />` :raw-html:`<br />`

        Follows the same format/semantics as :attr:`beforeRegs`, except the condition applies for coming after
        :attr:`addition` instead of before it :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    latest: :class:`bool`
        Whether to add :attr:`addition` at the latest valid location within the surrounded window, instead of
        the earliest one :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    Attributes
    ----------
    addition: Tuple[:class:`str`, :class:`str`]
        The `KVP`_ to add

    beforeRegs: Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]
        The registers that must come before :attr:`addition`

    afterRegs: Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]
        The registers that must come after :attr:`addition`

    latest: :class:`bool`
        Whether to add :attr:`addition` at the latest valid location within the surrounded window, instead of
        the earliest one
    """

    def __init__(self, addition: Tuple[str, str], beforeRegs: Optional[Dict[str, Optional[Callable[[str], bool]]]] = None,
                 afterRegs: Optional[Dict[str, Optional[Callable[[str], bool]]]] = None, latest: bool = False):
        # BaseIniGraphEdit is a pybind11-bound class now, so its C++ subobject has to be constructed
        # explicitly -- a Python subclass that defines __init__ without calling super().__init__()
        # never initializes it.
        super().__init__()

        self.addition = addition
        self.beforeRegs = {} if (beforeRegs is None) else beforeRegs
        self.afterRegs = {} if (afterRegs is None) else afterRegs
        self.latest = latest

        self._beforeFilters = self._buildKeyFilters(self.beforeRegs)
        self._afterFilters = self._buildKeyFilters(self.afterRegs)
        self._trackedKeys = set(self.beforeRegs.keys()) | set(self.afterRegs.keys())

    @staticmethod
    def _buildKeyFilters(regs: Dict[str, Optional[Callable[[str], bool]]]) -> Dict[str, Callable[[Optional[int], Any], bool]]:
        """
        Adapts the value-only predicates at 'regs' into the ``(index, value) -> bool`` shape expected by
        :meth:`IfContentPartColouring.getRanges`

        .. note::
            Registers whose predicate is ``None`` are left out on purpose -- accepting "any value" for a register
            is already handled by simply checking the register's existence (see :meth:`_getSatisfiedRange`)

        Parameters
        ----------
        regs: Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]
            The register predicates to adapt

        Returns
        -------
        Dict[:class:`str`, Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            The adapted predicates
        """

        result = {}
        for reg, pred in regs.items():
            if (pred is None):
                continue
            result[reg] = (lambda ind, val, pred = pred: pred(val))

        return result

    @classmethod
    def _getSatisfiedRange(cls, colouring: IfContentPartColouring, regs: Dict[str, Optional[Callable[[str], bool]]],
                           filters: Dict[str, Callable[[Optional[int], Any], bool]], includeKeyDefs: bool) -> Ranges:
        """
        Retrieves the ranges of indices within the current :class:`IfContentPart` at which every register
        specified at 'regs' has already been seen at least once (and is currently holding a value accepted by
        its predicate, if it has one)

        Parameters
        ----------
        colouring: :class:`IfContentPartColouring`
            The current `KVP`_ states to search within

        regs: Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]
            The registers that all need to have been seen

        filters: Dict[:class:`str`, Callable[[Optional[:class:`int`], Any], :class:`bool`]]
            The corresponding predicates for 'regs', pre-adapted by :meth:`_buildKeyFilters`

        includeKeyDefs: :class:`bool`
            Whether the exact index a register gets (re)defined at counts as part of the satisfied range :raw-html:`<br />` :raw-html:`<br />`

            See :meth:`IfContentPartColouring.getRanges` for the full semantics

        Returns
        -------
        :class:`Ranges`
            The ranges of indices where all of 'regs' have been satisfied
        """

        if (not regs):
            return Ranges.createFull()

        keysExists = {reg: True for reg in regs}
        return colouring.getRanges(keysExists = keysExists, keyFilters = filters, existsRequireAll = True, filtersRequireAll = True,
                                    globalRequireAll = True, includeKeyDefs = includeKeyDefs)

    @staticmethod
    def _keysExistSomewhere(graph: IniSectionGraph, keys: Set[str]) -> bool:
        """
        Checks whether every register in 'keys' is defined by at least one :class:`IfContentPart` somewhere in
        'graph' :raw-html:`<br />` :raw-html:`<br />`

        Used to rule out a register in :attr:`afterRegs` that can provably never be satisfied anywhere in the
        graph -- :meth:`_getValidRange` alone can't tell that case apart from "not reached yet, but still will
        be" (see its own note on this)

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to search

        keys: Set[:class:`str`]
            The registers that all need to be found somewhere in 'graph'

        Returns
        -------
        :class:`bool`
            Whether every register in 'keys' was found
        """

        remaining = set(keys)
        if (not remaining):
            return True

        for iterData in graph.iterByContentPart():
            part = iterData.part
            remaining.difference_update([key for key in remaining if key in part])

            if (not remaining):
                return True

        return False

    def _pickInsertInd(self, validRange: Ranges, part: IfContentPart) -> int:
        """
        Picks a single index from 'validRange' to add :attr:`addition` at, according to :attr:`latest`

        Parameters
        ----------
        validRange: :class:`Ranges`
            The valid, non-empty ranges of indices to add :attr:`addition` at

        part: :class:`IfContentPart`
            The part 'validRange' was computed for -- used as a fallback bound in case an endpoint of
            'validRange' is unbounded (shouldn't normally happen, since 'validRange' is expected to already be
            clipped to the bounds of 'part')

        Returns
        -------
        :class:`int`
            The chosen index
        """

        ranges = validRange.ranges

        if (self.latest):
            insertInd = ranges[-1][1]
            return len(part) if (insertInd is None) else insertInd - 1

        insertInd = ranges[0][0]
        return 0 if (insertInd is None) else insertInd

    def _getForwardValidRangeForPart(self, part: IfContentPart, beforeEntryFacts: Dict[str, bool],
                                     beforeReturnFacts: Dict[str, bool]) -> Ranges:
        """
        The lower bound half of :meth:`_getValidRangeForPart`: retrieves the ranges of indices within 'part' at
        which every :attr:`beforeRegs` register has already been satisfied, treating 'beforeEntryFacts' as the
        state at the very start of 'part' (in place of what an isolated, single-part colouring pass would show) :raw-html:`<br />` :raw-html:`<br />`

        A register already satisfied entering 'part' (``beforeEntryFacts[reg] == True``) but *also* (re)defined
        within 'part' itself needs special handling: :class:`IfContentPartColouring` has no notion of "already
        satisfied before this part started" once a key is touched locally, it can only reflect that key's local
        occurences from then on. So the carried-in satisfaction is reinstated by explicitly unioning back in
        every index up to (and including) that register's own first local occurence -- inserting there still
        lands 'addition' before that occurence, governed by the still-valid, already-accepted incoming value :raw-html:`<br />` :raw-html:`<br />`

        A register never touched by 'part' at all, and *not* already satisfied entering it, can still become
        satisfied purely through a ``run =`` call 'part' makes (eg. the callee itself defines it). That can only
        ever justify indices strictly after that call, never before or at it -- 'part' hasn't made the call yet
        at those earlier indices, so nothing the callee does could have happened there. See
        :meth:`_getBackwardValidRangeForPart` for the mirror image of this same call/return split

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part to search within

        beforeEntryFacts: Dict[:class:`str`, :class:`bool`]
            For every :attr:`beforeRegs` register, whether it was already satisfied entering 'part' (as computed
            by :meth:`GraphTools.runForwardMustFixpoint`)

        beforeReturnFacts: Dict[:class:`str`, :class:`bool`]
            For every :attr:`beforeRegs` register, whether it's satisfied entering the point right after 'part's
            own ``run =`` call has *returned* (as computed by :meth:`GraphTools.runForwardMustFixpoint`); identical to
            'beforeEntryFacts' if 'part' makes no call

        Returns
        -------
        :class:`Ranges`
            The valid ranges of indices to add :attr:`addition` at, from the 'beforeRegs' side alone
        """

        if (not self.beforeRegs):
            return Ranges.createFull()

        callInds = [ind for ind, val in part.getValsWithInds(IniKeywords.Run.value)]
        lastCallInd = max(callInds) if (callInds) else None

        perRegRanges = []
        for reg, pred in self.beforeRegs.items():
            filters = {reg: self._beforeFilters[reg]} if (reg in self._beforeFilters) else {}
            localColouring = IfContentPartColouring()
            localColouring.updateColouring(part, targetKeys = {reg})

            if (reg not in localColouring):
                regRange = Ranges.createFull() if (beforeEntryFacts.get(reg, False)) else Ranges.createEmpty()
                if (lastCallInd is not None and beforeReturnFacts.get(reg, False)):
                    regRange = regRange.union([Ranges([(lastCallInd + 1, None)])])
                perRegRanges.append(regRange)
                continue

            localRange = self._getSatisfiedRange(localColouring, {reg: pred}, filters, includeKeyDefs = False)
            if (beforeEntryFacts.get(reg, False)):
                firstInd = min(ind for ind, val in part.getValsWithInds(reg))
                localRange = localRange.union([Ranges([(0, firstInd + 1)])])

            perRegRanges.append(localRange)

        return perRegRanges[0].intersect(perRegRanges[1:])

    def _getBackwardValidRangeForPart(self, part: IfContentPart, afterExitFacts: Dict[str, bool],
                                      afterReturnFacts: Dict[str, bool]) -> Ranges:
        """
        The upper bound half of :meth:`_getValidRangeForPart`: retrieves the ranges of indices within 'part' at
        which every :attr:`afterRegs` register will still become satisfied :raw-html:`<br />` :raw-html:`<br />`

        Unlike the 'beforeRegs' side, this is a plain existential check, not a state-based one: 'addition' is
        valid at any index at-or-before *some* accepted occurence of a register (a union over that register's
        own accepted occurences within 'part') :raw-html:`<br />` :raw-html:`<br />`

        If 'part' makes no ``run =`` call, the entire part is additionally valid whenever 'afterExitFacts' says
        an accepted occurence is guaranteed somewhere after 'part' too (every index trivially precedes it) :raw-html:`<br />` :raw-html:`<br />`

        If 'part' *does* make a call, that bulk check is instead split at the index of its last call, using two
        independently-computed facts:

        * indices up to (and including) the last call: valid if 'afterExitFacts' holds -- reaching that index
          still guarantees flowing *into* the call, and 'afterExitFacts' already accounts for everything
          reachable from there onwards, callee included
        * indices strictly after the last call: valid only if 'afterReturnFacts' holds -- these only ever run
          once the call has actually *returned*, which 'afterExitFacts' alone can't establish. A call that
          never returns (eg. unconditional recursion with no escape) would otherwise let a position that can
          provably never execute vacuously "satisfy" this and look like a valid, reachable insertion point

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part to search within

        afterExitFacts: Dict[:class:`str`, :class:`bool`]
            For every :attr:`afterRegs` register, whether it's guaranteed to be satisfied somewhere after 'part'
            makes its call (or after 'part' itself, if it makes no call) (as computed by :meth:`GraphTools.runBackwardMustFixpoint`)

        afterReturnFacts: Dict[:class:`str`, :class:`bool`]
            For every :attr:`afterRegs` register, whether it's guaranteed to be satisfied somewhere after 'part's
            own ``run =`` call has *returned* (as computed by :meth:`GraphTools.runBackwardMustFixpoint`); identical to
            'afterExitFacts' if 'part' makes no call

        Returns
        -------
        :class:`Ranges`
            The valid ranges of indices to add :attr:`addition` at, from the 'afterRegs' side alone
        """

        if (not self.afterRegs):
            return Ranges.createFull()

        callInds = [ind for ind, val in part.getValsWithInds(IniKeywords.Run.value)]
        lastCallInd = max(callInds) if (callInds) else None

        perRegRanges = []
        for reg, pred in self.afterRegs.items():
            acceptedInds = [ind for ind, val in part.getValsWithInds(reg) if (pred is None or pred(val))]
            regRange = Ranges([(None, ind + 1) for ind in acceptedInds]) if (acceptedInds) else Ranges.createEmpty()

            if (lastCallInd is None):
                if (afterExitFacts.get(reg, False)):
                    regRange = regRange.union([Ranges.createFull()])
            else:
                if (afterExitFacts.get(reg, False)):
                    regRange = regRange.union([Ranges([(0, lastCallInd + 1)])])
                if (afterReturnFacts.get(reg, False)):
                    regRange = regRange.union([Ranges([(lastCallInd + 1, None)])])

            perRegRanges.append(regRange)

        return perRegRanges[0].intersect(perRegRanges[1:])

    def _getValidRangeForPart(self, part: IfContentPart, beforeEntryFacts: Dict[str, bool], beforeReturnFacts: Dict[str, bool],
                              afterExitFacts: Dict[str, bool], afterReturnFacts: Dict[str, bool]) -> Ranges:
        """
        Retrieves the ranges of indices within 'part' at which :attr:`addition` can be added -- combining
        :meth:`_getForwardValidRangeForPart` and :meth:`_getBackwardValidRangeForPart` :raw-html:`<br />` :raw-html:`<br />`

        Prefers a position that's valid *without* leaning on any of 'part's own call/return facts at all
        (``afterExitFacts``, ``afterReturnFacts``, and the "reg not touched locally" use of ``beforeReturnFacts``)
        -- falling back to those only when no such position exists. ``beforeEntryFacts`` is kept either way, since
        it reflects something that genuinely already happened earlier in the *same*, ordinary (non-recursive)
        traversal, not a hypothetical future lap :raw-html:`<br />` :raw-html:`<br />`

        Without this preference, a position justified *only* by looping back around 'part's own call (eg. the
        next lap of a cycle reaching a register again) could get chosen over a nearer position justified by a
        register occurence that's already sitting right there in 'part' -- both are technically valid per
        :attr:`afterRegs`'s "some accepted occurence somewhere after" semantics, but the nearer, already-present
        occurence is the more sensible one to actually use when it's available on its own

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part to search within

        beforeEntryFacts: Dict[:class:`str`, :class:`bool`]
            See :meth:`_getForwardValidRangeForPart`

        beforeReturnFacts: Dict[:class:`str`, :class:`bool`]
            See :meth:`_getForwardValidRangeForPart`

        afterExitFacts: Dict[:class:`str`, :class:`bool`]
            See :meth:`_getBackwardValidRangeForPart`

        afterReturnFacts: Dict[:class:`str`, :class:`bool`]
            See :meth:`_getBackwardValidRangeForPart`

        Returns
        -------
        :class:`Ranges`
            The valid ranges of indices to add :attr:`addition` at
        """

        localOnlyRange = self._getForwardValidRangeForPart(part, beforeEntryFacts, {}).intersect(
                            [self._getBackwardValidRangeForPart(part, {}, {})])
        if (not localOnlyRange.isEmpty()):
            return localOnlyRange

        mustComeAfterRange = self._getForwardValidRangeForPart(part, beforeEntryFacts, beforeReturnFacts)
        mustComeBeforeRange = self._getBackwardValidRangeForPart(part, afterExitFacts, afterReturnFacts)
        return mustComeAfterRange.intersect([mustComeBeforeRange])

    def _getClippedValidRangeForPart(self, iterData: SectionIterData, modType: "ModType",
                                     partFilter: Optional[Callable[[SectionIterData, "ModType", Optional["IniFile"]], Ranges]],
                                     beforeEntryFacts: Dict[str, Dict[Any, bool]], afterExitFacts: Dict[str, Dict[Any, bool]]) -> Ranges:
        """
        Same as :meth:`_getValidRangeForPart`, additionally clipped to the bounds of 'iterData's part and, if
        given, 'partFilter'

        Parameters
        ----------
        iterData: :class:`SectionIterData`
            The current part being visited

        modType: :class:`ModType`
            The type of mod to fix, forwarded to 'partFilter'

        partFilter: Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]
            The filter used to indicate the valid order indices to process 'iterData's part, if any

        beforeEntryFacts: Dict[:class:`str`, Dict[Any, :class:`bool`]]
            For every :attr:`beforeRegs` register, every `call graph`_ node (as built by :meth:`IniSectionGraph.buildCallGraph`)
            mapped to whether that register was satisfied entering that node (as computed by :meth:`_computeKeyFacts`)

        afterExitFacts: Dict[:class:`str`, Dict[Any, :class:`bool`]]
            For every :attr:`afterRegs` register, every `call graph`_ node mapped to whether that register is
            guaranteed to be satisfied after that node exits (as computed by :meth:`_computeKeyFacts`)

        Returns
        -------
        :class:`Ranges`
            The clipped, valid ranges of indices to add :attr:`addition` at
        """

        part = iterData.part
        partId = id(part)
        exitNodeId = ("exit", partId) if (part.getVals(IniKeywords.Run.value)) else partId

        partBeforeFacts = {reg: facts.get(partId, False) for reg, facts in beforeEntryFacts.items()}
        partBeforeReturnFacts = {reg: facts.get(exitNodeId, False) for reg, facts in beforeEntryFacts.items()}
        partAfterFacts = {reg: facts.get(partId, False) for reg, facts in afterExitFacts.items()}
        partAfterReturnFacts = {reg: facts.get(exitNodeId, False) for reg, facts in afterExitFacts.items()}

        validRange = self._getValidRangeForPart(part, partBeforeFacts, partBeforeReturnFacts, partAfterFacts, partAfterReturnFacts)
        if (validRange.isEmpty()):
            return validRange

        bounds = [Ranges([(0, len(part) + 1)])]
        if (partFilter is not None):
            bounds.append(partFilter(iterData, modType, None))

        return validRange.intersect(bounds)

    @staticmethod
    def _computeLocalForwardFact(part: IfContentPart, reg: str, pred: Optional[Callable[[str], bool]]) -> Tuple[bool, bool]:
        """
        Computes, using only 'part's own content (no surrounding context), whether 'reg' is touched at all
        within 'part', and if so, whether it's still satisfied by the very end of 'part'

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part to search within

        reg: :class:`str`
            The register to check

        pred: Optional[Callable[[:class:`str`], :class:`bool`]]
            The predicate for which particular occurence of 'reg' to accept

        Returns
        -------
        Tuple[:class:`bool`, :class:`bool`]
            A tuple containing whether 'part' touches 'reg' at all, and (only meaningful if it does) whether
            'reg' is still satisfied once every KVP in 'part' has been processed
        """

        colouring = IfContentPartColouring()
        colouring.updateColouring(part, targetKeys = {reg})
        if (reg not in colouring):
            return False, False

        filters = {reg: (lambda ind, val, pred = pred: pred(val))} if (pred is not None) else {}
        satisfiedRange = colouring.getRanges(keysExists = {reg: True}, keyFilters = filters, includeKeyDefs = True)
        return True, satisfiedRange.has(len(part))

    @staticmethod
    def _computeLocalBackwardFact(part: IfContentPart, reg: str, pred: Optional[Callable[[str], bool]]) -> bool:
        """
        Checks whether 'part' contains *any* accepted occurence of 'reg', anywhere within it (a plain
        existential check over 'part's own content, no surrounding context)

        Parameters
        ----------
        part: :class:`IfContentPart`
            The part to search within

        reg: :class:`str`
            The register to check

        pred: Optional[Callable[[:class:`str`], :class:`bool`]]
            The predicate for which particular occurence of 'reg' to accept

        Returns
        -------
        :class:`bool`
            Whether an accepted occurence of 'reg' was found
        """

        return any((pred is None or pred(val)) for ind, val in part.getValsWithInds(reg))

    def _computeKeyFacts(self, graph: IniSectionGraph) -> Tuple[Dict[str, Dict[Any, bool]], Dict[str, Dict[Any, bool]]]:
        """
        Runs :meth:`GraphTools.runForwardMustFixpoint` for every :attr:`beforeRegs` register and
        :meth:`GraphTools.runBackwardMustFixpoint` for every :attr:`afterRegs` register, over one shared
        :meth:`IniSectionGraph.buildCallGraph`, then clamps every result via
        :meth:`GraphTools.clampFactsToReachable` so a node that can provably never execute never contributes a fact

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to compute the facts for

        Returns
        -------
        Tuple[Dict[:class:`str`, Dict[Any, :class:`bool`]], Dict[:class:`str`, Dict[Any, :class:`bool`]]]
            A tuple containing:

            #. Every :attr:`beforeRegs` register, mapped to its own (clamped) forward `fixpoint`_ result
            #. Every :attr:`afterRegs` register, mapped to its own (clamped) backward `fixpoint`_ result
        """

        if (not self.beforeRegs and not self.afterRegs):
            return {}, {}

        callGraph = graph.buildCallGraph()
        reachableNodes = GraphTools.getReachableNodes(callGraph.forwardEdges, callGraph.rootNodeIds)

        beforeEntryFacts = {}
        for reg, pred in self.beforeRegs.items():
            localFacts = {pid: self._computeLocalForwardFact(part, reg, pred) for pid, part in callGraph.partsById.items()}
            rawFacts = GraphTools.runForwardMustFixpoint(callGraph.forwardEdges, callGraph.backwardEdges, callGraph.rootNodeIds, localFacts)
            beforeEntryFacts[reg] = GraphTools.clampFactsToReachable(rawFacts, reachableNodes)

        afterExitFacts = {}
        for reg, pred in self.afterRegs.items():
            localFacts = {pid: self._computeLocalBackwardFact(part, reg, pred) for pid, part in callGraph.partsById.items()}
            rawFacts = GraphTools.runBackwardMustFixpoint(callGraph.forwardEdges, callGraph.backwardEdges, localFacts)
            afterExitFacts[reg] = GraphTools.clampFactsToReachable(rawFacts, reachableNodes)

        return beforeEntryFacts, afterExitFacts

    def _editEarliest(self, graph: IniSectionGraph, modType: "ModType",
                      partFilter: Optional[Callable[[SectionIterData, "ModType", Optional["IniFile"]], Ranges]]) -> IniSectionGraph:
        """
        Adds :attr:`addition` at the earliest valid location of every still-open `surrounded`_ window in 'graph',
        skipping any part whose window was already claimed by a predecessor part (see
        :meth:`IniSectionGraph.buildPartPredecessorGraph`)

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit

        modType: :class:`ModType`
            The type of mod to fix

        partFilter: Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]
            The filter used to indicate the valid order indices to process some part

        Returns
        -------
        :class:`IniSectionGraph`
            The resultant graph that got editted
        """

        predecessors = graph.buildPartPredecessorGraph()
        beforeEntryFacts, afterExitFacts = self._computeKeyFacts(graph)
        claimed: Dict[int, bool] = {}

        for iterData in graph.iterByContentPart(colour = True, colourKeys = self._trackedKeys):
            part = iterData.part
            isClaimed = any(claimed.get(pid, False) for pid in predecessors.get(id(part), []))

            if (not isClaimed):
                validRange = self._getClippedValidRangeForPart(iterData, modType, partFilter, beforeEntryFacts, afterExitFacts)
                if (not validRange.isEmpty()):
                    part.addKVPAt(self._pickInsertInd(validRange, part), self.addition[0], self.addition[1])
                    isClaimed = True

            claimed[id(part)] = isClaimed

        return graph

    def _editLatest(self, graph: IniSectionGraph, modType: "ModType",
                    partFilter: Optional[Callable[[SectionIterData, "ModType", Optional["IniFile"]], Ranges]]) -> IniSectionGraph:
        """
        Adds :attr:`addition` at the latest valid location of every still-open `surrounded`_ window in 'graph' :raw-html:`<br />` :raw-html:`<br />`

        Whether a part is actually the *latest* valid one for its window depends on whether any part that comes
        after it (see :meth:`IniSectionGraph.buildPartPredecessorGraph`) also has a valid location for that same
        window -- which isn't known until every such part has been visited. So this collects every part's own candidate location
        first (in the same single pass :meth:`_editEarliest` decides in), then decides/commits in reverse of that
        visiting order -- by the time a given part is decided, everything that comes after it already has been

        Parameters
        ----------
        graph: :class:`IniSectionGraph`
            The graph to edit

        modType: :class:`ModType`
            The type of mod to fix

        partFilter: Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]
            The filter used to indicate the valid order indices to process some part

        Returns
        -------
        :class:`IniSectionGraph`
            The resultant graph that got editted
        """

        predecessors = graph.buildPartPredecessorGraph()
        successors: Dict[int, List[int]] = {}
        for pid, preds in predecessors.items():
            for predId in preds:
                successors.setdefault(predId, []).append(pid)

        beforeEntryFacts, afterExitFacts = self._computeKeyFacts(graph)
        candidates: Dict[int, Optional[Tuple[IfContentPart, int]]] = {}
        visitOrder: List[int] = []

        for iterData in graph.iterByContentPart(colour = True, colourKeys = self._trackedKeys):
            part = iterData.part
            validRange = self._getClippedValidRangeForPart(iterData, modType, partFilter, beforeEntryFacts, afterExitFacts)
            candidates[id(part)] = None if (validRange.isEmpty()) else (part, self._pickInsertInd(validRange, part))
            visitOrder.append(id(part))

        claimed: Dict[int, bool] = {}
        for pid in reversed(visitOrder):
            isClaimed = any(claimed.get(sid, False) for sid in successors.get(pid, []))

            if (not isClaimed and candidates[pid] is not None):
                candidatePart, candidateInd = candidates[pid]
                candidatePart.addKVPAt(candidateInd, self.addition[0], self.addition[1])
                isClaimed = True

            claimed[pid] = isClaimed

        return graph

    def edit(self, graph: IniSectionGraph, modType: "ModType", modName: str = "", partFilter: Optional[Callable[[SectionIterData, "ModType", Optional["IniFile"]], Ranges]] = None,
             trackKeys: bool = False, keysToTrack: Optional[Set[str]] = None) -> IniSectionGraph:
        # 'trackKeys'/'keysToTrack' are the caller's key-tracking defaults, handed down by
        # BaseIniGraphEdit's contract (GraphGroupEdit passes its own). This edit builds its own
        # colourings from its own beforeRegs/afterRegs, so it has no use for them -- they are
        # accepted only so the shared call convention keeps working.
        if (self.afterRegs and not self._keysExistSomewhere(graph, self.afterRegs.keys())):
            return graph

        if (self.latest):
            return self._editLatest(graph, modType, partFilter)
        return self._editEarliest(graph, modType, partFilter)
##### EndScript
