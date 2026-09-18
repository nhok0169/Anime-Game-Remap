import os
from typing import Dict, List, Optional, Sequence, Tuple, Union

from .DumpMod import Character, DumpMod, importAPI
from .DraftWriter import DraftTarget, DraftWriter, ProposedSheet
from .VGMatcher import VGMatch, VGMatcher
from .VertexGroups import VertexGroups


class VGRemapFinder():
    """
    Proposes the vertex group remap between two characters from their geometry --- see
    :class:`Character` for the forms a folder may hold (3dmigoto dumps, a mod's raw ``.buf``
    files, or a raw frame analysis) and how a character of several components is read, and
    :class:`VGMatcher` for how the vertex groups are compared and matched

    Parameters
    ----------
    fromFolder: :class:`str`
        The geometry folder of the character being remapped

    toFolder: :class:`str`
        The geometry folder of the remapped character

    fromName: Optional[:class:`str`]
        The name of the character being remapped. Defaults to what :class:`Character` reads off the files

    toName: Optional[:class:`str`]
        The name of the remapped character. Defaults to what :class:`Character` reads off the files

    version: Optional[:class:`str`]
        The game version the remap targets, for the sheet titles

    weighted: :class:`bool`
        Whether the centres and spreads are blend-weight-weighted (see :class:`VertexGroups`)

    bothWays: :class:`bool`
        Whether to also propose the reverse remap (``to`` onto ``from``)

    metric: :class:`str`
        How far apart two vertex groups are --- see :class:`VGMatcher`

    mode: :class:`str`
        How the matches are chosen --- see :class:`VGMatcher`

    stayCost: :class:`float`
        See :class:`VGMatcher`

    skipCost: :class:`float`
        See :class:`VGMatcher`

    jumpCost: :class:`float`
        See :class:`VGMatcher`

    fromHashes: Union[None, :class:`str`, Sequence[:class:`str`]]
        When ``fromFolder`` is a raw frame analysis: the character's ``(position, blend, ib)``
        hashes, or the path of its ``hash.json`` (needed when it has several components)

    toHashes: Union[None, :class:`str`, Sequence[:class:`str`]]
        As ``fromHashes``, for ``toFolder``

    Attributes
    ----------
    fromChar: :class:`Character`
        The character being remapped, once :meth:`load` has run

    toChar: :class:`Character`
        The remapped character, once :meth:`load` has run

    fromGroups: :class:`VertexGroups`
        The vertex groups of the character being remapped, once :meth:`load` has run

    toGroups: :class:`VertexGroups`
        The vertex groups of the remapped character, once :meth:`load` has run
    """

    def __init__(self, fromFolder: str, toFolder: str, fromName: Optional[str] = None, toName: Optional[str] = None,
                 version: Optional[str] = None, weighted: bool = True, bothWays: bool = True,
                 metric: str = VGMatcher.MetricGaussian, mode: str = VGMatcher.ModeChains,
                 stayCost: float = VGMatcher.DefaultStayCost, skipCost: float = VGMatcher.DefaultSkipCost,
                 jumpCost: float = VGMatcher.DefaultJumpCost,
                 fromHashes: Union[None, str, Sequence[str]] = None, toHashes: Union[None, str, Sequence[str]] = None):
        self.fromFolder = fromFolder
        self.toFolder = toFolder
        self.fromName = fromName
        self.toName = toName
        self.fromHashes = fromHashes
        self.toHashes = toHashes
        self.version = version
        self.weighted = weighted
        self.bothWays = bothWays
        self.metric = metric
        self.mode = mode
        self.stayCost = stayCost
        self.skipCost = skipCost
        self.jumpCost = jumpCost

        self.fromChar: Optional[Character] = None
        self.toChar: Optional[Character] = None
        self.fromGroups: Optional[VertexGroups] = None
        self.toGroups: Optional[VertexGroups] = None

    @property
    def fromMod(self) -> Optional[DumpMod]:
        """
        The one component of a single-component source, once loaded (``None`` for several)

        :getter: Retrieves the component
        :type: Optional[:class:`DumpMod`]
        """

        return None if (self.fromGroups is None) else self.fromGroups.mod

    @property
    def toMod(self) -> Optional[DumpMod]:
        """
        The one component of a single-component target, once loaded (``None`` for several)

        :getter: Retrieves the component
        :type: Optional[:class:`DumpMod`]
        """

        return None if (self.toGroups is None) else self.toGroups.mod

    def load(self, silent: bool = False):
        """
        Reads both characters and summarises their vertex groups

        Parameters
        ----------
        silent: :class:`bool`
            Whether to skip printing progress
        """

        self.fromChar = Character.fromFolder(self.fromFolder, name = self.fromName, silent = silent, hashes = self.fromHashes)
        self.toChar = Character.fromFolder(self.toFolder, name = self.toName, silent = silent, hashes = self.toHashes)
        self.fromName = self.fromChar.name
        self.toName = self.toChar.name
        self.fromGroups = VertexGroups(self.fromChar, weighted = self.weighted)
        self.toGroups = VertexGroups(self.toChar, weighted = self.weighted)

    def makeMatcher(self, fromGroups: VertexGroups, toGroups: VertexGroups) -> VGMatcher:
        """
        Builds the matcher for one direction, with this finder's settings

        Parameters
        ----------
        fromGroups: :class:`VertexGroups`
            The vertex groups of the character being remapped

        toGroups: :class:`VertexGroups`
            The vertex groups of the remapped character

        Returns
        -------
        :class:`VGMatcher`
            The matcher
        """

        return VGMatcher(fromGroups, toGroups, metric = self.metric, mode = self.mode,
                         stayCost = self.stayCost, skipCost = self.skipCost, jumpCost = self.jumpCost)

    @classmethod
    def sheetsFor(cls, fromGroups: VertexGroups, toGroups: VertexGroups, matches: List[VGMatch], version: Optional[str]) -> List[ProposedSheet]:
        """
        Splits one direction's matches into the draft's sheets: one per **source** component,
        each with one target column per **target** component

        Parameters
        ----------
        fromGroups: :class:`VertexGroups`
            The vertex groups of the character being remapped

        toGroups: :class:`VertexGroups`
            The vertex groups of the remapped character

        matches: List[:class:`VGMatch`]
            The direction's matches, every source component together

        version: Optional[:class:`str`]
            The game version, for the sheet titles

        Returns
        -------
        List[:class:`ProposedSheet`]
            The sheets, in the source's component order
        """

        result = []
        for component in fromGroups.componentNames:
            result.append(ProposedSheet(fromGroups.character.displayName(component), toGroups.name, version,
                                        [match for match in matches if (match.fromComponent == component)], toGroups.componentNames))
        return result

    def find(self, silent: bool = False) -> List[ProposedSheet]:
        """
        Proposes the remap(s)

        Parameters
        ----------
        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        List[:class:`ProposedSheet`]
            The proposed remaps as draft sheets --- one per source component of the ``from -> to``
            direction first, then of ``to -> from`` if :attr:`bothWays` is set. (Each unpacks as
            the older ``(fromName, toName, version, matches)`` tuple too)
        """

        if (self.fromGroups is None or self.toGroups is None):
            self.load(silent = silent)

        result = self.sheetsFor(self.fromGroups, self.toGroups, self.makeMatcher(self.fromGroups, self.toGroups).match(), self.version)
        if (self.bothWays):
            result += self.sheetsFor(self.toGroups, self.fromGroups, self.makeMatcher(self.toGroups, self.fromGroups).match(), self.version)
        return result

    @classmethod
    def summary(cls, fromName: str, toName: str, matches: List[VGMatch], groupsFrom: VertexGroups, groupsTo: VertexGroups) -> str:
        """
        A printable summary of one proposed sheet

        Parameters
        ----------
        fromName: :class:`str`
            The sheet's column A header (the source character, or one of its components)

        toName: :class:`str`
            The name of the remapped character

        matches: List[:class:`VGMatch`]
            The proposed remap

        groupsFrom: :class:`VertexGroups`
            The vertex groups of the character being remapped

        groupsTo: :class:`VertexGroups`
            The vertex groups of the remapped character

        Returns
        -------
        :class:`str`
            The summary
        """

        def describe(groups: VertexGroups, only: Optional[str] = None) -> str:
            parts = []
            for component in groups.componentNames:
                if (only is not None and component != only):
                    continue
                own = groups.byComponent[component]
                withVertices = sum(1 for group in own if (not group.isEmpty))
                label = f"{component}: " if (component) else ""
                parts.append(f"{label}{len(own)} vertex groups, {withVertices} with vertices")
            return "; ".join(parts)

        sourceComponent = matches[0].fromComponent if (matches) else None
        lines = [f"== {fromName} ({describe(groupsFrom, sourceComponent)}) -> {toName} ({describe(groupsTo)}) =="]

        targets: Dict[Tuple[str, int], List[str]] = {}
        for match in matches:
            if (match.toGroup is not None):
                targets.setdefault(match.toGroup.key, []).append(match.fromGroup.label)

        unmatched = [match.fromGroup.label for match in matches if (match.toGroup is None)]
        unused = [group.label for group in groupsTo.nonEmpty if (group.key not in targets)]
        shared = {groupsTo.get(*key).label: froms for key, froms in targets.items() if (len(froms) > 1)}
        uncertain = sorted(matches, key = lambda match: -match.uncertainty)[:10]

        chains: List[Tuple] = []
        for match in matches:
            if (match.chain is not None and (not chains or chains[-1] != match.chain)):
                chains.append(match.chain)

        lines.append(f"  unmatched source groups (no vertices): {unmatched if unmatched else 'none'}")
        lines.append(f"  target groups nothing maps onto: {unused if unused else 'none'}")
        if (shared):
            lines.append("  target groups several sources map onto:")
            for to in sorted(shared):
                lines.append(f"    {to} <- {shared[to]}")

        if (chains):
            lines.append("  chains (consecutive sources onto consecutive targets):")
            lines.append("    " + ", ".join(f"{a.label}-{b.index} -> {c.label}-{d.index}" for a, b, c, d in chains))

        lines.append("  least certain matches:")
        for match in uncertain:
            if (match.toGroup is None):
                continue
            lines.append(f"    {match.fromGroup.label} -> {match.toGroup.label}  (uncertainty {match.uncertainty:.2f}; {match.comment()})")

        return "\n".join(lines)

    @classmethod
    def libraryRemap(cls, fromName: str, toName: str, fromComponent: str = "", toComponents: Sequence[str] = ("",)) -> Dict[int, DraftTarget]:
        """
        The vertex group remap the AG Remap library itself ships for a pair of mods, as read out of
        its compiled tables (the ``from`` mod type's ``vgRemaps`` table, at the latest version),
        merged over the target's components

        Parameters
        ----------
        fromName: :class:`str`
            The library's name for the mod being remapped (eg. ``"KeqingOpulent"``)

        toName: :class:`str`
            The library's name for the remapped mod (eg. ``"Keqing"``)

        fromComponent: :class:`str`
            The source component the rows are keyed by (``""`` for a single-component source)

        toComponents: Sequence[:class:`str`]
            The target components to read rows for (``[""]`` for a single-component target)

        Returns
        -------
        Dict[:class:`int`, :data:`DraftTarget`]
            The remap, source index to target: an index for a single-component target,
            ``(component, index)`` otherwise

        Raises
        ------
        :class:`KeyError`
            If the library has no such mod type, or no remap between the two
        """

        FRB = importAPI()
        FRB.CppGlobalModTypes.registerAll()

        modTypes = {modType.name: modType for modType in FRB.CppGlobalModTypes.all()}
        if (fromName not in modTypes):
            raise KeyError(f"the library has no mod type named '{fromName}' (it has: {', '.join(sorted(modTypes))})")

        toComponents = list(toComponents) or [""]
        classic = toComponents == [""]
        result: Dict[int, DraftTarget] = {}
        found = False
        for component in toComponents:
            remap = modTypes[fromName].getVGRemap(toName, fromComp = fromComponent, toComp = component)
            if (remap is None):
                continue
            found = True
            for key, value in dict(remap.remap).items():
                result[int(key)] = int(value) if (classic) else (component, int(value))

        if (not found):
            raise KeyError(f"the library ships no vertex group remap from '{fromName}' ({fromComponent or 'single component'}) to '{toName}'")
        return result

    @classmethod
    def compare(cls, matches: List[VGMatch], expected: Dict[int, DraftTarget]) -> str:
        """
        Scores a proposed remap against a known one

        Parameters
        ----------
        matches: List[:class:`VGMatch`]
            The proposed remap (one source component's matches)

        expected: Dict[:class:`int`, :data:`DraftTarget`]
            The known remap, source index to target (an index, or a ``(component, index)`` pair)

        Returns
        -------
        :class:`str`
            A printable report of the agreement and every disagreement
        """

        def normalise(value: DraftTarget) -> Optional[Tuple[str, int]]:
            if (value is None):
                return None
            if (isinstance(value, tuple)):
                return (value[0], int(value[1]))
            return ("", int(value))

        def label(key: Optional[Tuple[str, int]]) -> str:
            if (key is None):
                return "-"
            return f"{key[0]}:{key[1]}" if (key[0]) else str(key[1])

        agree = 0
        scored = 0
        disagreements: List[str] = []

        for match in matches:
            expectedTo = normalise(expected.get(match.fromIndex))
            if (expectedTo is None):
                continue

            scored += 1
            if (match.toKey == expectedTo):
                agree += 1
                continue

            runnerUpNote = ""
            if (match.runnerUp is not None and match.runnerUp.key == expectedTo):
                runnerUpNote = " (expected was the runner-up)"
            disagreements.append(f"    {match.fromGroup.label}: proposed {label(match.toKey)}, expected {label(expectedTo)}, "
                                 f"uncertainty {match.uncertainty:.2f}{runnerUpNote}")

        lines = [f"  agreement with the known remap: {agree}/{scored}" + (f" ({100.0 * agree / scored:.1f}%)" if scored else "")]
        if (disagreements):
            lines.append("  disagreements:")
            lines.extend(disagreements)
        return "\n".join(lines)

    def run(self, output: Optional[str] = None, compareTo: Optional[str] = None, silent: bool = False,
            compareLibrary: bool = False) -> List[ProposedSheet]:
        """
        Proposes the remap(s), prints a summary, and optionally writes the draft workbook and
        scores the proposal against an existing draft and/or the remap the library ships

        Parameters
        ----------
        output: Optional[:class:`str`]
            The ``.xlsx`` to write the draft into, if any

        compareTo: Optional[:class:`str`]
            An existing draft workbook (``Data/RemapDrafts/*.xlsx`` format) to score against, if any

        silent: :class:`bool`
            Whether to skip printing progress and the summary

        compareLibrary: :class:`bool`
            Whether to also score against the remap the AG Remap library ships for the pair (the
            mod names must be the library's own, eg. ``Keqing`` / ``KeqingOpulent``)

        Returns
        -------
        List[:class:`ProposedSheet`]
            The proposed remaps, as :meth:`find` returns them
        """

        sheets = self.find(silent = silent)

        if (not silent):
            print(f"metric: {self.metric}, mode: {self.mode}")
            for sheet in sheets:
                forward = sheet.toName == self.toName
                groupsFrom = self.fromGroups if (forward) else self.toGroups
                groupsTo = self.toGroups if (forward) else self.fromGroups
                print(self.summary(sheet.fromName, sheet.toName, sheet.matches, groupsFrom, groupsTo))

                if (compareTo is not None):
                    try:
                        expected = DraftWriter.readSheet(compareTo, sheet.fromName, sheet.toName)
                    except KeyError as e:
                        print(f"  (no sheet to compare against: {e})")
                    else:
                        print(self.compare(sheet.matches, expected))

                if (compareLibrary):
                    fromComponent = sheet.matches[0].fromComponent if (sheet.matches) else ""
                    try:
                        expected = self.libraryRemap(groupsFrom.name, sheet.toName, fromComponent, sheet.toComponents)
                    except KeyError as e:
                        print(f"  (no library remap to compare against: {e})")
                    else:
                        print("  against the remap the library ships:")
                        print(self.compare(sheet.matches, expected))
                print()

        if (output is not None):
            DraftWriter(output).write(sheets)
            if (not silent):
                print(f"Wrote {output}")

        return sheets
