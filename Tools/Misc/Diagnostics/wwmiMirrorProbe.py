r"""Draw a component's triangles a SECOND time, wound the other way with their normals flipped.

This is GI's `Component::mirroredObjs` by hand, on one component, as an `.ini` edit -- a prototype of
the fix before any of it is compiled. If the skirt's dark patches are its inside being lit as a back
face, the twin presents those pixels to the shader as a front face with an outward normal and they
go; if they stay, the back-face hypothesis is dead and nothing was built on it.

The mesh is a single sheet (measured: 0 twinned triangles in any of the 8 components), so the twin
is new geometry rather than a duplicate of something already there.

    py -3 mirrorProbe.py on  [--component 5]
    py -3 mirrorProbe.py off
"""
import argparse
import pathlib
import re
import struct
import sys

sys.stdout.reconfigure(errors="replace")

ap = argparse.ArgumentParser()
ap.add_argument("state", choices=["on", "off"])
ap.add_argument("--mod", default="ChisaParfaitIdentity")
ap.add_argument("--component", default="5")
args = ap.parse_args()

W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
root = next((c for c in (W / "Mods" / args.mod, W / args.mod) if c.is_dir()), None)
assert root is not None, "NOTHING WAS CHECKED: %s not found" % args.mod
inis = sorted(root.rglob("*.ini"))
mesh = next(p.parent for p in root.rglob("Position.buf"))

if (args.state == "off"):
    n = 0
    for path in inis:
        b = path.with_suffix(path.suffix + ".mirrorProbe")
        if (b.is_file()):
            path.write_bytes(b.read_bytes())
            b.unlink()
            n += 1
            print("  restored %s" % path.name)
    for p in mesh.glob("Mirror*.buf"):
        p.unlink()
        print("  removed %s" % p.name)
    assert n > 0, "NOTHING WAS CHECKED: no backup"
    raise SystemExit

# ---- the component's own index window, off the mod's own sections -------------------------------
text = next(p for p in root.rglob("mod.ini")).read_bytes().decode("utf-8", "replace").replace("\r\n", "\n")
body = re.search(r"^\[TextureOverrideComponent%s\]\n(.*?)(?=\n\[)" % args.component, text, re.S | re.M)
assert body is not None, "NOTHING WAS CHECKED: no section for component %s" % args.component
first = int(re.search(r"match_first_index = (\d+)", body.group(1)).group(1))
count = int(re.search(r"match_index_count = (\d+)", body.group(1)).group(1))
print("  component %s: %d indices from %d" % (args.component, count, first))

# ---- the reversed-winding index buffer ----------------------------------------------------------
idxRaw = (mesh / "Index.buf").read_bytes()
tris = struct.unpack("<%dI" % (len(idxRaw) // 4), idxRaw)
out = []
for t in range(first, first + count - 2, 3):
    out += [tris[t], tris[t + 2], tris[t + 1]]           # swap two corners = reverse the winding
(mesh / "MirrorIndex.buf").write_bytes(struct.pack("<%dI" % len(out), *out))
print("  MirrorIndex.buf: %d indices (%d triangles)" % (len(out), len(out) // 3))

# ---- the whole Vector.buf with every normal negated ----------------------------------------------
# R8G8B8A8_SNORM, stride 8: the normal in bytes 0..3 and the tangent in 4..7. -128 has no positive
# counterpart in SNORM, so it clamps to -127's mirror rather than wrapping to itself.
vecRaw = bytearray((mesh / "Vector.buf").read_bytes())
for i in range(0, len(vecRaw), 8):
    for c in range(3):
        v = vecRaw[i + c] - 256 if vecRaw[i + c] > 127 else vecRaw[i + c]
        v = max(-127, min(127, -v))
        vecRaw[i + c] = v + 256 if v < 0 else v
(mesh / "MirrorVector.buf").write_bytes(bytes(vecRaw))
print("  MirrorVector.buf: %d vertices, normals negated" % (len(vecRaw) // 8))

# ---- the second draw ----------------------------------------------------------------------------
Header = "[TextureOverrideComponent%sChisaRemapFix]" % args.component
total = 0
for path in inis:
    backup = path.with_suffix(path.suffix + ".mirrorProbe")
    if (not backup.is_file()):
        backup.write_bytes(path.read_bytes())
    raw = backup.read_bytes()
    crlf = b"\r\n" in raw
    lines = raw.replace(b"\r\n", b"\n").decode("utf-8").split("\n")

    out2, inSection, n = [], False, 0
    for line in lines:
        s = line.strip()
        if (s.startswith("[")):
            inSection = (s == Header)
        out2.append(line)
        got = re.match(r"^(\s*)drawindexed\s*=\s*", line)
        if (inSection and got):
            pad = got.group(1)
            out2.append("%sib = ResourceMirrorIndex" % pad)
            out2.append("%svb1 = ResourceMirrorVector" % pad)
            out2.append("%sdrawindexed = %d, 0, 0" % (pad, len(out)))
            n += 1
    if (n):
        out2.append("")
        out2.append("[ResourceMirrorIndex]")
        out2.append("type = Buffer")
        out2.append("format = DXGI_FORMAT_R32_UINT")
        out2.append("stride = 12")
        out2.append("filename = Meshes/MirrorIndex.buf")
        out2.append("")
        out2.append("[ResourceMirrorVector]")
        out2.append("type = Buffer")
        out2.append("format = DXGI_FORMAT_R8G8B8A8_SNORM")
        out2.append("stride = 8")
        out2.append("filename = Meshes/MirrorVector.buf")
        out2.append("")
        print("  %s: a mirrored twin draw added" % path.name)
    total += n
    blob = "\n".join(out2).encode("utf-8")
    path.write_bytes(blob.replace(b"\n", b"\r\n") if crlf else blob)

assert total > 0, "NOTHING WAS CHECKED: %s has no drawindexed" % Header
print("\n%d twin draw(s) added" % total)
