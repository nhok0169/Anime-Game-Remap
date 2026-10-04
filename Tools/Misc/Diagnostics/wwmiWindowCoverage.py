r"""Which of Chisa's bone windows does the fix actually merge into its OWN skeleton?

The fix merges a window per DRAWN target slot, through `CommandListMergeSlot<N>`, into
`ResourceMergedSkeletonRW<fix>`. A slot nothing is remapped onto gets a `...RemapHide` section
instead, and that one runs the HOST's copied `CommandListMergeSkeleton<fix>` -- which writes
`ResourceMergedSkeletonRW`, the mod's own buffer, NOT the fix's.

So any bone in a hidden slot's window is never filled in the buffer the remap reads, and every
vertex weighted to it skins against a zero matrix: it collapses to the origin, and a triangle with
one corner at the origin is a plane across the scene.

Reports, per window, whether a drawn section merges it and how much of the mesh depends on it.
"""
import collections
import pathlib
import re
import struct

W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
mod = next(p for p in (W / "Mods" / "ChisaParfaitIdentity", W / "ChisaParfaitIdentity")
           if (p / "mod.ini").is_file())
ini = (mod / "mod.ini").read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")
mesh = mod / "Meshes"

# Chisa's windows, read from the fix's own merge lists rather than transcribed.
windows = {}
for name, body in re.findall(r"^\[(CommandListMergeSlot\d+ChisaRemapFix)\]\n(.*?)(?=\n\[)", ini,
                             re.S | re.M):
    off = re.search(r"vg_offset = (\d+)", body)
    cnt = re.search(r"vg_count = (\d+)", body)
    if (off and cnt):
        windows[name] = (int(off.group(1)), int(cnt.group(1)))
assert windows, "NOTHING WAS CHECKED: no merge lists in the .ini"

# Which of those lists a section actually runs, and which windows only a Hide section merges.
run = set(re.findall(r"run = (CommandListMergeSlot\d+ChisaRemapFix)", ini))
hidden = []
for header, body in re.findall(r"^\[(TextureOverride\w*RemapHide)\]\n(.*?)(?=\n\[)", ini, re.S | re.M):
    off = re.search(r"vg_offset = (\d+)", body)
    cnt = re.search(r"vg_count = (\d+)", body)
    # Either of the fix's own merge lists counts: the per-slot one (merge + bind) or the
    # merge-only CommandListMergeWindow a hidden slot runs.
    toOurs = ("CommandListMergeSlot" in body) or ("CommandListMergeWindow" in body)
    if (off and cnt):
        hidden.append((header, int(off.group(1)), int(cnt.group(1)), toOurs))

merged = []
for name, (off, cnt) in sorted(windows.items(), key=lambda kv: kv[1][0]):
    merged.append((off, cnt, name in run, name))

vg = (mesh / "ChisaRemapBlendRemapVertexVG.buf").read_bytes()
blend = (mesh / "ChisaRemapBlend.buf").read_bytes()
verts = len(blend) // 16
used = collections.Counter()
for v in range(verts):
    ids = struct.unpack_from("<8H", vg, v * 16)
    for k in range(8):
        if (blend[v * 16 + 8 + k]):
            used[ids[k]] += 1

covered = set()
print(f"{'window':>14} {'merged by':>34} {'bones used':>11} {'weighted slots':>15}")
for off, cnt, isRun, name in merged:
    inWin = [b for b in used if off <= b < off + cnt]
    slots = sum(used[b] for b in inWin)
    if (isRun):
        covered.update(inWin)
    print(f"  [{off:4d}, {off + cnt:4d}) {('a drawn slot' if isRun else 'NOTHING (list never run)'):>34}"
          f" {len(inWin):11d} {slots:15d}")

for header, off, cnt, toOurs in hidden:
    inWin = [b for b in used if off <= b < off + cnt]
    slots = sum(used[b] for b in inWin)
    where = "the fix's buffer" if toOurs else "the HOST's buffer -- NOT the fix's"
    if (toOurs):
        covered.update(inWin)
    print(f"  [{off:4d}, {off + cnt:4d}) {where:>34} {len(inWin):11d} {slots:15d}"
          f"   <- {header}")

orphan = {b: n for b, n in used.items() if b not in covered}
print(f"\nbones the mesh uses whose window the fix never merges into its own skeleton: {len(orphan)}")
print(f"weighted influence slots riding on them                                    : {sum(orphan.values())}"
      f"  ({100.0 * sum(orphan.values()) / max(1, sum(used.values())):.1f}% of all)")
for b in sorted(orphan, key=lambda k: -orphan[k])[:12]:
    print(f"    bone {b:4d}: {orphan[b]:7d} weighted slots")
