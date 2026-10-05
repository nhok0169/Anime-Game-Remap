"""Typing a mod's texture FILE, for a hash the library's history cannot reach.

WHY THIS EXISTS. The hash history resolves an old hash backwards to a role, and that is the whole of
what :class:`~ModHashFixer.ModHashFixer.ModHashFixer` could do on its own --- so a hash that was
never recorded is a dead end, reported as *unrecognised* and left alone. That limit is not evenly
spread, and it falls hardest exactly where it hurts: the history was largely derived by correlating
mod files against the game's own textures at ~1.00, which only fires for a file the mod ships
UNCHANGED. A mod that REPAINTS the body --- which is most of them --- never matched, so the body and
the legs are the two roles most likely to be missing, and a run would report every accessory fixed
while the torso stayed broken.

The file is still evidence, though, and of a kind that does not depend on any history:

* **The component.** WWMI names every texture it exports ``Components-<N> t=<hash>.dds``, and N is
  the component the mod's own ``.ini`` binds it for. A file can only play a role of THAT component,
  whatever its pixels look like.
* **The kind.** Mask, normal map or diffuse, from the pixels -- see :func:`kindOf`.

For the clothing components those two together name exactly one role, which is the useful case.

NEVER WRONG, ONLY SILENT, is the property this is built to keep. Every table below is derived from
the character's own download folder rather than hand-written, and every derivation abstains when its
evidence is ambiguous: scored against the hand-made table in
``Tools/Misc/Prototypes/chisaParfaitFix.py`` --- which is an oracle, being what the confirmed-in-game
remap uses --- the component rule agrees on 14 roles, abstains on 7 and disagrees on none. A role
this cannot type stays *unrecognised*, exactly as before.
"""
import collections
import glob
import json
import os
import re
from typing import Dict, List, Optional, Tuple

ModComponentPattern = re.compile(r"components?[\s_-]*(\d+(?:-\d+)*)[\s_-]+t=", re.IGNORECASE)
#: WWMI's export name and nothing after the hash -- `Components-3 t=c7a7ec1b.dds`
ExportedName = re.compile(r"^components?[\s_-]*\d+(?:-\d+)*[\s_-]+t=[0-9a-fA-F]{8}\.dds$", re.IGNORECASE)


def componentsOfModFile(name: str) -> List[int]:
    """Every component a WWMI-exported texture's name claims (usually exactly one)"""
    match = ModComponentPattern.search(os.path.basename(name))
    return [int(n) for n in match.group(1).split("-")] if (match) else []


_shapes: Dict[Tuple[str, int, int], Optional[Tuple[Tuple[float, float, float], float, int]]] = {}


def shapeOf(path: str) -> Optional[Tuple[Tuple[float, float, float], float, int]]:
    """(mean RGB, mean saturation, distinct colours) of a ``.dds``, or None if it will not open.

    Sampled down to at most 128 x 128 before the colours are counted: the count is a measure of how
    CONTINUOUS the texture is, and an 8192 x 8192 atlas would otherwise take seconds to answer a
    question a thumbnail settles.

    Memoised on (path, size, mtime), because the same 8192 x 8192 atlas is asked about once per
    section that names it and a mod may name one five times.
    """
    try:
        stat = os.stat(path)
        key = (os.path.normcase(os.path.abspath(path)), stat.st_size, int(stat.st_mtime))
    except OSError:
        return None
    if (key in _shapes):
        return _shapes[key]
    _shapes[key] = out = _readShape(path)
    return out


def _readShape(path: str):
    try:
        import numpy as np
        import FixRaidenBoss2 as FRB
        texture = FRB.TextureFile(path)
        texture.open()
        pixels = np.frombuffer(texture.getPixels(), dtype = np.uint8).reshape(texture.height, texture.width, 4)
    except Exception:
        return None
    rgb = pixels[..., :3].astype(np.float32)
    high, low = rgb.max(2), rgb.min(2)
    saturation = float(np.mean(np.where(high > 0, (high - low) / np.maximum(high, 1), 0)))
    step = (max(1, rgb.shape[0] // 128), max(1, rgb.shape[1] // 128))
    small = pixels[::step[0], ::step[1], :3]
    colours = int(len(np.unique(small.reshape(-1, 3), axis = 0)))
    return ((float(rgb[..., 0].mean()), float(rgb[..., 1].mean()), float(rgb[..., 2].mean())), saturation, colours)


_thumbs: Dict[Tuple[str, int, int], "object"] = {}


def thumbOf(path: str, size: int = 64):
    """A ``size`` x ``size`` x 3 float thumbnail of a ``.dds``, mean-subtracted per channel, or None.

    What the thumbnail is FOR: comparing a mod's texture with the game's texture for each candidate
    role. A mod's repainted normal map still has the game's normal map's layout -- the same UV
    islands in the same places -- so it correlates far better with the game's normal than with the
    game's mask, and that comparison is what a threshold on the pixels could not do reliably.
    """
    try:
        import numpy as np
        stat = os.stat(path)
        key = (os.path.normcase(os.path.abspath(path)), stat.st_size, int(stat.st_mtime))
    except OSError:
        return None
    if (key in _thumbs):
        return _thumbs[key]
    out = None
    try:
        import FixRaidenBoss2 as FRB
        texture = FRB.TextureFile(path)
        texture.open()
        pixels = np.frombuffer(texture.getPixels(), dtype = np.uint8).reshape(texture.height, texture.width, 4)
        step = (max(1, pixels.shape[0] // size), max(1, pixels.shape[1] // size))
        small = pixels[::step[0], ::step[1], :3][:size, :size].astype(np.float32)
        if (small.shape[0] == size and small.shape[1] == size):
            out = small - small.mean(axis = (0, 1), keepdims = True)
    except Exception:
        out = None
    _thumbs[key] = out
    return out


def correlate(a, b) -> float:
    """How alike two mean-subtracted thumbnails are, in [-1, 1]; 0 when either is flat"""
    import numpy as np
    if (a is None or b is None or a.shape != b.shape):
        return 0.0
    na, nb = float(np.linalg.norm(a)), float(np.linalg.norm(b))
    if (na < 1e-6 or nb < 1e-6):
        return 0.0
    return float((a * b).sum() / (na * nb))


def kindOf(shape) -> Tuple[str, str]:
    """"Mask", "Normal" or "Diffuse" from a texture's pixels, with the reason.

    CHECKED AGAINST AN ORACLE BEFORE IT WAS USED, and the obvious rules fail that check: "R and G
    centred on 127" is true of the three CLOTHING normals and false of both hair ones (Chisa's front
    hair normal averages 27, 50, 101), and "saturated with a flat alpha" is true of every mask AND of
    both hair normals. What separates those two is how many distinct colours the texture holds --- a
    mask is a handful of codes and a normal map is a continuous field. This rule gets 17 of 17 of the
    textures whose roles are known; the first one written got 15.

    Carried over from ``Tools/Misc/Prototypes/chisaParfaitFix.py``'s ``kindOfShape``, where it was
    derived and scored.
    """
    mean, saturation, colours = shape
    if (colours <= 40 and saturation >= 0.9):
        return "Mask", f"{colours} distinct colours at full saturation, so codes rather than shading"
    if (abs(mean[0] - 127) < 8 and abs(mean[1] - 127) < 8):
        return "Normal", "R and G both centred on 127"
    if (saturation >= 0.25 and colours >= 40):
        return "Normal", f"saturated and continuous ({colours} colours)"
    return "Diffuse", "neither a set of codes nor a normal map"


class TextureTyper():
    """The character's ``(component, kind) -> role`` table, built from its download folder.

    Unavailable, rather than wrong, when the folder or its usage manifest is missing --- ask
    :attr:`ready` before trusting :meth:`roleOfFile`.
    """

    def __init__(self, modType, repo: str, version: str = None):
        self.modType = modType
        self.version = version
        self.folder = self._folderOf(repo, modType.name)
        #: {component: {role: the game's own thumbnail for it}} -- what a mod's file is compared with
        self._componentThumbs: Dict[int, Dict[str, "object"]] = collections.defaultdict(dict)
        #: {component: {role: "Mask" / "Normal" / "Diffuse"}}, from the game's own textures
        self._componentKinds: Dict[int, Dict[str, str]] = collections.defaultdict(dict)
        #: {(component, role): the hash the game binds it under}, for reporting
        self.roleOf: Dict[Tuple[int, str], str] = {}
        if (self.folder):
            self._build()

    @property
    def ready(self) -> bool:
        return bool(self._componentThumbs)

    @staticmethod
    def _folderOf(repo: str, name: str) -> Optional[str]:
        """The newest ``Data/Mod Downloads/<game>/<name>/<version>`` there is for this character"""
        found = sorted(glob.glob(os.path.join(glob.escape(os.path.join(repo, "Data", "Mod Downloads")),
                                              "*", glob.escape(name), "*", "")))
        return found[-1] if (found) else None

    def _currentRole(self, value: str) -> Optional[str]:
        """The role the library files this hash under, if it is this character's"""
        try:
            key = (self.modType.hashes.getKey(value, self.version) if (self.version)
                   else self.modType.hashes.getKey(value))
        except Exception:
            return None
        return key[-1] if (key and key[0] == self.modType.name) else None

    def _build(self):
        """Derive role -> (component, kind) from the usage manifest and the game's own textures.

        TWO RULES, THE SECOND ONLY BREAKING THE FIRST'S TIES, and both may abstain:

        1. **The component that binds it, when only one does.** Straightforward, and right for
           anything a single component owns.
        2. **The component whose OWN pass binds it**, when several do. A component's own pass is the
           shader pair binding the most of its textures; a texture appearing under a FOREIGN pass is
           another component reaching into this one --- the face pass samples the body atlas at the
           neck, which is why ``upperDiffuse`` is bound under components 2 and 3 and belongs to 3.

        What is left abstaining is the shared and global art (the sheen, the ramps, the hair atlas
        several components sample), which no single component owns and which a mod's file naming one
        component is therefore not.
        """
        manifests = glob.glob(os.path.join(glob.escape(self.folder), "*TextureUsage.json"))
        if (not manifests):
            return
        try:
            with open(manifests[0], encoding = "utf-8") as f:
                usage = json.load(f)
        except Exception:
            return

        ownPass, boundUnder = {}, collections.defaultdict(set)
        for section, registers in usage.items():
            try:
                component = int(str(section).split()[-1])
            except ValueError:
                continue
            tally = collections.defaultdict(set)
            for entries in registers.values():
                for entry in entries:
                    value, _, shader = str(entry).partition("-")
                    tally[shader].add(value.lower())
                    boundUnder[value.lower()].add((component, shader))
            if (tally):
                ownPass[component] = max(tally, key = lambda s: len(tally[s]))

        for value, places in boundUnder.items():
            role = self._currentRole(value)
            if (not role):
                continue
            components = sorted({c for c, _ in places})
            if (len(components) != 1):
                components = sorted({c for c, shader in places if (ownPass.get(c) == shader)})
            if (len(components) != 1):
                continue                                        # shared art: owned by no one component
            texture = glob.glob(os.path.join(glob.escape(self.folder), f"*{value}*.dds"))
            thumb = thumbOf(texture[0]) if (texture) else None
            if (thumb is None):
                continue
            self._componentThumbs[components[0]][role] = thumb
            shape = shapeOf(texture[0])
            if (shape):
                self._componentKinds[components[0]][role] = kindOf(shape)[0]
            self.roleOf[(components[0], role)] = value

    #: how much better the best role must correlate than the runner-up before a file is assigned.
    #:   A repaint keeps the game texture's LAYOUT, so the right role wins by a wide margin and a
    #:   thin one means the evidence does not actually separate them -- abstain instead.
    Margin = 0.15

    @staticmethod
    def key(path: str) -> str:
        """The one spelling of a path this module keys by.

        Not cosmetic: the mod folder arrives as whatever the caller typed, so the ``.ini``-relative
        paths come back normpath'd with backslashes while a glob of the folder keeps the forward
        slashes of the argument, and ``files[section] in typed`` then compares two spellings of the
        same file and finds nothing. Every lookup goes through here.
        """
        return os.path.normcase(os.path.abspath(path))

    def assign(self, paths: List[str]) -> Dict[str, Tuple[str, str]]:
        """{file: (role, why)} for a mod's texture files, assigned ONE TO ONE inside each component.

        WHY NOT FILE BY FILE. Classifying each texture on its own -- "is this a mask, a normal map or
        a diffuse?" -- was tried first and scored 176 agreements against 24 DISAGREEMENTS with the
        hash history, and every disagreement was a kind confusion inside the right component
        (``upperNormal`` read as ``upperMask``, ``lowerDiffuse`` as ``lowerNormal``). The thresholds
        had been calibrated on the GAME's textures, and a repainted one does not keep their
        statistics.

        What a repaint DOES keep is the layout, so each file is instead compared with the game's own
        texture for each role that component has, and the best pairing is taken greedily with a
        :attr:`Margin` -- one role per file and one file per role. A file whose best two candidates
        are within the margin is left unassigned, which is the safe answer.
        """
        out: Dict[str, Tuple[str, str]] = {}
        if (not self.folder):
            return out
        byComponent: Dict[int, List[str]] = collections.defaultdict(list)
        for path in paths:
            components = componentsOfModFile(path)
            # a file serving several components is not any one component's, so it is left alone; and
            #   the name must be WWMI's own export EXACTLY, because the count of a component's files
            #   is what the bijection below rests on -- a previous fix's `...118a1a1fRemapTex.dds`
            #   sitting beside the three real ones would make the component look like it ships four
            if (len(components) == 1 and ExportedName.match(os.path.basename(path)) and os.path.isfile(path)):
                byComponent[components[0]].append(path)

        for component, files in byComponent.items():
            roles = dict(self._componentThumbs.get(component, {}))
            if (not roles):
                continue
            takenFiles, takenRoles = set(), set()

            # (1) LAYOUT. A file the mod ships unchanged, or repaints over the game's UVs, correlates
            #     with that role's texture and with nothing else. This is the strongest evidence
            #     there is, so it goes first -- but a mod whose outfit has its OWN UVs correlates
            #     with nothing (all three of a new outfit's body textures score about 0.0), which is
            #     why it cannot be the only rule.
            scores = sorted(((correlate(thumbOf(path), other), path, role)
                             for path in files for role, other in roles.items()), reverse = True)
            best: Dict[str, List[Tuple[float, str]]] = collections.defaultdict(list)
            for score, path, role in scores:
                best[path].append((score, role))
            for score, path, role in scores:
                if (path in takenFiles or role in takenRoles or score <= 0):
                    continue
                runnerUp = next((s for s, r in best[path] if (r != role and r not in takenRoles)), 0.0)
                if (score - runnerUp < self.Margin):
                    continue
                takenFiles.add(path)
                takenRoles.add(role)
                out[self.key(path)] = (role, f"component {component} in its name, and laid out like the game's "
                                             f"{role} ({score:.2f} against {runnerUp:.2f} for the next)")

            # (2) KIND, but only as a BIJECTION. Asked of one file at a time this was wrong 13 times
            #     over the mods on hand -- a hair normal read as a hair mask, a lower mask as a lower
            #     normal -- because the thresholds were calibrated on the GAME's textures. Asked of a
            #     component's files together it is a much narrower claim: a component has one diffuse,
            #     one normal map and one mask, so if its remaining files carry DISTINCT kinds and
            #     those kinds match the remaining roles one for one, there is only one way to pair
            #     them and no threshold has to be trusted to rank anything. Any repeat or any gap and
            #     the whole component abstains.
            kinds = self._componentKinds.get(component, {})
            wanted = {role: kind for role, kind in kinds.items() if (role not in takenRoles)}
            # The component must field the FULL diffuse / normal / mask triple of ROLES. Where only
            #   two are known there is nothing but the kind rule choosing between the two pairings,
            #   which is a coin flip dressed as a deduction -- and it is the case the audit got wrong,
            #   a hair mask and a hair normal swapped.
            if (sorted(wanted.values()) != ["Diffuse", "Mask", "Normal"]):
                continue
            roleOfKind = {kind: role for role, kind in wanted.items()}

            mine = {}
            for path in (p for p in files if (p not in takenFiles)):
                shape = shapeOf(path)
                if (shape):
                    mine[path] = kindOf(shape)[0]
            # NOT a count of files against a count of roles: a component may ship MORE than its three
            #   own textures -- Chisa16's lower body carries the shared red detail map as a fourth --
            #   so requiring the two to be equal abstains on exactly the mods that need this most.
            #   The claim is made per KIND instead: a file is this component's diffuse when it is the
            #   ONLY file of the component that reads as one. An extra that reads as a normal map
            #   costs the normal map its assignment and leaves the other two standing, which is the
            #   right blast radius.
            tally = collections.Counter(mine.values())
            for path, kind in mine.items():
                if (tally[kind] != 1 or kind not in roleOfKind):
                    continue
                out[self.key(path)] = (roleOfKind[kind], f"component {component} in its name, and it is the only one "
                                                         f"of the {len(mine)} textures it ships that reads as a {kind.lower()}")
        return out
