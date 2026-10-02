r"""Paint every remapped draw its OWN flat colour, so one screenshot names which draw is which.

The ghost arm is one of the fix's own draws (it took the magenta) and the mesh is correctly skinned
(edge stretch median 1.000 against the mod on its own character), so it is a SECOND draw of geometry
that is already drawn -- and the question left is which one.

Disabling the draws one at a time is seven reloads. Giving each its own colour is one: the ghost's
colour is its answer, and anything that comes out a colour no section was given is not ours at all.

  py -3 paintProbe.py on  [--mod ChisaParfait1]
  py -3 paintProbe.py off
"""
import argparse
import pathlib
import re
import struct
import sys

sys.stdout.reconfigure(errors="replace")

parser = argparse.ArgumentParser()
parser.add_argument("state", choices=["on", "off"])
parser.add_argument("--mod", default="ChisaParfait1")
parser.add_argument("--regs", default="0,1,2")
args = parser.parse_args()

W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
root = next((c for c in (W / args.mod, W / "Mods" / args.mod) if c.is_dir()), None)
assert root is not None, "NOTHING WAS CHECKED: %s not found" % args.mod
inis = [p for p in root.rglob("*.ini") if "Out UI" not in p.name]
mesh = next((p.parent for p in root.rglob("*BlendRemapReverse.buf")), None)
texDir = (mesh.parent / "Textures") if mesh is not None else root

# one unmistakable, well-separated colour per component index
Palette = [(255, 0, 0), (0, 255, 0), (0, 128, 255), (255, 255, 0),
           (255, 0, 255), (0, 255, 255), (255, 128, 0), (128, 0, 255)]

if (args.state == "off"):
    for path in inis:
        b = path.with_suffix(path.suffix + ".paint")
        if (b.is_file()):
            path.write_bytes(b.read_bytes())
            b.unlink()
            print("  restored %s" % path.name)
    for f in texDir.glob("PaintProbe*.dds"):
        f.unlink()
    print("  removed the probe textures")
    raise SystemExit


def dds(r, g, b):
    head = struct.pack("<4sIIIIIII44sIIIIIIIIIIIII",
                       b"DDS ", 124, 0x100F, 4, 4, 16, 0, 0, b"\0" * 44,
                       32, 0x41, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
                       0x1000, 0, 0, 0, 0)
    assert len(head) == 128
    return head + bytes([b, g, r, 255]) * 16


texDir.mkdir(parents=True, exist_ok=True)
for i, (r, g, b) in enumerate(Palette):
    (texDir / ("PaintProbe%d.dds" % i)).write_bytes(dds(r, g, b))
rel = "Textures\\PaintProbe%d.dds" if texDir.name == "Textures" else "PaintProbe%d.dds"
regs = ["ps-t" + s.strip() for s in args.regs.split(",") if s.strip()]

Draw = re.compile(r"^(\s*)drawindexed = ")
Header = re.compile(r"^\[TextureOverride\w*?Component(\d+)\w*RemapFix\]$")
decl = "".join("\n[ResourcePaintProbe%d]\nfilename = %s\n" % (i, rel % i) for i in range(len(Palette)))
total = 0

for path in inis:
    backup = path.with_suffix(path.suffix + ".paint")
    if (not backup.is_file()):
        backup.write_bytes(path.read_bytes())
    raw = backup.read_bytes()
    crlf = b"\r\n" in raw
    text = raw.replace(b"\r\n", b"\n").decode("utf-8")

    out, comp, n = [], None, 0
    for line in text.split("\n"):
        got = Header.match(line.strip())
        if (line.startswith("[")):
            comp = int(got.group(1)) % len(Palette) if got else None
        d = Draw.match(line)
        if (d and comp is not None):
            for reg in regs:
                out.append("%s%s = ResourcePaintProbe%d" % (d.group(1), reg, comp))
            n += 1
            print("    %s -> colour %d %s" % (line.strip(), comp, Palette[comp]))
        out.append(line)

    print("  %s: %d draw(s) painted" % (path.name, n))
    total += n
    blob = ("\n".join(out) + decl).encode("utf-8")
    path.write_bytes(blob.replace(b"\n", b"\r\n") if crlf else blob)

assert total > 0, "NOTHING WAS CHECKED: no remapped draw matched"
print("\n%d draw(s) painted, one colour per component" % total)
