r"""Is the skirt already DOUBLE-SIDED geometry, or a single sheet?

It decides whether a back-face fix is even the right shape. GI's `Component::mirroredObjs` adds a
reversed twin per triangle because the cloth is a single sheet; if this mesh already carries the
twins, its inside is drawn by real triangles with their own normals and adding more would be
duplicate geometry, not a fix.

A twin is a triangle over the same three POSITIONS wound the other way, so this counts them by
position key rather than by vertex index -- an exporter that splits the seam gives the twin its own
vertex ids with identical coordinates.
"""
import collections
import pathlib
import re
import struct
import sys

sys.stdout.reconfigure(errors="replace")

W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
mod = next(c for c in (W / "Mods" / "ChisaParfaitIdentity", W / "ChisaParfaitIdentity") if c.is_dir())
mesh = next(p.parent for p in mod.rglob("Position.buf"))
ini = next(p for p in mod.rglob("mod.ini"))

text = ini.read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")
windows = {}
for name, body in re.findall(r"^\[TextureOverrideComponent(\d+)\]\n(.*?)(?=\n\[)", text, re.S | re.M):
    first = re.search(r"match_first_index = (\d+)", body)
    count = re.search(r"match_index_count = (\d+)", body)
    if (first and count):
        windows[int(name)] = (int(first.group(1)), int(count.group(1)))

posRaw = (mesh / "Position.buf").read_bytes()
idxRaw = (mesh / "Index.buf").read_bytes()
pos = [struct.unpack_from("<3f", posRaw, v * 12) for v in range(len(posRaw) // 12)]
tris = struct.unpack("<%dI" % (len(idxRaw) // 4), idxRaw)
print("%d vertices, %d indices\n" % (len(pos), len(tris)))


def key(v):
    return tuple(round(c, 4) for c in pos[v])


print("%-10s %-9s %-9s %-9s %s" % ("component", "triangles", "twinned", "share", "verdict"))
for component in sorted(windows):
    first, count = windows[component]
    wound = collections.Counter()
    faces = []
    for t in range(first, first + count - 2, 3):
        a, b, c = tris[t], tris[t + 1], tris[t + 2]
        ka, kb, kc = key(a), key(b), key(c)
        faces.append((ka, kb, kc))
        wound[frozenset((ka, kb, kc))] += 1

    twinned = 0
    seen = collections.Counter()
    for ka, kb, kc in faces:
        seen[frozenset((ka, kb, kc))] += 1
    for face, n in seen.items():
        if (n >= 2):
            twinned += n

    share = 100.0 * twinned / max(1, len(faces))
    verdict = "DOUBLE-SIDED" if share > 60 else ("mixed" if share > 5 else "single sheet")
    print("  %-8d %-9d %-9d %6.1f%%   %s" % (component, len(faces), twinned, share, verdict))
