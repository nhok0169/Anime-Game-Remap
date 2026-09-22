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
                 extraHashes: dict = None):
        """``extraHashes`` is your own ``{old hash: new hash}`` table, and it WINS over the library.

        It is what makes this usable for a character the library does not carry: the history in
        ``HashData.cpp`` only covers registered characters, and there is no reason a mod of anyone
        else should be out of reach when you already know the pair. It is also how a community
        table (wwmi_fix's ``hash_maps.json`` and friends) is fed in --- see :meth:`loadHashMaps`.

        Anything resolved this way is reported under the role ``custom (yours)``, so a run never
        hides which answers came from the library and which from you.
        """
        self.mod = mod
        self.character = character
        self.version = version
        self.geometry = geometry
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
        for path, raw in self._texts.items():
            crlf = b"\r\n" in raw
            lines = raw.decode("utf-8", "replace").replace("\r\n", "\n").split("\n")
            edited = False
            for section, i, value in self._hashesIn(raw):
                got = self._resolve(modType, value)
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
