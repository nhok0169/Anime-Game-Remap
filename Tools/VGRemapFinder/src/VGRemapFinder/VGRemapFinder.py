import os
from typing import Dict, List, Optional, Sequence, Tuple

from .DumpMod import DumpMod, importAPI
from .DraftWriter import DraftWriter
from .VGMatcher import VGMatch, VGMatcher
from .VertexGroups import VertexGroups


class VGRemapFinder():
    """
    Proposes the vertex group remap between two mods from their geometry --- see :class:`DumpMod`
    for the two forms a folder may hold (3dmigoto dumps, or a mod's raw ``.buf`` files) and
    :class:`VGMatcher` for how the vertex groups are compared and matched

    Parameters
    ----------
    fromFolder: :class:`str`
        The geometry folder of the mod being remapped

    toFolder: :class:`str`
        The geometry folder of the remapped mod

    fromName: Optional[:class:`str`]
        The name of the mod being remapped. Defaults to what :class:`DumpMod` reads off the files

    toName: Optional[:class:`str`]
        The name of the remapped mod. Defaults to what :class:`DumpMod` reads off the files

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

    fromHashes: Optional[Sequence[:class:`str`]]
        When ``fromFolder`` is a raw frame analysis: the mod's ``(position, blend, ib)`` hashes

    toHashes: Optional[Sequence[:class:`str`]]
        When ``toFolder`` is a raw frame analysis: the mod's ``(position, blend, ib)`` hashes

    Attributes
    ----------
    fromMod: :class:`DumpMod`
        The mod being remapped, once :meth:`load` has run

    toMod: :class:`DumpMod`
        The remapped mod, once :meth:`load` has run

    fromGroups: :class:`VertexGroups`
        The vertex groups of the mod being remapped, once :meth:`load` has run

    toGroups: :class:`VertexGroups`
        The vertex groups of the remapped mod, once :meth:`load` has run
    """

    def __init__(self, fromFolder: str, toFolder: str, fromName: Optional[str] = None, toName: Optional[str] = None,
                 version: Optional[str] = None, weighted: bool = True, bothWays: bool = True,
                 metric: str = VGMatcher.MetricGaussian, mode: str = VGMatcher.ModeChains,
                 stayCost: float = VGMatcher.DefaultStayCost, skipCost: float = VGMatcher.DefaultSkipCost,
                 jumpCost: float = VGMatcher.DefaultJumpCost,
                 fromHashes: Optional[Sequence[str]] = None, toHashes: Optional[Sequence[str]] = None):
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

        self.fromMod: Optional[DumpMod] = None
        self.toMod: Optional[DumpMod] = None
        self.fromGroups: Optional[VertexGroups] = None
        self.toGroups: Optional[VertexGroups] = None

    def load(self, silent: bool = False):
        """
        Reads both mods and summarises their vertex groups

        Parameters
        ----------
        silent: :class:`bool`
            Whether to skip printing progress
        """

        self.fromMod = DumpMod.fromFolder(self.fromFolder, name = self.fromName, silent = silent, hashes = self.fromHashes)
        self.toMod = DumpMod.fromFolder(self.toFolder, name = self.toName, silent = silent, hashes = self.toHashes)
        self.fromName = self.fromMod.name
        self.toName = self.toMod.name
        self.fromGroups = VertexGroups(self.fromMod, weighted = self.weighted)
        self.toGroups = VertexGroups(self.toMod, weighted = self.weighted)

    def makeMatcher(self, fromGroups: VertexGroups, toGroups: VertexGroups) -> VGMatcher:
        """
        Builds the matcher for one direction, with this finder's settings

        Parameters
        ----------
        fromGroups: :class:`VertexGroups`
            The vertex groups of the mod being remapped

        toGroups: :class:`VertexGroups`
            The vertex groups of the remapped mod

        Returns
        -------
        :class:`VGMatcher`
            The matcher
        """

        return VGMatcher(fromGroups, toGroups, metric = self.metric, mode = self.mode,
                         stayCost = self.stayCost, skipCost = self.skipCost, jumpCost = self.jumpCost)

    def find(self, silent: bool = False) -> List[Tuple[str, str, Optional[str], List[VGMatch]]]:
        """
        Proposes the remap(s)

        Parameters
        ----------
        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        List[Tuple[:class:`str`, :class:`str`, Optional[:class:`str`], List[:class:`VGMatch`]]]
            The proposed remaps, each as ``(fromName, toName, version, matches)`` --- the
            ``from -> to`` direction first, then ``to -> from`` if :attr:`bothWays` is set
        """

        if (self.fromGroups is None or self.toGroups is None):
            self.load(silent = silent)

        result = [(self.fromName, self.toName, self.version, self.makeMatcher(self.fromGroups, self.toGroups).match())]
        if (self.bothWays):
            result.append((self.toName, self.fromName, self.version, self.makeMatcher(self.toGroups, self.fromGroups).match()))
        return result

    @classmethod
    def summary(cls, fromName: str, toName: str, matches: List[VGMatch], groupsFrom: VertexGroups, groupsTo: VertexGroups) -> str:
        """
        A printable summary of one proposed remap

        Parameters
        ----------
        fromName: :class:`str`
            The name of the mod being remapped

        toName: :class:`str`
            The name of the remapped mod

        matches: List[:class:`VGMatch`]
            The proposed remap

        groupsFrom: :class:`VertexGroups`
            The vertex groups of the mod being remapped

        groupsTo: :class:`VertexGroups`
            The vertex groups of the remapped mod

        Returns
        -------
        :class:`str`
            The summary
        """

        lines = [f"== {fromName} ({len(groupsFrom)} vertex groups, {len(groupsFrom.nonEmpty)} with vertices) -> "
                 f"{toName} ({len(groupsTo)} vertex groups, {len(groupsTo.nonEmpty)} with vertices) =="]

        targets: Dict[int, List[int]] = {}
        for match in matches:
            if (match.toIndex is not None):
                targets.setdefault(match.toIndex, []).append(match.fromIndex)

        unmatched = [match.fromIndex for match in matches if (match.toIndex is None)]
        unused = [group.index for group in groupsTo.nonEmpty if (group.index not in targets)]
        shared = {to: froms for to, froms in targets.items() if (len(froms) > 1)}
        uncertain = sorted(matches, key = lambda match: -match.uncertainty)[:10]

        chains: List[Tuple[int, int, int, int]] = []
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
            lines.append("    " + ", ".join(f"{a}-{b} -> {c}-{d}" for a, b, c, d in chains))

        lines.append("  least certain matches:")
        for match in uncertain:
            if (match.toIndex is None):
                continue
            lines.append(f"    {match.fromIndex} -> {match.toIndex}  (uncertainty {match.uncertainty:.2f}; {match.comment()})")

        return "\n".join(lines)

    @classmethod
    def libraryRemap(cls, fromName: str, toName: str) -> Dict[int, Optional[int]]:
        """
        The vertex group remap the AG Remap library itself ships for a pair of mods, as read out of
        its compiled tables (the ``from`` mod type's ``vgRemaps`` table, at the latest version)

        Parameters
        ----------
        fromName: :class:`str`
            The library's name for the mod being remapped (eg. ``"KeqingOpulent"``)

        toName: :class:`str`
            The library's name for the remapped mod (eg. ``"Keqing"``)

        Returns
        -------
        Dict[:class:`int`, Optional[:class:`int`]]
            The remap, source index to target index

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

        remap = modTypes[fromName].getVGRemap(toName)
        if (remap is None):
            raise KeyError(f"the library ships no vertex group remap from '{fromName}' to '{toName}'")
        return {int(key): int(value) for key, value in dict(remap.remap).items()}

    @classmethod
    def compare(cls, matches: List[VGMatch], expected: Dict[int, Optional[int]]) -> str:
        """
        Scores a proposed remap against a known one

        Parameters
        ----------
        matches: List[:class:`VGMatch`]
            The proposed remap

        expected: Dict[:class:`int`, Optional[:class:`int`]]
            The known remap, source index to target index

        Returns
        -------
        :class:`str`
            A printable report of the agreement and every disagreement
        """

        agree = 0
        scored = 0
        disagreements: List[str] = []

        for match in matches:
            if (match.fromIndex not in expected or expected[match.fromIndex] is None):
                continue

            scored += 1
            expectedTo = expected[match.fromIndex]
            if (match.toIndex == expectedTo):
                agree += 1
                continue

            runnerUpNote = ""
            if (match.runnerUp is not None and match.runnerUp.index == expectedTo):
                runnerUpNote = " (expected was the runner-up)"
            disagreements.append(f"    {match.fromIndex}: proposed {match.toIndex}, expected {expectedTo}, "
                                 f"uncertainty {match.uncertainty:.2f}{runnerUpNote}")

        lines = [f"  agreement with the known remap: {agree}/{scored}" + (f" ({100.0 * agree / scored:.1f}%)" if scored else "")]
        if (disagreements):
            lines.append("  disagreements:")
            lines.extend(disagreements)
        return "\n".join(lines)

    def run(self, output: Optional[str] = None, compareTo: Optional[str] = None, silent: bool = False,
            compareLibrary: bool = False) -> List[Tuple[str, str, Optional[str], List[VGMatch]]]:
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
        List[Tuple[:class:`str`, :class:`str`, Optional[:class:`str`], List[:class:`VGMatch`]]]
            The proposed remaps, as :meth:`find` returns them
        """

        sheets = self.find(silent = silent)

        if (not silent):
            print(f"metric: {self.metric}, mode: {self.mode}")
            for fromName, toName, _, matches in sheets:
                groupsFrom = self.fromGroups if (fromName == self.fromName) else self.toGroups
                groupsTo = self.toGroups if (fromName == self.fromName) else self.fromGroups
                print(self.summary(fromName, toName, matches, groupsFrom, groupsTo))

                if (compareTo is not None):
                    try:
                        expected = DraftWriter.readSheet(compareTo, fromName, toName)
                    except KeyError as e:
                        print(f"  (no sheet to compare against: {e})")
                    else:
                        print(self.compare(matches, expected))

                if (compareLibrary):
                    try:
                        expected = self.libraryRemap(fromName, toName)
                    except KeyError as e:
                        print(f"  (no library remap to compare against: {e})")
                    else:
                        print("  against the remap the library ships:")
                        print(self.compare(matches, expected))
                print()

        if (output is not None):
            DraftWriter(output).write(sheets)
            if (not silent):
                print(f"Wrote {output}")

        return sheets
