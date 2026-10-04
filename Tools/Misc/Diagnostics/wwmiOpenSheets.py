r"""Which components are OPEN SHEETS, and so can show the inside a back-face twin exists to fix?

`WWMIFixerConfig::mirroredComponents` costs a draw and two buffers, so it belongs on the components
that can actually show their inside. A closed solid -- a head, an arm -- never does: every edge of
it is shared by two triangles. An open sheet has a BORDER, and that is where its inside becomes
visible, which is exactly where ChisaParfait's skirt drew dark quadrilaterals.

Edges are keyed by POSITION rather than by vertex index, because a UV or normal seam splits a vertex
and would otherwise read as a border that is not one.

    py -3 openSheets.py [<a WWMI mod folder>]
"""
import collections
import pathlib
import re
import struct
import sys

sys.stdout.reconfigure(errors="replace")

Default = (r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI"
           r"\ChisaParfaitIdentity")
root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else Default)
assert root.is_dir(), "NOTHING WAS CHECKED: %s" % root

mesh = next(p.parent for p in root.rglob("Position.buf"))
ini = next(p for p in root.rglob("mod.ini"))
text = ini.read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")

windows = {}
for name, body in re.findall(r"^\[TextureOverrideComponent(\d+)\]\n(.*?)(?=\n\[)", text, re.S | re.M):
    first = re.search(r"match_first_index = (\d+)", body)
    count = re.search(r"match_index_count = (\d+)", body)
    if (first and count):
        windows[int(name)] = (int(first.group(1)), int(count.group(1)))
assert windows, "NOTHING WAS CHECKED: no component sections"

posRaw = (mesh / "Position.buf").read_bytes()
idxRaw = (mesh / "Index.buf").read_bytes()
pos = [struct.unpack_from("<3f", posRaw, v * 12) for v in range(len(posRaw) // 12)]
tris = struct.unpack("<%dI" % (len(idxRaw) // 4), idxRaw)
print("%s\n%d vertices, %d indices\n" % (root.name, len(pos), len(tris)))


def key(v):
    return tuple(round(c, 4) for c in pos[v])


print("%-10s %-10s %-11s %-9s %s" % ("component", "triangles", "border edges", "share", "verdict"))
for component in sorted(windows):
    first, count = windows[component]
    edges = collections.Counter()
    n = 0
    for t in range(first, first + count - 2, 3):
        a, b, c = key(tris[t]), key(tris[t + 1]), key(tris[t + 2])
        n += 1
        for p, q in ((a, b), (b, c), (c, a)):
            edges[frozenset((p, q))] += 1

    border = sum(1 for e, seen in edges.items() if seen == 1)
    share = 100.0 * border / max(1, len(edges))
    verdict = ("OPEN SHEET -- a twin can matter" if share >= 8 else
               ("some border" if share >= 2 else "closed -- a twin would be waste"))
    print("  %-8d %-10d %-11d %6.1f%%   %s" % (component, n, border, share, verdict))
