"""
Splitting a single-component mod across a target skin of several components :raw-html:`<br />` :raw-html:`<br />`

A mod made for Yelan is one ``Position.buf`` / ``Blend.buf`` / ``Texcoord.buf`` and one ``.ib``
per object. YelanTranquil draws a ``Body``, a ``Bang`` and an ``Eye``, each from its own buffers,
each with its own bone numbering. So remapping the mod onto her is not one blend remap but a
*split*: every target component needs buffers holding the part of the mod that belongs to it,
skinned with that component's bones. Two strategies, both from the maintainer's issue #190 notes:

**Negative index** (:func:`negativeIndexSplit`) --- feed the whole mod to every component, and
give each component a ``RemapBlend.buf`` remapped with only *its* rows of the vertex group remap.
A source bone that belongs to another component becomes the negative sentinel ``-index-1``, which
the game's skinning turns into a garbage transform, so the vertices that are not this component's
collapse. Nothing is filtered, no index buffer is rewritten; the price is that a triangle
straddling two components stretches between a kept vertex and a collapsed one.

**Graph cut** (:func:`graphCutSplit`) --- per component, keep the triangles whose vertices all
belong to it (``strict``: every non-zero bone of every vertex is in the component's rows), collect
the vertices those triangles reference, filter every vertex buffer to them (an ``.ib`` index is a
line number in every ``.buf``, so one vertex set filters all of them), renumber the index
buffers, and remap the blend. ``majority`` assigns each vertex to the component holding most of
its weight and each triangle to the component most of its vertices went to, so every triangle
is drawn exactly once and the seams close; a vertex a triangle drags into a component it has no
bone in is skinned to the bone its triangle neighbours use there.
"""

import os
import re
from collections import Counter
from typing import Dict, List, Optional, Sequence, Tuple

import numpy as np

from .DraftWriter import DraftWriter
from .DumpMod import DumpMod, importAPI


class ModBuffers():
    """
    A single-component mod's raw buffers, as the game consumes them

    Parameters
    ----------
    folder: :class:`str`
        The mod folder (not searched recursively)

    prefix: Optional[:class:`str`]
        The files' shared prefix (``"yelan"`` for ``yelanPosition.buf``). ``None`` takes the
        one the folder's ``*Position.buf`` has

    objectOrder: Optional[Sequence[:class:`str`]]
        The mod's objects in the order its ``.ini`` draws them, which is the order they are
        handed to the target's draw slots. ``None`` is :attr:`DefaultObjectOrder` first, then
        whatever else the folder has alphabetically

    texcoordPath: Optional[:class:`str`]
        Read this ``Texcoord.buf`` (an edited copy) instead of ``<prefix>Texcoord.buf``

    Attributes
    ----------
    folder: :class:`str`
        The mod folder

    prefix: :class:`str`
        The files' shared prefix

    vertexCount: :class:`int`
        The number of vertices (blend lines)

    position: :class:`numpy.ndarray`
        The ``Position.buf`` bytes as ``(vertexCount, stride)`` uint8 rows

    texcoord: :class:`numpy.ndarray`
        The ``Texcoord.buf`` bytes as ``(vertexCount, stride)`` uint8 rows

    blendWeights: :class:`numpy.ndarray`
        ``(vertexCount, 4)`` float32

    blendIndices: :class:`numpy.ndarray`
        ``(vertexCount, 4)`` int32

    ibs: Dict[:class:`str`, :class:`numpy.ndarray`]
        Per object name (``Head``, ``Body``, ...), the ``.ib`` as ``(triangles, 3)`` uint32 --- an
        empty ``.ib`` is an empty array

    paths: Dict[:class:`str`, :class:`str`]
        The files read, keyed ``position`` / ``blend`` / ``texcoord`` / ``ib:<object>``
    """

    BlendStride = 32
    IbDtype = np.dtype("<u4")
    DefaultObjectOrder = ("Head", "Body", "Dress", "Extra")

    def __init__(self, folder: str, prefix: Optional[str] = None, objectOrder: Optional[Sequence[str]] = None, texcoordPath: Optional[str] = None):
        folder = os.path.abspath(folder)
        pairs, ibFiles = DumpMod.findModComponentFiles(folder)
        if (not pairs):
            raise FileNotFoundError(f"No '*Position.buf' + '*Blend.buf' pair in '{folder}'")
        if (prefix is None):
            prefix = sorted(pairs)[0]
        if (prefix not in pairs):
            raise FileNotFoundError(f"No '{prefix}Position.buf' + '{prefix}Blend.buf' in '{folder}' (found: {', '.join(sorted(pairs))})")

        self.folder = folder
        self.prefix = prefix
        positionPath, blendPath = pairs[prefix]
        if (texcoordPath is not None):
            texcoordPath = texcoordPath if (os.path.isabs(texcoordPath)) else os.path.join(folder, texcoordPath)
            if (not os.path.isfile(texcoordPath)):
                raise FileNotFoundError(f"texcoord override '{texcoordPath}' does not exist")
        else:
            for fileName in os.listdir(folder):
                if (fileName.lower() == f"{prefix}texcoord.buf".lower()):
                    texcoordPath = os.path.join(folder, fileName)
        if (texcoordPath is None):
            raise FileNotFoundError(f"No '{prefix}Texcoord.buf' in '{folder}'")

        self.paths = {"position": positionPath, "blend": blendPath, "texcoord": texcoordPath}

        blendRaw = np.frombuffer(open(blendPath, "rb").read(), dtype = np.uint8)
        if (len(blendRaw) % self.BlendStride):
            raise ValueError(f"'{blendPath}' is not a whole number of {self.BlendStride}-byte lines")
        self.vertexCount = len(blendRaw) // self.BlendStride
        blendRows = blendRaw.reshape(self.vertexCount, self.BlendStride)
        self.blendWeights = blendRows[:, :16].copy().view("<f4").reshape(self.vertexCount, 4)
        self.blendIndices = blendRows[:, 16:].copy().view("<i4").reshape(self.vertexCount, 4)

        self.position = self._rows(positionPath)
        self.texcoord = self._rows(texcoordPath)

        found: Dict[str, np.ndarray] = {}
        for stem, ibPath in sorted(ibFiles.items()):
            objectName = DumpMod.objectNameFromStem(stem, prefix)
            raw = np.frombuffer(open(ibPath, "rb").read(), dtype = self.IbDtype)
            if (len(raw) % 3):
                raise ValueError(f"'{ibPath}' is not a whole number of triangles")
            found[objectName] = raw.reshape(-1, 3).copy()
            self.paths[f"ib:{objectName}"] = ibPath

        order = list(objectOrder) if (objectOrder is not None) else list(self.DefaultObjectOrder)
        byLower = {name.lower(): name for name in found}
        ordered = [byLower[name.lower()] for name in order if (name.lower() in byLower)]
        ordered += [name for name in found if (name not in ordered)]
        self.ibs: Dict[str, np.ndarray] = {name: found[name] for name in ordered}

    @classmethod
    def fromFiles(cls, positionPath: str, blendPath: str, texcoordPath: str, ibPaths: Dict[str, str], folder: Optional[str] = None,
                  prefix: str = "") -> "ModBuffers":
        """
        A mod's buffers from explicit paths --- the ones its ``.ini`` names --- rather than from the
        ``<prefix>Position.buf`` naming convention

        Parameters
        ----------
        positionPath: :class:`str`
            The ``Position.buf``

        blendPath: :class:`str`
            The ``Blend.buf``

        texcoordPath: :class:`str`
            The ``Texcoord.buf``

        ibPaths: Dict[:class:`str`, :class:`str`]
            Per mod object, in draw order, its ``.ib`` (a missing or empty file is an object with no triangles)

        folder: Optional[:class:`str`]
            The mod folder the ``.ini`` paths are relative to (default: the position file's folder)

        prefix: :class:`str`
            A name for the output files

        Returns
        -------
        :class:`ModBuffers`
            The buffers
        """

        self = cls.__new__(cls)
        self.folder = os.path.abspath(folder or os.path.dirname(positionPath))
        self.prefix = prefix
        self.paths = {"position": positionPath, "blend": blendPath, "texcoord": texcoordPath}
        blendRaw = np.frombuffer(open(blendPath, "rb").read(), dtype = np.uint8)
        if (len(blendRaw) % cls.BlendStride):
            raise ValueError(f"'{blendPath}' is not a whole number of {cls.BlendStride}-byte lines")
        self.vertexCount = len(blendRaw) // cls.BlendStride
        blendRows = blendRaw.reshape(self.vertexCount, cls.BlendStride)
        self.blendWeights = blendRows[:, :16].copy().view("<f4").reshape(self.vertexCount, 4)
        self.blendIndices = blendRows[:, 16:].copy().view("<i4").reshape(self.vertexCount, 4)
        self.position = self._rows(positionPath)
        self.texcoord = self._rows(texcoordPath)
        self.ibs = {}
        for objectName, ibPath in ibPaths.items():
            if (ibPath and os.path.isfile(ibPath) and os.path.getsize(ibPath)):
                raw = np.frombuffer(open(ibPath, "rb").read(), dtype = cls.IbDtype)
                if (len(raw) % 3):
                    raise ValueError(f"'{ibPath}' is not a whole number of triangles")
                self.ibs[objectName] = raw.reshape(-1, 3).copy()
                self.paths[f"ib:{objectName}"] = ibPath
            else:
                self.ibs[objectName] = np.zeros((0, 3), dtype = np.uint32)
        return self

    def _rows(self, path: str) -> np.ndarray:
        raw = np.frombuffer(open(path, "rb").read(), dtype = np.uint8)
        if (len(raw) % self.vertexCount):
            raise ValueError(f"'{path}' ({len(raw)} bytes) is not {self.vertexCount} equal lines")
        return raw.reshape(self.vertexCount, len(raw) // self.vertexCount).copy()      # writable: a caller may edit the vertex colour / UVs

    @property
    def positionStride(self) -> int:
        return int(self.position.shape[1])

    @property
    def texcoordStride(self) -> int:
        return int(self.texcoord.shape[1])

    @property
    def objectNames(self) -> List[str]:
        return list(self.ibs)

    @classmethod
    def encodeBlend(cls, weights: np.ndarray, indices: np.ndarray) -> bytes:
        """
        ``(n, 4)`` weights and indices back into ``Blend.buf`` bytes (float32 x4, int32 x4 per line)

        Parameters
        ----------
        weights: :class:`numpy.ndarray`
            The weights

        indices: :class:`numpy.ndarray`
            The bone indices

        Returns
        -------
        :class:`bytes`
            The file's bytes
        """

        rows = np.concatenate([np.ascontiguousarray(weights, dtype = "<f4").view(np.uint8).reshape(len(weights), 16),
                               np.ascontiguousarray(indices, dtype = "<i4").view(np.uint8).reshape(len(indices), 16)], axis = 1)
        return rows.tobytes()


# ------------------------------------------------------------------------------------------------
# the vertex group remap, per target component


def remapsFromDraft(path: str, fromName: str, toName: str, sheetTitle: Optional[str] = None) -> Dict[str, Dict[int, int]]:
    """
    The per-component remap out of a drafts-format sheet whose target columns are one per
    component (``YelanTranquilBody | YelanTranquilBang | YelanTranquilEye``)

    Parameters
    ----------
    path: :class:`str`
        The workbook

    fromName: :class:`str`
        Column A's header

    toName: :class:`str`
        The target's name (the prefix of the target columns' headers)

    sheetTitle: Optional[:class:`str`]
        Read this sheet rather than the first whose header matches

    Returns
    -------
    Dict[:class:`str`, Dict[:class:`int`, :class:`int`]]
        Per target component, source index to target index
    """

    import openpyxl

    workbook = openpyxl.load_workbook(path, read_only = True, data_only = True)
    sheets = [workbook[sheetTitle]] if (sheetTitle) else list(workbook.worksheets)
    for sheet in sheets:
        rows = sheet.iter_rows(min_row = 1, values_only = True)
        header = next(rows, None)
        parsed = DraftWriter.parseHeader(header, fromName, toName)
        if (parsed is None):
            continue
        _, targets = parsed
        result: Dict[str, Dict[int, int]] = {component: {} for _, component in targets}
        for row in rows:
            if (not row or row[0] is None or str(row[0]).strip() == ""):
                continue
            try:
                source = int(row[0])
            except (TypeError, ValueError):
                continue
            for column, component in targets:
                if (column < len(row) and row[column] is not None and str(row[column]).strip() != ""):
                    result[component][source] = int(row[column])
                    break
        return result

    raise KeyError(f"'{path}' has no sheet headed '{fromName}' | '{toName}...'" + (f" named {sheetTitle!r}" if sheetTitle else ""))


def remapsFromLibrary(fromName: str, toName: str, components: Sequence[str], fromComponent: str = "") -> Dict[str, Dict[int, int]]:
    """
    The per-component remap out of the library's shared table (needs a build that has the rows)

    Parameters
    ----------
    fromName: :class:`str`
        The library's name of the source

    toName: :class:`str`
        The library's name of the target

    components: Sequence[:class:`str`]
        The target's components

    fromComponent: :class:`str`
        The source component the rows are keyed by

    Returns
    -------
    Dict[:class:`str`, Dict[:class:`int`, :class:`int`]]
        Per target component, source index to target index
    """

    FRB = importAPI()
    FRB.CppGlobalModTypes.registerAll()
    table = FRB.CppGlobalModTypes.all()[0].vgRemaps
    result = {}
    for component in components:
        remap = table.get([fromName, fromComponent, toName, component], errorOnNotFound = False)
        if (remap is None):
            raise KeyError(f"the library has no row {fromName}/{fromComponent!r} -> {toName}/{component!r}")
        result[component] = {int(k): int(v) for k, v in dict(remap.remap).items()}
    return result


def augmentFromReverse(remaps: Dict[str, Dict[int, int]], path: str, fromName: str, toName: str,
                       components: Sequence[str]) -> Dict[str, Dict[int, Tuple[int, float]]]:
    """
    Adds to a component's remap every source group that one of the component's OWN bones
    corresponds to, read off the reverse sheets (``<toName><Component> | <fromName> | Uncertainty``)
    --- for the negative-index strategy, where a vertex needs **every** bone it carries to exist in
    the component. YelanTranquil's ``Bang 0`` is Yelan's head (``64``), which the forward rows send
    to ``Body:13`` only; without this a hair vertex weighted head + bang gets a sentinel on the head
    and collapses in the Bang draw

    Parameters
    ----------
    remaps: Dict[:class:`str`, Dict[:class:`int`, :class:`int`]]
        Per target component, source index to target index (not edited: the result is the
        **secondary** remap :func:`negativeIndexSplit` honours only on vertices that also carry
        one of the component's own bones, so a vertex weighted to the head alone -- the whole
        face -- is not drawn a second time by the Bang)

    path: :class:`str`
        The workbook holding the reverse sheets

    fromName: :class:`str`
        The source's name

    toName: :class:`str`
        The target's name

    components: Sequence[:class:`str`]
        The components to augment

    Returns
    -------
    Dict[:class:`str`, Dict[:class:`int`, Tuple[:class:`int`, :class:`float`]]]
        Per component, the source groups added as ``(target bone, uncertainty)``; when several of
        the component's bones name the same source group, the least uncertain wins
    """

    import openpyxl

    workbook = openpyxl.load_workbook(path, read_only = True, data_only = True)
    added: Dict[str, Dict[int, Tuple[int, float]]] = {}
    for component in components:
        remap = remaps.get(component, {})
        candidates: Dict[int, Tuple[int, float]] = {}
        for sheet in workbook.worksheets:
            rows = sheet.iter_rows(min_row = 1, values_only = True)
            header = next(rows, None)
            parsed = DraftWriter.parseHeader(header, f"{toName}{component}", fromName)
            if (parsed is None):
                continue
            uncertaintyColumn, targets = parsed
            column = targets[0][0]
            for row in rows:
                if (not row or row[0] is None or column >= len(row) or row[column] is None or str(row[column]).strip() == ""):
                    continue
                try:
                    bone, source = int(row[0]), int(row[column])
                except (TypeError, ValueError):
                    continue
                try:
                    uncertainty = float(row[uncertaintyColumn]) if (uncertaintyColumn < len(row) and row[uncertaintyColumn] is not None) else 1.0
                except (TypeError, ValueError):
                    uncertainty = 1.0
                if (source not in remap and (source not in candidates or uncertainty < candidates[source][1])):
                    candidates[source] = (bone, uncertainty)
            break
        added[component] = candidates
    return added


def checkRemaps(remaps: Dict[str, Dict[int, int]], groupCount: int) -> List[str]:
    """
    The problems a per-component remap has: a source group in no component, or in several

    Parameters
    ----------
    remaps: Dict[:class:`str`, Dict[:class:`int`, :class:`int`]]
        Per target component, source index to target index

    groupCount: :class:`int`
        How many source groups there are

    Returns
    -------
    List[:class:`str`]
        The problems, empty when there are none
    """

    owners: Dict[int, List[str]] = {}
    for component, remap in remaps.items():
        for source in remap:
            owners.setdefault(source, []).append(component)
    problems = []
    missing = [i for i in range(groupCount) if i not in owners]
    if (missing):
        problems.append(f"source groups in no component: {missing}")
    duplicates = {i: c for i, c in owners.items() if len(c) > 1}
    if (duplicates):
        problems.append(f"source groups in several components: {duplicates}")
    return problems


# ------------------------------------------------------------------------------------------------
# the two splits


class ComponentResult():
    """
    What one target component gets out of a split

    Attributes
    ----------
    component: :class:`str`
        The target component

    vertices: :class:`numpy.ndarray`
        The mod's vertex indices this component draws, sorted (all of them for the negative-index
        split)

    blendWeights: :class:`numpy.ndarray`
        ``(len(vertices), 4)`` weights, in the component's bones

    blendIndices: :class:`numpy.ndarray`
        ``(len(vertices), 4)`` bone indices, in the component's numbering (negative sentinels
        possible for the negative-index split)

    position: :class:`numpy.ndarray`
        The filtered ``Position.buf`` rows

    texcoord: :class:`numpy.ndarray`
        The filtered ``Texcoord.buf`` rows

    ibs: Dict[:class:`str`, :class:`numpy.ndarray`]
        Per mod object, the triangles this component draws, renumbered into ``vertices``

    live: Optional[:class:`numpy.ndarray`]
        For a negative-index result, which of ``vertices`` carry no sentinel (the ones the
        component really draws); ``None`` for a graph cut, where every vertex is live

    drawnObjects: Optional[List[:class:`str`]]
        The mod objects that have at least one triangle worth drawing (``None``: every object
        with triangles)

    stats: Dict[:class:`str`, object]
        Counts worth printing
    """

    def __init__(self, component: str):
        self.component = component
        self.live: Optional[np.ndarray] = None
        self.drawnObjects: Optional[List[str]] = None
        self.trimmed = False
        self.vertices = np.zeros(0, dtype = np.int64)
        self.blendWeights = np.zeros((0, 4), dtype = np.float32)
        self.blendIndices = np.zeros((0, 4), dtype = np.int32)
        self.position = np.zeros((0, 0), dtype = np.uint8)
        self.texcoord = np.zeros((0, 0), dtype = np.uint8)
        self.ibs: Dict[str, np.ndarray] = {}
        self.stats: Dict[str, object] = {}

    @property
    def vertexCount(self) -> int:
        return int(len(self.vertices))


def _componentShares(mod: ModBuffers, remaps: Dict[str, Dict[int, int]]) -> Dict[str, np.ndarray]:
    """
    Per component, every vertex's total weight on bones that belong to it
    """

    used = mod.blendWeights > 0
    result = {}
    for component, remap in remaps.items():
        member = np.isin(mod.blendIndices, np.array(list(remap), dtype = np.int32)) & used
        result[component] = (mod.blendWeights * member).sum(axis = 1)
    return result


def negativeIndexSplit(mod: ModBuffers, remaps: Dict[str, Dict[int, int]],
                       secondary: Optional[Dict[str, Dict[int, int]]] = None, trim: bool = True) -> Dict[str, ComponentResult]:
    """
    The negative-index strategy: one ``RemapBlend.buf`` per component over the whole mod, with
    the bones of the other components turned into the ``-index-1`` sentinel

    Parameters
    ----------
    mod: :class:`ModBuffers`
        The mod

    remaps: Dict[:class:`str`, Dict[:class:`int`, :class:`int`]]
        Per target component, source index to target index

    secondary: Optional[Dict[:class:`str`, Dict[:class:`int`, :class:`int`]]]
        Per target component, further source groups the component has a bone for
        (:func:`augmentFromReverse`), honoured only on a vertex that also carries one of the
        component's ``remaps`` bones --- a hair vertex weighted head + bang keeps its head weight
        on the Bang's head bone; a face vertex weighted to the head alone still collapses

    trim: :class:`bool`
        Whether each object's ``.ib`` is filtered to the triangles all of whose corners are live
        (no renumbering, the vertex buffers stay whole). A triangle with a sentinel corner is
        not invisible: the sentinel bone reads a garbage matrix and the corner lands near the
        origin, so the triangle stretches through the model as a sliver

    Returns
    -------
    Dict[:class:`str`, :class:`ComponentResult`]
        Per component: every vertex, the remapped blend, the untouched position / texcoord /
        index buffers, and how many vertices are fully, partly and not at all this component's
    """

    used = mod.blendWeights > 0
    shares = _componentShares(mod, remaps)
    totals = mod.blendWeights.sum(axis = 1)

    results = {}
    for component, remap in remaps.items():
        result = ComponentResult(component)
        indices = mod.blendIndices.copy()
        size = int(mod.blendIndices.max()) + 1
        lookup = np.full(size, -1, dtype = np.int64)
        for source, target in remap.items():
            if (source < size):
                lookup[source] = target
        mapped = lookup[np.clip(indices, 0, size - 1)]
        inComponent = (mapped >= 0) & used
        extraRemap = (secondary or {}).get(component, {})
        if (extraRemap):
            extraLookup = np.full(size, -1, dtype = np.int64)
            for source, target in extraRemap.items():
                if (source < size and source not in remap):
                    extraLookup[source] = target
            extraMapped = extraLookup[np.clip(indices, 0, size - 1)]
            anchored = inComponent.any(axis = 1)                    # the vertex carries one of the component's own bones
            takeExtra = (extraMapped >= 0) & used & ~inComponent & anchored[:, None]
            mapped = np.where(takeExtra, extraMapped, mapped)
            inComponent = inComponent | takeExtra
        newIndices = np.where(inComponent, mapped, -indices - 1)
        newIndices = np.where(used, newIndices, indices)          # a zero-weight slot is left alone

        result.vertices = np.arange(mod.vertexCount)
        result.blendWeights = mod.blendWeights.copy()
        result.blendIndices = newIndices.astype(np.int32)
        result.position = mod.position
        result.texcoord = mod.texcoord
        result.live = ((newIndices >= 0) | ~used).all(axis = 1) & (totals > 0)
        liveTriangles = {name: (int(result.live[ib].all(axis = 1).sum()) if (len(ib)) else 0) for name, ib in mod.ibs.items()}
        result.drawnObjects = [name for name, n in liveTriangles.items() if (n > 0)]
        if (trim):
            result.ibs = {name: (ib[result.live[ib].all(axis = 1)].copy() if (len(ib)) else ib.copy()) for name, ib in mod.ibs.items()}
        else:
            result.ibs = {name: ib.copy() for name, ib in mod.ibs.items()}
        result.trimmed = trim

        share = shares[component] / np.where(totals > 0, totals, 1)
        if (extraRemap):
            share = np.where(inComponent, mod.blendWeights, 0.0).sum(axis = 1) / np.where(totals > 0, totals, 1)
        result.stats = {"vertices": mod.vertexCount,
                        "fully this component": int((share >= 0.999).sum()),
                        "partly this component": int(((share > 0.001) & (share < 0.999)).sum()),
                        "not this component (collapse)": int((share <= 0.001).sum()),
                        "negative sentinels written": int(((newIndices < 0) & used).sum()),
                        "triangles fully live": liveTriangles,
                        "ib trimmed to them": trim}
        results[component] = result
    return results


def graphCutSplit(mod: ModBuffers, remaps: Dict[str, Dict[int, int]], mode: str = "strict",
                  excludeTriangles: Optional[Dict[str, np.ndarray]] = None, fillComponents: Optional[Sequence[str]] = None) -> Dict[str, ComponentResult]:
    """
    The graph-cut strategy: per component, the triangles that are its, the vertices they use,
    every buffer filtered to those vertices and the index buffers renumbered

    Parameters
    ----------
    mod: :class:`ModBuffers`
        The mod

    remaps: Dict[:class:`str`, Dict[:class:`int`, :class:`int`]]
        Per target component, source index to target index

    mode: :class:`str`
        ``"strict"``: a vertex belongs to a component when **every** bone it carries with a
        non-zero weight is in the component's rows, and a triangle is kept when all three of its
        vertices belong --- a triangle straddling two components is dropped by both (a hole at
        the seam). ``"relaxed"``: the same membership, but a triangle is kept when at least
        **two** of its vertices belong, so the seam triangles go to the side holding two of
        their corners (the third corner's foreign weight is dropped and the rest renormalised).
        ``"majority"``: a vertex belongs to the component holding most of its weight,
        a triangle goes to the component most of its vertices belong to (ties by summed share),
        every triangle is drawn exactly once, and a vertex a triangle drags into a component it
        has no bone in is skinned to the bone its triangle neighbours use there.
        ``"fill"``: ``majority`` among ``fillComponents`` only, over the triangles not in
        ``excludeTriangles`` --- for a mix, where the negative-index components draw the
        triangles all of whose corners are live in them and the cut components take the rest

    excludeTriangles: Optional[Dict[:class:`str`, :class:`numpy.ndarray`]]
        Per mod object, the triangles no cut component may keep (drawn elsewhere)

    fillComponents: Optional[Sequence[:class:`str`]]
        For ``"fill"``: the components that share the remaining triangles out (default: all)

    Returns
    -------
    Dict[:class:`str`, :class:`ComponentResult`]
        Per component: the vertex subset, the filtered and remapped buffers, the renumbered index
        buffers, and the triangle counts kept and dropped per object
    """

    if (mode not in ("strict", "relaxed", "majority", "fill")):
        raise ValueError(f"unknown mode '{mode}' (expected 'strict', 'relaxed', 'majority' or 'fill')")

    used = mod.blendWeights > 0
    shares = _componentShares(mod, remaps)
    components = list(remaps)
    shareMatrix = np.stack([shares[c] for c in components], axis = 1)         # (vertices, components)
    totals = mod.blendWeights.sum(axis = 1)

    lookups = {}
    for component, remap in remaps.items():
        lookup = np.full(int(mod.blendIndices.max()) + 1, -1, dtype = np.int64)
        for source, target in remap.items():
            if (source < len(lookup)):
                lookup[source] = target
        lookups[component] = lookup

    fillColumns = [i for i, c in enumerate(components) if (fillComponents is None or c in fillComponents)]
    if (mode in ("strict", "relaxed")):
        # vertex v belongs to c iff its whole weight is on c's bones
        belongs = np.isclose(shareMatrix, totals[:, None]) & (totals[:, None] > 0)
    else:
        # the owner of a vertex: the (fill) component holding most of its weight, when it holds any
        ownerShare = shareMatrix[:, fillColumns] if (mode == "fill") else shareMatrix
        owner = ownerShare.argmax(axis = 1)
        belongs = np.zeros_like(shareMatrix, dtype = bool)
        hasShare = ownerShare.max(axis = 1) > 0
        ownerColumn = np.array(fillColumns)[owner] if (mode == "fill") else owner
        belongs[np.arange(len(owner))[hasShare], ownerColumn[hasShare]] = True

    results = {}
    for column, component in enumerate(components):
        result = ComponentResult(component)
        vertexBelongs = belongs[:, column]
        keptTriangles: Dict[str, np.ndarray] = {}
        dropped: Dict[str, int] = {}
        for objectName, ib in mod.ibs.items():
            if (not len(ib) or (mode == "fill" and column not in fillColumns)):
                keptTriangles[objectName] = ib[:0]
                dropped[objectName] = int(len(ib))
                continue
            if (mode == "strict"):
                keep = vertexBelongs[ib].all(axis = 1)
            elif (mode == "relaxed"):
                # at least two of the three vertices are wholly the component's; the third is dragged
                #   in and skinned to the component's bones it has (or its neighbours', if none)
                keep = vertexBelongs[ib].sum(axis = 1) >= 2
            else:
                votes = belongs[ib]                                  # (triangles, 3, components)
                counts = votes.sum(axis = 1)                          # (triangles, components)
                weightSum = shareMatrix[ib].sum(axis = 1)             # tie-break by summed share
                score = counts * 10.0 + weightSum / max(1.0, weightSum.max() if weightSum.size else 1.0)
                if (mode == "fill"):
                    score[:, [i for i in range(len(components)) if (i not in fillColumns)]] = -1.0
                    keep = (score.argmax(axis = 1) == column) & (score.max(axis = 1) > 0)
                    if (excludeTriangles is not None and objectName in excludeTriangles):
                        keep &= ~excludeTriangles[objectName]
                else:
                    keep = score.argmax(axis = 1) == column
            keptTriangles[objectName] = ib[keep]
            dropped[objectName] = int((~keep).sum())
        droppedLabel = "triangles dropped" if (mode in ("strict", "relaxed")) else "triangles drawn by another component"
        if (mode == "fill"):
            droppedLabel = "triangles left to the others"

        referenced = np.unique(np.concatenate([ib.ravel() for ib in keptTriangles.values()] or [np.zeros(0, dtype = np.uint32)]))
        vertices = referenced.astype(np.int64)
        renumber = np.full(mod.vertexCount, -1, dtype = np.int64)
        renumber[vertices] = np.arange(len(vertices))

        # the blend, in the component's bones
        lookup = lookups[component]
        weights = mod.blendWeights[vertices].copy()
        indices = mod.blendIndices[vertices].copy()
        mapped = lookup[np.clip(indices, 0, len(lookup) - 1)]
        inComponent = (mapped >= 0) & (weights > 0)
        weights = np.where(inComponent, weights, 0.0)
        indices = np.where(inComponent, mapped, 0)
        rowTotals = weights.sum(axis = 1)
        orphans = np.nonzero(rowTotals <= 0)[0]
        if (len(orphans)):
            # a vertex dragged in by a triangle with no bone of this component: skin it to the bone
            #   its triangle neighbours mostly use here (fallback: the component's commonest bone)
            neighbourBone = _neighbourBones(keptTriangles, renumber, indices, weights, orphans)
            for row, bone in zip(orphans, neighbourBone):
                weights[row] = [1.0, 0.0, 0.0, 0.0]
                indices[row] = [bone, 0, 0, 0]
            rowTotals = weights.sum(axis = 1)
        weights = weights / rowTotals[:, None]

        result.vertices = vertices
        result.blendWeights = weights.astype(np.float32)
        result.blendIndices = indices.astype(np.int32)
        result.position = mod.position[vertices]
        result.texcoord = mod.texcoord[vertices]
        result.ibs = {name: renumber[ib].astype(np.uint32) for name, ib in keptTriangles.items()}
        result.stats = {"vertices": int(len(vertices)), "of": mod.vertexCount,
                        "triangles kept": {name: int(len(ib)) for name, ib in keptTriangles.items()},
                        droppedLabel: dropped,
                        "vertices renormalised": int((~inComponent & (mod.blendWeights[vertices] > 0)).any(axis = 1).sum()),
                        "vertices skinned to a neighbour's bone": int(len(orphans))}
        results[component] = result

    return results


def _neighbourBones(keptTriangles: Dict[str, np.ndarray], renumber: np.ndarray, indices: np.ndarray, weights: np.ndarray, orphans: np.ndarray) -> List[int]:
    """
    For each orphan row (a vertex with no bone of the component), the bone most of its triangle
    neighbours are dominated by; the component's commonest bone when it has no such neighbour
    """

    dominant = indices[np.arange(len(indices)), weights.argmax(axis = 1)]
    valid = weights.sum(axis = 1) > 0
    overall = Counter(dominant[valid].tolist()).most_common(1)
    fallback = overall[0][0] if (overall) else 0

    neighbours: Dict[int, List[int]] = {int(row): [] for row in orphans}
    orphanSet = set(neighbours)
    for ib in keptTriangles.values():
        if (not len(ib)):
            continue
        local = renumber[ib]
        for tri in local:
            for row in tri:
                if (int(row) in orphanSet):
                    neighbours[int(row)].extend(int(other) for other in tri if (other != row and valid[other]))

    result = []
    for row in orphans:
        votes = Counter(dominant[n] for n in neighbours[int(row)])
        result.append(int(votes.most_common(1)[0][0]) if (votes) else int(fallback))
    return result


# ------------------------------------------------------------------------------------------------
# writing


def writeComponent(folder: str, prefix: str, result: ComponentResult, writeGeometry: bool = True) -> Dict[str, str]:
    """
    Writes one component's buffers

    Parameters
    ----------
    folder: :class:`str`
        The folder to write into (created)

    prefix: :class:`str`
        The files' prefix (``yelanBody`` -> ``yelanBodyRemapBlend.buf``, ``yelanBodyPosition.buf``, ...)

    result: :class:`ComponentResult`
        The component's buffers

    writeGeometry: :class:`bool`
        Whether to write the position / texcoord / index buffers too (the negative-index split
        reuses the mod's own, so only the blend is written)

    Returns
    -------
    Dict[:class:`str`, :class:`str`]
        The files written, keyed ``blend`` / ``position`` / ``texcoord`` / ``ib:<object>``
    """

    os.makedirs(folder, exist_ok = True)
    written = {}
    blendPath = os.path.join(folder, f"{prefix}RemapBlend.buf")
    with open(blendPath, "wb") as f:
        f.write(ModBuffers.encodeBlend(result.blendWeights, result.blendIndices))
    written["blend"] = blendPath

    if (not writeGeometry and getattr(result, "trimmed", False)):
        for objectName, ib in result.ibs.items():
            path = os.path.join(folder, f"{prefix}{objectName}.ib")
            with open(path, "wb") as f:
                f.write(np.ascontiguousarray(ib, dtype = "<u4").tobytes())
            written[f"ib:{objectName}"] = path

    if (writeGeometry):
        for key, rows in (("position", result.position), ("texcoord", result.texcoord)):
            path = os.path.join(folder, f"{prefix}{key.capitalize()}.buf")
            with open(path, "wb") as f:
                f.write(np.ascontiguousarray(rows).tobytes())
            written[key] = path
        for objectName, ib in result.ibs.items():
            path = os.path.join(folder, f"{prefix}{objectName}.ib")
            with open(path, "wb") as f:
                f.write(np.ascontiguousarray(ib, dtype = "<u4").tobytes())
            written[f"ib:{objectName}"] = path
    return written


class IniLayout():
    """
    What a target component's draw needs from the ``.ini`` beyond its hashes: which texture
    registers each object draw sets and which external fix it runs --- skin knowledge rather than
    geometry, so it is passed in by the caller rather than guessed

    Parameters
    ----------
    registers: Union[Sequence[Tuple[:class:`str`, :class:`str`]], Dict[:class:`str`, Sequence[Tuple[:class:`str`, :class:`str`]]]]
        The texture registers an object draw sets, as ``(register, texture kind)`` where the
        kind (``"Diffuse"`` / ``"LightMap"`` / ``"NormalMap"`` / ...) names the mod's
        ``<modName><Object><Kind>.dds``. Either one list for every draw slot of the component, or
        a dict keyed by the target's object slot (``"A"``, ``"B"``, ...) when the slots differ

    fixCommandList: Optional[:class:`str`]
        The external fix to run per draw (``CommandList\\global\\ORFix\\ORFix``), if any

    extraLines: Optional[Sequence[:class:`str`]]
        Lines written after the fix and before ``drawindexed`` in every draw of the component
        (``ps-t3 = null`` to hunt a game texture the mod does not override)

    Attributes
    ----------
    registers: Union[List[Tuple[:class:`str`, :class:`str`]], Dict[:class:`str`, List[Tuple[:class:`str`, :class:`str`]]]]
        The texture registers

    fixCommandList: Optional[:class:`str`]
        The external fix
    """

    def __init__(self, registers, fixCommandList: Optional[str] = None, extraLines: Optional[Sequence[str]] = None):
        if (isinstance(registers, dict)):
            self.registers = {slot: list(regs) for slot, regs in registers.items()}
        else:
            self.registers = list(registers)
        self.fixCommandList = fixCommandList
        self.extraLines = list(extraLines or [])

    def registersFor(self, slot: str) -> List[Tuple[str, str]]:
        """
        The registers a draw in ``slot`` sets

        Parameters
        ----------
        slot: :class:`str`
            The target's object slot (``"A"``)

        Returns
        -------
        List[Tuple[:class:`str`, :class:`str`]]
            The ``(register, kind)`` pairs
        """

        if (isinstance(self.registers, dict)):
            return self.registers.get(slot, [])
        return self.registers


def findTexture(modFolder: str, modName: str, modObject: str, kind: str) -> Optional[str]:
    """
    The mod's ``<modName><Object><Kind>.dds``, matched without case

    Parameters
    ----------
    modFolder: :class:`str`
        The mod folder (not searched recursively)

    modName: :class:`str`
        The mod's name

    modObject: :class:`str`
        The object (``Head``)

    kind: :class:`str`
        The texture kind (``Diffuse``)

    Returns
    -------
    Optional[:class:`str`]
        The file's path, ``None`` when the mod has no such texture
    """

    wanted = f"{modName}{modObject}{kind}.dds".lower()
    for fileName in os.listdir(modFolder):
        if (fileName.lower() == wanted and os.path.isfile(os.path.join(modFolder, fileName))):
            return os.path.join(modFolder, fileName)
    return None


def iniText(modName: str, toName: str, results: Dict[str, ComponentResult], hashes: Dict[str, Dict[str, object]],
            layouts: Dict[str, IniLayout], files: Dict[str, Dict[str, str]], modFolder: str,
            positionStride: int, texcoordStride: int, blendStride: int = ModBuffers.BlendStride,
            sharedGeometry: Optional[Dict[str, str]] = None, strategy: str = "",
            faceDiffuseHash: Optional[str] = None, componentGeometry: Optional[Dict[str, Dict[str, str]]] = None,
            componentStrategies: Optional[Dict[str, str]] = None, faceRegister: str = "ps-t0",
            textureOverrides: Optional[Dict[Tuple[str, str], str]] = None, objectSlots: Optional[Dict[str, str]] = None) -> str:
    """
    The remap-only ``.ini`` that draws the split mod on the target skin. It is self-contained
    (it declares its own texture resources, under names that cannot collide with the mod's
    own ``.ini``), meant to sit beside the mod's ``.ini`` in the mod folder --- the mod keeps
    drawing on the source skin, this file draws it on the target

    Parameters
    ----------
    modName: :class:`str`
        The mod's name, for section names and texture file names (``Yelan``)

    toName: :class:`str`
        The target skin's name, for section names (``YelanTranquil``)

    results: Dict[:class:`str`, :class:`ComponentResult`]
        The split, per component

    hashes: Dict[:class:`str`, Dict[:class:`str`, object]]
        Per component, its ``hash.json`` record (:meth:`DumpMod.readHashJson`'s dict)

    layouts: Dict[:class:`str`, :class:`IniLayout`]
        Per component, its draw layout

    files: Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]
        Per component, the files :func:`writeComponent` wrote

    modFolder: :class:`str`
        The mod folder --- the ``.ini`` goes there, so every file name is relative to it

    positionStride: :class:`int`
        The ``Position.buf`` stride

    texcoordStride: :class:`int`
        The ``Texcoord.buf`` stride

    blendStride: :class:`int`
        The ``Blend.buf`` stride

    sharedGeometry: Optional[Dict[:class:`str`, :class:`str`]]
        For the negative-index split: the mod's own ``position`` / ``texcoord`` / ``ib:<object>``
        paths, which every component uses instead of per-component copies

    strategy: :class:`str`
        A word for the header comment

    faceDiffuseHash: Optional[:class:`str`]
        The target's face diffuse texture hash, to point the mod's ``<modName>FaceHeadDiffuse.dds``
        at it (skipped when the mod has no such file)

    componentGeometry: Optional[Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
        Per component, the ``position`` / ``texcoord`` / ``ib:<object>`` paths to use instead of
        the component's own files --- for a mix where some components are negative-index (the
        mod's own geometry) and others graph-cut (their own); takes precedence over
        ``sharedGeometry`` for the components it names

    componentStrategies: Optional[Dict[:class:`str`, :class:`str`]]
        Per component, a word for its header comment

    faceRegister: :class:`str`
        The register the face diffuse override binds the texture to (GI 6.x swapped the face's
        diffuse and light map registers; older mods say ``ps-t0``)

    textureOverrides: Optional[Dict[Tuple[:class:`str`, :class:`str`], :class:`str`]]
        Per ``(mod object, texture kind)``, a file in the mod folder to bind instead of the
        mod's own ``<modName><Object><Kind>.dds`` --- an edited lightmap, typically

    objectSlots: Optional[Dict[:class:`str`, :class:`str`]]
        Per mod object, the target's draw slot (``"A"``) to draw it in, overriding the rank
        rule --- the slots differ in which material ramps their shader carries (YelanTranquil's
        slot A has the skin band, slot B, the dress, has none). Applies to every component that
        has the slot; objects not named keep the rank rule among the slots left

    Returns
    -------
    :class:`str`
        The ``.ini`` text
    """

    def rel(path: str) -> str:
        return os.path.relpath(path, modFolder).replace("/", "\\")

    prefix = f"{modName}{toName}"
    lines = [f"; {modName} -> {toName}: {strategy} split, generated by Tools/VGRemapFinder (ComponentSplit)",
             f"; Sits beside the mod's own .ini, which keeps drawing it on {modName}; this file draws the same mod on",
             f"; {toName}, one section group per component. Only one .ini drawing {toName} may be enabled at a time",
             f"; (a DISABLED prefix on the file name turns one off; hand-made {toName} overrides count too).", ""]

    resources: List[str] = []
    textures: Dict[str, Optional[str]] = {}       # resource name -> file (None: the mod lacks it)

    def textureResource(modObject: str, kind: str) -> str:
        name = f"Resource{prefix}{modObject}{kind}"
        if (name not in textures):
            override = (textureOverrides or {}).get((modObject, kind))
            if (override is not None):
                overridePath = override if (os.path.isabs(override)) else os.path.join(modFolder, override)
                if (not os.path.isfile(overridePath)):
                    raise FileNotFoundError(f"texture override for {modObject} {kind}: '{overridePath}' does not exist")
                textures[name] = overridePath
            else:
                textures[name] = findTexture(modFolder, modName, modObject, kind)
        return name

    def drawLines(tag: str, layout: IniLayout, slot: str, modObject: str) -> List[str]:
        result = [f"ib = Resource{tag}{modObject}IB"]
        for register, kind in layout.registersFor(slot):
            name = textureResource(modObject, kind)
            if (textures[name] is None):
                result.append(f"; {register} = {name}   (the mod has no {modName}{modObject}{kind}.dds -- add one, or leave the game's)")
            else:
                result.append(f"{register} = {name}")
        if (layout.fixCommandList):
            result.append(f"run = {layout.fixCommandList}")
        result += list(layout.extraLines)
        result.append("drawindexed = auto")
        return result

    for component, result in results.items():
        layout = layouts[component]
        record = hashes[component]
        tag = f"{prefix}{component}"
        own = files[component]
        geometry = own if (sharedGeometry is None) else sharedGeometry
        if (componentGeometry is not None and component in componentGeometry):
            geometry = dict(componentGeometry[component])
            geometry.update({k: v for k, v in own.items() if (k.startswith("ib:"))})     # a trimmed .ib is the component's own
        modObjects = [name for name in result.ibs if (len(result.ibs[name]) and (result.drawnObjects is None or name in result.drawnObjects))]

        how = f", {componentStrategies[component]}" if (componentStrategies and component in componentStrategies) else ""
        lines += [f"; ---------------- {toName} {component} ({result.vertexCount} vertices{how}) ----------------", ""]
        if (record.get("draw")):
            lines += [f"[TextureOverride{tag}VertexLimitRaise]", f"hash = {record['draw']}",
                      f"override_byte_stride = {positionStride}", f"override_vertex_count = {result.vertexCount}", ""]
        lines += [f"[TextureOverride{tag}Position]", f"hash = {record['position']}", f"vb0 = Resource{tag}Position", ""]
        lines += [f"[TextureOverride{tag}Blend]", f"hash = {record['blend']}", f"vb1 = Resource{tag}Blend", "handling = skip",
                  f"draw = {result.vertexCount},0", ""]
        if (record.get("texcoord")):
            lines += [f"[TextureOverride{tag}Texcoord]", f"hash = {record['texcoord']}", f"vb1 = Resource{tag}Texcoord", ""]
        lines += [f"[TextureOverride{tag}IB]", f"hash = {record['ib']}", "handling = skip", "drawindexed = auto", ""]

        # the target's object slots in draw order; the mod object of the same rank is drawn in each
        slots = list(record.get("objects") or [])
        firstIndexes = list(record.get("objectIndexes") or [])
        # pinned objects go to their slot; the rest follow the rank rule over the slots nobody is pinned to
        pinned = {o: s for o, s in (objectSlots or {}).items() if (o in modObjects and s in slots)}
        freeObjects = [o for o in modObjects if (o not in pinned)]
        freeSlots = [s for s in slots if (s not in pinned.values())] or slots[-1:]
        for rank, slot in enumerate(slots):
            firstIndex = firstIndexes[rank] if (rank < len(firstIndexes)) else 0
            lines += [f"[TextureOverride{tag}{slot}]", f"hash = {record['ib']}", f"match_first_index = {firstIndex}"]
            # the mod object of the same rank; the last slot also draws whatever objects are left over,
            #   one ib / drawindexed pair after another in the SAME section (two sections on one
            #   hash + match_first_index collide)
            objectsHere = [o for o in modObjects if (pinned.get(o) == slot)]
            if (slot in freeSlots):
                freeRank = freeSlots.index(slot)
                objectsHere += freeObjects[freeRank:freeRank + 1] if (freeRank < len(freeSlots) - 1) else freeObjects[freeRank:]
            if (not objectsHere):
                lines += ["; no mod object for this slot: the original part is hidden", "ib = null"]
            for modObject in objectsHere:
                lines += [f"; draws the mod's {modObject}"] + drawLines(tag, layout, slot, modObject)
            lines.append("")

        resources += [f"[Resource{tag}Position]", "type = Buffer", f"stride = {positionStride}", f"filename = {rel(geometry['position'])}", "",
                      f"[Resource{tag}Blend]", "type = Buffer", f"stride = {blendStride}", f"filename = {rel(own['blend'])}", ""]
        if (record.get("texcoord")):
            resources += [f"[Resource{tag}Texcoord]", "type = Buffer", f"stride = {texcoordStride}", f"filename = {rel(geometry['texcoord'])}", ""]
        for modObject in modObjects:
            resources += [f"[Resource{tag}{modObject}IB]", "type = Buffer", "format = DXGI_FORMAT_R32_UINT", f"filename = {rel(geometry[f'ib:{modObject}'])}", ""]

    if (faceDiffuseHash):
        facePath = (textureOverrides or {}).get(("FaceHead", "Diffuse")) or findTexture(modFolder, modName, "FaceHead", "Diffuse")
        if (facePath is not None and not os.path.isabs(facePath)):
            facePath = os.path.join(modFolder, facePath)
        if (facePath is not None):
            name = f"Resource{prefix}FaceHeadDiffuse"
            lines += ["; ---------------- face ----------------", "",
                      f"[TextureOverride{prefix}FaceHeadDiffuse]", f"hash = {faceDiffuseHash}", f"{faceRegister} = {name}", ""]
            textures[name] = facePath

    lines += ["; ---------------- resources ----------------", ""] + resources
    for name, path in textures.items():
        if (path is not None):
            lines += [f"[{name}]", f"filename = {rel(path)}", ""]
    return "\n".join(lines) + "\n"


def verifySplit(results: Dict[str, ComponentResult], mod: ModBuffers, strategy: str) -> List[str]:
    """
    The invariants a split must satisfy, as a list of failures (empty when it is sound)

    Parameters
    ----------
    results: Dict[:class:`str`, :class:`ComponentResult`]
        The split

    mod: :class:`ModBuffers`
        The mod it came from

    strategy: :class:`str`
        ``"negative"`` or ``"graphcut"``

    Returns
    -------
    List[:class:`str`]
        The failures
    """

    failures = []
    for component, result in results.items():
        n = result.vertexCount
        if (result.blendWeights.shape != (n, 4) or result.blendIndices.shape != (n, 4)):
            failures.append(f"{component}: blend shape {result.blendWeights.shape} / {result.blendIndices.shape} for {n} vertices")
        if (len(result.position) != n or len(result.texcoord) != n):
            failures.append(f"{component}: position/texcoord rows {len(result.position)}/{len(result.texcoord)} for {n} vertices")
        for objectName, ib in result.ibs.items():
            if (len(ib) and int(ib.max()) >= n):
                failures.append(f"{component}/{objectName}: index {int(ib.max())} beyond {n} vertices")
        used = result.blendWeights > 0
        if (strategy == "graphcut"):
            if (((result.blendIndices < 0) & used).any()):
                failures.append(f"{component}: negative bone index in a graph-cut blend")
            sums = result.blendWeights.sum(axis = 1)
            if (n and not np.allclose(sums, 1.0, atol = 1e-3)):
                failures.append(f"{component}: {int((~np.isclose(sums, 1.0, atol = 1e-3)).sum())} vertices whose weights do not sum to 1")
        else:
            if (not np.array_equal(result.blendWeights, mod.blendWeights)):
                failures.append(f"{component}: weights changed in a negative-index blend")

    if (strategy == "graphcut"):
        # every triangle of the mod is drawn at most once across the components
        for objectName in mod.ibs:
            total = sum(int(len(r.ibs.get(objectName, ()))) for r in results.values())
            if (total > len(mod.ibs[objectName])):
                failures.append(f"{objectName}: {total} triangles drawn across the components, the mod has {len(mod.ibs[objectName])}")
    return failures


# ------------------------------------------------------------------------------------------------
# the driver the per-mod scripts call


def makeArgParser(description: str, strategy: str, defaults: Dict[str, str]):
    """
    The command line the split scripts share

    Parameters
    ----------
    description: :class:`str`
        The script's description

    strategy: :class:`str`
        ``"negative"`` or ``"graphcut"`` (the latter adds ``--mode``)

    defaults: Dict[:class:`str`, :class:`str`]
        Defaults for ``mod``, ``hashJson``, ``draft``, ``sheet``, ``fromName``, ``toName``,
        ``out``, ``prefix``

    Returns
    -------
    :class:`argparse.ArgumentParser`
        The parser
    """

    import argparse

    parser = argparse.ArgumentParser(description = description, formatter_class = argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--mod", default = defaults.get("mod"), help = "the mod folder holding <prefix>Position.buf / Blend.buf / Texcoord.buf and the .ib files (default: %(default)s)")
    parser.add_argument("--prefix", default = defaults.get("prefix"), help = "the buffers' file prefix (default: the folder's only *Position.buf)")
    parser.add_argument("--objects", nargs = "+", default = None, metavar = "OBJECT",
                        help = "the mod's objects in the order its .ini draws them, handed to the target's draw slots in that order (default: Head Body Dress Extra, then the rest)")
    parser.add_argument("--hashJson", default = defaults.get("hashJson"), help = "the target skin's hash.json (default: %(default)s)")
    parser.add_argument("--draft", default = defaults.get("draft"), help = "the RemapDrafts workbook with the per-component remap (default: %(default)s)")
    parser.add_argument("--sheet", default = defaults.get("sheet"), help = "the sheet in it (default: %(default)s)")
    parser.add_argument("--library", action = "store_true", help = "take the remap from the installed API's shared table instead of the draft (needs a build carrying the rows)")
    parser.add_argument("--fromName", default = defaults.get("fromName"), help = "the source skin's name (default: %(default)s)")
    parser.add_argument("--toName", default = defaults.get("toName"), help = "the target skin's name (default: %(default)s)")
    parser.add_argument("--out", default = defaults.get("out"), help = "the folder the per-component buffers go to, relative to the mod folder (default: %(default)s)")
    parser.add_argument("--ini", default = defaults.get("ini"), help = "the .ini file name to write in the mod folder (default: %(default)s)")
    parser.add_argument("--base", default = defaults.get("base"), help = "a .ini in the mod folder whose text is prepended (the mod's own sections), so the output is a complete file (default: %(default)s)")
    parser.add_argument("--baseEnd", default = defaults.get("baseEnd"), help = "the first line of --base at which to stop copying, exactly (default: %(default)s, i.e. the whole file)")
    if (strategy == "graphcut"):
        parser.add_argument("--mode", default = "strict", choices = ["strict", "relaxed", "majority", "fill"],
                            help = "strict: a triangle is kept by a component only when all its vertices are wholly that component's (seam triangles are dropped); "
                                   "relaxed: kept when at least two of its vertices are, the third dragged in; "
                                   "majority: every vertex and triangle goes to the component holding most of its weight, so nothing is dropped; "
                                   "fill: the cut components take, by majority, every triangle a negative-index component does not draw completely (default: %(default)s)")
    parser.add_argument("--noAugment", action = "store_true",
                        help = "do not add, to a negative-index component's remap, the source groups its own bones correspond to per the reverse sheets (see augmentFromReverse)")
    parser.add_argument("--faceRegister", default = defaults.get("faceRegister", "ps-t0"), help = "the register the face diffuse override binds to (default: %(default)s)")
    parser.add_argument("--keepAllTriangles", action = "store_true",
                        help = "a negative-index component draws every triangle of the mod, sentinel corners included (default: only the triangles all of whose corners are live)")
    parser.add_argument("--texcoordFile", default = defaults.get("texcoordFile"), help = "read this Texcoord.buf (a vertex-colour edit) instead of <prefix>Texcoord.buf")
    parser.add_argument("--objectSlots", nargs = "+", metavar = "OBJECT=SLOT", default = defaults.get("objectSlots"),
                        help = "draw a mod object in that target slot instead of by rank (e.g. --objectSlots Head=A Body=A: both through slot A, whose ramp has the skin band)")
    parser.add_argument("--texture", nargs = 3, action = "append", metavar = ("OBJECT", "KIND", "FILE"), default = None,
                        help = "bind FILE (in the mod folder) as the mod object's texture of that kind instead of <modName><Object><Kind>.dds, e.g. --texture Body LightMap yelanBodyLightMapLifted77.dds"
                               + (f" (default: {defaults['textures']})" if (defaults.get("textures")) else ""))
    return parser


def runSplit(strategy, args, layouts: Dict[str, IniLayout], modName: str, mode: str = "strict") -> Dict[str, ComponentResult]:
    """
    Reads the mod and the remap, splits, writes the buffers and the ``.ini``, prints what happened

    Parameters
    ----------
    strategy: Union[:class:`str`, Dict[:class:`str`, :class:`str`]]
        ``"negative"`` or ``"graphcut"`` for every component, or a dict naming one per component
        (issue #190's table: ``{"Body": "graphcut", "Eye": "graphcut", "Bang": "negative"}``)

    args: :class:`argparse.Namespace`
        :func:`makeArgParser`'s result

    layouts: Dict[:class:`str`, :class:`IniLayout`]
        Per target component, its draw layout

    modName: :class:`str`
        The mod's name for section and texture names

    mode: :class:`str`
        The graph cut's mode

    Returns
    -------
    Dict[:class:`str`, :class:`ComponentResult`]
        The split
    """

    modFolder = os.path.abspath(args.mod)
    mod = ModBuffers(modFolder, args.prefix, getattr(args, "objects", None), texcoordPath = getattr(args, "texcoordFile", None))
    print(f"mod: {mod.prefix}* in {modFolder}: {mod.vertexCount} vertices, position stride {mod.positionStride}, texcoord stride {mod.texcoordStride}"
          + (f" (texcoord from {os.path.basename(mod.paths['texcoord'])})" if (getattr(args, "texcoordFile", None)) else ""))
    for name, ib in mod.ibs.items():
        print(f"  {name}: {len(ib)} triangles")
    groupCount = int(mod.blendIndices[mod.blendWeights > 0].max()) + 1 if (mod.blendWeights > 0).any() else 0
    print(f"  bones referenced: 0..{groupCount - 1}")

    records = {record["name"]: record for record in DumpMod.readHashJson(args.hashJson)}
    components = [name for name in records if (name in layouts)]
    unknown = [name for name in records if (name not in layouts)]
    if (unknown):
        print(f"  (hash.json components without a layout, skipped: {', '.join(unknown)})")

    if (args.library):
        remaps = remapsFromLibrary(args.fromName, args.toName, components)
        print(f"remap: the API's shared table, {args.fromName} -> {args.toName}")
    else:
        remaps = remapsFromDraft(args.draft, args.fromName, args.toName, args.sheet)
        remaps = {component: remaps[component] for component in components if (component in remaps)}
        print(f"remap: '{os.path.basename(args.draft)}' sheet {args.sheet!r}")
    for component in components:
        print(f"  {component}: {len(remaps.get(component, {}))} source groups")
    for problem in checkRemaps(remaps, groupCount):
        print(f"  WARNING {problem}")

    strategies = {c: strategy for c in components} if (isinstance(strategy, str)) else dict(strategy)
    unknownStrategy = [c for c in components if strategies.get(c) not in ("negative", "graphcut")]
    if (unknownStrategy):
        raise ValueError(f"no strategy ('negative' / 'graphcut') for component(s): {unknownStrategy}")

    negativeComponents = [c for c in components if (strategies[c] == "negative")]
    secondary: Dict[str, Dict[int, int]] = {}
    if (negativeComponents and not args.library and not getattr(args, "noAugment", False)):
        added = augmentFromReverse(remaps, args.draft, args.fromName, args.toName, negativeComponents)
        for component in negativeComponents:
            extra = added.get(component, {})
            secondary[component] = {s: b for s, (b, _) in extra.items()}
            if (extra):
                print(f"  {component} (negative index) also honours, on vertices that carry one of its own bones, " + ", ".join(f"{args.fromName} {s} -> {component}:{b}" for s, (b, _) in sorted(extra.items())))
            else:
                print(f"  {component} (negative index): nothing to add from the reverse sheet")
    negativeResults = negativeIndexSplit(mod, remaps, secondary, trim = not getattr(args, "keepAllTriangles", False)) if (negativeComponents) else {}
    cutComponents = [c for c in components if (strategies[c] == "graphcut")]
    exclude: Dict[str, np.ndarray] = {}
    if (mode == "fill"):
        # a triangle every corner of which is live in some negative-index component is that component's
        for objectName, ib in mod.ibs.items():
            mask = np.zeros(len(ib), dtype = bool)
            for c in negativeComponents:
                if (len(ib)):
                    mask |= negativeResults[c].live[ib].all(axis = 1)
            exclude[objectName] = mask
    cutResults = graphCutSplit(mod, remaps, mode, excludeTriangles = exclude, fillComponents = cutComponents) if (cutComponents) else {}
    results = {c: (negativeResults if (strategies[c] == "negative") else cutResults)[c] for c in components}
    strategyWords = {c: ("negative index" if (strategies[c] == "negative") else f"graph cut ({mode})") for c in components}

    outRoot = os.path.join(modFolder, args.out)
    files = {}
    modGeometry = {k: v for k, v in mod.paths.items()}      # includes a --texcoordFile override, since ModBuffers recorded what it read
    componentGeometry = {}
    for component, result in results.items():
        negative = strategies[component] == "negative"
        files[component] = writeComponent(os.path.join(outRoot, component), f"{mod.prefix}{component}", result, writeGeometry = not negative)
        if (negative):
            componentGeometry[component] = modGeometry
        print(f"{component} ({strategyWords[component]}): " + ", ".join(f"{k} = {v}" for k, v in result.stats.items()))
        print(f"  wrote {len(files[component])} files to {os.path.relpath(os.path.join(outRoot, component), modFolder)}")

    failures = []
    for word in ("negative", "graphcut"):
        subset = {c: r for c, r in results.items() if (strategies[c] == word)}
        if (subset):
            failures += verifySplit(subset, mod, word)
    for failure in failures:
        print(f"  FAILED {failure}")

    # how every mod object's triangles are shared out, and how many nobody draws
    for objectName, ib in mod.ibs.items():
        if (not len(ib)):
            continue
        drawnBy = np.zeros(len(ib), dtype = int)
        parts = []
        for c in components:
            result = results[c]
            if (strategies[c] == "negative"):
                mask = result.live[ib].all(axis = 1)
            else:
                keys = {tuple(t) for t in result.vertices[result.ibs[objectName]].tolist()} if (len(result.ibs[objectName])) else set()
                mask = np.array([tuple(t) in keys for t in ib.tolist()], dtype = bool) if (keys) else np.zeros(len(ib), dtype = bool)
            drawnBy += mask
            parts.append(f"{c} {int(mask.sum())}")
        print(f"coverage {objectName}: {len(ib)} triangles: " + ", ".join(parts) + f"; drawn by nobody {int((drawnBy == 0).sum())}, by more than one {int((drawnBy > 1).sum())}")

    faceHash = None
    try:
        import json
        with open(args.hashJson, "r", encoding = "utf-8") as f:
            for entry in json.load(f):
                if ((entry.get("component_name") or "") == "Face"):
                    for group in (entry.get("texture_hashes") or []):
                        for kind, _, value in group:
                            if (kind == "Diffuse"):
                                faceHash = value
    except Exception:
        pass

    words = sorted(set(strategyWords.values()))
    text = iniText(modName, args.toName, results, {c: records[c] for c in components}, layouts, files, modFolder,
                   mod.positionStride, mod.texcoordStride, ModBuffers.BlendStride,
                   componentGeometry = componentGeometry, componentStrategies = strategyWords,
                   strategy = (words[0] if (len(words) == 1) else "mixed " + " / ".join(words)), faceDiffuseHash = faceHash,
                   faceRegister = getattr(args, "faceRegister", "ps-t0"),
                   textureOverrides = {(o, k): f for o, k, f in (getattr(args, "texture", None) or getattr(args, "defaultTextures", None) or [])},
                   objectSlots = {item.split("=", 1)[0]: item.split("=", 1)[1] for item in (getattr(args, "objectSlots", None) or []) if ("=" in item)})

    base = getattr(args, "base", None)
    if (base):
        basePath = os.path.join(modFolder, base)
        with open(basePath, "r", encoding = "utf-8", errors = "replace") as f:
            baseLines = f.read().splitlines()
        baseEnd = getattr(args, "baseEnd", None)
        if (baseEnd):
            stops = [i for i, line in enumerate(baseLines) if (line.strip() == baseEnd.strip())]
            if (not stops):
                raise ValueError(f"'{basePath}' has no line {baseEnd!r} to stop at")
            baseLines = baseLines[:stops[0]]
        text = "\n".join(baseLines).rstrip() + "\n\n\n" + f"; ================ the mod's own sections above (from {base}); {args.toName} below ================\n\n" + text
        print(f"base: {len(baseLines)} lines of {base} prepended")

    iniPath = os.path.join(modFolder, args.ini)
    with open(iniPath, "w", encoding = "utf-8", newline = "\r\n") as f:
        f.write(text)
    print(f"ini: {iniPath}")
    for (o, k), f in {(o, k): f for o, k, f in (getattr(args, "texture", None) or getattr(args, "defaultTextures", None) or [])}.items():
        print(f"  {o} {k} bound to {f}")
    print("verification: " + ("OK" if (not failures) else f"{len(failures)} problem(s) above"))
    return results
