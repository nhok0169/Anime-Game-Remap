import os
import re
import sys
from collections import Counter
from typing import Dict, List, Optional, Sequence, Tuple

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
    The geometry of one mod: every vertex's position and the vertex groups it belongs to, plus
    which drawn object each vertex is part of :raw-html:`<br />` :raw-html:`<br />`

    Read from either of the two forms a GI character's geometry comes in:

    - the 3dmigoto frame-analysis **dumps** (``*-vb0=<hash>.txt`` and ``*-ib=<hash>.txt``), in the
      layout the `GI-Model-Importer-Assets <https://github.com/SilentNightSound/GI-Model-Importer-Assets>`_
      repo uses --- see :meth:`fromDumpFolder`
    - the raw binary files of a **mod** (``*Position.buf``, ``*Blend.buf`` and the ``*.ib`` files)
      --- see :meth:`fromModFolder`
    - a raw 3dmigoto **frame analysis** folder (``NNNNNN-vb0=<hash>-vs=...txt`` and friends, thousands
      of files, many duplicated), picked out by the character's position / blend / index buffer
      hashes --- see :meth:`fromFrameAnalysis`

    :meth:`fromFolder` tells the three apart by what the folder holds

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

    # the drawn objects of a character, in the order their index buffers are drawn
    ObjectOrder = ["Head", "Body", "Dress", "Extra"]

    PositionKey = "POSITION"
    BlendIndicesKey = "BLENDINDICES"
    BlendWeightKey = "BLENDWEIGHT"

    def __init__(self, name: str, positions: np.ndarray, blendIndices: np.ndarray, blendWeights: np.ndarray,
                 objects: Optional[Dict[str, np.ndarray]] = None):
        self.name = name
        self.positions = positions
        self.blendIndices = blendIndices
        self.blendWeights = blendWeights
        self.objects = {} if (objects is None) else objects

    @property
    def vertexCount(self) -> int:
        """
        The number of vertices in the mod

        :getter: Retrieves the vertex count
        :type: :class:`int`
        """

        return int(self.positions.shape[0])

    @property
    def vertexGroupCount(self) -> int:
        """
        The number of vertex groups the mod refers to (one past the largest index used) :raw-html:`<br />` :raw-html:`<br />`

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
              object's ``vb0`` dump of a character is the same file, so it is only read once)
            * the index buffer dumps, keyed by the file's stem (eg. ``"GanyuTwilightBody"``), holding
              ``(path, vbHash)``, where ``vbHash`` is the hash of the vertex buffer dump next to it
        """

        vbFiles: Dict[str, str] = {}
        ibFiles: Dict[str, Tuple[str, str]] = {}
        stemsToVbHash: Dict[str, str] = {}

        for fileName in sorted(os.listdir(folder)):
            vbMatch = cls.VbFilePattern.match(fileName)
            if (vbMatch is not None):
                vbFiles.setdefault(vbMatch.group("hash"), os.path.join(folder, fileName))
                stemsToVbHash[vbMatch.group("stem")] = vbMatch.group("hash")

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
        Finds a mod's binary geometry files in a folder

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

        positionFiles: List[str] = []
        blendFiles: List[str] = []
        ibFiles: Dict[str, str] = {}

        for fileName in sorted(os.listdir(folder)):
            path = os.path.join(folder, fileName)
            if (not os.path.isfile(path)):
                continue

            if (cls.PositionBufPattern.match(fileName)):
                positionFiles.append(path)
            elif (cls.BlendBufPattern.match(fileName)):
                blendFiles.append(path)
            else:
                ibMatch = cls.IbBinaryPattern.match(fileName)
                if (ibMatch is not None):
                    ibFiles[ibMatch.group("stem")] = path

        def preferOwn(paths: List[str]) -> Optional[str]:
            own = [path for path in paths if (cls.RemappedBufMarker not in os.path.basename(path).lower())]
            chosen = own if (own) else paths
            return chosen[0] if (chosen) else None

        return preferOwn(positionFiles), preferOwn(blendFiles), ibFiles

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

        positionFile, blendFile, _ = cls.findModFiles(folder)
        return positionFile is not None and blendFile is not None

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

    # ---------------------------------------------------------------------------------------------
    # reading

    @classmethod
    def fromFolder(cls, folder: str, name: Optional[str] = None, silent: bool = False,
                   hashes: Optional[Sequence[str]] = None) -> "DumpMod":
        """
        Reads a mod's geometry out of a folder, whichever form it is in: a raw frame analysis
        (:meth:`fromFrameAnalysis`) if its files are named by draw call, 3dmigoto dumps
        (:meth:`fromDumpFolder`) if the folder holds a ``*-vb0=<hash>.txt``, otherwise a mod's
        binary files (:meth:`fromModFolder`)

        Parameters
        ----------
        folder: :class:`str`
            The folder holding the geometry files

        name: Optional[:class:`str`]
            The name of the mod. If ``None``, it is taken from the files (see the readers)

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
        """

        folder = os.path.abspath(folder)
        if (not os.path.isdir(folder)):
            raise FileNotFoundError(f"'{folder}' is not a folder")

        if (cls.isFrameAnalysisFolder(folder)):
            if (hashes is None or len(hashes) != 3):
                raise FileNotFoundError(f"'{folder}' is a raw frame analysis: the character's position, blend and ib hashes are needed "
                                        f"to pick its files out of it. The text dumps it holds, by slot (vb0 = position, vb1 = blend):\n"
                                        f"{cls.describeFrameAnalysisHashes(folder)}")
            positionHash, blendHash, ibHash = hashes
            return cls.fromFrameAnalysis(folder, positionHash, blendHash, ibHash, name = name, silent = silent)
        if (cls.isDumpFolder(folder)):
            return cls.fromDumpFolder(folder, name = name, silent = silent)
        if (cls.isModFolder(folder)):
            return cls.fromModFolder(folder, name = name, silent = silent)

        raise FileNotFoundError(f"'{folder}' holds neither a '*-vb0=<hash>.txt' dump, nor a '*Position.buf' + '*Blend.buf' pair, "
                                "nor a frame analysis (this tool does not search subfolders: point it at the folder that holds the files)")

    @classmethod
    def fromFrameAnalysis(cls, folder: str, positionHash: str, blendHash: str, ibHash: str,
                          name: Optional[str] = None, silent: bool = False) -> "DumpMod":
        """
        Reads a character's geometry straight out of a raw 3dmigoto frame analysis folder :raw-html:`<br />` :raw-html:`<br />`

        A frame analysis dumps every buffer of every draw call, named ``NNNNNN-<slot>=<hash>-...``,
        so a character's files are picked out by hash: its position buffer is a ``vb0`` dump and its
        blend buffer a ``vb1`` dump of the same (pose) draw call, and its index buffer is drawn once
        per object and once more per pass. The same buffer dumped by several draw calls is the same
        file under several names, so only the first of each is read; the index buffers are told
        apart by their ``first index`` and named :attr:`ObjectOrder` in that order

        Parameters
        ----------
        folder: :class:`str`
            The frame analysis folder

        positionHash: :class:`str`
            The character's position buffer hash (``position_vb`` in a ``hash.json``)

        blendHash: :class:`str`
            The character's blend buffer hash (``blend_vb``)

        ibHash: :class:`str`
            The character's index buffer hash (``ib``)

        name: Optional[:class:`str`]
            The name of the mod. If ``None``, it is taken from the folder's name
            (``FrameAnalysis-Keqing-2026-09-09-213224`` -> ``Keqing``)

        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        :class:`DumpMod`
            The mod's geometry

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
                                    f"'{os.path.basename(blendFiles[0])}' has {blendIndices.shape[0]}: they are not the same character's buffers")

        FRB = importAPI()

        # one index buffer per object, each dumped by several draw calls: keep the first per 'first index'
        objectsByFirstIndex: Dict[int, np.ndarray] = {}
        skippedIbs = 0
        for ibPath in ibFiles:
            firstIndex = cls._frameIbFirstIndex(ibPath)
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

        objects: Dict[str, np.ndarray] = {}
        for order, firstIndex in enumerate(sorted(objectsByFirstIndex)):
            objectName = cls.ObjectOrder[order] if (order < len(cls.ObjectOrder)) else f"Object{order + 1}"
            objects[objectName] = objectsByFirstIndex[firstIndex]

        return cls(name, positions, blendIndices, blendWeights, objects)

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
    def _frameIbFirstIndex(cls, path: str) -> int:
        """
        The ``first index`` of a frame analysis index buffer text dump's header
        """

        with open(path, "r", encoding = "utf-8") as f:
            for _ in range(10):
                line = f.readline()
                if (line.lower().startswith("first index:")):
                    return int(line.split(":", 1)[1].strip())
        return 0

    @classmethod
    def fromDumpFolder(cls, folder: str, name: Optional[str] = None, silent: bool = False) -> "DumpMod":
        """
        Reads a mod's geometry out of a character's 3dmigoto dump folder

        Parameters
        ----------
        folder: :class:`str`
            The folder holding the ``*-vb0=<hash>.txt`` and ``*-ib=<hash>.txt`` dumps

        name: Optional[:class:`str`]
            The name of the mod. If ``None``, the folder's own name is used

        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        :class:`DumpMod`
            The mod's geometry

        Raises
        ------
        :class:`FileNotFoundError`
            If the folder has no vertex buffer dump
        """

        folder = os.path.abspath(folder)
        if (not os.path.isdir(folder)):
            raise FileNotFoundError(f"'{folder}' is not a folder")

        if (name is None):
            name = os.path.basename(folder.rstrip("/\\"))

        vbFiles, ibFiles = cls.findDumpFiles(folder)
        if (not vbFiles):
            raise FileNotFoundError(f"No '*-vb0=<hash>.txt' vertex buffer dump found in '{folder}'")

        if (len(vbFiles) > 1):
            hashes = ", ".join(sorted(vbFiles))
            raise FileNotFoundError(f"'{folder}' holds vertex buffer dumps for more than one vertex buffer ({hashes}); point the tool at a folder of a single character")

        FRB = importAPI()

        vbHash, vbPath = next(iter(vbFiles.items()))
        if (not silent):
            print(f"Reading {os.path.basename(vbPath)}")

        vbFile = FRB.VbFile(b"", [])
        with open(vbPath, "r", encoding = "utf-8") as f:
            vbFile.readDumpStr(f.read())
        columns = vbFile.decodeAll()

        positions = cls._stackColumns(columns, cls.PositionKey, 3, vbPath)
        blendIndices = cls._stackColumns(columns, cls.BlendIndicesKey, 4, vbPath).astype(np.int64)
        blendWeights = cls._stackColumns(columns, cls.BlendWeightKey, 4, vbPath).astype(np.float64)

        objects: Dict[str, np.ndarray] = {}
        for stem, (ibPath, _) in ibFiles.items():
            if (not silent):
                print(f"Reading {os.path.basename(ibPath)}")

            ibFile = FRB.IbFile(b"")
            with open(ibPath, "r", encoding = "utf-8") as f:
                ibFile.readDumpStr(f.read())

            vertexIndices = cls._ibVertices(FRB, ibFile)
            if (vertexIndices is not None):
                objects[cls.objectNameFromStem(stem, name)] = vertexIndices

        return cls(name, positions, blendIndices, blendWeights, objects)

    @classmethod
    def fromModFolder(cls, folder: str, name: Optional[str] = None, silent: bool = False) -> "DumpMod":
        """
        Reads a mod's geometry out of its raw binary files: the ``*Position.buf`` (positions),
        the ``*Blend.buf`` (vertex groups and weights) and the ``*.ib`` files (which object each
        vertex is drawn in) :raw-html:`<br />` :raw-html:`<br />`

        The ``Texcoord.buf`` is not needed and not read. A ``*RemapBlend.buf`` the remapper wrote
        next to the mod's own blend file is ignored in favour of the mod's own

        Parameters
        ----------
        folder: :class:`str`
            The folder holding the ``.buf`` and ``.ib`` files (not searched recursively)

        name: Optional[:class:`str`]
            The name of the mod. If ``None``, it is whatever precedes ``Position.buf`` in the
            position file's name (``GanyuPosition.buf`` -> ``Ganyu``), or the folder's own name
            when the file is just ``Position.buf``

        silent: :class:`bool`
            Whether to skip printing progress

        Returns
        -------
        :class:`DumpMod`
            The mod's geometry

        Raises
        ------
        :class:`FileNotFoundError`
            If the folder lacks a position or a blend file, or the two disagree on the vertex count
        """

        folder = os.path.abspath(folder)
        if (not os.path.isdir(folder)):
            raise FileNotFoundError(f"'{folder}' is not a folder")

        positionPath, blendPath, ibFiles = cls.findModFiles(folder)
        if (positionPath is None or blendPath is None):
            missing = "*Position.buf" if (positionPath is None) else "*Blend.buf"
            raise FileNotFoundError(f"No '{missing}' found in '{folder}' (this tool does not search subfolders)")

        if (name is None):
            name = cls.PositionBufPattern.match(os.path.basename(positionPath)).group("prefix")
            if (not name):
                name = os.path.basename(folder.rstrip("/\\"))

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
                                    f"'{os.path.basename(blendPath)}' has {blendIndices.shape[0]}: they are not the same mod's files")

        objects: Dict[str, np.ndarray] = {}
        for stem, ibPath in ibFiles.items():
            if (not silent):
                print(f"Reading {os.path.basename(ibPath)}")

            vertexIndices = cls._ibVertices(FRB, FRB.IbFile(ibPath))
            if (vertexIndices is not None):
                objects[cls.objectNameFromStem(stem, name)] = vertexIndices

        return cls(name, positions, blendIndices, blendWeights, objects)

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

        result: List[np.ndarray] = []
        for i in range(count):
            column = columns.get((key, i))
            if (column is None):
                raise KeyError(f"'{os.path.basename(path)}' has no element '{key}' with {count} values")
            result.append(column)

        return np.stack(result, axis = 1)
