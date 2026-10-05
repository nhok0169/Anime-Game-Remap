import collections
import glob
import os
import re
import sys

# THE CHECKOUT WINS OVER AN INSTALLED COPY, and the order matters more than it looks: trying the
#   import first and falling back to the checkout only on ImportError is wrong, because a STALE
#   `FixRaidenBoss2` installed on the machine imports perfectly well and then has no `WWMIBuilder` --
#   the fallback never runs and the failure arrives as an AttributeError from somewhere unrelated.
#   So: if this file sits in a checkout, that API takes precedence; otherwise the installed package
#   is used, which is what a notebook with only `pip install AnimeGameRemap` has. Either way the path
#   must end up ABSOLUTE -- the API's native extensions cannot load from a relative sys.path entry --
#   and it is computed from `__file__` so it survives being run from another directory.
Here = os.path.dirname(os.path.abspath(__file__))
Repo = os.path.normpath(os.path.join(Here, "..", "..", ".."))
ApiSrc = os.path.abspath(os.path.join(Repo, "Anime Game Remap (for all users)", "api", "src", "py"))
if (os.path.isdir(os.path.join(ApiSrc, "FixRaidenBoss2"))):
    sys.path.insert(0, ApiSrc)
import FixRaidenBoss2 as FRB                                   # noqa: E402
from .TextureTyper import ExportedName, TextureTyper           # noqa: E402

if (not hasattr(FRB, "WWMIBuilder")):
    raise ImportError(
        f"the AGRemap API found at {os.path.dirname(os.path.abspath(FRB.__file__))} is too old for WuWa: it has no "
        f"WWMIBuilder, so it carries no Wuthering Waves character and no hash history to repair a mod with. "
        f"Update it (`pip install -U AnimeGameRemap`), or point sys.path at a checkout whose api/src/py has one.")


class ModHashFixer():
    """Brings a mod's stale asset hashes onto the game version the library knows.

    A WuWa mod whose textures have "broken" is usually a mod whose HASHES have gone stale. WWMI binds
    a mod's textures with ``[TextureOverrideTexture<N>] hash = <h>``, and the game REHASHES a texture
    between versions -- so once the hash its author exported no longer matches anything the game
    emits, the override never fires and the surface draws with the GAME's own art. Nothing is
    corrupt; the mod is addressing textures that no longer exist under those names.

    No dump is needed: the library carries each registered character's hash history in
    ``data/HashData.cpp``, so an old hash resolves BACKWARDS to a role and the role resolves FORWARD
    to the hash the current version uses.
    """

    SectionPattern = re.compile(r"^\s*\[(?P<name>[^\]]+)\]\s*$")
    HashPattern = re.compile(r"^(?P<lead>\s*hash\s*=\s*)(?P<hash>[0-9a-fA-F]{8})(?P<tail>\s*)$", re.IGNORECASE)
    BackupSuffix = ".hashfixBKUP"

    #: hash types naming GEOMETRY or a callback rather than a texture. Left alone unless asked for:
    #:   a mod whose ``vb0`` is stale does not draw AT ALL, which is a different symptom from broken
    #:   textures, and rewriting it on a mod that does draw breaks what works.
    GeometryTypes = {"vb0", "cb4", "shapekey_offsets", "shapekey_scale"}

    def __init__(self, mod: str, character: str = None, version: str = None, geometry: bool = False,
                 extraHashes: dict = None, byFile: bool = True):
        """``extraHashes`` is your own ``{old hash: new hash}`` table, and it WINS over the library.

        It is what makes this usable for a character the library does not carry: the history in
        ``HashData.cpp`` only covers registered characters, and there is no reason a mod of anyone
        else should be out of reach when you already know the pair. It is also how a community
        table (wwmi_fix's ``hash_maps.json`` and friends) is fed in --- see :meth:`loadHashMaps`.

        Anything resolved this way is reported under the role ``custom (yours)``, so a run never
        hides which answers came from the library and which from you.

        ``byFile`` allows the second resolution route, for a hash the history cannot reach: the
        texture FILE that section names is typed by its component and its pixels instead --- see
        :mod:`ModHashFixer.TextureTyper`. It matters most on the body and the legs, whose older
        hashes are the ones most often missing. Resolved that way is reported separately, and never
        overrides an answer the history gives.
        """
        self.mod = mod
        self.character = character
        self.version = version
        self.geometry = geometry
        self.byFile = byFile
        self.extraHashes = {str(k).lower(): str(v).lower() for k, v in (extraHashes or {}).items()}
        self.files = [p for p in glob.glob(os.path.join(glob.escape(mod), "**", "*.ini"), recursive = True)
                      if ("DISABLED" not in os.path.basename(p).upper())]
        self._texts = {p: open(p, "rb").read() for p in self.files}

    # ---- the library's hash history ------------------------------------------------------------

    @classmethod
    def characters(cls):
        """{name: ModType} for every registered WuWa character"""
        return {t.name: t for t in FRB.WWMIBuilder.all()}

    HashValue = re.compile(r"^[0-9a-fA-F]{8}$")

    @classmethod
    def loadHashMaps(cls, *paths):
        """{old: new} from json tables of any nesting -- the community fixers' hash_maps.json shape.

        Every string-to-string pair whose two sides both look like a hash is taken, wherever it sits
        in the document, because these tables are published with no agreed layout.
        """
        import json
        out = {}

        def walk(node):
            if (isinstance(node, dict)):
                for k, v in node.items():
                    if (isinstance(v, str) and cls.HashValue.match(str(k)) and cls.HashValue.match(v)):
                        out[str(k).lower()] = v.lower()
                    else:
                        walk(v)
            elif (isinstance(node, list)):
                for v in node:
                    walk(v)

        for path in paths:
            with open(path, encoding = "utf-8") as f:
                walk(json.load(f))
        return out

    def _resolve(self, modType, value: str):
        """(role, the hash that role has at the target version) for a hash of this character"""
        # yours first: a table you supplied is a statement, the library's history is an inference
        if (value in self.extraHashes):
            return ("custom (yours)", self.extraHashes[value])
        try:
            key = modType.hashes.getKey(value, self.version) if (self.version) else modType.hashes.getKey(value)
        except Exception:
            return None
        if (not key or key[0] != modType.name):
            return None
        role = key[-1]
        try:
            current = (modType.hashes.get((modType.name, role), self.version) if (self.version)
                       else modType.hashes.get((modType.name, role)))
        except Exception:
            return None
        return (role, current) if (current) else None

    # ---- reading -------------------------------------------------------------------------------

    @classmethod
    def _lines(cls, raw: bytes):
        """(sectionName, lineIndex, line) per line, so a rewrite can happen in place"""
        text = raw.decode("utf-8", "replace").replace("\r\n", "\n")
        out, cur = [], None
        for i, line in enumerate(text.split("\n")):
            match = cls.SectionPattern.match(line)
            if (match):
                cur = match.group("name")
            out.append((cur, i, line))
        return out

    def _hashesIn(self, raw: bytes):
        """every (section, lineIndex, hash) a fix of ours did not write"""
        for section, i, line in self._lines(raw):
            match = self.HashPattern.match(line)
            # a section named `...Remap...` is a previous fix's, and its hashes are the TARGET's
            if (match and section is not None and "remap" not in section.lower()):
                yield section, i, match.group("hash").lower()

    ThisPattern = re.compile(r"^\s*this\s*=\s*(?P<ref>\S+)\s*$", re.IGNORECASE)
    FilePattern = re.compile(r"^\s*filename\s*=\s*(?P<file>.+?)\s*$", re.IGNORECASE)

    def _resourceLines(self):
        """{resource section: (path, line index, the value)} for every ``filename =`` in the mod"""
        out = {}
        for path, raw in self._texts.items():
            for section, i, line in self._lines(raw):
                match = self.FilePattern.match(line)
                if (match and section is not None and section.lower() not in out):
                    out[section.lower()] = (path, i, match.group("file"))
        return out

    def _pointsAt(self):
        """{texture section: the resource section its ``this =`` names}"""
        out = {}
        for path, raw in self._texts.items():
            for section, _, line in self._lines(raw):
                match = self.ThisPattern.match(line)
                if (match and section is not None and section.lower() not in out):
                    out[section.lower()] = match.group("ref").lower()
        return out

    def danglingRepairs(self):
        """[(resource, path, line, old name, new name, texture section)] -- broken ``filename`` lines.

        A DANGLING FILENAME IS NOT A STALE HASH, AND IT IS WORSE. The override fires, binds a file
        that is not there, and the surface draws with the game's own art -- while every hash in the
        file is correct and every check that follows a hash passes. Chisa16's lower body is exactly
        that: ``ResourceTexture15`` names ``Components-4 t=0c153c12.dds``, which the mod does not
        ship, while ``Components-4 t=ffa1f581.dds`` -- named for that very section's hash -- sits
        beside it referenced by nothing. Her stockings were shaded as bare skin because of it.

        The repair is only offered where the evidence is exact: the section's OWN hash names a file
        the mod ships, under WWMI's ``Components-<N> t=<hash>.dds``, and there is exactly one such
        file. Anything less is left alone and reported.
        """
        files, points, resources = self._fileOfSection(), self._pointsAt(), self._resourceLines()
        onDisk = {}
        for p in self._textureFiles():
            # WWMI's export name EXACTLY -- nothing after the hash. A modder disables a texture by
            #   renaming it, and `Components-4 t=21f813ba off.dds` sits beside a live one in a real
            #   Chisa mod: matching the hash loosely would repair a reference by switching back on
            #   something its author deliberately switched off.
            if (not ExportedName.match(os.path.basename(p))):
                continue
            match = self.NamedHash.search(os.path.basename(p))
            if (match):
                onDisk.setdefault(match.group("hash").lower(), []).append(p)

        out, seen = [], set()
        for path, raw in self._texts.items():
            for section, _, value in self._hashesIn(raw):
                named = files.get(section.lower())
                if (not named or os.path.isfile(named)):
                    continue                                    # nothing wrong, or nothing to go on
                candidates = onDisk.get(value, [])
                resource = points.get(section.lower())
                if (len(candidates) != 1 or resource not in resources):
                    continue
                where, line, old = resources[resource]
                if ((where, line) in seen):
                    continue
                seen.add((where, line))
                # keep the author's own directory and separators; only the file name was ever wrong
                new = old[:len(old) - len(os.path.basename(old.replace("\\", "/")))] + os.path.basename(candidates[0])
                if (new == old):
                    # the same NAME, somewhere else in the tree -- an LOD subfolder, say. The path is
                    #   the author's business and renaming the file to itself repairs nothing.
                    continue
                out.append((resource, where, line, old, new, section))
        return out

    def _fileOfSection(self):
        """{section: the .dds it ultimately names}, across every .ini of the mod.

        A texture section does not hold its own file: ``[TextureOverrideTexture7] this =
        ResourceTexture7`` points at a resource section, and the ``filename =`` is over there --
        relative to the ``.ini`` that declares it, which is why resources are resolved per file
        before the two halves are joined.
        """
        resources, points = {}, {}
        for path, raw in self._texts.items():
            folder = os.path.dirname(path)
            for section, _, line in self._lines(raw):
                if (section is None):
                    continue
                match = self.FilePattern.match(line)
                if (match):
                    name = match.group("file").replace("\\", os.sep).replace("/", os.sep)
                    resources.setdefault(section.lower(), os.path.normpath(os.path.join(folder, name)))
                    continue
                match = self.ThisPattern.match(line)
                if (match):
                    points.setdefault(section.lower(), match.group("ref").lower())
        out = {}
        for section, ref in points.items():
            if (ref in resources):
                out[section] = resources[ref]
        for section, file in resources.items():                 # a section holding its own filename
            out.setdefault(section, file)
        return out

    def _textureFiles(self):
        """Every ``.dds`` under the mod -- the component's shipped set, whatever the .ini references"""
        return sorted(glob.glob(os.path.join(glob.escape(self.mod), "**", "*.dds"), recursive = True))

    NamedHash = re.compile(r"t=(?P<hash>[0-9a-fA-F]{8})", re.IGNORECASE)

    @classmethod
    def _fileIsFor(cls, path: str, value: str) -> bool:
        """Is this file the one the game hashed as ``value``? WWMI's own name says so.

        A texture exported by WWMI is called ``Components-<N> t=<hash>.dds``, so a section whose
        ``hash =`` matches its file's name is naming its OWN texture and the file is evidence about
        it. A section pointing at a file named for a DIFFERENT hash is not: a mod may register one
        file under several of its historical hashes, and the extra sections then describe a texture
        the file is not. That is the whole of the last disagreement the audit found -- a ``lowerMask``
        section pointing at the ``lowerDiffuse``'s file -- so the route declines those.

        A file whose name carries no hash at all is not WWMI-exported, and is left alone.
        """
        match = cls.NamedHash.search(os.path.basename(path))
        return bool(match) and match.group("hash").lower() == value.lower()

    def typer(self, modType):
        """The character's file typer, built once per run (None when it has no usable download folder)"""
        if (not self.byFile):
            return None
        if (not hasattr(self, "_typer")):
            built = TextureTyper(modType, Repo, self.version)
            self._typer = built if (built.ready) else None
        return self._typer

    def detect(self):
        """(name, {name: how many hashes it explains}) -- who this mod is FOR.

        By whoever explains the most hashes, never by the folder's name: a mod folder is named
        after whatever its author or the downloader called it.
        """
        scores = {}
        for name, modType in self.characters().items():
            scores[name] = sum(1 for raw in self._texts.values()
                               for _, _, h in self._hashesIn(raw) if (self._resolve(modType, h)))
        best = max(scores, key = lambda k: scores[k]) if (scores) else None
        return (best if (best and scores[best]) else None), scores

    # ---- the plan ------------------------------------------------------------------------------

    def plan(self, modType):
        """(changes, counts, unrecognised, newBytes) without writing anything"""
        changes, unrecognised = [], collections.Counter()
        counts = collections.Counter()
        newBytes = {}
        # BEFORE anything else: a resource pointing at a file the mod does not ship. Repaired first
        #   because the file route below types the file a section NAMES, and a section naming
        #   nothing can be typed by nothing -- so the two fixes only compose in this order.
        repairs = self.danglingRepairs() if (self.byFile) else []
        repaired = {}
        for resource, path, line, old, new, _ in repairs:
            repaired.setdefault(path, {})[line] = new
        self.repairs = repairs

        typer = self.typer(modType)
        files = self._fileOfSection() if (typer) else {}
        for resource, _, _, old, new, section in repairs:                # type against the repaired name
            if (files.get(section.lower())):
                files[section.lower()] = os.path.join(os.path.dirname(files[section.lower()]),
                                                      os.path.basename(new.replace("\\", "/")))
        # assigned once, over every texture file at once: the assignment is one role per file inside
        #   a component, so it cannot be decided a section at a time. The candidates come from the
        #   FOLDER rather than from what the `.ini` references, because the assignment rests on how
        #   many textures a component ships -- and a mod may carry a dangling `filename =` (Chisa16
        #   points its lower-body mask section at a file that is not there, while the real one sits
        #   beside it unreferenced), which would make that component look like it ships two of three.
        typed = typer.assign(self._textureFiles()) if (typer) else {}
        byFile = {}
        for path, raw in self._texts.items():
            crlf = b"\r\n" in raw
            lines = raw.decode("utf-8", "replace").replace("\r\n", "\n").split("\n")
            edited = False
            for i, new in repaired.get(path, {}).items():
                match = self.FilePattern.match(lines[i])
                if (match):
                    lines[i] = lines[i].replace(match.group("file"), new)
                    edited = True
            for section, i, value in self._hashesIn(raw):
                got = self._resolve(modType, value)
                # the history first, ALWAYS: it is a record, and typing a file is an inference. Only
                #   a hash it cannot reach is handed to the file the section names.
                file = files.get(section.lower())
                if (not got and file and TextureTyper.key(file) in typed and self._fileIsFor(file, value)):
                    role, why = typed[TextureTyper.key(file)]
                    try:
                        current = (modType.hashes.get((modType.name, role), self.version) if (self.version)
                                   else modType.hashes.get((modType.name, role)))
                    except Exception:
                        current = None
                    if (current):
                        got = (role, current)
                        byFile[value] = (role, why, os.path.basename(file))
                if (not got):
                    unrecognised[value] += 1
                    continue
                role, current = got
                if (role in self.GeometryTypes and not self.geometry):
                    counts["geometry"] += 1
                    continue
                if (current.lower() == value):
                    counts["current"] += 1
                    continue
                match = self.HashPattern.match(lines[i])
                lines[i] = f"{match.group('lead')}{current}{match.group('tail')}"
                edited = True
                changes.append((os.path.relpath(path, self.mod), section, role, value, current))
            if (edited):
                body = "\n".join(lines)
                newBytes[path] = (body.replace("\n", "\r\n") if (crlf) else body).encode("utf-8")
        #: {old hash: (role, why, file)} for whatever the FILE route resolved this run, so a caller
        #:   can report those apart from the ones the history explained
        self.byFileResolved = byFile
        return changes, counts, unrecognised, newBytes

    # ---- writing -------------------------------------------------------------------------------

    def write(self, newBytes):
        """write the planned files, keeping one backup per .ini"""
        written = []
        for path, data in newBytes.items():
            backup = path + self.BackupSuffix
            if (not os.path.isfile(backup)):
                open(backup, "wb").write(self._texts[path])
            open(path, "wb").write(data)
            written.append(path)
        return written

    def undo(self):
        """restore every .ini this tool backed up"""
        restored = []
        for path in self.files:
            backup = path + self.BackupSuffix
            if (os.path.isfile(backup)):
                open(path, "wb").write(open(backup, "rb").read())
                os.remove(backup)
                restored.append(path)
        return restored
