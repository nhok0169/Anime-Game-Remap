from typing import Dict, List, Optional, Tuple

import numpy as np

from .VertexGroups import VertexGroup, VertexGroups


class VGMatch():
    """
    The proposed remap of one vertex group

    Parameters
    ----------
    fromGroup: :class:`VertexGroup`
        The vertex group of the mod being remapped

    toGroup: Optional[:class:`VertexGroup`]
        The vertex group of the remapped mod it maps onto, or ``None`` if no match was found

    distance: :class:`float`
        The distance between the two groups (by whichever metric the matcher used)

    runnerUp: Optional[:class:`VertexGroup`]
        The next best candidate

    runnerUpDistance: :class:`float`
        The distance to the runner-up

    uncertainty: :class:`float`
        How unsure the match is, from 0 (confident) to 1 (a guess). See :class:`VGMatcher` for
        how it is scored

    Attributes
    ----------
    fromGroup: :class:`VertexGroup`
        The vertex group of the mod being remapped

    toGroup: Optional[:class:`VertexGroup`]
        The vertex group it maps onto

    distance: :class:`float`
        The distance between the two groups

    runnerUp: Optional[:class:`VertexGroup`]
        The next best candidate

    runnerUpDistance: :class:`float`
        The distance to the runner-up

    uncertainty: :class:`float`
        How unsure the match is

    chain: Optional[Tuple[:class:`int`, :class:`int`, :class:`int`, :class:`int`]]
        When the match is part of a run of consecutive source indices mapped onto consecutive
        target indices: ``(firstFrom, lastFrom, firstTo, lastTo)`` of that run. ``None`` for a
        match that stands alone
    """

    def __init__(self, fromGroup: VertexGroup, toGroup: Optional[VertexGroup], distance: float,
                 runnerUp: Optional[VertexGroup], runnerUpDistance: float, uncertainty: float):
        self.fromGroup = fromGroup
        self.toGroup = toGroup
        self.distance = distance
        self.runnerUp = runnerUp
        self.runnerUpDistance = runnerUpDistance
        self.uncertainty = uncertainty
        self.chain: Optional[Tuple[int, int, int, int]] = None

        # only set by the "vertices" mode: what share of the source group's skin the chosen /
        #   runner-up target bone drives on the other skin
        self.share: Optional[float] = None
        self.runnerUpShare: Optional[float] = None

    @property
    def fromIndex(self) -> int:
        return self.fromGroup.index

    @property
    def toIndex(self) -> Optional[int]:
        return None if (self.toGroup is None) else self.toGroup.index

    def comment(self) -> str:
        """
        A one-line explanation of the match, for the draft's ``Comments`` column

        Returns
        -------
        :class:`str`
            The comment
        """

        fromObjects = "/".join(sorted(self.fromGroup.objects)) or "-"
        if (self.toGroup is None):
            return f"no vertices in the source group (objects: {fromObjects}); unmatched"

        toObjects = "/".join(sorted(self.toGroup.objects)) or "-"
        if (self.share is not None):
            result = (f"{self.fromGroup.vertexCount} verts ({fromObjects}) -> {self.toGroup.vertexCount} verts ({toObjects}): "
                      f"drives {100 * self.share:.0f}% of the nearest skin, mean vertex dist {self.distance:.4f}")
            if (self.runnerUp is not None and self.runnerUpShare is not None):
                result += f"; runner-up {self.runnerUp.index} drives {100 * self.runnerUpShare:.0f}%"
            return result

        result = (f"{self.fromGroup.vertexCount} verts ({fromObjects}) -> {self.toGroup.vertexCount} verts ({toObjects}), "
                  f"dist {self.distance:.4f}")

        if (self.runnerUp is not None):
            result += f"; runner-up {self.runnerUp.index} at dist {self.runnerUpDistance:.4f}"

        if (self.chain is not None):
            firstFrom, lastFrom, firstTo, lastTo = self.chain
            result += f"; in chain {firstFrom}-{lastFrom} -> {firstTo}-{lastTo}"

        return result


class VGMatcher():
    """
    Matches the vertex groups of one mod onto those of another :raw-html:`<br />` :raw-html:`<br />`

    Two things are chosen independently:

    **The metric** --- how far apart two vertex groups are:

    - ``"center"``: the distance between the groups' centre points
    - ``"gaussian"``: the 2-Wasserstein distance between a Gaussian fitted to each group's
      vertices (its centre and spread), so two groups whose vertices reach differently far, or in
      different directions, are further apart than their centres alone would say. This is the
      centre distance plus a spread term in the same units, with nothing to tune

    **The mode** --- how the matches are chosen:

    - ``"nearest"``: every source group independently maps onto the closest target group
    - ``"chains"``: the game numbers a skeleton's bones in order, so a chain of bones (hair, a
      ribbon, a skirt's rows) is a run of consecutive indices in **both** mods. Each run of
      consecutive source indices is aligned onto the target indices as a whole, by the cheapest
      path where stepping to the next target index is free, skipping one costs
      :attr:`skipCost`, and staying on the same target (several sources onto one target) or
      jumping anywhere else costs :attr:`stayCost` / :attr:`jumpCost`. Costs are in units of the
      target mod's :attr:`VertexGroups.spacing`
    - ``"vertices"``: ignores the group summaries and the metric altogether. Two skins of one
      character share most of their skin, so for every vertex a source group drives, the
      *nearest vertex* on the target skin is found, and the target bones driving those vertices
      are tallied (each source vertex's weight times each target bone's weight). The group maps
      onto the bone with the largest share --- a direct answer to "what moves this patch of skin
      on the other skin". The match's :attr:`VGMatch.distance` is the mean distance to the
      nearest vertices, a measure of whether that patch of skin exists on the target at all

    Parameters
    ----------
    fromGroups: :class:`VertexGroups`
        The vertex groups of the mod being remapped

    toGroups: :class:`VertexGroups`
        The vertex groups of the remapped mod

    metric: :class:`str`
        ``"center"`` or ``"gaussian"``

    mode: :class:`str`
        ``"nearest"`` or ``"chains"``

    stayCost: :class:`float`
        In ``"chains"`` mode, the cost of mapping consecutive source groups onto the *same* target

    skipCost: :class:`float`
        In ``"chains"`` mode, the cost of skipping one target index between consecutive sources

    jumpCost: :class:`float`
        In ``"chains"`` mode, the cost of any other step between consecutive sources

    Attributes
    ----------
    fromGroups: :class:`VertexGroups`
        The vertex groups of the mod being remapped

    toGroups: :class:`VertexGroups`
        The vertex groups of the remapped mod

    metric: :class:`str`
        The metric used

    mode: :class:`str`
        The mode used

    stayCost: :class:`float`
        The cost of staying on the same target

    skipCost: :class:`float`
        The cost of skipping one target index

    jumpCost: :class:`float`
        The cost of any other step
    """

    MetricCenter = "center"
    MetricGaussian = "gaussian"
    Metrics = [MetricGaussian, MetricCenter]

    ModeNearest = "nearest"
    ModeChains = "chains"
    ModeVertices = "vertices"
    Modes = [ModeChains, ModeNearest, ModeVertices]

    DefaultStayCost = 1.0
    DefaultSkipCost = 0.25
    DefaultJumpCost = 1.0

    def __init__(self, fromGroups: VertexGroups, toGroups: VertexGroups, metric: str = MetricGaussian, mode: str = ModeChains,
                 stayCost: float = DefaultStayCost, skipCost: float = DefaultSkipCost, jumpCost: float = DefaultJumpCost):
        if (metric not in self.Metrics):
            raise ValueError(f"unknown metric '{metric}' (expected one of {self.Metrics})")
        if (mode not in self.Modes):
            raise ValueError(f"unknown mode '{mode}' (expected one of {self.Modes})")

        self.fromGroups = fromGroups
        self.toGroups = toGroups
        self.metric = metric
        self.mode = mode
        self.stayCost = stayCost
        self.skipCost = skipCost
        self.jumpCost = jumpCost

    # ---------------------------------------------------------------------------------------------
    # distances

    @classmethod
    def centerDistances(cls, fromGroups: List[VertexGroup], toGroups: List[VertexGroup]) -> np.ndarray:
        """
        The distance between every pair of centres

        Parameters
        ----------
        fromGroups: List[:class:`VertexGroup`]
            The source groups (all non-empty)

        toGroups: List[:class:`VertexGroup`]
            The target groups (all non-empty)

        Returns
        -------
        :class:`numpy.ndarray`
            ``(len(fromGroups), len(toGroups))`` distances
        """

        fromCenters = np.stack([group.center for group in fromGroups], axis = 0)
        toCenters = np.stack([group.center for group in toGroups], axis = 0)
        return np.linalg.norm(fromCenters[:, None, :] - toCenters[None, :, :], axis = 2)

    @classmethod
    def gaussianDistances(cls, fromGroups: List[VertexGroup], toGroups: List[VertexGroup]) -> np.ndarray:
        """
        The 2-Wasserstein distance between Gaussians fitted to every pair of groups:
        ``W2^2 = |c1 - c2|^2 + tr(S1) + tr(S2) - 2 tr((S2^1/2 S1 S2^1/2)^1/2)``

        Parameters
        ----------
        fromGroups: List[:class:`VertexGroup`]
            The source groups (all non-empty)

        toGroups: List[:class:`VertexGroup`]
            The target groups (all non-empty)

        Returns
        -------
        :class:`numpy.ndarray`
            ``(len(fromGroups), len(toGroups))`` distances
        """

        centerTerm = cls.centerDistances(fromGroups, toGroups) ** 2

        fromSpreads = np.stack([group.spread for group in fromGroups], axis = 0)
        toSpreads = np.stack([group.spread for group in toGroups], axis = 0)

        # matrix square roots of every target spread, batched
        values, vectors = np.linalg.eigh(toSpreads)
        values = np.clip(values, 0, None)
        toRoots = np.einsum("nij,nj,nkj->nik", vectors, np.sqrt(values), vectors)

        fromTraces = np.trace(fromSpreads, axis1 = 1, axis2 = 2)
        toTraces = np.trace(toSpreads, axis1 = 1, axis2 = 2)

        # M[i, j] = R_j S_i R_j, then tr(sqrt(M)) = sum of sqrt of its eigenvalues
        inner = np.einsum("jab,ibc,jcd->ijad", toRoots, fromSpreads, toRoots)
        inner = 0.5 * (inner + np.swapaxes(inner, -1, -2))
        innerValues = np.clip(np.linalg.eigvalsh(inner), 0, None)
        crossTerm = np.sqrt(innerValues).sum(axis = 2)

        squared = centerTerm + fromTraces[:, None] + toTraces[None, :] - 2 * crossTerm
        return np.sqrt(np.clip(squared, 0, None))

    def distances(self, fromGroups: List[VertexGroup], toGroups: List[VertexGroup]) -> np.ndarray:
        """
        The distance between every pair of groups, by :attr:`metric`

        Parameters
        ----------
        fromGroups: List[:class:`VertexGroup`]
            The source groups (all non-empty)

        toGroups: List[:class:`VertexGroup`]
            The target groups (all non-empty)

        Returns
        -------
        :class:`numpy.ndarray`
            ``(len(fromGroups), len(toGroups))`` distances
        """

        if (self.metric == self.MetricCenter):
            return self.centerDistances(fromGroups, toGroups)
        return self.gaussianDistances(fromGroups, toGroups)

    # ---------------------------------------------------------------------------------------------
    # uncertainty

    @classmethod
    def ratioUncertainty(cls, bestDistance: float, runnerUpDistance: Optional[float]) -> float:
        """
        The uncertainty of an independent nearest match: the ratio of the best distance to the
        runner-up's --- near 0 when the chosen group is much closer than any other candidate, near
        1 when the runner-up is just as close and the choice is effectively a coin toss

        Parameters
        ----------
        bestDistance: :class:`float`
            The distance to the chosen group

        runnerUpDistance: Optional[:class:`float`]
            The distance to the next best candidate, or ``None`` if there is no other candidate

        Returns
        -------
        :class:`float`
            The uncertainty
        """

        if (runnerUpDistance is None):
            return 0.0
        if (runnerUpDistance <= 0):
            return 1.0
        return float(min(1.0, bestDistance / runnerUpDistance))

    @classmethod
    def marginUncertainty(cls, bestCost: float, runnerUpCost: Optional[float], spacing: float) -> float:
        """
        The uncertainty of a match made as part of a chain alignment: ``exp(-margin / spacing)``,
        where the margin is how much more the whole chain's best alignment would cost if this one
        group were forced onto its runner-up instead --- 1 when the two are tied, about 0.37 when
        the runner-up costs one bone-spacing more, near 0 when it is clearly worse

        Parameters
        ----------
        bestCost: :class:`float`
            The cost of the chain's best alignment

        runnerUpCost: Optional[:class:`float`]
            The cost of the best alignment with this group on its runner-up, or ``None`` if there
            is no other candidate

        spacing: :class:`float`
            The target mod's bone spacing

        Returns
        -------
        :class:`float`
            The uncertainty
        """

        if (runnerUpCost is None):
            return 0.0
        if (spacing <= 0):
            return 1.0 if (runnerUpCost <= bestCost) else 0.0
        return float(np.exp(-max(0.0, runnerUpCost - bestCost) / spacing))

    # ---------------------------------------------------------------------------------------------
    # matching

    def match(self) -> List[VGMatch]:
        """
        Matches every source vertex group

        Returns
        -------
        List[:class:`VGMatch`]
            One match per source vertex group, in index order. A source group with no vertices
            has no target (its :attr:`VGMatch.toGroup` is ``None``)
        """

        sources = self.fromGroups.nonEmpty
        candidates = self.toGroups.nonEmpty

        matches: Dict[int, VGMatch] = {}
        if (sources and candidates):
            if (self.mode == self.ModeVertices):
                matches = self._matchVertices(sources, candidates)
            else:
                distances = self.distances(sources, candidates)
                if (self.mode == self.ModeNearest):
                    matches = self._matchNearest(sources, candidates, distances)
                else:
                    matches = self._matchChains(sources, candidates, distances)

        result: List[VGMatch] = []
        for group in self.fromGroups:
            match = matches.get(group.index)
            if (match is None):
                match = VGMatch(group, None, float("nan"), None, float("nan"), 1.0)
            result.append(match)

        self._markChains(result)
        return result

    def _matchNearest(self, sources: List[VertexGroup], candidates: List[VertexGroup], distances: np.ndarray) -> Dict[int, VGMatch]:
        result: Dict[int, VGMatch] = {}
        for row, fromGroup in enumerate(sources):
            order = np.argsort(distances[row], kind = "stable")
            best = candidates[order[0]]
            bestDistance = float(distances[row, order[0]])

            runnerUp: Optional[VertexGroup] = None
            runnerUpDistance: Optional[float] = None
            if (len(order) > 1):
                runnerUp = candidates[order[1]]
                runnerUpDistance = float(distances[row, order[1]])

            uncertainty = self.ratioUncertainty(bestDistance, runnerUpDistance)
            result[fromGroup.index] = VGMatch(fromGroup, best, bestDistance, runnerUp,
                                              float("nan") if (runnerUpDistance is None) else runnerUpDistance, uncertainty)
        return result

    @classmethod
    def nearestVertices(cls, points: np.ndarray, targets: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """
        The nearest of ``targets`` to every point

        Parameters
        ----------
        points: :class:`numpy.ndarray`
            ``(n, 3)`` query points

        targets: :class:`numpy.ndarray`
            ``(m, 3)`` points to search

        Returns
        -------
        Tuple[:class:`numpy.ndarray`, :class:`numpy.ndarray`]
            The distance to, and the index of, the nearest target of every point
        """

        try:
            from scipy.spatial import cKDTree
        except ImportError:
            cKDTree = None

        if (cKDTree is not None):
            distances, indices = cKDTree(targets).query(points)
            return np.asarray(distances, dtype = np.float64), np.asarray(indices, dtype = np.int64)

        # no scipy: brute force in chunks small enough to keep the distance block in memory
        distances = np.empty(len(points), dtype = np.float64)
        indices = np.empty(len(points), dtype = np.int64)
        chunk = max(1, 20_000_000 // max(1, len(targets)))
        for start in range(0, len(points), chunk):
            block = np.linalg.norm(points[start:start + chunk, None, :] - targets[None, :, :], axis = 2)
            indices[start:start + chunk] = block.argmin(axis = 1)
            distances[start:start + chunk] = block[np.arange(block.shape[0]), indices[start:start + chunk]]
        return distances, indices

    def _matchVertices(self, sources: List[VertexGroup], candidates: List[VertexGroup]) -> Dict[int, VGMatch]:
        fromMod = self.fromGroups.mod
        toMod = self.toGroups.mod
        targetCount = len(self.toGroups)

        fromUsed = fromMod.blendWeights > 0
        toUsed = toMod.blendWeights > 0
        toIndices = np.where(toUsed, toMod.blendIndices, -1)
        toWeights = np.where(toUsed, toMod.blendWeights, 0.0)

        # one nearest-neighbour query for every source vertex that any group drives, over the
        #   whole target skin. (Restricting the search to the same drawn object -- Body against
        #   Body -- was tried and scored far worse: the Head/Body/Dress split is arbitrary and does
        #   not correspond between two skins of one character.)
        driven = np.nonzero(fromUsed.any(axis = 1))[0]
        nearestDistance = np.full(fromMod.vertexCount, np.nan)
        nearestIndex = np.full(fromMod.vertexCount, -1, dtype = np.int64)
        if (len(driven)):
            nearestDistance[driven], nearestIndex[driven] = self.nearestVertices(fromMod.positions[driven], toMod.positions)

        byIndex = {group.index: group for group in candidates}
        result: Dict[int, VGMatch] = {}
        for fromGroup in sources:
            membership = (fromMod.blendIndices == fromGroup.index) & fromUsed
            rows = np.nonzero(membership.any(axis = 1))[0]
            if (not len(rows)):
                continue

            sourceWeights = (fromMod.blendWeights[rows] * membership[rows]).sum(axis = 1)
            nearest = nearestIndex[rows]
            tallyIndices = toIndices[nearest]                                # (rows, 4)
            tallyWeights = toWeights[nearest] * sourceWeights[:, None]
            valid = tallyIndices >= 0
            shares = np.bincount(tallyIndices[valid], weights = tallyWeights[valid], minlength = targetCount)
            total = shares.sum()
            if (total <= 0):
                continue
            shares = shares / total

            order = np.argsort(-shares, kind = "stable")
            best = byIndex.get(int(order[0]))
            if (best is None):
                continue

            runnerUp: Optional[VertexGroup] = None
            runnerUpShare: Optional[float] = None
            if (len(order) > 1 and shares[order[1]] > 0):
                runnerUp = byIndex.get(int(order[1]))
                runnerUpShare = float(shares[order[1]])

            bestShare = float(shares[order[0]])
            uncertainty = 0.0 if (runnerUpShare is None) else float(min(1.0, runnerUpShare / bestShare))
            match = VGMatch(fromGroup, best, float(np.nanmean(nearestDistance[rows])), runnerUp, float("nan"), uncertainty)
            match.share = bestShare
            match.runnerUpShare = runnerUpShare
            result[fromGroup.index] = match

        return result

    def _transitionCosts(self, candidates: List[VertexGroup], spacing: float) -> np.ndarray:
        """
        ``T[k, l]``: the cost of the next source group mapping onto ``candidates[l]`` when the
        previous one mapped onto ``candidates[k]``
        """

        indices = np.array([group.index for group in candidates])
        delta = indices[None, :] - indices[:, None]

        result = np.full((len(candidates), len(candidates)), self.jumpCost * spacing)
        result[delta == 1] = 0.0
        result[delta == 2] = self.skipCost * spacing
        result[delta == 0] = self.stayCost * spacing
        return result

    def _matchChains(self, sources: List[VertexGroup], candidates: List[VertexGroup], distances: np.ndarray) -> Dict[int, VGMatch]:
        spacing = self.toGroups.spacing
        transitions = self._transitionCosts(candidates, spacing)
        rowOf = {group.index: row for row, group in enumerate(sources)}

        result: Dict[int, VGMatch] = {}
        for chain in self.fromGroups.chains():
            rows = [rowOf[index] for index in chain]
            unary = distances[rows]                                     # (chainLen, candidates)
            length = len(rows)

            # forward and backward min-sum passes, so every group gets the cost of the best whole
            # alignment passing through each candidate (its min-marginals)
            forward = np.empty_like(unary)
            forward[0] = unary[0]
            for step in range(1, length):
                forward[step] = (forward[step - 1][:, None] + transitions).min(axis = 0) + unary[step]

            backward = np.zeros_like(unary)
            for step in range(length - 2, -1, -1):
                backward[step] = (transitions + (unary[step + 1] + backward[step + 1])[None, :]).min(axis = 1)

            marginals = forward + backward

            for step, index in enumerate(chain):
                order = np.argsort(marginals[step], kind = "stable")
                best = candidates[order[0]]
                bestCost = float(marginals[step, order[0]])
                bestDistance = float(unary[step, order[0]])

                runnerUp: Optional[VertexGroup] = None
                runnerUpCost: Optional[float] = None
                runnerUpDistance = float("nan")
                if (len(order) > 1):
                    runnerUp = candidates[order[1]]
                    runnerUpCost = float(marginals[step, order[1]])
                    runnerUpDistance = float(unary[step, order[1]])

                uncertainty = self.marginUncertainty(bestCost, runnerUpCost, spacing)
                result[index] = VGMatch(sources[rows[step]], best, bestDistance, runnerUp, runnerUpDistance, uncertainty)

        return result

    @classmethod
    def _markChains(cls, matches: List[VGMatch]):
        """
        Records, on every match that is part of one, the run of consecutive source indices that
        landed on consecutive target indices
        """

        run: List[VGMatch] = []

        def flush():
            if (len(run) > 1):
                chain = (run[0].fromIndex, run[-1].fromIndex, run[0].toIndex, run[-1].toIndex)
                for match in run:
                    match.chain = chain
            run.clear()

        previous: Optional[VGMatch] = None
        for match in matches:
            continues = (previous is not None and match.toIndex is not None and previous.toIndex is not None
                         and match.fromIndex == previous.fromIndex + 1 and match.toIndex == previous.toIndex + 1)
            if (not continues):
                flush()
            run.append(match)
            previous = match

        flush()
