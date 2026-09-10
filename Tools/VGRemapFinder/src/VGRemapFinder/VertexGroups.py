from typing import Dict, List, Set

import numpy as np

from .DumpMod import DumpMod


class VertexGroup():
    """
    One vertex group of a mod, summarised by the vertices it owns

    Parameters
    ----------
    index: :class:`int`
        The vertex group's index

    vertexCount: :class:`int`
        How many vertices carry this group with a non-zero weight

    center: :class:`numpy.ndarray`
        The centre point (mean position) of those vertices, in the dump's own axis order.
        Undefined (all ``nan``) when :attr:`vertexCount` is 0

    spread: :class:`numpy.ndarray`
        The ``3 x 3`` covariance of those vertices' positions about :attr:`center` --- how far,
        and in which directions, the group's vertices extend. All zero when the group is empty

    objects: Set[:class:`str`]
        The drawn objects (``"Head"``, ``"Body"``, ...) any of those vertices belong to

    Attributes
    ----------
    index: :class:`int`
        The vertex group's index

    vertexCount: :class:`int`
        How many vertices carry this group with a non-zero weight

    center: :class:`numpy.ndarray`
        The centre point of the vertices

    spread: :class:`numpy.ndarray`
        The covariance of the vertices' positions

    objects: Set[:class:`str`]
        The drawn objects the vertices belong to
    """

    def __init__(self, index: int, vertexCount: int, center: np.ndarray, spread: np.ndarray, objects: Set[str]):
        self.index = index
        self.vertexCount = vertexCount
        self.center = center
        self.spread = spread
        self.objects = objects

    @property
    def isEmpty(self) -> bool:
        """
        Whether no vertex carries this group

        :getter: Retrieves whether the group is empty
        :type: :class:`bool`
        """

        return self.vertexCount == 0

    @property
    def extent(self) -> np.ndarray:
        """
        The standard deviation of the vertices along each of the spread's principal axes, largest
        first --- a rough "size" of the group in each direction

        :getter: Retrieves the extents
        :type: :class:`numpy.ndarray`
        """

        return np.sqrt(np.clip(np.linalg.eigvalsh(self.spread), 0, None))[::-1]

    def __repr__(self) -> str:
        return f"VertexGroup({self.index}, vertices = {self.vertexCount}, center = {self.center}, objects = {sorted(self.objects)})"


class VertexGroups():
    """
    Every vertex group of a mod

    Parameters
    ----------
    mod: :class:`DumpMod`
        The mod to summarise

    weighted: :class:`bool`
        Whether a group's centre and spread are blend-weight-weighted (the default), rather than
        treating every vertex the group touches equally. With the plain mean a vertex counts fully
        towards every group it carries at all, so a group that only lightly influences a wide area
        gets its centre pulled towards that area --- measured against the hand-made drafts, the
        weighted summary is the more accurate one by a clear margin

    Attributes
    ----------
    mod: :class:`DumpMod`
        The mod summarised

    weighted: :class:`bool`
        Whether the centres are blend-weight-weighted

    groups: List[:class:`VertexGroup`]
        The vertex groups, indexed by their vertex group index (so ``groups[i].index == i``)
    """

    def __init__(self, mod: DumpMod, weighted: bool = True):
        self.mod = mod
        self.weighted = weighted
        self.groups: List[VertexGroup] = self._build()

    def __len__(self) -> int:
        return len(self.groups)

    def __iter__(self):
        return iter(self.groups)

    def __getitem__(self, index: int) -> VertexGroup:
        return self.groups[index]

    @property
    def nonEmpty(self) -> List[VertexGroup]:
        """
        The vertex groups that own at least one vertex

        :getter: Retrieves the non-empty groups
        :type: List[:class:`VertexGroup`]
        """

        return [group for group in self.groups if (not group.isEmpty)]

    @property
    def spacing(self) -> float:
        """
        The typical distance between a vertex group's centre and its nearest neighbour's --- the
        scale on which "close" and "far" mean anything for this mod

        :getter: Retrieves the spacing (0 if there are fewer than 2 non-empty groups)
        :type: :class:`float`
        """

        groups = self.nonEmpty
        if (len(groups) < 2):
            return 0.0

        centers = np.stack([group.center for group in groups], axis = 0)
        distances = np.linalg.norm(centers[:, None, :] - centers[None, :, :], axis = 2)
        np.fill_diagonal(distances, np.inf)
        return float(np.median(distances.min(axis = 1)))

    def chains(self) -> List[List[int]]:
        """
        The runs of consecutive vertex group indices with no empty group in between --- the game
        numbers a skeleton's bones in order, so a chain of bones (hair, a ribbon, a skirt's rows) is
        a run of consecutive indices

        Returns
        -------
        List[List[:class:`int`]]
            The runs, each a list of consecutive vertex group indices
        """

        result: List[List[int]] = []
        current: List[int] = []
        for group in self.groups:
            if (group.isEmpty):
                if (current):
                    result.append(current)
                    current = []
                continue
            current.append(group.index)

        if (current):
            result.append(current)
        return result

    def _build(self) -> List[VertexGroup]:
        mod = self.mod
        groupCount = mod.vertexGroupCount
        if (groupCount == 0):
            return []

        # every (vertex, slot) pair with a non-zero weight is one membership
        weights = mod.blendWeights
        indices = mod.blendIndices
        used = weights > 0

        memberGroups = indices[used]
        memberWeights = weights[used]
        memberVertices = np.nonzero(used)[0]
        memberPositions = mod.positions[memberVertices]

        counts = np.bincount(memberGroups, minlength = groupCount)

        if (self.weighted):
            contribution = memberWeights
        else:
            contribution = np.ones_like(memberWeights)

        totals = np.bincount(memberGroups, weights = contribution, minlength = groupCount)
        nonZero = totals > 0

        centers = np.full((groupCount, 3), np.nan, dtype = np.float64)
        for axis in range(3):
            sums = np.bincount(memberGroups, weights = memberPositions[:, axis] * contribution, minlength = groupCount)
            centers[nonZero, axis] = sums[nonZero] / totals[nonZero]

        # covariance about the centre, per group
        offsets = memberPositions - np.where(np.isnan(centers[memberGroups]), 0, centers[memberGroups])
        spreads = np.zeros((groupCount, 3, 3), dtype = np.float64)
        for row in range(3):
            for col in range(row, 3):
                sums = np.bincount(memberGroups, weights = offsets[:, row] * offsets[:, col] * contribution, minlength = groupCount)
                values = np.zeros(groupCount)
                values[nonZero] = sums[nonZero] / totals[nonZero]
                spreads[:, row, col] = values
                spreads[:, col, row] = values

        # which drawn objects each group appears in
        objectsPerGroup: Dict[int, Set[str]] = {i: set() for i in range(groupCount)}
        for objectName, vertexIndices in mod.objects.items():
            inObject = np.zeros(mod.vertexCount, dtype = bool)
            inObject[vertexIndices] = True
            groupsInObject = np.unique(memberGroups[inObject[memberVertices]])
            for group in groupsInObject.tolist():
                objectsPerGroup[group].add(objectName)

        return [VertexGroup(i, int(counts[i]), centers[i], spreads[i], objectsPerGroup[i]) for i in range(groupCount)]
