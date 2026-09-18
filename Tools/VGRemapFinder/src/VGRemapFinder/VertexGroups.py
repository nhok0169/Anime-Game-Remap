from typing import Dict, List, Optional, Set, Tuple, Union

import numpy as np

from .DumpMod import Character, DumpMod


class VertexGroup():
    """
    One vertex group of a character, summarised by the vertices it owns

    Parameters
    ----------
    index: :class:`int`
        The vertex group's index within its component

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

    component: :class:`str`
        The component the group belongs to (``""`` for a single-component character). Two
        components' index spaces are unrelated: ``Body 64`` and ``Bang 64`` are different bones

    Attributes
    ----------
    index: :class:`int`
        The vertex group's index within its component

    vertexCount: :class:`int`
        How many vertices carry this group with a non-zero weight

    center: :class:`numpy.ndarray`
        The centre point of the vertices

    spread: :class:`numpy.ndarray`
        The covariance of the vertices' positions

    objects: Set[:class:`str`]
        The drawn objects the vertices belong to

    component: :class:`str`
        The component the group belongs to
    """

    def __init__(self, index: int, vertexCount: int, center: np.ndarray, spread: np.ndarray, objects: Set[str], component: str = ""):
        self.index = index
        self.vertexCount = vertexCount
        self.center = center
        self.spread = spread
        self.objects = objects
        self.component = component

    @property
    def key(self) -> Tuple[str, int]:
        """
        ``(component, index)`` --- what identifies the group within its character

        :getter: Retrieves the key
        :type: Tuple[:class:`str`, :class:`int`]
        """

        return (self.component, self.index)

    @property
    def label(self) -> str:
        """
        The group as a draft or a summary names it: ``"64"`` in a single-component character,
        ``"Body:64"`` otherwise

        :getter: Retrieves the label
        :type: :class:`str`
        """

        return f"{self.component}:{self.index}" if (self.component) else str(self.index)

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
        return f"VertexGroup({self.label}, vertices = {self.vertexCount}, center = {self.center}, objects = {sorted(self.objects)})"


class VertexGroups():
    """
    Every vertex group of a character, across all of its components

    Parameters
    ----------
    source: Union[:class:`Character`, :class:`DumpMod`]
        The character to summarise (a lone :class:`DumpMod` is taken as a character of that one
        component)

    weighted: :class:`bool`
        Whether a group's centre and spread are blend-weight-weighted (the default), rather than
        treating every vertex the group touches equally. With the plain mean a vertex counts fully
        towards every group it carries at all, so a group that only lightly influences a wide area
        gets its centre pulled towards that area --- measured against the hand-made drafts, the
        weighted summary is the more accurate one by a clear margin

    Attributes
    ----------
    character: :class:`Character`
        The character summarised

    mod: Optional[:class:`DumpMod`]
        The one component of a single-component character, else ``None``

    weighted: :class:`bool`
        Whether the centres are blend-weight-weighted

    byComponent: Dict[:class:`str`, List[:class:`VertexGroup`]]
        Each component's vertex groups, indexed by their index within the component (so
        ``byComponent[c][i].index == i``), in the character's component order

    groups: List[:class:`VertexGroup`]
        Every vertex group, component by component then by index --- the order the "global"
        indices used inside the matcher follow
    """

    def __init__(self, source: Union[Character, DumpMod], weighted: bool = True):
        if (isinstance(source, DumpMod)):
            source = Character(source.name, {source.component: source})

        self.character = source
        self.mod: Optional[DumpMod] = source.single() if (len(source.components) == 1) else None
        self.weighted = weighted

        self.byComponent: Dict[str, List[VertexGroup]] = {}
        for componentName, mod in source.components.items():
            self.byComponent[componentName] = self._build(mod, componentName, weighted)
        self.groups: List[VertexGroup] = [group for groups in self.byComponent.values() for group in groups]

    @property
    def name(self) -> str:
        """
        The character's name

        :getter: Retrieves the name
        :type: :class:`str`
        """

        return self.character.name

    @property
    def componentNames(self) -> List[str]:
        """
        The component names in draw order

        :getter: Retrieves the names
        :type: List[:class:`str`]
        """

        return list(self.byComponent)

    @property
    def isMultiComponent(self) -> bool:
        """
        Whether the character has more than one component

        :getter: Retrieves whether the character is multi-component
        :type: :class:`bool`
        """

        return self.character.isMultiComponent

    def __len__(self) -> int:
        return len(self.groups)

    def __iter__(self):
        return iter(self.groups)

    def __getitem__(self, key: Union[int, Tuple[str, int]]) -> VertexGroup:
        if (isinstance(key, tuple)):
            return self.byComponent[key[0]][key[1]]
        if (len(self.byComponent) != 1):
            raise TypeError(f"'{self.name}' has several components; index its groups by (component, index)")
        return next(iter(self.byComponent.values()))[key]

    def get(self, component: str, index: int) -> VertexGroup:
        """
        One vertex group

        Parameters
        ----------
        component: :class:`str`
            The component

        index: :class:`int`
            The index within the component

        Returns
        -------
        :class:`VertexGroup`
            The group
        """

        return self.byComponent[component][index]

    def count(self, component: str) -> int:
        """
        How many vertex groups a component has (one past its largest index in use)

        Parameters
        ----------
        component: :class:`str`
            The component

        Returns
        -------
        :class:`int`
            The count
        """

        return len(self.byComponent[component])

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
        scale on which "close" and "far" mean anything for this character

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

    def chains(self) -> List[List[VertexGroup]]:
        """
        The runs of consecutive vertex group indices with no empty group in between, within each
        component --- the game numbers a skeleton's bones in order, so a chain of bones (hair, a
        ribbon, a skirt's rows) is a run of consecutive indices

        Returns
        -------
        List[List[:class:`VertexGroup`]]
            The runs, each a list of consecutive groups of one component
        """

        result: List[List[VertexGroup]] = []
        for groups in self.byComponent.values():
            current: List[VertexGroup] = []
            for group in groups:
                if (group.isEmpty):
                    if (current):
                        result.append(current)
                        current = []
                    continue
                current.append(group)

            if (current):
                result.append(current)
        return result

    @classmethod
    def _build(cls, mod: DumpMod, component: str, weighted: bool = True) -> List[VertexGroup]:
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

        if (weighted):
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

        return [VertexGroup(i, int(counts[i]), centers[i], spreads[i], objectsPerGroup[i], component = component) for i in range(groupCount)]
