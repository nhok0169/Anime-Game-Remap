#
# ===== wwmiTextureFix =====
#
# Brings a WWMI mod's [TextureOverrideTexture] sections up to the CURRENT game version, and ships the
# textures it references but forgot -- the two ways an older WuWa mod's textures stop working, both
# silent: a texture override on a hash the game no longer binds simply never fires (the mod renders
# with the game's texture there), and an override on a current hash whose file the mod does not
# carry binds a missing resource (the material mask goes black). Neither says anything anywhere.
#
#   python wwmiTextureFix.py <mod folder> --assets <folder> [--maps hash_maps.json ...] [--apply]
#
# `--assets` is where the character's CURRENT textures are: a WWMI-Assets/PlayerCharacterData/<Name>
# folder (`Components-N t=<hash>.dds`) or a Data/Mod Downloads/WuWa/<Name>/<ver> folder
# (`<Name>Texture<hash>.dds`). `--maps` takes the community fixers' old -> new hash tables
# (wwmi_fix's hash_maps.json, any nesting) and chains them. Without --apply nothing is written.
#
# How an old hash is resolved, in order, and every decision is printed with its reason:
#   1. the hash is current -- kept;
#   2. a community map chains it to a current hash;
#   3. the mod's own file for it is pixel-identical (image correlation over 0.95, RGB and alpha, at
#      128 x 128, whatever its size or format) to exactly one current texture -- the mod carried the game's texture
#      under its old hash, so the current hash is that texture's (0.95: a face diffuse whose alpha the
#      author touched scored 0.97 on alpha and 1.00 on colour). Repainted textures do not match
#      this way and stay unresolved, on purpose: a guess there rebinds the author's art to a slot
#      nobody measured;
#   4. otherwise it is left alone and listed.
# A referenced file that is missing from the mod is copied in from the assets under the referenced
# name when the (resolved) hash has a current texture. The .ini is backed up beside itself with the
# WWMI `DISABLED` prefix, so the game never loads the backup.
#
# Needs the API's Python (`py -3` here) for the DDS decode (Tools/TexConverter); numpy and Pillow.
#

import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

import numpy as np

Repo = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
TexConverter = os.path.join(Repo, "Tools", "TexConverter", "main.py")
SectionPattern = re.compile(r"^\[(?P<name>[^\]]+)\]\s*$")
AssetName = re.compile(r"Components-[\d-]+ t=([0-9a-f]{8})\.dds$", re.IGNORECASE)
DownloadName = re.compile(r"Texture([0-9a-f]{8})\.dds$", re.IGNORECASE)


def ddsHeader(path):
    with open(path, "rb") as f:
        d = f.read(148)
    if (d[:4] != b"DDS "):
        return None
    h, w = struct.unpack_from("<II", d, 12)
    fmt = str(struct.unpack_from("<I", d, 128)[0]) if (d[84:88] == b"DX10") else d[84:88].decode("ascii", "replace")
    return (w, h, fmt)


def currentTextures(folder):
    """{hash: path} of the character's current textures in an asset or download folder"""
    out = {}
    for f in os.listdir(folder):
        m = AssetName.search(f) or DownloadName.search(f)
        if (m):
            out[m.group(1).lower()] = os.path.join(folder, f)
    return out


def loadMaps(paths):
    """every old -> new pair of the community tables, whatever their nesting"""
    pairs = {}

    def walk(node):
        if (isinstance(node, dict)):
            for k, v in node.items():
                if (isinstance(v, str)):
                    a, b = re.sub(r"^hash = ", "", k).strip().lower(), re.sub(r"^hash = ", "", v).strip().lower()
                    if (re.fullmatch(r"[0-9a-f]{8}", a) and re.fullmatch(r"[0-9a-f]{8}", b)):
                        pairs[a] = b
                else:
                    walk(v)
    for p in paths:
        with open(p, "r", encoding = "utf-8") as f:
            walk(json.load(f))
    return pairs


def chain(pairs, h, current):
    seen = []
    while (h in pairs and h not in current and h not in seen):
        seen.append(h); h = pairs[h]
    return h


def decode(path, cache):
    """the texture as a 128x128 RGBA float array, through TexConverter"""
    if (path in cache):
        return cache[path]
    from PIL import Image
    with tempfile.TemporaryDirectory() as tmp:
        r = subprocess.run([sys.executable, TexConverter, path, tmp, "-a", "keep", "-s"], capture_output = True, text = True)
        pngs = [f for f in os.listdir(tmp) if f.lower().endswith(".png")]
        if (r.returncode != 0 or not pngs):
            cache[path] = None
            return None
        cache[path] = np.asarray(Image.open(os.path.join(tmp, pngs[0])).convert("RGBA").resize((128, 128))).astype(float)
    return cache[path]


def corr(a, b):
    a = a - a.mean(); b = b - b.mean(); n = np.linalg.norm(a) * np.linalg.norm(b)
    return float((a * b).sum() / n) if n else 0.0


def main():
    parser = argparse.ArgumentParser(description = "bring a WWMI mod's texture overrides up to the current game version, and ship the textures it references but lacks")
    parser.add_argument("mod", help = "the mod folder (every .ini under it that is not DISABLED)")
    parser.add_argument("--assets", required = True, help = "the character's current textures: a WWMI-Assets PlayerCharacterData folder or a Data/Mod Downloads/WuWa/<Name>/<ver> folder")
    parser.add_argument("--maps", nargs = "*", default = [], help = "community old -> new hash tables (json), chained")
    parser.add_argument("--apply", action = "store_true", help = "write the changes (default: report only)")
    parser.add_argument("--threshold", type = float, default = 0.95, help = "image correlation needed to call a mod texture the game's own (default: %(default)s)")
    args = parser.parse_args()

    current = currentTextures(args.assets)
    if (not current):
        raise SystemExit(f"no current textures found in {args.assets}")
    pairs = loadMaps(args.maps)
    sizes = {h: ddsHeader(p) for h, p in current.items()}
    cache = {}

    for root, _, names in os.walk(args.mod):
        for name in sorted(names):
            if (not name.lower().endswith(".ini") or name.upper().startswith("DISABLED")):
                continue
            iniPath = os.path.join(root, name)
            with open(iniPath, "r", encoding = "utf-8", errors = "replace", newline = "") as f:
                text = f.read()
            ending = "\r\n" if ("\r\n" in text) else "\n"
            lines = text.split(ending)
            # sections: name -> (start, end); resources: name -> filename
            sections, order = {}, []
            for k, line in enumerate(lines):
                m = SectionPattern.match(line)
                if (m):
                    order.append((m.group("name"), k))
            for i, (sec, start) in enumerate(order):
                sections[sec] = (start, order[i + 1][1] if (i + 1 < len(order)) else len(lines))
            resources = {}
            for sec, (s, e) in sections.items():
                if (sec.startswith("Resource")):
                    for line in lines[s:e]:
                        km = re.match(r"\s*filename\s*=\s*(.+?)\s*$", line)
                        if (km):
                            resources[sec] = km.group(1).replace("\\", "/")
            print(f"\n{os.path.relpath(iniPath, args.mod)}")
            changes, copies, unresolved = [], [], []
            for sec, (s, e) in sections.items():
                if (not sec.startswith("TextureOverrideTexture")):
                    continue
                hashLine = next((k for k in range(s, e) if re.match(r"\s*hash\s*=\s*[0-9a-f]{8}\s*$", lines[k], re.IGNORECASE)), None)
                if (hashLine is None):
                    continue
                old = re.search(r"[0-9a-f]{8}", lines[hashLine], re.IGNORECASE).group(0).lower()
                refs = [v.strip() for k in range(s, e) for v in [re.sub(r"^\s*this\s*=\s*", "", lines[k])] if re.match(r"\s*this\s*=", lines[k])]
                files = [resources.get(r) for r in refs if resources.get(r)]
                new, why = old, None
                if (old in current):
                    why = "current"
                else:
                    mapped = chain(pairs, old, current)
                    if (mapped in current):
                        new, why = mapped, f"hash map {old} -> {mapped}"
                    else:
                        # the mod's own file, compared against every current texture
                        modFile = next((os.path.join(root, f) for f in files if os.path.isfile(os.path.join(root, f))), None)
                        if (modFile is not None):
                            # every current texture is a candidate, whatever its size or format: authors ship
                            # upscaled or re-encoded copies of the game's textures, and the comparison is
                            # made at 128 x 128 anyway
                            cands = list(current)
                            scores = []
                            x = decode(modFile, cache) if cands else None
                            for h in cands:
                                y = decode(current[h], cache)
                                if (x is not None and y is not None):
                                    scores.append((min(corr(x[..., :3], y[..., :3]), corr(x[..., 3], y[..., 3])), h))
                            scores.sort(reverse = True)
                            if (scores and scores[0][0] >= args.threshold and (len(scores) == 1 or scores[1][0] < args.threshold)):
                                new, why = scores[0][1], f"the mod's file is the game's own texture (correlation {scores[0][0]:.2f})"
                            elif (scores):
                                why = f"no current twin (best correlation {scores[0][0]:.2f} with {scores[0][1]})"
                            else:
                                why = "no current texture of this size and format"
                        else:
                            why = "no file to compare"
                if (new != old):
                    changes.append((sec, hashLine, old, new, why))
                    print(f"  [{sec}] hash {old} -> {new}: {why}")
                elif (why != "current"):
                    unresolved.append((sec, old, why))
                    print(f"  [{sec}] hash {old} left alone: {why}")
                # a referenced file the mod does not ship, when the (resolved) hash has a current texture
                for r in refs:
                    fn = resources.get(r)
                    if (fn and not os.path.isfile(os.path.join(root, fn))):
                        if (new in current):
                            copies.append((current[new], os.path.join(root, fn), sec))
                            print(f"  [{sec}] {fn} is MISSING from the mod: the game's own {new} goes in under that name")
                        else:
                            print(f"  [{sec}] {fn} is MISSING from the mod and {new} has no current texture to fill it")
            print(f"  {len(changes)} hash(es) to update, {len(copies)} missing file(s) to supply, {len(unresolved)} left alone")
            if (args.apply and (changes or copies)):
                backup = os.path.join(root, f"DISABLED {name}_texfix_backup")
                if (not os.path.exists(backup)):
                    shutil.copyfile(iniPath, backup)
                for sec, k, old, new, why in changes:
                    lines[k] = re.sub(old, new, lines[k], flags = re.IGNORECASE)
                with open(iniPath, "w", encoding = "utf-8", newline = "") as f:
                    f.write(ending.join(lines))
                for src, dst, sec in copies:
                    os.makedirs(os.path.dirname(dst), exist_ok = True)
                    shutil.copyfile(src, dst)
                print(f"  written; the original is {os.path.basename(backup)}")


if (__name__ == "__main__"):
    main()
