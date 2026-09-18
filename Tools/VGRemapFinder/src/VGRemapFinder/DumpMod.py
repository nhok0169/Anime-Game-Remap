import json
import os
import re
import sys
from collections import Counter
from typing import Dict, List, Optional, Sequence, Tuple, Union

import numpy as np

from .constants.Paths import APISrcPath


_api = None


def setAPI(api):
    """
    Uses an already-imported AG Remap API module (``FixRaidenBoss2`` from a checkout, or
    ``AnimeGameRemap`` from PyPI) instead of importing one from the repo's checkout

    Parameters
    ----------
    api: :class:`ModuleType`
        The API module
    """

    global _api
    _api = api


def importAPI():
    """
    Imports the AG Remap API, with a readable error if the API isn't built yet :raw-html:`<br />` :raw-html:`<br />`

    Uses the module given to :func:`setAPI` if there is one; otherwise imports ``FixRaidenBoss2``
    from this repo's checkout

    Returns
    -------
    The API module
    """

    if (_api is not None):
        return _api

    if (APISrcPath not in sys.path):
        sys.path.insert(1, APISrcPath)

    try:
        import FixRaidenBoss2 as FRB
    except ImportError as e:
        raise ImportError(
            f"Could not import the AG Remap API from '{APISrcPath}'.\n"
            "The API's native extensions probably aren't built yet -- see "
            "'AI Agent Help/Setup/CLAUDE.md' and 'AI Agent Help/Building/CLAUDE.md'.\n"
            f"Original error: {e}"
        ) from e

    return FRB


class DumpMod():
    """
    The geometry of one **component** of a mod: every vertex's position and the vertex groups it
    belongs to, plus which drawn object each vertex is part of :raw-html:`<br />` :raw-html:`<br />`

    Older GI characters are a single component (a ``hash.json`` names it ``""``), drawn from one
    position / blend / index buffer set. Newer skins are several --- YelanTranquil is a ``Body``,
    a ``Bang`` and an ``Eye`` --- **each with its own buffers and so its own vertex group index
    space**: ``Body 64`` and ``Bang 64`` are different bones. A whole character is a
    :class:`Character`, an ordered set of these

    Read from any of the three forms a GI character's geometry comes in --- see
    :meth:`Character.fromFolder`

    Parameters
    ----------
    name: :class:`str`
        The name of the mod (eg. ``"GanyuTwilight"``)

    positions: :class:`numpy.ndarray`
        ``(vertexCount, 3)`` array of vertex positions, in the order the file stores them
        (``x, z, y`` -- the axis order does not matter to this tool, only that both mods agree)

    blendIndices: :class:`numpy.ndarray`
        ``(vertexCount, 4)`` integer array of the vertex groups each vertex belongs to

    blendWeights: :class:`numpy.ndarray`
        ``(vertexCount, 4)`` float array of the weights matching :attr:`blendIndices`

    objects: Dict[:class:`str`, :class:`numpy.ndarray`]
        For each drawn object (``"Head"``, ``"Body"``, ``"Dress"``, ...) the sorted, unique
        vertex indices its index buffer references

    component: :class:`str`
        The component this geometry is (``""`` for a single-component character)

    Attributes
    ----------
    name: :class:`str`
        The name of the mod

    positions: :class:`numpy.ndarray`
        ``(vertexCount, 3)`` vertex positions

    blendIndices: :class:`numpy.ndarray`
        ``(vertexCount, 4)`` vertex group indices per vertex

    blendWeights: :class:`numpy.ndarray`
        ``(vertexCount, 4)`` vertex group weights per vertex

    objects: Dict[:class:`str`, :class:`numpy.ndarray`]
        The vertex indices each drawn object references

    component: :class:`str`
        The component this geometry is
    """

    VbFilePattern = re.compile(r"^(?P<stem>.*)-vb0=(?P<hash>[0-9a-fA-F]+)\.txt$")
    IbFilePattern = re.compile(r"^(?P<stem>.*)-ib=(?P<hash>[0-9a-fA-F]+)\.txt$")

    PositionBufPattern = re.compile(r"^(?P<prefix>.*)Position\.buf$", re.IGNORECASE)
    BlendBufPattern = re.compile(r"^(?P<prefix>.*)Blend\.buf$", re.IGNORECASE)
    IbBinaryPattern = re.compile(r"^(?P<stem>.*)\.ib$", re.IGNORECASE)

    # the blend files the remapper itself writes next to a mod's own; never the mod's geometry
    RemappedBufMarker = "remap"

    # a 3dmigoto frame analysis names every file by its draw call: 000123-vb0=<hash>-vs=<hash>.txt
    FrameFilePattern = re.compile(r"^(?P<draw>\d{6})-(?P<slot>vb\d+|ib)=(?P<hash>[0-9a-fA-F]+)(?P<rest>-.*)?\.(?P<ext>txt|buf)$")
    FrameLinePattern = re.compile(r"^vb(?P<slot>\d+)\[(?P<vertex>\d+)\]\+(?P<offset>\d+) (?P<name>[A-Za-z0-9_]+): (?P<values>.*)$")
    FrameFolderPattern = re.compile(r"^FrameAnalysis[-_ ]*(?P<name>.*?)[-_ ]*\d{4}-\d{2}-\d{2}-\d{6}$")

    # the drawn objects of a single-component character, in the order their index buffers are drawn
    ObjectOrder = ["Head", "Body", "Dress", "Extra"]

    HashJsonName = "hash.json"

    PositionKey = "POSITION"
    BlendIndicesKey = "BLENDINDICES"
    BlendWeightKey = "BLENDWEIGHT"

    # newer dumps (YelanTranquil, 5.7) spell the weights element BLENDWEIGHTS and type the indices
    #   unsigned; either spelling is accepted wherever an element is looked up by name
    ElementAliases = {"BLENDWEIGHT": ("BLENDWEIGHT", "BLENDWEIGHTS")}

    def __init__(self, name: str, positions: np.ndarray, blendIndices: np.ndarray, blendWeights: np.ndarray,
                 objects: Optional[Dict[str, np.ndarray]] = None, component: str = ""):
        self.name = name
        self.positions = positions
        self.blendIndices = blendIndices
        self.blendWeights = blendWeights
        self.objects = {} if (objects is None) else objects
        self.component = component

    @property
    def vertexCount(self) -> int:
        """
        The number of vertices in the component

        :getter: Retrieves the vertex count
        :type: :class:`int`
        """

        return int(self.positions.shape[0])

    @property
    def vertexGroupCount(self) -> int:
        """
        The number of vertex groups the component refers to (one past the largest index used) :raw-html:`<br />` :raw-html:`<br />`

        A vertex group with no vertices still counts, if a larger index is in use --- the game's
        bone list has no holes, so neither does the remap

        :getter: Retrieves the vertex group count
        :type: :class:`int`
        """

        if (self.blendIndices.size == 0):
            return 0
        return int(self.blendIndices.max()) + 1

    # ---------------------------------------------------------------------------------------------
    # finding the files

    @classmethod
    def findDumpFiles(cls, folder: str) -> Tuple[Dict[str, str], Dict[str, Tuple[str, str]]]:
        """
        Finds the dump files in a character folder

        Parameters
        ----------
        folder: :class:`str`
            The folder to search (not recursive)

        Returns
        -------
        Tuple[Dict[:class:`str`, :class:`str`], Dict[:class:`str`, Tuple[:class:`str`, :class:`str`]]]
            * the vertex buffer dumps, keyed by their hash (one path per distinct hash --- every
              object's ``vb0`` dump of a component is the same file, so it is only read once)
            * the index buffer dumps, keyed by the file's stem (eg. ``"GanyuTwilightBody"``), holding
              ``(path, vbHash)``, where ``vbHash`` is the hash of the vertex buffer dump next to it
        """

        vbFiles: Dict[str, str] = {}
        ibFiles: Dict[str, Tuple[str, str]] = {}
        stemsToVbHash: Dict[str, str] = {}

        for fileName in sorted(os.listdir(folder)):
            vbMatch = cls.VbFilePattern.match(fileName)
            if (vbMatch is not None):
                vbFiles.setdefault(vbMatch.group("hash").lower(), os.path.join(folder, fileName))
                stemsToVbHash[vbMatch.group("stem")] = vbMatch.group("hash").lower()

        for fileName in sorted(os.listdir(folder)):
            ibMatch = cls.IbFilePattern.match(fileName)
            if (ibMatch is None):
                continue

            stem = ibMatch.group("stem")
            vbHash = stemsToVbHash.get(stem)
            if (vbHash is None and len(vbFiles) == 1):
                vbHash = next(iter(vbFiles))

            if (vbHash is None):
                continue
            ibFiles[stem] = (os.path.join(folder, fileName), vbHash)

        return vbFiles, ibFiles

    @classmethod
    def findModFiles(cls, folder: str) -> Tuple[Optional[str], Optional[str], Dict[str, str]]:
        """
        Finds a single-component mod's binary geometry files in a folder

        Parameters
        ----------
        folder: :class:`str`
            The folder to search (not recursive)

        Returns
        -------
        Tuple[Optional[:class:`str`], Optional[:class:`str`], Dict[:class:`str`, :class:`str`]]
            * the ``*Position.buf``, if there is one
            * the ``*Blend.buf``, if there is one --- the mod's own, never a ``*RemapBlend.buf``
              the remapper wrote next to it, unless that is all there is
            * the ``*.ib`` files, keyed by their stem (eg. ``"GanyuBody"``)
        """

        pairs, ibFiles = cls.findModComponentFiles(folder)
        if (not pairs):
            return None, None, ibFiles
        prefix = sorted(pairs)[0]
        positionPath, blendPath = pairs[prefix]
        return positionPath, blendPath, ibFiles

    @classmethod
    def findModComponentFiles(cls, folder: str) -> Tuple[Dict[str, Tuple[str, str]], Dict[str, str]]:
        """
        Finds every ``<prefix>Position.buf`` / ``<prefix>Blend.buf`` pair in a mod folder --- one
        pair per component --- and its ``*.ib`` files

        Parameters
        ----------
        folder: :class:`str`
            The folder to search (not recursive)

        Returns
        -------
        Tuple[Dict[:class:`str`, Tuple[:class:`str`, :class:`str`]], Dict[:class:`str`, :class:`str`]]
            * the ``(position, blend)`` paths keyed by the files' shared prefix (eg.
              ``"YelanTranquilBody"``), the mod's own blend preferred over a ``*RemapBlend.buf``
            * the ``*.ib`` files, keyed by their stem
        """

        positions: Dict[str, List[str]] = {}
        blends: Dict[str, List[str]] = {}
        ibFiles: Dict[str, str] = {}

        for fileName in sorted(os.listdir(folder)):
            path = os.path.join(folder, fileName)
            if (not os.path.isfile(path)):
                continue

            positionMatch = cls.PositionBufPattern.match(fileName)
            blendMatch = cls.BlendBufPattern.match(fileName)
            if (positionMatch):
                positions.setdefault(positionMatch.group("prefix"), []).append(path)
            elif (blendMatch):
                prefix = blendMatch.group("prefix")
                if (cls.RemappedBufMarker in prefix.lower()):
                    # 'XRemapBlend.buf' belongs to component prefix 'X', as the remapper's own output
                    base = re.sub(r"(?i)[A-Za-z]*remap$", "", prefix)
                    blends.setdefault(base, []).append(path)
                else:
                    blends.setdefault(prefix, []).append(path)
            else:
                ibMatch = cls.IbBinaryPattern.match(fileName)
                if (ibMatch is not None):
                    ibFiles[ibMatch.group("stem")] = path

        def preferOwn(paths: List[str]) -> Optional[str]:
            own = [path for path in paths if (cls.RemappedBufMarker not in os.path.basename(path).lower())]
            chosen = own if (own) else paths
            return chosen[0] if (chosen) else None

        pairs: Dict[str, Tuple[str, str]] = {}
        for prefix, positionPaths in positions.items():
            blendPath = preferOwn(blends.get(prefix, []))
            if (blendPath is None):
                continue
            pairs[prefix] = (positionPaths[0], blendPath)

        return pairs, ibFiles

    @classmethod
    def isFrameAnalysisFolder(cls, folder: str) -> bool:
        """
        Whether a folder is a raw 3dmigoto frame analysis (its files are named by draw call:
        ``000123-vb0=<hash>-...``)

        Parameters
        ----------
        folder: :class:`str`
            The folder to check

        Returns
        -------
        :class:`bool`
            Whether the folder is a frame analysis
        """

        return any(cls.FrameFilePattern.match(fileName) for fileName in os.listdir(folder))

    @classmethod
    def frameAnalysisHashes(cls, folder: str) -> Dict[str, Counter]:
        """
        The buffer hashes a frame analysis holds text dumps for, and how many draw calls used each

        Parameters
        ----------
        folder: :class:`str`
            The frame analysis folder

        Returns
        -------
        Dict[:class:`str`, :class:`collections.Counter`]
            For each slot (``"vb0"``, ``"vb1"``, ..., ``"ib"``), the hashes and their draw call counts
        """

        result: Dict[str, Counter] = {}
        for fileName in os.listdir(folder):
            match = cls.FrameFilePattern.match(fileName)
            if (match is None or match.group("ext") != "txt"):
                continue
            result.setdefault(match.group("slot"), Counter())[match.group("hash").lower()] += 1
        return result

    @classmethod
    def describeFrameAnalysisHashes(cls, folder: str) -> str:
        """
        A printable listing of :meth:`frameAnalysisHashes`, for telling the user which hashes to
        pick from

        Parameters
        ----------
        folder: :class:`str`
            The frame analysis folder

        Returns
        -------
        :class:`str`
            The listing
        """

        lines = []
        for slot, counts in sorted(cls.frameAnalysisHashes(folder).items()):
            hashes = ", ".join(f"{hashValue} (x{count})" for hashValue, count in counts.most_common())
            lines.append(f"  {slot}: {hashes}")
        return "\n".join(lines) if (lines) else "  (no text dumps found)"

    @classmethod
    def findFrameFiles(cls, folder: str, slot: str, hashValue: str) -> List[str]:
        """
        The text dumps of one buffer in a frame analysis, in draw call order

        Parameters
        ----------
        folder: :class:`str`
            The frame analysis folder

        slot: :class:`str`
            ``"vb0"``, ``"vb1"``, ... or ``"ib"``

        hashValue: :class:`str`
            The buffer's hash

        Returns
        -------
        List[:class:`str`]
            The matching ``.txt`` files
        """

        hashValue = hashValue.lower()
        result = []
        for fileName in sorted(os.listdir(folder)):
            match = cls.FrameFilePattern.match(fileName)
            if (match is not None and match.group("ext") == "txt" and match.group("slot") == slot and match.group("hash").lower() == hashValue):
                result.append(os.path.join(folder, fileName))
        return result

    @classmethod
    def nameFromFrameAnalysisFolder(cls, folder: str) -> str:
        """
        A mod name out of a frame analysis folder's name: ``FrameAnalysis-Keqing-2026-09-09-213224``
        gives ``Keqing``. Falls back to the folder's whole name

        Parameters
        ----------
        folder: :class:`str`
            The frame analysis folder

        Returns
        -------
        :class:`str`
            The name
        """

        baseName = os.path.basename(os.path.abspath(folder).rstrip("/\\"))
        match = cls.FrameFolderPattern.match(baseName)
        if (match is not None and match.group("name")):
            return match.group("name")
        return baseName

    @classmethod
    def isDumpFolder(cls, folder: str) -> bool:
        """
        Whether a folder holds 3dmigoto dumps (a ``*-vb0=<hash>.txt``)

        Parameters
        ----------
        folder: :class:`str`
            The folder to check

        Returns
        -------
        :class:`bool`
            Whether the folder holds dumps
        """

        return any(cls.VbFilePattern.match(fileName) for fileName in os.listdir(folder))

    @classmethod
    def isModFolder(cls, folder: str) -> bool:
        """
        Whether a folder holds a mod's binary geometry (a ``*Position.buf`` and a ``*Blend.buf``)

        Parameters
        ----------
        folder: :class:`str`
            The folder to check

        Returns
        -------
        :class:`bool`
            Whether the folder holds a mod's ``.buf`` files
        """

        pairs, _ = cls.findModComponentFiles(folder)
        return bool(pairs)

    @classmethod
    def objectNameFromStem(cls, stem: str, modName: str) -> str:
        """
        Turns a file's stem (eg. ``"GanyuTwilightBody"``) into the object it draws
        (``"Body"``), by stripping the mod's name off the front

        Parameters
        ----------
        stem: :class:`str`
            The stem of the file

        modName: :class:`str`
            The name of the mod

        Returns
        -------
        :class:`str`
            The object's name
        """

        if (modName and stem.lower().startswith(modName.lower()) and len(stem) > len(modName)):
            return stem[len(modName):]
        return stem

    @classmethod
    def readHashJson(cls, path: str) -> List[Dict[str, object]]:
        """
        The components a ``hash.json`` (as GI-Model-Importer-Assets writes it) describes, keeping
        only the ones that have geometry --- a ``Face`` entry with no buffers is dropped

        Parameters
        ----------
        path: :class:`str`
            The ``hash.json``

        Returns
        -------
        List[Dict[:class:`str`, :class:`object`]]
            One dict per component: ``name``, ``position``, ``blend``, ``ib`` (hashes) and
            ``objects`` (the object classifications in draw order)
        """

        with open(path, "r", encoding = "utf-8") as f:
            entries = json.load(f)

        result = []
        for entry in entries:
            position = (entry.get("position_vb") or "").lower()
            blend = (entry.get("blend_vb") or "").lower()
            ib = (entry.get("ib") or "").lower()
            if (not position or not ib):
                continue
            result.append({"name": entry.get("component_name") or "", "position": position, "blend": blend, "ib": ib,
                           "texcoord": (entry.get("texcoord_vb") or "").lower(), "draw": (entry.get("draw_vb") or "").lower(),
                           "objects": list(entry.get("object_classifications") or []),
                           "objectIndexes": [int(value) for value in (entry.get("object_indexes") or [])]})
        return result

    # ---------------------------------------------------------------------------------------------
    # reading

    @classmethod
    def fromFolder(cls, folder: str, name: Optional[str] = None, silent: bool = False,
                   hashes: Optional[Sequence[str]] = None) -> "DumpMod":
        """
        Reads a **single-component** mod's geometry out of a folder, whichever form it is in ---
        see :meth:`Character.fromFolder`, which this wraps

        Parameters
        ----------
        folder: :class:`str`
            The folder holding the geometry files

        name: Optional[:class:`str`]
            The name of the mod. If ``None``, it is taken from the files

        silent: :class:`bool`
            Whether to skip printing progress

        hashes: Optional[Sequence[:class:`str`]]
            For a frame analysis: the character's ``(position, blend, ib)`` hashes

        Returns
        -------
        :class:`DumpMod`
            The mod's geometry

        Raises
        ------
        :class:`FileNotFoundError`
            If the folder holds none of the forms, or is a frame analysis given without hashes

        :class:`ValueError`
            If the folder holds a character of several components
        """

        return Character.fromFolder(folder, name = name, silent = silent, hashes = hashes).single()

    @classmethod
    def fromFrameAnalysis(cls, folder: str, positionHash: str, blendHash: str, ibHash: str,
                          name: Optional[str] = None, silent: bool = False, component: str = "",
                          objectNames: Optional[Sequence[str]] = None) -> "DumpMod":
        """
        Reads one component's geometry straight out of a raw 3dmigoto frame analysis folder :raw-html:`<br />` :raw-html:`<br />`

        A frame analysis dumps every buffer of every draw call, named ``NNNNNN-<slot>=<hash>-...``,
        so a character's files are picked out by hash: its position buffer is a ``vb0`` dump and its
        blend buffer a ``vb1`` dump of the same (pose) draw call, and its index buffer is drawn once
        per object and once more per pass. The same buffer dumped by several draw calls is the same
        file under several names, so only the first of each is read; the index buffers are told
        apart by their ``first index`` and named ``objectNames`` (default :attr:`ObjectOrder`) in
        that order

        Parameters
        ----------
        folder: :class:`str`
            The frame analysis folder

        positionHash: :class:`str`
            The component's position buffer hash (``position_vb`` in a ``hash.json``)

        blendHash: :class:`str`
            The component's blend buffer hash (``blend_vb``)

        ibHash: :class:`str`
            The component's index buffer hash (``ib``)

        name: Optional[:class:`str`]
            The name of the mod. If ``None``, it is taken from the folder's name
            (``FrameAnalysis-Keqing-2026-09-09-213224`` -> ``Keqing``)

        silent: :class:`bool`
            Whether to skip printing progress

        component: :class:`str`
            The component being read

        objectNames: Optional[Sequence[:class:`str`]]
            The names of the component's objects, in draw order

        Returns
        -------
        :class:`DumpMod`
            The component's geometry

        Raises
        ------
        :class:`FileNotFoundError`
            If a hash has no text dump in the folder, or the position and blend dumps disagree on
            the vertex count
        """

        folder = os.path.abspath(folder)
        if (not os.path.isdir(folder)):
            raise FileNotFoundError(f"'{folder}' is not a folder")

        if (name is None):
            name = cls.nameFromFrameAnalysisFolder(folder)

        positionFiles = cls.findFrameFiles(folder, "vb0", positionHash)
        blendFiles = cls.findFrameFiles(folder, "vb1", blendHash)
        ibFiles = cls.findFrameFiles(folder, "ib", ibHash)

        for label, files, hashValue in [("position (vb0)", positionFiles, positionHash), ("blend (vb1)", blendFiles, blendHash), ("ib", ibFiles, ibHash)]:
            if (not files):
                raise FileNotFoundError(f"No {label} text dump with hash '{hashValue}' in '{folder}'. The text dumps it holds, by slot:\n"
                                        f"{cls.describeFrameAnalysisHashes(folder)}")

        if (not silent):
            print(f"Reading {os.path.basename(positionFiles[0])}" + (f" (+{len(positionFiles) - 1} duplicates skipped)" if (len(positionFiles) > 1) else ""))
        positionColumns = cls._readFrameVbText(positionFiles[0])

        if (not silent):
            print(f"Reading {os.path.basename(blendFiles[0])}" + (f" (+{len(blendFiles) - 1} duplicates skipped)" if (len(blendFiles) > 1) else ""))
        blendColumns = cls._readFrameVbText(blendFiles[0])

        positions = cls._frameElement(positionColumns, cls.PositionKey, 3, positionFiles[0])
        blendIndices = cls._frameElement(blendColumns, cls.BlendIndicesKey, 4, blendFiles[0]).astype(np.int64)
        blendWeights = cls._frameElement(blendColumns, cls.BlendWeightKey, 4, blendFiles[0]).astype(np.float64)

        if (positions.shape[0] != blendIndices.shape[0]):
            raise FileNotFoundError(f"'{os.path.basename(positionFiles[0])}' has {positions.shape[0]} vertices but "
                                    f"'{os.path.basename(blendFiles[0])}' has {blendIndices.shape[0]}: they are not the same component's buffers")

        FRB = importAPI()

        # one index buffer per object, each dumped by several draw calls: keep the first per 'first index'
        objectsByFirstIndex: Dict[int, np.ndarray] = {}
        skippedIbs = 0
        for ibPath in ibFiles:
            firstIndex = cls._ibFirstIndex(ibPath)
            if (firstIndex in objectsByFirstIndex):
                skippedIbs += 1
                continue

            if (not silent):
                print(f"Reading {os.path.basename(ibPath)} (first index {firstIndex})")

            ibFile = FRB.IbFile(b"")
            with open(ibPath, "r", encoding = "utf-8") as f:
                ibFile.readDumpStr(f.read())

            vertexIndices = cls._ibVertices(FRB, ibFile)
            if (vertexIndices is not None):
                objectsByFirstIndex[firstIndex] = vertexIndices

        if (not silent and skippedIbs):
            print(f"  (+{skippedIbs} duplicate ib dumps skipped)")

        objects = cls._nameObjectsByOrder(objectsByFirstIndex, objectNames)
        return cls(name, positions, blendIndices, blendWeights, objects, component = component)

    @classmethod
    def readDumpFiles(cls, vbPath: str, ibPaths: Sequence[str], name: str, component: str = "",
                      objectNames: Optional[Sequence[str]] = None, silent: bool = False) -> "DumpMod":
        """
        Reads one component's geometry out of its 3dmigoto dump files (the asset-repo layout)

        Parameters
        ----------
        vbPath: :class:`str`
            The ``*-vb0=<hash>.txt`` dump (position, blend and texture data in one file)

        ibPaths: Sequence[:class:`str`]
            The ``*-ib=<hash>.txt`` dumps, one per drawn object

        name: :class:`str`
            The name of the mod

        component: :class:`str`
            The component being read

        objectNames: Optional[Sequence[:class:`str`]]
            The objects' names in draw order (by their ``first index``). ``None`` names each by
            its file's stem with the mod name stripped (``GanyuBody`` -> ``Body``)

        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        :class:`DumpMod`
            The component's geometry
        """

        FRB = importAPI()

        if (not silent):
            print(f"Reading {os.path.basename(vbPath)}")

        vbFile = FRB.VbFile(b"", [])
        with open(vbPath, "r", encoding = "utf-8") as f:
            vbFile.readDumpStr(f.read())
        columns = vbFile.decodeAll()

        positions = cls._stackColumns(columns, cls.PositionKey, 3, vbPath)
        blendIndices = cls._stackColumns(columns, cls.BlendIndicesKey, 4, vbPath).astype(np.int64)
        blendWeights = cls._stackColumns(columns, cls.BlendWeightKey, 4, vbPath).astype(np.float64)

        read: List[Tuple[int, str, np.ndarray]] = []       # (firstIndex, stem, vertices)
        for ibPath in ibPaths:
            if (not silent):
                print(f"Reading {os.path.basename(ibPath)}")

            ibFile = FRB.IbFile(b"")
            with open(ibPath, "r", encoding = "utf-8") as f:
                ibFile.readDumpStr(f.read())

            vertexIndices = cls._ibVertices(FRB, ibFile)
            if (vertexIndices is not None):
                stem = cls.IbFilePattern.match(os.path.basename(ibPath)).group("stem")
                read.append((cls._ibFirstIndex(ibPath), stem, vertexIndices))

        objects: Dict[str, np.ndarray] = {}
        if (objectNames is not None):
            objects = cls._nameObjectsByOrder({firstIndex: vertices for firstIndex, _, vertices in read}, objectNames)
        else:
            for _, stem, vertices in read:
                objects[cls.objectNameFromStem(stem, name + component)] = vertices

        return cls(name, positions, blendIndices, blendWeights, objects, component = component)

    @classmethod
    def readModFiles(cls, positionPath: str, blendPath: str, ibPaths: Dict[str, str], name: str,
                     component: str = "", silent: bool = False) -> "DumpMod":
        """
        Reads one component's geometry out of a mod's raw binary files: the ``*Position.buf``
        (positions), the ``*Blend.buf`` (vertex groups and weights) and the ``*.ib`` files (which
        object each vertex is drawn in). The ``Texcoord.buf`` is not needed and not read

        Parameters
        ----------
        positionPath: :class:`str`
            The ``*Position.buf``

        blendPath: :class:`str`
            The ``*Blend.buf``

        ibPaths: Dict[:class:`str`, :class:`str`]
            The ``*.ib`` files keyed by object name

        name: :class:`str`
            The name of the mod

        component: :class:`str`
            The component being read

        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        :class:`DumpMod`
            The component's geometry

        Raises
        ------
        :class:`FileNotFoundError`
            If the position and blend files disagree on the vertex count
        """

        FRB = importAPI()

        if (not silent):
            print(f"Reading {os.path.basename(positionPath)}")
        positionColumns = FRB.PositionFile(positionPath).decodeAll()

        if (not silent):
            print(f"Reading {os.path.basename(blendPath)}")
        blendColumns = FRB.BlendFile(blendPath).decodeAll()

        positions = cls._stackColumns(positionColumns, cls.PositionKey, 3, positionPath)
        blendIndices = cls._stackColumns(blendColumns, cls.BlendIndicesKey, 4, blendPath).astype(np.int64)
        blendWeights = cls._stackColumns(blendColumns, cls.BlendWeightKey, 4, blendPath).astype(np.float64)

        if (positions.shape[0] != blendIndices.shape[0]):
            raise FileNotFoundError(f"'{os.path.basename(positionPath)}' has {positions.shape[0]} vertices but "
                                    f"'{os.path.basename(blendPath)}' has {blendIndices.shape[0]}: they are not the same component's files")

        objects: Dict[str, np.ndarray] = {}
        for objectName, ibPath in ibPaths.items():
            if (not silent):
                print(f"Reading {os.path.basename(ibPath)}")

            vertexIndices = cls._ibVertices(FRB, FRB.IbFile(ibPath))
            if (vertexIndices is not None):
                objects[objectName] = vertexIndices

        return cls(name, positions, blendIndices, blendWeights, objects, component = component)

    # ---------------------------------------------------------------------------------------------
    # helpers

    @classmethod
    def _nameObjectsByOrder(cls, objectsByFirstIndex: Dict[int, np.ndarray], objectNames: Optional[Sequence[str]]) -> Dict[str, np.ndarray]:
        names = list(objectNames) if (objectNames) else cls.ObjectOrder
        objects: Dict[str, np.ndarray] = {}
        for order, firstIndex in enumerate(sorted(objectsByFirstIndex)):
            objectName = names[order] if (order < len(names)) else f"Object{order + 1}"
            objects[objectName] = objectsByFirstIndex[firstIndex]
        return objects

    @classmethod
    def _readFrameVbText(cls, path: str) -> Dict[str, Tuple[List[int], List[List[float]]]]:
        """
        Reads the data lines of one frame analysis vertex buffer text dump
        (``vb1[12]+016 BLENDINDICES: 17, 18, 16, 0``) into, per element name, the vertex indices
        and the values of every line :raw-html:`<br />` :raw-html:`<br />`

        The header is deliberately ignored: a frame analysis writes the *whole* input layout
        (every slot's elements) into every slot's dump, so it does not describe the lines below it
        """

        result: Dict[str, Tuple[List[int], List[List[float]]]] = {}
        with open(path, "r", encoding = "utf-8") as f:
            for line in f:
                match = cls.FrameLinePattern.match(line.rstrip("\r\n"))
                if (match is None):
                    continue

                vertices, values = result.setdefault(match.group("name"), ([], []))
                vertices.append(int(match.group("vertex")))
                values.append([float(value) for value in match.group("values").split(",")])

        return result

    @classmethod
    def _frameElement(cls, columns: Dict[str, Tuple[List[int], List[List[float]]]], key: str, count: int, path: str) -> np.ndarray:
        """
        One element of :meth:`_readFrameVbText`'s result as a ``(vertexCount, count)`` array,
        checked to be one line per vertex in order
        """

        key = next((alias for alias in cls.ElementAliases.get(key, (key,)) if (alias in columns)), key)
        if (key not in columns):
            raise KeyError(f"'{os.path.basename(path)}' has no '{key}' lines")

        vertices, values = columns[key]
        vertexIndices = np.array(vertices)
        if (not np.array_equal(vertexIndices, np.arange(len(vertices)))):
            raise KeyError(f"'{os.path.basename(path)}' does not hold one '{key}' line per vertex, in order")

        result = np.array(values, dtype = np.float64)
        if (result.ndim != 2 or result.shape[1] < count):
            raise KeyError(f"'{os.path.basename(path)}' has fewer than {count} values per '{key}' line")
        return result[:, :count]

    @classmethod
    def _ibFirstIndex(cls, path: str) -> int:
        """
        The ``first index`` of an index buffer text dump's header
        """

        with open(path, "r", encoding = "utf-8") as f:
            for _ in range(10):
                line = f.readline()
                if (line.lower().startswith("first index:")):
                    return int(line.split(":", 1)[1].strip())
        return 0

    @classmethod
    def _ibVertices(cls, FRB, ibFile) -> Optional[np.ndarray]:
        """
        The sorted, unique vertex indices an index buffer references, or ``None`` if it has none
        """

        columns = ibFile.decodeAll()
        triangleColumns = [column for key, column in columns.items() if (key[0] == FRB.IbFile.TriangleBufElementKey)]
        if (not triangleColumns):
            return None
        return np.unique(np.concatenate(triangleColumns)).astype(np.int64)

    @classmethod
    def _stackColumns(cls, columns: Dict[Tuple[str, int], np.ndarray], key: str, count: int, path: str) -> np.ndarray:
        """
        Pulls the ``count`` columns of one element out of a :meth:`CppBufFile.decodeAll` result and
        stacks them into a ``(vertexCount, count)`` array

        Parameters
        ----------
        columns: Dict[Tuple[:class:`str`, :class:`int`], :class:`numpy.ndarray`]
            The decoded columns

        key: :class:`str`
            The element's key (eg. ``"POSITION"``)

        count: :class:`int`
            How many columns the element has

        path: :class:`str`
            The file the columns came from, for the error message

        Returns
        -------
        :class:`numpy.ndarray`
            The stacked columns

        Raises
        ------
        :class:`KeyError`
            If the file has no such element
        """

        key = next((alias for alias in cls.ElementAliases.get(key, (key,)) if ((alias, 0) in columns)), key)

        result: List[np.ndarray] = []
        for i in range(count):
            column = columns.get((key, i))
            if (column is None):
                raise KeyError(f"'{os.path.basename(path)}' has no element '{key}' with {count} values")
            result.append(column)

        return np.stack(result, axis = 1)


class Character():
    """
    A whole character's geometry: its components in draw order, each a :class:`DumpMod` with its
    own vertex group index space :raw-html:`<br />` :raw-html:`<br />`

    A vertex group is identified by ``(component, index)`` throughout the finder; a
    single-component character has one component named ``""``, which is also what the library's
    remap rows use for it

    Parameters
    ----------
    name: :class:`str`
        The character's name

    components: Dict[:class:`str`, :class:`DumpMod`]
        The components, in draw order

    Attributes
    ----------
    name: :class:`str`
        The character's name

    components: Dict[:class:`str`, :class:`DumpMod`]
        The components, in draw order
    """

    def __init__(self, name: str, components: Dict[str, DumpMod]):
        self.name = name
        self.components = components

    @property
    def isMultiComponent(self) -> bool:
        """
        Whether the character has more than one component (or one that is not ``""``)

        :getter: Retrieves whether the character is multi-component
        :type: :class:`bool`
        """

        return len(self.components) != 1 or "" not in self.components

    @property
    def componentNames(self) -> List[str]:
        """
        The component names in draw order

        :getter: Retrieves the names
        :type: List[:class:`str`]
        """

        return list(self.components)

    def single(self) -> DumpMod:
        """
        The one component of a single-component character

        Returns
        -------
        :class:`DumpMod`
            The component

        Raises
        ------
        :class:`ValueError`
            If the character has several components
        """

        if (len(self.components) != 1):
            raise ValueError(f"'{self.name}' has {len(self.components)} components ({', '.join(self.componentNames)}); use the Character, not one DumpMod")
        return next(iter(self.components.values()))

    def displayName(self, component: str) -> str:
        """
        The name a component's vertex groups go under in a draft's header: the character's name
        with the component appended (``YelanTranquilBody``), or just the name for ``""``

        Parameters
        ----------
        component: :class:`str`
            The component

        Returns
        -------
        :class:`str`
            The display name
        """

        return self.name + component

    @classmethod
    def parseHashes(cls, hashes: Union[None, str, Sequence[str]]) -> Optional[Dict[str, Dict[str, object]]]:
        """
        Turns the ``hashes`` a caller gives for a frame analysis into per-component hash records:
        either three hashes ``(position, blend, ib)`` for a single component, or the path of a
        ``hash.json`` describing every component

        Parameters
        ----------
        hashes: Union[None, :class:`str`, Sequence[:class:`str`]]
            The hashes, or the ``hash.json`` path (also accepted as a one-element sequence)

        Returns
        -------
        Optional[Dict[:class:`str`, Dict[:class:`str`, :class:`object`]]]
            Per component name: ``position``, ``blend``, ``ib`` and ``objects``; ``None`` if no
            hashes were given
        """

        if (hashes is None):
            return None
        if (isinstance(hashes, str)):
            hashes = [hashes]
        hashes = list(hashes)

        if (len(hashes) == 1 and hashes[0].lower().endswith(".json")):
            return {entry["name"]: entry for entry in DumpMod.readHashJson(hashes[0])}
        if (len(hashes) == 3):
            position, blend, ib = hashes
            return {"": {"name": "", "position": position, "blend": blend, "ib": ib, "objects": []}}
        raise ValueError("hashes must be three values (position, blend, ib) or the path of a hash.json")

    @classmethod
    def fromFolder(cls, folder: str, name: Optional[str] = None, silent: bool = False,
                   hashes: Union[None, str, Sequence[str]] = None) -> "Character":
        """
        Reads a character's geometry out of a folder, whichever form it is in: a raw frame
        analysis (:meth:`fromFrameAnalysis`) if its files are named by draw call, 3dmigoto dumps
        (:meth:`fromDumpFolder`) if the folder holds a ``*-vb0=<hash>.txt``, otherwise a mod's
        binary files (:meth:`fromModFolder`)

        Parameters
        ----------
        folder: :class:`str`
            The folder holding the geometry files

        name: Optional[:class:`str`]
            The character's name. If ``None``, it is taken from the files (see the readers)

        silent: :class:`bool`
            Whether to skip printing progress

        hashes: Union[None, :class:`str`, Sequence[:class:`str`]]
            For a frame analysis: the character's ``(position, blend, ib)`` hashes, or the path
            of its ``hash.json`` when it has several components

        Returns
        -------
        :class:`Character`
            The character's geometry

        Raises
        ------
        :class:`FileNotFoundError`
            If the folder holds none of the forms, or is a frame analysis given without hashes
        """

        folder = os.path.abspath(folder)
        if (not os.path.isdir(folder)):
            raise FileNotFoundError(f"'{folder}' is not a folder")

        if (DumpMod.isFrameAnalysisFolder(folder)):
            components = cls.parseHashes(hashes)
            if (components is None):
                raise FileNotFoundError(f"'{folder}' is a raw frame analysis: the character's position, blend and ib hashes (or its hash.json) are needed "
                                        f"to pick its files out of it. The text dumps it holds, by slot (vb0 = position, vb1 = blend):\n"
                                        f"{DumpMod.describeFrameAnalysisHashes(folder)}")
            return cls.fromFrameAnalysis(folder, components, name = name, silent = silent)
        if (DumpMod.isDumpFolder(folder)):
            return cls.fromDumpFolder(folder, name = name, silent = silent)
        if (DumpMod.isModFolder(folder)):
            return cls.fromModFolder(folder, name = name, silent = silent)

        raise FileNotFoundError(f"'{folder}' holds neither a '*-vb0=<hash>.txt' dump, nor a '*Position.buf' + '*Blend.buf' pair, "
                                "nor a frame analysis (this tool does not search subfolders: point it at the folder that holds the files)")

    @classmethod
    def fromFrameAnalysis(cls, folder: str, components: Dict[str, Dict[str, object]], name: Optional[str] = None,
                          silent: bool = False) -> "Character":
        """
        Reads a character out of a raw frame analysis, one component per hash record

        Parameters
        ----------
        folder: :class:`str`
            The frame analysis folder

        components: Dict[:class:`str`, Dict[:class:`str`, :class:`object`]]
            Per component name, its ``position``, ``blend``, ``ib`` hashes and ``objects``
            (see :meth:`parseHashes`)

        name: Optional[:class:`str`]
            The character's name. Defaults to what the folder's name says

        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        :class:`Character`
            The character's geometry
        """

        if (name is None):
            name = DumpMod.nameFromFrameAnalysisFolder(folder)

        read: Dict[str, DumpMod] = {}
        for componentName, entry in components.items():
            if (not silent and componentName):
                print(f"-- component {componentName}")
            read[componentName] = DumpMod.fromFrameAnalysis(folder, entry["position"], entry["blend"], entry["ib"], name = name,
                                                            silent = silent, component = componentName, objectNames = entry.get("objects") or None)
        return cls(name, read)

    @classmethod
    def fromDumpFolder(cls, folder: str, name: Optional[str] = None, silent: bool = False) -> "Character":
        """
        Reads a character out of its 3dmigoto dump folder (the asset-repo layout) :raw-html:`<br />` :raw-html:`<br />`

        With a ``hash.json`` in the folder, its components are read one by one --- each one's
        ``*-vb0=<position hash>.txt`` and ``*-ib=<ib hash>.txt`` files, with the objects named by
        its ``object_classifications`` in draw order. Without one, the folder must hold a single
        ``vb0`` dump, read as the one component ``""``

        Parameters
        ----------
        folder: :class:`str`
            The folder holding the ``*-vb0=<hash>.txt`` and ``*-ib=<hash>.txt`` dumps

        name: Optional[:class:`str`]
            The character's name. If ``None``, the folder's own name is used

        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        :class:`Character`
            The character's geometry

        Raises
        ------
        :class:`FileNotFoundError`
            If the folder has no vertex buffer dump, or has several with no ``hash.json`` to tell
            them apart
        """

        folder = os.path.abspath(folder)
        if (not os.path.isdir(folder)):
            raise FileNotFoundError(f"'{folder}' is not a folder")

        if (name is None):
            name = os.path.basename(folder.rstrip("/\\"))

        vbFiles, ibFiles = DumpMod.findDumpFiles(folder)
        if (not vbFiles):
            raise FileNotFoundError(f"No '*-vb0=<hash>.txt' vertex buffer dump found in '{folder}'")

        hashJson = os.path.join(folder, DumpMod.HashJsonName)
        entries = DumpMod.readHashJson(hashJson) if (os.path.isfile(hashJson)) else []

        # every ib dump keyed by its hash
        ibByHash: Dict[str, List[str]] = {}
        for fileName in os.listdir(folder):
            match = DumpMod.IbFilePattern.match(fileName)
            if (match is not None):
                ibByHash.setdefault(match.group("hash").lower(), []).append(os.path.join(folder, fileName))

        components: Dict[str, DumpMod] = {}
        if (entries):
            for entry in entries:
                vbPath = vbFiles.get(entry["position"])
                if (vbPath is None):
                    raise FileNotFoundError(f"'{folder}': hash.json's component '{entry['name']}' has position hash {entry['position']} but no '*-vb0={entry['position']}.txt' dump")
                if (not silent and entry["name"]):
                    print(f"-- component {entry['name']}")
                components[entry["name"]] = DumpMod.readDumpFiles(vbPath, sorted(ibByHash.get(entry["ib"], [])), name, component = entry["name"],
                                                                  objectNames = entry["objects"] or None, silent = silent)
            return cls(name, components)

        if (len(vbFiles) > 1):
            hashes = ", ".join(sorted(vbFiles))
            raise FileNotFoundError(f"'{folder}' holds vertex buffer dumps for more than one vertex buffer ({hashes}) and no hash.json to say which component each is")

        vbHash, vbPath = next(iter(vbFiles.items()))
        ibPaths = [path for _, (path, ibVbHash) in sorted(ibFiles.items()) if (ibVbHash == vbHash)]
        components[""] = DumpMod.readDumpFiles(vbPath, ibPaths, name, component = "", objectNames = None, silent = silent)
        return cls(name, components)

    @classmethod
    def fromModFolder(cls, folder: str, name: Optional[str] = None, silent: bool = False) -> "Character":
        """
        Reads a character out of a mod's raw binary files. One ``*Position.buf`` / ``*Blend.buf``
        pair is the single component ``""``; several pairs are several components, named by
        what follows their shared prefix (``YelanTranquilBodyPosition.buf`` and
        ``YelanTranquilBangPosition.buf`` -> ``Body`` and ``Bang``, mod ``YelanTranquil``). An
        ``*.ib`` file belongs to the component whose prefix it starts with

        Parameters
        ----------
        folder: :class:`str`
            The folder holding the ``.buf`` and ``.ib`` files (not searched recursively)

        name: Optional[:class:`str`]
            The character's name. If ``None``, it is whatever precedes ``Position.buf`` (for one
            pair) or the pairs' shared prefix (for several), or the folder's own name when that
            is empty

        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        :class:`Character`
            The character's geometry

        Raises
        ------
        :class:`FileNotFoundError`
            If the folder has no position/blend pair
        """

        folder = os.path.abspath(folder)
        if (not os.path.isdir(folder)):
            raise FileNotFoundError(f"'{folder}' is not a folder")

        pairs, ibFiles = DumpMod.findModComponentFiles(folder)
        if (not pairs):
            raise FileNotFoundError(f"No '*Position.buf' + '*Blend.buf' pair found in '{folder}' (this tool does not search subfolders)")

        prefixes = sorted(pairs)
        if (len(prefixes) == 1):
            modName = prefixes[0]
            componentOf = {prefixes[0]: ""}
        else:
            modName = os.path.commonprefix(prefixes)
            componentOf = {prefix: prefix[len(modName):] for prefix in prefixes}

        if (name is None):
            name = modName or os.path.basename(folder.rstrip("/\\"))

        components: Dict[str, DumpMod] = {}
        for prefix in prefixes:
            positionPath, blendPath = pairs[prefix]
            componentName = componentOf[prefix]
            # the component's .ib files: the ones whose stem starts with its prefix (longest prefix wins)
            ownIbs: Dict[str, str] = {}
            for stem, ibPath in ibFiles.items():
                owner = max((p for p in prefixes if stem.startswith(p)), key = len, default = None)
                if (owner == prefix):
                    ownIbs[DumpMod.objectNameFromStem(stem, prefix)] = ibPath
            if (not silent and componentName):
                print(f"-- component {componentName}")
            components[componentName] = DumpMod.readModFiles(positionPath, blendPath, ownIbs, name, component = componentName, silent = silent)

        return cls(name, components)
