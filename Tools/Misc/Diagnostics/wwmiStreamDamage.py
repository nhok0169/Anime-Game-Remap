r"""Exactly which vertices the unbound vb6 displaces, and by how much.

`vb6` is the game's live shape-key stream: stride 24, whose first element is an
`R32G32B32_FLOAT` position offset, addressed BY VERTEX ID and sized for the vertex count of the
TARGET's draw. The fix binds vb0..vb4 and leaves vb6 alone, so a remapped vertex takes the offset
Chisa's buffer holds at the same index -- an offset computed for an unrelated vertex of a different
model -- and any index past her count reads outside the buffer entirely.

Pairs each of the fix's draws with the Chisa draw it rides on, and reports the displacement the mesh
actually receives.
"""
import collections
import glob
import os
import pathlib
import re
import struct

W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
dump = W / "FrameAnalysis-Chisa-Max-LOD-2026-09-21-080231"
mod = next(p for p in (W / "Mods" / "ChisaParfaitIdentity", W / "ChisaParfaitIdentity")
           if (p / "mod.ini").is_file())
mesh = mod / "Meshes"

idx = (mesh / "Index.buf").read_bytes()
indices = struct.unpack("<%dI" % (len(idx) // 4), idx)

# Chisa's draws that carry a vb6, keyed by the index range that identifies the component.
rides = {}
for txt in glob.glob(os.path.join(str(dump), "*-ib=*.txt")):
    head = open(txt, encoding="utf-8", errors="replace").read(400)
    first = re.search(r"first index: (\d+)", head)
    count = re.search(r"index count: (\d+)", head)
    if (not first or not count):
        continue
    drawId = os.path.basename(txt).split("-")[0]
    vb6 = glob.glob(os.path.join(str(dump), "%s-vb6=*.buf" % drawId))
    if (not vb6):
        continue
    raw = open(vb6[0], "rb").read()
    n = len(raw) // 24
    offs = [struct.unpack_from("<3f", raw, i * 24) for i in range(n)]
    rides[(int(first.group(1)), int(count.group(1)))] = (drawId, offs)

print("Chisa draws carrying a vb6 shape-key stream: %d\n" % len(rides))

# The fix's own draws, with the target range each one replaces.
ini = (mod / "mod.ini").read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")
more = (mod / "modRemapFix1.ini").read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")
blocks = re.findall(r"^\[TextureOverrideComponent\d+ChisaRemapFix\]\n(.*?)(?=\n\[)",
                    ini + "\n" + more, re.S | re.M)

print("%-26s %9s %9s %9s %12s" % ("fix draw (target range)", "verts", "past end", "displaced", "worst |offset|"))
anyHit = False
for body in blocks:
    first = re.search(r"match_first_index = (\d+)", body)
    count = re.search(r"match_index_count = (\d+)", body)
    draw = re.search(r"drawindexed = (\d+), (\d+),", body)
    if (not first or not count or not draw):
        continue
    key = (int(first.group(1)), int(count.group(1)))
    if (key not in rides):
        continue
    anyHit = True
    drawId, offs = rides[key]
    n, start = int(draw.group(1)), int(draw.group(2))
    verts = sorted(set(indices[start:start + n]))
    past = sum(1 for v in verts if v >= len(offs))
    mags = [(v, (offs[v][0] ** 2 + offs[v][1] ** 2 + offs[v][2] ** 2) ** 0.5)
            for v in verts if v < len(offs)]
    moved = [m for m in mags if m[1] > 0.01]
    worst = max((m[1] for m in mags), default=0.0)
    print("  first %-8d count %-8d %9d %9d %9d %12.3f"
          % (key[0], key[1], len(verts), past, len(moved), worst))

if (not anyHit):
    print("  (no fix draw rides a vb6-carrying Chisa draw)")

print("\nFor scale: the mesh itself spans 113-153 units from the origin, and a bone's whole")
print("influence moves a vertex a few units -- so an offset of even 1.0 is visible, and the")
print("values above are applied to vertices that never asked for them.")
