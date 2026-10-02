r"""Bind a flat unmistakable colour at chosen ps-t registers of every remapped draw, and bisect.

The guides' own method for a colour fault nothing in the files explains, and the one that found both
earlier Chisa colour faults after the careful hypotheses failed: make one input unmistakable and see
whether the symptom moves.

The line goes immediately before each `drawindexed` of the fix's own sections, which is after the
texture lists run, so it wins over whatever they bound. A 4 x 4 texture is fine -- every texel is the
same colour, so the sample is the colour wherever the UVs land.

  py -3 flatProbe.py on  --regs 3,4,5,6,7,8,9 [--colour 255,0,255]
  py -3 flatProbe.py off
"""
import argparse
import pathlib
import re
import struct
import sys

sys.stdout.reconfigure(errors="replace")       # the mod's folder name is not cp1252

parser = argparse.ArgumentParser()
parser.add_argument("state", choices=["on", "off"])
parser.add_argument("--mod", default="ChisaParfait3")
parser.add_argument("--regs", default="3,4,5,6,7,8,9")
parser.add_argument("--colour", default="255,0,255", help="R,G,B of the flat texture")
args = parser.parse_args()

W = pathlib.Path(r"C:\Users\AlexX\Documents\Games\Mods\XXMI-Launcher-Portable-v1.8.5\WWMI")
root = next((c for c in (W / args.mod, W / "Mods" / args.mod) if c.is_dir()), None)
assert root is not None, "NOTHING WAS CHECKED: %s not found" % args.mod
inis = [p for p in root.rglob("*.ini") if "Out UI" not in p.name]
assert inis, "NOTHING WAS CHECKED: no .ini under %s" % root

mesh = next((p.parent for p in root.rglob("*BlendRemapReverse.buf")), None)
texDir = (mesh.parent / "Textures") if mesh is not None else root
flat = texDir / "FlatProbe.dds"

if (args.state == "off"):
    for path in inis:
        backup = path.with_suffix(path.suffix + ".flatProbe")
        if (backup.is_file()):
            path.write_bytes(backup.read_bytes())
            backup.unlink()
            print("  restored %s" % path.name)
    if (flat.is_file()):
        flat.unlink()
        print("  removed %s" % flat.name)
    raise SystemExit

# ---- a 4x4 uncompressed BGRA8 .dds ------------------------------------------------------------
r, g, b = (int(x) for x in args.colour.split(","))
# magic + 7 uint32 + 44 reserved + the 8-uint32 pixel format + 5 uint32 = 4 + 124
header = struct.pack("<4sIIIIIII44sIIIIIIIIIIIII",
                     b"DDS ", 124, 0x100F, 4, 4, 16, 0, 0, b"\0" * 44,
                     32, 0x41, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
                     0x1000, 0, 0, 0, 0)
assert len(header) == 128, "DDS header is %d bytes" % len(header)
texDir.mkdir(parents=True, exist_ok=True)
flat.write_bytes(header + bytes([b, g, r, 255]) * 16)
print("  wrote %s (%d,%d,%d)" % (flat.name, r, g, b))

regs = ["ps-t" + s.strip() for s in args.regs.split(",") if s.strip()]
rel = "Textures\\FlatProbe.dds" if texDir.name == "Textures" else "FlatProbe.dds"
decl = ("\n[ResourceFlatProbe]\nfilename = %s\n" % rel)
Draw = re.compile(r"^(\s*)drawindexed = ")

total = 0
for path in inis:
    backup = path.with_suffix(path.suffix + ".flatProbe")
    if (not backup.is_file()):
        backup.write_bytes(path.read_bytes())
    raw = backup.read_bytes()
    crlf = b"\r\n" in raw
    text = raw.replace(b"\r\n", b"\n").decode("utf-8")

    out = []
    section = ""
    n = 0
    for line in text.split("\n"):
        if (line.startswith("[")):
            section = line
        got = Draw.match(line)
        if (got and "RemapFix" in section):
            for reg in regs:
                out.append("%s%s = ResourceFlatProbe" % (got.group(1), reg))
            n += 1
        out.append(line)

    print("  %s: %d remapped draw(s) probed" % (path.name, n))
    total += n
    blob = ("\n".join(out) + decl).encode("utf-8")
    path.write_bytes(blob.replace(b"\n", b"\r\n") if crlf else blob)

assert total > 0, "NOTHING WAS CHECKED: no remapped drawindexed found"
print("\n%s flat on %d draw(s) across %d file(s)" % (", ".join(regs), total, len(inis)))
