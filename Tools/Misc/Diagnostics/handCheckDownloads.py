##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

# Independent check of a Data/Mod Downloads folder: parse the dump TEXT by hand (no VbFile / IbFile),
# rebuild Position / Blend / Texcoord / <obj>.ib bytes and compare with what giDownloadFolder wrote.
# Also md5 every .dds against the asset folder.
#   py -3 handCheckDownloads.py <asset folder> <download folder> <prefix>
import glob, hashlib, json, os, re, struct, sys

assets, out, prefix = sys.argv[1:4]
hashes = json.load(open(os.path.join(assets, "hash.json"), encoding="utf-8"))
checked = mismatched = 0

def compare(name, data):
    global checked, mismatched
    path = os.path.join(out, name)
    if not os.path.isfile(path):
        print("MISSING", name); mismatched += 1; return
    have = open(path, "rb").read()
    checked += 1
    if have != data:
        mismatched += 1
        print(f"DIFFER  {name}: built {len(have)} bytes, hand {len(data)} bytes")
    else:
        print(f"ok      {name} ({len(data)} bytes)")

def parseVb(path):
    verts = {}
    for line in open(path, encoding="utf-8"):
        m = re.match(r"vb0\[(\d+)\]\+\d+ (\w+): (.*)", line)
        if m:
            verts.setdefault(int(m.group(1)), []).append((m.group(2), [x.strip() for x in m.group(3).split(",")]))
    return [verts[i] for i in range(len(verts))]

for entry in hashes:
    comp = entry.get("component_name", "")
    if not (entry.get("position_vb") and entry.get("blend_vb")):
        continue
    objs = entry["object_classifications"]
    vbPath = glob.glob(os.path.join(glob.escape(assets), f"*{comp}{objs[0]}-vb0={entry['position_vb']}.txt"))
    vbPath = min(vbPath, key=lambda p: len(os.path.basename(p)))
    pos, blend, tex = bytearray(), bytearray(), bytearray()
    for v in parseVb(vbPath):
        d = {}
        order = []
        for sem, vals in v:
            key = sem
            while key in d: key += "'"
            d[key] = vals; order.append(key)
        for s in ("POSITION", "NORMAL", "TANGENT"):
            pos += struct.pack(f"<{len(d[s])}f", *map(float, d[s]))
        w = d.get("BLENDWEIGHTS", d.get("BLENDWEIGHT"))
        blend += struct.pack("<4f", *map(float, w)) + struct.pack("<4I", *map(int, d["BLENDINDICES"]))
        for k in order:
            if k.startswith("COLOR"):
                tex += bytes(int(round(float(x) * 255)) for x in d[k])
            elif k.startswith("TEXCOORD"):
                tex += struct.pack(f"<{len(d[k])}f", *map(float, d[k]))
    compare(f"{prefix}{comp}Position.buf", bytes(pos))
    compare(f"{prefix}{comp}Blend.buf", bytes(blend))
    compare(f"{prefix}{comp}Texcoord.buf", bytes(tex))
    for obj in objs:
        ibPath = glob.glob(os.path.join(glob.escape(assets), f"*{comp}{obj}-ib={entry['ib']}.txt"))
        ibPath = min(ibPath, key=lambda p: len(os.path.basename(p)))
        idx = []
        started = False
        for line in open(ibPath, encoding="utf-8"):
            if not line.strip():
                started = True; continue
            if started:
                idx += [int(x) for x in line.split()]
        compare(f"{prefix}{comp}{obj}.ib", struct.pack(f"<{len(idx)}I", *idx))

md5 = lambda p: hashlib.md5(open(p, "rb").read()).hexdigest()
assetMd5 = {md5(p): os.path.basename(p) for p in glob.glob(os.path.join(glob.escape(assets), "*.dds"))}
for p in sorted(glob.glob(os.path.join(glob.escape(out), "*.dds"))):
    checked += 1
    src = assetMd5.get(md5(p))
    if src is None:
        mismatched += 1; print("DDS NOT FROM ASSETS", os.path.basename(p))
    else:
        print(f"ok      {os.path.basename(p)} == {src}")

files = len(os.listdir(out))
print(f"\n{checked} checked of {files} files in the folder, {mismatched} mismatched")
if checked == 0 or checked != files:
    print("NOT EVERY FILE WAS CHECKED")
    sys.exit(1)
sys.exit(1 if mismatched else 0)
